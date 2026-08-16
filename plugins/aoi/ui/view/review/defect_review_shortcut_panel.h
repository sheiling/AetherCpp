#pragma once

/**
 * @file defect_review_shortcut_panel.h
 * @brief 复判区「操作快捷键」说明条（键帽 + 文案，与 §3.1.4 / ViewModel 按键逻辑一致）
 */

#include <QWidget>

class QLabel;

namespace aoi::review {

class DefectReviewShortcutPanel : public QWidget
{
    Q_OBJECT

public:
    explicit DefectReviewShortcutPanel(QWidget *parent = nullptr);

    void retranslateUi();

private:
    void buildUi();
    void applyTheme();

    QLabel *makeKeyCap(const QString &text);
    QWidget *makeShortcutGroup(const QStringList &keyTexts, QLabel **outDesc);

    QLabel *m_titleLabel       = nullptr;
    QLabel *m_prevNextDesc     = nullptr;
    QLabel *m_defectToggleDesc = nullptr;
    QLabel *m_enterDesc        = nullptr;
    QLabel *m_okDesc           = nullptr;
    QLabel *m_resetDesc        = nullptr;
};

} // namespace aoi::review
