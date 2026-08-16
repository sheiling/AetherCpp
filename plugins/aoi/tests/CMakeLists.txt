# ============================================================
# aoi/tests — AOI 插件单元测试
#
# 测试目标：
#   aoi_plugin_tests  — 覆盖 AoiPlugin、RecipeDocument、RecipeRepository
#
# 依赖：
#   Aoi              — AOI 插件共享库
#   GTest            — Google Test 单元测试框架
#   Qt               — Qt Core / Widgets（插件头文件依赖）
#
# 运行方式：
#   方式 1（直接）：./bin/<Config>/aoi_plugin_tests[.exe]
#   方式 2（CTest）：cd build && ctest -C <Config> --output-on-failure
# ============================================================

find_package(GTest REQUIRED)

# ── aoi_plugin_tests 可执行文件 ──────────────────────────────────────────────────
file(GLOB TEST_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

add_executable(aoi_plugin_tests
    ${TEST_SOURCES}
)

target_link_libraries(aoi_plugin_tests PRIVATE
    Aoi
    GTest::gtest
    GTest::gtest_main
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
)

target_include_directories(aoi_plugin_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../ui
    ${CMAKE_CURRENT_SOURCE_DIR}/../ui/view
    ${CMAKE_CURRENT_SOURCE_DIR}/../ui/vm
    ${CMAKE_CURRENT_SOURCE_DIR}/../ui/vm/home
    ${CMAKE_CURRENT_SOURCE_DIR}/../ui/vm/recipe_config
    ${CMAKE_CURRENT_SOURCE_DIR}/../ui/vm/review
    ${CMAKE_CURRENT_SOURCE_DIR}/../ui/view/review
    ${CMAKE_CURRENT_SOURCE_DIR}/../ui/binding
    ${CMAKE_CURRENT_SOURCE_DIR}/../ui/binding/fsm
    ${CMAKE_CURRENT_SOURCE_DIR}/../ui/widgets
    ${CMAKE_CURRENT_SOURCE_DIR}/../data
    ${CMAKE_CURRENT_SOURCE_DIR}/../data/dataDefinition
    ${CMAKE_CURRENT_SOURCE_DIR}/../data/dataModel
    ${CMAKE_CURRENT_SOURCE_DIR}/../data/dataModel/base
    ${CMAKE_CURRENT_SOURCE_DIR}/../service
    ${CMAKE_CURRENT_SOURCE_DIR}/../service/inspection
    ${CMAKE_CURRENT_SOURCE_DIR}/../service/inspection/session
    ${CMAKE_CURRENT_SOURCE_DIR}/../service/inspection/detect
    ${CMAKE_CURRENT_SOURCE_DIR}/../service/inspection/result
    ${CMAKE_CURRENT_SOURCE_DIR}/../service/inspection/virtual_inspect
    ${CMAKE_CURRENT_SOURCE_DIR}/../service/inspection/history
    ${CMAKE_CURRENT_SOURCE_DIR}/../service/marker
    ${CMAKE_CURRENT_SOURCE_DIR}/../service/vision_bridge
    ${CMAKE_CURRENT_SOURCE_DIR}/../service/fov_planner
    ${CMAKE_CURRENT_SOURCE_DIR}/../service/flow
    ${CMAKE_CURRENT_SOURCE_DIR}/../service/capture
    ${CMAKE_CURRENT_SOURCE_DIR}/../service/path_planner
    ${CMAKE_CURRENT_SOURCE_DIR}/../service/affineTransform
    ${CMAKE_CURRENT_SOURCE_DIR}/../step
    ${CMAKE_CURRENT_SOURCE_DIR}/../repository/inspection_history
    ${CMAKE_CURRENT_SOURCE_DIR}/../repository/orm
    ${CMAKE_CURRENT_SOURCE_DIR}/../ui/vm/inspection_history
    ${CMAKE_CURRENT_SOURCE_DIR}/../ui/view/inspection_history
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/common
    ${PROJECT_SOURCE_DIR}/common/core
    ${PROJECT_SOURCE_DIR}/common/core/scheduler
    ${PROJECT_SOURCE_DIR}/plugins/camera/include
    ${PROJECT_SOURCE_DIR}/plugins/motion_control/include
    ${PROJECT_SOURCE_DIR}/plugins/vision_algo/include
    ${PROJECT_SOURCE_DIR}/common/core/config/include
    ${PROJECT_SOURCE_DIR}/common/utility/diagnostics
)

target_compile_features(aoi_plugin_tests PRIVATE cxx_std_17)

fz2_target_msvc_options(aoi_plugin_tests)

if(MSVC)
    target_compile_options(aoi_plugin_tests PRIVATE /wd4251)
endif()

# 输出到与主程序相同的 bin/<Config>/ 目录
set_target_properties(aoi_plugin_tests PROPERTIES
    FOLDER "tests"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG          "${CMAKE_BINARY_DIR}/bin/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE        "${CMAKE_BINARY_DIR}/bin/Release"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/bin/RelWithDebInfo"
)

# 注册 CTest 测试用例
add_test(
    NAME    AoiPluginTest
    COMMAND aoi_plugin_tests
)

# ── 集成测试子目录 ──────────────────────────────────────────────────────────────────
if(BUILD_TESTING)
    add_subdirectory(integration)
endif()

message(STATUS "  [aoi/tests] aoi_plugin_tests configured.")