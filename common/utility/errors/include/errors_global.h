#ifndef COMMON_UTILITY_ERRORS_GLOBAL_H
#define COMMON_UTILITY_ERRORS_GLOBAL_H

#if defined(_WIN32) || defined(_WIN64)
#  ifdef ERRORS_LIBRARY
#    define ERRORS_API __declspec(dllexport)
#  else
#    define ERRORS_API __declspec(dllimport)
#  endif
#else
#  define ERRORS_API
#endif

#endif // COMMON_UTILITY_ERRORS_GLOBAL_H
