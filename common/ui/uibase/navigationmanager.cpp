#include "navigationmanager.h"
#include "languagemanager.h"
#include "thememanager.h"
#include <QDebug>

namespace {
QColor themeColorOrDefault(const QString &name, const QColor &fallback)
{
    const QColor c = TM ? TM->themeColor(name) : QColor{};
    return c.isValid() ? c : fallback;
}
}

// ─────────────────────────────────────────────
//  PrimaryNavigationManager
// ─────────────────────────────────────────────

PrimaryNavigationManager::PrimaryNavigationManager(QWidget *navBar,
                                                   QStackedLayout *contentLayout,
                                                   QObject *parent)
    : QObject(parent)
    , m_navBar(navBar)
    , m_contentLayout(contentLayout)
{
    m_navLayout = new QHBoxLayout(m_navBar);
    m_navLayout->setContentsMargins(10, 0, 0, 0);
    m_navLayout->setSpacing(0);
    m_navLayout->addStretch();

    connect(LM, &LanguageManager::languageChanged,
            this, &PrimaryNavigationManager::onLanguageChanged);
}

void PrimaryNavigationManager::addRightWidget(QWidget *widget)
{
    if (widget) m_navLayout->addWidget(widget);
}

void PrimaryNavigationManager::addNavigation(const QString &id,
                                             const QString &titleKey,
                                             PageWidget *page)
{
    if (m_items.contains(id) || !page) return;

    NavigationItem item(id, titleKey, page);
    page->setPageId(id);
    page->setPageTitle(TR(titleKey));

    QPushButton *button = new QPushButton(TR(titleKey), m_navBar);
    button->setObjectName("primaryNavBtn");
    button->setProperty("navId", id);
    button->setFixedHeight(32);
    button->setMinimumWidth(80);
    button->setCheckable(true);
    button->setCursor(Qt::PointingHandCursor);
    connect(button, &QPushButton::clicked,
            this, &PrimaryNavigationManager::onButtonClicked);

    item.button = button;
    m_items[id] = item;
    m_navOrder.append(id);

    m_navLayout->insertWidget(m_navLayout->count() - 1, button);
    m_contentLayout->addWidget(page);

    if (m_items.size() == 1) switchToPage(id);
}

void PrimaryNavigationManager::switchToPage(const QString &id)
{
    if (!m_items.contains(id) || m_currentPageId == id) return;
    m_currentPageId = id;
    m_contentLayout->setCurrentWidget(m_items[id].page);
    updateButtonStates();
    emit pageChanged(id);
}

void PrimaryNavigationManager::switchToFirstVisible()
{
    for (const QString &id : m_navOrder) {
        if (!m_items.contains(id))
            continue;
        const NavigationItem &item = m_items[id];
        if (item.button && !item.button->isHidden()) {
            switchToPage(id);
            return;
        }
    }
    m_currentPageId.clear();
}

void PrimaryNavigationManager::activateFirstVisible()
{
    switchToFirstVisible();
}

void PrimaryNavigationManager::setNavItemVisible(const QString &id, bool visible)
{
    if (!m_items.contains(id)) return;
    auto &item = m_items[id];
    if (item.button) {
        // 使用 isHidden() 判断显式隐藏状态，避免窗口未显示时 isVisible() 全为 false 的误判
        bool isIntendedVisible = !item.button->isHidden();
        if (isIntendedVisible == visible) return;

        item.button->setVisible(visible);

        // 若当前页被隐藏，自动跳转到第一个可见页
        if (!visible && m_currentPageId == id) {
            switchToFirstVisible();
        }
    }
}

bool PrimaryNavigationManager::isNavItemVisible(const QString &id) const
{
    if (!m_items.contains(id)) return false;
    const auto &item = m_items[id];
    return item.button && !item.button->isHidden();
}

QStringList PrimaryNavigationManager::registeredIds() const
{
    return m_navOrder;
}

PageWidget *PrimaryNavigationManager::currentPage() const
{
    return m_items.contains(m_currentPageId) ? m_items[m_currentPageId].page : nullptr;
}

void PrimaryNavigationManager::onButtonClicked()
{
    if (auto *btn = qobject_cast<QPushButton *>(sender()))
        switchToPage(btn->property("navId").toString());
}

void PrimaryNavigationManager::updateButtonStates()
{
    for (auto &item : m_items)
        if (item.button) item.button->setChecked(item.id == m_currentPageId);
}

void PrimaryNavigationManager::onLanguageChanged()
{
    for (auto &item : m_items) {
        if (item.button) item.button->setText(TR(item.titleKey));
        if (item.page)   item.page->setPageTitle(TR(item.titleKey));
    }
}

// ─────────────────────────────────────────────
//  SecondaryNavigationManager
// ─────────────────────────────────────────────

SecondaryNavigationManager::SecondaryNavigationManager(QWidget *navBar,
                                                       QStackedLayout *contentLayout,
                                                       QObject *parent)
    : QObject(parent)
    , m_navBar(navBar)
    , m_contentLayout(contentLayout)
{
    m_navLayout = new QVBoxLayout(m_navBar);
    m_navLayout->setContentsMargins(14, 14, 14, 14);
    m_navLayout->setSpacing(10);
    m_navLayout->addStretch();

    auto applySecondaryNavStyle = [this]() {
        const QColor panelBg = themeColorOrDefault(QStringLiteral("secondaryColor"), QColor(QStringLiteral("#1b2f4a")));
        const QColor panelBorder = themeColorOrDefault(QStringLiteral("secondaryLightColor"), QColor(QStringLiteral("#2b4668")));
        const QColor textNormal = themeColorOrDefault(QStringLiteral("secondaryTextColor"), QColor(QStringLiteral("#9fb7d6")));
        const QColor textActive = themeColorOrDefault(QStringLiteral("primaryTextColor"), QColor(QStringLiteral("#ffffff")));
        const QColor hoverBg = themeColorOrDefault(QStringLiteral("primaryLightColor"), QColor(QStringLiteral("#1f4a77")));
        const QColor checkedBg = themeColorOrDefault(QStringLiteral("primaryColor"), QColor(QStringLiteral("#1f4a77")));
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
    };

    applySecondaryNavStyle();
    connect(TM, &ThemeManager::themeChanged, m_navBar, [applySecondaryNavStyle](ThemeManager::Theme) {
        applySecondaryNavStyle();
    });

    connect(LM, &LanguageManager::languageChanged,
            this, &SecondaryNavigationManager::onLanguageChanged);
}

void SecondaryNavigationManager::addNavigation(const QString &id,
                                               const QString &titleKey,
                                               PageWidget *page)
{
    if (m_items.contains(id) || !page) return;

    NavigationItem item(id, titleKey, page);
    page->setPageId(id);
    page->setPageTitle(TR(titleKey));

    QPushButton *button = new QPushButton(TR(titleKey), m_navBar);
    button->setObjectName("secondaryNavBtn");
    button->setProperty("navId", id);
    button->setFixedHeight(40);
    button->setCheckable(true);
    button->setCursor(Qt::PointingHandCursor);
    connect(button, &QPushButton::clicked,
            this, &SecondaryNavigationManager::onButtonClicked);

    item.button = button;
    m_items[id] = item;
    m_navOrder.append(id);

    m_navLayout->insertWidget(m_navLayout->count() - 1, button);
    m_contentLayout->addWidget(page);

    if (m_items.size() == 1) switchToPage(id);
}

void SecondaryNavigationManager::switchToPage(const QString &id)
{
    if (!m_items.contains(id)) return;
    m_contentLayout->setCurrentWidget(m_items[id].page);
    m_currentPageId = id;
    updateButtonStates();
    emit pageChanged(id);
}

void SecondaryNavigationManager::switchToFirstVisible()
{
    for (const QString &id : m_navOrder) {
        if (!m_items.contains(id))
            continue;
        const NavigationItem &item = m_items[id];
        if (item.button && !item.button->isHidden()) {
            switchToPage(id);
            return;
        }
    }
    m_currentPageId.clear();
}

void SecondaryNavigationManager::activateFirstVisible()
{
    switchToFirstVisible();
}

void SecondaryNavigationManager::setNavItemVisible(const QString &id, bool visible)
{
    if (!m_items.contains(id)) return;
    auto &item = m_items[id];
    if (item.button) {
        // 使用 isHidden() 判断显式隐藏状态，而非 isVisible()
        // isVisible() 在窗口未显示时对所有子控件返回 false，
        // 会导致初始化阶段的隐藏调用被误判为"无需操作"而跳过。
        // isHidden() 只有在显式调用 hide() / setVisible(false) 后才为 true，
        // 能正确反映控件的"意图可见性"。
        bool isIntendedVisible = !item.button->isHidden();
        if (isIntendedVisible == visible) return;

        item.button->setVisible(visible);

        if (!visible && m_currentPageId == id) {
            switchToFirstVisible();
        }
    }
}

bool SecondaryNavigationManager::isNavItemVisible(const QString &id) const
{
    if (!m_items.contains(id)) return false;
    const auto &item = m_items[id];
    return item.button && !item.button->isHidden();
}

QStringList SecondaryNavigationManager::registeredIds() const
{
    return m_navOrder;
}

PageWidget *SecondaryNavigationManager::currentPage() const
{
    return m_items.contains(m_currentPageId) ? m_items[m_currentPageId].page : nullptr;
}

void SecondaryNavigationManager::refreshCurrentPage()
{
    if (!m_currentPageId.isEmpty() && m_items.contains(m_currentPageId)) {
        m_contentLayout->setCurrentWidget(m_items[m_currentPageId].page);
        updateButtonStates();
    } else if (!m_items.isEmpty()) {
        switchToFirstVisible();
    }
}

void SecondaryNavigationManager::clear()
{
    for (auto &item : m_items) {
        if (item.button) { m_navLayout->removeWidget(item.button); item.button->deleteLater(); }
        if (item.page)   m_contentLayout->removeWidget(item.page);
    }
    m_items.clear();
    m_navOrder.clear();
    m_currentPageId.clear();
}

void SecondaryNavigationManager::onButtonClicked()
{
    if (auto *btn = qobject_cast<QPushButton *>(sender()))
        switchToPage(btn->property("navId").toString());
}

void SecondaryNavigationManager::updateButtonStates()
{
    for (auto &item : m_items)
        if (item.button) item.button->setChecked(item.id == m_currentPageId);
}

void SecondaryNavigationManager::onLanguageChanged()
{
    for (auto &item : m_items) {
        if (item.button) item.button->setText(TR(item.titleKey));
        if (item.page)   item.page->setPageTitle(TR(item.titleKey));
    }
}
