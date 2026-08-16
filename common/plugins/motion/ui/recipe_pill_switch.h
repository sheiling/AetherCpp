#pragma once

#include <QWidget>

#include <functional>

namespace motion {

/** 与 lighten RecipePillSwitch 一致的胶囊开关 */
class RecipePillSwitch final : public QWidget
{
public:
    explicit RecipePillSwitch(bool checked, QWidget *parent = nullptr, int trackHeight = 22);

    bool isChecked() const;
    void setChecked(bool on);
    void setOnToggled(std::function<void(bool)> handler);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    bool                       m_checked = false;
    int                        m_trackH   = 22;
    std::function<void(bool)> m_onToggled;
};

} // namespace motion
