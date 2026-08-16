/**
 * @file inspection_history_coordinator.cpp
 */

#include "inspection_history_coordinator.h"

namespace aoi::inspection {

InspectionHistoryCoordinator &InspectionHistoryCoordinator::instance()
{
    static InspectionHistoryCoordinator coord;
    return coord;
}

void InspectionHistoryCoordinator::setLastAppendedRecordId(qint64 id)
{
    m_lastRecordId = id;
}

} // namespace aoi::inspection
