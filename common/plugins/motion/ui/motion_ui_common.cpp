#include "motion_ui_common.h"

#include <QLabel>
#include <QLineEdit>
#include <QFontMetrics>
#include <QRadioButton>

namespace motion {

QString formatIoPointPortHeader(bool isOutput, int port)
{
    QString portStr = QString::number(port);
    if (portStr.size() == 1)
        portStr = QStringLiteral(" ") + portStr;
    return (isOutput ? QStringLiteral("DO:") : QStringLiteral("DI:")) + portStr;
}

void styleMotionIoFormLabel(QLabel *label)
{
    label->setObjectName(QStringLiteral("motionIoFormLabel"));
    label->setFixedSize(kMotionIoFormLabelWidth, kMotionIoFormRowHeight);
    label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

int calcMotionIoFormControlWidth(const QRadioButton *lowActive,
                                 const QRadioButton *highActive,
                                 const QLineEdit    *portEdit)
{
    auto *low  = const_cast<QRadioButton *>(lowActive);
    auto *high = const_cast<QRadioButton *>(highActive);
    auto *edit = const_cast<QLineEdit *>(portEdit);
    low->ensurePolished();
    high->ensurePolished();
    edit->ensurePolished();

    const int radioRowW = low->sizeHint().width() + kMotionIoFormRadioSpacing + high->sizeHint().width();

    const QFontMetrics fm(edit->font());
    constexpr int kPortEditHPadding = 12;
    const int portMinW = fm.horizontalAdvance(QStringLiteral("999")) + kPortEditHPadding;

    return qMax(radioRowW, portMinW);
}

} // namespace motion
