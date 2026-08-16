/**
 * @file disk_storage_panel.cpp
 * @brief DiskStoragePanel：挂载卷枚举、行 UI 构建与主题着色
 */

#include "disk/disk_storage_panel.h"

#include "group/themed_section_panel.h"
#include "theme/theme_refresh.h"
#include "theme/theme_tokens.h"

#include "uibase/languagemanager.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QScrollArea>
#include <QStorageInfo>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>

namespace ui {

namespace {

QString formatGiB(qint64 bytes)
{
    if (bytes <= 0)
        return QStringLiteral("-");
    return QString::number(static_cast<double>(bytes) / (1024.0 * 1024.0 * 1024.0), 'f', 1)
        + QStringLiteral(" GB");
}

QWidget *makeTransparentHost(QWidget *parent)
{
    auto *host = new QWidget(parent);
    host->setAttribute(Qt::WA_StyledBackground, true);
    host->setAutoFillBackground(false);
    host->setStyleSheet(QStringLiteral("background: transparent;"));
    return host;
}

QColor usageChunkColor(const ThemeTokens &t, int pct)
{
    if (pct > 80)
        return t.dangerText;
    if (pct > 60)
        return t.warningText;
    return t.accent;
}

QString progressBarStyle(const ThemeTokens &t, const QColor &chunk)
{
    return QStringLiteral(
               "QProgressBar#diskStorageBar {"
               "  border: 1px solid %1;"
               "  border-radius: 4px;"
               "  background-color: %2;"
               "  min-height: 7px; max-height: 7px;"
               "  text-align: center;"
               "}"
               "QProgressBar#diskStorageBar::chunk {"
               "  background-color: %3;"
               "  border-radius: 3px;"
               "  margin: 1px;"
               "}")
        .arg(t.inputBorder.name(), t.inputBackground.name(), chunk.name());
}

} // namespace

DiskStoragePanel::DiskStoragePanel(QWidget *parent, DiskStoragePanelOptions options)
    : QWidget(parent)
    , m_options(std::move(options))
{
    setObjectName(QStringLiteral("diskStoragePanel"));
    buildLayout();
    bindThemeRefresh(this, [this]() { applyRowStyles(); });
    refreshDisks();
    startRefreshTimer();
}

void DiskStoragePanel::setOptions(const DiskStoragePanelOptions &options)
{
    m_options = options;
    if (m_panel)
        m_panel->setTitleKey(m_options.titleKey);
    startRefreshTimer();
    refreshDisks();
}

void DiskStoragePanel::startRefreshTimer()
{
    if (!m_timer) {
        m_timer = new QTimer(this);
        connect(m_timer, &QTimer::timeout, this, &DiskStoragePanel::refreshDisks);
    }
    if (m_options.refreshIntervalMs <= 0) {
        m_timer->stop();
        return;
    }
    m_timer->setInterval(m_options.refreshIntervalMs);
    if (!m_timer->isActive())
        m_timer->start();
}

void DiskStoragePanel::buildLayout()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_panel = new ThemedSectionPanel(m_options.titleKey, this);
    auto *scroll = new QScrollArea(m_panel->contentHost());
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setStyleSheet(QStringLiteral("QScrollArea { background: transparent; border: none; }"));

    auto *rowsHost = makeTransparentHost(scroll);
    m_rowsLayout   = new QVBoxLayout(rowsHost);
    m_rowsLayout->setContentsMargins(2, 0, 2, 0);
    m_rowsLayout->setSpacing(6);
    scroll->setWidget(rowsHost);
    m_panel->setContentWidget(scroll, 1);
    layout->addWidget(m_panel);
}

void DiskStoragePanel::refreshDisks()
{
    while (QLayoutItem *item = m_rowsLayout->takeAt(0)) {
        if (QWidget *w = item->widget())
            w->deleteLater();
        delete item;
    }
    m_rows.clear();

    QList<QStorageInfo> volumes = QStorageInfo::mountedVolumes();
    std::sort(volumes.begin(), volumes.end(),
              [](const QStorageInfo &a, const QStorageInfo &b) {
                  return a.rootPath() < b.rootPath();
              });

    for (const QStorageInfo &info : volumes) {
        if (!info.isValid() || !info.isReady() || info.bytesTotal() <= 0)
            continue;
        const QString root = info.rootPath();
        // 跳过 UNC 网络路径，避免无意义条目
        if (root.startsWith(QStringLiteral("//")))
            continue;

        const qint64 total = info.bytesTotal();
        const qint64 free  = info.bytesAvailable();
        const qint64 used  = qMax<qint64>(0, total - free);
        const int    pct   = total > 0 ? static_cast<int>((used * 100) / total) : 0;

        auto *row = new QWidget(m_rowsLayout->parentWidget());
        row->setObjectName(QStringLiteral("diskStorageRow"));
        auto *vl = new QVBoxLayout(row);
        vl->setContentsMargins(4, 2, 4, 2);
        vl->setSpacing(3);

        auto *header = new QWidget(row);
        auto *hl     = new QHBoxLayout(header);
        hl->setContentsMargins(0, 0, 0, 0);
        hl->setSpacing(6);

        DiskRowWidgets widgets;
        widgets.usagePct = pct;

        widgets.drive = new QLabel(header);
        widgets.drive->setObjectName(QStringLiteral("diskStorageDrive"));
        widgets.drive->setText(info.displayName().isEmpty() ? root : info.displayName());

        widgets.usage = new QLabel(header);
        widgets.usage->setObjectName(QStringLiteral("diskStorageUsage"));
        widgets.usage->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        widgets.usage->setText(TR(m_options.usedPctFormatKey.toUtf8().constData()).arg(pct));

        hl->addWidget(widgets.drive, 0);
        hl->addStretch(1);
        hl->addWidget(widgets.usage, 0);

        widgets.bar = new QProgressBar(row);
        widgets.bar->setObjectName(QStringLiteral("diskStorageBar"));
        widgets.bar->setRange(0, 100);
        widgets.bar->setValue(pct);
        widgets.bar->setTextVisible(false);

        widgets.detail = new QLabel(row);
        widgets.detail->setObjectName(QStringLiteral("diskStorageDetail"));
        widgets.detail->setText(TR(m_options.detailFormatKey.toUtf8().constData())
                                    .arg(formatGiB(total), formatGiB(free)));

        vl->addWidget(header);
        vl->addWidget(widgets.bar);
        vl->addWidget(widgets.detail);
        m_rowsLayout->addWidget(row);
        m_rows.push_back(widgets);
    }

    m_rowsLayout->addStretch(1);
    applyRowStyles();
}

void DiskStoragePanel::applyRowStyles()
{
    const ThemeTokens t = ThemeTokens::resolve();
    const QString     driveStyle =
        QStringLiteral("QLabel#diskStorageDrive { color: %1; font-size: 12px; font-weight: 600; "
                       "background: transparent; }")
            .arg(t.textPrimary.name());
    const QString detailStyle =
        QStringLiteral("QLabel#diskStorageDetail { color: %1; font-size: 11px; background: "
                       "transparent; }")
            .arg(t.textSecondary.name());

    for (const DiskRowWidgets &row : m_rows) {
        if (row.drive)
            row.drive->setStyleSheet(driveStyle);
        if (row.detail)
            row.detail->setStyleSheet(detailStyle);
        if (row.usage) {
            const QColor color = usageChunkColor(t, row.usagePct);
            row.usage->setStyleSheet(
                QStringLiteral("QLabel#diskStorageUsage { color: %1; font-size: 11px; font-weight: "
                               "600; background: transparent; }")
                    .arg(color.name()));
        }
        if (row.bar)
            row.bar->setStyleSheet(progressBarStyle(t, usageChunkColor(t, row.usagePct)));
    }
}

void DiskStoragePanel::retranslateUi()
{
    if (m_panel)
        m_panel->retranslatePanel();
    refreshDisks();
}

} // namespace ui
