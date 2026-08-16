# AOI UI 数据绑定（`ui/binding`）

本目录存放 AOI 插件对 `common/ui/mvvm` 的扩展。

## 文件

| 文件 | 职责 |
| --- | --- |
| `aoi_param_binding.h` | `bindStructFieldPillSwitch`：`PillSwitch` ↔ struct 布尔字段 |
| `command_button_binding.h` | `bindCommandToButton`：侧栏 `QAbstractButton` ↔ `Command` |
| `home_runtime_binding.h/.cpp` | 首页 `HomeRuntimeState` → KPI/硬件/流程/虚拟图/日志/快照/预览 分区单向绑定 |
| `disk/disk_storage_panel.h/.cpp`（`ui_component`） | 本机磁盘容量监控；i18n 键 `ui.disk_storage.*` 见 `config/app_*.ts` |
| `fsm/machine_status_display.h` | `MachineStatus` → KPI 设备状态/运行模式 **i18n key** |
| `fsm/home_fsm_binding.h/.cpp` | 首页 FSM：`statusChanged` → KPI + 侧栏命令一键挂载 |

## 首页 FSM 绑定（示例）

```cpp
binding::fsm::HomeSidebarCommands cmds;
binding::fsm::bindHomeMachineFsm(vm, fsm, actions, &cmds);
// View 层：bindCommandToButton(btn, cmds.start);
```

- KPI `deviceState` / `runMode` 由 `MachineStatusNotifier::statusChanged` 自动写入（i18n key）
- 命令 `canExecute` 随同一信号刷新，无需 ViewModel 内手写守卫函数

## 相关

- FSM 信号：`scheduler::MachineFSM::statusNotifier()` → `statusChanged(from, to)`
- 框架 API：`common/ui/mvvm/databinding.h`
