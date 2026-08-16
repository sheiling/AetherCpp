/**
 * @file help_view.cpp
 * @brief HelpView 二级导航布局、版本信息页与 PDF 帮助文档页实现
 */

#include "help_view.h"

#include "help_config.h"
#include "pdfwidget.h"
#include "uibase/thememanager.h"

#include <QtGlobal>

#include <QFile>

#include <QAbstractItemView>
#include <QButtonGroup>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

/** HelpView 本地主题色与布局辅助 */
namespace {

/** 取主题色，无效时回退默认值 */
QColor themeColorOrDefault(const QString &name, const QColor &fallback)
{
    const QColor c = TM ? TM->themeColor(name) : QColor{};
    return c.isValid() ? c : fallback;
}

/** 帮助页分区外壳（GroupBox + 内部标题 + 内容区） */
struct HelpSectionShell {
    QGroupBox *shell = nullptr;
    QWidget   *body  = nullptr;
};

/** 创建帮助页分区外壳 */
HelpSectionShell makeHelpSection(const QString &title, QWidget *parent, bool transparentWrap = false)
{
    HelpSectionShell out;
    out.shell = new QGroupBox(parent);
    out.shell->setObjectName(QStringLiteral("helpSection"));
    out.shell->setTitle(QString());
    out.shell->setAttribute(Qt::WA_StyledBackground, true);
    out.shell->setAutoFillBackground(false);
    if (transparentWrap)
        out.shell->setFlat(true);
    out.shell->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto *outer = new QVBoxLayout(out.shell);
    outer->setContentsMargins(12, 12, 12, 12);
    outer->setSpacing(10);

    auto *head = new QLabel(title, out.shell);
    head->setObjectName(QStringLiteral("helpSectionTitle"));
    QFont hf = head->font();
    hf.setBold(true);
    hf.setPointSizeF(hf.pointSizeF() + 1.0);
    head->setFont(hf);
    outer->addWidget(head);

    out.body = new QWidget(out.shell);
    out.body->setObjectName(QStringLiteral("helpSectionBody"));
    out.body->setAutoFillBackground(false);
    out.body->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    outer->addWidget(out.body, 1);
    return out;
}

} // namespace

/** @brief HelpView 成员实现（二级导航布局与版本信息页） */
namespace help {

HelpView::HelpView(QWidget *parent)
    : PageWidget(parent)
{
    setPageId(QStringLiteral("help"));
    setPageTitle(QStringLiteral("帮助"));
}

void HelpView::applyTheme()
{
    setObjectName(QStringLiteral("helpRoot"));
    setAttribute(Qt::WA_StyledBackground, true);
    applyRootStylesheet();
    applyNavBarStyle();
    applyMainPanelStyle();
}

void HelpView::applyRootStylesheet()
{
    const QString kBorder =
        themeColorOrDefault(QStringLiteral("secondaryLightColor"), QColor(QStringLiteral("#2b4668"))).name();

    setStyleSheet(QStringLiteral(
        "#helpRoot {"
        "  background-color: #151c26;"
        "}"

        "#helpRoot QLabel { color: #e8eef6; font-size: 13px; background-color: transparent; }"
        "#helpRoot QLabel#helpMutedLabel { color: #a0aab7; font-size: 13px; background-color: transparent; }"

        "#helpRoot QGroupBox#helpSection {"
        "  background-color: transparent;"
        "  border: 1px solid %2;"
        "  border-radius: 8px;"
        "  margin-top: 0px;"
        "  padding: 0px;"
        "}"
        "#helpRoot QLabel#helpSectionTitle {"
        "  color: #ffffff;"
        "  font-size: 14px;"
        "  font-weight: 700;"
        "  background-color: transparent;"
        "}"
        "#helpRoot QWidget#helpSectionBody {"
        "  background-color: transparent;"
        "}"

        "#helpRoot QStackedWidget {"
        "  background-color: transparent;"
        "}"

        "#helpRoot QTableWidget#helpVersionTable {"
        "  background-color: transparent;"
        "  alternate-background-color: transparent;"
        "  color: #e8eef6;"
        "  gridline-color: %1;"
        "  border: 1px solid %1;"
        "  border-radius: 4px;"
        "  font-size: 13px;"
        "}"
        "#helpRoot QTableWidget#helpVersionTable::item {"
        "  padding: 10px 8px;"
        "  background-color: transparent;"
        "}"
        "#helpRoot QTableWidget#helpVersionTable QTableCornerButton::section {"
        "  background-color: transparent;"
        "  border: none;"
        "}"
        "#helpRoot QTableWidget#helpVersionTable QWidget#qt_scrollarea_viewport {"
        "  background-color: transparent;"
        "}"

        "#helpRoot QScrollBar:vertical { background: #151c26; width: 10px; margin: 0; }"
        "#helpRoot QScrollBar::handle:vertical {"
        "  background: #3a4a5d; min-height: 28px; border-radius: 4px;"
        "}"
        "#helpRoot QScrollBar:horizontal { background: #151c26; height: 10px; margin: 0; }"
        "#helpRoot QScrollBar::handle:horizontal {"
        "  background: #3a4a5d; min-width: 28px; border-radius: 4px;"
        "}"
    ).arg(kBorder, kBorder));
}

void HelpView::applyNavBarStyle()
{
    if (!m_navBar)
        return;

    const QColor panelBg      = themeColorOrDefault(QStringLiteral("secondaryColor"), QColor(QStringLiteral("#1b2f4a")));
    const QColor panelBorder  = themeColorOrDefault(QStringLiteral("secondaryLightColor"), QColor(QStringLiteral("#2b4668")));
    const QColor textNormal   = themeColorOrDefault(QStringLiteral("secondaryTextColor"), QColor(QStringLiteral("#9fb7d6")));
    const QColor textActive   = themeColorOrDefault(QStringLiteral("primaryTextColor"), QColor(QStringLiteral("#ffffff")));
    const QColor hoverBg      = themeColorOrDefault(QStringLiteral("primaryLightColor"), QColor(QStringLiteral("#1f4a77")));
    const QColor checkedBg    = themeColorOrDefault(QStringLiteral("primaryColor"), QColor(QStringLiteral("#1f4a77")));
    const QColor navCheckedBorder(QStringLiteral("#4f8fd5"));

    m_navBar->setStyleSheet(QStringLiteral(
        "QWidget#secondaryNavBar {"
        "  background-color: %1;"
        "  border: 1px solid %2;"
        "  border-radius: 8px;"
        "}"
        "QPushButton#secondaryNavBtn {"
        "  min-height: 42px;"
        "  text-align: left;"
        "  padding: 0 14px;"
        "  color: %3;"
        "  background-color: transparent;"
        "  border: 1px solid %2;"
        "  border-radius: 6px;"
        "}"
        "QPushButton#secondaryNavBtn:hover {"
        "  color: %4;"
        "  background-color: %5;"
        "  border-color: %6;"
        "}"
        "QPushButton#secondaryNavBtn:checked {"
        "  color: %4;"
        "  background-color: %7;"
        "  border-color: %8;"
        "}"
    ).arg(panelBg.name(),
          panelBorder.name(),
          textNormal.name(),
          textActive.name(),
          hoverBg.name(),
          hoverBg.name(),
          checkedBg.name(),
          navCheckedBorder.name()));
}

void HelpView::applyMainPanelStyle()
{
    if (!m_mainPanel)
        return;

    const QColor panelBg     = themeColorOrDefault(QStringLiteral("secondaryColor"), QColor(QStringLiteral("#1b2f4a")));
    const QColor panelBorder = themeColorOrDefault(QStringLiteral("secondaryLightColor"), QColor(QStringLiteral("#2b4668")));

    m_mainPanel->setStyleSheet(QStringLiteral(
        "QWidget#helpMainPanel {"
        "  background-color: %1;"
        "  border: 1px solid %2;"
        "  border-radius: 8px;"
        "}"
    ).arg(panelBg.name(), panelBorder.name()));
}

void HelpView::onLoad()
{
    buildLayout();
}

void HelpView::buildLayout()
{
    m_navBar = new QWidget(this);
    m_navBar->setObjectName(QStringLiteral("secondaryNavBar"));
    m_navBar->setFixedWidth(320);
    auto *nvLay = new QVBoxLayout(m_navBar);
    nvLay->setContentsMargins(14, 14, 14, 14);
    nvLay->setSpacing(10);
    nvLay->addStretch();

    m_navGroup = new QButtonGroup(this);
    const QStringList navLabels = {
        QStringLiteral("版本信息"),
        QStringLiteral("用户手册"),
        QStringLiteral("维护手册"),
        QStringLiteral("帮助文档"),
    };
    for (int i = 0; i < navLabels.size(); ++i) {
        auto *b = new QPushButton(navLabels[i], m_navBar);
        b->setObjectName(QStringLiteral("secondaryNavBtn"));
        b->setCheckable(true);
        b->setFixedHeight(40);
        b->setCursor(Qt::PointingHandCursor);
        nvLay->insertWidget(nvLay->count() - 1, b);
        m_navGroup->addButton(b, i);
    }
    m_navGroup->button(0)->setChecked(true);

    m_mainPanel = new QWidget(this);
    m_mainPanel->setObjectName(QStringLiteral("helpMainPanel"));
    m_mainPanel->setAttribute(Qt::WA_StyledBackground, true);

    auto *panelLay = new QVBoxLayout(m_mainPanel);
    panelLay->setContentsMargins(14, 14, 14, 14);
    panelLay->setSpacing(0);

    m_stack = new QStackedWidget(m_mainPanel);
    m_stack->setAutoFillBackground(false);

    const HelpDocuments docs = loadHelpDocuments();
    m_stack->addWidget(buildVersionPage());
    m_stack->addWidget(buildPdfPage(docs.userManual));
    m_stack->addWidget(buildPdfPage(docs.maintenance));
    m_stack->addWidget(buildPdfPage(docs.helpDoc));
    panelLay->addWidget(m_stack, 1);

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    connect(m_navGroup, &QButtonGroup::idClicked, m_stack, &QStackedWidget::setCurrentIndex);
#else
    connect(m_navGroup, QOverload<int>::of(&QButtonGroup::buttonClicked), m_stack, &QStackedWidget::setCurrentIndex);
#endif

    connect(TM, &ThemeManager::themeChanged, this, [this](ThemeManager::Theme) {
        applyTheme();
    });

    auto *rootLay = new QHBoxLayout(this);
    rootLay->setContentsMargins(18, 14, 18, 14);
    rootLay->setSpacing(8);
    rootLay->addWidget(m_navBar);
    rootLay->addWidget(m_mainPanel, 1);

    applyTheme();
}

QWidget *HelpView::buildVersionPage()
{
    const auto sec  = makeHelpSection(QStringLiteral("版本信息"), m_stack, true);
    auto      *page = sec.shell;
    auto      *body = sec.body;

    struct InfoRow {
        QString key;
        QString value;
    };

    const HelpVersionInfo ver = loadHelpVersionInfo();
    const QVector<InfoRow> info = {
        {QStringLiteral("软件名称"), ver.appName},
        {QStringLiteral("当前版本"), ver.currentVersion},
        {QStringLiteral("构建时间"), ver.buildTime},
        {QStringLiteral("更新时间"), ver.updateTime},
    };

    auto *table = new QTableWidget(int(info.size()), 3, body);
    table->setObjectName(QStringLiteral("helpVersionTable"));
    table->setAttribute(Qt::WA_StyledBackground, true);
    table->setAutoFillBackground(false);
    if (QWidget *vp = table->viewport()) {
        vp->setAutoFillBackground(false);
    }
    table->horizontalHeader()->hide();
    table->verticalHeader()->hide();
    table->setShowGrid(true);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setFocusPolicy(Qt::NoFocus);
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    table->verticalHeader()->setDefaultSectionSize(44);

    for (int r = 0; r < info.size(); ++r) {
        auto *k = new QTableWidgetItem(info[r].key);
        k->setTextAlignment(Qt::AlignCenter);
        QFont f = k->font();
        f.setBold(true);
        k->setFont(f);
        table->setItem(r, 0, k);

        table->setItem(r, 1, new QTableWidgetItem(QString()));

        auto *v = new QTableWidgetItem(info[r].value);
        v->setTextAlignment(Qt::AlignCenter);
        table->setItem(r, 2, v);
    }

    auto *bl = new QVBoxLayout(body);
    bl->setContentsMargins(0, 0, 0, 0);
    bl->setSpacing(0);
    bl->addWidget(table);
    return page;
}

QWidget *HelpView::buildPdfPage(const QString &pdfPathConfig)
{
    auto *page = new QWidget(m_stack);
    page->setObjectName(QStringLiteral("helpPdfPage"));
    page->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    const QString pdfPath = resolveRuntimePath(pdfPathConfig);

    if (QFile::exists(pdfPath)) {
        auto *pdfWidget = new PdfWidget(page);
        pdfWidget->setPdf(pdfPath);
        layout->addWidget(pdfWidget, 1);
        return page;
    }

    auto *lbl = new QLabel(
        QStringLiteral("无法加载 PDF：%1").arg(pdfPath), page);
    lbl->setObjectName(QStringLiteral("helpMutedLabel"));
    lbl->setWordWrap(true);
    lbl->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    layout->addWidget(lbl, 1);
    return page;
}

} // namespace help
