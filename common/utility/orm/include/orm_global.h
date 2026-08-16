/**
 * @file    orm_global.h
 * @brief   ORM 模块 DLL 导出/导入宏定义
 *
 * 用法：
 *   编译 orm 库本身时  → 定义 ORM_LIBRARY（CMake 中设置）→ __declspec(dllexport)
 *   使用方（链接 orm）  → 不定义 ORM_LIBRARY               → __declspec(dllimport)
 */
#pragma once

#if defined(_WIN32) || defined(_WIN64)
#  ifdef ORM_LIBRARY
#    define ORM_API __declspec(dllexport)
#  else
#    define ORM_API __declspec(dllimport)
#  endif
#else
#  define ORM_API __attribute__((visibility("default")))
#endif
