#pragma once

#include <QtCore/qglobal.h>

#if defined(UIBASE_LIBRARY)
#  define UIBASE_EXPORT Q_DECL_EXPORT
#else
#  define UIBASE_EXPORT Q_DECL_IMPORT
#endif
