#pragma once

/**
 * @file recipe_config_param_panel.h
 * @brief 配方配置 · 右侧参数面板（六段可折叠 FormGroupBox）
 */

#include <QVector>
#include <QWidget>

namespace ui {

class FormGroupBox;

} // namespace ui

namespace aoi {

class RecipeConfigAlgoView;
class RecipeConfigBasicView;
class RecipeConfigPanelCalibView;
class RecipeConfigCameraView;
class RecipeConfigMarkView;
class RecipeConfigPositionCalibView;
class RecipeConfigViewModel;
struct RecipeDocument;

/**
 * @brief 右侧六段可折叠参数面板：基本/相机/算法/Mark/标定/其它
 */
class RecipeConfigParamPanel : public QWidget
{
    Q_OBJECT

public:
    explicit RecipeConfigParamPanel(RecipeConfigViewModel *viewModel, QWidget *parent = nullptr);

    void setupBindings();
    void updateAllSources();
    void beginSuspendUpdates();
    void endSuspendUpdates();
    void retranslatePanel();
    void refreshFromDocument(const RecipeDocument &doc);
    /** 刷新参数面板字号与紧凑布局（主题切换 / 首屏后调用） */
    void refreshPresentation();

    RecipeConfigMarkView  *markView() const { return m_markView; }
    RecipeConfigPositionCalibView *positionCalibView() const { return m_positionCalibView; }
    const QVector<ui::FormGroupBox *> &formGroups() const { return m_formGroups; }

private:
    void buildLayout();
    ui::FormGroupBox *makeFormGroup(const QString &titleKey, QWidget *sectionContent,
                                    QWidget *parent, bool expanded = true);

    RecipeConfigViewModel *m_viewModel = nullptr;

    RecipeConfigBasicView  *m_basicView  = nullptr;
    RecipeConfigCameraView *m_cameraView = nullptr;
    RecipeConfigAlgoView   *m_algoView   = nullptr;
    RecipeConfigMarkView   *m_markView   = nullptr;
    RecipeConfigPanelCalibView    *m_panelCalibView    = nullptr;
    RecipeConfigPositionCalibView *m_positionCalibView = nullptr;

    QVector<ui::FormGroupBox *> m_formGroups;
};

} // namespace aoi
