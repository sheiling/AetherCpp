#include "recipe_config_model.h"

#include "data/data.h"
#include "system_config_model.h"

#include <common/utility/json_tool/json_utils.h>

#include <QtGlobal>

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QSet>
#include <QTextStream>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#endif

#include <algorithm>

namespace aoi {

namespace {

MarkPointConfig makeDefaultMark(const QString &tplFile, const QString &tplName)
{
    MarkPointConfig cfg;
    cfg.capturePos           = {};
    cfg.markerInfo.markType      = vision_algo::MarkerType::Circle;
    cfg.markerInfo.markWidth     = 32;
    cfg.markerInfo.markHeight    = 32;
    cfg.markerInfo.area          = 120.f;
    cfg.markerInfo.minThresh     = 0.75f;
    cfg.markerInfo.maxThresh     = 0.95f;
    cfg.markerInfo.isWhiteMark   = 0;
    cfg.markerInfo.modelFileName = tplFile.toStdString();
    cfg.markerInfo.modelName     = tplName.toStdString();
    return cfg;
}

} // namespace

QString &recipeStorageDirectory()
{
    static QString dir;
    return dir;
}

QString recipeParamDir()
{
    if (!recipeStorageDirectory().isEmpty())
        return recipeStorageDirectory();
    return data::recipeStorageDir();
}

QString recipeFilePath(const QString &recipeId)
{
    return QDir(recipeParamDir()).filePath(recipeId + QString::fromLatin1(data::kRecipeFileSuffix));
}

bool readRootObjectByPath(const QString &filePath, json &root)
{
    return common::utility::json_parseFromFileEx(
        root, filePath.toStdString(),
        common::utility::JsonParseFileOptions{true, true});
}

bool writeRootObjectByPath(const QString &filePath, const json &root)
{
    return common::utility::json_saveToFileEx(
        root, filePath.toStdString(),
        common::utility::JsonSaveFileOptions{true});
}

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

QString &defaultIdStore()
{
    static QString preferredId;
    return preferredId;
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

bool copyRecipeJsonToCurrent(const QString &recipeId)
{
    QString src = recipeFilePath(recipeId);
    if (!QFileInfo::exists(src)) {
        const auto it = recipeStore().constFind(recipeId);
        if (it == recipeStore().constEnd() || !it.value().saveToFile())
            return false;
        src = recipeFilePath(recipeId);
    }

    const QString dst = data::currentRecipeFilePath();
    const QString paramRoot = data::paramRootDir();
    if (!QDir().mkpath(paramRoot))
        return false;
    if (QFileInfo::exists(dst) && !QFile::remove(dst))
        return false;
    return QFile::copy(src, dst);
}

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
    if (!readRootObjectByPath(data::currentRecipeFilePath(), root))
        return {};
    if (!root.contains("general") || !root.at("general").is_object())
        return {};
    const json &g = root.at("general");
    if (!g.contains("id") || !g.at("id").is_string())
        return {};
    return QString::fromStdString(g.at("id").get<std::string>()).trimmed();
}

void finishLoad()
{
    auto &store = recipeStore();

    const QString fromSnapshot = defaultIdFromCurrentRecipeSnapshot();
    if (!fromSnapshot.isEmpty() && store.contains(fromSnapshot))
        defaultIdStore() = fromSnapshot;
    else if (defaultIdStore().isEmpty() || !store.contains(defaultIdStore()))
        defaultIdStore() = firstStoreKey(store);

    storeInitialized() = true;
    syncCurrentRecipeFromDefault();
}

void ensureStoreLoaded()
{
    if (storeInitialized())
        return;
    RecipeRepository::load();
}

QString recipeImportExportLogPath(const QString &fileName)
{
    const QString dir = recipeParamDir();
    if (!QDir().mkpath(dir))
        return {};
    return dir + QLatin1Char('/') + fileName;
}

QString sanitizeRecipeExportBaseName(const QString &rawName, const QString &fallbackId)
{
    QString base = rawName.trimmed();
    if (base.isEmpty())
        base = fallbackId.trimmed();
    if (base.isEmpty())
        return QStringLiteral("recipe");
    for (QChar &ch : base) {
        if (ch == QLatin1Char('\\') || ch == QLatin1Char('/') || ch == QLatin1Char(':')
            || ch == QLatin1Char('*') || ch == QLatin1Char('?') || ch == QLatin1Char('"')
            || ch == QLatin1Char('<') || ch == QLatin1Char('>') || ch == QLatin1Char('|'))
            ch = QLatin1Char('_');
    }
    return base;
}

void normalizeRecipeDocument(RecipeDocument &doc)
{
    if (!doc.id.isEmpty())
        doc.basic.name = doc.id;
    else if (!doc.basic.name.isEmpty())
        doc.id = doc.basic.name;

    const int markCount = recipeMarkCount(SystemConfigRepository::load().common.trackMode);
    if (doc.marks.size() < markCount) {
        const RecipeDocument fallback = RecipeDocument::defaultDemo(
            doc.id.isEmpty() ? QStringLiteral("PANEL-A01") : doc.id, doc.basic.name);
        const int oldSize = doc.marks.size();
        doc.marks.resize(markCount);
        for (int i = oldSize; i < markCount; ++i)
            doc.marks[i] = fallback.marks[i];
    } else if (doc.marks.size() > markCount) {
        doc.marks.resize(markCount);
    }
}

bool RecipeDocument::saveToFilePath(const QString &filePath) const
{
    if (filePath.isEmpty())
        return false;
    try {
        json root;
        json general;
        reflectToJson(general);
        root["general"] = general;
        return writeRootObjectByPath(filePath, root);
    } catch (const std::exception &) {
        return false;
    }
}

bool RecipeDocument::saveToFile() const
{
    if (id.isEmpty())
        return false;
    return saveToFilePath(recipeFilePath(id));
}

bool RecipeDocument::loadFromFile(const QString &filePath, RecipeDocument &out)
{
    try {
        json root;
        if (!readRootObjectByPath(filePath, root))
            return false;
        if (!root.contains("general") || !root.at("general").is_object())
            return false;
        out.reflectFromJson(root.at("general"));
        if (out.id.isEmpty()) {
            const QFileInfo fi(filePath);
            out.id = fi.baseName();
        }
        normalizeRecipeDocument(out);
        return !out.id.isEmpty();
    } catch (const std::exception &) {
        return false;
    }
}

RecipeDocument RecipeDocument::defaultDemo(const QString &id, const QString &name)
{
    RecipeDocument doc;
    doc.id                 = id;
    doc.basic.name         = name;
    doc.basic.width        = 100;
    doc.basic.height       = 160;
    doc.basic.groupPitchXmm    = 1.25;
    doc.basic.groupPitchYmm    = 1.25;
    doc.basic.chipPitchInGroupMm = 0.50;
    // 华星 2.95 μm/px 等价 mm：芯片 51×20、焊盘 109×45、芯片组 109×175 px
    doc.basic.chipWidthMm  = 0.15045;
    doc.basic.chipHeightMm = 0.059;
    doc.basic.padWidthMm   = 0.32155;
    doc.basic.padHeightMm  = 0.13275;
    doc.basic.groupWidthMm     = 0.32155;
    doc.basic.groupHeightMm    = 0.51625;
    doc.basic.offsetGroupMarkBl   = {2.380, 2.120};
    doc.basic.offsetGroupMarkTr = {-2.380, 2.120};

    doc.camera.lightBrightness = 153;
    doc.camera.exposureUs      = 12000;
    doc.camera.imageWidth      = 1920;
    doc.camera.imageHeight     = 1080;
    doc.camera.captureAccuracy = 2.95;
    doc.algo                   = AlgoThresholds{};

    const int markCount = recipeMarkCount(SystemConfigRepository::load().common.trackMode);
    doc.marks.resize(markCount);
    doc.marks[0] = makeDefaultMark(QStringLiteral("mark_std_t1.tpl"),
                                   QStringLiteral("1轨左下角标准Mark"));
    doc.marks[1] = makeDefaultMark(QStringLiteral("mark_std_t1.tpl"),
                                   QStringLiteral("1轨右上角标准Mark"));
    if (markCount > 2) {
        doc.marks[2] = makeDefaultMark(QStringLiteral("mark_std_t2.tpl"),
                                       QStringLiteral("2轨左下角标准Mark"));
        doc.marks[3] = makeDefaultMark(QStringLiteral("mark_std_t2.tpl"),
                                       QStringLiteral("2轨右上角标准Mark"));
    }

    doc.panelCalib.mode           = CalibMode::Panel;
    doc.panelCalib.fovInfo        = QStringLiteral("aoi.recipe.calib.fov_not_generated");
    doc.panelCalib.pathModeIndex  = 0;
    doc.panelCalib.gerberFileName = QStringLiteral("aoi.recipe.calib.gerber_not_imported");

    doc.positionCalib.trackPosition = 0.0;
    doc.positionCalib.scanPosition  = {};
    return doc;
}

// ── RecipeRepository ───────────────────────────────────────────────────────────

void RecipeRepository::ensureSeeded()
{
    auto &store = recipeStore();
    if (!store.isEmpty())
        return;

    const QVector<QPair<QString, QString>> items = {
        {QStringLiteral("PANEL-A01"), QStringLiteral("PANEL-A01")},
        {QStringLiteral("PANEL-A02"), QStringLiteral("PANEL-A02")},
        {QStringLiteral("PANEL-B01"), QStringLiteral("PANEL-B01")},
        {QStringLiteral("PANEL-C08"), QStringLiteral("PANEL-C08")},
    };
    for (const auto &it : items) {
        RecipeDocument doc = RecipeDocument::defaultDemo(it.first, it.second);
        doc.saveToFile();
        store.insert(it.first, doc);
    }
    defaultIdStore() = QStringLiteral("PANEL-A01");
}

void RecipeRepository::load()
{
    if (storeInitialized())
        return;

    auto &store = recipeStore();
    store.clear();

    const QString paramDir = recipeParamDir();
    QDir          dir(paramDir);
    if (!dir.exists()) {
        if (!QDir().mkpath(paramDir)) {
            finishLoad();
            return;
        }
    }

    const QStringList jsonFiles =
        dir.entryList({QStringLiteral("*.json")}, QDir::Files);
    const QString currentBase =
        QFileInfo(data::currentRecipeFilePath()).fileName();

    for (const QString &fileName : jsonFiles) {
        if (fileName == currentBase)
            continue;
        RecipeDocument doc;
        if (!RecipeDocument::loadFromFile(dir.absoluteFilePath(fileName), doc))
            continue;
        if (doc.id.isEmpty())
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
        RecipeListItem        item;
        item.id   = doc.id;
        item.name = doc.basic.name;
        item.isDefault = (doc.id == defaultId);
        list.append(item);
    }
    std::sort(list.begin(), list.end(),
              [](const RecipeListItem &a, const RecipeListItem &b) { return a.id < b.id; });
    return list;
}

bool RecipeRepository::load(const QString &recipeId, RecipeDocument &out)
{
    ensureStoreLoaded();
    if (recipeId.isEmpty() || !recipeStore().contains(recipeId))
        return false;
    out = recipeStore().value(recipeId);
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
        if (store.contains(toSave.id) && toSave.id != prev)
            return false;
        const QString oldPath = recipeFilePath(prev);
        if (QFileInfo::exists(oldPath))
            QFile::remove(oldPath);
        store.remove(prev);
        if (defaultIdStore() == prev)
            defaultIdStore() = toSave.id;
    }

    if (!toSave.saveToFile())
        return false;

    store.insert(toSave.id, toSave);
    if (toSave.id == defaultIdStore())
        copyRecipeJsonToCurrent(toSave.id);
    return true;
}

bool RecipeRepository::remove(const QString &recipeId)
{
    ensureStoreLoaded();
    if (recipeId.isEmpty() || recipeId == defaultIdStore())
        return false;
    if (!recipeStore().remove(recipeId))
        return false;
    const QString path = recipeFilePath(recipeId);
    if (QFileInfo::exists(path))
        QFile::remove(path);
    return true;
}

bool RecipeRepository::setDefault(const QString &recipeId)
{
    ensureStoreLoaded();
    if (recipeId.isEmpty() || !recipeStore().contains(recipeId))
        return false;
    defaultIdStore() = recipeId;
    return copyRecipeJsonToCurrent(recipeId);
}

QString RecipeRepository::generateNewRecipeId()
{
    ensureStoreLoaded();
    for (int n = 1; n < 1000; ++n) {
        const QString id =
            QStringLiteral("PANEL-NEW-%1").arg(n, 2, 10, QChar(QLatin1Char('0')));
        if (!recipeStore().contains(id))
            return id;
    }
    return QStringLiteral("PANEL-NEW-999");
}

QString RecipeRepository::defaultRecipeId()
{
    ensureStoreLoaded();
    return defaultIdStore();
}

bool RecipeRepository::exportAllToDirectory(const QString &dirPath, RecipeExportReport &report)
{
    ensureStoreLoaded();
    report.exportDirectory = dirPath.trimmed();
    report.items.clear();
    report.timestamp       = QDateTime::currentDateTime();

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

        const QString baseName = sanitizeRecipeExportBaseName(doc.basic.name, doc.id);
        const QString filePath = trimmed + QLatin1Char('/') + baseName + QStringLiteral(".json");
        entry.filePath         = filePath;

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

    report.overallOk = (failCount == 0);
    if (report.overallOk)
        report.overallMessage = QStringLiteral("全部导出成功");
    else if (okCount > 0)
        report.overallMessage = QStringLiteral("部分导出失败");
    else
        report.overallMessage = QStringLiteral("全部导出失败");
    return report.overallOk;
}

void RecipeRepository::writeExportLog(RecipeExportReport &report)
{
    const QString timeStr =
        report.timestamp.toString(QStringLiteral("yyyyMMdd_HHmmss"));
    const QString logPath =
        recipeImportExportLogPath(QStringLiteral("export_") + timeStr + QStringLiteral(".txt"));
    report.logFilePath = logPath;
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

    out << QStringLiteral("导出目录: ") << report.exportDirectory << QLatin1Char('\n');
    out << QStringLiteral("成功: ") << successCount << QStringLiteral(" 失败: ") << failCount
        << QLatin1Char('\n');
    out << QStringLiteral("结果: ") << report.overallMessage << QLatin1Char('\n');
    for (const RecipeExportItemResult &item : report.items) {
        out << item.recipeId << QLatin1Char(' ') << item.filePath << QLatin1Char(' ')
            << (item.ok ? QStringLiteral("OK") : QStringLiteral("FAIL")) << QLatin1Char(' ')
            << item.message << QLatin1Char('\n');
    }
}

bool RecipeRepository::importFromJsonFiles(const QStringList &filePaths,
                                           RecipeImportReport &report)
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
            entry.message = QStringLiteral("解析 JSON 失败");
            report.items.append(entry);
            ++failCount;
            continue;
        }

        normalizeRecipeDocument(doc);
        if (doc.id.isEmpty()) {
            entry.ok      = false;
            entry.message = QStringLiteral("配方 ID 为空");
            report.items.append(entry);
            ++failCount;
            continue;
        }

        if (existingIds.contains(doc.id)) {
            entry.ok      = false;
            entry.message = QStringLiteral("配方 ID 已存在");
            report.items.append(entry);
            ++failCount;
            continue;
        }

        if (!save(doc)) {
            entry.ok      = false;
            entry.message = QStringLiteral("保存配方失败");
            report.items.append(entry);
            ++failCount;
            continue;
        }

        existingIds.insert(doc.id);
        entry.recipeId   = doc.id;
        entry.recipeName = doc.basic.name;
        entry.ok         = true;
        entry.message    = QStringLiteral("导入成功");
        report.items.append(entry);
        ++successCount;
    }

    report.overallOk = (failCount == 0 && successCount > 0);
    if (report.overallOk)
        report.overallMessage = QStringLiteral("全部导入成功");
    else if (successCount > 0)
        report.overallMessage = QStringLiteral("部分导入失败");
    else
        report.overallMessage = QStringLiteral("全部导入失败");
    return report.overallOk;
}

void RecipeRepository::writeImportLog(RecipeImportReport &report)
{
    const QString timeStr =
        report.timestamp.toString(QStringLiteral("yyyyMMdd_HHmmss"));
    const QString logPath =
        recipeImportExportLogPath(QStringLiteral("import_") + timeStr + QStringLiteral(".txt"));
    report.logFilePath = logPath;
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

    out << QStringLiteral("成功: ") << successCount << QStringLiteral(" 失败: ") << failCount
        << QLatin1Char('\n');
    out << QStringLiteral("结果: ") << report.overallMessage << QLatin1Char('\n');
    for (const RecipeImportItemResult &item : report.items) {
        out << item.filePath << QLatin1Char(' ') << item.recipeId << QLatin1Char(' ')
            << (item.ok ? QStringLiteral("OK") : QStringLiteral("FAIL")) << QLatin1Char(' ')
            << item.message << QLatin1Char('\n');
    }
}

void RecipeRepository::setStorageDirectory(const QString &dir)
{
    recipeStorageDirectory() = QDir(dir).absolutePath();
}

QString RecipeRepository::storageDirectory()
{
    return recipeParamDir();
}

} // namespace aoi
