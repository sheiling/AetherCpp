# AOI Plugin Documentation

> **Version:** 2.0.0 · **Last Updated:** 2026-07-24

---

## Overview

The AOI (Automated Optical Inspection) plugin provides the complete inspection UI for the Aether Platform. It implements a full inspection workflow including recipe management, real-time inspection, defect review, and historical analysis — all within a clean MVVM architecture.

### Key Features

| Feature | Description |
|---------|-------------|
| 🏠 **Home Dashboard** | Real-time device status, process flow control, virtual map |
| 📋 **Recipe Management** | Create, edit, delete, import/export recipes with JSON |
| 📷 **Real-time Inspection** | Camera preview, image capture, FOV planning, path planning |
| 🔍 **Defect Review** | Defect classification, filtering, map visualization, batch review |
| 📊 **History Analysis** | Inspection history database, KPI metrics, filtering |
| ⚙️ **System Configuration** | Camera, algorithm, motion, language, theme settings |

---

## Architecture

### MVVM Layer Structure

```
┌─────────────────────────────────────────────────────────────────┐
│  View Layer (ui/view/)                                          │
│  HomeView · RecipeConfigView · DefectReviewView                 │
│  InspectionHistoryView · SystemConfigView                       │
│  Passive widgets — only bind and display, no business logic     │
├─────────────────────────────────────────────────────────────────┤
│  ViewModel Layer (ui/vm/)                                       │
│  HomeViewModel · RecipeConfigViewModel · DefectReviewVM         │
│  InspectionHistoryViewModel · SystemConfigViewModel             │
│  Owns state, business logic, data binding                       │
├─────────────────────────────────────────────────────────────────┤
│  Service Layer (service/)                                       │
│  InspectionDetectService · FovPlanService · CaptureService      │
│  PathPlanner · MarkerService · VisionBridge                     │
│  Domain logic, algorithm integration, hardware coordination     │
├─────────────────────────────────────────────────────────────────┤
│  Data Layer (data/ + repository/)                               │
│  RecipeDocument · RecipeRepository · InspectionHistoryRepo      │
│  Data models, persistence, file I/O                             │
└─────────────────────────────────────────────────────────────────┘
```

### Module Dependency

```
AoiPlugin
├── ui/              — MVVM views and ViewModels
│   ├── view/        — QWidget subclasses (passive)
│   ├── vm/          — ViewModel subclasses (state + logic)
│   ├── binding/     — Data binding and FSM bindings
│   ├── widgets/     — Custom widgets (diagram, domain, list)
│   └── image/       — Image display utilities
│
├── service/         — Business logic services
│   ├── inspection/  — Detection, session, results, history
│   ├── vision_bridge/ — Vision algorithm integration
│   ├── fov_planner/ — FOV (Field of View) planning
│   ├── capture/     — Image capture management
│   ├── path_planner/ — Motion path planning
│   ├── marker/      — Mark point detection
│   ├── flow/        — Scan flow lifecycle
│   ├── affineTransform/ — 2D affine transformation
│   └── param_change/ — Parameter change auditing
│
├── data/            — Data models and definitions
│   ├── dataModel/   — RecipeDocument, system config models
│   ├── dataDefinition/ — Inspection type definitions
│   └── orm/         — Database ORM (optional)
│
├── repository/      — Data persistence
│   ├── inspection_history/ — History storage
│   ├── param_change/ — Parameter change tracking
│   └── orm/         — Database schema initialization
│
├── flow_host/       — Flow lifecycle coordination
├── step/            — Inspection step definitions
├── i18n/            — Translation files (zh_CN, en_US)
├── config/          — Plugin configuration files
└── docs/            — Documentation
```

---

## Plugin Lifecycle

### Initialization Phases

```
loadPlugins()
│
├── [1] initialize()  — 注册页面、主题、翻译
│   ├── 注册 5 个顶级页面到 IMainWindowService
│   ├── 系统配置以二级导航容器注册（3 个子页）
│   ├── 注册 AOI 主题钩子和全局主题变量
│   └── 注册插件翻译文件到 LanguageManager
│
├── [2] extensionsInitialized() — 绑定流程宿主
│   ├── 获取 ISchedulerService 接口
│   └── 将 flowHostCoordinator 绑定到调度服务
│
└── [3] delayedInitialize() — 预热引擎 + 检查 VisionAlgo
    ├── 检查 VisionAlgo 是否就绪（未就绪则弹窗警告）
    └── 预热 InspectionDetectService 检测引擎
```

### Page Registration

| Page | Navigation Key | Order | Description |
|------|---------------|-------|-------------|
| 🏠 Home | `aoi.nav.home` | 20 | Dashboard, device status, process flow |
| ⚙️ System Config | `aoi.nav.system` | 21 | 3 sub-pages: switch/general/threshold |
| 📋 Recipe Config | `aoi.nav.recipe` | 22 | Recipe management and parameter setup |
| 📊 Inspection Results | `aoi.nav.results` | 23 | History query and KPI analysis |
| 🔍 Defect Review | `aoi.nav.review` | 24 | Defect classification and review workflow |

---

## Directory Structure

```
plugins/aoi/
├── aoiplugin.h/cpp              # Plugin entry point & lifecycle
├── CMakeLists.txt               # Build configuration
├── aoiplugin.json               # Plugin metadata (Name, Version, Dependencies)
├── aoi_plugin_name.h            # Plugin name constants
│
├── data/                        # Data models & definitions
│   ├── data.h/cpp               # Data module initialization
│   ├── dataDefinition/          # Inspection type definitions
│   │   ├── inspection_types.h/cpp
│   │   └── param_define.h
│   ├── dataModel/               # Recipe & system config models
│   │   ├── recipe_config_model.h/cpp
│   │   ├── system_config_model.h/cpp
│   │   └── base/               # X-macro base utilities
│   │       ├── param_meta.h/cpp
│   │       ├── param_model_macros.h
│   │       └── param_serde.h/cpp
│   └── orm/                    # ORM registration
│       ├── register.cpp
│       └── entities/RevisionHistory.h
│
├── service/                     # Business logic services
│   ├── inspection/             # Core inspection
│   │   ├── session/           # Inspection session management
│   │   ├── detect/            # Detection service
│   │   ├── result/            # Result management
│   │   ├── history/           # History coordination
│   │   └── virtual_inspect/   # Virtual inspection runner
│   ├── vision_bridge/         # Vision algorithm integration
│   │   ├── chip_detect_bridge.cpp/h
│   │   ├── mark_point_locator.cpp/h
│   │   ├── mark_template_kind.cpp/h
│   │   └── model_generator.cpp/h
│   ├── fov_planner/           # FOV planning
│   │   ├── fov_plan_service.cpp/h
│   │   ├── fov_geometry_planner.cpp/h
│   │   ├── fov_grid_planner.cpp/h
│   │   ├── fov_chip_index_map.cpp/h
│   │   ├── fov_coordinate_system.cpp/h
│   │   ├── fov_layout_types.cpp/h
│   │   └── recipe_fov_layout_adapter.cpp/h
│   ├── capture/               # Capture management
│   │   ├── capture_device_access.cpp/h
│   │   ├── capture_motion_gateway.cpp/h
│   │   ├── capture_profile.cpp/h
│   │   └── fov_capture_executor.cpp/h
│   ├── path_planner/          # Motion path planning
│   │   ├── capture_path_service.cpp/h
│   │   ├── capture_path_planner.cpp/h
│   │   ├── capture_motion_binding.cpp/h
│   │   ├── capture_path_types.h
│   │   └── fov_visit_order.cpp/h
│   ├── marker/                # Mark point detection
│   │   ├── inspection_marker_service.cpp/h
│   │   └── inspection_marker_types.h
│   ├── flow/                  # Scan flow lifecycle
│   │   ├── scan_flow_lifecycle.cpp/h
│   │   ├── scan_flow_pause.cpp/h
│   │   ├── scan_flow_context.h
│   │   └── scan_flow_progress_bridge.cpp/h
│   ├── affineTransform/       # 2D affine transformation
│   │   └── affine_transform_2d.cpp/h
│   └── param_change/          # Parameter change audit
│       ├── param_change_audit_manager.cpp/h
│       ├── param_change_parser.cpp/h
│       └── param_change_store.h
│
├── repository/                  # Data persistence
│   ├── inspection_history/     # History storage
│   │   ├── inspection_history_repository.cpp/h
│   │   └── inspection_history_types.h
│   ├── param_change/           # Parameter change storage
│   │   ├── param_change_db_store.cpp/h
│   │   └── param_change_file_store.cpp/h
│   ├── orm/                    # Database schema
│   │   ├── schema_initializer.cpp/h
│   │   └── sql_paths.cpp/h
│   └── aoi_db_paths.h
│
├── flow_host/                   # Flow coordination
│   ├── flow_host_coordinator.cpp/h
│   └── flow_lifecycle_handler.h
│
├── step/                        # Inspection steps
│   ├── aoi_inspection_steps.cpp/h
│   ├── machine_motion_steps.cpp/h
│   └── inspection_step_utils.h
│
├── ui/                          # User interface
│   ├── view/                   # View components
│   │   ├── home/              # Home dashboard
│   │   ├── recipe_config/     # Recipe configuration (6 sub-views)
│   │   ├── review/            # Defect review (8 sub-views)
│   │   ├── inspection_history/ # History view
│   │   ├── system_config/     # System config (3 sub-views)
│   │   └── mark_template/     # Mark template dialogs
│   │
│   ├── vm/                    # ViewModels
│   │   ├── home/              # Home page VMs
│   │   ├── recipe_config/     # Recipe config VMs (4 VMs)
│   │   ├── review/            # Defect review VMs (4 files)
│   │   └── inspection_history/ # History VM
│   │
│   ├── binding/               # Data binding
│   │   ├── fsm/               # State machine bindings
│   │   ├── home_runtime_binding.cpp/h
│   │   ├── aoi_param_binding.h
│   │   ├── command_button_binding.h
│   │   └── role_image_preview_binding.cpp/h
│   │
│   ├── widgets/               # Custom widgets
│   │   ├── domain/            # Domain-specific widgets
│   │   ├── list/              # List widgets
│   │   ├── diagram/           # Diagram widgets
│   │   └── theme/             # Theme bootstrap
│   │
│   └── image/                 # Image display
│       └── image_info_display.cpp/h
│
├── tests/                       # Tests
│   ├── CMakeLists.txt          # Unit test configuration
│   ├── test_aoi_plugin.cpp     # Unit tests (placeholder)
│   └── integration/            # Integration tests
│       ├── CMakeLists.txt
│       └── test_recipe_workflow.cpp
│
├── i18n/                        # Translations
│   ├── aoi_zh_CN.ts
│   └── aoi_en_US.ts
│
├── config/                      # Configuration files
│   ├── app.toml
│   ├── device_config.json
│   └── flows/                  # Flow definitions (JSON)
│
├── docs/                        # Documentation
│   ├── README.md               # This file
│   ├── qt6_compatibility_audit.md
│   ├── code_quality_guidelines.md
│   ├── recipe_model_architecture.md
│   ├── AOI 检测软件 设计方案.md
│   ├── AOI 检测软件 软件操作SOP.md
│   ├── guides/                 # Maintenance guides
│   ├── knowledge-base/         # Knowledge base
│   └── doc-index/              # Document indexing
│
├── scripts/                     # Deployment scripts
│   ├── deploy.bat
│   └── deploy-remote.ps1
│
└── CHANGELOG.md                 # Change history
```

---

## Build Instructions

### Prerequisites

| Tool | Version | Notes |
|------|---------|-------|
| Qt | 6.11.1 | msvc2022_64 |
| CMake | 3.16+ | Build system |
| MSVC | 2022 (17.14+) | Windows compiler |
| Google Test | Latest | For unit/integration tests |

### Build Commands

```bash
# Configure
cmake -B build -G "Visual Studio 17 2022" -A x64 \
    -DCMAKE_PREFIX_PATH="D:/ProgramInstall/Qt/6.11.1/msvc2022_64"

# Build plugin only
cmake --build build --target Aoi --config Release

# Build with tests
cmake --build build --target Aoi aoi_plugin_tests --config Release
```

### Run Tests

```bash
# Run all tests
cd build
ctest -C Release --output-on-failure

# Run specific test
ctest -C Release --output-on-failure -R AoiPluginTest

# Verbose output
ctest -C Release --output-on-failure --verbose
```

---

## Testing

### Test Suites

| Test Target | Type | Location | Description |
|-------------|------|----------|-------------|
| `aoi_plugin_tests` | Unit | `tests/` | Plugin metadata, models, repositories |
| `aoi_integration_tests` | Integration | `tests/integration/` | Full recipe workflow (CRUD, import/export) |

### Test Coverage

| Layer | Target Coverage | Status |
|-------|----------------|--------|
| Data Models | 80%+ | 🟡 Placeholder |
| Repository | 80%+ | 🟡 Placeholder |
| Services | 70%+ | 🔴 Not started |
| ViewModels | 60%+ | 🔴 Not started |

---

## Documentation Index

### Architecture & Design
- [Design Specification](AOI%20检测软件%20设计方案.md) — 设计方案真相源（中文）
- [Recipe Model Architecture](recipe_model_architecture.md) — 配方数据模型详解
- [Code Quality Guidelines](code_quality_guidelines.md) — C++/Qt6 编码规范

### Compatibility
- [Qt6 Compatibility Audit](qt6_compatibility_audit.md) — Qt6 兼容性审计报告

### Operations
- [Software SOP](AOI%20检测软件%20软件操作SOP.md) — 现场操作手册（给操作员/FAE）
- [SOP Description](AOI%20检测软件%20SOP描述.txt) — SOP 描述驱动源

### Guides
- [Maintenance Guide](guides/文档维护部署指南.md) — 文档维护与部署
- [Knowledge Base Usage](guides/知识库使用指南.md) — 知识库使用说明
- [Agent Prompt Examples](guides/Agent提问示例.md) — Agent 提问示例

### Document Indexing
- [Module ↔ Section Mapping](doc-index/sop-modules.md)
- [Code Path ↔ Design Section](doc-index/design-sections.md)

---

## Development Workflow

1. **Make changes** to the source code
2. **Build** the plugin: `cmake --build build --target Aoi --config Release`
3. **Run tests** to verify no regressions
4. **Test in application** to verify functionality
5. **Commit** changes with descriptive messages

### Commit Convention

```
<type>(<scope>): <description>

feat(aoi): Add recipe export functionality
fix(aoi): Fix crash when loading empty recipe list
docs(aoi): Add recipe model architecture documentation
test(aoi): Add unit tests for RecipeRepository
refactor(aoi): Simplify FOV planner coordinate system
```

---

## Change Log

### 2.0.0 (2026-07-24)
- ✅ Qt6 compatibility audit completed
- ✅ Unit test framework setup (Google Test)
- ✅ Code quality guidelines created
- ✅ Recipe model architecture documented
- ✅ Integration test framework for recipe workflow

### 1.0.0
- Initial release with full MVVM architecture
- 5 top-level pages: Home, Recipe, History, Review, System Config
- Flow host integration with scheduler service
- Vision algorithm bridge integration