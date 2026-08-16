#include "data.h"

#include <QCoreApplication>
#include <QDir>

namespace aoi::data {
namespace {

QString joinUnderExe(const QString &relativePath)
{
    return QDir(QCoreApplication::applicationDirPath()).filePath(relativePath);
}

} // namespace

QString paramRootDir()
{
    return joinUnderExe(QString::fromLatin1(kParamDirName));
}

QString recipeStorageDir()
{
    return QDir(paramRootDir()).filePath(QString::fromLatin1(kRecipeDirName));
}

QString systemConfigFilePath()
{
    return QDir(paramRootDir()).filePath(QString::fromLatin1(kSystemConfigFileName));
}

QString currentRecipeFilePath()
{
    return QDir(paramRootDir()).filePath(QString::fromLatin1(kCurrentRecipeFileName));
}

QString recipeFilePath(const QString &recipeId)
{
    return QDir(recipeStorageDir()).filePath(recipeId + QString::fromLatin1(kRecipeFileSuffix));
}

} // namespace aoi::data
