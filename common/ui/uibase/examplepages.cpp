#include "examplepages.h"
#include "languagemanager.h"

#include <QVBoxLayout>
#include <QFont>

// ─────────────────────────────────────────────
//  SimplePage
// ─────────────────────────────────────────────

SimplePage::SimplePage(const QString &contentKey, QWidget *parent)
    : PageWidget(parent)
    , m_contentKey(contentKey)
{
}

void SimplePage::onLoad()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(40, 40, 40, 40);

    m_label = new QLabel(TR(m_contentKey), this);
    m_label->setObjectName("pageContentLabel");
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setWordWrap(true);

    QFont f = m_label->font();
    f.setPointSize(18);
    m_label->setFont(f);

    layout->addStretch();
    layout->addWidget(m_label);
    layout->addStretch();
}

void SimplePage::retranslateUi()
{
    if (m_label) m_label->setText(TR(m_contentKey));
}

// ─────────────────────────────────────────────
//  SubPage
// ─────────────────────────────────────────────

SubPage::SubPage(const QString &titleKey, QWidget *parent)
    : PageWidget(parent)
    , m_titleKey(titleKey)
{
}

void SubPage::onLoad()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(30, 30, 30, 30);
    layout->setSpacing(12);

    m_titleLabel = new QLabel(TR(m_titleKey), this);
    m_titleLabel->setObjectName("subPageTitle");
    QFont f = m_titleLabel->font();
    f.setPointSize(16);
    f.setBold(true);
    m_titleLabel->setFont(f);

    m_contentLabel = new QLabel(TR("subpage.desc").arg(TR(m_titleKey)), this);
    m_contentLabel->setObjectName("subPageContent");
    m_contentLabel->setWordWrap(true);

    layout->addWidget(m_titleLabel);
    layout->addWidget(m_contentLabel);
    layout->addStretch();
}

void SubPage::retranslateUi()
{
    if (m_titleLabel)   m_titleLabel->setText(TR(m_titleKey));
    if (m_contentLabel) m_contentLabel->setText(TR("subpage.desc").arg(TR(m_titleKey)));
}

// ─────────────────────────────────────────────
//  PageWithSubNav
// ─────────────────────────────────────────────

PageWithSubNav::PageWithSubNav(QWidget *parent)
    : PageWidget(parent)
{
    // 在构造函数中完整初始化，保证 addSubPage() 在 show 前调用也能正常工作
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(18, 14, 18, 14);
    layout->setSpacing(0);

    m_navBar = new QWidget(this);
    m_navBar->setObjectName("secondaryNavBar");
    m_navBar->setFixedWidth(320);

    m_contentArea   = new QWidget(this);
    m_contentLayout = new QStackedLayout(m_contentArea);
    m_contentLayout->setContentsMargins(0, 0, 0, 0);

    layout->addWidget(m_navBar);
    layout->addWidget(m_contentArea, 1);

    m_navManager = new SecondaryNavigationManager(m_navBar, m_contentLayout, this);
}

void PageWithSubNav::addSubPage(const QString &id,
                                const QString &titleKey,
                                PageWidget    *page)
{
    // 建立 titleKey → navId 映射，供后续 setSubItemVisible 使用
    m_keyToId[titleKey] = id;

    if (m_navManager)
        m_navManager->addNavigation(id, titleKey, page);
}

void PageWithSubNav::setSubItemVisible(const QString &titleKey, bool visible)
{
    const QString navId = m_keyToId.value(titleKey, titleKey);
    setSubItemVisibleById(navId, visible);
}

void PageWithSubNav::setSubItemVisibleById(const QString &subPageId, bool visible)
{
    if (m_navManager)
        m_navManager->setNavItemVisible(subPageId, visible);
}

QStringList PageWithSubNav::subNavTitleKeys() const
{
    return m_keyToId.keys();
}

QString PageWithSubNav::currentSubPageId() const
{
    return m_navManager ? m_navManager->currentPageId() : QString();
}

bool PageWithSubNav::isCurrentSubVisible() const
{
    if (!m_navManager)
        return false;
    const QString id = m_navManager->currentPageId();
    return !id.isEmpty() && m_navManager->isNavItemVisible(id);
}

void PageWithSubNav::activateFirstVisibleSubPage()
{
    if (m_navManager)
        m_navManager->activateFirstVisible();
}
