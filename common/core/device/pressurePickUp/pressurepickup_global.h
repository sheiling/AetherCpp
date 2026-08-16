#ifndef PRESSUREPICKUP_GLOBAL_H
#define PRESSUREPICKUP_GLOBAL_H

#if defined(_WIN32) || defined(_WIN64)
  #if defined(PRESSUREPICKUP_LIBRARY)
    #define PRESSUREPICKUP_EXPORT __declspec(dllexport)
  #else
    #define PRESSUREPICKUP_EXPORT __declspec(dllimport)
  #endif
#else
  #define PRESSUREPICKUP_EXPORT __attribute__((visibility("default")))
#endif

#endif // PRESSUREPICKUP_GLOBAL_H
