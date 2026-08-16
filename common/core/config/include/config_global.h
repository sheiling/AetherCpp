#pragma once

// =============================================================================
// config_global.h — DLL 导出/导入宏
//
// 编译 config SHARED 库时，CMakeLists.txt 定义 CONFIG_LIBRARY，
// CONFIG_EXPORT 展开为 __declspec(dllexport)；
// 使用方未定义该宏时，CONFIG_EXPORT 展开为 __declspec(dllimport)。
// =============================================================================

#if defined(_WIN32) || defined(__CYGWIN__)
#  if defined(CONFIG_LIBRARY)
#    define CONFIG_EXPORT __declspec(dllexport)
#  else
#    define CONFIG_EXPORT __declspec(dllimport)
#  endif
#else
#  if defined(CONFIG_LIBRARY)
#    define CONFIG_EXPORT __attribute__((visibility("default")))
#  else
#    define CONFIG_EXPORT
#  endif
#endif
