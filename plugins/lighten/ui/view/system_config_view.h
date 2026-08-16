#pragma once

/**
 * @file system_config_view.h
 * @brief 系统配置视图（开关、检测项分色、路径、延时、判定阈值）
 */

#include "pagewidget.h"

#include <QVector>
#include <QSpinBox>
#include <QPointer>

class QDialog;
class QLineEdit;
class QScrollArea;
class QSpinBox;
class QWidget;

class CfgPillSwitch;

namespace lighten {

class SystemConfigViewModel;

/** 检测项分色开关控件聚合（红/绿/蓝/总开关） */
struct DetectRowSwitches {
    CfgPillSwitch *red    = nullptr;
    CfgPillSwitch *green  = nullptr;
    CfgPillSwitch *blue   = nullptr;
    CfgPillSwitch *master = nullptr;
};

/** 判定阈值上下限 SpinBox 对 */
struct ThresholdSpinPair {
    QString   name;
    QSpinBox *lower = nullptr;
    QSpinBox *upper = nullptr;
};

/**
 * @brief 系统配置 — 单页四象限内容区
 */
class SystemConfigView : public PageWidget
{
    Q_OBJECT

public:
    explicit SystemConfigView(QWidget *parent = nullptr);

protected:
    void onLoad() override;
    void applyTheme() override;

private:
    void applyMainStylesheet();
    void applyMainPanelStyle();
    void setupBindings();
    void showSaveSuccessTip();
    void showSaveFailedTip();
    void showSaveTip(const QString &message);

    CfgPillSwitch          *m_scanBarcodeSwitch       = nullptr;
    CfgPillSwitch          *m_saveRawImageSwitch      = nullptr;
    CfgPillSwitch          *m_saveResultImageSwitch   = nullptr;
    CfgPillSwitch          *m_saveAlgoDataSwitch      = nullptr;
    CfgPillSwitch          *m_algoFailAlarmSwitch     = nullptr;
    CfgPillSwitch          *m_upstreamSwitch          = nullptr;
    CfgPillSwitch          *m_downstreamSwitch        = nullptr;
    QVector<DetectRowSwitches> m_detectRowSwitches;
    QLineEdit              *m_imagePathEdit           = nullptr;
    QLineEdit              *m_resultPathEdit          = nullptr;
    QLineEdit              *m_thirdPartyPathEdit      = nullptr;
    QSpinBox               *m_lightDelaySpin           = nullptr;
    QSpinBox               *m_switchDelaySpin         = nullptr;
    QSpinBox               *m_maxUseSpin              = nullptr;
    QSpinBox               *m_curUseSpin              = nullptr;
    QVector<ThresholdSpinPair> m_thresholdSpins;
    QScrollArea            *m_scroll     = nullptr;
    QWidget                *m_mainPanel  = nullptr;
    SystemConfigViewModel  *m_viewModel  = nullptr;
    QPointer<QDialog>       m_saveSuccessTipDlg;
};

} // namespace lighten
