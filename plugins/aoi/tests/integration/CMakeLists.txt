# ============================================================
# aoi/tests/integration — AOI 插件集成测试
#
# 测试目标：
#   aoi_integration_tests — 覆盖配方完整工作流（创建/保存/加载/删除/导入/导出）
#
# 依赖：
#   Aoi              — AOI 插件共享库
#   GTest            — Google Test 单元测试框架
#   Qt               — Qt Core / Widgets（QTemporaryDir 等）
#
# 运行方式：
#   方式 1（直接）：./bin/<Config>/aoi_integration_tests[.exe]
#   方式 2（CTest）：cd build && ctest -C <Config> --output-on-failure -R aoi_integration
# ============================================================

# ── aoi_integration_tests 可执行文件 ──────────────────────────────────────────────────
file(GLOB INTEGRATION_TEST_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

add_executable(aoi_integration_tests
    ${INTEGRATION_TEST_SOURCES}
)

target_link_libraries(aoi_integration_tests PRIVATE
    Aoi
    GTest::gtest
    GTest::gtest_main
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
)

target_include_directories(aoi_integration_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../ui
    ${CMAKE_CURRENT_SOURCE_DIR}/../../ui/view
    ${CMAKE_CURRENT_SOURCE_DIR}/../../ui/vm
    ${CMAKE_CURRENT_SOURCE_DIR}/../../data
    ${CMAKE_CURRENT_SOURCE_DIR}/../../data/dataModel
    ${CMAKE_CURRENT_SOURCE_DIR}/../../data/dataModel/base
    ${CMAKE_CURRENT_SOURCE_DIR}/../../service
    ${CMAKE_CURRENT_SOURCE_DIR}/../../service/inspection
    ${CMAKE_CURRENT_SOURCE_DIR}/../../repository
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/common
    ${PROJECT_SOURCE_DIR}/common/core
    ${PROJECT_SOURCE_DIR}/plugins/camera/include
    ${PROJECT_SOURCE_DIR}/plugins/motion_control/include
    ${PROJECT_SOURCE_DIR}/plugins/vision_algo/include
    ${PROJECT_SOURCE_DIR}/common/core/config/include
)

target_compile_features(aoi_integration_tests PRIVATE cxx_std_17)

fz2_target_msvc_options(aoi_integration_tests)

if(MSVC)
    target_compile_options(aoi_integration_tests PRIVATE /wd4251)
endif()

# 输出到与主程序相同的 bin/<Config>/ 目录
set_target_properties(aoi_integration_tests PROPERTIES
    FOLDER "tests"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG          "${CMAKE_BINARY_DIR}/bin/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE        "${CMAKE_BINARY_DIR}/bin/Release"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/bin/RelWithDebInfo"
)

# 注册 CTest 测试用例
add_test(
    NAME    AoiIntegrationTest
    COMMAND aoi_integration_tests
)

message(STATUS "  [aoi/tests/integration] aoi_integration_tests configured.")