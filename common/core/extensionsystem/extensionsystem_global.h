#pragma once

#include <QtCore/qglobal.h>

// ============================================================
// DLL 导出/导入宏
// extensionsystem 必须编译为 SHARED 库，确保各 DLL 之间共享
// 同一个 PluginManager 单例
// ============================================================
#if defined(EXTENSIONSYSTEM_LIBRARY)
#  define EXTENSIONSYSTEM_EXPORT Q_DECL_EXPORT
#else
#  define EXTENSIONSYSTEM_EXPORT Q_DECL_IMPORT
#endif

// ============================================================
// 插件 IID 常量
// 所有基于本框架的插件必须在 Q_PLUGIN_METADATA 宏中使用此 IID
// ============================================================
#define EXTENSIONSYSTEM_IID "com.example.pluginsystem.IPlugin"
