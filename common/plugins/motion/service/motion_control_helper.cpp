#include "motion_control_helper.h"

#include <common/core/container/application.h>
#include <common/core/device/deviceManager/device_manager.h>

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>

#include <3rdparty/nlohmann/json.hpp>

#include <cctype>
#include <string>
#include <vector>

using ordered_json = nlohmann::ordered_json;

namespace motion {

namespace {

QString configRootDir()
{
    return QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("config"));
}

QString deviceConfigFilePath()
{
    return QDir(configRootDir()).filePath(QStringLiteral("device_config.json"));
}

QString ioCombineConfigPath()
{
    return QDir(configRootDir()).filePath(QStringLiteral("ioCombine.json"));
}

int orderedJsonToInt(const ordered_json &v, int fallback = 0)
{
    if (v.is_number_integer() || v.is_number_unsigned() || v.is_number_float())
        return v.get<int>();
    if (v.is_string()) {
        try {
            return std::stoi(v.get<std::string>());
        } catch (...) {
            return fallback;
        }
    }
    return fallback;
}

bool orderedJsonToBool(const ordered_json &v, bool fallback = false)
{
    if (v.is_boolean())
        return v.get<bool>();
    if (v.is_number_integer() || v.is_number_unsigned() || v.is_number_float())
        return v.get<int>() != 0;
    if (v.is_string()) {
        std::string s = v.get<std::string>();
        while (!s.empty() && (s.front() == ' ' || s.front() == '\t'))
            s.erase(s.begin());
        while (!s.empty() && (s.back() == ' ' || s.back() == '\t'))
            s.pop_back();
        for (char &c : s)
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return s == "true" || s == "1";
    }
    return fallback;
}

ordered_json loadOrderedJsonFile(const QString &path, const char *logContext)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << logContext << "Cannot open:" << path;
        return {};
    }
    try {
        return ordered_json::parse(file.readAll().constData());
    } catch (const std::exception &e) {
        qWarning() << logContext << "JSON parse error:" << e.what();
        return {};
    }
}

const ordered_json *motionControllerSection(const ordered_json &root, const char *logContext)
{
    if (!root.contains("DeviceManager") || !root["DeviceManager"].is_object()) {
        qWarning() << logContext << "DeviceManager section is empty or missing.";
        return nullptr;
    }
    const ordered_json &deviceManager = root["DeviceManager"];
    if (!deviceManager.contains("MotionController") || !deviceManager["MotionController"].is_object()) {
        qWarning() << logContext << "MotionController section is empty or missing.";
        return nullptr;
    }
    return &deviceManager["MotionController"];
}

bool getPortBitValue(const std::vector<uint32_t> &vecValue, uint32_t bit, bool &value)
{
    const uint32_t wordIndex = bit / 32u;
    const uint32_t bitIndex  = bit % 32u;
    if (wordIndex >= vecValue.size())
        return false;
    value = ((vecValue[wordIndex] >> bitIndex) & 1u) != 0;
    return true;
}

QHash<QString, bool> mapPortBitmapToIoStates(const std::vector<uint32_t>        &vecValue,
                                              const QVector<MotionIoPointConfig> &configs)
{
    QHash<QString, bool> result;
    result.reserve(configs.size());

    for (const MotionIoPointConfig &cfg : configs) {
        bool bitValue = false;
        if (!getPortBitValue(vecValue, static_cast<uint32_t>(cfg.port), bitValue))
            continue;

        const uint32_t logic = cfg.highActive ? 1u : 0u;
        result.insert(cfg.name, static_cast<uint32_t>(bitValue) == logic);
    }
    return result;
}

QVector<MotionIoPointConfig> parseDeviceIo(const ordered_json &root, const char *logContext)
{
    QVector<MotionIoPointConfig> result;

    const ordered_json *motionController = motionControllerSection(root, logContext);
    if (!motionController || !motionController->contains("DeviceIo")
        || !(*motionController)["DeviceIo"].is_object()) {
        qWarning() << logContext << "DeviceIo is empty or missing.";
        return result;
    }

    const ordered_json &deviceIo = (*motionController)["DeviceIo"];
    result.reserve(deviceIo.size());
    for (auto it = deviceIo.begin(); it != deviceIo.end(); ++it) {
        if (!it.value().is_object())
            continue;

        const ordered_json &io = it.value();
        MotionIoPointConfig cfg;
        cfg.name       = QString::fromStdString(it.key());
        cfg.port       = orderedJsonToInt(io.contains("port") ? io["port"] : ordered_json{}, 0);
        cfg.isOutput   = orderedJsonToInt(io.contains("type") ? io["type"] : ordered_json{}, 0) == 1;
        cfg.highActive = orderedJsonToInt(io.contains("logic") ? io["logic"] : ordered_json{}, 0) == 1;
        result.append(cfg);
    }

    return result;
}

QStringList parseMotionCardNames(const ordered_json &root, const char *logContext)
{
    QStringList result;

    const ordered_json *motionController = motionControllerSection(root, logContext);
    if (!motionController || !motionController->contains("Card")
        || !(*motionController)["Card"].is_object()) {
        qWarning() << logContext << "Card is empty or missing.";
        return result;
    }

    const ordered_json &cards = (*motionController)["Card"];
    result.reserve(static_cast<int>(cards.size()));
    for (auto it = cards.begin(); it != cards.end(); ++it)
        result.append(QString::fromStdString(it.key()));

    return result;
}

} // namespace

std::shared_ptr<common::device::MotionController> motionControllerOrNull()
{
    auto *app = Container::Application::instance();
    if (!app || !app->bound<common::device::DeviceManager>())
        return nullptr;
    const auto dm = app->make<common::device::DeviceManager>();
    return dm ? dm->MotionHandler() : nullptr;
}

bool writeIo(const QString &ioName, bool value, const char *logContext)
{
    const auto motion = motionControllerOrNull();
    if (!motion) {
        qWarning() << logContext << "MotionController unavailable, io=" << ioName;
        return false;
    }

    try {
        const auto err = motion->WriteIO(ioName.toStdString(), value);
        if (err) {
            qWarning() << logContext << "WriteIO failed, io=" << ioName
                       << ", err=" << QString::fromStdString(err->Message());
            return false;
        }
    } catch (const std::exception &e) {
        qWarning() << logContext << "WriteIO exception, io=" << ioName << ":" << e.what();
        return false;
    }
    return true;
}

void writeIoCombineOutputs(const QVector<IoCombineOutput> &outputs, const char *logContext)
{
    for (const IoCombineOutput &output : outputs)
        writeIo(output.name, output.set, logContext);
}

QHash<QString, bool> queryIoStates(const std::shared_ptr<common::device::MotionController> &motion,
                                   const QStringList                  &cardNames,
                                   const QVector<MotionIoPointConfig> &configs,
                                   bool                                readDoPorts)
{
    QHash<QString, bool> states;
    if (!motion || cardNames.isEmpty() || configs.isEmpty())
        return states;

    for (const QString &cardKey : cardNames) {
        std::vector<uint32_t> vecValue;
        const auto            err = readDoPorts
                                        ? motion->ReadDoPorts(cardKey.toStdString(), vecValue)
                                        : motion->ReadDiPorts(cardKey.toStdString(), vecValue);
        if (err)
            continue;

        const QHash<QString, bool> partial = mapPortBitmapToIoStates(vecValue, configs);
        for (auto it = partial.constBegin(); it != partial.constEnd(); ++it)
            states.insert(it.key(), it.value());
    }
    return states;
}

QVector<MotionIoPointConfig> filterIoConfigsByDirection(const QVector<MotionIoPointConfig> &all,
                                                        bool                                  isOutput)
{
    QVector<MotionIoPointConfig> filtered;
    filtered.reserve(all.size());
    for (const MotionIoPointConfig &cfg : all) {
        if (cfg.isOutput == isOutput)
            filtered.append(cfg);
    }
    return filtered;
}

void MotionDeviceConfigReader::partitionByDirection(const QVector<MotionIoPointConfig> &all,
                                                    QVector<MotionIoPointConfig>       *inputs,
                                                    QVector<MotionIoPointConfig>       *outputs)
{
    if (!inputs || !outputs)
        return;
    *inputs  = filterIoConfigsByDirection(all, false);
    *outputs = filterIoConfigsByDirection(all, true);
}

MotionDeviceConfigReader::Snapshot MotionDeviceConfigReader::loadSnapshot()
{
    Snapshot snapshot;

    const ordered_json root =
        loadOrderedJsonFile(deviceConfigFilePath(), "[MotionDeviceConfigReader]");
    if (!root.is_object())
        return snapshot;

    constexpr const char *kLogContext = "[MotionDeviceConfigReader]";
    snapshot.deviceIo        = parseDeviceIo(root, kLogContext);
    snapshot.motionCardNames = parseMotionCardNames(root, kLogContext);
    return snapshot;
}

QVector<MotionIoPointConfig> MotionDeviceConfigReader::loadDeviceIo()
{
    return loadSnapshot().deviceIo;
}

QStringList MotionDeviceConfigReader::loadMotionCardNames()
{
    return loadSnapshot().motionCardNames;
}

bool MotionDeviceConfigReader::updateDeviceIoBatch(
    const QHash<QString, QPair<int, int>> &portLogicByName)
{
    if (portLogicByName.isEmpty())
        return true;

    const QString path = deviceConfigFilePath();
    ordered_json root = loadOrderedJsonFile(path, "[MotionDeviceConfigReader]");
    if (!root.is_object())
        return false;

    ordered_json *motionController = nullptr;
    if (root.contains("DeviceManager") && root["DeviceManager"].is_object()
        && root["DeviceManager"].contains("MotionController")
        && root["DeviceManager"]["MotionController"].is_object()) {
        motionController = &root["DeviceManager"]["MotionController"];
    }
    if (!motionController || !motionController->contains("DeviceIo")
        || !(*motionController)["DeviceIo"].is_object()) {
        qWarning() << "[MotionDeviceConfigReader] DeviceIo is empty or missing.";
        return false;
    }

    ordered_json &deviceIo = (*motionController)["DeviceIo"];
    for (auto it = portLogicByName.constBegin(); it != portLogicByName.constEnd(); ++it) {
        const std::string key = it.key().toStdString();
        if (!deviceIo.contains(key)) {
            qWarning() << "[MotionDeviceConfigReader] DeviceIo entry not found:" << it.key();
            continue;
        }
        // 就地更新已有键，保留 DeviceIo 条目内字段顺序及整文件键序（ordered_map）。
        deviceIo[key]["port"]  = it.value().first;
        deviceIo[key]["logic"] = it.value().second;
    }

    QFile out(path);
    if (!out.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "[MotionDeviceConfigReader] Cannot open for write:" << path;
        return false;
    }
    const std::string dumped = root.dump(4) + '\n';
    out.write(dumped.data(), static_cast<qint64>(dumped.size()));
    return true;
}

QVector<IoCombineGroup> loadIoCombineConfig()
{
    QVector<IoCombineGroup> result;

    const ordered_json root = loadOrderedJsonFile(ioCombineConfigPath(), "[IoCombineConfig]");
    if (!root.is_array()) {
        if (!root.empty())
            qWarning() << "[IoCombineConfig] Root must be a JSON array.";
        return result;
    }

    result.reserve(static_cast<int>(root.size()));

    for (const ordered_json &groupVal : root) {
        if (!groupVal.is_object())
            continue;

        IoCombineGroup group;
        if (groupVal.contains("group") && groupVal["group"].is_string())
            group.group = QString::fromStdString(groupVal["group"].get<std::string>());
        if (group.group.isEmpty())
            continue;

        if (!groupVal.contains("actions") || !groupVal["actions"].is_array())
            continue;

        const ordered_json &actions = groupVal["actions"];
        group.actions.reserve(static_cast<int>(actions.size()));

        for (const ordered_json &actionVal : actions) {
            if (!actionVal.is_object())
                continue;

            IoCombineAction action;
            if (actionVal.contains("name") && actionVal["name"].is_string())
                action.name = QString::fromStdString(actionVal["name"].get<std::string>());
            if (action.name.isEmpty())
                continue;

            if (!actionVal.contains("outputs") || !actionVal["outputs"].is_array())
                continue;

            const ordered_json &outputs = actionVal["outputs"];
            action.outputs.reserve(static_cast<int>(outputs.size()));

            for (const ordered_json &outVal : outputs) {
                if (!outVal.is_object())
                    continue;

                IoCombineOutput output;
                if (outVal.contains("name") && outVal["name"].is_string())
                    output.name = QString::fromStdString(outVal["name"].get<std::string>());
                if (output.name.isEmpty())
                    continue;

                output.set = orderedJsonToBool(outVal.contains("set") ? outVal["set"] : ordered_json{});
                action.outputs.append(output);
            }

            if (!action.outputs.isEmpty())
                group.actions.append(action);
        }

        if (!group.actions.isEmpty())
            result.append(group);
    }

    return result;
}

} // namespace motion
