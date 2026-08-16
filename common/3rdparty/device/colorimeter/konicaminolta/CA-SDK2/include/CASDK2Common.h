//
//  CASDK2Common.h
//
//  Copyright (c) 2016 KONICA MINOLTA, INC.
//
//
#pragma once

#ifndef CASDK2_API
#  ifdef _WIN32
#     if defined(CASDK2_API_BUILD_SHARED) /* build dll */
#         define CASDK2_API __declspec(dllexport)
#     elif !defined(CASDK2_API_BUILD_STATIC) /* use dll */
#         define CASDK2_API //__declspec(dllimport)
#     else /* static library */
#         define CASDK2_API
#     endif
#  else
#     if __GNUC__ >= 4
#         define CASDK2_API __attribute__((visibility("default")))
#     else
#         define CASDK2_API
#     endif
#  endif
#endif