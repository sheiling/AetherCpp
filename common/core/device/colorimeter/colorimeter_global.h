#pragma once

#if defined(_WIN32) || defined(_WIN64)
  #ifdef COLORIMETER_LIBRARY
    #define COLORIMETER_EXPORT __declspec(dllexport)
  #else
    #define COLORIMETER_EXPORT __declspec(dllimport)
  #endif
#else
  #if __GNUC__ >= 4
    #define COLORIMETER_EXPORT __attribute__((visibility("default")))
  #else
    #define COLORIMETER_EXPORT
  #endif
#endif
