# AOI Recipe Model Architecture

> **Version:** 1.0.0 · **Last Updated:** 2026-07-24

---

## Overview

The AOI recipe model provides a comprehensive data structure for storing and managing inspection recipes. It uses a macro-based field declaration system (X-macro pattern) that enables automatic JSON serialization, equality comparison, and Qt metatype registration from a single field definition.

---

## Core Components

### RecipeDocument

The central data structure containing all recipe parameters:

```
RecipeDocument
├── id: QString                    — 配方唯一标识
├── basic: RecipeBasicInfo         — 基板/芯片几何尺寸与布局
│   ├── name: QString              — 配方名称
│   ├── width: int                 — 基板宽度 (mm)
│   ├── height: int                — 基板高度 (mm)
│   ├── groupPitchXmm: double      — 组间距 X (mm)
│   ├── groupPitchYmm: double      — 组间距 Y (mm)
│   ├── chipPitchInGroupMm: double — 组内芯片间距 (mm)
│   ├── chipWidthMm: double        — 芯片宽度 (mm)
│   ├── chipHeightMm: double       — 芯片高度 (mm)
│   ├── padWidthMm: double         — 焊盘宽度 (mm)
│   ├── padHeightMm: double        — 焊盘高度 (mm)
│   ├── groupWidthMm: double       — 组宽度 (mm)
│   ├── groupHeightMm: double      — 组高度 (mm)
│   ├── chipColorOrder: int        — 芯片颜色顺序
│   ├── chipArrangement: int       — 芯片排列方式
│   ├── offsetGroupMarkBl: Point   — 左下 Mark 偏移
│   └── offsetGroupMarkTr: Point   — 右上 Mark 偏移
│
├── camera: CameraSettings         — 相机采集参数
│   ├── lightBrightness: int       — 光源亮度
│   ├── exposureUs: int            — 曝光时间 (μs)
│   ├── imageWidth: int            — 图像宽度 (px)
│   ├── imageHeight: int           — 图像高度 (px)
│   └── captureAccuracy: double    — 采集精度
│
├── algo: AlgoThresholds           — 算法检测阈值
│   ├── groupConfidence: double    — 组置信度阈值
│   ├── chipConfidence: double     — 芯片置信度阈值
│   ├── chipDefectClass: double    — 芯片缺陷分类阈值
│   ├── padDefectClass: double     — 焊盘缺陷分类阈值
│   ├── offsetXUm: double          — X 偏移容差 (μm)
│   ├── offsetYUm: double          — Y 偏移容差 (μm)
│   └── angleOffsetDeg: double     — 角度偏移容差 (°)
│
├── marks: MarkPointConfig[]       — Mark 点检测配置列表
│   ├── capturePos: Point3         — 采集坐标 (X, Y, Z)
│   └── markerInfo: MarkerDetectInfo — 视觉检测参数
│       ├── markType: MarkerType   — Mark 类型 (Circle/Cross/etc.)
│       ├── markWidth: int         — Mark 宽度 (px)
│       ├── markHeight: int        — Mark 高度 (px)
│       ├── isWhiteMark: uint16_t  — 是否为白 Mark
│       ├── area: float            — 面积阈值
│       ├── minThresh: float       — 最小阈值
│       ├── maxThresh: float       — 最大阈值
│       ├── modelFileName: string  — 模板文件名
│       └── modelName: string      — 模板名称
│
├── panelCalib: PanelCalibSettings — 面板标定设置
│   ├── mode: CalibMode            — 标定模式 (Panel/Strip)
│   ├── fovInfo: QString           — FOV 信息
│   ├── pathModeIndex: int         — 路径模式索引
│   └── gerberFileName: QString    — Gerber 文件名
│
└── positionCalib: PositionCalibSettings — 位置标定参数
    ├── trackPosition: double      — 轨道位置
    └── scanPosition: Point3       — 扫描坐标 (X, Y, Z)
```

### Data Flow

```
┌──────────┐     ┌──────────────┐     ┌──────────────────┐     ┌────────────┐
│   UI     │     │  ViewModel   │     │   Repository     │     │ File System│
│ (Widget) │────▶│  (VM)        │────▶│  (CRUD)          │────▶│ (JSON)     │
│          │◀────│              │◀────│                  │◀────│            │
└──────────┘     └──────────────┘     └──────────────────┘     └────────────┘
     ▲                 ▲                      ▲                      ▲
     │                 │                      │                      │
  绑定属性          业务逻辑              持久化管理             文件存储
```

### Component Relationships

| Component | File | Responsibility |
|-----------|------|---------------|
| `RecipeDocument` | `recipe_config_model.h` | 配方数据结构，JSON 序列化 |
| `RecipeBasicInfo` | `recipe_config_model.h` | 基板/芯片几何参数 |
| `CameraSettings` | `recipe_config_model.h` | 相机采集参数 |
| `AlgoThresholds` | `recipe_config_model.h` | 算法检测阈值 |
| `MarkPointConfig` | `recipe_config_model.h` | Mark 点检测配置 |
| `PanelCalibSettings` | `recipe_config_model.h` | 面板标定设置 |
| `PositionCalibSettings` | `recipe_config_model.h` | 位置标定参数 |
| `RecipeRepository` | `recipe_config_model.h` | 持久化 CRUD 操作 |
| `RecipeConfigViewModel` | `recipe_config_view_model.h` | UI 业务逻辑 |
| `RecipeConfigView` | `recipe_config_view.h` | UI 视图层 |

---

## X-Macro Pattern

### How It Works

The recipe model uses the **X-macro** pattern to declare fields once and generate multiple code artifacts from the same definition:

```cpp
#define RECIPE_BASIC_INFO_FIELDS(X) \
    X(QString, name, QString(), "name") \
    X(int, width, 100, "width") \
    X(double, chipWidthMm, 0.15045, "chip_width_mm")
```

This macro is then invoked with different "X" transformers:

| Transformer | Generated Code | Purpose |
|-------------|---------------|---------|
| `PARAM_DECLARE` | Field declarations | `QString name; int width;` |
| `REFLECT_FROM_FIELD` | JSON deserialization | `j["name"] >> name;` |
| `REFLECT_TO_FIELD` | JSON serialization | `j["name"] = name;` |
| `EQ_FIELD_SUFFIX` | Comparison | `&& name == o.name` |

### Benefits

1. **Single Source of Truth**: Fields are defined once, eliminating drift between declarations, serialization, and comparison code
2. **Compile-Time Generation**: All code is generated at compile time — no runtime overhead
3. **Type Safety**: Full C++ type checking applies to all generated code
4. **Easy Maintenance**: Adding a field requires only adding one line to the X-macro

### Macro Chain

```
X-macro definition
    │
    ├── PARAM_DECLARE      → 字段声明 (struct members)
    ├── DEFINE_PARAM_STRUCT → 构造函数、序列化方法
    ├── REFLECT_FROM_FIELD  → JSON → struct
    ├── REFLECT_TO_FIELD    → struct → JSON
    ├── EQ_FIELD_SUFFIX     → operator==
    └── DEFINE_PARAM_TYPE_META → Qt metatype registration
```

---

## RecipeRepository

### Interface

| Method | Description | Returns |
|--------|-------------|---------|
| `load()` | Initialize repository, scan storage directory | `void` |
| `listRecipes()` | Get all available recipes | `QVector<RecipeListItem>` |
| `load(id, &out)` | Load specific recipe by ID | `bool` |
| `contains(id)` | Check if recipe exists | `bool` |
| `save(doc, prevId?)` | Save recipe (rename if prevId provided) | `bool` |
| `remove(id)` | Delete recipe by ID | `bool` |
| `setDefault(id)` | Mark recipe as default | `bool` |
| `defaultRecipeId()` | Get default recipe ID | `QString` |
| `generateNewRecipeId()` | Generate unique recipe ID | `QString` |
| `exportAllToDirectory(dir, &report)` | Export all recipes to directory | `bool` |
| `importFromJsonFiles(files, &report)` | Import recipes from JSON files | `bool` |
| `setStorageDirectory(dir)` | Set storage directory | `void` |
| `storageDirectory()` | Get storage directory | `QString` |

### Storage Format

Recipes are stored as individual JSON files:

```json
{
    "id": "recipe_001",
    "basic": {
        "name": "PANEL-A01",
        "width": 100,
        "height": 160,
        "group_pitch_x_mm": 1.25,
        "group_pitch_y_mm": 1.25,
        "chip_pitch_in_group_mm": 0.50,
        "chip_width_mm": 0.15045,
        "chip_height_mm": 0.059,
        "pad_width_mm": 0.32155,
        "pad_height_mm": 0.13275,
        "group_width_mm": 0.32155,
        "group_height_mm": 0.51625,
        "chip_color_order": 0,
        "chip_arrangement": 0,
        "offset_group_mark_bl": {"x": 0.0, "y": 0.0},
        "offset_group_mark_tr": {"x": 0.0, "y": 0.0}
    },
    "camera": {
        "light_brightness": 153,
        "exposure_us": 12000,
        "image_width": 1920,
        "image_height": 1080,
        "capture_accuracy": 2.95
    },
    "algo": {
        "group_confidence": 0.72,
        "chip_confidence": 0.68,
        "chip_defect_class": 0.70,
        "pad_defect_class": 0.65,
        "offset_x_um": 15.0,
        "offset_y_um": 15.0,
        "angle_offset_deg": 0.5
    },
    "marks": [
        {
            "capture_pos": {"x": 0.0, "y": 0.0, "z": 0.0},
            "marker_info": {
                "mark_type": 0,
                "mark_width": 32,
                "mark_height": 32,
                "is_white_mark": 0,
                "area": 100.0,
                "min_thresh": 0.6,
                "max_thresh": 1.0,
                "model_file_name": "",
                "model_name": ""
            }
        }
    ],
    "panel_calib": {
        "mode": 0,
        "fov_info": "未生成",
        "path_mode_index": 0,
        "gerber_file_name": "未导入"
    },
    "position_calib": {
        "track_position": 0.0,
        "scan_position": {"x": 0.0, "y": 0.0, "z": 0.0}
    }
}
```

---

## Integration with ViewModel

### Typical Usage

```cpp
// RecipeConfigViewModel 中通过 RecipeRepository 完成数据操作

void RecipeConfigViewModel::loadRecipeList()
{
    m_recipes = RecipeRepository::listRecipes();
    emit recipesChanged();
}

bool RecipeConfigViewModel::saveCurrentRecipe()
{
    if (!RecipeRepository::save(m_currentDoc))
        return false;
    updateRecipeList();
    return true;
}

bool RecipeConfigViewModel::deleteRecipe(const QString &id)
{
    if (!RecipeRepository::remove(id))
        return false;
    updateRecipeList();
    return true;
}
```

### JSON 序列化流程

```
ViewModel (业务逻辑层)
    │
    ├── saveRecipe()
    │   ├── RecipeDocument.reflectToJson(json)  → 结构体 → JSON
    │   └── RecipeRepository.save(doc)           → JSON → 文件
    │
    ├── loadRecipe()
    │   ├── RecipeRepository.load(id, doc)       → 文件 → JSON
    │   └── RecipeDocument.reflectFromJson(json) → JSON → 结构体
    │
    └── importRecipes()
        ├── RecipeRepository.importFromJsonFiles() → 批量导入
        └── 更新 UI 列表
```

---

## Key Design Decisions

### Why X-macro instead of Reflection?

C++ lacks built-in reflection. The X-macro pattern provides a lightweight alternative that:

| Approach | Pros | Cons |
|----------|------|------|
| **X-macro** | Compile-time, zero overhead, type-safe | Verbose macro definition |
| **Manual serialization** | Simple to understand | Easy to miss fields, maintenance burden |
| **Qt MetaObject** | Built-in, no macros | Limited to QObject, no nested structs |
| **External codegen** | Clean C++ | Build complexity, dependency |

### Why Separate Sub-structs?

Each aspect of the recipe (basic info, camera, algo, marks) is a separate struct:

1. **Independent reuse**: Camera settings can be used independently in other contexts
2. **Clear ownership**: Each struct has a single responsibility
3. **Easier testing**: Sub-structs can be tested in isolation
4. **Parallel development**: Different team members can work on different sub-structs

---

## Best Practices

1. **Always use Repository for persistence** — never access files directly
2. **Validate before save** — call `normalizeRecipeDocument()` before saving
3. **Handle import/export errors** — always check return values and reports
4. **Cache recipe list** — `listRecipes()` is called frequently; cache when appropriate
5. **Thread safety** — Repository methods are not thread-safe; use mutex in multi-threaded contexts
6. **Add new fields correctly** — append to the X-macro, never insert in the middle (breaks JSON compatibility)

---

## Related Files

| File | Path |
|------|------|
| Recipe model header | `plugins/aoi/data/dataModel/recipe_config_model.h` |
| Recipe model impl | `plugins/aoi/data/dataModel/recipe_config_model.cpp` |
| Base param macros | `plugins/aoi/data/dataModel/base/param_model_macros.h` |
| Param serialization | `plugins/aoi/data/dataModel/base/param_serde.h` |
| Param metatype | `plugins/aoi/data/dataModel/base/param_meta.h` |
| Recipe ViewModel | `plugins/aoi/ui/vm/recipe_config/recipe_config_view_model.h` |
| Recipe Config View | `plugins/aoi/ui/view/recipe_config/recipe_config_view.h` |