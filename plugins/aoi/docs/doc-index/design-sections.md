# 设计方案章节索引

AI 增量更新 `plugins/aoi/docs/AOI 检测软件 设计方案.md` 时，按变更范围定位章节。
SOP 模块 id 见同目录 [sop-modules.md](./sop-modules.md)。

> 代码路径相对于**工作区根目录** `Project_Aether/`。

| 代码路径 / 关键词 | 设计文档章节 | 关联 SOP 模块 id |
| --- | --- | --- |
| `plugins/aoi/aoiplugin.cpp`、页面注册 | 核心流程 §3 · 模块详解 plugins/aoi | sys-config, recipe-config |
| `plugins/aoi/data/dataModel/base/` | 模块详解 plugins/aoi · 接口与配置 AOI 系统配置/配方字段 | sys-config, recipe-config |
| `plugins/aoi/data/dataModel/recipe_config_model.*` | 核心流程 §3 · 接口与配置 RecipeDocument | recipe-config |
| `plugins/aoi/data/dataModel/system_config_model.*` | 接口与配置 AoiSystemConfig | sys-config |
| `plugins/aoi/repository/param_change/` | 模块详解 plugins/aoi · 异常与安全 参数安全 | — |
| `plugins/aoi/service/param_change/` | 模块详解 plugins/aoi · 异常与安全 参数安全 | — |
| `plugins/aoi/algorithm/` | 模块详解 plugins/aoi · 核心流程 §3 | recipe-config |
| `plugins/aoi/ui/vm/recipe_config_view_model.*` | 核心流程 §3、§5 相机采图 | recipe-config |
| `plugins/aoi/ui/vm/system_config_view_model.*` | 模块详解 plugins/aoi | sys-config |
| `plugins/aoi/ui/view/recipe_config/`（薄壳 `recipe_config_view` + `center_view` + `param_panel` + 六 SubView + `recipe_config_widgets.h`） | 模块详解 plugins/aoi · MVVM | recipe-config |
| `plugins/aoi/ui/widgets/domain/panel_overview_painter.*`、`panel_navigator_overlay.*` | 核心流程 §3.2 全图导航 | recipe-config |
| `plugins/aoi/service/fov_planner/`（`recipe_fov_layout_adapter`、`fov_coordinate_system`、`fov_layout_types`） | 核心流程 §3.2 全图导航 · §3.1.1 规划工步 | recipe-config, home |
| `plugins/aoi/service/path_planner/` | 核心流程 §3.1.1 `aoi.plan_geometry` | home |
| `plugins/aoi/service/flow/` | 核心流程 §3.1.1 扫描流程协作（进度桥/生命周期/暂停）· 首页 UI | home |
| `plugins/aoi/service/capture/` | 核心流程 §3.1.1 运动采图（Marker/FOV）· §5 ImageCapture 工步直调 | home |
| `plugins/aoi/flow_host/` | 调度宿主 FlowHostCoordinator（不含 AOI 业务） | home |
| `plugins/aoi/service/marker/`、`service/inspection/` | 核心流程 §3.1.1 扫描工步链 | home |
| `plugins/aoi/ui/view/system_config/`（含 `system_config_view` 底栏保存） | 模块详解 plugins/aoi | sys-config |
| `plugins/aoi/ui/view/home/`、`ThemedSectionPanel`、`ui/widgets/group/` | 核心流程 §3.1 · 模块详解 plugins/aoi · UI 注释约定 | home |
| `plugins/aoi/ui/widgets/image/`、`plugins/aoi/ui/widgets/logView/` | 模块详解 plugins/aoi · ui/widgets 子目录表 | home |
| `plugins/aoi/ui/view/mark_template/`、`plugins/aoi/ui/widgets/`（controls/table/list/dialog/feedback） | 模块详解 plugins/aoi · ui/widgets 子目录表 | sys-config, recipe-config |
| `plugins/aoi/ui/binding/` | 模块详解 MVVM · plugins/aoi ui/binding | sys-config, recipe-config |
| `plugins/camera/include/icamera_service.h`、`camera_types.h` | 核心流程 §5 · ICameraService / CameraFrame | recipe-config |
| `plugins/camera/service/camera_service_impl.*` | 核心流程 §5 · 模块详解 plugins/camera | recipe-config |
| `plugins/camera/service/capture_manager.*` | 核心流程 §5.2 · §5.6 | recipe-config |
| `plugins/camera/service/frame_dispatcher.*` | 核心流程 §5.4 · §5.2 | recipe-config |
| `common/core/device/imageCapture/` | 核心流程 §5.3 · 模块详解 imageCapture | recipe-config |
| `common/core/device/deviceManager/` | 核心流程 §5.2 DeviceManager · §5.8 虚拟相机 | recipe-config, sys-config |
| `common/core/device/camera/virtual_camera/` | 核心流程 §5.8 · 模块详解 camera 驱动 | recipe-config |
| `CMakeLists.txt`、`common/CMakeLists.txt`、`plugins/CMakeLists.txt`、`app/CMakeLists.txt`、`app/cmake/AppOptions.cmake`、`common/cmake/ProjectOptions.cmake`、`plugins/cmake/PluginOptions.cmake` | 接口与配置 · CMake 编译开关归类 | — |
| `common/plugins/core/mainwindow.cpp`、`PermService` | 核心流程 §2 · 异常与安全 权限 | 全书 |
| `common/plugins/permission/` | 核心流程 §7 · 异常与安全 权限 | 全书 |
| `common/ui/uibase/permission/permservice.*`、`PermCatalogScope` | 异常与安全 · 权限资源目录对齐 | 全书 |
| `common/core/extensionsystem/plugin_name_slot.h`、各插件 `*_plugin_name.h` | 异常与安全 · 权限资源目录对齐 | 全书 |
| `common/plugins/permission/service/perm_catalog_registrar.*`、`perm_catalog_reconcile.*`、`IPluginAvailability` | 异常与安全 · 权限资源目录对齐 | 全书 |
| `common/core/scheduler/machine/machine_fsm.*` | 核心流程 §3.1.3 · 模块详解 common/core/scheduler | home |
| `common/core/scheduler/machine/button_monitor.*` | 核心流程 §3.1.3 · 模块详解 common/core/scheduler | home |
| `common/core/scheduler/api/`、`engine/`、`step/` | 模块详解 common/core/scheduler · 核心流程 §3.1.1 | home |
| `plugins/aoi/ui/vm/home_view_model.*` | 核心流程 §3.1 · §3.1.3 UI 映射 | home |
| `plugins/aoi/data/dataDefinition/inspection_types.h` | 核心流程 §3.1.4 · 缺陷/复判数据模型（`DefectInfoItem` 晶粒坐标、`DefectInfoList` 板级规模） | — |
| `plugins/aoi/ui/view/review/`、`ui/view/review/defect_map_canvas.*` | 核心流程 §3.1.4 缺陷复判 Widget | — |
| `plugins/aoi/ui/vm/review/`、`defect_review_view_model.*` | 核心流程 §3.1.4 缺陷复判 VM | — |
| `plugins/aoi/ui/vm/review/defect_review_debug_data.*` | 核心流程 §3.1.4 bridge 联调 `DefectInfoList` 构造 | — |
| `plugins/aoi/ui/vm/review/defect_review_image_crop.*` | 核心流程 §3.1.4 原图/结果图裁切（分区路径 + `roi`） | — |
| `plugins/aoi/service/inspection/`、`saveDefectInfoList` | 核心流程 §3.1.4 复判落盘 · §3.1.5 历史库编解码 | inspection-history |
| `plugins/aoi/repository/inspection_history/` | 核心流程 §3.1.5 `aoi_inspection_history` 仓储 | inspection-history |
| `plugins/aoi/repository/orm/`、`config/sql/sqlite/006_aoi_inspection_history_schema.sql` | 核心流程 §3.1.5 schema 脚本与建表 | inspection-history |
| `plugins/aoi/repository/aoi_db_paths.h` | 核心流程 §3.1.5 `{exe}/data/record.db` | inspection-history |
| `plugins/aoi/ui/view/inspection_history/`、`ui/vm/inspection_history/` | 核心流程 §3.1.5 检测历史页 | inspection-history |
| `plugins/aoi/ui/view/review/defect_review_dialog.*` | 核心流程 §3.1.5 历史详情弹窗 | inspection-history |

## 全局章节（少改）

| 章节 | 何时更新 |
| --- | --- |
| 名词解释 | 新增核心术语 |
| 设计目标 | 目标/非目标变化 |
| 相关联文档 | 新增关联文档路径 |
| 修订记录 | 每次架构/接口增量维护 |
