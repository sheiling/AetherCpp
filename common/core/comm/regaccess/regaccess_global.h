#pragma once

// =============================================================================
// regaccess_global.h — 寄存器访问模块 DLL 导出/导入宏
//
// 说明：
//   - 为 keyence/melsec/modbus 三个动态库以及公共接口 IRegAccess 提供统一导出宏
//   - 在对应 CMakeLists.txt 中通过定义以下宏来控制导出/导入：
//       REGACCESS_KEYENCE_LIBRARY
//       REGACCESS_MELSEC_LIBRARY
//       REGACCESS_MODBUS_LIBRARY
// =============================================================================

#if defined(_WIN32) || defined(__CYGWIN__)
#  if defined(REGACCESS_KEYENCE_LIBRARY) \
   || defined(REGACCESS_MELSEC_LIBRARY)  \
   || defined(REGACCESS_MODBUS_LIBRARY)
#    define REGACCESS_EXPORT __declspec(dllexport)
#  else
#    define REGACCESS_EXPORT __declspec(dllimport)
#  endif
#else
#  if defined(REGACCESS_KEYENCE_LIBRARY) \
   || defined(REGACCESS_MELSEC_LIBRARY)  \
   || defined(REGACCESS_MODBUS_LIBRARY)
#    define REGACCESS_EXPORT __attribute__((visibility("default")))
#  else
#    define REGACCESS_EXPORT
#  endif
#endif

