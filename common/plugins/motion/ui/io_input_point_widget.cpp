#include "io_input_point_widget.h"

#include <QLabel>
#include <QPainter>

namespace motion {

namespace {

constexpr int kIoStateDotSize = 14;

} // namespace

class IoInputPointWidget::IoStateIndicatorLabel final : public QLabel
{
public:
    explicit IoStateIndicatorLabel(QWidget *parent = nullptr)
        : QLabel(parent)
    {
        setFixedSize(kIoStateDotSize, kIoStateDotSize);
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        setAlignment(Qt::AlignCenter);
        setToolTip(QStringLiteral("输入状态"));
    }

    void setActive(bool active)
    {
        if (m_active == active)
            return;
        m_active = active;
        update();
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event);
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setPen(Qt::NoPen);
        p.setBrush(m_active ? QColor(0x4c, 0xaf, 0x50) : QColor(0xe5, 0x39, 0x35));
        const int d = qMin(width(), height());
        const int x = (width() - d) / 2;
        const int y = (height() - d) / 2;
        p.drawEllipse(x, y, d, d);
    }

private:
    bool m_active = false;
};

IoInputPointWidget::IoInputPointWidget(const QString &title, QWidget *parent)
    : IoPointWidget(title, false, parent)
{
    m_stateIndicator = new IoStateIndicatorLabel(this);
    insertStateWidget(m_stateIndicator);
}

void IoInputPointWidget::setActiveState(bool active)
{
    if (m_stateIndicator)
        m_stateIndicator->setActive(active);
}

} // namespace motion
