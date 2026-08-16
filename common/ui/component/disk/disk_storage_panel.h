#pragma once

/**
 * @file disk_storage_panel.h
 * @brief 本机磁盘容量监控面板（定时刷新，跨插件复用）
 *
 * 展示 `QStorageInfo::mountedVolumes()` 中有效本地卷的总/剩余容量与进度条。
 * 使用率阈值着色：>60% 警告色、>80% 危险色（见 `ThemeTokens`）。
 *
 * 默认 i18n 键见 `DiskStoragePanelOptions`（`ui.disk_storage.*`），
 * 译文维护于工程根目录 `config/app_zh_CN.ts` / `config/app_en_US.ts`，
 * 编译为 `config/i18n/app_*.qm` 由 `LanguageManager` 加载。
 */

#include "component_global.h"

#include <QWidget>

class QLabel;
class QProgressBar;
class QTimer;
class QVBoxLayout;

namespace ui {

class ThemedSectionPanel;

/** 磁盘面板 i18n 键与刷新策略（格式串占位与 TR 键一致） */
struct DiskStoragePanelOptions
{
    /** 区块标题翻译键 */
    QString titleKey = QStringLiteral("ui.disk_storage.title");
    /** 已用百分比，占位 %1 */
    QString usedPctFormatKey = QStringLiteral("ui.disk_storage.used_pct");
    /** 总/剩余容量，占位 %1=total %2=free（GiB 字符串） */
    QString detailFormatKey = QStringLiteral("ui.disk_storage.detail");
    /** 定时刷新间隔（ms）；≤0 表示不启动定时器 */
    int refreshIntervalMs = 30'000;
};

/**
 * @brief 磁盘容量监控卡片
 *
 * - 外框：`ThemedSectionPanel` + 可滚动行列表
 * - 数据：`refreshDisks()` 扫描卷；`retranslateUi()` 重译并重建行
 * - 主题：`bindThemeRefresh` 驱动行内 QSS 与进度条 chunk 色
 */
class UI_COMPONENT_EXPORT DiskStoragePanel : public QWidget
{
    Q_OBJECT

public:
    explicit DiskStoragePanel(QWidget *parent = nullptr,
                            DiskStoragePanelOptions options = {});

    /** 重译标题并重建各盘符行（语言切换时调用） */
    void retranslateUi();

    /** 立即重新扫描挂载卷并刷新 UI */
    void refreshDisks();

    DiskStoragePanelOptions options() const { return m_options; }
    void setOptions(const DiskStoragePanelOptions &options);

private:
    struct DiskRowWidgets
    {
        QLabel       *drive    = nullptr;
        QLabel       *detail   = nullptr;
        QLabel       *usage    = nullptr;
        QProgressBar *bar      = nullptr;
        int           usagePct = 0;
    };

    void buildLayout();
    void applyRowStyles();
    void startRefreshTimer();

    DiskStoragePanelOptions  m_options;
    ThemedSectionPanel      *m_panel      = nullptr;
    QVBoxLayout             *m_rowsLayout = nullptr;
    QTimer                  *m_timer      = nullptr;
    QVector<DiskRowWidgets>  m_rows;
};

} // namespace ui
