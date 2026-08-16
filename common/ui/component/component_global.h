#pragma once

#include <QtCore/qglobal.h>

#if defined(UI_COMPONENT_LIBRARY)
#  define UI_COMPONENT_EXPORT Q_DECL_EXPORT
#else
#  define UI_COMPONENT_EXPORT Q_DECL_IMPORT
#endif
