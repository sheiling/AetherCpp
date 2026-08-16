#pragma once

/**
 * @file themed_form_label.h
 * @brief 表单字段标签：翻译键、语言切换自动刷新；字号由 setPixelSize 可选指定
 *
 * 未调用 setPixelSize 时字体由父级 QSS / 主题决定；主题切换后可用 applyFontsIn 批量重应用字号。
 */

#include <QLabel>
#include <QString>

#include <optional>
#include "component_global.h"

class QWidget;

namespace ui {

/** 带 TR 翻译键的表单标签（objectName: themedFormLabel） */
class UI_COMPONENT_EXPORT ThemedFormLabel : public QLabel
{
    Q_OBJECT

public:
    explicit ThemedFormLabel(const QString &translationKey, QWidget *parent = nullptr);

    QString translationKey() const { return m_translationKey; }
    void setTranslationKey(const QString &key);

    /** 显式指定像素字号；未调用时由父级 QSS / 主题决定字体 */
    void setPixelSize(int pixelSize);
    bool hasPixelSize() const { return m_pixelSize.has_value(); }
    std::optional<int> pixelSize() const { return m_pixelSize; }

    /** 按当前 translationKey 重新 TR 文案 */
    void retranslate();
    /** 按已设 pixelSize 与 themeResolvedFontFamily 更新 QFont */
    void applyFontMetrics();

    /** 对子树中全部 ThemedFormLabel 重新应用已设字号（主题切换后） */
    static void applyFontsIn(QWidget *root);
    /** 对子树中全部 ThemedFormLabel 重新 retranslate */
    static void retranslateIn(QWidget *root);

private:
    QString              m_translationKey;
    std::optional<int>   m_pixelSize;
};

/** 工厂：创建带翻译键的 ThemedFormLabel */
UI_COMPONENT_EXPORT ThemedFormLabel *makeFormLabel(const QString &translationKey, QWidget *parent = nullptr);
UI_COMPONENT_EXPORT ThemedFormLabel *makeFormLabel(const QString &translationKey, QWidget *parent, int pixelSize);

} // namespace ui
