# 文档维护索引（随功能演化）

本目录存放**会随页面/模块增加而更新**的映射表，与业务文档同属 `plugins/aoi/docs/`，日常维护只改此处及三份正文，**不必改 `.cursor/`**。

| 文件 | 用途 |
| --- | --- |
| [sop-modules.md](./sop-modules.md) | UI 模块 id ↔ SOP描述 / 操作手册章节 / 截图 |
| [design-sections.md](./design-sections.md) | 代码路径 ↔ 设计方案章节 ↔ SOP 模块 id |

## 与 `.cursor/` 的分工

- **`.cursor/`**：规则、Skill、模板、文风（部署后少改）
- **`plugins/aoi/docs/`**：设计方案、SOP描述、操作手册、**本目录**、UI截图（随功能演化）
- **`plugins/aoi/docs/guides/`**：部署指南、Agent 提问示例（少改）
