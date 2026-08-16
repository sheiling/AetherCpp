#pragma once

/**
 * @file panel_virtual_map_widget.h
 * @brief 主页「灯板虚拟图」：灯板组网格 + FOV 分区、采集进度与滑动高亮
 *
 * 数据来自 `PanelVirtualMapState`（HomeRuntimeState.virtualMap），
 * 当前 FOV 矩形在归一化坐标 [0,1] 内插值动画；采集中（inspecting）即时跳变以免跟不上采集节拍。
 */

#include "vm/home/home_runtime_state.h"

#include <QWidget>

class QLabel;
class QTimer;

namespace aoi {

/** 仅负责绘图的灯板虚拟图区域（细网格 / FOV 分区 / 滑动矩形） */
class PanelVirtualMapCanvas : public QWidget
{
    Q_OBJECT

public:
    explicit PanelVirtualMapCanvas(QWidget *parent = nullptr);

    void applyState(const PanelVirtualMapState &state);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void startFocusAnimation(const QRectF &targetNormRect);
    void onAnimationTick();
    QRectF mapRect() const;
    QRectF normRectToPixel(const QRectF &normRect) const;
    int cellKey(int row, int col) const;

    PanelVirtualMapState m_state;
    QRectF               m_displayFocusNorm;
    QRectF               m_animFromNorm;
    QRectF               m_animToNorm;
    qreal                m_animProgress = 1.0;
    QTimer              *m_animTimer    = nullptr;
};

/** 灯板虚拟图：右上角轨道 / 状态 / 进度 + 画布 */
class PanelVirtualMapWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PanelVirtualMapWidget(QWidget *parent = nullptr);

    void applyState(const PanelVirtualMapState &state);
    void retranslateUi();

private:
    void rebuildHeaderText(const PanelVirtualMapState &state);

    PanelVirtualMapCanvas *m_canvas     = nullptr;
    QLabel                *m_headerLabel = nullptr;
    PanelVirtualMapState   m_lastState;
};

} // namespace aoi
