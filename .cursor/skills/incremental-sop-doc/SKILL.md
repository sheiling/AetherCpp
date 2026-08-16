---

name: incremental-sop-doc

description: >-

  Incrementally updates plugins/aoi/docs/AOI 检测软件 SOP描述.txt and plugins/aoi/docs/AOI 检测软件 软件操作SOP.md

  after user-visible changes. Use when sop-maintenance protocol triggers SOP updates.

  Enforces field-facing writing style, sop-output-constraints for the .md manual, and revision-log date rules.

---



# SOP / 操作手册增量更新



在**用户可见**功能、菜单、流程、提示变化后，增量维护：



1. `plugins/aoi/docs/AOI 检测软件 SOP描述.txt`

2. `plugins/aoi/docs/AOI 检测软件 软件操作SOP.md`



> **设计方案**由 `.cursor/skills/incremental-design-doc/SKILL.md` 维护；本 skill **不**写架构章节。



## 执行流程



1. 查 `plugins/aoi/docs/doc-index/sop-modules.md` 定位模块与章节

2. 读 `plugins/aoi/docs/UI截图/` 插图（若有）确认控件名称

3. 增量改 SOP描述 对应【页面】段落

4. 同步 `AOI 检测软件 软件操作SOP.md` 对应 § 章节：

   - 全书结构：`.cursor/sop/templates/sop-document-template.md`（**封面、文档说明、目录**）

   - 成品约束：`.cursor/sop/templates/sop-output-constraints.md`（**必遵**）

   - 单页参考：`.cursor/sop/templates/page-sop-template.md`（非机械一页一图）

5. 按 `.cursor/sop/templates/revision-log-policy.md` 更新 **变更记录**（**先比日期**）

6. 增删页面时**同步更新「## 目录」**；缺封面/目录时补全

7. 自检



## 写作风格（必遵）



- `.cursor/sop/templates/sop-writing-style.md` — 现场用语

- `.cursor/sop/templates/sop-output-constraints.md` — **仅 AOI 检测软件 软件操作SOP.md**：禁维护用语、灵活插图、富版式



## 变更记录



1. 最后一行日期 **≠** 本次日期 → **末尾追加新行**

2. **同一自然日** 且改动小 → 只扩写最后一行（`；` 连接）



## 红线



- ❌ 把研发设计文档整段复制进操作手册

- ❌ 在 AOI 检测软件 软件操作SOP.md 正文写「UI截图」「SOP描述.txt」「设计方案.md」等（路径与图注见 output-constraints）

- ❌ 跨自然日仍合并到同一条变更记录



## 全书结构（生成/全量时必含）



| 区块 | 要求 |

| --- | --- |

| 封面 | 文档名称、适用系统、版本、生效日期、审批栏 |

| 文档说明 | 3～5 条，读者与优先级（不写维护文件名） |

| 目录 | 链到全部 `##` / `###` 页面 + 变更记录 + 自检清单 |

| 正文 | §0～§N + 变更记录 + 自检清单 |



## 自检



- [ ] 已读 `sop-writing-style.md`、`sop-output-constraints.md`、`sop-document-template.md`

- [ ] 含封面、目录，目录与正文标题一致

- [ ] AOI 检测软件 软件操作SOP.md 无禁止字眼；插图编排合理（可多图同节）

- [ ] 变更记录日期判断正确

- [ ] 未虚构功能


