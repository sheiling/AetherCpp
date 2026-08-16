#pragma once

/**
 * @file system_config_view_model.h
 * @brief 系统配置 ViewModel（三子页共享单例）
 *
 * 职责：
 * - 持有可绑定属性 `config`（`AoiSystemConfig`），供开关/通用/阈值子页 DataBinding 双向同步
 * - `load()` 从仓储读取；`save()` 写回并触发参数变更审计
 * - 子页 View 保存前须对各 BindingSession 调用 `updateAllSources()` flush Manual 项
 *
 * 生命周期：由 `AoiPlugin::initialize()` 创建并挂到 `PageWithSubNav` 容器下；
 * `SystemConfigSwitchView` / `GeneralView` / `ThresholdView` 仅持有指针。
 *
 * Command 绑定：
 * - `system.save` — 比较磁盘与内存，有变更则 `save()` 并 `saveSucceeded`
 */

#include "viewmodel.h"
#include "command.h"
#include "system_config_model.h"

namespace aoi {

class SystemConfigViewModel : public ViewModel
{
    Q_OBJECT

public:
    /** ViewModel 属性名，值为完整 `AoiSystemConfig` */
    static constexpr const char *PROP_CONFIG = "config";

    VIEWMODEL_STRUCT_PROPERTY(AoiSystemConfig, config, Config)

signals:
    void configChanged();
    void saveSucceeded();
    void saveFailed();

public:
    explicit SystemConfigViewModel(const QString &pageKey, QObject *parent = nullptr);

    /** Command 绑定：name=`system.save`，触发持久化与审计 */
    Command *saveCommand() const { return m_saveCommand; }

public slots:
    /** 从 `SystemConfigRepository` 加载并 setConfig（load 期间抑制自动持久化标志） */
    void load();
    /** 若与磁盘一致则 true；写入失败 emit saveFailed */
    bool save();

private:
    /** load 期间为 true，避免 setConfig 触发写盘（预留扩展） */
    bool                  m_suppressPersist = false;
    QString               m_pageKey;
    FunctionalCommand    *m_saveCommand     = nullptr;
};

} // namespace aoi
