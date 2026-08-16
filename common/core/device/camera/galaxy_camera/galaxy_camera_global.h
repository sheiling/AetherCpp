#ifndef COMMON_CORE_DEVICE_CAMERA_GALAXY_CAMERA_GLOBAL_H
#define COMMON_CORE_DEVICE_CAMERA_GALAXY_CAMERA_GLOBAL_H

#if defined(_WIN32) || defined(__CYGWIN__)
  #ifdef GALAXY_CAMERA_LIBRARY
    #define GALAXY_CAMERA_EXPORT __declspec(dllexport)
  #else
    #define GALAXY_CAMERA_EXPORT __declspec(dllimport)
  #endif
#else
  #define GALAXY_CAMERA_EXPORT
#endif

#endif // COMMON_CORE_DEVICE_CAMERA_GALAXY_CAMERA_GLOBAL_H

