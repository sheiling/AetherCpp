/**
 * @file recipe_config_view_model.cpp
 * @brief 配方 ViewModel：Command 注册、仓储 CRUD、相机预览；功能委托子 VM
 */

#include "recipe_config_view_model.h"

#include "recipe_scan_detect_view_model.h"
#include "recipe_mark_calc_view_model.h"
#include "recipe_virtual_inspect_view_model.h"

#include "system_config_model.h"
#include "service/param_change/param_change_audit_manager.h"

#include <icamera_service.h>
#include <imotion_control_service.h>

#include <common/utility/diagnostics/timing_debug.h>

#include <command.h>

#include "extensionsystem/pluginmanager.h"
#include "uibase/languagemanager.h"
#include "aoi_plugin_name.h"
#include "uibase/permission/perm_catalog_scope.h"
#include "uibase/permission/permservice.h"

#include <Logger.h>

using namespace ExtensionSystem;

namespace aoi {

RecipeConfigViewModel::RecipeConfigViewModel(const QString &pageKey, QObject *parent)
    : ViewModel(parent)
    , m_pageKey(pageKey)
{
    const PermCatalogScope catalogScope(aoi::pluginName());

    qRegisterMetaType<RecipeDocument>("RecipeDocument");
    qRegisterMetaType<aoi::CurrentInspectionSnapshot>("aoi::CurrentInspectionSnapshot");

    registerProperty(PROP_SEARCH_KEYWORD, QString());
    registerProperty(PROP_DOCUMENT, QVariant::fromValue(RecipeDocument{}));
    registerProperty(PROP_CURRENT_ID, RecipeRepository::defaultRecipeId());
    registerProperty(PROP_STATUS, TR("aoi.recipe.status.ready"));

    m_scanVm           = new RecipeScanDetectViewModel(this, this);
    m_markVm           = new RecipeMarkCalcViewModel(this, this);
    m_virtualInspectVm = new RecipeVirtualInspectViewModel(this, this);

    m_addCommand = new FunctionalCommand([this]() { /* 由 View 弹窗后调用 addRecipe */ }, nullptr,
                                         this);
    m_addCommand->setName(QStringLiteral("recipe.add"));

    m_copyCommand = new FunctionalCommand(
        [this]() {
            RecipeDocument doc = document();
            const QString id   = RecipeRepository::generateNewRecipeId();
            doc.id             = id;
            doc.basic.name     = document().basic.name + QStringLiteral("-COPY");
            RecipeRepository::save(doc);
            loadRecipeList();
            selectRecipe(id);
            setStatus(TR("aoi.recipe.action.copied"));
        },
        nullptr, this);
    m_copyCommand->setName(QStringLiteral("recipe.copy"));

    m_removeCommand = new FunctionalCommand(
        [this]() {
            if (document().id == RecipeRepository::defaultRecipeId()) {
                setStatus(TR("aoi.recipe.action.remove_default_denied"));
                return;
            }
            const QString removed = document().id;
            if (!RecipeRepository::remove(removed)) {
                setStatus(TR("aoi.recipe.action.remove_failed"));
                return;
            }
            loadRecipeList();
            selectRecipe(RecipeRepository::defaultRecipeId());
            setStatus(TR("aoi.recipe.action.removed"));
        },
        nullptr, this);
    m_removeCommand->setName(QStringLiteral("recipe.remove"));

    m_saveCommand = new FunctionalCommand(
        [this]() {
            const RecipeDocument before = document();
            const QString        prevId = currentRecipeId();
            if (RecipeRepository::save(document(), prevId)) {
                ParamAuditContext ctx;
                ctx.moduleCode = QStringLiteral("aoi.recipe");
                ctx.documentId = document().id;
                ctx.sourcePage = QStringLiteral("产品配方配置");
                ParamChangeAuditManager::instance().audit(before, document(), ctx);
                setStatus(TR("aoi.recipe.action.saved"));
            } else {
                setStatus(TR("aoi.recipe.action.save_failed"));
            }
        },
        nullptr, this);
    m_saveCommand->setName(QStringLiteral("recipe.save"));

    m_setDefaultCommand = new FunctionalCommand(
        [this]() {
            if (RecipeRepository::setDefault(document().id)) {
                loadRecipeList();
                setStatus(TR("aoi.recipe.status.set_default"));
            }
        },
        nullptr, this);
    m_setDefaultCommand->setName(QStringLiteral("recipe.setDefault"));

    m_snapCommand = new FunctionalCommand(
        [this]() { snapFromToolbar(QStringLiteral("main"), 2); },
        [this]() {
            auto *camSvc = PluginManager::getInterface<ICameraService>();
            if (!camSvc)
                return false;
            const QStringList names = camSvc->captureNames();
            return !names.isEmpty() && camSvc->isCaptureReady(names.first());
        },
        this);
    m_snapCommand->setName(QStringLiteral("recipe.snap"));

    if (!m_pageKey.isEmpty()) {
        PermService *ps = PermService::instance();
        ps->registerAction(QStringLiteral("recipe.add"), m_pageKey,
                           QStringLiteral("aoi.recipe.action.add"));
        ps->registerAction(QStringLiteral("recipe.copy"), m_pageKey,
                           QStringLiteral("aoi.recipe.action.copy"));
        ps->registerAction(QStringLiteral("recipe.remove"), m_pageKey,
                           QStringLiteral("aoi.recipe.action.remove"));
        ps->registerAction(QStringLiteral("recipe.save"), m_pageKey,
                           QStringLiteral("aoi.recipe.action.save"));
        ps->registerAction(QStringLiteral("recipe.setDefault"), m_pageKey,
                           QStringLiteral("aoi.recipe.action.set_default"));
        ps->registerAction(QStringLiteral("recipe.scanDetect"), m_pageKey,
                           QStringLiteral("aoi.recipe.toolbar.scan_detect"));
        ps->registerAction(QStringLiteral("recipe.inspect"), m_pageKey,
                           QStringLiteral("aoi.recipe.toolbar.inspect"));
        ps->registerAction(QStringLiteral("recipe.snap"), m_pageKey,
                           QStringLiteral("aoi.recipe.toolbar.snap"));
    }

    connect(this, &RecipeConfigViewModel::documentChanged, this, [this]() {
        updateStatusBarFromDocument();
        emit statusBarChanged();
    });

    loadRecipeList();
    selectRecipe(RecipeRepository::defaultRecipeId());
}

RecipeConfigViewModel::~RecipeConfigViewModel() = default;

void RecipeConfigViewModel::setDocument(const RecipeDocument &value)
{
    assignProperty(PROP_DOCUMENT, QVariant::fromValue(value));
    emit documentChanged();
}

void RecipeConfigViewModel::modifyDocument(const std::function<void(RecipeDocument &)> &modifier)
{
    if (!modifier)
        return;
    RecipeDocument doc = document();
    modifier(doc);
    setDocument(doc);
}

bool RecipeConfigViewModel::isLivePreviewActive() const
{
    const auto *camSvc = PluginManager::getInterface<ICameraService>();
    return camSvc && camSvc->isPreviewActive();
}

void RecipeConfigViewModel::ensureCameraConnections()
{
    if (m_cameraConnectionsEstablished)
        return;

    auto *camSvc = PluginManager::getInterface<ICameraService>();
    if (!camSvc)
        return;

    connect(camSvc, &ICameraService::frameReady, this, [this, camSvc](QImage image) {
        if (image.isNull())
            return;

        const QString modeLabel =
            camSvc->isPreviewActive() ? QStringLiteral("实时预览") : QStringLiteral("单帧");
        common::utility::TimingDebugScope vmTiming(
            common::utility::TimingDebugKeys::CaptureTiming, "UI.预览接收",
            modeLabel.toStdString());

        emit previewImageReady(image);
        image.swap(m_lastPreviewImage);
        m_previewImageSourcePath.clear();
        m_previewImageFromFile = false;
        if (!camSvc->isPreviewActive())
            setStatus(TR("aoi.recipe.snap.success"));
    });
    connect(camSvc, &ICameraService::previewActiveChanged, this, [this](bool active) {
        if (!active) {
            setStatus(TR("aoi.recipe.preview.stopped"));
            emit previewStopped();
        }
    });
    connect(camSvc, &ICameraService::captureError, this, [this](const QString &message) {
        setStatus(message);
    });
    connect(camSvc, &ICameraService::offline, this, [this](const QString &) {
        setStatus(TR("aoi.recipe.preview.offline"));
        emit previewStopped();
    });

    m_cameraConnectionsEstablished = true;
}

void RecipeConfigViewModel::applyCameraPreviewFrame(QImage image)
{
    if (image.isNull())
        return;
    image.swap(m_lastPreviewImage);
    emit previewImageReady(m_lastPreviewImage);
    m_previewImageSourcePath.clear();
    m_previewImageFromFile = false;
}

void RecipeConfigViewModel::loadRecipeList()
{
    m_recipes = RecipeRepository::listRecipes();
    emit recipesChanged();
}

void RecipeConfigViewModel::selectRecipe(const QString &recipeId)
{
    RecipeDocument doc;
    if (!RecipeRepository::load(recipeId, doc))
        return;
    setCurrentRecipeId(recipeId);
    clearMarkLocateCache();
    setDocument(doc);
}

void RecipeConfigViewModel::clearMarkLocateCache()
{
    if (m_markVm)
        m_markVm->clearLocateCache();
}

void RecipeConfigViewModel::applySearch(const QString &keyword)
{
    setSearchKeyword(keyword);
    loadRecipeList();
    if (!keyword.isEmpty()) {
        QVector<RecipeListItem> filtered;
        for (const auto &r : m_recipes) {
            if (r.name.contains(keyword, Qt::CaseInsensitive))
                filtered.append(r);
        }
        m_recipes = filtered;
        emit recipesChanged();
    }
}

bool RecipeConfigViewModel::addRecipe(const QString &name)
{
    QString errorKey;
    if (!validateRecipeName(name, &errorKey)) {
        setStatus(TR(errorKey));
        return false;
    }

    const QString trimmed = name.trimmed();
    const QString id      = RecipeRepository::generateNewRecipeId();
    RecipeDocument doc    = RecipeDocument::defaultDemo(id, trimmed);
    RecipeRepository::save(doc);
    loadRecipeList();
    selectRecipe(id);
    setStatus(TR("aoi.recipe.action.added"));
    return true;
}

bool RecipeConfigViewModel::renameRecipe(const QString &recipeId, const QString &newName)
{
    QString errorKey;
    if (!validateRecipeName(newName, &errorKey, recipeId)) {
        setStatus(TR(errorKey));
        return false;
    }

    RecipeDocument doc;
    if (!RecipeRepository::load(recipeId, doc))
        return false;

    const QString trimmed = newName.trimmed();
    if (doc.basic.name == trimmed)
        return true;

    doc.basic.name = trimmed;
    RecipeRepository::save(doc);

    if (recipeId == currentRecipeId())
        setDocument(doc);

    loadRecipeList();
    setStatus(TR("aoi.recipe.action.renamed"));
    return true;
}

bool RecipeConfigViewModel::validateRecipeName(const QString &name, QString *errorKey,
                                               const QString &excludeId)
{
    const QString trimmed = name.trimmed();
    if (trimmed.isEmpty()) {
        if (errorKey)
            *errorKey = QStringLiteral("aoi.recipe.add_dialog.empty_name");
        return false;
    }

    for (const auto &r : RecipeRepository::listRecipes()) {
        if (r.id == excludeId)
            continue;
        if (r.name.compare(trimmed, Qt::CaseInsensitive) == 0) {
            if (errorKey)
                *errorKey = QStringLiteral("aoi.recipe.add_dialog.duplicate_name");
            return false;
        }
    }
    return true;
}

void RecipeConfigViewModel::updateStatusBarFromDocument()
{
    m_statusBar.currentRecipeName = document().basic.name;
    m_statusBar.canSave           = true;
    emit statusBarChanged();
}

void RecipeConfigViewModel::setPreviewImage(QImage image, const QString &sourcePath)
{
    if (image.isNull())
        return;

    m_previewImageSourcePath = sourcePath;
    m_previewImageFromFile   = !sourcePath.isEmpty();
    m_lastPreviewImage.swap(image);
    emit previewImageReady(m_lastPreviewImage);
}

void RecipeConfigViewModel::clearPreviewImageSourcePath()
{
    m_previewImageSourcePath.clear();
    m_previewImageFromFile = false;
}

void RecipeConfigViewModel::notifyStatus(const QString &text)
{
    if (status() == text) {
        emit statusChanged();
        return;
    }
    setStatus(text);
}

QString RecipeConfigViewModel::exportAllRecipes(const QString &dirPath)
{
    const QString trimmed = dirPath.trimmed();
    if (trimmed.isEmpty())
        return QStringLiteral("全部导出失败");

    RecipeExportReport report;
    report.exportDirectory = trimmed;
    RecipeRepository::exportAllToDirectory(trimmed, report);
    RecipeRepository::writeExportLog(report);
    return report.overallMessage;
}

QString RecipeConfigViewModel::importRecipesFromFiles(const QStringList &filePaths)
{
    if (filePaths.isEmpty())
        return QStringLiteral("全部导入失败");

    RecipeImportReport report;
    RecipeRepository::importFromJsonFiles(filePaths, report);
    RecipeRepository::writeImportLog(report);

    if (report.overallOk || report.items.size() > 0) {
        loadRecipeList();
        if (!report.items.isEmpty()) {
            const QString lastOkId = [&report]() {
                for (int i = report.items.size() - 1; i >= 0; --i) {
                    if (report.items[i].ok && !report.items[i].recipeId.isEmpty())
                        return report.items[i].recipeId;
                }
                return QString();
            }();
            if (!lastOkId.isEmpty())
                selectRecipe(lastOkId);
            else if (!currentRecipeId().isEmpty() && RecipeRepository::contains(currentRecipeId()))
                selectRecipe(currentRecipeId());
            else
                selectRecipe(RecipeRepository::defaultRecipeId());
        }
    }

    return report.overallMessage;
}

void RecipeConfigViewModel::snapFromToolbar(const QString &cameraId, int previewWinId)
{
    Q_UNUSED(previewWinId);

    auto *camSvc = PluginManager::getInterface<ICameraService>();
    if (!camSvc) {
        setStatus(TR("aoi.recipe.snap.no_service"));
        return;
    }
    if (!camSvc->isCaptureReady(cameraId)) {
        setStatus(TR("aoi.recipe.snap.not_connected"));
        return;
    }

    ensureCameraConnections();

    if (camSvc->isPreviewActive())
        camSvc->stopPreview(false);

    const int exposureUs = document().camera.exposureUs;
    setStatus(TR("aoi.recipe.snap.grabbing"));

    if (!camSvc->snapOnce(cameraId, exposureUs))
        setStatus(TR("aoi.recipe.snap.failed"));
}

bool RecipeConfigViewModel::tryStartLivePreview(const QString &cameraId)
{
    auto *camSvc = PluginManager::getInterface<ICameraService>();
    if (!camSvc) {
        setStatus(TR("aoi.recipe.preview.no_service"));
        return false;
    }

    ensureCameraConnections();

    if (camSvc->isPreviewActive()) {
        if (camSvc->activeCaptureName() == cameraId)
            return true;
        camSvc->stopPreview(false);
    }

    if (!camSvc->isCaptureReady(cameraId)) {
        setStatus(TR("aoi.recipe.preview.not_connected"));
        return false;
    }

    const int exposureUs = document().camera.exposureUs;
    if (!camSvc->startPreview(cameraId, exposureUs)) {
        setStatus(TR("aoi.recipe.preview.failed"));
        return false;
    }

    setStatus(TR("aoi.recipe.toolbar.preview"));
    return true;
}

void RecipeConfigViewModel::stopLivePreview(const QString &cameraId, bool deferHardwareStop)
{
    Q_UNUSED(cameraId);

    auto *camSvc = PluginManager::getInterface<ICameraService>();
    if (!camSvc || !camSvc->isPreviewActive())
        return;

    camSvc->stopPreview(deferHardwareStop);
}

bool RecipeConfigViewModel::moveCaptureToPhysicalXY(const QString &cameraId, double xMm, double yMm)
{
    auto *motionSvc =
        PluginManager::getInterface<motion_control_plugin::IMotionControlService>();
    if (!motionSvc || !motionSvc->isReady()) {
        notifyStatus(TR("aoi.recipe.nav.motion_not_ready"));
        return false;
    }

    auto *camSvc = PluginManager::getInterface<ICameraService>();
    if (!camSvc) {
        notifyStatus(TR("aoi.recipe.snap.no_service"));
        return false;
    }

    const QStringList axes = camSvc->captureAxisNames(cameraId);
    if (axes.size() < 2) {
        notifyStatus(TR("aoi.recipe.nav.axis_missing"));
        return false;
    }

    const bool okX = motionSvc->moveToPosition(axes.at(0), xMm, true);
    const bool okY = motionSvc->moveToPosition(axes.at(1), yMm, true);
    if (okX && okY) {
        motionSvc->refreshAxisStates();
        notifyStatus(TR("aoi.recipe.nav.moving"));
        return true;
    }

    notifyStatus(TR("aoi.recipe.nav.move_failed"));
    return false;
}

bool RecipeConfigViewModel::moveCaptureToPhysicalXYZ(const QString &cameraId, double xMm, double yMm,
                                                     double zMm)
{
    auto *motionSvc =
        PluginManager::getInterface<motion_control_plugin::IMotionControlService>();
    if (!motionSvc || !motionSvc->isReady()) {
        notifyStatus(TR("aoi.recipe.nav.motion_not_ready"));
        return false;
    }

    auto *camSvc = PluginManager::getInterface<ICameraService>();
    if (!camSvc) {
        notifyStatus(TR("aoi.recipe.snap.no_service"));
        return false;
    }

    const QStringList axes = camSvc->captureAxisNames(cameraId);
    if (axes.size() < 2) {
        notifyStatus(TR("aoi.recipe.nav.axis_missing"));
        return false;
    }

    const bool okX = motionSvc->moveToPosition(axes.at(0), xMm, true);
    const bool okY = motionSvc->moveToPosition(axes.at(1), yMm, true);
    bool         okZ = true;
    if (axes.size() > 2)
        okZ = motionSvc->moveToPosition(axes.at(2), zMm, true);

    if (okX && okY && okZ) {
        motionSvc->refreshAxisStates();
        notifyStatus(TR("aoi.recipe.nav.moving"));
        return true;
    }

    notifyStatus(TR("aoi.recipe.nav.move_failed"));
    return false;
}

} // namespace aoi
