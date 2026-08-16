#pragma once

#if defined(_WIN32) || defined(__CYGWIN__)
#  if defined(MOTION_CONTROL_LIBRARY)
#    define MOTION_CONTROL_EXPORT __declspec(dllexport)
#  else
#    define MOTION_CONTROL_EXPORT __declspec(dllimport)
#  endif
#else
#  if defined(MOTION_CONTROL_LIBRARY)
#    define MOTION_CONTROL_EXPORT __attribute__((visibility("default")))
#  else
#    define MOTION_CONTROL_EXPORT
#  endif
#endif
