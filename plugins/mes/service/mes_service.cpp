#include "mes_service.h"

#include <utility/string_tool/time_format.hpp>

#include <random>

namespace mes {
namespace {

std::string toUtf8Std(const QString &text)
{
    return text.toUtf8().toStdString();
}

QString makeGuid()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<long long> dis(100000000000LL, 999999999999LL);
    return QString::fromStdString(common::utility::TimeFormat::GetCurrentTimeStr())
         + QString::number(dis(gen));
}

QString currentLocalTime()
{
    return QString::fromStdString(
        common::utility::TimeFormat::getCurrentTime("%Y-%m-%d %H:%M:%S"));
}

QString equipmentStatusMessage(const QString &statusCode)
{
    if (statusCode == QStringLiteral("SJX001"))
        return QStringLiteral("设备正常运行中");
    if (statusCode == QStringLiteral("SJX003"))
        return QStringLiteral("设备空闲中");
    if (statusCode == QStringLiteral("SJX005"))
        return QStringLiteral("设备报警");
    return {};
}

QString dieResultCode(const QString &failReason)
{
    if (failReason == QStringLiteral("大面积灭点"))
        return QStringLiteral("A1");
    if (failReason == QStringLiteral("线性不良"))
        return QStringLiteral("F");
    return QStringLiteral("T");
}

} // namespace

MesService::MesService(MesConfig config)
    : m_transport(std::move(config))
{
}

void MesService::setConfig(const MesConfig &config)
{
    m_transport.setConfig(config);
}

const MesConfig &MesService::config() const
{
    return m_transport.config();
}

void MesService::setLogCallback(MesLogCallback callback)
{
    m_transport.setLogCallback(std::move(callback));
}

nlohmann::json MesService::baseIdentityPayload() const
{
    const MesConfig &cfg = m_transport.config();
    return {
        {"UseCode", toUtf8Std(cfg.useCode)},
        {"VerType", toUtf8Std(cfg.verType)},
        {"VersionNumber", toUtf8Std(cfg.verNumber)},
        {"FactoryEquNum", toUtf8Std(cfg.equipmentCode)},
        {"OperatorId", toUtf8Std(cfg.operatorId)},
    };
}

MesResult MesService::productionCheck(const QString &sn) const
{
    auto body        = baseIdentityPayload();
    body["LotSN"]    = toUtf8Std(sn);
    return m_transport.post(api::kProductionCheck, body,
                            QStringLiteral("生产检查"));
}

MesResult MesService::productionNodeComplete(const QString &sn, bool isOk) const
{
    auto body     = baseIdentityPayload();
    body["LotSN"] = toUtf8Std(sn);
    body["IsOK"]  = isOk ? "1" : "0";
    return m_transport.post(api::kProductionNodeComplete, body,
                            QStringLiteral("生产节点工作完成"));
}

MesResult MesService::reportEquipmentStatus(const MesEquipmentStatus &status) const
{
    const MesConfig &cfg = m_transport.config();
    const QString guid   = makeGuid();
    const QString local  = currentLocalTime();

    nlohmann::json body = {
        {"GUID", toUtf8Std(guid)},
        {"LocalTime", toUtf8Std(local)},
        {"FactoryEquNum", toUtf8Std(cfg.equipmentCode)},
        {"EquName", toUtf8Std(cfg.equipmentName)},
        {"UpperLevelStatusCode", toUtf8Std(status.upperLevelCode)},
        {"OperationType", toUtf8Std(status.currentCode)},
        {"CurrentStatusCode", toUtf8Std(status.currentCode)},
        {"Operator", toUtf8Std(cfg.operatorName)},
        {"UseCode", toUtf8Std(cfg.useCode)},
        {"VerType", toUtf8Std(cfg.verType)},
        {"VersionNumber", toUtf8Std(cfg.verNumber)},
    };

    const QString message = equipmentStatusMessage(status.currentCode);
    if (!message.isEmpty())
        body["Message"] = toUtf8Std(message);

    return m_transport.post(api::kUpEquipmentLogInfo, body,
                            QStringLiteral("设备状态上报"));
}

nlohmann::json MesService::buildFlowInfos(const QString &sn,
                                          const QString &guid,
                                          const QString &localTime,
                                          const QVector<MesDefectPoint> &defects,
                                          const QString &failReason) const
{
    nlohmann::json flowInfos = nlohmann::json::array();
    const QString dieResult  = dieResultCode(failReason);

    for (int i = 0; i < defects.size(); ++i) {
        const MesDefectPoint &point = defects.at(i);
        flowInfos.push_back({
            {"No", std::to_string(i + 1)},
            {"ProductCode", toUtf8Std(sn)},
            {"OperationType", "XJX002"},
            {"X_Index", std::to_string(point.x)},
            {"Y_Index", std::to_string(point.y)},
            {"RGB", toUtf8Std(point.color)},
            {"Die_Result", toUtf8Std(dieResult)},
            {"Remark", toUtf8Std(point.type)},
            {"CheckNum", "1"},
            {"GUID", toUtf8Std(guid)},
            {"LocalTime", toUtf8Std(localTime)},
        });
    }
    return flowInfos;
}

MesResult MesService::reportDetectionInfo(const MesDetectionReport &report) const
{
    const MesConfig &cfg = m_transport.config();
    const bool isOk      = report.defects.isEmpty();
    const QString guid   = makeGuid();
    const QString local  = currentLocalTime();

    nlohmann::json body = {
        {"GUID", toUtf8Std(guid)},
        {"LocalTime", toUtf8Std(local)},
        {"FactoryEquNum", toUtf8Std(cfg.equipmentCode)},
        {"EquName", toUtf8Std(cfg.equipmentName)},
        {"ProductCode", toUtf8Std(report.sn)},
        {"OperationType", isOk ? "XJX001" : "XJX002"},
        {"Operator", toUtf8Std(cfg.operatorName)},
        {"Message", isOk ? "检测结果OK" : "检测结果NG"},
        {"UseCode", toUtf8Std(cfg.useCode)},
        {"VerType", toUtf8Std(cfg.verType)},
        {"VersionNumber", toUtf8Std(cfg.verNumber)},
        {"flowInfos", buildFlowInfos(report.sn, guid, local, report.defects, report.failReason)},
    };

    return m_transport.post(api::kUpEquipmentLogInfo, body,
                            QStringLiteral("设备生产信息上传"));
}

MesResult MesService::uploadElectricalPara(const MesElectricalPara &para) const
{
    const MesConfig &cfg = m_transport.config();

    const nlohmann::json body = {
        {"FactoryEQNum", toUtf8Std(cfg.equipmentCode)},
        {"Spacing", toUtf8Std(para.productName)},
        {"LEDRDeadLimit", std::to_string(para.ledRedDeadLimit)},
        {"LEDGDeadLimit", std::to_string(para.ledGreenDeadLimit)},
        {"LEDBDeadLimit", std::to_string(para.ledBlueDeadLimit)},
        {"LEDRDarkMinLimit", std::to_string(para.ledRedDeadLimit)},
        {"LEDGDarkMinLimit", std::to_string(para.ledGreenDeadLimit)},
        {"LEDBDarkMinLimit", std::to_string(para.ledBlueDeadLimit)},
        {"LEDRDarkMaxLimit", std::to_string(para.ledRedDarkMaxLimit)},
        {"LEDGDarkMaxLimit", std::to_string(para.ledGreenDarkMaxLimit)},
        {"LEDBDarkMaxLimit", std::to_string(para.ledBlueDarkMaxLimit)},
        {"BlackScreenLimit", std::to_string(para.blackScreenLimit)},
        {"PowerOutagesLimit", std::to_string(para.powerOutagesLimit)},
        {"NonlinearityLimit", std::to_string(para.nonlinearityLimit)},
    };

    return m_transport.post(api::kUploadElectricalPara, body,
                            QStringLiteral("设备配置数据上传"));
}

} // namespace mes
