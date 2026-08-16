#pragma once

/**
 * @file lighten_inspection_history_service_impl.h
 * @brief ILightenInspectionHistoryService 实现
 */

#include <interface/ilighten_inspection_history_service.h>

#include <QObject>

/**
 * @brief 检测历史写入服务实现，注册到 PluginManager 对象池
 */
class LightenInspectionHistoryServiceImpl final : public QObject, public ILightenInspectionHistoryService
{
public:
    LightenInspectionHistoryServiceImpl();
    ~LightenInspectionHistoryServiceImpl() override = default;

    bool isReady() const override;
    qint64 appendRecord(const LightenInspectionHistoryRecord &record,
                        QString *errorOut = nullptr) override;
    QVector<LightenInspectionHistoryRecord> queryRecords(
        const LightenInspectionHistoryQuery &query = {},
        QString *errorOut = nullptr) const override;
    qint64 upsertRecord(const LightenInspectionHistoryRecord &record,
                        QString *errorOut = nullptr) override;
};
