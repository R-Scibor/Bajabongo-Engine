# Mirror the shared Visual Studio 2022 (v143) compile/link flags from
# src/engine/engine.vcxproj and src/game/game.vcxproj.
#
# Shared settings (all configs, x64 and Win32):
#   LanguageStandard        stdcpp20
#   WarningLevel            Level3 in the vcxproj. CMake project targets
#                           use /W4 when BAJA_STRICT_WARNINGS is on (Linux
#                           presets already set that; Windows CMake users
#                           can turn it on). Default MSVC stays at /W3 so
#                           a VS .sln build is unchanged.
#   SDLCheck                true            -> /sdl
#   ConformanceMode         true            -> /permissive-
#   AdditionalOptions       /utf-8
#   AdditionalIncludeDirectories  $(SolutionDir)src
#   CharacterSet            Unicode         -> UNICODE, _UNICODE
#   GenerateDebugInformation true           -> /DEBUG (linker)
#
# Release only:
#   WholeProgramOptimization true           -> /GL + /LTCG
#   FunctionLevelLinking     true           -> /Gy
#   IntrinsicFunctions       true           -> /Oi
#   NDEBUG
#
# Debug only:
#   _DEBUG
#
# Platform:
#   WIN32 is defined only for the Win32 (x86) configurations, not x64
#   (MSVC also predefined WIN32 anyway; we keep the explicit define
#   only on 32-bit to mirror the vcxproj).
#
# Precompiled headers: the vcxproj creates pch.h from pch.cpp but sets
# PrecompiledHeader=NotUsing on every other TU, so PCH is effectively
# unused. This CMake build does the same (no target_precompile_headers).
#
# Runtime library is left at the VS/vcpkg default (/MD, /MDd), set
# globally in the top-level CMakeLists.txt.

# Opt-in extra warnings for this project's targets only (engine/game/tests).
# Does not affect vcpkg dependencies. Linux presets set this ON.
# Linux/GCC/Clang: -Wall -Wextra -Wshadow -Wconversion.
# MSVC: /W4 (CMake only; the .vcxproj WarningLevel stays Level3).
option(BAJA_STRICT_WARNINGS "Enable extra compiler warnings on engine/game/tests" OFF)

# Opt-in ASan + UBSan for this project's targets only (engine/game/tests).
# Does not rebuild vcpkg dependencies. Linux-only (GCC/Clang). The
# linux-ninja-debug-sanitize preset turns this ON in a separate build tree.
option(BAJA_SANITIZE "Enable AddressSanitizer and UndefinedBehaviorSanitizer (GCC/Clang)" OFF)

# Opt-in gcov coverage for this project's targets only (engine/game/tests).
# Does not rebuild vcpkg dependencies. Linux-only (GCC/Clang). Use a
# separate build tree (the linux-ninja-debug-coverage preset). Do not
# combine with BAJA_SANITIZE.
option(BAJA_COVERAGE "Enable gcov coverage instrumentation (GCC/Clang)" OFF)

function(bajabongo_apply_compiler_options target)
    target_compile_features(${target} PRIVATE cxx_std_20)
    target_include_directories(${target} PUBLIC "${PROJECT_SOURCE_DIR}/src")

    if(MSVC)
        target_compile_options(${target} PRIVATE
            $<IF:$<BOOL:${BAJA_STRICT_WARNINGS}>,/W4,/W3>
            /sdl
            /permissive-
            /utf-8
            $<$<CONFIG:Release>:/Gy /Oi /GL>
        )
        target_link_options(${target} PRIVATE
            /DEBUG
            $<$<CONFIG:Release>:/LTCG>
        )
        target_compile_definitions(${target} PRIVATE
            UNICODE
            _UNICODE
            $<$<CONFIG:Debug>:_DEBUG>
            $<$<CONFIG:Release>:NDEBUG>
            $<$<EQUAL:${CMAKE_SIZEOF_VOID_P},4>:WIN32>
        )
    else()
        # Base: -Wall is a reasonable non-MSVC equivalent of /W3.
        # BAJA_STRICT_WARNINGS adds -Wextra -Wshadow -Wconversion.
        # Not -Werror / /WX: the vcxproj does not enable TreatWarningAsError.
        if(BAJA_STRICT_WARNINGS)
            target_compile_options(${target} PRIVATE
                -Wall
                -Wextra
                -Wshadow
                -Wconversion
                $<$<CONFIG:Release>:-O2>
            )
        else()
            target_compile_options(${target} PRIVATE
                -Wall
                $<$<CONFIG:Release>:-O2>
            )
        endif()
        target_compile_definitions(${target} PRIVATE
            $<$<CONFIG:Debug>:_DEBUG>
            $<$<CONFIG:Release>:NDEBUG>
        )
        if(BAJA_SANITIZE)
            target_compile_options(${target} PRIVATE
                -fsanitize=address,undefined
                -fno-omit-frame-pointer
                -fno-sanitize-recover=address,undefined
            )
            target_link_options(${target} PRIVATE
                -fsanitize=address,undefined
            )
        endif()
        if(BAJA_COVERAGE)
            target_compile_options(${target} PRIVATE
                --coverage
                -O0
                -g
            )
            target_link_options(${target} PRIVATE
                --coverage
            )
        endif()
    endif()
endfunction()

if(BAJA_SANITIZE)
    if(MSVC)
        message(WARNING "BAJA_SANITIZE is supported on GCC/Clang only and is ignored on MSVC.")
    else()
        message(STATUS "BAJA_SANITIZE: AddressSanitizer + UndefinedBehaviorSanitizer on engine/game/tests")
    endif()
endif()

if(BAJA_COVERAGE)
    if(MSVC)
        message(WARNING "BAJA_COVERAGE is supported on GCC/Clang only and is ignored on MSVC.")
    else()
        message(STATUS "BAJA_COVERAGE: gcov instrumentation on engine/game/tests")
    endif()
endif()

if(BAJA_COVERAGE AND BAJA_SANITIZE)
    message(WARNING "BAJA_COVERAGE and BAJA_SANITIZE are both ON. Use separate build trees.")
endif()
