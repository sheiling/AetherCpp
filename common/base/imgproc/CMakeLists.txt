set(CMAKE_FOLDER common/base/imgproc)

# Legacy subdirectory entry: library build has been unified into common/base/CMakeLists.txt.
if(DEMO_IMGPROC_EXAMPLE_BUILD)
    add_subdirectory(example)
endif()

message(STATUS "    [imgproc] Example-only entry (library is ${COMMON_LIB_PREFIX}base)")
