# Mirrors src/engine/engine.vcxproj (StaticLibrary, all 24 ClCompile sources).
# Headers missing from the vcxproj but present on disk are listed so IDEs
# show them; they are not required to compile.

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

# _LIB is defined in every engine.vcxproj configuration.
target_compile_definitions(engine PRIVATE _LIB)

# Public so the game executable inherits include path and link deps,
# matching the vcxproj ProjectReference + vcpkg integration.
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
