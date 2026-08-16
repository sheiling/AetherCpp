cmake_minimum_required(VERSION 3.16)

fz2_collect_sources(ERRORS_SOURCES ERRORS_HEADERS)

fz2_add_common_library(errors SHARED
    ${ERRORS_SOURCES}
    ${ERRORS_HEADERS}
)
set(_tgt ${FZ2_COMMON_LIBRARY_TARGET})

target_include_directories(${_tgt}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_definitions(${_tgt} PRIVATE ERRORS_LIBRARY)
target_compile_features(${_tgt} PUBLIC cxx_std_17)

fz2_target_msvc_options(${_tgt})
if(NOT MSVC)
    target_compile_options(${_tgt} PRIVATE -Wall -Wextra)
endif()
fz2_setup_target(${_tgt} "common/utility/errors")

fz2_source_group_tree(errors FILES ${ERRORS_SOURCES} ${ERRORS_HEADERS})

if(DEMO_ERRORS_EXAMPLE_BUILD)
    add_subdirectory(example)
endif()

message(STATUS "    [errors] Configured")
