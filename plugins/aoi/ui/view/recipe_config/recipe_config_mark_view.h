#pragma once

/**
 * @file recipe_config_mark_view.h
 * @brief 配方配置 · Mark 制作子面板
 */

#include "service/vision_bridge/mark_point_locator.h"
#include "binding_session.h"
#include "component/dialog/themed_message_box.h"
#include "recipe_config/recipe_config_widgets.h"
#include "recipe_config_model.h"

#include <functional>
#include <QWidget>

class IGraphicsVew;
class QTimer;

namespace ui {
class ThemedButtonGroup;
class ThemedPushButton;
} // namespace ui

namespace aoi {

class MarkTemplateDialog;
class MarkTemplatePickDialog;
class RecipeConfigViewModel;

/**
 * @brief Mark 制作：双 Mark 切换、模板制作/选用、定位计算与预览 ROI 联动
 */
class RecipeConfigMarkView : public QWidget
{
    Q_OBJECT

public:
    explicit RecipeConfigMarkView(RecipeConfigViewModel *viewModel, QWidget *parent = nullptr);

    void setPreviewContext(IGraphicsVew *graphicsView, int winId);
    void setCameraIdProvider(std::function<QString()> provider);
    void setupExternalConnections(RecipeConfigViewModel *viewModel);

    int activeMarkSlot() const { return m_activeMarkSlot; }

    void setupBindings();
    void updateAllSources();
    void beginSuspendUpdates() { m_bindings.beginSuspendUpdates(); }
    void endSuspendUpdates() { m_bindings.endSuspendUpdates(); }
    void retranslatePanel();
    void refreshMarkTabs();
    void refreshMarkSlotUi(const RecipeDocument &doc, int slotIndex);
    void refreshCompactLayout();

public slots:
    void onMarkTabChanged(int newSlotIndex);

private:
    void buildLayout();
    QWidget *buildMarkParamsSection(QWidget *parent);
    QWidget *buildMarkCalcSection(QWidget *parent);

    void openMarkTemplateDialog();
    void closeMarkTemplateDialog();
    void openMarkTemplatePickDialog();
    void closeMarkTemplatePickDialog();
    void refreshMarkTemplateRoiHint();
    void onMarkTemplateMakeRequested();
    void onMarkTemplatePickConfirmed(const QString &modelPath, const QString &modelName);
    void onMarkTemplateDeleteRequested(const QString &modelPath, const QString &modelName);
    bool applyMarkTemplateToForm(const QString &modelPath, const QString &modelName,
                                 bool showError = true);
    void notifyUser(const QString &text,
                    ui::ThemedMessageBoxIcon icon = ui::ThemedMessageBoxIcon::Information);
    void applyMarkCalcResult(const MarkLocateResult &result);
    void syncActiveMarkSlotToViewModel();

    RecipeConfigViewModel *m_viewModel = nullptr;
    BindingSession         m_bindings;

    IGraphicsVew            *m_graphicsView = nullptr;
    int                      m_previewWinId = 2;
    std::function<QString()> m_cameraIdProvider;

    ui::ThemedButtonGroup      *m_markTabGroup = nullptr;
    int                         m_activeMarkSlot = 0;
    RecipeMarkFormWidgets       m_markWidgets;
    MarkTemplateDialog         *m_markTemplateDialog     = nullptr;
    MarkTemplatePickDialog     *m_markTemplatePickDialog = nullptr;
    QTimer                     *m_markRoiTimer           = nullptr;

    ui::ThemedPushButton *m_pickTemplateBtn = nullptr;
    ui::ThemedPushButton *m_newTemplateBtn  = nullptr;
};

} // namespace aoi
