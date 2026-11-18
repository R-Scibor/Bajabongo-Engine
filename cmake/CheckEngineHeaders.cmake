# Build one translation unit per header under src/engine that includes only
# that header (as "engine/..."). If a header is not self-contained, this
# target fails to compile. Intended to be included from tests/CMakeLists.txt.

set(BAJA_HEADER_CHECK_DIR "${CMAKE_CURRENT_BINARY_DIR}/header_check")
file(MAKE_DIRECTORY "${BAJA_HEADER_CHECK_DIR}")

file(GLOB_RECURSE BAJA_ENGINE_HEADERS CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/src/engine/*.hpp"
    "${PROJECT_SOURCE_DIR}/src/engine/*.h"
)

set(BAJA_HEADER_CHECK_SOURCES)
foreach(header IN LISTS BAJA_ENGINE_HEADERS)
    file(RELATIVE_PATH rel "${PROJECT_SOURCE_DIR}/src/engine" "${header}")
    string(REPLACE "\\" "/" rel "${rel}")
    string(REPLACE "/" "_" safe "${rel}")
    string(REPLACE "." "_" safe "${safe}")
    set(tu "${BAJA_HEADER_CHECK_DIR}/${safe}.cpp")
    file(WRITE "${tu}" "#include \"engine/${rel}\"\n")
    list(APPEND BAJA_HEADER_CHECK_SOURCES "${tu}")
endforeach()

list(LENGTH BAJA_HEADER_CHECK_SOURCES _baja_header_count)
if(_baja_header_count EQUAL 0)
    message(FATAL_ERROR "CheckEngineHeaders: no headers found under src/engine")
endif()

add_library(baja_header_check OBJECT ${BAJA_HEADER_CHECK_SOURCES})
bajabongo_apply_compiler_options(baja_header_check)
target_link_libraries(baja_header_check
    PRIVATE
        bajabongo::engine
)
