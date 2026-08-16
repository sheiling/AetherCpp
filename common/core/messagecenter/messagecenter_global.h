#pragma once

#include <QtCore/qglobal.h>

// ============================================================
// DLL 导出/导入宏
// messagecenter 必须编译为 SHARED 库，确保各 DLL 之间共享
// 同一套 MessageCenter 内部状态（Attach/Detach/Notify）
// ============================================================
#if defined(MESSAGECENTER_LIBRARY)
#  define MESSAGECENTER_EXPORT Q_DECL_EXPORT
#else
#  define MESSAGECENTER_EXPORT Q_DECL_IMPORT
#endif
