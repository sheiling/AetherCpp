# ============================================================
# orm examples — SQLite 与 MySQL 示例程序
# ============================================================

cmake_minimum_required(VERSION 3.16)

set(QXORM_ROOT "${CMAKE_SOURCE_DIR}/common/3rdparty/QxOrm")

# ──────────────────────────────────────────────────────────────────────────────
# 辅助宏：为示例可执行文件统一配置链接 / 编译选项 / 输出目录
# ──────────────────────────────────────────────────────────────────────────────
macro(orm_example_setup target folder)
    target_include_directories(${target}
        PRIVATE
            ${CMAKE_SOURCE_DIR}/common/utility/orm/include
            ${QXORM_ROOT}/include
    )

    target_compile_features(${target} PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(${target} PRIVATE /utf-8 /W3 /wd4251 /wd4275)
    endif()

    target_link_libraries(${target}
        PRIVATE
            ${COMMON_TARGET_NAMESPACE}::orm
    )

    fz2_setup_target(${target} ${folder})

    # 拷贝 QxOrm 的 SQL 驱动插件（sqldrivers）到输出目录
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:${target}>/sqldrivers"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${QXORM_ROOT}/dep/sqldrivers/msvc2019_64"
            "$<TARGET_FILE_DIR:${target}>/sqldrivers"
        COMMENT "[orm_example] Copying Qt SQL drivers..."
        VERBATIM
    )
endmacro()

# ──────────────────────────────────────────────────────────────────────────────
# SQLite 示例
# ──────────────────────────────────────────────────────────────────────────────
if(DEMO_ORM_SQLITE_EXAMPLE_BUILD)
    add_executable(orm_sqlite_example
        sqlite/main.cpp
        sqlite/models/register.cpp
        sqlite/models/Department.h
        sqlite/models/User.h
    )

    orm_example_setup(orm_sqlite_example "common/utility/orm/example/sqlite")

    message(STATUS "      [orm_sqlite_example] Configured")
endif()

# ──────────────────────────────────────────────────────────────────────────────
# MySQL 示例
# ──────────────────────────────────────────────────────────────────────────────
if(DEMO_ORM_MYSQL_EXAMPLE_BUILD)
    add_executable(orm_mysql_example
        mysql/main.cpp
        mysql/models/register.cpp
        mysql/models/Category.h
        mysql/models/Product.h
    )

    orm_example_setup(orm_mysql_example "common/utility/orm/example/mysql")

    # 额外拷贝 MySQL 客户端 DLL
    add_custom_command(TARGET orm_mysql_example POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${QXORM_ROOT}/dep/mysql/libmysql.dll"
            "${QXORM_ROOT}/dep/mysql/libcrypto-3-x64.dll"
            "${QXORM_ROOT}/dep/mysql/libssl-3-x64.dll"
            "$<TARGET_FILE_DIR:orm_mysql_example>"
        COMMENT "[orm_mysql_example] Copying MySQL client DLLs..."
        VERBATIM
    )

    message(STATUS "      [orm_mysql_example] Configured")
endif()
