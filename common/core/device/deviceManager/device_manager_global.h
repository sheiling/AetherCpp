#pragma once

// device_manager_global.h — DeviceManager 模块 DLL 导出/导入宏
// 规则：构建 DeviceManager SHARED 库时由 CMake 定义 DEVICE_MANAGER_LIBRARY。
//  - 定义了 DEVICE_MANAGER_LIBRARY：__declspec(dllexport)
//  - 未定义：__declspec(dllimport)
#if defined(_WIN32) || defined(__CYGWIN__)
#  ifdef DEVICE_MANAGER_LIBRARY
#    define DEVICE_MANAGER_EXPORT __declspec(dllexport)
#  else
#    define DEVICE_MANAGER_EXPORT __declspec(dllimport)
#  endif
#else
#  ifdef DEVICE_MANAGER_LIBRARY
#    define DEVICE_MANAGER_EXPORT __attribute__((visibility("default")))
#  else
#    define DEVICE_MANAGER_EXPORT
#  endif
#endif

