#pragma once

#include "mes_config_model.h"
#include "mes_http_transport.h"
#include "mes_types.h"

namespace mes {

/**
 * @brief 兆驰点亮 MES 业务服务
 *
 * 在 MesHttpTransport 之上提供语义化接口，调用方只需准备参数结构体。
 */
class MesService
{
public:
    explicit MesService(MesConfig config = {});

    void setConfig(const MesConfig &config);
    const MesConfig &config() const;

    void setLogCallback(MesLogCallback callback);

    /** 生产检查（过站校验） */
    MesResult productionCheck(const QString &sn) const;

    /** 生产节点完成 */
    MesResult productionNodeComplete(const QString &sn, bool isOk) const;

    /** 设备状态上报 */
    MesResult reportEquipmentStatus(const MesEquipmentStatus &status) const;

    /** 检测结果上报（含缺陷点 flowInfos） */
    MesResult reportDetectionInfo(const MesDetectionReport &report) const;

    /** 电气/阈值配置上报 */
    MesResult uploadElectricalPara(const MesElectricalPara &para) const;

private:
    nlohmann::json baseIdentityPayload() const;
    nlohmann::json buildFlowInfos(const QString &sn,
                                  const QString &guid,
                                  const QString &localTime,
                                  const QVector<MesDefectPoint> &defects,
                                  const QString &failReason) const;

    MesHttpTransport m_transport;
};

} // namespace mes
