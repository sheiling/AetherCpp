/**
 * @file recipe_model.cpp
 * @brief 配方文档序列化、持久化与配方目录管理实现
 */

#include "recipe_model.h"

#include "lighten_param_persist.h"
#include "virtual_pixel_model.h"

#include <QCoreApplication>
#include <QDate>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QSet>
#include <QTextStream>
#include <QRegularExpression>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#endif

/** @brief 配方文档序列化、目录扫描与 RecipeRepository 持久化实现 */
namespace lighten {

const RecipeDebugToolbar *findDebugEntry(const QVector<RecipeDebugToolbar> &debugs,
                                         CameraIndex camera, ColorChannel channel)
{
    for (const RecipeDebugToolbar &entry : debugs) {
        if (entry.camera == camera && entry.colorChannel == channel)
            return &entry;
    }
    return nullptr;
}

RecipeDebugToolbar &findOrCreateDebugEntry(QVector<RecipeDebugToolbar> &debugs,
                                           CameraIndex camera, ColorChannel channel)
{
    for (RecipeDebugToolbar &entry : debugs) {
        if (entry.camera == camera && entry.colorChannel == channel)
            return entry;
    }
    RecipeDebugToolbar entry;
    entry.camera       = camera;
    entry.colorChannel = channel;
    debugs.append(entry);
    return debugs.last();
}

/** 配方 JSON 读写与目录路径辅助 */
namespace {

QList<double> readFlatDoubleArray(const json &value)
{
    QList<double> values;
    if (!value.is_array())
        return values;
    for (const json &v : value) {
        if (v.is_number())
            values.append(v.get<double>());
        else if (v.is_object()) {
            values.append(v.value("x", 0.0));
            values.append(v.value("y", 0.0));
        }
    }
    return values;
}

bool migrateLegacyDebugNode(json &general)
{
    if (general.contains("debugs") || !general.contains("debug") || !general.at("debug").is_object())
        return false;

    const json &dbg = general.at("debug");
    json          debugs = json::array();

    if (dbg.contains("mapRoi") && dbg.at("mapRoi").is_object()) {
        for (auto it = dbg.at("mapRoi").begin(); it != dbg.at("mapRoi").end(); ++it) {
            const QStringList parts = QString::fromStdString(it.key()).split(QLatin1Char('_'));
            if (parts.size() != 2)
                continue;

            json entry;
            entry["camera"]     = parts.at(0).toInt();
            entry["roiEnabled"] = dbg.value("roiEnabled", true);
            entry["rois"]       = json::array();
            for (double value : readFlatDoubleArray(it.value()))
                entry["rois"].push_back(value);
            debugs.push_back(std::move(entry));
        }
    }

    if (debugs.empty()) {
        json entry;
        entry["camera"]     = dbg.value("camera", 0);
        entry["roiEnabled"] = dbg.value("roiEnabled", true);
        entry["rois"]       = json::array();
        debugs.push_back(std::move(entry));
    }

    general["debugs"] = std::move(debugs);
    general.erase("debug");
    return true;
}

void readRoisFromJson(const json &j, QList<double> &rois)
{
    rois.clear();
    if (!j.contains("rois"))
        return;
    rois = readFlatDoubleArray(j.at("rois"));
}

void writeRoisToJson(const QList<double> &rois, json &j)
{
    json arr = json::array();
    for (double value : rois)
        arr.push_back(value);
    j["rois"] = std::move(arr);
}

} // namespace

void RecipeDebugToolbar::reflectFromJson(const json &j)
{
    FROM_JSON_AUTO_SAME_NAME(j, camera);
    FROM_JSON_AUTO_SAME_NAME(j, roiEnabled);
    readRoisFromJson(j, rois);
}

void RecipeDebugToolbar::reflectToJson(json &j) const
{
    TO_JSON_AUTO_SAME_NAME(j, camera);
    TO_JSON_AUTO_SAME_NAME(j, roiEnabled);
    writeRoisToJson(rois, j);
}

bool PerCameraParams::operator==(const PerCameraParams &o) const
{
    return pureExposureMs == o.pureExposureMs && pureGrayLevel == o.pureGrayLevel
           && seriesExposureMs == o.seriesExposureMs && seriesGrayLevel == o.seriesGrayLevel
           && whiteExposureMs == o.whiteExposureMs && blackExposureMs == o.blackExposureMs
           && qualityOkR == o.qualityOkR && qualityNgG == o.qualityNgG && qualityOkB == o.qualityOkB
           && rotationDeg == o.rotationDeg;
}

bool RecipeDebugToolbar::operator==(const RecipeDebugToolbar &o) const
{
    return camera == o.camera && colorChannel == o.colorChannel && roiEnabled == o.roiEnabled
           && roiModeIndex == o.roiModeIndex && rois == o.rois;
}

bool RecipeDocument::operator==(const RecipeDocument &o) const
{
    return id == o.id && basic == o.basic && cameras == o.cameras && algo == o.algo
           && specialItems == o.specialItems && other == o.other && debugs == o.debugs
           && debugResultChips == o.debugResultChips;
}

AlgoThresholds AlgoThresholds::defaultDemo()
{
    AlgoThresholds t;
    const QStringList names = {
        QStringLiteral("死灯阈值"), QStringLiteral("亮灯阈值"), QStringLiteral("暗灯阈值"),
        QStringLiteral("串灯阈值"), QStringLiteral("串色R"), QStringLiteral("串色G"),
        QStringLiteral("串色B"), QStringLiteral("混色初筛"), QStringLiteral("混色筛选"),
    };
    const QVector<RgbTriple> vals = {
        {18, 18, 18}, {22, 22, 22}, {14, 14, 14}, {16, 16, 16}, {10, 10, 10},
        {10, 10, 10}, {10, 10, 10}, {12, 12, 12}, {15, 15, 15},
    };
    for (int i = 0; i < names.size(); ++i)
        t.rows.append({names[i], vals[i]});
    return t;
}

QString sanitizeRecipeId(const QString &rawId)
{
    QString base = rawId.trimmed();
    if (base.isEmpty())
        return {};
    static const QRegularExpression invalid(QStringLiteral(R"([\\/:*?"<>|])"));
    base.replace(invalid, QStringLiteral("_"));
    return base;
}

void syncRecipeIdAndName(RecipeDocument &doc)
{
    if (!doc.id.isEmpty())
        doc.basic.name = doc.id;
    else if (!doc.basic.name.isEmpty())
        doc.id = doc.basic.name;
}

RecipeDocument RecipeDocument::defaultDemo(const QString &id)
{
    RecipeDocument doc;
    doc.id         = id;
    doc.basic.name = id;
    doc.basic.width   = 1920;
    doc.basic.height  = 1080;
    doc.cameras.resize(kCameraCount);
    for (int c = 0; c < kCameraCount; ++c) {
        auto &p              = doc.cameras[c];
        p.pureExposureMs     = {12, 12, 12};
        p.pureGrayLevel      = {220, 220, 220};
        p.seriesExposureMs   = {8, 8, 8};
        p.seriesGrayLevel    = {180, 180, 180};
        p.whiteExposureMs    = 10;
        p.blackExposureMs    = 6;
        p.rotationDeg        = 0;
    }
    doc.algo = AlgoThresholds::defaultDemo();
    doc.specialItems = {
        {QStringLiteral("大面积灭点"), true, 30, QStringLiteral("自动下料")},
        {QStringLiteral("非边缘缺陷"), true, 8, QStringLiteral("报警")},
        {QStringLiteral("线不良"), false, 5, QStringLiteral("报警")},
        {QStringLiteral("区块不良"), true, 12, QStringLiteral("自动下料")},
    };
    doc.other = RecipeOtherSettings{};
    RecipeDebugToolbar mainRed;
    mainRed.camera       = CameraIndex::Main;
    mainRed.colorChannel = ColorChannel::Red;
    mainRed.rois         = {480.0, 270.0, 1440.0, 270.0, 1440.0, 810.0, 480.0, 810.0};
    doc.debugs.append(mainRed);
    static const DetectItemKind kDebugResultChipKinds[] = {
        DetectItemKind::DeadLed,        DetectItemKind::DimLed,       DetectItemKind::BrightLed,
        DetectItemKind::ColorVariation, DetectItemKind::ColorMapDead, DetectItemKind::ColorMapLive,
        DetectItemKind::MixedColor,     DetectItemKind::BlackScreenOn,
    };
    for (DetectItemKind kind : kDebugResultChipKinds)
        doc.debugResultChips.append(DefectResultChip{kind});
    return doc;
}

/** 配方内存缓存、默认 ID 与列表变更通知 */
namespace {

QHash<QString, RecipeDocument> &recipeStore()
{
    static QHash<QString, RecipeDocument> store;
    return store;
}

bool &storeInitialized()
{
    static bool initialized = false;
    return initialized;
}

QString currentRecipeFilePath()
{
    return config_dir::root() + QStringLiteral("/current_recipe.json");
}

/** 将 config/recipe/{recipeId}.json 同步为 config/current_recipe.json（先删目标再拷贝） */
bool copyRecipeJsonToCurrent(const QString &recipeId)
{
    QString src = recipe_file::filePath(recipeId);
    if (!QFileInfo::exists(src)) {
        const auto it = recipeStore().constFind(recipeId);
        if (it == recipeStore().constEnd() || !it.value().saveToFile())
            return false;
        src = recipe_file::filePath(recipeId);
    }

    const QString dst = currentRecipeFilePath();
    if (!QDir().mkpath(config_dir::root()))
        return false;
    if (QFileInfo::exists(dst) && !QFile::remove(dst))
        return false;
    return QFile::copy(src, dst);
}

void normalizeRecipeDocument(RecipeDocument &doc)
{
    syncRecipeIdAndName(doc);

    if (doc.cameras.size() == kCameraCount)
        return;

    const RecipeDocument fallback = RecipeDocument::defaultDemo(doc.id);
    const int oldSize = doc.cameras.size();
    doc.cameras.resize(kCameraCount);
    for (int i = oldSize; i < kCameraCount; ++i)
        doc.cameras[i] = fallback.cameras[i];
}

QString firstStoreKey(const QHash<QString, RecipeDocument> &store)
{
    QString key;
    for (auto it = store.constBegin(); it != store.constEnd(); ++it) {
        if (key.isEmpty() || it.key() < key)
            key = it.key();
    }
    return key;
}

QString &defaultIdStore()
{
    static QString preferredId;
    return preferredId;
}

void reconcileDefaultId()
{
    auto &store = recipeStore();
    if (defaultIdStore().isEmpty() || !store.contains(defaultIdStore()))
        defaultIdStore() = firstStoreKey(store);
}

/** 默认配方存在时，刷新 current_recipe.json */
void syncCurrentRecipeFromDefault()
{
    const QString defaultId = defaultIdStore();
    if (defaultId.isEmpty() || !recipeStore().contains(defaultId))
        return;
    copyRecipeJsonToCurrent(defaultId);
}

QString defaultIdFromCurrentRecipeSnapshot()
{
    json root;
    if (!recipe_file::readRoot(currentRecipeFilePath(), root))
        return {};
    if (!root.contains("general") || !root.at("general").is_object())
        return {};
    const json &g = root.at("general");
    if (g.contains("id") && g.at("id").is_string()) {
        const QString id = QString::fromStdString(g.at("id").get<std::string>()).trimmed();
        if (!id.isEmpty())
            return id;
    }
    if (g.contains("basic") && g.at("basic").is_object()) {
        const json &basic = g.at("basic");
        if (basic.contains("name") && basic.at("name").is_string())
            return QString::fromStdString(basic.at("name").get<std::string>()).trimmed();
    }
    return {};
}

void finishLoad()
{
    auto &store = recipeStore();

    const QString fromSnapshot = defaultIdFromCurrentRecipeSnapshot();
    if (!fromSnapshot.isEmpty() && store.contains(fromSnapshot))
        defaultIdStore() = fromSnapshot;
    else
        reconcileDefaultId();

    storeInitialized() = true;
    syncCurrentRecipeFromDefault();
    RecipeCatalogNotifier::instance()->slot_notifyRecipeListChanged();
}

void ensureStoreLoaded()
{
    if (storeInitialized())
        return;
    RecipeRepository::load();
}

void ensureSeeded()
{
    auto &store = recipeStore();
    if (!store.isEmpty())
        return;

    const QStringList seedIds = {
        QStringLiteral("PANEL-A01"),
        QStringLiteral("PANEL-A02"),
        QStringLiteral("PANEL-B01"),
        QStringLiteral("PANEL-C08"),
    };
    for (const QString &id : seedIds)
        store.insert(id, RecipeDocument::defaultDemo(id));
    defaultIdStore() = QStringLiteral("PANEL-A01");
}

} // namespace

RecipeCatalogNotifier *RecipeCatalogNotifier::instance()
{
    static RecipeCatalogNotifier notifier;
    return &notifier;
}

void RecipeCatalogNotifier::slot_notifyRecipeListChanged()
{
    emit sig_recipeListChanged();
}

void RecipeCatalogNotifier::slot_notifyDefaultRecipeChanged(const QString &recipeId)
{
    emit sig_defaultRecipeChanged(recipeId);
}

bool RecipeDocument::saveToFilePath(const QString &filePath) const
{
    if (filePath.isEmpty())
        return false;

    try {
        json root;
        if (!recipe_file::readRoot(filePath, root))
            return false;

        json general;
        reflectToJson(general);
        root["general"] = general;
        return recipe_file::writeRoot(filePath, root);
    } catch (const std::exception &) {
        return false;
    }
}

bool RecipeDocument::saveToFile() const
{
    if (id.isEmpty())
        return false;
    return saveToFilePath(recipe_file::filePath(id));
}

bool RecipeDocument::loadFromFile(const QString &filePath, RecipeDocument &out)
{
    try {
        json j;
        if (!recipe_file::readRoot(filePath, j))
            return false;
        if (!j.contains("general") || !j.at("general").is_object())
            return false;
        json general = j.at("general");
        migrateLegacyDebugNode(general);
        out.reflectFromJson(general);
        if (out.id.isEmpty())
            return false;
        normalizeRecipeDocument(out);
        return true;
    } catch (const std::exception &) {
        return false;
    }
}

void RecipeRepository::load()
{
    const QString recipeDir = recipe_file::recipeDir();
    QDir dir(recipeDir);
    if (!dir.exists()) {
        ensureSeeded();
        finishLoad();
        return;
    }

    const QStringList jsonFiles =
        dir.entryList({QStringLiteral("*.json")}, QDir::Files);
    if (jsonFiles.isEmpty()) {
        ensureSeeded();
        finishLoad();
        return;
    }

    auto &store = recipeStore();
    store.clear();

    for (const QString &fileName : jsonFiles) {
        RecipeDocument doc;
        if (!RecipeDocument::loadFromFile(dir.absoluteFilePath(fileName), doc))
            continue;
        store.insert(doc.id, doc);
    }

    if (store.isEmpty())
        ensureSeeded();
    finishLoad();
}

QVector<RecipeListItem> RecipeRepository::listRecipes()
{
    ensureStoreLoaded();
    const QString defaultId = defaultIdStore();
    QVector<RecipeListItem> list;
    list.reserve(recipeStore().size());
    for (auto it = recipeStore().constBegin(); it != recipeStore().constEnd(); ++it) {
        const RecipeDocument &doc = it.value();
        RecipeListItem item;
        item.id   = doc.id;
        item.name = doc.id;
        item.tag  = (doc.id == defaultId) ? RecipeListTag::Default : RecipeListTag::None;
        list.append(item);
    }
    return list;
}

bool RecipeRepository::load(const QString &recipeId, RecipeDocument &out)
{
    ensureStoreLoaded();
    if (recipeId.isEmpty() || !recipeStore().contains(recipeId))
        return false;
    out = recipeStore().value(recipeId);
    syncRecipeIdAndName(out);
    return true;
}

bool RecipeRepository::contains(const QString &recipeId)
{
    ensureStoreLoaded();
    return !recipeId.isEmpty() && recipeStore().contains(recipeId);
}

bool RecipeRepository::save(const RecipeDocument &doc, const QString &previousRecipeId)
{
    ensureStoreLoaded();
    if (doc.id.isEmpty())
        return false;

    RecipeDocument toSave = doc;
    normalizeRecipeDocument(toSave);
    if (toSave.id.isEmpty())
        return false;

    const QString prev = previousRecipeId.trimmed();
    const bool    isRename = !prev.isEmpty() && prev != toSave.id;

    auto &store = recipeStore();
    if (isRename) {
        if (!store.contains(prev))
            return false;
        if (store.contains(toSave.id) && toSave.id != prev)
            return false;
        store.remove(prev);
        QFile::remove(recipe_file::filePath(prev));
        if (defaultIdStore() == prev)
            defaultIdStore() = toSave.id;
    }

    if (!toSave.saveToFile())
        return false;
    store.insert(toSave.id, toSave);

    if (isRename)
        VirtualPixelRepository::migrateRecipeId(prev, toSave.id);

    const bool isDefaultRecipe = (toSave.id == defaultIdStore());
    if (isDefaultRecipe)
        copyRecipeJsonToCurrent(toSave.id);

    RecipeCatalogNotifier::instance()->slot_notifyRecipeListChanged();
    return true;
}

bool RecipeRepository::remove(const QString &recipeId)
{
    ensureStoreLoaded();
    if (recipeId.isEmpty() || !recipeStore().remove(recipeId))
        return false;
    QFile::remove(recipe_file::filePath(recipeId));
    VirtualPixelRepository::evictFromStore(recipeId);
    reconcileDefaultId();
    RecipeCatalogNotifier::instance()->slot_notifyRecipeListChanged();
    return true;
}

bool RecipeRepository::setDefault(const QString &recipeId)
{
    ensureStoreLoaded();
    if (recipeId.isEmpty() || !recipeStore().contains(recipeId))
        return false;

    defaultIdStore() = recipeId;
    if (!copyRecipeJsonToCurrent(recipeId))
        return false;

    RecipeCatalogNotifier::instance()->slot_notifyDefaultRecipeChanged(recipeId);
    RecipeCatalogNotifier::instance()->slot_notifyRecipeListChanged();
    return true;
}

QString RecipeRepository::defaultRecipeId()
{
    ensureStoreLoaded();
    return defaultIdStore();
}

/** 配方导入/导出文件名消毒与目录拷贝 */
namespace {

QString sanitizeRecipeExportBaseName(const QString &rawName, const QString &fallbackId)
{
    QString base = rawName.trimmed();
    if (base.isEmpty())
        base = fallbackId.trimmed();
    if (base.isEmpty())
        base = QStringLiteral("recipe");
    static const QRegularExpression invalid(QStringLiteral(R"([\\/:*?"<>|])"));
    base.replace(invalid, QStringLiteral("_"));
    return base;
}

QString recipeExportPath(const QString &dirPath, const QString &baseName)
{
    return dirPath + QLatin1Char('/') + baseName + QStringLiteral(".json");
}

QString recipeImportExportLogDir()
{
    return QCoreApplication::applicationDirPath() + QStringLiteral("/import_export");
}

QString recipeImportExportLogPath(const QString &fileName)
{
    const QString dir = recipeImportExportLogDir();
    if (!QDir().mkpath(dir))
        return {};
    return dir + QLatin1Char('/') + fileName;
}

} // namespace

bool RecipeRepository::exportAllToDirectory(const QString &dirPath, RecipeExportReport &report)
{
    ensureStoreLoaded();
    report.exportDirectory = dirPath.trimmed();
    report.items.clear();

    const QString trimmed = report.exportDirectory;
    if (trimmed.isEmpty()) {
        report.overallOk      = false;
        report.overallMessage = QStringLiteral("全部导出失败");
        return false;
    }
    if (!QDir().mkpath(trimmed)) {
        report.overallOk      = false;
        report.overallMessage = QStringLiteral("全部导出失败");
        return false;
    }

    const QVector<RecipeListItem> items = listRecipes();
    if (items.isEmpty()) {
        report.overallOk      = false;
        report.overallMessage = QStringLiteral("全部导出失败");
        return false;
    }

    int okCount   = 0;
    int failCount = 0;
    for (const RecipeListItem &item : items) {
        RecipeExportItemResult entry;
        entry.recipeId   = item.id;
        entry.recipeName = item.name;

        RecipeDocument doc;
        if (!load(item.id, doc)) {
            entry.ok      = false;
            entry.message = QStringLiteral("加载配方失败");
            report.items.append(entry);
            ++failCount;
            continue;
        }

        const QString baseName = sanitizeRecipeExportBaseName(doc.id, doc.id);
        const QString filePath = recipeExportPath(trimmed, baseName);
        entry.filePath         = filePath;

        if (QFileInfo::exists(filePath)) {
            entry.ok      = false;
            entry.message = QStringLiteral("目标文件已存在");
            report.items.append(entry);
            ++failCount;
            continue;
        }

        if (!doc.saveToFilePath(filePath)) {
            entry.ok      = false;
            entry.message = QStringLiteral("写入文件失败");
            report.items.append(entry);
            ++failCount;
            continue;
        }

        entry.ok      = true;
        entry.message = QStringLiteral("导出成功");
        report.items.append(entry);
        ++okCount;
    }

    if (okCount > 0 && failCount == 0) {
        report.overallOk      = true;
        report.overallMessage = QStringLiteral("全部导出成功");
        return true;
    }
    if (okCount == 0) {
        report.overallOk      = false;
        report.overallMessage = QStringLiteral("全部导出失败");
        return false;
    }

    report.overallOk      = false;
    report.overallMessage = QStringLiteral("部分导出失败");
    return false;
}

void RecipeRepository::writeExportLog(RecipeExportReport &report)
{
    const QString timeStr =
        report.timestamp.toString(QStringLiteral("yyyyMMdd_HHmmss"));
    const QString logPath = recipeImportExportLogPath(
        QStringLiteral("export_") + timeStr + QStringLiteral(".txt"));
    report.logFilePath    = logPath;
    if (logPath.isEmpty())
        return;

    QFile file(logPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    out.setEncoding(QStringConverter::Utf8);
#else
    out.setCodec("UTF-8");
#endif

    int successCount = 0;
    int failCount    = 0;
    for (const RecipeExportItemResult &item : report.items) {
        if (item.ok)
            ++successCount;
        else
            ++failCount;
    }

    out << QStringLiteral("================================================================================\n");
    out << QStringLiteral("导出时间: ")
        << report.timestamp.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")) << QLatin1Char('\n');
    out << QStringLiteral("总体结果: ")
        << (report.overallOk ? QStringLiteral("成功") : QStringLiteral("失败")) << QLatin1Char('\n');
    if (!report.exportDirectory.isEmpty())
        out << QStringLiteral("导出目录: ") << report.exportDirectory << QLatin1Char('\n');
    if (!report.overallMessage.isEmpty())
        out << QStringLiteral("概要: ") << report.overallMessage << QLatin1Char('\n');

    out << QStringLiteral("\n--- 导出前保存当前配方 ---\n");
    if (!report.attemptedSaveCurrent) {
        out << QStringLiteral("已执行: 否（无当前选中配方）\n");
    } else {
        out << QStringLiteral("已执行: 是\n");
        out << QStringLiteral("配方ID: ") << report.currentRecipeId << QLatin1Char('\n');
        out << QStringLiteral("配方名称: ") << report.currentRecipeName << QLatin1Char('\n');
        out << QStringLiteral("保存结果: ")
            << (report.savedCurrentOk ? QStringLiteral("成功") : QStringLiteral("失败"))
            << QLatin1Char('\n');
        if (!report.currentSaveMessage.isEmpty())
            out << QStringLiteral("说明: ") << report.currentSaveMessage << QLatin1Char('\n');
    }

    out << QStringLiteral("\n--- 配方导出明细 (共 %1 项，成功 %2，失败 %3) ---\n")
               .arg(report.items.size())
               .arg(successCount)
               .arg(failCount);
    for (const RecipeExportItemResult &item : report.items) {
        out << (item.ok ? QStringLiteral("[成功] ") : QStringLiteral("[失败] "));
        out << item.recipeName << QStringLiteral(" (id: ") << item.recipeId
            << QStringLiteral(")");
        if (!item.filePath.isEmpty())
            out << QStringLiteral(" -> ") << item.filePath;
        if (!item.message.isEmpty())
            out << QStringLiteral(" | ") << item.message;
        out << QLatin1Char('\n');
    }
    out << QLatin1Char('\n');
}

bool RecipeRepository::importFromJsonFiles(const QStringList &filePaths, RecipeImportReport &report)
{
    ensureStoreLoaded();
    report.items.clear();
    report.timestamp = QDateTime::currentDateTime();

    if (filePaths.isEmpty()) {
        report.overallOk      = false;
        report.overallMessage = QStringLiteral("全部导入失败");
        return false;
    }

    QSet<QString> existingIds;
    for (const RecipeListItem &item : listRecipes())
        existingIds.insert(item.id);

    int successCount = 0;
    int failCount    = 0;

    for (const QString &filePath : filePaths) {
        RecipeImportItemResult entry;
        entry.filePath = filePath;

        RecipeDocument doc;
        if (!RecipeDocument::loadFromFile(filePath, doc)) {
            entry.ok      = false;
            entry.message = QStringLiteral("读取 JSON 或解析 general 节点失败");
            report.items.append(entry);
            ++failCount;
            continue;
        }

        entry.recipeId   = doc.id;
        entry.recipeName = doc.id;

        if (existingIds.contains(doc.id)) {
            entry.ok      = false;
            entry.message = QStringLiteral("配方 ID 已存在");
            report.items.append(entry);
            ++failCount;
            continue;
        }

        if (!save(doc)) {
            entry.ok      = false;
            entry.message = QStringLiteral("保存配方参数失败");
            report.items.append(entry);
            ++failCount;
            continue;
        }

        RecipeDocument saved;
        if (!load(doc.id, saved)) {
            entry.ok      = false;
            entry.message = QStringLiteral("保存后加载配方失败");
            report.items.append(entry);
            ++failCount;
            continue;
        }

        existingIds.insert(doc.id);
        entry.ok      = true;
        entry.message = QStringLiteral("导入成功");
        report.items.append(entry);
        ++successCount;
    }

    if (successCount > 0)
        RecipeCatalogNotifier::instance()->slot_notifyRecipeListChanged();

    if (successCount > 0 && failCount == 0) {
        report.overallOk      = true;
        report.overallMessage = QStringLiteral("全部导入成功");
        return true;
    }
    if (successCount == 0) {
        report.overallOk      = false;
        report.overallMessage = QStringLiteral("全部导入失败");
        return false;
    }

    report.overallOk      = false;
    report.overallMessage = QStringLiteral("部分导入失败");
    return false;
}

void RecipeRepository::writeImportLog(RecipeImportReport &report)
{
    const QString timeStr =
        report.timestamp.toString(QStringLiteral("yyyyMMdd_HHmmss"));
    const QString logPath = recipeImportExportLogPath(
        QStringLiteral("import_") + timeStr + QStringLiteral(".txt"));
    report.logFilePath      = logPath;
    if (logPath.isEmpty())
        return;

    QFile file(logPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    out.setEncoding(QStringConverter::Utf8);
#else
    out.setCodec("UTF-8");
#endif

    int successCount = 0;
    int failCount    = 0;
    for (const RecipeImportItemResult &item : report.items) {
        if (item.ok)
            ++successCount;
        else
            ++failCount;
    }

    out << QStringLiteral("================================================================================\n");
    out << QStringLiteral("导入时间: ")
        << report.timestamp.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")) << QLatin1Char('\n');
    out << QStringLiteral("总体结果: ")
        << (report.overallOk ? QStringLiteral("成功") : QStringLiteral("失败或部分成功"))
        << QLatin1Char('\n');
    if (!report.overallMessage.isEmpty())
        out << QStringLiteral("概要: ") << report.overallMessage << QLatin1Char('\n');

    out << QStringLiteral("\n--- 配方导入明细 (共 %1 项，成功 %2，失败 %3) ---\n")
               .arg(report.items.size())
               .arg(successCount)
               .arg(failCount);
    for (const RecipeImportItemResult &item : report.items) {
        out << (item.ok ? QStringLiteral("[成功] ") : QStringLiteral("[失败] "));
        out << item.filePath;
        if (!item.recipeName.isEmpty() || !item.recipeId.isEmpty())
            out << QStringLiteral(" | ") << item.recipeName << QStringLiteral(" (id: ")
                << item.recipeId << QStringLiteral(")");
        if (!item.message.isEmpty())
            out << QStringLiteral(" | ") << item.message;
        out << QLatin1Char('\n');
    }
    out << QLatin1Char('\n');
}

} // namespace lighten
