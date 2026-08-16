# Camera 插件 — 对外公开头文件

本目录为 **Camera** 动态库对外 API，供 AOI 等插件链接引用。

## 架构与职责边界

```
UI（配方预览 / 主页）
  ↑ frameReady(QImage) / roleImageReady(...)
Camera 插件
  ↑ DispCB（预览）    ↑ publishRoleImageFromInfo（工步）
ImageCapture（DeviceManager）
  ↑ 工步直调（raw / debayer，不经 Camera 插件）
Inspection 工步
```

| 链路 | 说明 |
|------|------|
| 预览 / 单帧 | `snapOnce` / `startPreview` → `execDispFuncCB=true` → debayer → `frameReady` |
| 角色上抛 | 工步 `ImageInfo` → `publishRoleImageFromInfo` → 降采样 → `roleImageReady` |
| 工步采图 | **直调** `DeviceManager::ImageCapture`，不经 `ICameraService` |

QImage 仅出现在 Camera 插件与 UI；AOI 后台禁止 QImage。

## 头文件清单

| 文件 | 说明 |
| --- | --- |
| `camera_plugin_global.h` | 导出宏 `CAMERA_PLUGIN_EXPORT` |
| `camera_types.h` | `CameraFrame`、`CameraImageKind`、`FrameHandler` |
| `icamera_service.h` | 采图服务接口 `ICameraService` |

## 使用示例

```cpp
#include <icamera_service.h>
#include "extensionsystem/pluginmanager.h"

auto *cam = ExtensionSystem::PluginManager::getInterface<ICameraService>();
cam->startPreview(QStringLiteral("AOI检测相机"));
connect(cam, &ICameraService::frameReady, this, [](QImage img) { /* 显示 */ });
```

工步角色预览：

```cpp
cam->publishRoleImageFromInfo(captureName, QStringLiteral("Fov"), processImage);
```

## 链接

- 链接 `Camera` 导入库
- 编译时 **不要** 定义 `CAMERA_PLUGIN_LIBRARY`
- `target_include_directories` 指向本目录

## 内部实现

| 路径 | 说明 |
| --- | --- |
| `service/camera_service_impl.*` | 接口实现、预览会话、DispCB |
| `service/capture_manager.*` | DeviceManager 采集器枚举 |
| `service/frame_dispatcher.*` | 帧合并、ImageInfo→QImage |
| `service/display_image_utils.*` | 角色图降采样 |
| `cameraplugin.*` | 插件入口 |
