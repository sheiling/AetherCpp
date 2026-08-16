#pragma once

/**
 * @file themed_line_edit.h
 * @brief 主题 LineEdit：动态 QSS，颜色跟随 ThemeTokens
 *
 * 样式见 line_edit_style.h；ReadOnlyChange / EnabledChange 时刷新 QSS。
 */

#include <QLineEdit>
#include "component_global.h"

class QEvent;
class QWidget;

namespace ui {

/** 随 ThemeTokens 刷新的 QLineEdit */
class UI_COMPONENT_EXPORT ThemedLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit ThemedLineEdit(QWidget *parent = nullptr);

protected:
    void changeEvent(QEvent *event) override;
    /** 应用 buildLineEditStylesheet；PathLineEdit 可 override */
    virtual void refreshStyle();

private:
    void bindTheme();
};

/** 工厂：创建 ThemedLineEdit */
UI_COMPONENT_EXPORT ThemedLineEdit *makeLineEdit(QWidget *parent = nullptr);

} // namespace ui
