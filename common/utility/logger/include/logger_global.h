#pragma once

// =============================================================================
// logger_global.h — DLL 导出/导入宏
//
// 编译 logger SHARED 库时，CMakeLists.txt 定义 LOGGER_LIBRARY，
// LOGGER_EXPORT 展开为 __declspec(dllexport)；
// 使用方未定义该宏时，LOGGER_EXPORT 展开为 __declspec(dllimport)。
// =============================================================================

#if defined(_WIN32) || defined(__CYGWIN__)
#  if defined(LOGGER_LIBRARY)
#    define LOGGER_EXPORT __declspec(dllexport)
#  else
#    define LOGGER_EXPORT __declspec(dllimport)
#  endif
#else
#  if defined(LOGGER_LIBRARY)
#    define LOGGER_EXPORT __attribute__((visibility("default")))
#  else
#    define LOGGER_EXPORT
#  endif
#endif
