#pragma once

/**
 * @file inspection_history_coordinator.h
 * @brief 最近一次检测历史行 id（供复判确认 UPDATE 定位）
 */

#include <QtGlobal>

namespace aoi::inspection {

class InspectionHistoryCoordinator
{
public:
    static InspectionHistoryCoordinator &instance();

    void   setLastAppendedRecordId(qint64 id);
    qint64 lastAppendedRecordId() const { return m_lastRecordId; }
    void   clearLastAppendedRecordId() { m_lastRecordId = -1; }

private:
    InspectionHistoryCoordinator() = default;

    qint64 m_lastRecordId = -1;
};

} // namespace aoi::inspection
