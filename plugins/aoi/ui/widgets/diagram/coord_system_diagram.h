#pragma once

/**
 * @file coord_system_diagram.h
 * @brief 坐标系示意图（通用 diagram 控件，ComboBox index 0..3 对应四种原点朝向）
 *
 * 在输入区风格画布上绘制 X/Y 轴箭头，原点位于四角之一。
 * index 与工艺配置中「坐标系方向」下拉选项一一对应。
 */

#include <QWidget>

namespace aoi {

/**
 * 坐标系方向示意图。
 *
 * coordIndex 映射：
 *   0 — 左下角原点，X 向右，Y 向上
 *   1 — 右下角原点，X 向左，Y 向上
 *   2 — 左上角原点，X 向右，Y 向下
 *   3 — 右上角原点，X 向左，Y 向下
 */
class CoordSystemDiagram final : public QWidget
{
    Q_OBJECT

public:
    explicit CoordSystemDiagram(QWidget *parent = nullptr);

    /** 设置坐标系索引并触发重绘；idx 会被 clamp 到 [0, 3] */
    void setCoordIndex(int idx);

    /** @return 当前坐标系索引 */
    int  coordIndex() const { return m_idx; }

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    /** 订阅 ThemeManager 变更，主题切换时 update() */
    void bindTheme();

    int m_idx = 0;  ///< 当前坐标系规格索引（0..3）
};

} // namespace aoi
