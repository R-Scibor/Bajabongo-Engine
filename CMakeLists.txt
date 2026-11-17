# Bajabongo Engine — top-level build.
#
# This CMake build mirrors the Visual Studio 2022 solution:
#   - engine: static library  (src/engine/engine.vcxproj)
#   - game:   console executable (src/game/game.vcxproj)
#
# Toolchain: intended to be used with vcpkg manifest mode (vcpkg.json at the
# repo root). Use the presets in CMakePresets.json, or pass:
#   -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
#
# The .sln / .vcxproj files are the Windows/MSVC source of truth and are not
# modified by this build.

cmake_minimum_required(VERSION 3.21)

project(Bajabongo-Engine
    VERSION 0.1.0
    DESCRIPTION "Modular, data-driven 2D game engine (C++20, SFML 3, EnTT, Box2D 3)"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Match vcpkg's default Windows triplet (x64-windows, /MD /MDd).
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND NOT DEFINED VCPKG_CHAINLOAD_TOOLCHAIN_FILE)
    message(NOTICE
        "No CMAKE_TOOLCHAIN_FILE was set. A reproducible build uses vcpkg:\n"
        "  export VCPKG_ROOT=/path/to/vcpkg\n"
        "  cmake --preset linux-ninja-debug    # or windows-vs2022\n"
        "System packages (find_package CONFIG) will be tried as a fallback, "
        "but SFML 3 / Box2D 3 / imgui-sfml are often not available that way."
    )
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CompilerOptions)
# Extra warnings: cmake -DBAJA_STRICT_WARNINGS=ON (on by default in linux presets).
option(BAJA_BUILD_TESTS "Build unit tests (Catch2) and enable CTest" OFF)
# Linux presets turn this ON. Requires the catch2 vcpkg dependency.

# Third-party dependencies from vcpkg.json:
#   sfml, spdlog, entt, box2d, nlohmann-json, imgui, imgui-sfml
# The vcxproj does not list these as AdditionalDependencies; vcpkg's
# MSBuild integration injects them. CMake does the same via CONFIG packages.
find_package(SFML 3 CONFIG REQUIRED COMPONENTS Graphics Window Audio System)
find_package(spdlog CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(box2d CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(ImGui-SFML CONFIG REQUIRED)

add_subdirectory(src/engine)
# engine CMakeLists runs CheckEngineDecoupling (no engine -> game includes).
add_subdirectory(src/game)

if(BAJA_BUILD_TESTS)
    enable_testing()
    find_package(Catch2 3 CONFIG REQUIRED)
    add_subdirectory(tests)
endif()
