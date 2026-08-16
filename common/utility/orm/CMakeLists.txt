# ============================================================
# orm — 基于 QxOrm 的数据库操作封装模块
#
# 提供功能：
#   - DbConfig     — 驱动/主机/端口/账号等连接参数封装（支持 SQLite / MySQL / PostgreSQL）
#   - DbManager    — 连接管理器单例（封装 qx::QxSqlDatabase，试连接验证，事务辅助）
#   - Repository<T>— 泛型 CRUD 模板（insert/update/save/remove/find/count/batchXxx）
#   - OrmBase.h    — 模型注册辅助宏（ORM_ENTITY_HPP/CPP、ORM_REGISTER、ORM_SOFT_DELETE）
#   - Orm.h        — 统一入口头文件
#
# 依赖：
#   - QxOrm（预编译 DLL，位于 common/3rdparty/QxOrm）
#   - Qt::Sql（QSqlDatabase / QSqlError 等）
#   - Qt::Core
#
# 对外目标：
#   - ${COMMON_LIB_PREFIX}orm（SHARED 库）
#   - ${COMMON_TARGET_NAMESPACE}::orm（别名，供下游 target_link_libraries 使用）
# ============================================================

cmake_minimum_required(VERSION 3.16)

# ── 收集源文件 ─────────────────────────────────────────────────────────────────
fz2_collect_sources(ORM_SOURCES ORM_HEADERS)

# ── 创建共享库目标 ─────────────────────────────────────────────────────────────
fz2_add_common_library(orm SHARED
    ${ORM_SOURCES}
    ${ORM_HEADERS}
)
set(_tgt ${FZ2_COMMON_LIBRARY_TARGET})

# ── 头文件目录 ─────────────────────────────────────────────────────────────────
set(QXORM_ROOT "${CMAKE_SOURCE_DIR}/common/3rdparty/QxOrm")

target_include_directories(${_tgt}
    PUBLIC
        # ORM 封装层公共头文件（使用方 #include <Orm.h>）
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        # QxOrm 头文件（PUBLIC，因为 Orm.h 直接 include QxOrm 类型）
        $<BUILD_INTERFACE:${QXORM_ROOT}/include>
)

# ── 编译宏 ─────────────────────────────────────────────────────────────────────
target_compile_definitions(${_tgt}
    PRIVATE
        ORM_LIBRARY          # 导出符号（dllexport）
)

# ── 编译选项 ───────────────────────────────────────────────────────────────────
target_compile_features(${_tgt} PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(${_tgt} PRIVATE /utf-8 /W3 /wd4251 /wd4275)
else()
    target_compile_options(${_tgt} PRIVATE -Wall -Wextra)
endif()

# ── 查找 Qt ────────────────────────────────────────────────────────────────────
if(QT_VERSION_MAJOR EQUAL 6)
    find_package(Qt6 REQUIRED COMPONENTS Core Sql)
    target_link_libraries(${_tgt} PUBLIC Qt6::Core Qt6::Sql)
else()
    find_package(Qt5 5.15 REQUIRED COMPONENTS Core Sql)
    target_link_libraries(${_tgt} PUBLIC Qt5::Core Qt5::Sql)
endif()

# ── 链接 QxOrm 预编译库 ───────────────────────────────────────────────────────
target_link_libraries(${_tgt}
    PUBLIC
        $<$<CONFIG:Debug>:${QXORM_ROOT}/lib/Debug/QxOrmd.lib>
        $<$<CONFIG:Release>:${QXORM_ROOT}/lib/Release/QxOrm.lib>
        $<$<CONFIG:RelWithDebInfo>:${QXORM_ROOT}/lib/RelWithDebInfo/QxOrm.lib>
)

# ── 拷贝 QxOrm DLL 到输出目录 ─────────────────────────────────────────────────
add_custom_command(TARGET ${_tgt} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<$<CONFIG:Debug>:${QXORM_ROOT}/bin/Debug/QxOrmd.dll>"
        "$<$<CONFIG:Release>:${QXORM_ROOT}/bin/Release/QxOrm.dll>"
        "$<$<CONFIG:RelWithDebInfo>:${QXORM_ROOT}/bin/RelWithDebInfo/QxOrm.dll>"
        "$<TARGET_FILE_DIR:${_tgt}>"
    COMMENT "[orm] Copying QxOrm DLL..."
    VERBATIM
)

# ── 输出目录 ───────────────────────────────────────────────────────────────────
fz2_setup_target(${_tgt} "common/utility/orm")

# ── VS 源文件分组 ──────────────────────────────────────────────────────────────
fz2_source_group_tree(orm FILES ${ORM_SOURCES} ${ORM_HEADERS})

# ── 示例程序（可选）──────────────────────────────────────────────────────────
if(DEMO_ORM_EXAMPLE_BUILD)
    message(STATUS "    [orm/example] Configuring...")
    add_subdirectory(example)
endif()

message(STATUS "    [orm] Configured — QxOrm wrapper (SQLite / MySQL / PostgreSQL)")
