/**
 * @file virtual_pixel_view_model.cpp
 * @brief VirtualPixelViewModel 文档加载/保存与通道配置实现
 */

#include "virtual_pixel_view_model.h"

#include "home_model.h"
#include "lighten_param_persist.h"
#include "recipe_model.h"

#include <QDateTime>
#include <QMetaType>

/** @brief VirtualPixelViewModel 成员实现 */
namespace lighten {

VirtualPixelViewModel::VirtualPixelViewModel(QObject *parent)
    : ViewModel(parent)
{
    qRegisterMetaType<VirtualPixelDocument>("VirtualPixelDocument");
    qRegisterMetaType<VirtualPixelChannelConfig>("VirtualPixelChannelConfig");

    VirtualPixelRepository::load();
    RecipeRepository::load();

    VirtualPixelDocument initial;
    const QString defaultId = RecipeRepository::defaultRecipeId();
    if (!VirtualPixelRepository::load(defaultId, initial))
        initial = VirtualPixelDocument::defaultDemo(defaultId);

    registerProperty(PROP_DOCUMENT, QVariant::fromValue(initial));
    registerProperty(PROP_ACTIVE_CHANNEL, 0);
    registerProperty(PROP_STATUS, QStringLiteral("就绪"));

    m_saveCommand = new FunctionalCommand([this]() { slot_save(); }, nullptr, this);
    m_saveCommand->setName(QStringLiteral("virtualPixel.save"));

    m_loadRecipeCommand = new FunctionalCommand([this]() {
        slot_loadForRecipe(document().selectedRecipeId);
    },
                                                nullptr, this);
    m_loadRecipeCommand->setName(QStringLiteral("virtualPixel.loadRecipe"));
}

void VirtualPixelViewModel::slot_loadForRecipe(const QString &recipeId)
{
    if (recipeId.isEmpty()) {
        setStatus(QStringLiteral("请先选择配方"));
        return;
    }

    VirtualPixelDocument doc;
    if (!VirtualPixelRepository::load(recipeId, doc)) {
        setStatus(QStringLiteral("加载配方 %1 的虚拟像素配置失败").arg(recipeId));
        return;
    }

    doc.selectedRecipeId = recipeId;
    setDocument(doc);
    setStatus(QStringLiteral("已加载配方 %1 的虚拟像素配置").arg(recipeId));
}

void VirtualPixelViewModel::slot_save()
{
    const QString recipeId = document().selectedRecipeId;
    if (recipeId.isEmpty()) {
        setStatus(QStringLiteral("请先选择配方"));
        return;
    }
    VirtualPixelDocument before;
    if (!VirtualPixelRepository::load(recipeId, before))
        before = VirtualPixelDocument::defaultDemo(recipeId);
    const VirtualPixelDocument after = document();
    if (VirtualPixelRepository::save(after)) {
        ParameterChangeLogger::logVirtualPixelDocumentSave(before, after);
        if (status() != QStringLiteral("虚拟像素配置已保存"))
            setStatus(QStringLiteral("虚拟像素配置已保存"));
        modifyDocument([&](VirtualPixelDocument &doc) {
            RunLogEntry e;
            e.timestamp = QDateTime::currentDateTime();
            e.level     = QStringLiteral("INFO");
            e.message   = QStringLiteral("虚拟像素配置已保存");
            doc.logs.append(e);
        });
        emit sig_saveSucceeded();
    } else {
        setStatus(QStringLiteral("保存虚拟像素配置失败"));
    }
}

void VirtualPixelViewModel::slot_setChannelConfig(int rgbIndex, const VirtualPixelChannelConfig &cfg)
{
    if (rgbIndex < 0 || rgbIndex > 2)
        return;
    modifyDocument([&](VirtualPixelDocument &doc) { doc.channels[rgbIndex] = cfg; });
}

void VirtualPixelViewModel::slot_appendLog(const QString &message, const QString &level)
{
    modifyDocument([&](VirtualPixelDocument &doc) {
        RunLogEntry e;
        e.timestamp = QDateTime::currentDateTime();
        e.level     = level;
        e.message   = message;
        doc.logs.append(e);
    });
}

} // namespace lighten
