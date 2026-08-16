/**
 * @file themed_form_label.cpp
 * @brief ThemedFormLabel 实现：TR 文案、可选像素字号、子树批量刷新
 */

#include "controls/label/themed_form_label.h"

#include "theme/theme_font.h"
#include "theme/theme_refresh.h"

#include "uibase/languagemanager.h"

#include <QSizePolicy>

namespace ui {

ThemedFormLabel::ThemedFormLabel(const QString &translationKey, QWidget *parent)
    : QLabel(parent)
    , m_translationKey(translationKey)
{
    setObjectName(QStringLiteral("themedFormLabel"));
    setWordWrap(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    retranslate();

    if (LM) {
        connect(LM, &LanguageManager::languageChanged, this, &ThemedFormLabel::retranslate);
    }
    bindThemeRefresh(this, [this]() { applyFontMetrics(); });
}

void ThemedFormLabel::setTranslationKey(const QString &key)
{
    if (key.isEmpty())
        return;
    m_translationKey = key;
    retranslate();
}

void ThemedFormLabel::setPixelSize(const int pixelSize)
{
    if (pixelSize <= 0)
        return;
    m_pixelSize = pixelSize;
    applyFontMetrics();
}

void ThemedFormLabel::retranslate()
{
    if (!m_translationKey.isEmpty())
        setText(TR(m_translationKey.toUtf8().constData()));
}

void ThemedFormLabel::applyFontMetrics()
{
    if (!m_pixelSize.has_value())
        return;

    QFont f;
    const QString family = themeResolvedFontFamily();
    if (!family.isEmpty())
        f.setFamily(family);
    f.setPixelSize(*m_pixelSize);
    setFont(f);
}

void ThemedFormLabel::applyFontsIn(QWidget *root)
{
    if (!root)
        return;
    for (auto *lbl : root->findChildren<ThemedFormLabel *>())
        lbl->applyFontMetrics();
}

void ThemedFormLabel::retranslateIn(QWidget *root)
{
    if (!root)
        return;
    for (auto *lbl : root->findChildren<ThemedFormLabel *>())
        lbl->retranslate();
}

ThemedFormLabel *makeFormLabel(const QString &translationKey, QWidget *parent)
{
    return new ThemedFormLabel(translationKey, parent);
}

ThemedFormLabel *makeFormLabel(const QString &translationKey, QWidget *parent,
                               const int pixelSize)
{
    auto *lbl = new ThemedFormLabel(translationKey, parent);
    lbl->setPixelSize(pixelSize);
    return lbl;
}

} // namespace ui
