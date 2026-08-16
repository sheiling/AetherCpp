#ifndef COMMON_CORE_DEVICE_CAMERA_TOUP_CAMERA_GLOBAL_H
#define COMMON_CORE_DEVICE_CAMERA_TOUP_CAMERA_GLOBAL_H

#if defined(_WIN32) || defined(__CYGWIN__)
  #ifdef TOUP_CAMERA_LIBRARY
    #define TOUP_CAMERA_EXPORT __declspec(dllexport)
  #else
    #define TOUP_CAMERA_EXPORT __declspec(dllimport)
  #endif
#else
  #define TOUP_CAMERA_EXPORT
#endif

#endif // COMMON_CORE_DEVICE_CAMERA_TOUP_CAMERA_GLOBAL_H
