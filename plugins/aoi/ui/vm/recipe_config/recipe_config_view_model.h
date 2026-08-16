#pragma once

/**
 * @file recipe_config_view_model.h
 * @brief 配方配置 ViewModel：列表/文档编辑、相机预览；扫码/Mark/虚拟调测由子 VM 承载
 *
 * 相机采图由 ICameraService 负责；本 VM 转发 frameReady → previewImageReady。
 * 子 VM（`RecipeScanDetectViewModel` / `RecipeMarkCalcViewModel` /
 * `RecipeVirtualInspectViewModel`）自持状态与业务信号，View 经 `*Vm()` 直连。
 */

#include "viewmodel.h"
#include "command.h"
#include "recipe_config_model.h"

#include <QImage>
#include <QString>

#include <functional>
#include <memory>

namespace aoi {

class RecipeScanDetectViewModel;
class RecipeMarkCalcViewModel;
class RecipeVirtualInspectViewModel;

class RecipeConfigViewModel : public ViewModel
{
    Q_OBJECT

public:
    static constexpr const char *PROP_RECIPES        = "recipes";
    static constexpr const char *PROP_CURRENT_ID     = "currentRecipeId";
    static constexpr const char *PROP_DOCUMENT       = "document";
    static constexpr const char *PROP_STATUS_BAR     = "statusBar";
    static constexpr const char *PROP_SEARCH_KEYWORD = "searchKeyword";
    static constexpr const char *PROP_STATUS         = "status";

    VIEWMODEL_PROPERTY(QString, searchKeyword, SearchKeyword)
    VIEWMODEL_PROPERTY(QString, currentRecipeId, CurrentRecipeId)
    VIEWMODEL_PROPERTY(QString, status, Status)

public:
    RecipeDocument document() const { return getProperty(PROP_DOCUMENT).value<RecipeDocument>(); }
    void setDocument(const RecipeDocument &value);
    void modifyDocument(const std::function<void(RecipeDocument &)> &modifier);

signals:
    void searchKeywordChanged();
    void currentRecipeIdChanged();
    void statusChanged();
    void recipesChanged();
    void documentChanged();
    void statusBarChanged();
    void previewImageReady(QImage image);
    void previewStopped();

public:
    explicit RecipeConfigViewModel(const QString &pageKey, QObject *parent = nullptr);
    ~RecipeConfigViewModel() override;

    const QVector<RecipeListItem> &recipes() const { return m_recipes; }
    RecipeStatusBar                statusBar() const { return m_statusBar; }

    RecipeScanDetectViewModel      *scanDetectVm() const { return m_scanVm; }
    RecipeMarkCalcViewModel        *markCalcVm() const { return m_markVm; }
    RecipeVirtualInspectViewModel  *virtualInspectVm() const { return m_virtualInspectVm; }

    Command *addCommand() const { return m_addCommand; }
    Command *copyCommand() const { return m_copyCommand; }
    Command *removeCommand() const { return m_removeCommand; }
    Command *saveCommand() const { return m_saveCommand; }
    Command *setDefaultCommand() const { return m_setDefaultCommand; }
    Command *snapCommand() const { return m_snapCommand; }

    bool isLivePreviewActive() const;

    // ── 子 VM 宿主接口（预览帧、状态栏）────────────────────────────
    const QImage &lastPreviewImage() const { return m_lastPreviewImage; }
    void          setLastPreviewImage(const QImage &image) { m_lastPreviewImage = image; }
    bool          previewImageFromFile() const { return m_previewImageFromFile; }
    void          notifyStatus(const QString &text);
    void          applyCameraPreviewFrame(QImage image);
    void          ensureCameraConnections();

public slots:
    void loadRecipeList();
    void selectRecipe(const QString &recipeId);
    void applySearch(const QString &keyword);
    bool addRecipe(const QString &name);
    bool renameRecipe(const QString &recipeId, const QString &newName);
    QString exportAllRecipes(const QString &dirPath);
    QString importRecipesFromFiles(const QStringList &filePaths);
    void setPreviewImage(QImage image, const QString &sourcePath = {});
    void clearPreviewImageSourcePath();
    QString previewImageSourcePath() const { return m_previewImageSourcePath; }
    void snapFromToolbar(const QString &cameraId, int previewWinId);
    bool tryStartLivePreview(const QString &cameraId);
    void stopLivePreview(const QString &cameraId, bool deferHardwareStop = false);
    bool moveCaptureToPhysicalXY(const QString &cameraId, double xMm, double yMm);
    bool moveCaptureToPhysicalXYZ(const QString &cameraId, double xMm, double yMm, double zMm);

private:
    bool validateRecipeName(const QString &name, QString *errorKey,
                            const QString &excludeId = {});
    void updateStatusBarFromDocument();
    void clearMarkLocateCache();

    QVector<RecipeListItem> m_recipes;
    RecipeStatusBar         m_statusBar;

    RecipeScanDetectViewModel     *m_scanVm           = nullptr;
    RecipeMarkCalcViewModel       *m_markVm           = nullptr;
    RecipeVirtualInspectViewModel *m_virtualInspectVm = nullptr;

    FunctionalCommand *m_addCommand         = nullptr;
    FunctionalCommand *m_copyCommand       = nullptr;
    FunctionalCommand *m_removeCommand     = nullptr;
    FunctionalCommand *m_saveCommand     = nullptr;
    FunctionalCommand *m_setDefaultCommand = nullptr;
    FunctionalCommand *m_snapCommand       = nullptr;

    bool m_cameraConnectionsEstablished = false;
    QImage                  m_lastPreviewImage;
    QString                 m_previewImageSourcePath;
    bool                    m_previewImageFromFile = false;
    QString                 m_pageKey;
};

} // namespace aoi
