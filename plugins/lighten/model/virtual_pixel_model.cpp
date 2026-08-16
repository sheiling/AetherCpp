/**
 * @file virtual_pixel_model.cpp
 * @brief VirtualPixelDocument JSON 读写与默认值实现
 */

#include "virtual_pixel_model.h"

#include "lighten_param_persist.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>

/** @brief VirtualPixelDocument 持久化与 VirtualPixelRepository 内存缓存实现 */
namespace lighten {

bool VirtualPixelDocument::operator==(const VirtualPixelDocument &o) const
{
    return selectedRecipeId == o.selectedRecipeId && observe == o.observe && logs == o.logs
           && channels[0] == o.channels[0] && channels[1] == o.channels[1]
           && channels[2] == o.channels[2];
}

VirtualPixelDocument VirtualPixelDocument::defaultDemo(const QString &recipeId)
{
    VirtualPixelDocument doc;
    doc.selectedRecipeId = recipeId;
    for (int i = 0; i < 3; ++i) {
        doc.channels[i].layoutDemoId = 0;
        doc.channels[i].lampRows     = 64;
        doc.channels[i].lampCols     = 128;
        doc.channels[i].mapRule0     = QStringLiteral("R→列, G→行");
        doc.channels[i].mapRule1     = QStringLiteral("B→预留");
        doc.channels[i].mapRule2     = QString();
    }
    const QDateTime now = QDateTime::currentDateTime();
    doc.logs.append({now, QStringLiteral("INFO"), QStringLiteral("虚拟像素配置已加载")});
    return doc;
}

/** 配方 ID → 虚拟像素文档的进程内缓存与懒加载辅助 */
namespace {

QHash<QString, VirtualPixelDocument> &vpStore()
{
    static QHash<QString, VirtualPixelDocument> store;
    return store;
}

bool &vpStoreInitialized()
{
    static bool initialized = false;
    return initialized;
}

void normalizeVirtualPixelDocument(VirtualPixelDocument &doc, const QString &recipeId)
{
    if (doc.selectedRecipeId.isEmpty())
        doc.selectedRecipeId = recipeId;

    const VirtualPixelDocument fallback = VirtualPixelDocument::defaultDemo(recipeId);
    for (int i = 0; i < 3; ++i) {
        if (doc.channels[i].lampRows <= 0)
            doc.channels[i].lampRows = fallback.channels[i].lampRows;
        if (doc.channels[i].lampCols <= 0)
            doc.channels[i].lampCols = fallback.channels[i].lampCols;
    }
}

void ensureVpStoreLoaded()
{
    if (vpStoreInitialized())
        return;
    VirtualPixelRepository::load();
}

void seedVpStoreFromRecipes()
{
    auto &store = vpStore();
    if (!store.isEmpty())
        return;

    const QVector<QString> ids = {
        QStringLiteral("PANEL-A01"),
        QStringLiteral("PANEL-A02"),
        QStringLiteral("PANEL-B01"),
        QStringLiteral("PANEL-C08"),
    };
    for (const QString &id : ids)
        store.insert(id, VirtualPixelDocument::defaultDemo(id));
}

QString recipeIdFromFileName(const QString &fileName)
{
    return QFileInfo(fileName).completeBaseName();
}

} // namespace

bool VirtualPixelDocument::saveToFilePath(const QString &filePath) const
{
    if (filePath.isEmpty() || selectedRecipeId.isEmpty())
        return false;

    try {
        json root;
        if (!recipe_file::readRoot(filePath, root))
            return false;

        json virtualPixel;
        reflectToJson(virtualPixel);
        root["virtualPixel"] = virtualPixel;
        return recipe_file::writeRoot(filePath, root);
    } catch (const std::exception &) {
        return false;
    }
}

bool VirtualPixelDocument::saveToFile() const
{
    return saveToFilePath(recipe_file::filePath(selectedRecipeId));
}

bool VirtualPixelDocument::loadFromFile(const QString &filePath, VirtualPixelDocument &out)
{
    try {
        json root;
        if (!recipe_file::readRoot(filePath, root))
            return false;
        if (!root.contains("virtualPixel") || !root.at("virtualPixel").is_object())
            return false;

        out.reflectFromJson(root.at("virtualPixel"));
        const QString recipeId = recipeIdFromFileName(QFileInfo(filePath).fileName());
        normalizeVirtualPixelDocument(out, recipeId);
        return true;
    } catch (const std::exception &) {
        return false;
    }
}

void VirtualPixelRepository::load()
{
    const QString recipeDir = recipe_file::recipeDir();
    QDir dir(recipeDir);
    auto &store = vpStore();
    store.clear();

    if (!dir.exists()) {
        seedVpStoreFromRecipes();
        vpStoreInitialized() = true;
        return;
    }

    const QStringList jsonFiles = dir.entryList({QStringLiteral("*.json")}, QDir::Files);
    if (jsonFiles.isEmpty()) {
        seedVpStoreFromRecipes();
        vpStoreInitialized() = true;
        return;
    }

    for (const QString &fileName : jsonFiles) {
        const QString path     = dir.absoluteFilePath(fileName);
        const QString recipeId = recipeIdFromFileName(fileName);

        VirtualPixelDocument doc;
        if (VirtualPixelDocument::loadFromFile(path, doc)) {
            doc.selectedRecipeId = recipeId;
            store.insert(recipeId, doc);
            continue;
        }

        doc = VirtualPixelDocument::defaultDemo(recipeId);
        store.insert(recipeId, doc);
    }

    if (store.isEmpty())
        seedVpStoreFromRecipes();
    vpStoreInitialized() = true;
}

bool VirtualPixelRepository::load(const QString &recipeId, VirtualPixelDocument &out)
{
    ensureVpStoreLoaded();
    if (recipeId.isEmpty())
        return false;

    const auto it = vpStore().constFind(recipeId);
    if (it == vpStore().constEnd()) {
        if (!QFileInfo::exists(recipe_file::filePath(recipeId)))
            return false;
        out = VirtualPixelDocument::defaultDemo(recipeId);
        out.selectedRecipeId = recipeId;
        return true;
    }

    out                  = it.value();
    out.selectedRecipeId = recipeId;
    normalizeVirtualPixelDocument(out, recipeId);
    return true;
}

bool VirtualPixelRepository::contains(const QString &recipeId)
{
    ensureVpStoreLoaded();
    if (recipeId.isEmpty())
        return false;
    return vpStore().contains(recipeId) || QFileInfo::exists(recipe_file::filePath(recipeId));
}

bool VirtualPixelRepository::save(const VirtualPixelDocument &doc)
{
    ensureVpStoreLoaded();
    if (doc.selectedRecipeId.isEmpty())
        return false;

    VirtualPixelDocument toSave = doc;
    normalizeVirtualPixelDocument(toSave, toSave.selectedRecipeId);

    if (!toSave.saveToFile())
        return false;

    vpStore().insert(toSave.selectedRecipeId, toSave);
    return true;
}

void VirtualPixelRepository::evictFromStore(const QString &recipeId)
{
    if (recipeId.isEmpty())
        return;
    vpStore().remove(recipeId);
}

bool VirtualPixelRepository::migrateRecipeId(const QString &fromId, const QString &toId)
{
    ensureVpStoreLoaded();
    if (fromId.isEmpty() || toId.isEmpty() || fromId == toId)
        return true;

    VirtualPixelDocument doc;
    const auto it = vpStore().constFind(fromId);
    if (it != vpStore().constEnd())
        doc = it.value();
    else if (!VirtualPixelDocument::loadFromFile(recipe_file::filePath(fromId), doc))
        doc = VirtualPixelDocument::defaultDemo(fromId);

    doc.selectedRecipeId = toId;
    normalizeVirtualPixelDocument(doc, toId);
    vpStore().remove(fromId);

    json root;
    if (!recipe_file::readRoot(recipe_file::filePath(toId), root))
        root = json::object();

    json virtualPixel;
    doc.reflectToJson(virtualPixel);
    root["virtualPixel"] = virtualPixel;
    if (!recipe_file::writeRoot(recipe_file::filePath(toId), root))
        return false;

    vpStore().insert(toId, doc);
    return true;
}

} // namespace lighten
