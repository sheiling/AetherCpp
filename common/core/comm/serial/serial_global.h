#pragma once

#if defined(_WIN32) || defined(__CYGWIN__)
#  if defined(SERIAL_COMM_LIBRARY)
#    define SERIAL_COMM_EXPORT __declspec(dllexport)
#  else
#    define SERIAL_COMM_EXPORT __declspec(dllimport)
#  endif
#else
#  if defined(SERIAL_COMM_LIBRARY)
#    define SERIAL_COMM_EXPORT __attribute__((visibility("default")))
#  else
#    define SERIAL_COMM_EXPORT
#  endif
#endif
