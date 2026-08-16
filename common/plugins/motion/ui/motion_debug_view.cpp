#include "motion_debug_view.h"

#include "io_combine_debug_widget.h"
#include "io_point_widget.h"
#include "motion_debug_action_keys.h"

#include "uibase/permission/permservice.h"
#include "uibase/thememanager.h"

#include <QButtonGroup>
#include <QDebug>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QStackedWidget>
#include <QTimer>
#include <QVBoxLayout>

namespace motion {

namespace {

QString verticalLabel(const QString &text)
{
    QString out;
    for (const QChar &c : text) {
        if (!out.isEmpty())
            out += QLatin1Char('\n');
        out += c;
    }
    return out;
}

QColor themeColorOrDefault(const QString &name, const QColor &fallback)
{
    const QColor c = TM ? TM->themeColor(name) : QColor{};
    return c.isValid() ? c : fallback;
}

void clearLayout(QLayout *layout)
{
    if (!layout)
        return;
    while (QLayoutItem *item = layout->takeAt(0)) {
        if (QWidget *w = item->widget())
            delete w;
        delete item;
    }
}

} // namespace

MotionDebugView::MotionDebugView(QWidget *parent)
    : PageWidget(parent)
{
    setPageId(QStringLiteral("motion.motion_debug"));
}

void MotionDebugView::onLoad()
{
    buildLayout();
    bindPermissions();
    rebuildSinglePointIoPage();
    applyPermissions();
    applyTheme();

    if (!m_ioPollTimer) {
        m_ioPollTimer = new QTimer(this);
        connect(m_ioPollTimer, &QTimer::timeout, this, &MotionDebugView::refreshSinglePointIoStates);
    }
    m_ioPollTimer->start(200);
}

void MotionDebugView::applyTheme()
{
    setObjectName(QStringLiteral("motionDebugRoot"));
    setAttribute(Qt::WA_StyledBackground, true);

    const QString panelBg =
        themeColorOrDefault(QStringLiteral("secondaryColor"), QColor(QStringLiteral("#1b2f4a"))).name();
    const QString panelBorder =
        themeColorOrDefault(QStringLiteral("secondaryLightColor"), QColor(QStringLiteral("#2b4668")))
            .name();
    const QString sectionBg = QStringLiteral("#17273b");
    const QString textNormal =
        themeColorOrDefault(QStringLiteral("secondaryTextColor"), QColor(QStringLiteral("#9fb7d6")))
            .name();
    const QString textActive =
        themeColorOrDefault(QStringLiteral("primaryTextColor"), QColor(QStringLiteral("#ffffff")))
            .name();
    const QString checkedBg =
        themeColorOrDefault(QStringLiteral("primaryColor"), QColor(QStringLiteral("#1f4a77"))).name();

    setStyleSheet(QStringLiteral(
        "#motionDebugRoot { background: transparent; }"

        "#motionDebugRoot QWidget#motionPrimaryNav {"
        "  background-color: %1; border: 1px solid %2; border-radius: 8px;"
        "}"
        "#motionDebugRoot QPushButton#motionPrimaryNavBtn {"
        "  min-width: 36px; max-width: 36px; min-height: 120px;"
        "  padding: 8px 4px; color: %3; background: transparent;"
        "  border: none; border-radius: 4px; font-size: 13px;"
        "}"
        "#motionDebugRoot QPushButton#motionPrimaryNavBtn:hover {"
        "  color: %4; background-color: #243a55;"
        "}"
        "#motionDebugRoot QPushButton#motionPrimaryNavBtn:checked {"
        "  color: %4; background-color: %5; border-left: 3px solid #4f8fd5;"
        "}"

        "#motionDebugRoot QWidget#motionMainPanel {"
        "  background-color: %1; border: 1px solid %2; border-radius: 8px;"
        "}"
        "#motionDebugRoot QWidget#motionSubTabBar { background: transparent; }"
        "#motionDebugRoot QPushButton#motionSubTabBtn {"
        "  min-height: 36px; padding: 0 18px; color: %3; background: transparent;"
        "  border: none; border-bottom: 2px solid transparent; font-size: 14px;"
        "}"
        "#motionDebugRoot QPushButton#motionSubTabBtn:hover { color: #dbe4ee; }"
        "#motionDebugRoot QPushButton#motionSubTabBtn:checked {"
        "  color: #4f8fd5; border-bottom: 2px solid #4f8fd5; font-weight: 600;"
        "}"

        "#motionDebugRoot QWidget#motionIoColumn {"
        "  background-color: %6; border: 1px solid %2; border-radius: 8px;"
        "}"
        "#motionDebugRoot QLabel#motionIoColumnBadge {"
        "  min-width: 32px; max-width: 32px; min-height: 24px; max-height: 24px;"
        "  border-radius: 4px; font-size: 12px; font-weight: 700;"
        "  color: #9fb7d6; background-color: transparent; border: 1px solid %2;"
        "}"
        "#motionDebugRoot QLabel#motionIoColumnTitle {"
        "  color: #ffffff; font-size: 14px; font-weight: 700; background: transparent;"
        "}"
        "#motionDebugRoot QPushButton#motionIoColumnActionBtn {"
        "  min-height: 26px; padding: 0 12px; color: %3; background-color: #243a55;"
        "  border: 1px solid %2; border-radius: 4px; font-size: 13px;"
        "}"
        "#motionDebugRoot QPushButton#motionIoColumnActionBtn:hover {"
        "  color: %4; background-color: #2d4a68;"
        "}"

        "#motionDebugRoot QFrame#motionIoPoint {"
        "  background-color: %1; border: 1px solid %2; border-radius: 8px;"
        "}"
        "#motionDebugRoot QLabel#motionIoPointPort,"
        "#motionDebugRoot QLabel#motionIoPointTitle {"
        "  color: #ffffff; font-size: 14px; font-weight: 600; background: transparent;"
        "}"
        "#motionDebugRoot QLabel#motionIoFormLabel {"
        "  color: #dbe4ee; font-size: 13px; background: transparent;"
        "  min-height: 26px; max-height: 26px; padding: 0; margin: 0;"
        "}"
        "#motionDebugRoot QLineEdit#motionIoPortEdit {"
        "  background-color: #15263d;"
        "  color: #e8eef6;"
        "  border: 1px solid #3a4a5d;"
        "  border-radius: 4px;"
        "  padding: 0 6px;"
        "  selection-background-color: #1976d2;"
        "  selection-color: #ffffff;"
        "}"
        "#motionDebugRoot QRadioButton#motionIoRadio {"
        "  color: #dbe4ee; font-size: 13px; spacing: 6px; background: transparent;"
        "  min-height: 26px; max-height: 26px; padding: 0; margin: 0;"
        "}"
        "#motionDebugRoot QRadioButton#motionIoRadio::indicator {"
        "  width: 14px; height: 14px; border-radius: 7px;"
        "  border: 1px solid %2; background: #152232;"
        "}"
        "#motionDebugRoot QRadioButton#motionIoRadio::indicator:checked {"
        "  background-color: #2d7dd2; border-color: #4f8fd5;"
        "}"

        "#motionDebugRoot QLabel#motionPlaceholderTitle {"
        "  color: #ffffff; font-size: 16px; font-weight: 700; background: transparent;"
        "}"
        "#motionDebugRoot QLabel#motionPlaceholderHint {"
        "  color: %3; font-size: 13px; background: transparent;"
        "}"
        "#motionDebugRoot QGroupBox#motionIoCombineGroup {"
        "  background-color: %6; border: 1px solid %2; border-radius: 8px;"
        "  margin-top: 12px; color: #ffffff; font-size: 14px; font-weight: 600;"
        "}"
        "#motionDebugRoot QGroupBox#motionIoCombineGroup::title {"
        "  subcontrol-origin: margin; left: 12px; padding: 0 6px;"
        "  color: #ffffff; font-size: 14px; font-weight: 600;"
        "}"
        "#motionDebugRoot QPushButton#motionIoCombineActionBtn {"
        "  min-height: 32px; padding: 0 14px; color: %3; background-color: #243a55;"
        "  border: 1px solid %2; border-radius: 4px; font-size: 13px; text-align: left;"
        "}"
        "#motionDebugRoot QPushButton#motionIoCombineActionBtn:hover {"
        "  color: %4; background-color: #2d4a68;"
        "}"
        "#motionDebugRoot QLabel#motionIoCombineEmptyHint {"
        "  color: %3; font-size: 13px; background: transparent; padding: 24px;"
        "}"
        "#motionDebugRoot QScrollArea { background: transparent; border: none; }"
        "#motionDebugRoot QScrollBar:vertical {"
        "  background: transparent; width: 10px; margin: 0;"
        "}"
        "#motionDebugRoot QScrollBar::handle:vertical {"
        "  background: #3a4a5d; min-height: 28px; border-radius: 4px;"
        "}"
    ).arg(panelBg, panelBorder, textNormal, textActive, checkedBg, sectionBg));
}

QWidget *MotionDebugView::buildPlaceholderPage(const QString &title, const QString &hint)
{
    auto *page = new QWidget(this);
    auto *lay  = new QVBoxLayout(page);
    lay->setAlignment(Qt::AlignCenter);
    auto *t = new QLabel(title, page);
    t->setObjectName(QStringLiteral("motionPlaceholderTitle"));
    auto *h = new QLabel(hint, page);
    h->setObjectName(QStringLiteral("motionPlaceholderHint"));
    h->setAlignment(Qt::AlignCenter);
    lay->addWidget(t, 0, Qt::AlignHCenter);
    lay->addWidget(h, 0, Qt::AlignHCenter);
    return page;
}

QWidget *MotionDebugView::buildIoColumn(const QString &columnTitle,
                                        const QVector<MotionIoPointConfig> &points,
                                        bool isOutput,
                                        QWidget *parent,
                                        QHash<QString, IoPointWidget *> *pointRegistry,
                                        QPushButton **outReadBtn,
                                        QPushButton **outWriteBtn)
{
    auto *col = new QWidget(parent);
    col->setObjectName(QStringLiteral("motionIoColumn"));
    col->setAttribute(Qt::WA_StyledBackground, true);
    auto *lay = new QVBoxLayout(col);
    lay->setContentsMargins(12, 12, 12, 12);
    lay->setSpacing(10);

    auto *headerLay = new QHBoxLayout();
    headerLay->setContentsMargins(0, 0, 0, 0);
    headerLay->setSpacing(10);
    headerLay->addStretch();

    auto *headerCenter = new QHBoxLayout();
    headerCenter->setContentsMargins(0, 0, 0, 0);
    headerCenter->setSpacing(10);

    auto *badge = new QLabel(isOutput ? QStringLiteral("DO") : QStringLiteral("DI"), col);
    badge->setObjectName(QStringLiteral("motionIoColumnBadge"));
    badge->setAlignment(Qt::AlignCenter);
    headerCenter->addWidget(badge, 0, Qt::AlignVCenter);

    auto *title = new QLabel(columnTitle, col);
    title->setObjectName(QStringLiteral("motionIoColumnTitle"));
    headerCenter->addWidget(title, 0, Qt::AlignVCenter);
    headerCenter->addStretch();

    auto *readBtn = new QPushButton(QStringLiteral("读入"), col);
    readBtn->setObjectName(QStringLiteral("motionIoColumnActionBtn"));
    readBtn->setCursor(Qt::PointingHandCursor);
    auto *writeBtn = new QPushButton(QStringLiteral("写出"), col);
    writeBtn->setObjectName(QStringLiteral("motionIoColumnActionBtn"));
    writeBtn->setCursor(Qt::PointingHandCursor);
    headerCenter->addWidget(readBtn, 0, Qt::AlignVCenter);
    headerCenter->addWidget(writeBtn, 0, Qt::AlignVCenter);

    headerLay->addLayout(headerCenter);
    headerLay->addStretch();
    lay->addLayout(headerLay);

    if (outReadBtn)
        *outReadBtn = readBtn;
    if (outWriteBtn)
        *outWriteBtn = writeBtn;

    connect(readBtn, &QPushButton::clicked, this, [this, isOutput]() {
        if (!PermService::instance()->canAction(MotionDebugActionKeys::IoRead))
            return;
        loadIoColumnFromConfig(isOutput);
    });
    connect(writeBtn, &QPushButton::clicked, this, [this, isOutput]() {
        if (!PermService::instance()->canAction(MotionDebugActionKeys::IoWrite))
            return;
        saveIoColumnToConfig(isOutput);
    });

    auto *scroll = new QScrollArea(col);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto *gridHost = new QWidget(scroll);
    auto *grid = new QGridLayout(gridHost);
    grid->setContentsMargins(0, 0, 4, 0);
    grid->setHorizontalSpacing(12);
    grid->setVerticalSpacing(12);

    for (int i = 0; i < points.size(); ++i) {
        auto *point = IoPointWidget::create(points[i].name, isOutput, gridHost);
        point->applyFromConfig(points[i].port, points[i].highActive);
        if (pointRegistry)
            pointRegistry->insert(points[i].name, point);
        grid->addWidget(point, i / 2, i % 2);
    }
    if (!points.isEmpty())
        grid->setRowStretch((points.size() + 1) / 2, 1);

    scroll->setWidget(gridHost);
    lay->addWidget(scroll, 1);
    return col;
}

void MotionDebugView::bindPermissions()
{
    connect(PermService::instance(), &PermService::permissionsChanged, this,
            &MotionDebugView::applyPermissions, Qt::UniqueConnection);
}

void MotionDebugView::applyPermissions()
{
    const bool canRead  = PermService::instance()->canAction(MotionDebugActionKeys::IoRead);
    const bool canWrite = PermService::instance()->canAction(MotionDebugActionKeys::IoWrite);
    setShowFullDetails(canRead || canWrite);

    const auto applyBtn = [](QPushButton *btn, bool enabled) {
        if (btn)
            btn->setEnabled(enabled);
    };
    applyBtn(m_diReadBtn, canRead);
    applyBtn(m_doReadBtn, canRead);
    applyBtn(m_diWriteBtn, canWrite);
    applyBtn(m_doWriteBtn, canWrite);
}

void MotionDebugView::setShowFullDetails(bool show)
{
    m_showFullDetails = show;

    const auto applyColumn = [show](QPushButton *readBtn, QPushButton *writeBtn,
                                    const QHash<QString, IoPointWidget *> &points) {
        if (readBtn)
            readBtn->setVisible(show);
        if (writeBtn)
            writeBtn->setVisible(show);
        for (IoPointWidget *point : points) {
            if (point)
                point->setShowFullDetails(show);
        }
    };

    applyColumn(m_diReadBtn, m_diWriteBtn, m_inputPoints);
    applyColumn(m_doReadBtn, m_doWriteBtn, m_outputPoints);
}

void MotionDebugView::reloadDeviceIoCache()
{
    const MotionDeviceConfigReader::Snapshot snapshot = MotionDeviceConfigReader::loadSnapshot();
    m_deviceIoConfigs   = snapshot.deviceIo;
    m_motionCardNames   = snapshot.motionCardNames;
}

void MotionDebugView::applyIoStatesToPoints(const QHash<QString, IoPointWidget *> &points,
                                            const QHash<QString, bool>            &states)
{
    for (auto it = points.constBegin(); it != points.constEnd(); ++it) {
        const auto found = states.constFind(it.key());
        if (found != states.constEnd())
            it.value()->setActiveState(found.value());
    }
}

void MotionDebugView::loadIoColumnFromConfig(bool isOutput)
{
    if (!PermService::instance()->canAction(MotionDebugActionKeys::IoRead))
        return;

    reloadDeviceIoCache();
    const QVector<MotionIoPointConfig> columnConfigs =
        filterIoConfigsByDirection(m_deviceIoConfigs, isOutput);

    const auto &points = isOutput ? m_outputPoints : m_inputPoints;
    if (columnConfigs.size() != points.size()) {
        rebuildSinglePointIoPage(false);
        return;
    }

    for (const MotionIoPointConfig &cfg : columnConfigs) {
        IoPointWidget *point = points.value(cfg.name);
        if (!point) {
            rebuildSinglePointIoPage(false);
            return;
        }
        point->applyFromConfig(cfg.port, cfg.highActive);
    }
}

void MotionDebugView::saveIoColumnToConfig(bool isOutput)
{
    if (!PermService::instance()->canAction(MotionDebugActionKeys::IoWrite))
        return;

    const auto &points = isOutput ? m_outputPoints : m_inputPoints;
    QHash<QString, QPair<int, int>> updates;
    updates.reserve(points.size());

    for (auto it = points.constBegin(); it != points.constEnd(); ++it) {
        IoPointWidget *point = it.value();
        if (!point)
            continue;
        updates.insert(it.key(), qMakePair(point->port(), point->highActive() ? 1 : 0));
    }

    if (!MotionDeviceConfigReader::updateDeviceIoBatch(updates)) {
        qWarning() << "[MotionDebugView] Failed to save DeviceIo config, isOutput=" << isOutput;
        return;
    }
    reloadDeviceIoCache();
}

void MotionDebugView::rebuildSinglePointIoPage(bool reloadFromDisk)
{
    if (!m_singlePointIoPage)
        return;

    m_inputPoints.clear();
    m_outputPoints.clear();
    m_diReadBtn  = nullptr;
    m_diWriteBtn = nullptr;
    m_doReadBtn  = nullptr;
    m_doWriteBtn = nullptr;

    if (QLayout *oldLay = m_singlePointIoPage->layout()) {
        clearLayout(oldLay);
        delete oldLay;
    }

    if (reloadFromDisk)
        reloadDeviceIoCache();

    QVector<MotionIoPointConfig> inputs;
    QVector<MotionIoPointConfig> outputs;
    MotionDeviceConfigReader::partitionByDirection(m_deviceIoConfigs, &inputs, &outputs);

    auto *row = new QHBoxLayout(m_singlePointIoPage);
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(12);
    row->addWidget(buildIoColumn(QStringLiteral("输入"), inputs, false, m_singlePointIoPage, &m_inputPoints,
                                  &m_diReadBtn, &m_diWriteBtn),
                   1);
    row->addWidget(buildIoColumn(QStringLiteral("输出"), outputs, true, m_singlePointIoPage, &m_outputPoints,
                                  &m_doReadBtn, &m_doWriteBtn),
                   1);

    setShowFullDetails(m_showFullDetails);
    applyPermissions();
}

void MotionDebugView::refreshSinglePointIoStates()
{
    if (m_inputPoints.isEmpty() && m_outputPoints.isEmpty())
        return;
    if (m_deviceIoConfigs.isEmpty() || m_motionCardNames.isEmpty())
        return;

    try {
        const auto motion = motionControllerOrNull();
        if (!motion)
            return;

        applyIoStatesToPoints(m_inputPoints,
                              queryIoStates(motion, m_motionCardNames, m_deviceIoConfigs, false));
        applyIoStatesToPoints(m_outputPoints,
                              queryIoStates(motion, m_motionCardNames, m_deviceIoConfigs, true));
    } catch (const std::exception &e) {
        qWarning() << "[MotionDebugView] refreshSinglePointIoStates failed:" << e.what();
    }
}

void MotionDebugView::buildLayout()
{
    auto *rootLay = new QHBoxLayout(this);
    rootLay->setContentsMargins(12, 12, 12, 12);
    rootLay->setSpacing(12);

    m_primaryNav = new QWidget(this);
    m_primaryNav->setObjectName(QStringLiteral("motionPrimaryNav"));
    m_primaryNav->setAttribute(Qt::WA_StyledBackground, true);
    m_primaryNav->setFixedWidth(52);
    auto *pnLay = new QVBoxLayout(m_primaryNav);
    pnLay->setContentsMargins(4, 8, 4, 8);
    pnLay->setSpacing(6);

    m_primaryGroup = new QButtonGroup(this);
    const QStringList primaryLabels = {
        QStringLiteral("输入输出控制"),
        QStringLiteral("轴运动控制"),
    };
    for (int i = 0; i < primaryLabels.size(); ++i) {
        auto *b = new QPushButton(verticalLabel(primaryLabels[i]), m_primaryNav);
        b->setObjectName(QStringLiteral("motionPrimaryNavBtn"));
        b->setCheckable(true);
        b->setCursor(Qt::PointingHandCursor);
        pnLay->addWidget(b);
        m_primaryGroup->addButton(b, i);
    }
    m_primaryGroup->button(0)->setChecked(true);
    pnLay->addStretch();

    m_mainPanel = new QWidget(this);
    m_mainPanel->setObjectName(QStringLiteral("motionMainPanel"));
    m_mainPanel->setAttribute(Qt::WA_StyledBackground, true);
    auto *mainLay = new QVBoxLayout(m_mainPanel);
    mainLay->setContentsMargins(14, 12, 14, 14);
    mainLay->setSpacing(10);

    m_subTabBar = new QWidget(m_mainPanel);
    m_subTabBar->setObjectName(QStringLiteral("motionSubTabBar"));
    auto *subLay = new QHBoxLayout(m_subTabBar);
    subLay->setContentsMargins(0, 0, 0, 0);
    subLay->setSpacing(4);

    m_subTabGroup = new QButtonGroup(this);
    const QStringList subLabels = {
        QStringLiteral("单点调试"),
        QStringLiteral("联动调试"),
    };
    for (int i = 0; i < subLabels.size(); ++i) {
        auto *b = new QPushButton(subLabels[i], m_subTabBar);
        b->setObjectName(QStringLiteral("motionSubTabBtn"));
        b->setCheckable(true);
        b->setCursor(Qt::PointingHandCursor);
        subLay->addWidget(b);
        m_subTabGroup->addButton(b, i);
    }
    m_subTabGroup->button(0)->setChecked(true);
    subLay->addStretch();

    m_singlePointIoPage = new QWidget(m_mainPanel);
    m_ioCombinePage     = new IoCombineDebugWidget(m_mainPanel);
    m_ioSubStack        = new QStackedWidget(m_mainPanel);
    m_ioSubStack->addWidget(m_singlePointIoPage);
    m_ioSubStack->addWidget(m_ioCombinePage);

    m_mainStack = new QStackedWidget(m_mainPanel);
    m_mainStack->addWidget(m_ioSubStack);
    m_mainStack->addWidget(buildPlaceholderPage(QStringLiteral("轴运动控制"),
                                                QStringLiteral("轴运动控制功能开发中…")));

    mainLay->addWidget(m_subTabBar);
    mainLay->addWidget(m_mainStack, 1);

    rootLay->addWidget(m_primaryNav);
    rootLay->addWidget(m_mainPanel, 1);

    connect(m_primaryGroup, QOverload<int>::of(&QButtonGroup::idClicked), this, [this](int id) {
        m_mainStack->setCurrentIndex(id);
        m_subTabBar->setVisible(id == 0);
    });

    connect(m_subTabGroup, QOverload<int>::of(&QButtonGroup::idClicked), m_ioSubStack,
            &QStackedWidget::setCurrentIndex);
}

} // namespace motion
