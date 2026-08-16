#pragma once

#include <QImage>
#include <QMap>
#include <QObject>
#include "interface/igraphicsView.h"

class GraphicsWidget;
class QWidget;

/**
 * @brief IGraphicsVew 接口实现，向对象池提供图像显示控件
 */
class GraphicsViewImpl : public QObject, public IGraphicsVew
{
    Q_OBJECT

public:
    explicit GraphicsViewImpl();
    ~GraphicsViewImpl() override = default;

    QWidget *CreateWidget(int winId) override;
    bool SetVisible(int winId, bool bStatus, bool bDraw) override;

    void SetImage(int winId, QImage image) override;
    bool GetImage(int winId, QImage &out) const override;
    void Reset(int winId) override;
    void DrawRect(int winId) override;
    void DrawRotRect(int winId) override;
    void DrawCircle(int winId) override;
    void DrawRing(int winId) override;
    void DrawPolygon(int winId) override;
    void DrawCross(int winId) override;
    void DrawClean(int winId) override;
    void SetRoi(int winId, E_ItemType itemType, const QList<double> &roi) override;
    bool GetRoi(int winId, E_ItemType &itemType, QList<double> &roi) override;
    bool GetRoiRect(int winId, QRect &out) const override;
    void SetPixelScaleUmPerPx(int winId, double umPerPx) override;
    void SetDefectCrosshair(int winId, double imageX, double imageY) override;
    void ClearDefectCrosshair(int winId) override;

private:
    GraphicsWidget *graphicsWidget(int winId) const;

    QMap<int, QWidget *> m_widgets;
};
