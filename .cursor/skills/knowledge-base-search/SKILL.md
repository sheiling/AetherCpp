---
name: knowledge-base-search
description: >-
  Search and cite private docs under plugins/aoi/docs/knowledge-base before answering.
  Use when the user asks coding standards, internal process, terminology,
  or says knowledge base / 知识库 / 查规范 / C++规范.
---

# 知识库检索

在回答技术、流程、规范类问题前，从 `plugins/aoi/docs/knowledge-base/` 取事实，禁止臆造。

## 检索顺序

1. Read `plugins/aoi/docs/knowledge-base/_index.md`
2. 按 `tags`、标题、摘要匹配用户问题
3. Read 命中条目的 `path` 全文；必要时 Grep `plugins/aoi/docs/knowledge-base/**/*.md`
4. 仍不足：说明「知识库未覆盖」，再问用户或查工程代码

## 回答格式

- 先给结论，再列要点
- 每条规范注明来源：`[知识库] plugins/aoi/docs/knowledge-base/...`
- 与代码建议冲突时，说明规范出处并询问是否例外

## 新增条目（用户要求入库时）

1. 将 `.md` / `.txt` 放入 `plugins/aoi/docs/knowledge-base/<分类>/`
2. 在 `_index.md` 表格追加一行：`| id | 标题 | path | tags | 摘要 |`
3. id 用小写英文与连字符，path 用相对 `plugins/aoi/docs/` 的路径

## 跳过检索

- 纯闲聊、与规范无关的单次命令
- 用户明确「不要查知识库，只看当前文件」
