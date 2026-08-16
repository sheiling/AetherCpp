#include "io_output_point_widget.h"

#include "motion_control_helper.h"
#include "recipe_pill_switch.h"

namespace motion {

IoOutputPointWidget::IoOutputPointWidget(const QString &title, QWidget *parent)
    : IoPointWidget(title, true, parent)
{
    auto *sw = new RecipePillSwitch(true, this);
    sw->setToolTip(QStringLiteral("打开 / 关闭"));
    sw->setOnToggled([this](bool checked) {
        writeIo(m_title, !checked, "[IoOutputPointWidget]");
    });
    m_onOffSwitch = sw;
    insertStateWidget(m_onOffSwitch);
}

void IoOutputPointWidget::setActiveState(bool active)
{
    if (m_onOffSwitch)
        m_onOffSwitch->setChecked(active);
}

} // namespace motion
