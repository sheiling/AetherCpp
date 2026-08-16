# ============================================================
# tests — 服务容器单元测试
#
# 测试目标：
#   test_container  — 覆盖 Container / Application /
#                     ServiceProvider / Facade<T> 的全部核心功能
#   test_sample     — 最小 Qt Test 占位，校验测试构建链路
#
# 依赖：
#   container     — 服务容器共享库（common/core/container）
#   Qt::Test      — Qt 单元测试框架
#
# 运行方式：
#   方式 1（直接）：./bin/<Config>/test_container[.exe]
#   方式 2（CTest）：cd build && ctest -C <Config> --output-on-failure
# ============================================================

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Test)

enable_testing()

# ── test_container 可执行文件 ──────────────────────────────────────────────────
add_executable(test_container
    test_container.cpp
)

target_link_libraries(test_container
    PRIVATE
        ${COMMON_TARGET_NAMESPACE}::container                          # IoC 容器共享库（自动继承头文件路径）
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
)

target_compile_features(test_container PUBLIC cxx_std_17)

fz2_target_msvc_options(test_container)

# 输出到与主程序相同的 bin/<Config>/ 目录，确保运行时能找到 container.dll
set_target_properties(test_container PROPERTIES
    FOLDER "tests"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG          "${CMAKE_BINARY_DIR}/bin/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE        "${CMAKE_BINARY_DIR}/bin/Release"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/bin/RelWithDebInfo"
)

# 注册 CTest 测试用例
add_test(
    NAME    ContainerTest
    COMMAND test_container
)

# ── test_sample 可执行文件（仅 Qt Test，无业务库依赖）────────────────────────────
add_executable(test_sample
    test_sample.cpp
)

target_link_libraries(test_sample
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
)

target_compile_features(test_sample PUBLIC cxx_std_17)

fz2_target_msvc_options(test_sample)

set_target_properties(test_sample PROPERTIES
    FOLDER "tests"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG          "${CMAKE_BINARY_DIR}/bin/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE        "${CMAKE_BINARY_DIR}/bin/Release"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/bin/RelWithDebInfo"
)

add_test(
    NAME    SampleTest
    COMMAND test_sample
)

message(STATUS "  [tests] test_container configured.")
message(STATUS "  [tests] test_sample configured.")
