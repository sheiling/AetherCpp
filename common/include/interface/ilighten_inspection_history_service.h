#pragma once

/**
 * @file ilighten_inspection_history_service.h
 * @brief 检测历史写入服务接口（纯虚接口，无 QObject / Qt 插件 IID）
 *
 * Lighten 插件通过 LightenInspectionHistoryServiceImpl 实现此接口，并注册到 PluginManager 对象池。
 * 消费方通过 PluginManager::getInterface<ILightenInspectionHistoryService>() 获取实例，
 * 调用 appendRecord / queryRecords / upsertRecord。
 *
 * 包含方式：#include <interface/ilighten_inspection_history_service.h>
 */

#include <array>

#include <QtGlobal>
#include <QString>
#include <QVector>

/** 检测结果（与 lighten_inspection_history.inspection_result 一致：0=OK, 1=NG, 2=RW） */
enum class LightenInspectionResult
{
    Ok = 0,
    Ng = 1,
    Rw = 2,
};

/** 历史表 R/G/B 分色统计列数（6 类缺陷 × 3 色） */
constexpr int kLightenInspectionDefectRgbCount = 18;

/** 检测历史宽表一行（写入 API 数据载体，与 lighten_inspection_history schema 对齐） */
struct LightenInspectionHistoryRecord
{
    QString model;         ///< 产品型号
    QString serialNumber;  ///< 板卡序列号
    LightenInspectionResult result = LightenInspectionResult::Ok;
    int     inspectTimes           = 0;
    int     rwCount                = 0;
    int     inspectDurationMs      = 0;
    int     blackScreenLightCount  = 0;
    std::array<int, kLightenInspectionDefectRgbCount> defectCounts{};
    int     defectTypeMask = 0; ///< 缺陷类型位掩码
    int     flashResult    = 0; ///< 0=OK, 1=NG
    QString specialDetect;
    QString inspectedAt; ///< yyyy-MM-dd HH:mm:ss；可空，由实现自动补当前时间
};

inline bool operator==(const LightenInspectionHistoryRecord &a,
                       const LightenInspectionHistoryRecord &b)
{
    return a.model == b.model && a.serialNumber == b.serialNumber && a.result == b.result
           && a.inspectTimes == b.inspectTimes && a.rwCount == b.rwCount
           && a.inspectDurationMs == b.inspectDurationMs
           && a.blackScreenLightCount == b.blackScreenLightCount
           && a.defectCounts == b.defectCounts && a.defectTypeMask == b.defectTypeMask
           && a.flashResult == b.flashResult && a.specialDetect == b.specialDetect
           && a.inspectedAt == b.inspectedAt;
}

inline bool operator!=(const LightenInspectionHistoryRecord &a,
                       const LightenInspectionHistoryRecord &b)
{
    return !(a == b);
}

/** 跨插件历史查询条件（空字段表示不限） */
struct LightenInspectionHistoryQuery
{
    QString productModel; ///< 产品型号；空=不限
    QString boardSerial;  ///< 灯板编号；空=不限，非空时模糊匹配
};

/**
 * @brief 检测历史读写服务
 *
 * 数据读写 Lighten 插件进程内 DataStore 已打开的后端（SQLite / MySQL / Mock）。
 * 与 ILightenProductionKpiService 共用 lighten_inspection.db，职责分离。
 */
class ILightenInspectionHistoryService
{
public:
    virtual ~ILightenInspectionHistoryService() = default;

    /** DataStore 已初始化且连接可用 */
    virtual bool isReady() const = 0;

    /**
     * @brief 追加一条检测历史记录
     * @param record   model、serialNumber 必填
     * @param errorOut 失败时写入原因
     * @return 新行 id（>0）；失败返回 -1
     */
    virtual qint64 appendRecord(const LightenInspectionHistoryRecord &record,
                                QString *errorOut = nullptr) = 0;

    /**
     * @brief 按条件查询检测历史（按检测时间倒序）
     * @param query    筛选条件；各字段可空
     * @param errorOut 失败时写入原因
     */
    virtual QVector<LightenInspectionHistoryRecord> queryRecords(
        const LightenInspectionHistoryQuery &query = {},
        QString *errorOut = nullptr) const = 0;

    /**
     * @brief 按 product_model + board_sn 更新一条历史；不存在则插入
     * @param record   model、serialNumber 必填
     * @param errorOut 失败时写入原因
     * @return 行 id（>0）；失败返回 -1
     */
    virtual qint64 upsertRecord(const LightenInspectionHistoryRecord &record,
                                QString *errorOut = nullptr) = 0;
};
