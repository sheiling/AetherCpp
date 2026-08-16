#pragma once

/**
 * @file recipe_mark_calc_view_model.h
 * @brief 配方配置页 Mark 定位计算子 ViewModel
 */

#include "service/vision_bridge/mark_point_locator.h"

#include <QImage>
#include <QObject>
#include <QVector>

namespace aoi {

class RecipeConfigViewModel;

/**
 * @brief Mark 点定位：预览区当前帧 → MarkPointLocator → 按槽位缓存结果
 *
 * 取图由 `RecipeConfigMarkView` 负责：实时预览时先 `stopPreview(false)` 再
 * `IGraphicsVew::GetImage`；SDK/单元可用性由 `MarkPointLocator::locate` 统一校验。
 */
class RecipeMarkCalcViewModel : public QObject
{
    Q_OBJECT

public:
    explicit RecipeMarkCalcViewModel(RecipeConfigViewModel *host, QObject *parent = nullptr);

    bool                           markCalculating() const { return m_markCalculating; }
    MarkLocateResult               markLocateResult() const { return m_markLocateResult; }
    MarkLocateResult               markLocateResultForSlot(int slot) const;
    int                            markLocateSlot() const { return m_markLocateSlot; }
    int                            markSlotCount() const;

    void clearLocateCache();

public slots:
    void runMarkCalcFromImage(const QImage &image, int markSlotIndex);

signals:
    void markCalculatingChanged();
    void markLocateResultChanged();

private:
    void setMarkCalculating(bool calculating);
    void finishMarkCalc(const MarkLocateResult &result);

    RecipeConfigViewModel    *m_host = nullptr;
    bool                      m_markCalculating = false;
    MarkLocateResult          m_markLocateResult;
    QVector<MarkLocateResult> m_markLocateCache;
    int                       m_markLocateSlot = 0;
};

} // namespace aoi
