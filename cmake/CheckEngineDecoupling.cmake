# Enforce that engine sources do not include headers from src/game.
#
# Usable two ways:
#   include() during configure, with ENGINE_SRC_DIR already set
#   cmake -DENGINE_SRC_DIR=<src/engine> -P cmake/CheckEngineDecoupling.cmake
#
# Engine include dirs still use ${PROJECT_SOURCE_DIR}/src so #include "engine/..."
# works; this check is the enforcement, not a narrower include path.

if(NOT ENGINE_SRC_DIR)
    message(FATAL_ERROR "CheckEngineDecoupling: ENGINE_SRC_DIR is not set")
endif()

if(NOT EXISTS "${ENGINE_SRC_DIR}")
    message(FATAL_ERROR "CheckEngineDecoupling: ENGINE_SRC_DIR does not exist: ${ENGINE_SRC_DIR}")
endif()

file(GLOB_RECURSE _baja_engine_files
    "${ENGINE_SRC_DIR}/*.cpp"
    "${ENGINE_SRC_DIR}/*.cxx"
    "${ENGINE_SRC_DIR}/*.cc"
    "${ENGINE_SRC_DIR}/*.hpp"
    "${ENGINE_SRC_DIR}/*.hh"
    "${ENGINE_SRC_DIR}/*.h"
)

set(_baja_engine_violations "")
foreach(_baja_file IN LISTS _baja_engine_files)
    file(STRINGS "${_baja_file}" _baja_hits REGEX "^[ \t]*#[ \t]*include[ \t]*[<\"]game/")
    if(_baja_hits)
        file(RELATIVE_PATH _baja_rel "${ENGINE_SRC_DIR}" "${_baja_file}")
        list(APPEND _baja_engine_violations "${_baja_rel}")
    endif()
endforeach()

if(_baja_engine_violations)
    list(JOIN _baja_engine_violations "\n  " _baja_msg)
    message(FATAL_ERROR
        "src/engine must not include headers from src/game.\n"
        "Offending files:\n  ${_baja_msg}")
endif()
