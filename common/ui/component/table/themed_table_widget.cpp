/**
 * @file themed_table_widget.cpp
 * @brief ThemedTableWidget 主题 QSS、霓虹表头自绘、紧凑表单预设与列宽比例分配
 */

#include "table/themed_table_widget.h"

#include "table/table_style.h"

#include "theme/theme_color_math.h"
#include "theme/theme_refresh.h"
#include "theme/theme_tokens.h"

#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QEvent>
#include <QHeaderView>
#include <QPainter>
#include <QResizeEvent>
#include <QShowEvent>
#include <QSizePolicy>
#include <QTableWidget>
#include <QtMath>

#include <functional>

namespace ui {

namespace {

void drawHeaderRadialFill(QPainter &p, const QRectF &bounds,
                          const std::function<void(QRadialGradient &)> &setupGradient,
                          QPainter::CompositionMode mode = QPainter::CompositionMode_SourceOver)
{
    if (bounds.width() < 1.0 || bounds.height() < 1.0)
        return;

    const QPointF c  = bounds.center();
    const qreal   rx = bounds.width() * 0.5;
    const qreal   ry = bounds.height() * 0.5;

    QRadialGradient grad(QPointF(0.0, 0.0), 1.0);
    setupGradient(grad);

    p.save();
    if (mode != QPainter::CompositionMode_SourceOver)
        p.setCompositionMode(mode);
    p.translate(c);
    p.scale(rx, ry);
    p.setPen(Qt::NoPen);
    p.setBrush(grad);
    p.drawEllipse(QRectF(-1.0, -1.0, 2.0, 2.0));
    p.restore();
}

/** 表头单格：轻量玻璃底 + 多层柔光 + 细线分隔 */
void paintTableHeaderSection(QPainter &p, const QRect &rect, const QString &text,
                             const ThemeTokens &tokens, bool isLast)
{
    if (!rect.isValid())
        return;

    const bool   lightTheme = tableThemeIsLight(tokens);
    const QColor accent     = tokens.accent;

    const QColor baseTop =
        themeBlend(themeBlend(tokens.headerBg, tokens.surfaceElevated, 0.28), tokens.accentHover,
                   lightTheme ? 0.06 : 0.10);
    const QColor baseUpper = themeBlend(tokens.headerBg, tokens.surfaceElevated, 0.20);
    const QColor baseLower = themeBlend(tokens.headerBg, tokens.inputBackground, 0.14);
    const QColor baseBot   = themeBlend(baseLower, tokens.surface, 0.18);

    QLinearGradient bgGrad(rect.topLeft(), rect.bottomLeft());
    bgGrad.setColorAt(0.0, baseTop);
    bgGrad.setColorAt(0.32, baseUpper);
    bgGrad.setColorAt(0.68, baseLower);
    bgGrad.setColorAt(1.0, baseBot);
    p.fillRect(rect, bgGrad);

    QLinearGradient horizSheen(rect.topLeft(), rect.topRight());
    horizSheen.setColorAt(0.0, themeWithAlpha(accent, lightTheme ? 10 : 16));
    horizSheen.setColorAt(0.22, QColor(0, 0, 0, 0));
    horizSheen.setColorAt(0.50, themeWithAlpha(Qt::white, lightTheme ? 14 : 22));
    horizSheen.setColorAt(0.78, QColor(0, 0, 0, 0));
    horizSheen.setColorAt(1.0, themeWithAlpha(accent, lightTheme ? 10 : 16));
    p.fillRect(rect, horizSheen);

    const int hiH = qMax(2, rect.height() / 4);
    QLinearGradient topSheen(QPointF(rect.left(), rect.top()),
                             QPointF(rect.left(), rect.top() + hiH));
    topSheen.setColorAt(0.0, themeWithAlpha(Qt::white, lightTheme ? 28 : 42));
    topSheen.setColorAt(0.65, themeWithAlpha(accent, lightTheme ? 8 : 14));
    topSheen.setColorAt(1.0, QColor(0, 0, 0, 0));
    p.fillRect(QRect(rect.left() + 1, rect.top(), qMax(0, rect.width() - 2), hiH), topSheen);

    const qreal midY = rect.top() + rect.height() * 0.48;
    p.setPen(themeWithAlpha(accent, lightTheme ? 16 : 24));
    p.drawLine(QPointF(rect.left() + 6.0, midY), QPointF(rect.right() - 6.0, midY));
    p.setPen(themeWithAlpha(Qt::white, lightTheme ? 10 : 16));
    p.drawLine(QPointF(rect.left() + 6.0, midY - 0.6), QPointF(rect.right() - 6.0, midY - 0.6));

    p.save();
    p.setClipRect(rect);

    const QPointF innerOrigin(rect.center().x(), rect.bottom() - rect.height() * 0.08);
    const qreal   innerRx = rect.width() * 0.46;
    const qreal   innerRy = rect.height() * 0.72;
    drawHeaderRadialFill(
        p, QRectF(innerOrigin.x() - innerRx, innerOrigin.y() - innerRy, innerRx * 2.0, innerRy),
        [&](QRadialGradient &g) {
            const int peak = lightTheme ? 38 : 58;
            g.setColorAt(0.0, themeWithAlpha(accent, peak));
            g.setColorAt(0.40, themeWithAlpha(accent, peak / 2));
            g.setColorAt(0.78, themeWithAlpha(accent, peak / 6));
            g.setColorAt(1.0, QColor(0, 0, 0, 0));
        },
        lightTheme ? QPainter::CompositionMode_SourceOver : QPainter::CompositionMode_Plus);

    const QRectF outerBloom(rect.center().x() - rect.width() * 0.38,
                            rect.bottom() - rect.height() * 0.22, rect.width() * 0.76,
                            rect.height() * 0.38);
    drawHeaderRadialFill(p, outerBloom, [&](QRadialGradient &g) {
        const int peak = lightTheme ? 22 : 32;
        g.setColorAt(0.0, themeWithAlpha(accent, peak));
        g.setColorAt(0.55, themeWithAlpha(accent, peak / 3));
        g.setColorAt(1.0, QColor(0, 0, 0, 0));
    });

    p.restore();

    const int rimH = qMax(2, rect.height() / 6);
    QRect     rimRect(rect.left(), rect.bottom() - rimH, rect.width(), rimH);
    QLinearGradient rimGrad(rimRect.topLeft(), rimRect.bottomLeft());
    rimGrad.setColorAt(0.0, QColor(0, 0, 0, 0));
    rimGrad.setColorAt(0.55, themeWithAlpha(accent, lightTheme ? 24 : 38));
    rimGrad.setColorAt(1.0, themeWithAlpha(accent, lightTheme ? 42 : 62));
    p.fillRect(rimRect, rimGrad);

    const int flareW = qMax(16, qRound(rect.width() * 0.44));
    QRect     flare(rect.center().x() - flareW / 2, rect.bottom() - 1, flareW, 1);
    QLinearGradient flareGrad(flare.topLeft(), flare.topRight());
    flareGrad.setColorAt(0.0, QColor(0, 0, 0, 0));
    flareGrad.setColorAt(0.50, themeWithAlpha(themeLighten(accent, 0.30), lightTheme ? 95 : 130));
    flareGrad.setColorAt(1.0, QColor(0, 0, 0, 0));
    p.fillRect(flare, flareGrad);

    if (!isLast) {
        QLinearGradient divGrad(QPointF(rect.right(), rect.top()),
                                QPointF(rect.right(), rect.bottom()));
        divGrad.setColorAt(0.0, themeWithAlpha(accent, lightTheme ? 14 : 22));
        divGrad.setColorAt(0.45, themeWithAlpha(accent, lightTheme ? 32 : 48));
        divGrad.setColorAt(1.0, themeWithAlpha(accent, lightTheme ? 16 : 24));
        QPen divPen;
        divPen.setBrush(divGrad);
        divPen.setWidthF(1.0);
        p.setPen(divPen);
        p.drawLine(rect.topRight(), rect.bottomRight());

        p.setPen(themeWithAlpha(Qt::white, lightTheme ? 8 : 12));
        p.drawLine(rect.topRight() + QPoint(-1, 0), rect.bottomRight() + QPoint(-1, 0));
    }

    p.setPen(themeWithAlpha(tokens.inputBorder, lightTheme ? 150 : 170));
    p.drawLine(rect.bottomLeft(), rect.bottomRight());

    QFont font = p.font();
    font.setBold(true);
    font.setPixelSize(12);
    p.setFont(font);

    QColor fg = lightTheme ? themeBlend(tokens.textPrimary, themeDarken(accent, 0.28), 0.22)
                           : themeBlend(tokens.highlightText, themeLighten(accent, 0.28), 0.32);

    if (!lightTheme && accent.isValid()) {
        p.setPen(themeWithAlpha(themeLighten(accent, 0.35), 48));
        p.drawText(rect.adjusted(0, 1, 0, 1), Qt::AlignCenter, text);
    }

    p.setPen(fg);
    p.drawText(rect, Qt::AlignCenter, text);
}

/** 霓虹渐变表头；section 背景由 paintSection 自绘 */
class ThemedTableHeaderView : public QHeaderView
{
public:
    explicit ThemedTableHeaderView(Qt::Orientation orientation, QWidget *parent = nullptr)
        : QHeaderView(orientation, parent)
    {
        setDefaultAlignment(Qt::AlignCenter);
        setHighlightSections(false);
        setStretchLastSection(false);
    }

protected:
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override
    {
        if (!painter || !rect.isValid() || !model())
            return;

        const QString     text =
            model()->headerData(logicalIndex, orientation(), Qt::DisplayRole).toString();
        const ThemeTokens tokens = ThemeTokens::resolve();

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setRenderHint(QPainter::TextAntialiasing, true);
        paintTableHeaderSection(*painter, rect, text, tokens, logicalIndex == count() - 1);
        painter->restore();
    }
};

void installThemedHorizontalHeader(QTableWidget *table)
{
    if (!table)
        return;
    auto *header = new ThemedTableHeaderView(Qt::Horizontal, table);
    table->setHorizontalHeader(header);
}

} // namespace

ThemedTableWidget::ThemedTableWidget(QWidget *parent)
    : QTableWidget(parent)
{
    installThemedHorizontalHeader(this);
    bindTheme();
    refreshStyle();
}

ThemedTableWidget::ThemedTableWidget(int rows, int columns, QWidget *parent)
    : QTableWidget(rows, columns, parent)
{
    installThemedHorizontalHeader(this);
    bindTheme();
    refreshStyle();
}

void ThemedTableWidget::bindTheme()
{
    bindThemeRefresh(this, [this]() { refreshStyle(); });
}

void ThemedTableWidget::refreshStyle()
{
    setStyleSheet(buildTableWidgetStylesheet());
    if (horizontalHeader())
        horizontalHeader()->update();
}

/** 只读、无滚动、无选中；表头居中并拉伸末列，隐藏行号 */
void ThemedTableWidget::applyCompactFormDefaults()
{
    m_hideScrollBarWhenFits = true;
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumWidth(0);
    setMinimumHeight(0);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setSelectionMode(QAbstractItemView::NoSelection);
    setFocusPolicy(Qt::NoFocus);
    setShowGrid(true);
    setCornerButtonEnabled(false);

    if (horizontalHeader()) {
        horizontalHeader()->setStretchLastSection(true);
        horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
        horizontalHeader()->setHighlightSections(false);
    }
    if (verticalHeader()) {
        verticalHeader()->setVisible(false);
        verticalHeader()->setFixedWidth(0);
        verticalHeader()->setMaximumWidth(0);
        verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    }

    m_autoDistributeRowHeight = true;
    clearAllRowHeightOverrides();
    updateRowHeightLayout();
}

void ThemedTableWidget::setHorizontalStretchFactors(const QList<int> &factors)
{
    m_hStretchFactors = factors;
    if (!horizontalHeader())
        return;

    horizontalHeader()->setStretchLastSection(false);
    for (int col = 0; col < columnCount(); ++col)
        horizontalHeader()->setSectionResizeMode(col, QHeaderView::Fixed);
    applyHorizontalStretchFactors();
}

void ThemedTableWidget::clearHorizontalStretchFactors()
{
    m_hStretchFactors.clear();
}

/** 按 m_hStretchFactors 权重分配 viewport 宽度，末列吃掉余量 */
void ThemedTableWidget::applyHorizontalStretchFactors()
{
    if (m_hStretchFactors.isEmpty() || !horizontalHeader())
        return;

    int sum = 0;
    for (int factor : m_hStretchFactors)
        sum += qMax(0, factor);
    if (sum <= 0)
        return;

    const int cols = qMin(columnCount(), m_hStretchFactors.size());
    const int w    = viewport() ? viewport()->width() : 0;
    if (w <= 0)
        return;

    int used = 0;
    for (int col = 0; col < cols - 1; ++col) {
        const int cw = w * m_hStretchFactors[col] / sum;
        setColumnWidth(col, cw);
        used += cw;
    }
    if (cols > 0)
        setColumnWidth(cols - 1, qMax(0, w - used));

    int widthSum = 0;
    for (int col = 0; col < cols; ++col)
        widthSum += columnWidth(col);
    if (cols > 0 && widthSum < w)
        setColumnWidth(cols - 1, columnWidth(cols - 1) + (w - widthSum));
}

void ThemedTableWidget::resizeHeightToContents()
{
    if (!horizontalHeader())
        return;

    m_autoDistributeRowHeight = false;

    int totalH = horizontalHeader()->height();
    for (int row = 0; row < rowCount(); ++row)
        totalH += rowHeight(row);

    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setFixedHeight(totalH + frameWidth() * 2);
}

void ThemedTableWidget::setRowHeight(int row, int height)
{
    if (row < 0 || row >= rowCount())
        return;

    m_explicitRowHeights.insert(row);
    QTableWidget::setRowHeight(row, height);

    if (m_autoDistributeRowHeight)
        updateRowHeightLayout();
}

void ThemedTableWidget::setMinimumRowHeight(int pixels)
{
    m_minRowHeight = qMax(1, pixels);
    if (verticalHeader())
        verticalHeader()->setDefaultSectionSize(m_minRowHeight);
    if (m_autoDistributeRowHeight)
        updateRowHeightLayout();
}

void ThemedTableWidget::setAutoRowHeightDistribution(bool enabled)
{
    m_autoDistributeRowHeight = enabled;
    if (enabled) {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        setMaximumHeight(QWIDGETSIZE_MAX);
        updateRowHeightLayout();
    }
}

void ThemedTableWidget::clearRowHeightOverride(int row)
{
    m_explicitRowHeights.remove(row);
    if (m_autoDistributeRowHeight)
        updateRowHeightLayout();
}

void ThemedTableWidget::clearAllRowHeightOverrides()
{
    m_explicitRowHeights.clear();
    if (m_autoDistributeRowHeight)
        updateRowHeightLayout();
}

void ThemedTableWidget::setRowCount(int rows)
{
    QTableWidget::setRowCount(rows);

    for (auto it = m_explicitRowHeights.begin(); it != m_explicitRowHeights.end();) {
        if (*it >= rows)
            it = m_explicitRowHeights.erase(it);
        else
            ++it;
    }

    if (m_autoDistributeRowHeight)
        updateRowHeightLayout();
}

int ThemedTableWidget::availableRowAreaHeight() const
{
    if (isVisible() && viewport()) {
        const int viewportH = viewport()->height();
        if (viewportH > 0)
            return viewportH;
    }

    int area = height() - frameWidth() * 2;
    if (horizontalHeader() && horizontalHeader()->isVisible())
        area -= horizontalHeader()->height();
    return qMax(0, area);
}

void ThemedTableWidget::updateRowHeightLayout()
{
    if (!m_autoDistributeRowHeight || !verticalHeader())
        return;

    const int rows = rowCount();
    if (rows <= 0)
        return;

    const int available = availableRowAreaHeight();
    if (available <= 0)
        return;

    int explicitSum = 0;
    int autoCount   = 0;
    for (int row = 0; row < rows; ++row) {
        if (m_explicitRowHeights.contains(row))
            explicitSum += QTableWidget::rowHeight(row);
        else
            ++autoCount;
    }

    const int minH   = m_minRowHeight;
    int       autoH  = minH;
    int       rowSlack = 0;
    if (autoCount > 0) {
        const int remaining = available - explicitSum;
        autoH               = qMax(minH, remaining / autoCount);
        rowSlack            = remaining - autoH * autoCount;
    }

    int contentH = explicitSum + autoH * autoCount + rowSlack;
    const bool overflow = contentH > available;

    if (overflow) {
        autoH    = minH;
        rowSlack = 0;
        contentH = explicitSum + autoH * autoCount;
        setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    } else if (m_hideScrollBarWhenFits) {
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    } else {
        setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    }

    verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    int autoIndex = 0;
    for (int row = 0; row < rows; ++row) {
        if (m_explicitRowHeights.contains(row))
            continue;
        int h = autoH;
        if (!overflow && rowSlack > 0 && autoIndex < rowSlack)
            ++h;
        QTableWidget::setRowHeight(row, h);
        ++autoIndex;
    }

    Q_UNUSED(contentH);
}

void ThemedTableWidget::changeEvent(QEvent *event)
{
    QTableWidget::changeEvent(event);
    if (event->type() == QEvent::EnabledChange)
        refreshStyle();
}

void ThemedTableWidget::resizeEvent(QResizeEvent *event)
{
    QTableWidget::resizeEvent(event);
    applyHorizontalStretchFactors();
    updateRowHeightLayout();
}

void ThemedTableWidget::showEvent(QShowEvent *event)
{
    QTableWidget::showEvent(event);
    applyHorizontalStretchFactors();
    updateRowHeightLayout();
}

ThemedTableWidget *makeTableWidget(int rows, int columns, QWidget *parent)
{
    return new ThemedTableWidget(rows, columns, parent);
}

} // namespace ui
