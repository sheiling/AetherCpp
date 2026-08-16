#pragma once

/**
 * @file data.h
 * @brief AOI 插件运行时数据路径（相对可执行文件目录 `param/` 下）
 */

#include <QString>

namespace aoi::data {

/** 参数根目录名（`<exe>/param`） */
inline constexpr char kParamDirName[] = "param";
/** 配方 JSON 子目录名（`<exe>/param/recipe`） */
inline constexpr char kRecipeDirName[] = "recipe";
/** 系统配置文件名 */
inline constexpr char kSystemConfigFileName[] = "aoi_system.json";
/** 当前默认配方副本文件名 */
inline constexpr char kCurrentRecipeFileName[] = "current_recipe.json";
/** 配方单文件后缀 */
inline constexpr char kRecipeFileSuffix[] = ".json";

/** `<applicationDir>/param` */
QString paramRootDir();

/** `<applicationDir>/param/recipe` */
QString recipeStorageDir();

/** `<applicationDir>/param/aoi_system.json` */
QString systemConfigFilePath();

/** `<applicationDir>/param/current_recipe.json` */
QString currentRecipeFilePath();

/** `<applicationDir>/param/recipe/{recipeId}.json` */
QString recipeFilePath(const QString &recipeId);

} // namespace aoi::data
