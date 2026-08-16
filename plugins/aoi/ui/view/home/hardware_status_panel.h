#pragma once

/**
 * @file hardware_status_panel.h
 * @brief 硬件连接状态面板（行数与 runtime.hardware 同步，内容由 binding 写入）
 */

#include <QVector>
#include <QWidget>

namespace ui {

class StatusIndicatorRow;
class ThemedSectionPanel;

} // namespace ui

class QVBoxLayout;

namespace aoi {

class HardwareStatusPanel : public QWidget
{
    Q_OBJECT

public:
    explicit HardwareStatusPanel(QWidget *parent = nullptr);

    void retranslateUi();

    /** 与 runtime.hardware 行数对齐，动态增删 StatusIndicatorRow */
    void syncRowCount(int count);

    QVector<ui::StatusIndicatorRow *> hardwareRows() const { return m_hwRows; }

private:
    void buildLayout();
    void ensureRowLayout();

    ui::ThemedSectionPanel            *m_panel      = nullptr;
    QWidget                           *m_rowsHost   = nullptr;
    QVBoxLayout                       *m_rowsLayout = nullptr;
    QVector<ui::StatusIndicatorRow *>  m_hwRows;
};

} // namespace aoi
