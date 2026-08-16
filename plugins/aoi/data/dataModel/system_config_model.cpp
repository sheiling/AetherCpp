#include "system_config_model.h"

#include "data/data.h"

#include <common/utility/json_tool/json_utils.h>

#include <QFile>

#include <vector>

namespace aoi {

namespace {

bool loadConfigFromPath(const QString &path, AoiSystemConfig &out)
{
    json root;
    if (!common::utility::json_parseFromFileEx(
            root, path.toStdString(),
            common::utility::JsonParseFileOptions{true, true}))
        return false;
    if (!root.is_object() || root.empty())
        return false;
    out.reflectFromJson(root);
    return true;
}

} // namespace

void DefectSettings::ensureDefaults()
{
    for (int i = 0; i < inspection::kInspectionDefectTypeCount; ++i) {
        const auto type = static_cast<inspection::InspectionDefectType>(i);
        if (!items.contains(type)) {
            DefectItemSetting setting;
            setting.displayName = inspection::inspectionDefectTypeDisplayKey(type);
            items.insert(type, setting);
        } else if (items[type].displayName.isEmpty()) {
            items[type].displayName = inspection::inspectionDefectTypeDisplayKey(type);
        }
    }
}

DefectItemSetting &DefectSettings::item(inspection::InspectionDefectType type)
{
    if (!items.contains(type)) {
        DefectItemSetting setting;
        setting.displayName = inspection::inspectionDefectTypeDisplayKey(type);
        items.insert(type, setting);
    } else if (items[type].displayName.isEmpty()) {
        items[type].displayName = inspection::inspectionDefectTypeDisplayKey(type);
    }
    return items[type];
}

const DefectItemSetting &DefectSettings::item(inspection::InspectionDefectType type) const
{
    static const DefectItemSetting kDefault{};
    return items.contains(type) ? items.constFind(type).value() : kDefault;
}

void DefectSettings::reflectFromJson(const json &j)
{
    items.clear();
    if (j.is_object()) {
        for (auto it = j.begin(); it != j.end(); ++it) {
            const QString key = QString::fromStdString(it.key());
            if (key == QLatin1String(inspection::kDefectSettingsTotalStorageKey)) {
                if (it.value().is_object())
                    total.reflectFromJson(it.value());
                continue;
            }
            inspection::InspectionDefectType type;
            if (!inspection::inspectionDefectTypeFromStorageKey(key, &type) || !it.value().is_object())
                continue;
            DefectItemSetting setting;
            setting.reflectFromJson(it.value());
            items.insert(type, setting);
        }
    }
    ensureDefaults();
}

void DefectSettings::reflectToJson(json &j) const
{
    j = json::object();
    for (auto it = items.constBegin(); it != items.constEnd(); ++it) {
        json itemJson;
        it.value().reflectToJson(itemJson);
        j[inspection::inspectionDefectTypeStorageKey(it.key()).toStdString()] = itemJson;
    }
    json totalJson;
    total.reflectToJson(totalJson);
    j[inspection::kDefectSettingsTotalStorageKey] = totalJson;
}

bool DefectSettings::operator==(const DefectSettings &o) const
{
    return items == o.items && total == o.total;
}

AoiSystemConfig AoiSystemConfig::defaultDemo()
{
    AoiSystemConfig cfg;
    cfg.general.scanEnabled     = true;
    cfg.general.reviewEnabled   = true;
    cfg.common.trackMode        = TrackMode::DualTrack;
    cfg.common.captureName      = QStringLiteral("Camera0");
    cfg.common.axisXName        = QStringLiteral("AxisX");
    cfg.common.axisYName        = QStringLiteral("AxisY");
    cfg.common.axisZName        = QStringLiteral("AxisZ");
    cfg.common.motionSpeed      = 50.0;
    cfg.common.motionTimeoutMs  = 30000;
    cfg.iface.upstreamEnabled   = true;
    cfg.iface.downstreamEnabled = false;

    cfg.defects.ensureDefaults();
    cfg.defects.total.lower = 0;
    cfg.defects.total.upper = 5;

    cfg.paths.imagePath      = QStringLiteral(R"(D:\IET\images)");
    cfg.paths.resultPath     = QStringLiteral(R"(D:\IET\results)");
    cfg.paths.thirdPartyPath = QStringLiteral(R"(\\192.168.1.10\thirdparty)");
    return cfg;
}

QString SystemConfigRepository::configFilePath()
{
    return data::systemConfigFilePath();
}

AoiSystemConfig SystemConfigRepository::load()
{
    AoiSystemConfig cfg;
    const QString   path = data::systemConfigFilePath();
    if (QFile::exists(path) && loadConfigFromPath(path, cfg))
        return cfg;
    return AoiSystemConfig::defaultDemo();
}

bool SystemConfigRepository::save(const AoiSystemConfig &cfg)
{
    json root;
    cfg.reflectToJson(root);
    return common::utility::json_saveToFileEx(
        root, data::systemConfigFilePath().toStdString(),
        common::utility::JsonSaveFileOptions{true});
}

} // namespace aoi

namespace aoi::parammeta {

const ParamTypeMeta *ParamTypeMetaProvider<DefectSettings>::get()
{
    static std::vector<QByteArray> keyStorage;
    static QVector<ParamFieldMeta> fields;
    static ParamTypeMeta           meta;
    static bool                    built = false;

    if (!built) {
        keyStorage.reserve(inspection::kInspectionDefectTypeCount + 1);
        fields.reserve(inspection::kInspectionDefectTypeCount + 1);

        const ParamTypeMeta *itemMeta = ParamTypeMetaProvider<DefectItemSetting>::get();
        for (int i = 0; i < inspection::kInspectionDefectTypeCount; ++i) {
            const auto type = static_cast<inspection::InspectionDefectType>(i);
            keyStorage.push_back(inspection::inspectionDefectTypeStorageKey(type).toUtf8());
            const char *key = keyStorage.back().constData();
            fields.append({key, key, ParamValueKind::Object, itemMeta});
        }

        keyStorage.emplace_back(inspection::kDefectSettingsTotalStorageKey);
        const char *totalKey = keyStorage.back().constData();
        fields.append({totalKey, totalKey, ParamValueKind::Object,
                       ParamTypeMetaProvider<TotalDefectThreshold>::get()});

        meta  = ParamTypeMeta{"aoi::DefectSettings", "defects", fields};
        built = true;
    }
    return &meta;
}

} // namespace aoi::parammeta
