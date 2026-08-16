---
name: incremental-design-doc
description: >-
  Incrementally patches plugins/aoi/docs/AOI 检测软件 设计方案.md after code or architecture changes.
  Use when AI modifies application modules, flows, interfaces, config, handlers,
  UI actions, or when doc-maintenance protocol triggers a design doc update.
  For full regeneration from template, use .cursor/skills/code-to-template/SKILL.md.
---

# 设计方案增量更新

在代码变更后，**只修改** `plugins/aoi/docs/AOI 检测软件 设计方案.md` 中受影响的章节，禁止全量重写。

> **读代码时**：从本工程「Project_Aether」工作区根目录定位源码，不要用 `plugins/aoi/docs/../` 推断。

## 与 code-to-template 的分工

| 场景 | 使用 |
| --- | --- |
| AI 改代码后顺带维护设计文档 | **本 skill**（增量） |
| 用户明确要求「按模板重新生成设计方案」 | `.cursor/skills/code-to-template/SKILL.md`（全量） |

## 执行流程

1. 识别代码变更范围
2. 查 `plugins/aoi/docs/doc-index/design-sections.md` 定位章节
3. 只 Read + 编辑目标 `##` / `###` 段落
4. 按 [revision-log-policy.md](../../sop/templates/revision-log-policy.md) 更新 `## 修订记录`（**先比 lastDate 与 todayDate，跨日必追加新行**）
5. 自检

## 修订记录规则

策略全文：`.cursor/sop/templates/revision-log-policy.md`

1. **先比较日期**：最后一行日期 ≠ 本次日期 → **末尾追加新行**
2. **同一自然日**且改动小 → 只改最后一行「变更摘要」，用 `；` 追加；不递增版本号
3. **跳过**：正文未改的内部修复

## 判定是否需更新

**执行**：模块/Handler/流程/接口/配置 schema/UI Action 变更

**跳过**：纯格式化、注释、与架构/接口无关的内部细节

## 增量改写规则

- 内容必须来自代码/配置事实；无依据写「代码中未体现」
- 推断标注「推断：」；新增章节带 ` <!-- 自动补充 -->`
- 章节模板：`.cursor/sop/templates/design-section-template.md`

## 红线

- ❌ 不要全量重写设计文档
- ❌ 不要编造不存在的功能、接口、参数
- ❌ 跨自然日仍合并到同一修订行
- ❌ 同日小改动连续追加多行修订记录

## 自检

- [ ] 只改了受影响章节
- [ ] 修订记录已按 `revision-log-policy.md` 处理（**跨日已追加**；同日才合并）
- [ ] 未编造接口/流程
