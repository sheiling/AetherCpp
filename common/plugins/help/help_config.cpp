/**
 * @file help_config.cpp
 * @brief help.version / help.documents 读取与 exe 相对路径解析
 */

#include "help_config.h"

#include <Config.h>

#include <QCoreApplication>
#include <QDir>

namespace help {

namespace {

QString cfgQString(const char *key, const QString &fallback = {})
{
    const std::string v = config::ConfigManager::get<std::string>(key, fallback.toStdString());
    return QString::fromStdString(v);
}

/** help 键非空则用 help，否则回退 app 键，再否则 defaultValue */
QString helpOrAppString(const char *helpKey, const char *appKey, const QString &defaultValue)
{
    const QString helpVal = cfgQString(helpKey);
    if (!helpVal.isEmpty())
        return helpVal;
    if (appKey != nullptr) {
        const QString appVal = cfgQString(appKey);
        if (!appVal.isEmpty())
            return appVal;
    }
    return defaultValue;
}

} // namespace

HelpVersionInfo loadHelpVersionInfo()
{
    HelpVersionInfo info;
    info.appName = helpOrAppString("help.version.app_name", "app.name",
                                   QStringLiteral("Aether Platform"));
    info.currentVersion = helpOrAppString("help.version.current_version", "app.version",
                                          QStringLiteral("1.0.0"));
    info.buildTime = helpOrAppString("help.version.build_time", "app.build_date", {});
    info.updateTime = cfgQString("help.version.update_time");
    return info;
}

HelpDocuments loadHelpDocuments()
{
    HelpDocuments docs;
    docs.userManual = cfgQString("help.documents.user_manual",
                                 QStringLiteral("help/用户手册.pdf"));
    docs.maintenance = cfgQString("help.documents.maintenance",
                                  QStringLiteral("help/维护手册.pdf"));
    docs.helpDoc = cfgQString("help.documents.help_doc",
                              QStringLiteral("help/帮助文档.pdf"));
    return docs;
}

QString resolveRuntimePath(const QString &path)
{
    if (path.isEmpty())
        return path;

    const QDir base(QCoreApplication::applicationDirPath());
    if (QDir::isAbsolutePath(path))
        return QDir::cleanPath(path);
    return QDir::cleanPath(base.filePath(path));
}

} // namespace help
