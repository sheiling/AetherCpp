#pragma once

#include "io_point_widget.h"

namespace motion {

/** 输入 IO：只读状态指示（绿 / 红实心圆点） */
class IoInputPointWidget : public IoPointWidget
{
    Q_OBJECT

public:
    explicit IoInputPointWidget(const QString &title, QWidget *parent = nullptr);

    void setActiveState(bool active) override;

private:
    class IoStateIndicatorLabel;
    IoStateIndicatorLabel *m_stateIndicator = nullptr;
};

} // namespace motion
