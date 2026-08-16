#pragma once

// =============================================================================
// tcp_comm_global.h — TCP 通信模块 DLL 导出/导入宏
//
// 编译 tcp_comm SHARED 库时，CMakeLists.txt 定义 TCP_COMM_LIBRARY，
// TCP_COMM_EXPORT 展开为 __declspec(dllexport)；
// 使用方未定义该宏时，TCP_COMM_EXPORT 展开为 __declspec(dllimport)。
// =============================================================================

#if defined(_WIN32) || defined(__CYGWIN__)
#  if defined(TCP_COMM_LIBRARY)
#    define TCP_COMM_EXPORT __declspec(dllexport)
#  else
#    define TCP_COMM_EXPORT __declspec(dllimport)
#  endif
#else
#  if defined(TCP_COMM_LIBRARY)
#    define TCP_COMM_EXPORT __attribute__((visibility("default")))
#  else
#    define TCP_COMM_EXPORT
#  endif
#endif

