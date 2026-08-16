#pragma once

#if defined(_WIN32) || defined(__CYGWIN__)
#  if defined(LAMPHOUSE_LIBRARY)
#    define LAMPHOUSE_EXPORT __declspec(dllexport)
#  else
#    define LAMPHOUSE_EXPORT __declspec(dllimport)
#  endif
#else
#  if defined(LAMPHOUSE_LIBRARY)
#    define LAMPHOUSE_EXPORT __attribute__((visibility("default")))
#  else
#    define LAMPHOUSE_EXPORT
#  endif
#endif
