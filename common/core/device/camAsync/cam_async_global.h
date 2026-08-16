#pragma once

// cam_async_global.h — CamAsync 模块 DLL 导出/导入宏
#if defined(_WIN32) || defined(__CYGWIN__)
#  ifdef CAM_ASYNC_LIBRARY
#    define CAM_ASYNC_EXPORT __declspec(dllexport)
#  else
#    define CAM_ASYNC_EXPORT __declspec(dllimport)
#  endif
#else
#  ifdef CAM_ASYNC_LIBRARY
#    define CAM_ASYNC_EXPORT __attribute__((visibility("default")))
#  else
#    define CAM_ASYNC_EXPORT
#  endif
#endif
