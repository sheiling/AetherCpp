#pragma once

#include "io_point_widget.h"

namespace motion {

class RecipePillSwitch;

/** 输出 IO：可操作的胶囊开关 */
class IoOutputPointWidget : public IoPointWidget
{
    Q_OBJECT

public:
    explicit IoOutputPointWidget(const QString &title, QWidget *parent = nullptr);

    void setActiveState(bool active) override;

private:
    RecipePillSwitch *m_onOffSwitch = nullptr;
};

} // namespace motion
