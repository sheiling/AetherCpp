#pragma once

#include <QString>
#include <QVector>
#include <functional>

namespace mes {

/** MES 接口调用结果 */
struct MesResult
{
    enum class Code {
        Success,   ///< 业务成功（state=true）
        Skipped,   ///< MES 未启用，调用被跳过（视为成功）
        Transport, ///< HTTP 传输失败或超时
        Protocol,  ///< 响应格式不符合约定
        Business   ///< 业务失败（state=false）
    };

    Code    code    = Code::Transport;
    QString message;
    QString rawBody;

    bool ok() const { return code == Code::Success || code == Code::Skipped; }
    bool isSkipped() const { return code == Code::Skipped; }

    static MesResult success(const QString &msg = {})
    {
        return {Code::Success, msg, {}};
    }

    static MesResult makeSkipped()
    {
        return {Code::Skipped, QStringLiteral("MES 未启用"), {}};
    }

    static MesResult failure(Code code, const QString &msg, const QString &raw = {})
    {
        return {code, msg, raw};
    }
};

/** 缺陷点（检测结果上报） */
struct MesDefectPoint
{
    int     x           = 0;
    int     y           = 0;
    QString color;
    QString type;
    QString description;
    QString thread;
};

/** 设备状态上报参数 */
struct MesEquipmentStatus
{
    QString upperLevelCode = QStringLiteral("SJX003");
    QString currentCode    = QStringLiteral("SJX003");
};

/** 检测结果上报参数 */
struct MesDetectionReport
{
    QString              sn;
    QString              failReason;
    QVector<MesDefectPoint> defects;
};

/** 电气/阈值配置上报参数 */
struct MesElectricalPara
{
    QString productName;
    int     ledRedDeadLimit     = 0;
    int     ledGreenDeadLimit   = 0;
    int     ledBlueDeadLimit    = 0;
    int     ledRedDarkMaxLimit  = 0;
    int     ledGreenDarkMaxLimit = 0;
    int     ledBlueDarkMaxLimit = 0;
    int     blackScreenLimit    = 0;
    int     powerOutagesLimit   = 0;
    int     nonlinearityLimit   = 0;
};

/** MES 请求/响应日志回调 */
using MesLogCallback = std::function<void(const QString &title, const QString &content)>;

/** 兆驰 MES 接口路径 */
namespace api {

inline constexpr const char *kProductionCheck       = "/api/system/msa/equipment/productioncheck";
inline constexpr const char *kProductionNodeComplete = "/api/system/msa/equipment/productionnodecomplete";
inline constexpr const char *kUpEquipmentLogInfo     = "/api/system/msa/equipment/upequipmentloginfo";
inline constexpr const char *kUploadElectricalPara   = "/api/system/msa/equipment/UploadElectricalPara";

} // namespace api

} // namespace mes
