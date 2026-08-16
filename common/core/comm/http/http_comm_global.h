#pragma once

// =============================================================================
// http_comm_global.h — HTTP 通信模块 DLL 导出/导入宏
//
// 编译 http_comm SHARED 库时，CMakeLists.txt 定义 HTTP_COMM_LIBRARY，
// HTTP_COMM_EXPORT 展开为 __declspec(dllexport)；
// 使用方未定义该宏时，HTTP_COMM_EXPORT 展开为 __declspec(dllimport)。
// =============================================================================

#if defined(_WIN32) || defined(__CYGWIN__)
#  if defined(HTTP_COMM_LIBRARY)
#    define HTTP_COMM_EXPORT __declspec(dllexport)
#  else
#    define HTTP_COMM_EXPORT __declspec(dllimport)
#  endif
#else
#  if defined(HTTP_COMM_LIBRARY)
#    define HTTP_COMM_EXPORT __attribute__((visibility("default")))
#  else
#    define HTTP_COMM_EXPORT
#  endif
#endif

