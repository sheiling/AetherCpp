#pragma once

// =============================================================================
// secs_global.h — SECS 通信模块 DLL 导出/导入宏
//
// 编译 secs SHARED 库时，CMakeLists.txt 定义 SECS_LIBRARY，
// SECS_EXPORT 展开为 __declspec(dllexport)；
// 使用方未定义该宏时，SECS_EXPORT 展开为 __declspec(dllimport)。
// =============================================================================

#if defined(_WIN32) || defined(__CYGWIN__)
#  if defined(SECS_LIBRARY)
#    define SECS_EXPORT __declspec(dllexport)
#  else
#    define SECS_EXPORT __declspec(dllimport)
#  endif
#else
#  if defined(SECS_LIBRARY)
#    define SECS_EXPORT __attribute__((visibility("default")))
#  else
#    define SECS_EXPORT
#  endif
#endif

