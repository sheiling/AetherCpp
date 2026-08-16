# AOI 参数模型基础设施（`data/dataModel/base`）

本目录存放参数反射、序列化与共享枚举定义，供 `recipe_config_model`、`system_config_model` 及参数审计模块复用。

## 文件职责

| 文件 | 职责 |
|------|------|
| `param_define.h` | 共享枚举（`DefectKind`、`TrackMode` 等）、几何类型（`Point3`/`Point`/`Rect`）及常量 |
| `param_meta.h` / `.cpp` | 参数类型元数据、变更审计标签归一化（`ParamTypeMeta`、`DEFINE_PARAM_TYPE_META`） |
| `param_serde.h` / `.cpp` | JSON 反射扩展：`QString`、`QVector<T>`、`enum class` 序列化 |
| `param_model_macros.h` | X-Macro：`DEFINE_PARAM_STRUCT` 自动生成 `reflectFromJson` / `reflectToJson` / `operator==` |

## 依赖关系

```
param_define.h          （无 base 内依赖）
param_serde.h           → reflector.h
param_model_macros.h    → param_serde.h
param_meta.h            → reflector.h
业务 model *.h          → param_define.h + param_meta.h + param_model_macros.h
```

## 新增业务结构体字段

1. 在对应 `*_FIELDS` X-Macro 中追加一行：`X(类型, 字段名, 默认值, "json_key")`
2. 在结构体内保留 `DEFINE_PARAM_STRUCT(结构体名, *_FIELDS)`
3. 在文件末尾追加 `DEFINE_PARAM_TYPE_META(aoi::结构体名, "label", *_FIELDS)`
4. 若类型含模板逗号（如 `QMap<K,V>`），使用 `using` 别名后再写入宏

## 几何类型（`Point3` / `Point` / `Rect`）

定义于 `param_define.h`，由 `DEFINE_PARAM_STRUCT` 自动生成 JSON 反射。配方模型直接引用，勿在业务头文件中重复定义。

## 轨道模式（`TrackMode`）

- 枚举定义于 `param_define.h`
- 系统配置字段：`AoiSystemConfig::common.trackMode`（`CommonParams`）
- 配方 Mark 槽位数量：`recipeMarkCount(trackMode)`（单轨 2 / 双轨 4），`trackMode` 来自 `AoiSystemConfig::common`

## 新增 `DefectKind` 缺陷类型

按顺序修改（**禁止改动已有枚举整型值**）：

1. **`param_define.h`**：在 `DefectKind` 末尾追加成员，`kDefectKindCount += 1`
2. **`system_config_model.cpp`**：`defectKindStorageKey` / `defectKindFromStorageKey` 增加 snake_case 键
3. **`system_config_model.h`**：`ParamTypeMetaProvider<DefectSettings>` 手写字段列表增加对应 JSON 键
4. **`recipe_config_model.cpp`**：`defectKindDisplayKey` 增加 UI 翻译键
5. **UI**：`ui/view/system_config/*`、`recipe_config_view.cpp`、首页快照表等依赖 `kDefectKindCount` 的循环会自动扩展，但需补充翻译资源（`.ts`）
6. **文档**：`docs/AOI 检测软件 设计方案.md` 修订记录与缺陷相关章节

## 新增其他枚举成员

- 在 `param_define.h` 对应 `enum class` 末尾追加，**不修改已有数值**
- `TrackMode` 变更时同步系统配置 UI 与配方 Mark 相关逻辑
- 若枚举用于 UI 下拉（如 Mark 类型），在对应 View 的匿名命名空间内维护 `QList<QPair<int, QString>>` 项表
- Mark 检测参数以 `vision_algo::MarkerDetectInfo` 为真相源；配方侧在 `recipe_config_model.h` 用 `MARKER_DETECT_INFO_FIELDS` + `DEFINE_PARAM_TYPE_META` 注册嵌套元信息，`MarkPointConfig` 经 `MARK_POINT_CONFIG_FIELDS` 包含 `marker` 字段

## Include 约定

CMake 已将 `data/dataModel/base` 加入 include 路径，业务代码直接：

```cpp
#include "param_define.h"
#include "param_meta.h"
#include "param_model_macros.h"
```

`system_config_model` 仅需 `DefectKind` 时应包含 `param_define.h`，**不要**为枚举而 include 整个 `recipe_config_model.h`。

## 参数审计

`DEFINE_PARAM_TYPE_META` 生成的元数据驱动 `param_change_parser` 生成变更标签。自定义 JSON 形状（如 `DefectSettings` 扁平键对象）需手写 `ParamTypeMetaProvider` 特化，使 `jsonKey` 与持久化 JSON 一致。

## UI 数据绑定

`AoiSystemConfig`、`RecipeDocument` 作为 ViewModel 的 QVariant 属性，由 `DataBinding::bindStructField` 与表单控件同步。AOI 侧扩展见 `plugins/aoi/ui/binding/README.md`。
