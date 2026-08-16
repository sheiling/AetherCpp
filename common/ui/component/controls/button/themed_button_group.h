#pragma once

/**
 * @file themed_button_group.h
 * @brief 主题互斥按钮组：外部传入翻译键列表，选中时发出 currentIndexChanged
 *
 * 按钮样式见 button_group_style.h；支持等宽 stretch 与 LanguageManager 自动 retranslate。
 */

#include <QEvent>
#include <QStringList>
#include <QWidget>
#include "component_global.h"

class QButtonGroup;
class QHBoxLayout;
class QPushButton;

namespace ui {

/** 水平排列的互斥 QPushButton 组，内部 QButtonGroup exclusive */
class UI_COMPONENT_EXPORT ThemedButtonGroup : public QWidget
{
    Q_OBJECT

public:
    explicit ThemedButtonGroup(QWidget *parent = nullptr);

    /** 设置条目翻译键列表，会重建按钮并默认选中第一项 */
    void setItemKeys(const QStringList &translationKeys);
    QStringList itemKeys() const { return m_itemKeys; }

    /** 程序化选中指定索引（越界忽略） */
    void setCurrentIndex(int index);
    int currentIndex() const;

    /** true 时各按钮 stretch=1 等宽；false 时按内容宽度 */
    void setStretchFactors(bool equalWidth);

signals:
    void currentIndexChanged(int index);

protected:
    void changeEvent(QEvent *event) override;

private:
    void bindTheme();
    void refreshStyle();
    void rebuildButtons();
    void retranslateButtons();

    QStringList          m_itemKeys;
    QHBoxLayout         *m_layout       = nullptr;
    QButtonGroup        *m_buttonGroup  = nullptr;
    QList<QPushButton *> m_buttons;
    bool                 m_equalWidth   = true;
    int                  m_currentIndex = -1;
};

} // namespace ui
