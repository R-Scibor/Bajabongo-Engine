# Runtime layout so the existing hardcoded relative paths resolve
# without any source changes.
#
# The game loads files as:
#   ../../assets/...          (textures, data, shaders, fonts fallback)
#   assets/fonts/default.ttf  (first font attempt)
#   config/logging.ini        (cwd-relative; lives in src/game/config)
#
# This helper places the CMake-built executable at:
#   ${CMAKE_BINARY_DIR}/out/game/$<CONFIG>/game(.exe)
# copies the assets tree to:
#   ${CMAKE_BINARY_DIR}/out/assets
# and copies logging.ini to:
#   $<TARGET_FILE_DIR:game>/config/logging.ini
#
# From that working directory, ../../assets -> ${CMAKE_BINARY_DIR}/out/assets
# and config/logging.ini is found next to the executable. On MSVC the
# debugger working directory is set to the executable folder.
#
# Textures, sounds and music are gitignored in this repo, so only the
# files that are actually committed (data, fonts, shaders) are copied.

function(bajabongo_setup_game_runtime target)
    set(_rt "${CMAKE_BINARY_DIR}/out/game")
    set_target_properties(${target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${_rt}/$<CONFIG>"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${_rt}/Debug"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${_rt}/Release"
        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${_rt}/RelWithDebInfo"
        RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${_rt}/MinSizeRel"
        VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${target}>"
    )

    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${PROJECT_SOURCE_DIR}/assets"
            "${CMAKE_BINARY_DIR}/out/assets"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${PROJECT_SOURCE_DIR}/src/game/config"
            "$<TARGET_FILE_DIR:${target}>/config"
        COMMENT "Copying assets and config into the runtime layout (../../assets, config/logging.ini)"
        VERBATIM
    )
endfunction()
