# Engine static library. Headers are listed so IDEs show them; they are
# not required to compile.

set(ENGINE_SOURCES
    assets/AssetManifestLoader.cpp
    assets/MapLoader.cpp
    audio/AudioSystem.cpp
    core/Application.cpp
    core/GuiService.cpp
    core/StateManager.cpp
    ecs/HierarchySystem.cpp
    input/SFMLInputManager.cpp
    logging/SpdlogLogger.cpp
    logging/SpdlogManager.cpp
    pch.cpp
    physics/PhysicsBodyCreationSystem.cpp
    physics/PhysicsEventSystem.cpp
    physics/PhysicsSyncSystem.cpp
    rendering/AnimationSystem.cpp
    rendering/CameraSystem.cpp
    rendering/RenderSystem.cpp
    rendering/SFMLRenderer.cpp
    rendering/SFMLResourceManager.cpp
    ecs/ArchetypeManager.cpp
    ecs/EditorSystem.cpp
    ecs/EntityFactory.cpp
    systems/AnimationStateMachineSystem.cpp
    systems/LifetimeSystem.cpp
)

add_library(engine STATIC ${ENGINE_SOURCES})
add_library(bajabongo::engine ALIAS engine)

bajabongo_apply_compiler_options(engine)

target_compile_definitions(engine PRIVATE _LIB)

# Public so the game executable inherits include path and link deps.
target_link_libraries(engine
    PUBLIC
        SFML::Graphics
        SFML::Window
        SFML::Audio
        SFML::System
        spdlog::spdlog
        EnTT::EnTT
        box2d::box2d
        nlohmann_json::nlohmann_json
        imgui::imgui
        ImGui-SFML::ImGui-SFML
)

# Fail configure/build if any engine source includes a game/ header.
# Include dirs still use ${PROJECT_SOURCE_DIR}/src so #include "engine/..."
# works; this check (not a narrower include path) is the enforcement.
set(ENGINE_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
include("${PROJECT_SOURCE_DIR}/cmake/CheckEngineDecoupling.cmake")

# Custom target (not PRE_BUILD): Ninja treats PRE_BUILD as PRE_LINK, and a
# phony target is reliable on every generator. engine depends on this check.
add_custom_target(check_engine_decoupling ALL
    COMMAND "${CMAKE_COMMAND}"
        "-DENGINE_SRC_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
        -P "${PROJECT_SOURCE_DIR}/cmake/CheckEngineDecoupling.cmake"
    COMMENT "Checking that engine sources do not include game/ headers"
    VERBATIM
)
add_dependencies(engine check_engine_decoupling)
