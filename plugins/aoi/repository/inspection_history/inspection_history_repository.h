#pragma once

/**
 * @file inspection_history_repository.h
 * @brief aoi_inspection_history 表 SQLite 访问（{exe}/data/record.db）
 */

#include "inspection_history_types.h"

#include "inspection_types.h"

#include <QString>

namespace aoi::inspection {

/**
 * @brief 检测历史仓储（进程内单例，独立 SQLite 连接名）
 */
class InspectionHistoryRepository
{
public:
    static InspectionHistoryRepository &instance();

    bool isReady() const;

    /** 检测完成追加；返回新行 id，失败 -1 */
    qint64 appendRecord(const DefectInfoList &list, QString *errorOut = nullptr);

    /** 复判确认后更新派生列与分区 JSON */
    bool updateRecord(qint64 id, const DefectInfoList &list, QString *errorOut = nullptr);

    bool updateRecordBySnTime(const QString &sn, const QString &detectTime, const DefectInfoList &list,
                              QString *errorOut = nullptr);

    InspectionHistoryPage queryPage(const InspectionHistoryQuery &query, QString *errorOut = nullptr) const;

    InspectionHistoryStats queryStats(const InspectionHistoryQuery &query, QString *errorOut = nullptr) const;

    bool exportToCsv(const InspectionHistoryQuery &query, const QString &filePath, QString *errorOut = nullptr) const;

private:
    InspectionHistoryRepository();
    ~InspectionHistoryRepository();
    InspectionHistoryRepository(const InspectionHistoryRepository &)            = delete;
    InspectionHistoryRepository &operator=(const InspectionHistoryRepository &) = delete;

    bool ensureConnection();
    bool ensureSchema();

    QString m_dbPath;
    QString m_connName;
    bool    m_schemaReady = false;
};

} // namespace aoi::inspection
