#pragma once

// algorithm_global.h — algorithm 模块 DLL 导出/导入宏
// 规则：构建 algorithm SHARED 库时由 CMake 定义 ALGORITHM_LIBRARY。
//  - 定义了 ALGORITHM_LIBRARY：__declspec(dllexport)
//  - 未定义：__declspec(dllimport)
#if defined(_WIN32) || defined(__CYGWIN__)
#  ifdef ALGORITHM_LIBRARY
#    define ALGORITHM_EXPORT __declspec(dllexport)
#  else
#    define ALGORITHM_EXPORT __declspec(dllimport)
#  endif
#else
#  ifdef ALGORITHM_LIBRARY
#    define ALGORITHM_EXPORT __attribute__((visibility("default")))
#  else
#    define ALGORITHM_EXPORT
#  endif
#endif

