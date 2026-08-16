#ifndef COMMON_CORE_DEVICE_CAMERA_MV_CAMERA_GLOBAL_H
#define COMMON_CORE_DEVICE_CAMERA_MV_CAMERA_GLOBAL_H

#if defined(_WIN32) || defined(__CYGWIN__)
    #ifdef MV_CAMERA_LIBRARY
        #define MV_CAMERA_EXPORT __declspec(dllexport)
    #else
        #define MV_CAMERA_EXPORT __declspec(dllimport)
    #endif
#else
    #define MV_CAMERA_EXPORT
#endif

#endif // COMMON_CORE_DEVICE_CAMERA_MV_CAMERA_GLOBAL_H

