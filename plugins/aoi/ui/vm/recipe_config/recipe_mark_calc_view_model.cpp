/**
 * @file recipe_mark_calc_view_model.cpp
 */

#include "recipe_mark_calc_view_model.h"
#include "recipe_config_view_model.h"

#include "system_config_model.h"
#include "image/image_info_display.h"

#include <uibase/languagemanager.h>

namespace aoi {

RecipeMarkCalcViewModel::RecipeMarkCalcViewModel(RecipeConfigViewModel *host, QObject *parent)
    : QObject(parent)
    , m_host(host)
{
    if (m_host)
        m_markLocateCache.resize(markSlotCount());
}

int RecipeMarkCalcViewModel::markSlotCount() const
{
    return recipeMarkCount(SystemConfigRepository::load().common.trackMode);
}

void RecipeMarkCalcViewModel::clearLocateCache()
{
    m_markLocateResult = {};
    m_markLocateCache.fill(MarkLocateResult{}, markSlotCount());
    emit markLocateResultChanged();
}

MarkLocateResult RecipeMarkCalcViewModel::markLocateResultForSlot(int slot) const
{
    if (slot < 0 || slot >= m_markLocateCache.size())
        return {};
    return m_markLocateCache.at(slot);
}

void RecipeMarkCalcViewModel::setMarkCalculating(bool calculating)
{
    if (m_markCalculating == calculating)
        return;
    m_markCalculating = calculating;
    emit markCalculatingChanged();
}

void RecipeMarkCalcViewModel::runMarkCalcFromImage(const QImage &image, int markSlotIndex)
{
    if (!m_host || m_markCalculating)
        return;

    if (markSlotIndex < 0 || markSlotIndex >= m_host->document().marks.size()) {
        m_host->setStatus(TR("aoi.recipe.mark.locate_failed"));
        return;
    }

    if (image.isNull()) {
        m_host->setStatus(TR("aoi.recipe.mark.no_preview"));
        return;
    }

    m_markLocateSlot = markSlotIndex;
    setMarkCalculating(true);
    m_host->setStatus(TR("aoi.recipe.mark.calc_running"));

    const MarkLocateResult result = MarkPointLocator::locate(
        image_display::qImageToRgbImageInfo(image), m_host->document().marks.at(markSlotIndex));
    finishMarkCalc(result);
}

void RecipeMarkCalcViewModel::finishMarkCalc(const MarkLocateResult &result)
{
    if (!m_host)
        return;

    setMarkCalculating(false);

    m_markLocateResult = result;
    if (m_markLocateCache.size() < markSlotCount())
        m_markLocateCache.resize(markSlotCount());
    m_markLocateCache[m_markLocateSlot] = result;
    emit markLocateResultChanged();

    if (result.ok) {
        m_host->setStatus(TR("aoi.recipe.mark.calc_success"));
        return;
    }

    m_host->setStatus(!result.message.isEmpty()
                          ? result.message
                          : TR(result.errorKey.isEmpty() ? QStringLiteral("aoi.recipe.mark.locate_failed")
                                                         : result.errorKey));
}

} // namespace aoi
