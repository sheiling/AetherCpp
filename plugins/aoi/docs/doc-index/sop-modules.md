---
project: Project_Aether
systemDisplayName: AOI 检测软件
systemCode: V2.0
sopTitle: AOI 检测软件 软件操作SOP
docsDir: plugins/aoi/docs
sopSource: plugins/aoi/docs/AOI 检测软件 SOP描述.txt
sopOutput: plugins/aoi/docs/AOI 检测软件 软件操作SOP.md
designDoc: plugins/aoi/docs/AOI 检测软件 设计方案.md
---

# SOP 模块索引

> 系统：**AOI 检测软件（V2.0）**  
> 增量维护时查本表定位章节。

| id | 标题 | SOP描述位置 | SOP.md 章节 | 设计文档章节 | 截图 |
| --- | --- | --- | --- | --- | --- |
| global-check | 班前/换型/班后检查 | 〇、0） | §0 | 核心流程 | — |
| sys-config-switch | 系统配置·开关设置 | 一、1） | §1.1 | 模块详解 plugins/aoi | 系统配置-开关设置.png |
| sys-config-general | 系统配置·通用设置 | 一、2） | §1.2 | 接口与配置 AoiSystemConfig | 系统配置-通用配置.png |
| sys-config-threshold | 系统配置·阈值范围 | 一、3） | §1.3 | 接口与配置 AoiSystemConfig | 系统配置-阈值范围.png |
| recipe-config-main | 产品配方配置·主界面 | 二、1） | §2.1 | 核心流程 §3 | 产品配方配置.png |
| recipe-config-basic | 产品配方配置·基本信息 | 二、2） | §2.2 | 接口与配置 RecipeDocument | 产品配方配置-产品信息.png |
| recipe-config-camera | 产品配方配置·相机设置 | 二、3） | §2.3 | 接口与配置 RecipeDocument | 产品配方配置-相机设置.png |
| recipe-config-algo | 产品配方配置·算法阈值 | 二、4） | §2.4 | 接口与配置 RecipeDocument | 产品配方配置-算法阈值.png |
| recipe-config-mark | 产品配方配置·MARK制作 | 二、5） | §2.5 | 核心流程 §3/§5 | 产品配方配置-Marker制作.png |
| recipe-config-calib | 产品配方配置·标定 | 二、6） | §2.6 | 接口与配置 RecipeDocument | 产品配方配置-标定.png |
| recipe-config-other | 产品配方配置·其它设置 | 二、7） | §2.7 | 接口与配置 RecipeDocument | 产品配方配置-其它设置.png |
| home-main | 首页·运行监控 | 三、1） | §3.1 | 核心流程 §3.1 | 待补充 首页.png |
| inspection-history | 检测结果·历史列表与详情 | 四、1） | §4.1 | 核心流程 §3.1.5 | 检测结果.png、检测详情弹窗.png |

## 交叉索引

- 代码路径 → 设计章节：同目录 [design-sections.md](./design-sections.md)
- 文风与模板：`.cursor/sop/templates/`（固定）

## 待补充（已挂接插件、尚无 SOP 详操）

| id | 说明 |
| --- | --- |
| home | `nav.aoi.home` 已注册；SOP描述已写页面事实，缺 **首页.png** 与 §3.1 逐步操作 |

## 新增页面时

1. 更新 `plugins/aoi/docs/AOI 检测软件 SOP描述.txt`
2. **本表**增加一行
3. 更新 `plugins/aoi/docs/AOI 检测软件 软件操作SOP.md` 与目录
4. `plugins/aoi/docs/UI截图/` 放入插图
