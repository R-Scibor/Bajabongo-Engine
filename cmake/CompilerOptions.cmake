# Mirror the shared Visual Studio 2022 (v143) compile/link flags from
# src/engine/engine.vcxproj and src/game/game.vcxproj.
#
# Shared settings (all configs, x64 and Win32):
#   LanguageStandard        stdcpp20
#   WarningLevel            Level3          -> /W3
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

function(bajabongo_apply_compiler_options target)
    target_compile_features(${target} PRIVATE cxx_std_20)
    target_include_directories(${target} PUBLIC "${PROJECT_SOURCE_DIR}/src")

    if(MSVC)
        target_compile_options(${target} PRIVATE
            /W3
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
        # Reasonable non-MSVC equivalent of /W3. Not /WX or -Werror:
        # the vcxproj does not enable TreatWarningAsError.
        target_compile_options(${target} PRIVATE
            -Wall
            $<$<CONFIG:Release>:-O2>
        )
        target_compile_definitions(${target} PRIVATE
            $<$<CONFIG:Debug>:_DEBUG>
            $<$<CONFIG:Release>:NDEBUG>
        )
    endif()
endfunction()
