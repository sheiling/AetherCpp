#include "capture_profile.h"

namespace aoi::capture {
namespace {

/** primary 非零取 primary，否则取 fallback */
double pickZ(double primary, double fallback)
{
    if (!qFuzzyIsNull(primary))
        return primary;
    return fallback;
}

} // namespace

double resolveMarkCaptureZ(const RecipeDocument &recipe, int markSlot)
{
    const double scanZ = recipe.positionCalib.scanPosition.z;
    if (markSlot < 0 || markSlot >= recipe.marks.size())
        return scanZ;
    return pickZ(recipe.marks[markSlot].capturePos.z, scanZ);
}

double resolveFovCaptureZ(const RecipeDocument &recipe)
{
    const double scanZ = recipe.positionCalib.scanPosition.z;
    if (!recipe.marks.isEmpty())
        return pickZ(scanZ, recipe.marks[0].capturePos.z);
    return scanZ;
}

CaptureProfile buildCaptureProfile(const AoiSystemConfig &sysConfig, const RecipeDocument &recipe)
{
    CaptureProfile profile;
    profile.axisZName       = sysConfig.common.axisZName;
    profile.hasZAxis        = !profile.axisZName.isEmpty();
    profile.exposureUs      = recipe.camera.exposureUs;
    profile.lightBrightness = recipe.camera.lightBrightness;
    profile.mmPerPixel      = recipe.camera.captureAccuracy > 0.0 ? recipe.camera.captureAccuracy
                                                                    : 1.0;
    profile.markCaptureZmm  = resolveMarkCaptureZ(recipe, 0);
    profile.fovCaptureZmm   = resolveFovCaptureZ(recipe);
    return profile;
}

} // namespace aoi::capture
