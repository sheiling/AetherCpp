# scheduler — 流程调度引擎

> 头文件统一以 `#include <scheduler/...>` 引用（CMake `PUBLIC` 根目录为 `common/core/`）。

## 目录结构

```
scheduler/
├── scheduler_global.h      # DLL 导出宏
├── README.md               # 本文件
├── docs/                   # 设计/分析文档（非运行时依赖）
├── api/                    # ★ 对外公共 API（Bootstrap / 插件优先 include 此层）
│   ├── ischeduler_service.h
│   ├── scheduler_runtime.h / .cpp
│   └── scheduler_service_impl.h / .cpp
├── core/                   # FlowState、StepResult、UUID 等基础类型
├── context/                # FlowContext、变量作用域
├── event/                  # EventBus
├── engine/                 # FlowLoader、Scheduler、FlowInstance、FlowDefinition
├── step/                   # IStep、StepFactory、控制流/同步步骤、StepFlowController
│   ├── control/
│   └── sync/
├── machine/                # MachineFSM、ButtonMonitor
├── alarm/                  # AlarmManager
├── station/                # StationManager、LineLoader（多工位）
├── persistence/            # IFlowPersistence、JSON 持久化
├── compensation/           # CompensationManager
└── examples/               # 示例与 JSON 样例（独立 CMake 目标）
```

## 分层（自下而上）

| 层 | 目录 | 职责 |
| --- | --- | --- |
| 基础 | `core/`、`context/`、`event/` | 类型、上下文、事件总线 |
| 引擎 | `engine/` | 流程注册、调度、实例生命周期 |
| 步骤 | `step/` | 工步抽象、JSON `type` 工厂、流程控制网关 |
| 工业场景 | `machine/`、`alarm/`、`station/` | 整机 FSM、报警、多工位 |
| 对外 | `api/` | Bootstrap 装配 + `ISchedulerService` 插件发现 |

## 典型用法

**Bootstrap**（`SchedulerModule`）：

```cpp
#include <scheduler/api/scheduler_runtime.h>
#include <scheduler/api/scheduler_service_impl.h>
```

**插件**（业务工步 + 获取调度服务）：

```cpp
#include <scheduler/step/i_step.h>
#include <scheduler/step/step_factory.h>
#include <scheduler/api/ischeduler_service.h>
// PluginManager::getInterface<ISchedulerService>()
```

**流程 JSON**：由 `FlowLoader` 加载，格式见 `examples/flow_inspection.json`。

## 文档

- 设计：`docs/scheduler_design.md`
- 源码分析：`docs/scheduler_analysis.md`
