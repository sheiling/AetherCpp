#pragma once

#if defined(_WIN32) || defined(_WIN64)
  #ifdef RANGEFINDER_LIBRARY
    #define RANGEFINDER_EXPORT __declspec(dllexport)
  #else
    #define RANGEFINDER_EXPORT __declspec(dllimport)
  #endif
#else
  #if __GNUC__ >= 4
    #define RANGEFINDER_EXPORT __attribute__((visibility("default")))
  #else
    #define RANGEFINDER_EXPORT
  #endif
#endif
