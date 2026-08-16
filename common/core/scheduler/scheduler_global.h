#pragma once

// ============================================================
// scheduler_global.h — DLL 导出/导入宏
// ============================================================

#if defined(_WIN32) || defined(__CYGWIN__)
#  if defined(SCHEDULER_LIBRARY)
#    define SCHEDULER_EXPORT __declspec(dllexport)
#  else
#    define SCHEDULER_EXPORT __declspec(dllimport)
#  endif
#else
#  if defined(SCHEDULER_LIBRARY)
#    define SCHEDULER_EXPORT __attribute__((visibility("default")))
#  else
#    define SCHEDULER_EXPORT
#  endif
#endif
