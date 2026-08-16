/**
 * @file lighten_param_persist.cpp
 * @brief 设备配置读取、参数变更日志与配方 JSON 持久化实现
 */

#include "lighten_param_persist.h"

#include <QCoreApplication>
#include <QDate>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSet>
#include <QTextStream>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#endif

#include <common/3rdparty/nlohmann/json.hpp>
#include <common/core/container/application.h>
#include <common/core/device/deviceManager/device_manager.h>

#include <fstream>

/** @brief DeviceConfigReader、ParameterChangeLogger 与 recipe_file 实现 */
namespace lighten {

namespace config_dir {

QString root()
{
    return QCoreApplication::applicationDirPath() + QStringLiteral("/config");
}

} // namespace config_dir

/** device_config.json 读取辅助 */
namespace {

// --- device config ---

std::string deviceConfigFilePath()
{
    return QDir(config_dir::root()).filePath(QStringLiteral("device_config.json")).toStdString();
}

// --- parameter change log ---

QString parameterChangeLogDir()
{
    return QCoreApplication::applicationDirPath() + QStringLiteral("/parameter_change");
}

QString parameterChangeDailyLogPath()
{
    const QString dir = parameterChangeLogDir();
    if (!QDir().mkpath(dir))
        return {};
    const QString fileName =
        QDate::currentDate().toString(QStringLiteral("yyyy-MM-dd")) + QStringLiteral(".txt");
    return dir + QLatin1Char('/') + fileName;
}

void setUtf8Stream(QTextStream &out)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    out.setEncoding(QStringConverter::Utf8);
#else
    out.setCodec("UTF-8");
#endif
}

QString jsonValueToDisplayString(const json &v)
{
    if (v.is_null())
        return QStringLiteral("null");
    if (v.is_string())
        return QString::fromStdString(v.get<std::string>());
    if (v.is_boolean())
        return v.get<bool>() ? QStringLiteral("true") : QStringLiteral("false");
    if (v.is_number_integer())
        return QString::number(v.get<long long>());
    if (v.is_number_unsigned())
        return QString::number(v.get<unsigned long long>());
    if (v.is_number_float())
        return QString::number(v.get<double>());
    return QString::fromStdString(v.dump());
}

void collectJsonDiffLines(const json &before,
                          const json &after,
                          const QString &path,
                          QStringList &lines)
{
    if (before == after)
        return;

    const bool beforeStructured = before.is_object() || before.is_array();
    const bool afterStructured  = after.is_object() || after.is_array();
    if (!beforeStructured || !afterStructured || before.type() != after.type()) {
        lines.append(QStringLiteral("%1: %2 -> %3")
                         .arg(path.isEmpty() ? QStringLiteral("(root)") : path,
                              jsonValueToDisplayString(before),
                              jsonValueToDisplayString(after)));
        return;
    }

    if (before.is_object()) {
        QSet<QString> keys;
        for (auto it = before.begin(); it != before.end(); ++it)
            keys.insert(QString::fromStdString(it.key()));
        for (auto it = after.begin(); it != after.end(); ++it)
            keys.insert(QString::fromStdString(it.key()));

        const QList<QString> sortedKeys = keys.values();
        for (const QString &key : sortedKeys) {
            const std::string stdKey = key.toStdString();
            const json        jb     = before.contains(stdKey) ? before.at(stdKey) : json();
            const json        ja     = after.contains(stdKey) ? after.at(stdKey) : json();
            const QString     childPath =
                path.isEmpty() ? key : path + QLatin1Char('.') + key;
            collectJsonDiffLines(jb, ja, childPath, lines);
        }
        return;
    }

    const size_t count = (std::max)(before.size(), after.size());
    for (size_t i = 0; i < count; ++i) {
        const json jb = i < before.size() ? before.at(i) : json();
        const json ja = i < after.size() ? after.at(i) : json();
        collectJsonDiffLines(jb, ja, path + QLatin1Char('[') + QString::number(i) + QLatin1Char(']'),
                             lines);
    }
}

json toJson(const SystemConfig &cfg)
{
    json j;
    cfg.reflectToJson(j);
    return j;
}

json toJson(const RecipeDocument &doc)
{
    json j;
    doc.reflectToJson(j);
    return j;
}

json toJson(const VirtualPixelDocument &doc)
{
    json j;
    doc.reflectToJson(j);
    return j;
}

} // namespace

/** DeviceConfigReader：device_config.json 解析辅助 */
namespace {

nlohmann::json loadDeviceConfigRoot()
{
    const std::string configPath = deviceConfigFilePath();
    std::ifstream file(configPath);
    if (!file.is_open()) {
        qWarning() << "[DeviceConfigReader] Cannot open device config:"
                   << QString::fromStdString(configPath);
        return {};
    }

    nlohmann::json root;
    try {
        file >> root;
    } catch (const std::exception &e) {
        qWarning() << "[DeviceConfigReader] device_config.json parse error:" << e.what();
        return {};
    }

    if (!root.is_object()) {
        qWarning() << "[DeviceConfigReader] device_config.json root must be an object.";
        return {};
    }
    return root;
}

nlohmann::json loadDeviceManagerSection(const char *section)
{
    const nlohmann::json root = loadDeviceConfigRoot();
    if (root.empty())
        return {};

    const nlohmann::json &deviceManager = root.value("DeviceManager", nlohmann::json::object());
    const nlohmann::json &sectionObj = deviceManager.value(section, nlohmann::json::object());
    return sectionObj.is_object() ? sectionObj : nlohmann::json::object();
}

nlohmann::json loadDeviceConfigCameraObject()
{
    return loadDeviceManagerSection("Camera");
}

std::vector<std::string> collectJsonObjectKeys(const nlohmann::json &obj)
{
    std::vector<std::string> keys;
    if (!obj.is_object())
        return keys;
    keys.reserve(obj.size());
    for (auto it = obj.begin(); it != obj.end(); ++it)
        keys.push_back(it.key());
    return keys;
}

} // namespace

std::vector<std::string> DeviceConfigReader::loadCameraKeys()
{
    const nlohmann::json camera = loadDeviceConfigCameraObject();
    std::vector<std::string> keys;
    keys.reserve(camera.size());
    for (auto it = camera.begin(); it != camera.end(); ++it) {
        keys.push_back(it.key());
    }
    return keys;
}

std::vector<std::string> DeviceConfigReader::loadCameraPaths()
{
    const nlohmann::json camera = loadDeviceConfigCameraObject();
    std::vector<std::string> paths;
    paths.reserve(camera.size());
    for (auto it = camera.begin(); it != camera.end(); ++it) {
        if (it->is_object() && it->contains("path") && (*it)["path"].is_string()) {
            paths.push_back((*it)["path"].get<std::string>());
        } else {
            paths.push_back({});
        }
    }
    return paths;
}

std::vector<std::string> DeviceConfigReader::loadFlashDetectKeys()
{
    return collectJsonObjectKeys(loadDeviceManagerSection("FlashDetect"));
}

std::vector<std::string> DeviceConfigReader::loadScannerKeys()
{
    return collectJsonObjectKeys(loadDeviceManagerSection("Scanner"));
}

std::vector<std::string> DeviceConfigReader::loadMotionCardNames()
{
    const nlohmann::json motion = loadDeviceManagerSection("MotionController");
    const nlohmann::json cards = motion.value("Card", nlohmann::json::object());
    return collectJsonObjectKeys(cards.is_object() ? cards : nlohmann::json::object());
}

std::unordered_map<std::string, std::string> DeviceConfigReader::loadCameraMap()
{
    const nlohmann::json camera = loadDeviceConfigCameraObject();
    std::unordered_map<std::string, std::string> cameras;
    cameras.reserve(camera.size());

    for (auto it = camera.begin(); it != camera.end(); ++it) {
        std::string path;
        if (it->is_object() && it->contains("path") && (*it)["path"].is_string()) {
            path = (*it)["path"].get<std::string>();
        }
        cameras.emplace(it.key(), std::move(path));
    }
    return cameras;
}

bool DeviceConfigReader::loadCameraImageConfig(const QString &cameraName, CameraImageConfig *out)
{
    if (!out || cameraName.isEmpty())
        return false;

    const nlohmann::json camera = loadDeviceConfigCameraObject();
    const std::string key       = cameraName.toStdString();
    const auto        it        = camera.find(key);
    if (it == camera.end() || !it->is_object())
        return false;

    const nlohmann::json &entry = *it;
    out->xResolution            = entry.value("xResolution", 0);
    out->yResolution            = entry.value("yResolution", 0);
    out->pixelFormat.clear();
    if (entry.contains("pixelFormat") && entry["pixelFormat"].is_string())
        out->pixelFormat = QString::fromStdString(entry["pixelFormat"].get<std::string>());

    return out->xResolution > 0 && out->yResolution > 0 && !out->pixelFormat.isEmpty();
}

std::vector<std::string> DeviceConfigReader::loadSoftScreenKeys()
{
    return collectJsonObjectKeys(loadDeviceManagerSection("SoftwareScreen"));
}

std::shared_ptr<common::device::DeviceManager> tryGetDeviceManager()
{
    try {
        auto *app = Container::Application::instance();
        if (!app->bound<common::device::DeviceManager>())
            return nullptr;
        return app->make<common::device::DeviceManager>();
    } catch (const std::exception &e) {
        qWarning() << "[DeviceConfigReader] DeviceManager unavailable:" << e.what();
        return nullptr;
    }
}

// --- ParameterChangeLogger ---

void ParameterChangeLogger::appendDocumentChange(const QString &pageTitle,
                                               const QString &typeName,
                                               const json &before,
                                               const json &after)
{
    const QString logPath = parameterChangeDailyLogPath();
    if (logPath.isEmpty())
        return;

    QFile file(logPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
        return;

    QTextStream out(&file);
    setUtf8Stream(out);

    QStringList diffLines;
    collectJsonDiffLines(before, after, QString(), diffLines);

    out << QStringLiteral(
               "================================================================================\n");
    out << QStringLiteral("时间: ")
        << QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"))
        << QLatin1Char('\n');
    out << QStringLiteral("页面: ") << pageTitle << QLatin1Char('\n');
    out << QStringLiteral("类型: ") << typeName << QLatin1Char('\n');

    out << QStringLiteral("\n--- 参数变更明细 ---\n");
    if (diffLines.isEmpty()) {
        out << QStringLiteral("无参数变更\n");
    } else {
        for (const QString &line : diffLines)
            out << line << QLatin1Char('\n');
    }

    out << QStringLiteral("\n--- 更改前 ---\n");
    out << QString::fromStdString(before.dump(4)) << QLatin1Char('\n');
    out << QStringLiteral("\n--- 更改后 ---\n");
    out << QString::fromStdString(after.dump(4)) << QLatin1Char('\n');
    out << QLatin1Char('\n');
}

void ParameterChangeLogger::logSystemConfigSave(const SystemConfig &before,
                                                const SystemConfig &after)
{
    appendDocumentChange(QStringLiteral("系统配置"), QStringLiteral("SystemConfig"),
                         toJson(before), toJson(after));
}

void ParameterChangeLogger::logRecipeDocumentSave(const RecipeDocument &before,
                                                  const RecipeDocument &after)
{
    appendDocumentChange(QStringLiteral("产品配方配置"), QStringLiteral("RecipeDocument"),
                         toJson(before), toJson(after));
}

void ParameterChangeLogger::logVirtualPixelDocumentSave(const VirtualPixelDocument &before,
                                                        const VirtualPixelDocument &after)
{
    appendDocumentChange(QStringLiteral("虚拟像素"), QStringLiteral("VirtualPixelDocument"),
                         toJson(before), toJson(after));
}

/** 配方 JSON 文件路径与读写（config/recipe 目录） */
namespace recipe_file {

QString recipeDir()
{
    return config_dir::root() + QStringLiteral("/recipe");
}

QString filePath(const QString &recipeId)
{
    return recipeDir() + QLatin1Char('/') + recipeId + QStringLiteral(".json");
}

bool readRoot(const QString &filePath, json &root)
{
    root = json::object();
    if (filePath.isEmpty())
        return false;

    QFile file(filePath);
    if (!file.exists())
        return true;

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    try {
        root = json::parse(file.readAll().toStdString());
        if (!root.is_object())
            root = json::object();
        return true;
    } catch (const std::exception &) {
        return false;
    }
}

bool writeRoot(const QString &filePath, const json &root)
{
    if (filePath.isEmpty())
        return false;

    try {
        const QFileInfo fileInfo(filePath);
        if (!QDir().mkpath(fileInfo.absolutePath()))
            return false;

        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            return false;
        file.write(root.dump(4).c_str());
        return true;
    } catch (const std::exception &) {
        return false;
    }
}

} // namespace recipe_file

} // namespace lighten
