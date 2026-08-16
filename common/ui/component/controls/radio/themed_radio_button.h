#pragma once

/**
 * @file themed_radio_button.h
 * @brief 主题 QRadioButton：动态 QSS，指示器色来自 ThemeTokens
 *
 * 样式见 radio_button_style.h；EnabledChange 时刷新 disabled 态。
 */

#include <QRadioButton>
#include "component_global.h"

class QEvent;

namespace ui {

/** 随 ThemeTokens 刷新的 QRadioButton */
class UI_COMPONENT_EXPORT ThemedRadioButton : public QRadioButton
{
    Q_OBJECT

public:
    explicit ThemedRadioButton(QWidget *parent = nullptr);
    explicit ThemedRadioButton(const QString &text, QWidget *parent = nullptr);

protected:
    void changeEvent(QEvent *event) override;

private:
    void bindTheme();
    void refreshStyle();
};

/** 工厂：创建带文案的 ThemedRadioButton */
UI_COMPONENT_EXPORT ThemedRadioButton *makeRadioButton(const QString &text, QWidget *parent = nullptr);

} // namespace ui
