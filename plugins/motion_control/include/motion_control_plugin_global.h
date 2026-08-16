#pragma once

#include <QtCore/qglobal.h>

#if defined(MOTION_CONTROL_PLUGIN_LIBRARY)
#  define MOTION_CONTROL_PLUGIN_EXPORT Q_DECL_EXPORT
#else
#  define MOTION_CONTROL_PLUGIN_EXPORT Q_DECL_IMPORT
#endif
