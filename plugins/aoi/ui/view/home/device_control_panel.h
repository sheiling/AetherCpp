#pragma once

/**
 * @file device_control_panel.h
 * @brief 设备控制按钮面板
 */

#include <QWidget>

namespace ui {

class ThemedCyberPillButton;
class ThemedSectionPanel;

} // namespace ui

namespace aoi {

/**
 * @brief 设备控制按钮区：主控行（启动/暂停/整机复位）+ 辅助行（清除报警/清料/单次检测）
 */
class DeviceControlPanel : public QWidget
{
    Q_OBJECT

public:
    explicit DeviceControlPanel(QWidget *parent = nullptr);

    void retranslateUi();
    void applyPanelStyle();

    /**
     * 设备控制按钮，顺序：
     * 0 启动 | 1 暂停 | 2 整机复位 | 3 清除报警 | 4 清料 | 5 单次检测
     */
    const QVector<ui::ThemedCyberPillButton *> &controlButtons() const { return m_ctrlBtns; }

private:
    void buildLayout();
    ui::ThemedCyberPillButton *addCtrlButton(QWidget *parent, int pillHeight, int minWidth, int hPad);

    ui::ThemedSectionPanel              *m_ctrlPanel   = nullptr;
    QWidget                             *m_primaryHost = nullptr;
    QWidget                             *m_auxHost     = nullptr;
    QVector<ui::ThemedCyberPillButton *> m_ctrlBtns;
};

} // namespace aoi
