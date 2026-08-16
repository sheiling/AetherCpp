#include "pagewidget.h"
#include "languagemanager.h"
#include "thememanager.h"

PageWidget::PageWidget(QWidget *parent)
    : QWidget(parent)
    , m_loaded(false)
{
    connect(LM, &LanguageManager::languageChanged,
            this, &PageWidget::onLanguageChanged);
    connect(TM, &ThemeManager::themeChanged,
            this, &PageWidget::onThemeChanged);
    connect(TM, &ThemeManager::stylesheetUpdated,
            this, &PageWidget::onThemeChanged);
}

void PageWidget::onLanguageChanged()
{
    if (m_loaded) retranslateUi();
}

void PageWidget::onThemeChanged()
{
    if (m_loaded) applyTheme();
}

void PageWidget::ensureLoaded()
{
    if (!m_loaded) {
        onLoad();
        m_loaded = true;
    }
}

void PageWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    ensureLoaded();
    onShow();
}

void PageWidget::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
    onHide();
}
