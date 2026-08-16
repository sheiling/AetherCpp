# 私有知识库索引

> 工程：**Project_Aether**  
> Agent 回答前先查本表，再 Read 对应 `path` 文件。

## 使用说明

1. 正文放在 `knowledge-base/<分类>/` 下（如 `cpp/`、`process/`）
2. 每新增一篇，在本表追加一行
3. `tags` 用英文或中文逗号分隔，便于 Grep

| id | 标题 | path | tags | 摘要 |
| --- | --- | --- | --- | --- |
| readme | 知识库说明 | plugins/aoi/docs/knowledge-base/README.md | guide | 使用说明 |
| perm-i18n-convention | 权限资源与 i18n 翻译约定 | plugins/aoi/docs/knowledge-base/i18n/权限资源与翻译约定.md | i18n,permission,PermService,owner_plugin,Catalog Reconcile,翻译 | 权限目录 display_name 存 key、owner_plugin、注册 API、Reconcile、qm 归属与 PTXT 命名 |
| cpp-coding-standard | C++编码规范 | plugins/aoi/docs/knowledge-base/cpp/C++编码规范.md | cpp,coding-standard | C++ 命名与模块约定 |
| leadshine-nmc-api-manual | 雷赛总线运动控制器 API 编程手册 V1.0 | plugins/aoi/docs/knowledge-base/运控卡/雷赛/雷赛总线运动控制器API函数编程手册V1.0.md | motion,运控,雷赛,NMC,API,Leadshine | 雷赛总线控制器 API 参考：初始化、点位/插补/回零、IO、参数配置与错误处理 |
| hikvision-camera-cpp-sdk | 海康相机 C++ 软件开发说明书 | plugins/aoi/docs/knowledge-base/相机/海康相机C++软件开发说明书.md | camera,相机,海康,Hikvision,MVS,SDK | 海康工业相机 SDK：取图流程、C 接口、参数节点、回调/主动采集与运行环境 |
| daheng-camera-cpp-sdk | 大恒相机 C++ 软件开发说明书 V1.2.5 | plugins/aoi/docs/knowledge-base/相机/大恒相机C++软件开发说明书.md | camera,相机,大恒,Daheng,Galaxy,SDK | 大恒 Galaxy SDK：环境搭建、枚举/打开设备、属性控制、采集与图像处理 |
