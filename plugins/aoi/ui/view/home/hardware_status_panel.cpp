/**
 * @file hardware_status_panel.cpp
 * @brief 硬件连接状态面板：行数随 runtime.hardware 动态伸缩，内容由 binding 写入
 */

#include "home/hardware_status_panel.h"

#include "component/controls/label/status_indicator_row.h"
#include "component/group/themed_section_panel.h"

#include <QVBoxLayout>

namespace aoi {

using namespace ui;

namespace {

QWidget *makeTransparentHost(QWidget *parent)
{
    auto *host = new QWidget(parent);
    host->setAttribute(Qt::WA_StyledBackground, true);
    host->setAutoFillBackground(false);
    host->setStyleSheet(QStringLiteral("background: transparent;"));
    return host;
}

} // namespace

HardwareStatusPanel::HardwareStatusPanel(QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("hardwareStatusPanel"));
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QStringLiteral("QWidget#hardwareStatusPanel { background: transparent; }"));
    buildLayout();
    retranslateUi();
}

void HardwareStatusPanel::buildLayout()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_panel = new ui::ThemedSectionPanel(QStringLiteral("aoi.sidebar.hardware_status"), this);
    ensureRowLayout();
    m_panel->contentLayout()->addWidget(m_rowsHost);
    layout->addWidget(m_panel);
}

void HardwareStatusPanel::ensureRowLayout()
{
    if (m_rowsHost)
        return;

    m_rowsHost   = makeTransparentHost(m_panel->contentHost());
    m_rowsLayout = new QVBoxLayout(m_rowsHost);
    m_rowsLayout->setContentsMargins(0, 0, 0, 0);
    m_rowsLayout->setSpacing(4);
}

void HardwareStatusPanel::syncRowCount(int count)
{
    ensureRowLayout();
    count = qMax(0, count);

    while (m_hwRows.size() > count) {
        ui::StatusIndicatorRow *row = m_hwRows.takeLast();
        m_rowsLayout->removeWidget(row);
        row->deleteLater();
    }

    while (m_hwRows.size() < count) {
        auto *row = new ui::StatusIndicatorRow(m_rowsHost);
        m_hwRows.append(row);
        m_rowsLayout->addWidget(row);
    }
}

void HardwareStatusPanel::retranslateUi()
{
    if (m_panel)
        m_panel->retranslatePanel();
}

} // namespace aoi
