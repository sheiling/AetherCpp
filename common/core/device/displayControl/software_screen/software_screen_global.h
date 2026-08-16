#pragma once

#if defined(_WIN32) || defined(__CYGWIN__)
#  if defined(SOFTWARE_SCREEN_LIBRARY)
#    define SOFTWARE_SCREEN_EXPORT __declspec(dllexport)
#  else
#    define SOFTWARE_SCREEN_EXPORT __declspec(dllimport)
#  endif
#else
#  if defined(SOFTWARE_SCREEN_LIBRARY)
#    define SOFTWARE_SCREEN_EXPORT __attribute__((visibility("default")))
#  else
#    define SOFTWARE_SCREEN_EXPORT
#  endif
#endif
