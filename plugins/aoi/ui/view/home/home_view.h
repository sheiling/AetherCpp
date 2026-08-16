#pragma once

/**
 * @file home_view.h
 * @brief 首页 View 声明：薄壳布局 + HomeViewModel 绑定
 *
 * 编排侧栏设备控制、KPI 行、流程条、日志、图像预览与快照表；
 * 展示数据来自 HomeRuntimeState（经 HomeViewModel），不含业务逻辑。
 */

#include "pagewidget.h"

class QLabel;
class QHBoxLayout;

namespace ui {

class ImagePreview;
class KpiMetricCard;
class DiskStoragePanel;
class LogView;
class ThemedSectionPanel;
class ThemedTableWidget;

} // namespace ui

namespace aoi {

class DeviceControlPanel;
class HardwareStatusPanel;
class HomeProcessFlowWidget;
class HomeViewModel;
class PanelVirtualMapWidget;

/** @brief AOI 首页：运行时 KPI / 硬件状态 / 流程 / 日志 / 预览 / 快照展示 */
class HomeView : public PageWidget
{
    Q_OBJECT

public:
    explicit HomeView(QWidget *parent = nullptr);

protected:
    void onLoad() override;
    void retranslateUi() override;
    void applyTheme() override;

private:
    void buildLayout();
    void buildSidebar();
    QHBoxLayout *buildKpiRow(QWidget *parent);
    ui::ThemedSectionPanel *buildVirtualPanel(QWidget *parent);
    HomeProcessFlowWidget *buildProcessFlow(QWidget *parent);
    ui::ThemedSectionPanel *buildLogPanel(QWidget *parent);
    ui::ThemedSectionPanel *buildPreviewPanel(QWidget *parent);
    ui::ThemedSectionPanel *buildSnapshotPanel(QWidget *parent);
    /** 侧栏 Command 绑定 + runtime 分区 property 绑定 */
    void setupBindings();
    /** 主题/语言切换后重推 runtime 至各分区绑定 */
    void notifyRuntimeRefresh();
    /** 快照 meta 行与预览 caption 的装饰 QSS（随 ThemeTokens 与 SN 总状态切换） */
    void applyHomeDecorStyles();

    HomeViewModel            *m_viewModel           = nullptr;
    QWidget                  *m_sidebar             = nullptr;
    DeviceControlPanel       *m_deviceControlPanel  = nullptr;
    HardwareStatusPanel      *m_hardwarePanel       = nullptr;
    ui::DiskStoragePanel     *m_diskPanel           = nullptr;
    QVector<ui::KpiMetricCard *>  m_kpiCards;
    PanelVirtualMapWidget        *m_virtualMapWidget = nullptr;
    HomeProcessFlowWidget        *m_processFlow     = nullptr;
    ui::ThemedSectionPanel       *m_logPanel        = nullptr;
    ui::LogView                  *m_logView         = nullptr;
    ui::ImagePreview             *m_mark1Preview    = nullptr;
    ui::ImagePreview             *m_mark2Preview    = nullptr;
    ui::ImagePreview             *m_fovPreview      = nullptr;
    ui::ThemedTableWidget        *m_snapshotTable   = nullptr;
    QLabel                   *m_snapshotSnLbl   = nullptr;
    QLabel                   *m_snapshotStatusLbl = nullptr;
    QLabel                   *m_mark1Title      = nullptr;
    QLabel                   *m_mark2Title      = nullptr;
    QLabel                   *m_fovTitle        = nullptr;
};

} // namespace aoi
