/**
 * @file recipe_config_view_model.cpp
 * @brief RecipeConfigViewModel 配方 CRUD、ROI 管理与导入导出实现
 */

#include "recipe_config_view_model.h"

#include "lighten_param_persist.h"

#include <QDateTime>
#include <QMetaType>

/** @brief RecipeConfigViewModel 成员实现 */
namespace lighten {

RecipeConfigViewModel::RecipeConfigViewModel(QObject *parent)
    : ViewModel(parent)
{
    qRegisterMetaType<RecipeDocument>("RecipeDocument");

    registerProperty(PROP_SEARCH_KEYWORD, QString());
    registerProperty(PROP_CURRENT_ID, RecipeRepository::defaultRecipeId());
    registerProperty(PROP_ACTIVE_CAMERA, static_cast<int>(CameraIndex::Main));
    registerProperty(PROP_ACTIVE_COLOR, static_cast<int>(ColorChannel::Red));
    registerProperty(PROP_STATUS, QStringLiteral("就绪"));
    registerProperty(PROP_DOCUMENT, QVariant::fromValue(RecipeDocument{}));

    m_saveCommand = new FunctionalCommand(
        [this]() {
            const QString prevId = currentRecipeId();
            RecipeDocument before;
            if (!prevId.isEmpty())
                RecipeRepository::load(prevId, before);
            const RecipeDocument after = document();
            if (RecipeRepository::save(after, prevId)) {
                ParameterChangeLogger::logRecipeDocumentSave(before, after);
                setCurrentRecipeId(after.id);
                if (status() != QStringLiteral("配方已保存"))
                    setStatus(QStringLiteral("配方已保存"));
                emit sig_saveSucceeded();
            } else {
                setStatus(QStringLiteral("保存配方失败"));
            }
        },
        [this]() { return !document().id.isEmpty(); }, this);
    m_saveCommand->setName(QStringLiteral("recipe.save"));

    m_setDefaultCommand = new FunctionalCommand(
        [this]() {
            if (document().id.isEmpty()) {
                setStatus(QStringLiteral("请先选择配方"));
                return;
            }
            if (RecipeRepository::setDefault(document().id)) {
                setStatus(QStringLiteral("已设为默认配方"));
            } else {
                setStatus(QStringLiteral("设为默认失败"));
            }
        },
        [this]() { return !document().id.isEmpty(); }, this);
    m_setDefaultCommand->setName(QStringLiteral("recipe.setDefault"));

    connect(this, &ViewModel::propertyChanged, this, [this](const QString &name, const QVariant &) {
        if (name != QLatin1String(PROP_DOCUMENT))
            return;
        updateStatusBarFromDocument();
        updateCommandStates();
        emit sig_debugToolbarChanged();
    });
    connect(this, &RecipeConfigViewModel::sig_recipesChanged, this,
            &RecipeConfigViewModel::updateCommandStates);
    connect(RecipeCatalogNotifier::instance(), &RecipeCatalogNotifier::sig_recipeListChanged, this,
            &RecipeConfigViewModel::slot_loadRecipeList);

    slot_load();
    slot_selectRecipe(RecipeRepository::defaultRecipeId());
    updateCommandStates();
}

void RecipeConfigViewModel::slot_load()
{
    RecipeRepository::load();
}

void RecipeConfigViewModel::slot_loadRecipeList()
{
    m_recipes = RecipeRepository::listRecipes();
    emit sig_recipesChanged();
    updateCommandStates();
}

void RecipeConfigViewModel::slot_selectRecipe(const QString &recipeId)
{
    RecipeDocument doc;
    if (!RecipeRepository::load(recipeId, doc))
        return;
    setCurrentRecipeId(recipeId);
    setActiveCamera(0);
    if (!doc.debugs.isEmpty())
        setActiveColorChannel(static_cast<int>(doc.debugs.first().colorChannel));
    else
        setActiveColorChannel(static_cast<int>(ColorChannel::Red));
    setDocument(doc);
    emit sig_recipeSelected();
}

void RecipeConfigViewModel::slot_applySearch(const QString &keyword)
{
    setSearchKeyword(keyword);
    slot_loadRecipeList();
    if (!keyword.isEmpty()) {
        QVector<RecipeListItem> filtered;
        for (const auto &r : m_recipes) {
            if (r.name.contains(keyword, Qt::CaseInsensitive))
                filtered.append(r);
        }
        m_recipes = filtered;
        emit sig_recipesChanged();
    }
}

RecipeDebugToolbar RecipeConfigViewModel::debugToolbar() const
{
    const auto camera = static_cast<CameraIndex>(activeCamera());
    const auto color  = static_cast<ColorChannel>(activeColorChannel());
    if (const RecipeDebugToolbar *entry = findDebugEntry(document().debugs, camera, color))
        return *entry;

    RecipeDebugToolbar toolbar;
    toolbar.camera       = camera;
    toolbar.colorChannel = color;
    return toolbar;
}

void RecipeConfigViewModel::slot_updateDebugToolbar(const RecipeDebugToolbar &toolbar)
{
    modifyDocument([&toolbar](RecipeDocument &doc) {
        findOrCreateDebugEntry(doc.debugs, toolbar.camera, toolbar.colorChannel) = toolbar;
    });
}

QList<double> RecipeConfigViewModel::roiPoints(CameraIndex camera, ColorChannel channel) const
{
    if (const RecipeDebugToolbar *entry = findDebugEntry(document().debugs, camera, channel))
        return entry->rois;
    return {};
}

QList<double> RecipeConfigViewModel::currentRoiPoints() const
{
    return roiPoints(static_cast<CameraIndex>(activeCamera()),
                     static_cast<ColorChannel>(activeColorChannel()));
}

void RecipeConfigViewModel::slot_setCurrentRoiPoints(const QList<double> &points)
{
    modifyDocument([this, points](RecipeDocument &doc) {
        RecipeDebugToolbar &entry = findOrCreateDebugEntry(
            doc.debugs, static_cast<CameraIndex>(activeCamera()),
            static_cast<ColorChannel>(activeColorChannel()));
        entry.rois = points;
    });
}

void RecipeConfigViewModel::slot_saveCurrentRoiPoints(bool roiEnabled, const QList<double> &points)
{
    modifyDocument([this, roiEnabled, points](RecipeDocument &doc) {
        RecipeDebugToolbar &entry = findOrCreateDebugEntry(
            doc.debugs, static_cast<CameraIndex>(activeCamera()),
            static_cast<ColorChannel>(activeColorChannel()));
        entry.roiEnabled = roiEnabled;
        entry.rois       = roiEnabled ? points : QList<double>();
    });
    setStatus(QStringLiteral("区域定位已保存"));
}

void RecipeConfigViewModel::slot_addRecipe(const QString &name)
{
    const QString trimmed = name.trimmed();
    if (trimmed.isEmpty()) {
        setStatus(QStringLiteral("配方名称不能为空"));
        return;
    }

    const QString id = sanitizeRecipeId(trimmed);
    if (id.isEmpty()) {
        setStatus(QStringLiteral("配方名称无效"));
        return;
    }
    if (RecipeRepository::contains(id)) {
        setStatus(QStringLiteral("配方名称已存在"));
        return;
    }
    RecipeDocument doc = RecipeDocument::defaultDemo(id);
    if (!RecipeRepository::save(doc)) {
        setStatus(QStringLiteral("新增配方失败"));
        return;
    }
    slot_selectRecipe(id);
    setStatus(QStringLiteral("已新增配方"));
}

void RecipeConfigViewModel::slot_copyRecipe(const QString &name)
{
    const QString trimmed = name.trimmed();
    if (trimmed.isEmpty()) {
        setStatus(QStringLiteral("配方名称不能为空"));
        return;
    }
    if (document().id.isEmpty()) {
        setStatus(QStringLiteral("请先选择配方"));
        return;
    }

    const QString id = sanitizeRecipeId(trimmed);
    if (id.isEmpty()) {
        setStatus(QStringLiteral("配方名称无效"));
        return;
    }
    if (RecipeRepository::contains(id)) {
        setStatus(QStringLiteral("配方名称已存在"));
        return;
    }
    RecipeDocument doc = document();
    doc.id         = id;
    doc.basic.name = id;
    if (!RecipeRepository::save(doc)) {
        setStatus(QStringLiteral("复制配方失败"));
        return;
    }
    slot_selectRecipe(id);
    setStatus(QStringLiteral("已复制当前配方"));
}

void RecipeConfigViewModel::slot_removeRecipe()
{
    if (document().id.isEmpty()) {
        setStatus(QStringLiteral("请先选择配方"));
        return;
    }
    if (RecipeRepository::listRecipes().size() <= 1) {
        setStatus(QStringLiteral("至少保留一个配方"));
        return;
    }
    const QString removedId = document().id;
    if (!RecipeRepository::remove(removedId)) {
        setStatus(QStringLiteral("删除配方失败"));
        return;
    }
    if (!m_recipes.isEmpty()) {
        const QString defaultId = RecipeRepository::defaultRecipeId();
        QString nextId          = defaultId;
        bool    found           = false;
        for (const RecipeListItem &item : m_recipes) {
            if (item.id == defaultId) {
                found = true;
                break;
            }
        }
        if (!found)
            nextId = m_recipes.first().id;
        slot_selectRecipe(nextId);
    }
    setStatus(QStringLiteral("已删除配方"));
}

QString RecipeConfigViewModel::slot_importRecipesFromFiles(const QStringList &filePaths)
{
    RecipeImportReport report;
    report.timestamp = QDateTime::currentDateTime();

    RecipeRepository::importFromJsonFiles(filePaths, report);
    RecipeRepository::writeImportLog(report);

    if (!report.overallMessage.isEmpty())
        setStatus(report.overallMessage);
    return report.overallMessage;
}

QString RecipeConfigViewModel::slot_exportAllRecipes(const QString &dirPath)
{
    RecipeExportReport report;
    report.timestamp = QDateTime::currentDateTime();

    const QString trimmed = dirPath.trimmed();
    if (trimmed.isEmpty()) {
        report.overallOk      = false;
        report.overallMessage = QStringLiteral("全部导出失败");
        setStatus(report.overallMessage);
        RecipeRepository::writeExportLog(report);
        return report.overallMessage;
    }
    report.exportDirectory = trimmed;

    if (!document().id.isEmpty()) {
        report.attemptedSaveCurrent = true;
        report.currentRecipeId        = document().id;
        report.currentRecipeName      = document().id;
        report.savedCurrentOk = RecipeRepository::save(document(), currentRecipeId());
        if (report.savedCurrentOk) {
            setCurrentRecipeId(document().id);
        }
        report.currentSaveMessage = report.savedCurrentOk ? QStringLiteral("保存当前配方成功")
                                                          : QStringLiteral("保存当前配方失败");
    }

    RecipeRepository::exportAllToDirectory(trimmed, report);
    setStatus(report.overallMessage);
    RecipeRepository::writeExportLog(report);
    return report.overallMessage;
}

bool RecipeConfigViewModel::canRemoveRecipe() const
{
    return !document().id.isEmpty() && RecipeRepository::listRecipes().size() > 1;
}

void RecipeConfigViewModel::updateStatusBarFromDocument()
{
    m_statusBar.currentRecipeName = document().id;
    if (kRecipeShowImageQualityUi) {
        m_statusBar.imageQualityOk   = true;
        m_statusBar.imageQualityText = QStringLiteral("通过(92%)");
    }
    m_statusBar.canSave         = true;
    m_statusBar.paramStatusText = QStringLiteral("可保存");
    emit sig_statusBarChanged();
}

void RecipeConfigViewModel::updateCommandStates()
{
    if (m_saveCommand)
        m_saveCommand->updateCanExecute();
    if (m_setDefaultCommand)
        m_setDefaultCommand->updateCanExecute();
}

} // namespace lighten
