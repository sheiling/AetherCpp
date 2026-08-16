#pragma once

/**
 * @file pill_switch.h
 * @brief 胶囊开关：自绘圆角轨道 + 滑块，颜色跟随 ThemeTokens
 *
 * 自绘 Token 映射：
 * - 开：switchTrackOn, switchBorderOn
 * - 关：switchTrackOff, switchBorderOff
 * - 滑块：switchThumb
 */

#include <QWidget>
#include "component_global.h"

namespace ui {

/** 52×26 胶囊 toggle，点击切换并 emit toggled */
class UI_COMPONENT_EXPORT PillSwitch final : public QWidget
{
    Q_OBJECT

public:
    explicit PillSwitch(bool checked, QWidget *parent = nullptr);

    bool isChecked() const;
    /** 设置开关状态并 emit toggled（与点击行为一致） */
    void setChecked(bool on);

signals:
    void toggled(bool checked);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    bool m_checked;
};

/** 工厂：创建指定初始状态的 PillSwitch */
UI_COMPONENT_EXPORT PillSwitch *makePillSwitch(bool checked, QWidget *parent = nullptr);

} // namespace ui
