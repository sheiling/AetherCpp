# Agent 提问示例

> 部署时填写的**软件系统中文名称**会写入 `plugins/aoi/docs/AOI 检测软件 SOP描述.txt` 的【系统信息】，并作为 `plugins/aoi/docs/AOI 检测软件 软件操作SOP.md` 标题。  
> **工程名称（ProjectName）** 用于 Cursor 规则，并标识**读代码时的起点**：本工程「Project_Aether」**工作区根目录**（Cursor Open Folder 所打开的文件夹），**不要**用 `plugins/aoi/docs/../` 推断源码位置（文档在 `plugins/aoi/docs` 时该路径只会落到 `plugins/aoi/`，会漏掉同级 `plugins/lighten` 等目录）。  
> 下面 `{{...}}` 占位符在部署时会自动替换；带「按实际填写」的仍需你手动改。

---

## 1. 第一次生成设计方案（全量，从代码 + 外部模板）

适用：还没有 `设计方案.md`，或要大版本重写。

```
请按 .cursor/skills/code-to-template/SKILL.md，
使用「设计方案」模板，
基于本工程「Project_Aether」工作区根目录下 {{列举目录，如 app/、common/、plugins/}} 的源码，
全量生成 plugins/aoi/docs/AOI 检测软件 设计方案.md。
要求：保留模板全部章节；代码没有的信息写「代码中未体现」；
需要处自动生成 mermaid 图；新增章节标记 <!-- 自动补充 -->。
```

若模板在固定目录（本 kit 默认）：

```
请按 code-to-template skill，模板关键词「设计方案」，
代码源为本工程「Project_Aether」根目录下的 {{列举目录，如 app/、common/、plugins/}}，
输出到 plugins/aoi/docs/AOI 检测软件 设计方案.md。
```

---

## 2. 第一次生成设计方案（无外部模板，从现有代码归纳）

适用：没有 `D:/Downloads/模板/` 里的模板文件。

```
请阅读本工程「Project_Aether」工作区根目录下的主要源码（{{列举目录，如 app/、common/、plugins/}}），
为「AOI 检测软件（V2.0）」
编写 plugins/aoi/docs/AOI 检测软件 设计方案.md 初稿。
结构包含：名词解释、架构、核心流程、模块详解、接口与配置、异常与安全。
内容必须基于代码事实，推断处标注「推断：」。
```

---

## 3. 第一次生成软件操作 SOP（全量，必做）

适用：已写好 `SOP描述.txt` 并放入 UI 截图。

```
请根据 plugins/aoi/docs/AOI 检测软件 SOP描述.txt、plugins/aoi/docs/UI截图/、
plugins/aoi/docs/AOI 检测软件 设计方案.md（仅作背景，勿把架构段落抄进操作手册），
生成/刷新 plugins/aoi/docs/AOI 检测软件 软件操作SOP.md。

系统中文名称：AOI 检测软件
首行标题必须是：# AOI 检测软件 软件操作SOP
标题下注明：适用系统、适用对象、文档性质（现场操作手册）

全书结构（必遵 .cursor/sop/templates/sop-document-template.md）：
- 正文 §0 之前须有 **封面**（版本、生效日期、审批栏）、**文档说明**、**目录**（链到全部 ## / ### 页面及变更记录、自检清单）

读者与文风（必遵）：
- 读者是操作员、FAE，不是研发；详见 .cursor/sop/templates/sop-writing-style.md
- AOI 检测软件 软件操作SOP.md 成品另必遵 .cursor/sop/templates/sop-output-constraints.md
- 步骤短、一步一事；按钮/菜单/页签名称与页面插图一致
- 步骤/判定/误区/异常中禁止 JSON、数据库表名、Controller、链路等研发用语
- 后台行为转写为现场说法；路径/配置键名仅放「附：工程师参考」（可选）

结构与约束：
- 严格遵守 AOI 检测软件 SOP描述.txt 的【硬性输出约束】与【页面清单与内容要求】
- 全书：sop-document-template.md；单页参考 page-sop-template.md（非机械一页一图）
- AOI 检测软件 软件操作SOP.md 正文中禁止出现「UI截图」「SOP描述.txt」「设计方案.md」等字眼；Markdown 图片路径可含 UI截图/
- 关联多图可写在同一小节；可用表格、强调、符号、公式、Mermaid 等
- 详细操作步骤不少于 8 步；常见误区≥3、异常处理≥3
- 含全局章节：班前/换型/班后检查、变更记录、自检清单
- 禁止虚构界面与 AOI 检测软件 SOP描述.txt 中未出现的功能；不足处写「待补充项」
```

---

## 4. 改代码后增量维护（设计 + SOP 联动）

适用：Agent 改完功能代码，需同步三份文档。

```
本次我修改了 {{模块/文件，如 MesHandler、主页 QML}}，
请按 .cursor/rules/sop-maintenance.mdc 协议：

1. 若影响架构/接口：按 incremental-design-doc skill 增量更新 plugins/aoi/docs/AOI 检测软件 设计方案.md
2. 若影响用户操作：按 incremental-sop-doc skill 更新 plugins/aoi/docs/AOI 检测软件 SOP描述.txt 与 plugins/aoi/docs/AOI 检测软件 软件操作SOP.md（必遵 sop-writing-style.md）
3. 变更记录按 revision-log-policy：**先比最后一行日期，跨自然日必须追加新行**；同日小改才合并最后一行

系统名称：AOI 检测软件。模块索引见 plugins/aoi/docs/doc-index/sop-modules.md。
策略见 revision-log-policy.md、sop-writing-style.md。
```

---

## 5. 仅增量更新 SOP（不改设计文档）

适用：只改了 UI 文案、按钮、操作流程，不动架构。

```
本次仅变更用户操作（{{页面名}}），不涉及架构。
请按 incremental-sop-doc skill：
1. 增量更新 plugins/aoi/docs/AOI 检测软件 SOP描述.txt 中「{{章节，如一、1）主页}}」
2. 同步 plugins/aoi/docs/AOI 检测软件 软件操作SOP.md 的 {{§章节号}}（现场用语，见 sop-writing-style.md）
3. 变更记录按 revision-log-policy（跨日追加新行）
SOP 标题保持 # AOI 检测软件 软件操作SOP，适用系统 AOI 检测软件。
```

---

## 6. 仅增量更新设计方案

适用：只改了 Handler、配置 schema、流程，UI 操作说明不变。

```
本次变更 {{模块}} 的架构/接口（{{简述}}），不涉及用户可见操作。
请按 incremental-design-doc skill 增量更新 plugins/aoi/docs/AOI 检测软件 设计方案.md 的 {{章节}}，
并更新修订记录（先比日期：跨日追加新行，见 revision-log-policy.md）。不要改 AOI 检测软件 软件操作SOP.md。
```

---

## 7. SOP 生成完成后补模块索引（推荐）

```
AOI 检测软件 软件操作SOP.md 已首次生成。请根据 plugins/aoi/docs/AOI 检测软件 软件操作SOP.md 的章节结构，
帮我填写 plugins/aoi/docs/doc-index/sop-modules.md 的模块表（id、SOP描述位置、SOP.md 章节、截图文件名）。
```

---

## 8. 部署后自检

```
请检查本工程文档维护体系是否就绪：
Project Rules 中 sop-maintenance 是否生效；
plugins/aoi/docs/AOI 检测软件 SOP描述.txt 是否与 UI 截图一致；
plugins/aoi/docs/AOI 检测软件 软件操作SOP.md 标题是否为「AOI 检测软件 软件操作SOP」。
列出缺失项。
```

---

## 名称对照（避免混淆）

| 名称 | 来源 | 用途 |
| --- | --- | --- |
| 工程名称 ProjectName | setup 提问 / 文件夹名 | Cursor Rule、sop-modules.md frontmatter 的 project；**读代码起点**（工作区根目录） |
| 软件系统中文名称 | setup 提问 | SOP 标题、SOP描述【系统信息】、操作手册封面 |
| 系统代号 | setup 提问（可留空） | 有则显示为「系统名（代号）」；留空则文档中只显示系统名，不出现括号 |
