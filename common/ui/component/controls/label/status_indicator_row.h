#pragma once

/**
 * @file status_indicator_row.h
 * @brief 连接状态行：玻璃指示灯 + 名称/次级状态 + 主状态文字
 *
 * 样式见 status_indicator_row_style.h；指示灯色随 LinkIndicatorState。
 */

#include <QWidget>
#include "component_global.h"

class QLabel;

namespace ui {

class ThemedGlassIndicatorDot;

/** 链路/设备连接语义状态 */
enum class LinkIndicatorState {
    Connected,
    Standby,
    Disconnected,
    Unknown
};

/** 硬件状态行（#statusIndicatorRow） */
class UI_COMPONENT_EXPORT StatusIndicatorRow : public QWidget
{
    Q_OBJECT

public:
    explicit StatusIndicatorRow(QWidget *parent = nullptr);

    void setName(const QString &name);
    void setStatusText(const QString &text);
    /** 次级活动状态（采图中、空闲中等）；空则隐藏 */
    void setSubStatusText(const QString &text);
    void setIndicatorState(LinkIndicatorState state);
    void setIndicatorDiameter(int pixels);

private:
    void bindTheme();
    void refreshStyle();
    QColor indicatorColorForState(LinkIndicatorState state) const;

    ThemedGlassIndicatorDot *m_dot        = nullptr;
    QLabel                    *m_nameLbl   = nullptr;
    QLabel                    *m_subLbl    = nullptr;
    QLabel                    *m_statusLbl = nullptr;
    LinkIndicatorState         m_state     = LinkIndicatorState::Unknown;
};

} // namespace ui
