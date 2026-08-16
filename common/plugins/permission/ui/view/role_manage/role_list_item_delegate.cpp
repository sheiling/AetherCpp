#include "ui/view/role_manage/role_list_item_delegate.h"

#include "theme/theme_color_math.h"
#include "theme/theme_tokens.h"

#include <QAbstractItemView>
#include <QFontMetrics>
#include <QHelpEvent>
#include <QPainter>
#include <QPainterPath>
#include <QTextLayout>
#include <QTextLine>
#include <QToolTip>

namespace {

constexpr int kMinItemHeight = 72;
constexpr int kCardRadius    = 8;
constexpr int kCardMarginV   = 4;
constexpr int kPadH          = 14;
constexpr int kPadV          = 12;
constexpr int kTitleDescGap  = 8;
constexpr int kTitlePx       = 14;
constexpr int kDescPx        = 12;
constexpr int kDescMaxLines  = 2;

int contentWidth(const QStyleOptionViewItem &option)
{
    int w = option.rect.width();
    if (w <= 0 && option.widget)
        w = option.widget->width();
    return qMax(0, w - 2 * kPadH);
}

struct DescLayoutMetrics {
    QString displayText;
    int     height     = 0;
    bool    truncated = false;
};

/// 用 QTextLayout 计算断行，再拼成最多 maxLines 行的展示文本，由 drawText 统一绘制（避免 QTextLine::draw 坐标问题）
DescLayoutMetrics buildDescriptionLayout(const QFont &font, const QString &text, int width,
                                         int maxLines)
{
    DescLayoutMetrics metrics;
    if (text.isEmpty() || maxLines <= 0 || width <= 0)
        return metrics;

    const QFontMetrics fm(font);

    QTextLayout layout(text);
    layout.setFont(font);
    QTextOption textOption;
    textOption.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    layout.setTextOption(textOption);

    layout.beginLayout();
    QVector<QTextLine> lines;
    for (int i = 0; i < maxLines + 1; ++i) {
        QTextLine line = layout.createLine();
        if (!line.isValid())
            break;
        line.setLineWidth(static_cast<qreal>(width));
        lines.append(line);
    }
    layout.endLayout();

    if (lines.isEmpty())
        return metrics;

    metrics.truncated = lines.size() > maxLines;
    if (metrics.truncated)
        lines.resize(maxLines);

    auto lineText = [&text](const QTextLine &line) {
        return text.mid(line.textStart(), line.textLength());
    };

    if (lines.size() == 1) {
        metrics.displayText = lineText(lines.first());
    } else {
        const QString first = lineText(lines.at(0));
        if (!metrics.truncated) {
            metrics.displayText = first + QLatin1Char('\n') + lineText(lines.at(1));
        } else {
            const QString rest = text.mid(lines.at(1).textStart());
            metrics.displayText =
                first + QLatin1Char('\n')
                + fm.elidedText(rest, Qt::ElideRight, qMax(0, width));
        }
    }

    const QRect bounds =
        fm.boundingRect(0, 0, width, INT_MAX, Qt::TextWordWrap | Qt::AlignTop, metrics.displayText);
    metrics.height = bounds.height();
    return metrics;
}

DescLayoutMetrics measureDescription(const QFont &font, const QString &text, int width,
                                     int maxLines)
{
    return buildDescriptionLayout(font, text, width, maxLines);
}

void paintDescription(QPainter *painter, const QRect &rect, const QString &text,
                      const QFont &font, int maxLines, bool *truncatedOut)
{
    const DescLayoutMetrics metrics =
        buildDescriptionLayout(font, text, rect.width(), maxLines);

    if (metrics.displayText.isEmpty()) {
        if (truncatedOut)
            *truncatedOut = false;
        return;
    }

    painter->setFont(font);
    painter->drawText(rect, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap,
                      metrics.displayText);

    if (truncatedOut)
        *truncatedOut = metrics.truncated;
}

} // namespace

RoleListItemDelegate::RoleListItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QSize RoleListItemDelegate::sizeHint(const QStyleOptionViewItem &option,
                                     const QModelIndex &index) const
{
    const int width = qMax(option.rect.width(), 1);
    const int textW = contentWidth(option);

    QFont titleFont = option.font;
    titleFont.setPixelSize(kTitlePx);
    titleFont.setWeight(QFont::DemiBold);

    QFont descFont = option.font;
    descFont.setPixelSize(kDescPx);
    descFont.setWeight(QFont::Normal);

    const QFontMetrics titleFm(titleFont);
    const QFontMetrics descFm(descFont);

    const QString desc = index.data(RoleDescriptionRole).toString();
    const DescLayoutMetrics descMetrics =
        measureDescription(descFont, desc, textW, kDescMaxLines);

    int h = kCardMarginV * 2 + kPadV * 2 + titleFm.height();
    if (!desc.isEmpty())
        h += kTitleDescGap + descMetrics.height;

    return QSize(width, qMax(kMinItemHeight, h));
}

void RoleListItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                 const QModelIndex &index) const
{
    if (!index.isValid())
        return;

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    const ui::ThemeTokens t = ui::ThemeTokens::resolve();
    const bool selected = option.state.testFlag(QStyle::State_Selected);

    QRect cardRect = option.rect;
    cardRect.adjust(0, kCardMarginV, 0, -kCardMarginV);

    QPainterPath cardPath;
    cardPath.addRoundedRect(cardRect, kCardRadius, kCardRadius);

    if (selected) {
        painter->fillPath(cardPath, ui::themeWithAlpha(t.accent, 90));
        QPen borderPen(t.accent, 1.5);
        painter->setPen(borderPen);
        painter->drawPath(cardPath);
    } else {
        painter->fillPath(cardPath, t.surfaceElevated);
        QPen borderPen(t.surfaceBorder, 1.0);
        painter->setPen(borderPen);
        painter->drawPath(cardPath);
    }

    const QString name = index.data(Qt::DisplayRole).toString();
    const QString desc = index.data(RoleDescriptionRole).toString();

    QRect textRect = cardRect.adjusted(kPadH, kPadV, -kPadH, -kPadV);

    QFont titleFont = option.font;
    titleFont.setPixelSize(kTitlePx);
    titleFont.setWeight(QFont::DemiBold);

    QFont descFont = option.font;
    descFont.setPixelSize(kDescPx);
    descFont.setWeight(QFont::Normal);

    const QFontMetrics titleFm(titleFont);
    const DescLayoutMetrics descMetrics =
        measureDescription(descFont, desc, textRect.width(), kDescMaxLines);

    QRect titleRect(textRect.left(), textRect.top(), textRect.width(), titleFm.height());
    QRect descRect(textRect.left(), titleRect.bottom() + kTitleDescGap, textRect.width(),
                   qMax(descMetrics.height, QFontMetrics(descFont).lineSpacing() * kDescMaxLines));

    painter->setFont(titleFont);
    painter->setPen(t.textPrimary);
    const QString elidedName =
        titleFm.elidedText(name, Qt::ElideRight, qMax(0, titleRect.width()));
    painter->drawText(titleRect, Qt::AlignLeft | Qt::AlignVCenter, elidedName);

    if (!desc.isEmpty()) {
        painter->setPen(t.textSecondary);
        bool truncated = false;
        paintDescription(painter, descRect, desc, descFont, kDescMaxLines, &truncated);
        Q_UNUSED(truncated);
    }

    painter->restore();
}

bool RoleListItemDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view,
                                     const QStyleOptionViewItem &option,
                                     const QModelIndex &index)
{
    if (!event || !view || !index.isValid())
        return QStyledItemDelegate::helpEvent(event, view, option, index);

    if (event->type() != QEvent::ToolTip)
        return QStyledItemDelegate::helpEvent(event, view, option, index);

    if (!option.rect.contains(event->pos())) {
        QToolTip::hideText();
        return true;
    }

    const QString name = index.data(Qt::DisplayRole).toString();
    const QString desc = index.data(RoleDescriptionRole).toString();

    QFont descFont = option.font;
    descFont.setPixelSize(kDescPx);
    QFont titleFont = option.font;
    titleFont.setPixelSize(kTitlePx);
    titleFont.setWeight(QFont::DemiBold);
    const QFontMetrics titleFm(titleFont);

    const bool descTruncated =
        measureDescription(descFont, desc, contentWidth(option), kDescMaxLines).truncated;

    const bool titleTruncated =
        titleFm.elidedText(name, Qt::ElideRight, contentWidth(option)) != name;

    if (!titleTruncated && !descTruncated) {
        QToolTip::hideText();
        return true;
    }

    QString tip = name;
    if (!desc.isEmpty()) {
        if (!tip.isEmpty())
            tip += QStringLiteral("\n");
        tip += desc;
    }

    QToolTip::showText(event->globalPos(), tip, view);
    return true;
}
