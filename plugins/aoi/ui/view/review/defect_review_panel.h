#pragma once

/**
 * @file defect_review_panel.h
 * @brief 复判区：双图预览、快捷键说明、坐标表
 */

#include "vm/review/defect_review_types.h"

#include <QWidget>

class QLabel;

namespace ui {
class CursorToast;
class ImagePreview;
class ThemedPushButton;
class ThemedTableWidget;
} // namespace ui

namespace aoi::review {

class DefectReviewViewModel;
class DefectReviewShortcutPanel;

class DefectReviewPanel : public QWidget
{
    Q_OBJECT

public:
    explicit DefectReviewPanel(DefectReviewViewModel *viewModel, QWidget *parent = nullptr);

    void retranslateUi();
    void refreshFromViewModel();

private:
    void buildUi();
    void wireViewModel();
    void refreshImages();
    void syncTableFromViewModel();
    void selectTableRow(int row);

    static constexpr int kTableColumnCount = 6;

    DefectReviewViewModel     *m_viewModel     = nullptr;
    ui::ImagePreview          *m_originPreview = nullptr;
    ui::ImagePreview          *m_resultPreview = nullptr;
    QLabel                    *m_originPlaceholder = nullptr;
    QLabel                    *m_resultPlaceholder = nullptr;
    DefectReviewShortcutPanel *m_shortcutPanel  = nullptr;
    ui::ThemedPushButton      *m_confirmButton  = nullptr;
    ui::ThemedTableWidget     *m_table         = nullptr;
    ui::CursorToast           *m_statusToast   = nullptr;
};

} // namespace aoi::review
