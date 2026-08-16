# MotionControl 插件 — 对外公开头文件

本目录为 **MotionControl** 动态库对外 API，供其他插件或独立进程链接引用。

## 头文件清单

| 文件 | 说明 |
| --- | --- |
| `motion_control_plugin_global.h` | 导出宏 `MOTION_CONTROL_PLUGIN_EXPORT` |
| `motion_control_types.h` | `AxisRuntimeState`、`IoRuntimeState`、分组 Map 类型 |
| `imotion_control_service.h` | 服务接口 `IMotionControlService` |

## 使用方式

```cpp
#include <imotion_control_service.h>
#include "extensionsystem/pluginmanager.h"

auto *svc = ExtensionSystem::PluginManager::getInterface<motion_control_plugin::IMotionControlService>();
if (svc && svc->isReady()) {
    svc->setAutoRefreshEnabled(true);
    connect(svc, &motion_control_plugin::IMotionControlService::axisStatesUpdated, ...);
    svc->startJog(QStringLiteral("X轴"), 1, 0.1);
}
```

## 链接

- 链接 `MotionControl` 导入库（与 Camera 插件相同模式）
- 编译时 **不要** 定义 `MOTION_CONTROL_PLUGIN_LIBRARY`
- `target_include_directories` 指向本目录

## 内部实现

实现代码位于 `plugins/motion_control/service/`，不对外暴露。
