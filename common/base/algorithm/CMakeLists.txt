set(CMAKE_FOLDER common/base/algorithm)

# Legacy subdirectory entry: library build has been unified into common/base/CMakeLists.txt.

if(DEMO_ALGORITHM_EXAMPLE_BUILD)
    add_subdirectory(example)
endif()

message(STATUS "    [algorithm] Example-only entry (library is ${COMMON_LIB_PREFIX}base)")

