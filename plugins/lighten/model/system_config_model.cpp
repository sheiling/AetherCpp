/**
 * @file system_config_model.cpp
 * @brief SystemConfig 默认值、JSON 读写与仓库封装实现
 */

#include "system_config_model.h"

#include "lighten_param_persist.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>

/** @brief SystemConfig 运算符、默认值与 JSON 持久化实现 */
namespace lighten {

bool GeneralSwitches::operator==(const GeneralSwitches &o) const
{
    return scanBarcode == o.scanBarcode && saveRawImage == o.saveRawImage
           && saveResultImage == o.saveResultImage && saveAlgoProcessData == o.saveAlgoProcessData
           && algoFailAlarm == o.algoFailAlarm;
}

bool InterfaceSwitches::operator==(const InterfaceSwitches &o) const
{
    return upstreamEnabled == o.upstreamEnabled && downstreamEnabled == o.downstreamEnabled;
}

bool DetectItemColorSwitches::operator==(const DetectItemColorSwitches &o) const
{
    return item == o.item && redEnabled == o.redEnabled && greenEnabled == o.greenEnabled
           && blueEnabled == o.blueEnabled && masterEnabled == o.masterEnabled
           && hasRgbColumns == o.hasRgbColumns;
}

bool DetectItemColorSwitches::showsHomeProgress() const
{
    if (hasRgbColumns)
        return redEnabled || greenEnabled || blueEnabled;
    return masterEnabled;
}

bool DetectItemColorSwitches::showsHomeProgressSlot(int colorIdx) const
{
    if (!hasRgbColumns)
        return masterEnabled;
    switch (colorIdx) {
    case 0: return redEnabled;
    case 1: return greenEnabled;
    case 2: return blueEnabled;
    default: return false;
    }
}

bool GeneralSettings::operator==(const GeneralSettings &o) const
{
    return imageStoragePath == o.imageStoragePath && resultStoragePath == o.resultStoragePath
           && thirdPartySoftwarePath == o.thirdPartySoftwarePath
           && lightDelayMs == o.lightDelayMs && switchDelayMs == o.switchDelayMs
           && maxUseCount == o.maxUseCount && currentUseCount == o.currentUseCount;
}

bool SystemConfig::operator==(const SystemConfig &o) const
{
    return generalSwitches == o.generalSwitches && interfaceSwitches == o.interfaceSwitches
           && detectItems == o.detectItems && generalSettings == o.generalSettings
           && thresholdRanges == o.thresholdRanges;
}

SystemConfig SystemConfig::defaultDemo()
{
    SystemConfig cfg;
    cfg.generalSettings.imageStoragePath =
        QStringLiteral(R"(D:\ET\images)");
    cfg.generalSettings.resultStoragePath =
        QStringLiteral(R"(D:\ET\results)");
    cfg.generalSettings.thirdPartySoftwarePath =
        QStringLiteral(R"(\\192.168.1.10\thirdparty)");

    cfg.detectItems.resize(kDetectItemCount);
    for (int i = 0; i < kDetectItemCount; ++i) {
        cfg.detectItems[i].item           = static_cast<DetectItemKind>(i);
        cfg.detectItems[i].redEnabled     = true;
        cfg.detectItems[i].greenEnabled   = true;
        cfg.detectItems[i].blueEnabled    = true;
        cfg.detectItems[i].masterEnabled  = true;
        cfg.detectItems[i].hasRgbColumns  =
            (i != static_cast<int>(DetectItemKind::BlackScreen)
             && i != static_cast<int>(DetectItemKind::BlackScreenOn)
             && i != static_cast<int>(DetectItemKind::Flash));
    }

    cfg.thresholdRanges = {
        {QStringLiteral("死灯数量"), 0, 3},
        {QStringLiteral("暗灯数量"), 0, 3},
        {QStringLiteral("亮灯数量"), 0, 2},
        {QStringLiteral("串色数量"), 0, 2},
        {QStringLiteral("串灯数量"), 0, 1},
        {QStringLiteral("混色数量"), 0, 1},
        {QStringLiteral("缺陷总数"), 0, 5},
    };

    return cfg;
}

namespace {

QString systemParamJsonPath()
{
    return config_dir::root() + QStringLiteral("/system_param.json");
}

SystemConfig &configSingleton()
{
    static SystemConfig cfg = SystemConfig::load();
    return cfg;
}

} // namespace

SystemConfig SystemConfig::load()
{
    const QString jsonFile = systemParamJsonPath();
    if (!QFile::exists(jsonFile))
        return defaultDemo();

    QFile file(jsonFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return defaultDemo();

    try {
        const json j = json::parse(file.readAll().toStdString());
        SystemConfig cfg;
        cfg.reflectFromJson(j);
        return cfg;
    } catch (const std::exception &) {
        return defaultDemo();
    }
}

bool SystemConfig::save(const SystemConfig &cfg)
{
    const QString jsonFile = systemParamJsonPath();
    try {
        const QFileInfo fileInfo(jsonFile);
        if (!QDir().mkpath(fileInfo.absolutePath()))
            return false;

        json j;
        cfg.reflectToJson(j);
        QFile file(jsonFile);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            return false;
        file.write(j.dump(4).c_str());
        return true;
    } catch (const std::exception &) {
        return false;
    }
}

SystemConfig SystemConfigRepository::load()
{
    return configSingleton();
}

bool SystemConfigRepository::save(const SystemConfig &cfg)
{
    configSingleton() = cfg;
    return SystemConfig::save(cfg);
}

} // namespace lighten
