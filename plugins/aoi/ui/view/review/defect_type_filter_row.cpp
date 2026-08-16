/**
 * @file defect_type_filter_row.cpp
 */

#include "review/defect_type_filter_row.h"

#include "component/theme/theme_tokens.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>

namespace aoi::review {

DefectTypeFilterRow::DefectTypeFilterRow(const QColor &swatchColor, QWidget *parent, bool showSwatch)
    : QWidget(parent)
    , m_swatchColor(swatchColor)
    , m_showSwatch(showSwatch)
{
    setCursor(Qt::PointingHandCursor);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    setFixedHeight(22);

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(2, 0, 2, 0);
    layout->setSpacing(6);

    m_checkIndicator = new QLabel(this);
    m_checkIndicator->setFixedSize(16, 16);
    m_checkIndicator->setAlignment(Qt::AlignCenter);

    m_swatch = new QLabel(this);
    m_swatch->setFixedSize(12, 12);
    if (m_showSwatch) {
        m_swatch->setStyleSheet(QStringLiteral(
            "background:%1; border-radius:2px; border:1px solid rgba(255,255,255,0.12);")
                                    .arg(swatchColor.name(QColor::HexRgb)));
    } else {
        m_swatch->hide();
    }

    m_label = new QLabel(this);
    m_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    layout->addWidget(m_checkIndicator, 0, Qt::AlignVCenter);
    if (m_showSwatch) {
        layout->addWidget(m_swatch, 0, Qt::AlignVCenter);
    } else {
        auto *alignSpacer = new QWidget(this);
        alignSpacer->setFixedSize(12, 12);
        layout->addWidget(alignSpacer, 0, Qt::AlignVCenter);
    }
    layout->addWidget(m_label, 1, Qt::AlignVCenter);

    updateCheckIndicator();
}

void DefectTypeFilterRow::setLabelText(const QString &text)
{
    m_label->setText(text);
}

void DefectTypeFilterRow::setChecked(bool checked)
{
    setCheckState(checked ? Qt::Checked : Qt::Unchecked);
}

void DefectTypeFilterRow::setCheckState(Qt::CheckState state)
{
    if (m_checkState == state)
        return;
    m_checkState = state;
    m_checked    = state == Qt::Checked;
    updateCheckIndicator();
}

void DefectTypeFilterRow::setEnabled(bool enabled)
{
    m_enabled = enabled;
    setCursor(enabled ? Qt::PointingHandCursor : Qt::ArrowCursor);

    const ui::ThemeTokens t = ui::ThemeTokens::resolve();
    if (!enabled)
        m_label->setStyleSheet(QStringLiteral("color: %1;")
                                   .arg(t.textSecondary.name(QColor::HexRgb)));
    else
        m_label->setStyleSheet(QString());

    updateCheckIndicator();
}

bool DefectTypeFilterRow::isChecked() const
{
    return m_checked;
}

Qt::CheckState DefectTypeFilterRow::checkState() const
{
    return m_checkState;
}

void DefectTypeFilterRow::toggle()
{
    if (!m_enabled)
        return;
  const bool nextChecked = m_checkState != Qt::Checked;
    setCheckState(nextChecked ? Qt::Checked : Qt::Unchecked);
    emit toggled(nextChecked);
}

void DefectTypeFilterRow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        toggle();
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void DefectTypeFilterRow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
}

void DefectTypeFilterRow::updateCheckIndicator()
{
    const ui::ThemeTokens t = ui::ThemeTokens::resolve();
    const QColor          border =
        m_enabled ? (t.surfaceBorder.isValid() ? t.surfaceBorder : QColor(0x64, 0x96, 0xff))
                  : QColor(0x47, 0x55, 0x69);
    const QColor bg = m_enabled ? t.inputBackground : t.surface;

    if (m_checkState == Qt::Checked) {
        m_checkIndicator->setText(QStringLiteral("☑"));
        m_checkIndicator->setStyleSheet(QStringLiteral(
            "color: %1; font-size: 14px; background: %2; border: 1px solid %3; border-radius: 2px;")
                                            .arg(t.accent.name(QColor::HexRgb),
                                                 bg.name(QColor::HexRgb),
                                                 border.name(QColor::HexRgb)));
    } else if (m_checkState == Qt::PartiallyChecked) {
        m_checkIndicator->setText(QStringLiteral("−"));
        m_checkIndicator->setStyleSheet(QStringLiteral(
            "color: %1; font-size: 12px; font-weight: bold; background: %2; border: 1px solid %3;"
            " border-radius: 2px;")
                                            .arg(t.accent.name(QColor::HexRgb),
                                                 bg.name(QColor::HexRgb),
                                                 border.name(QColor::HexRgb)));
    } else {
        m_checkIndicator->setText(QString());
        m_checkIndicator->setStyleSheet(QStringLiteral(
            "background: %1; border: 1px solid %2; border-radius: 2px;")
                                            .arg(bg.name(QColor::HexRgb), border.name(QColor::HexRgb)));
    }
}

} // namespace aoi::review
