#pragma once

#include "uibase_global.h"
#include <QWidget>

// Forward declare to avoid circular includes; actual includes in .cpp
class LanguageManager;
class ThemeManager;

/**
 * @brief 页面基类，支持懒加载
 */
class UIBASE_EXPORT PageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PageWidget(QWidget *parent = nullptr);
    virtual ~PageWidget() = default;

    void ensureLoaded();
    bool isLoaded() const { return m_loaded; }

    QString pageId() const    { return m_pageId; }
    void setPageId(const QString &id)    { m_pageId = id; }

    QString pageTitle() const { return m_pageTitle; }
    void setPageTitle(const QString &title) { m_pageTitle = title; }

protected:
    virtual void onLoad()        {}
    virtual void onShow()        {}
    virtual void onHide()        {}
    virtual void retranslateUi() {}
    virtual void applyTheme()    {}

    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private slots:
    void onLanguageChanged();
    void onThemeChanged();

private:
    bool    m_loaded;
    QString m_pageId;
    QString m_pageTitle;
};
