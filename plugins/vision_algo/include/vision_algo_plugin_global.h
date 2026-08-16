#pragma once

#include <QtCore/qglobal.h>

#if defined(VISION_ALGO_PLUGIN_LIBRARY)
#    define VISION_ALGO_EXPORT Q_DECL_EXPORT
#else
#    define VISION_ALGO_EXPORT Q_DECL_IMPORT
#endif
