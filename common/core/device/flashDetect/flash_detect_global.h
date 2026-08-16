#pragma once

#if defined(_WIN32) || defined(_WIN64)
  #ifdef FLASH_DETECT_LIBRARY
    #define FLASH_DETECT_EXPORT __declspec(dllexport)
  #else
    #define FLASH_DETECT_EXPORT __declspec(dllimport)
  #endif
#else
  #if __GNUC__ >= 4
    #define FLASH_DETECT_EXPORT __attribute__((visibility("default")))
  #else
    #define FLASH_DETECT_EXPORT
  #endif
#endif
