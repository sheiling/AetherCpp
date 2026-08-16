#pragma once

#if defined(_WIN32) || defined(__CYGWIN__)
#  ifdef IMGPROC_LIBRARY
#    define IMGPROC_EXPORT __declspec(dllexport)
#  else
#    define IMGPROC_EXPORT __declspec(dllimport)
#  endif
#else
#  define IMGPROC_EXPORT
#endif
