#pragma once

#include <QtCore/qglobal.h>

// ============================================================
// DLL 导出/导入宏
//
// container 必须编译为 SHARED 库，确保跨 DLL 共享同一个
// Application 单例（如需要）。
//
// 编译库本身时定义 CONTAINER_LIBRARY → Q_DECL_EXPORT
// 外部使用者只需包含头文件          → Q_DECL_IMPORT
// ============================================================
#if defined(CONTAINER_LIBRARY)
#  define CONTAINER_EXPORT Q_DECL_EXPORT
#else
#  define CONTAINER_EXPORT Q_DECL_IMPORT
#endif
