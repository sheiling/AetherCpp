#pragma once

/**
 * @file home_model.h
 * @brief 首页运行时数据结构（KPI、硬件、进度、快照、日志）
 */

#include "lighten_types.h"

#include <QDateTime>
#include <QString>
#include <QVector>
#include <QMetaType>

namespace lighten {

/** 首页 KPI 条（10 项） */
struct KpiSnapshot
{
    qint64  totalPanels   = 0;     ///< 累计点亮板数（来自 lighten_production_kpi）
    qint64  okCount       = 0;     ///< OK 数量
    qint64  ngCount       = 0;     ///< NG 数量
    qint64  rwCount       = 0;     ///< RW 数量
    int     currentUseCount = 0;     ///< 对插次数：当前次数（来自 lighten_production_kpi.insertion_count）
    int     maxUseCount     = 100000; ///< 对插次数：新件使用寿命次数（来自系统配置）
    double  passRate      = 0.0;     ///< 百分比 0–100
    double  cycleTimeSec  = 4.2;     ///< 节拍时间（秒）
    RunMode runMode       = RunMode::Auto;       ///< 运行模式
    DeviceState deviceState = DeviceState::Run;  ///< 设备状态
    QString currentRecipe; ///< 当前配方 ID

    /** 从系统配置同步对插次数（当前/寿命上限） */
    void syncUseCountFromConfig();
    /** 从系统配置仅同步对插次数上限（不覆盖 currentUseCount） */
    void syncMaxUseCountFromConfig();
    /** 从默认配方同步「当前配方」显示值 */
    void syncCurrentRecipeFromDefault();

    bool operator==(const KpiSnapshot &o) const
    {
        return totalPanels == o.totalPanels && okCount == o.okCount && ngCount == o.ngCount
               && rwCount == o.rwCount && currentUseCount == o.currentUseCount
               && maxUseCount == o.maxUseCount && qFuzzyCompare(passRate, o.passRate)
               && qFuzzyCompare(cycleTimeSec, o.cycleTimeSec) && runMode == o.runMode
               && deviceState == o.deviceState && currentRecipe == o.currentRecipe;
    }
    bool operator!=(const KpiSnapshot &o) const { return !(*this == o); }
};

/** 硬件连接一行 */
struct HardwareStatusRow
{
    QString           deviceName;  ///< 设备名称
    HardwareLinkState linkState = HardwareLinkState::Connected; ///< 连接状态

    bool operator==(const HardwareStatusRow &o) const
    {
        return deviceName == o.deviceName && linkState == o.linkState;
    }
    bool operator!=(const HardwareStatusRow &o) const { return !(*this == o); }
};

/** 检测项进度（首页最多 21 项：18 RGB + 3 无分色） */
struct DetectProgressRow
{
    QString statusText; ///< 已完成 / 进行中 / 未开始

    bool operator==(const DetectProgressRow &o) const
    {
        return statusText == o.statusText;
    }
    bool operator!=(const DetectProgressRow &o) const { return !(*this == o); }
};

/** 当前板结果快照表一行 */
struct ResultSnapshotRow
{
    DetectItemKind item  = DetectItemKind::DeadLed; ///< 检测项类型
    int            r     = -1; ///< <0 表示 UI 显示 "-"
    int            g     = -1;
    int            b     = -1;
    int            total = -1;
    InspectionResult result = InspectionResult::Ng; ///< 该项判定结果

    bool operator==(const ResultSnapshotRow &o) const
    {
        return item == o.item && r == o.r && g == o.g && b == o.b && total == o.total
               && result == o.result;
    }
    bool operator!=(const ResultSnapshotRow &o) const { return !(*this == o); }
};

/** 当前检测快照（表头 SN + 总状态 + 各行） */
struct CurrentInspectionSnapshot
{
    QString                    serialNumber;  ///< 当前板 SN
    InspectionResult           overallResult = InspectionResult::Ok; ///< 总体判定
    QVector<ResultSnapshotRow> rows;          ///< 各检测项结果行

    /** 首页「当前检测结果快照」初始态：SN 为空、总状态 OK，各检测项按规则清零/置 OK */
    static CurrentInspectionSnapshot defaultEmpty();

    bool operator==(const CurrentInspectionSnapshot &o) const
    {
        return serialNumber == o.serialNumber && overallResult == o.overallResult
               && rows == o.rows;
    }
    bool operator!=(const CurrentInspectionSnapshot &o) const { return !(*this == o); }
};

/** 运行日志条目 */
struct RunLogEntry
{
    QDateTime timestamp; ///< 日志时间
    QString   level;   ///< INFO / WARN / ERROR
    QString   message; ///< 日志内容

    bool operator==(const RunLogEntry &o) const
    {
        return timestamp == o.timestamp && level == o.level && message == o.message;
    }
    bool operator!=(const RunLogEntry &o) const { return !(*this == o); }
};

/** 工位通过状态（进料 / 检测 / 出料） */
struct StationTrackState
{
    bool feedPassed    = true;  ///< 进料工位已通过
    bool inspectPassed = false; ///< 检测工位已通过
    bool outPassed     = false; ///< 出料工位已通过

    bool operator==(const StationTrackState &o) const
    {
        return feedPassed == o.feedPassed && inspectPassed == o.inspectPassed
               && outPassed == o.outPassed;
    }
    bool operator!=(const StationTrackState &o) const { return !(*this == o); }
};

/** 首页图像预览区 UI 状态 */
struct HomePreviewState
{
    ColorChannel colorChannel   = ColorChannel::Red;   ///< 预览颜色通道
    CameraIndex  camera         = CameraIndex::Main;  ///< 预览相机
    bool         showResultImage = false; ///< false=原图, true=结果图
    DetectItemKind defectFilter = DetectItemKind::DeadLed; ///< 缺陷筛选类型

    bool operator==(const HomePreviewState &o) const
    {
        return colorChannel == o.colorChannel && camera == o.camera
               && showResultImage == o.showResultImage && defectFilter == o.defectFilter;
    }
    bool operator!=(const HomePreviewState &o) const { return !(*this == o); }
};

/** 首页运行时聚合（Model 层，无 UI） */
struct HomeRuntimeState
{
    KpiSnapshot                kpi;       ///< KPI 条数据
    QVector<HardwareStatusRow> hardware;  ///< 硬件连接列表
    QVector<DetectProgressRow> progress;  ///< 检测项进度（21 项）
    CurrentInspectionSnapshot  snapshot; ///< 当前板检测结果快照
    QVector<RunLogEntry>       logs;      ///< 运行日志
    StationTrackState          stations;  ///< 工位通过状态
    HomePreviewState           preview;   ///< 图像预览区状态

    /** 返回首页演示用默认运行时状态 */
    static HomeRuntimeState defaultDemo();

    /** 首页 KPI 条标签（10 项） */
    static const QStringList &kpiLabels();
    /** 格式化对插次数显示：当前使用次数/新件使用寿命次数 */
    static QString formatInsertionCount(int currentUseCount, int maxUseCount);
    /** 将 KPI 数值格式化为界面显示文本 */
    QStringList kpiDisplayValues() const;
    /** 结果表单元格：负值显示 "-" */
    static QString formatResultCell(int value);
    /** 结果表「总计」列：黑屏检测 / Flash 等无数量项固定显示 "-" */
    static QString formatSnapshotTotal(DetectItemKind item, int total);
};

} // namespace lighten

Q_DECLARE_METATYPE(lighten::KpiSnapshot)
Q_DECLARE_METATYPE(lighten::HardwareStatusRow)
Q_DECLARE_METATYPE(lighten::DetectProgressRow)
Q_DECLARE_METATYPE(lighten::ResultSnapshotRow)
Q_DECLARE_METATYPE(lighten::CurrentInspectionSnapshot)
Q_DECLARE_METATYPE(lighten::RunLogEntry)
Q_DECLARE_METATYPE(lighten::StationTrackState)
Q_DECLARE_METATYPE(lighten::HomePreviewState)
Q_DECLARE_METATYPE(lighten::HomeRuntimeState)
