#ifndef COMMON_CORE_DEVICE_CAMERA_VIRTUAL_CAMERA_GLOBAL_H
#define COMMON_CORE_DEVICE_CAMERA_VIRTUAL_CAMERA_GLOBAL_H

#if defined(_WIN32) || defined(__CYGWIN__)
#  ifdef VIRTUAL_CAMERA_LIBRARY
#    define VIRTUAL_CAMERA_EXPORT __declspec(dllexport)
#  else
#    define VIRTUAL_CAMERA_EXPORT __declspec(dllimport)
#  endif
#else
#  define VIRTUAL_CAMERA_EXPORT
#endif

#endif
