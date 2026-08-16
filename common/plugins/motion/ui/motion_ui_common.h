#pragma once

class QLabel;
class QLineEdit;
class QRadioButton;
class QString;

namespace motion {

constexpr int kMotionIoFormLabelWidth   = 56;
constexpr int kMotionIoFormRowSpacing   = 4;
constexpr int kMotionIoFormRowHeight    = 26;
constexpr int kMotionIoFormRadioSpacing = 16;
constexpr int kMotionIoFormBottomSpacing = 5;
constexpr int kMotionIoSectionSpacing   = 6;

QString formatIoPointPortHeader(bool isOutput, int port);
void    styleMotionIoFormLabel(QLabel *label);
int     calcMotionIoFormControlWidth(const QRadioButton *lowActive,
                                     const QRadioButton *highActive,
                                     const QLineEdit    *portEdit);

} // namespace motion
