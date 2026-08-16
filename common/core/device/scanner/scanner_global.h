#pragma once

#if defined(_WIN32) || defined(__CYGWIN__)
#  if defined(SCANNER_LIBRARY)
#    define SCANNER_EXPORT __declspec(dllexport)
#  else
#    define SCANNER_EXPORT __declspec(dllimport)
#  endif
#else
#  if defined(SCANNER_LIBRARY)
#    define SCANNER_EXPORT __attribute__((visibility("default")))
#  else
#    define SCANNER_EXPORT
#  endif
#endif
