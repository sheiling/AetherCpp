#pragma once

/**
 * @file camera_plugin_global.h
 * @brief Camera 插件 DLL 导出宏（CAMERA_PLUGIN_LIBRARY 编译时 export，否则 import）
 */

#include <QtCore/qglobal.h>

#if defined(CAMERA_PLUGIN_LIBRARY)
#  define CAMERA_PLUGIN_EXPORT Q_DECL_EXPORT
#else
#  define CAMERA_PLUGIN_EXPORT Q_DECL_IMPORT
#endif
