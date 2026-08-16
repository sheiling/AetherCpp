/**
 * @file defect_review_view_model.cpp
 * @brief 缺陷复判 ViewModel 实现
 */

#include "defect_review_view_model.h"

#include "defect_review_image_crop.h"
#include "flow_host/flow_host_coordinator.h"
#include "defect_info_persistence.h"
#include "inspection_history_coordinator.h"
#if COMMON_ORM_ENABLED
#include "repository/inspection_history/inspection_history_repository.h"
#endif

#include "uibase/languagemanager.h"

#include <QDebug>
#include <QKeyEvent>

#include <algorithm>
#include <optional>

namespace aoi::review {
namespace {

bool lampPassesTypeFilter(const DefectReviewLampRecord &rec, inspection::InspectionDefectTypeMask mask)
{
    if (mask == 0)
        return false;
    return ((rec.defectType | rec.rejudgeDefectType) & mask) != 0u;
}

bool lampIsOkReviewed(const DefectReviewLampRecord &rec)
{
    return rec.isRejudge && rec.rejudgeDefectType == 0;
}

bool lampIsAlgorithmOk(const DefectReviewLampRecord &rec)
{
    return rec.defectType == 0 && rec.rejudgeDefectType == 0;
}

bool lampPassesContentFilter(const DefectReviewLampRecord &rec,
                             inspection::InspectionDefectTypeMask typeMask,
                             bool showOk)
{
    if (lampPassesTypeFilter(rec, typeMask))
        return true;
    if (!showOk)
        return false;
    return lampIsOkReviewed(rec) || lampIsAlgorithmOk(rec);
}

int countEnabledDefectBits(inspection::InspectionDefectTypeMask mask)
{
    int count = 0;
    for (int i = 0; i < inspection::kInspectionDefectTypeCount; ++i) {
        const auto type = static_cast<inspection::InspectionDefectType>(i);
        if (inspection::inspectionMaskHasDefect(mask, type))
            ++count;
    }
    return count;
}

bool itemHasValidPanelDieIndex(const inspection::DefectInfoItem &item,
                               const DefectReviewDieGrid &grid)
{
    if (item.dieRowIndexInPanel < 0 || item.dieColIndexInPanel < 0)
        return false;
    if (!grid.isValid())
        return true;
    return item.dieRowIndexInPanel < grid.rowCount && item.dieColIndexInPanel < grid.colCount;
}

LampKey lampKeyFromItem(const inspection::DefectInfoItem &item)
{
    return LampKey{item.dieRowIndexInPanel, item.dieColIndexInPanel};
}

DefectDieChipInfo dieChipInfoFromItem(const inspection::DefectInfoItem &item)
{
    DefectDieChipInfo info;
    info.dieRowIndexInPanel = item.dieRowIndexInPanel;
    info.dieColIndexInPanel = item.dieColIndexInPanel;
    info.chipType           = item.chipType;
    return info;
}

QString maskToDefectNames(inspection::InspectionDefectTypeMask mask)
{
    QStringList parts;
    for (int i = 0; i < inspection::kInspectionDefectTypeCount; ++i) {
        const auto type = static_cast<inspection::InspectionDefectType>(i);
        if (inspection::inspectionMaskHasDefect(mask, type))
            parts.append(TR(inspection::inspectionDefectTypeDisplayKey(type)));
    }
    return parts.join(QStringLiteral("+"));
}

std::optional<DefectChipImageSource> selectedChipImageSource(
    const std::optional<LampKey> &selectedKey,
    const QHash<LampKey, DefectReviewLampRecord> &lampRecords,
    const inspection::DefectInfoList &workingCopy)
{
    if (!selectedKey.has_value() || !lampRecords.contains(*selectedKey))
        return std::nullopt;

    const DefectReviewLampRecord &rec = lampRecords[*selectedKey];
    if (rec.itemRefs.isEmpty())
        return std::nullopt;

    const DefectItemRef &ref = rec.itemRefs.first();
    DefectChipImageSource  source =
        resolveChipImageSource(workingCopy, ref.partitionIndex, ref.itemIndex);
    if (!source.isValid())
        return std::nullopt;
    return source;
}

} // namespace

DefectReviewViewModel::DefectReviewViewModel(bool bindBridge, QObject *parent)
    : ViewModel(parent)
    , m_bindBridge(bindBridge)
{
    qRegisterMetaType<aoi::review::LampKey>();
    if (m_bindBridge)
        bindActiveBridge();
}

void DefectReviewViewModel::setDieGrid(const DefectReviewDieGrid &grid)
{
    m_pendingDieGrid = grid;
}

void DefectReviewViewModel::bindActiveBridge()
{
    auto *bridge = flow_host::flowHostCoordinator().activeBridge();
    if (!bridge)
        return;

    /**
     * 新待复判会话到达时直接覆盖当前 VM 状态。
     * 这里不保留 dirty 保护，原因是复判页后续会作为外部页面唤起的模态业务窗口使用，
     * 每次展示都应反映当前 bridge 上送的待复判板数据。
     */
    connect(bridge, &flow::ScanFlowProgressBridge::reviewSessionReady, this,
            [this](const inspection::DefectInfoList &data) {
                loadReviewSession(data);
            },
            Qt::QueuedConnection);
}

void DefectReviewViewModel::loadReviewSession(const inspection::DefectInfoList &data)
{
    // 真实会话加载会同时重置 baseline / workingCopy，确保后续确认落盘以当前桥接数据为准。
    m_baseline    = data;
    m_workingCopy = data;
    if (m_pendingDieGrid.isValid()) {
        m_workingCopy.dieRowCount = m_pendingDieGrid.rowCount;
        m_workingCopy.dieColCount = m_pendingDieGrid.colCount;
        m_pendingDieGrid          = {};
    }
    m_validDefectType = data.validDefectType;
    // 默认等同侧栏「全选缺陷」：勾选全部已启用缺陷类型，不含 OK
    m_showOkFilter    = false;
    m_visibleTypeMask = m_validDefectType;
    m_selectedKey       = std::nullopt;
    m_dirty             = false;

    applyDieGridFromData();
    rebuildIndex();
    rebuildDieChipLayout();
    rebuildVisibleKeys();
    emit dirtyChanged(false);
    emit sessionChanged();
    emit selectionChanged();
}

void DefectReviewViewModel::applyDieGridFromData()
{
    m_dieGrid.rowCount = m_workingCopy.dieRowCount;
    m_dieGrid.colCount = m_workingCopy.dieColCount;
}

void DefectReviewViewModel::rebuildIndex()
{
    m_lampRecords.clear();

    for (int pi = 0; pi < m_workingCopy.defectPartitions.size(); ++pi) {
        const inspection::DefectInfoPartition &part = m_workingCopy.defectPartitions[pi];
        for (int ii = 0; ii < part.defectItems.size(); ++ii) {
            const inspection::DefectInfoItem &item = part.defectItems[ii];
            if (!itemHasValidPanelDieIndex(item, m_dieGrid)) {
                qWarning() << "[DefectReview] defect item missing valid dieRowIndexInPanel/"
                              "dieColIndexInPanel, skipped";
                continue;
            }

            const LampKey                     key = lampKeyFromItem(item);

            DefectReviewLampRecord &rec = m_lampRecords[key];
            if (rec.itemRefs.isEmpty()) {
                rec.key              = key;
                rec.groupRow         = item.rowIndexInPanel;
                rec.groupCol         = item.colIndexInPanel;
                rec.chipType         = item.chipType;
                rec.defectType       = item.defectType;
                rec.rejudgeDefectType = item.rejudgeDefectType;
                rec.isRejudge        = item.isRejudge;
            } else {
                rec.defectType |= item.defectType;
                if (rec.chipType != item.chipType)
                    qWarning() << "[DefectReview] chipType mismatch at die" << key.dieRow << key.dieCol;
            }

            rec.inDieGridBounds = m_dieGrid.isValid()
                                  && key.dieRow >= 0 && key.dieRow < m_dieGrid.rowCount
                                  && key.dieCol >= 0 && key.dieCol < m_dieGrid.colCount;

            rec.itemRefs.append(DefectItemRef{pi, ii});
        }
    }
}

void DefectReviewViewModel::rebuildDieChipLayout()
{
    m_dieChipLayout.clear();

    for (const inspection::DefectInfoPartition &part : m_workingCopy.defectPartitions) {
        for (const inspection::DefectInfoItem &item : part.defectItems) {
            if (!itemHasValidPanelDieIndex(item, m_dieGrid)) {
                qWarning() << "[DefectReview] defect item missing valid dieRowIndexInPanel/"
                              "dieColIndexInPanel, skipped in die layout";
                continue;
            }

            const LampKey         key  = lampKeyFromItem(item);
            const DefectDieChipInfo info = dieChipInfoFromItem(item);

            if (m_dieChipLayout.contains(key) && m_dieChipLayout[key].chipType != info.chipType) {
                qWarning() << "[DefectReview] die chipType mismatch at" << key.dieRow << key.dieCol;
            }
            m_dieChipLayout.insert(key, info);
        }
    }
}

void DefectReviewViewModel::rebuildVisibleKeys()
{
    m_visibleKeys.clear();
    if (!hasTypeChecked())
        return;

    for (auto it = m_lampRecords.constBegin(); it != m_lampRecords.constEnd(); ++it) {
        const DefectReviewLampRecord &rec = it.value();
        // 算法 OK 仅用于 MAP 着色/tooltip，不参与缺陷列表与复判导航
        if (lampIsAlgorithmOk(rec))
            continue;
        if (!lampPassesContentFilter(rec, m_visibleTypeMask, m_showOkFilter))
            continue;
        m_visibleKeys.append(it.key());
    }

    std::sort(m_visibleKeys.begin(), m_visibleKeys.end(), [](const LampKey &a, const LampKey &b) {
        if (a.dieRow != b.dieRow)
            return a.dieRow < b.dieRow;
        return a.dieCol < b.dieCol;
    });

    if (m_selectedKey.has_value()) {
        if (!m_visibleKeys.contains(*m_selectedKey))
            m_selectedKey = std::nullopt;
    }
}

int DefectReviewViewModel::selectedVisibleIndex() const
{
    if (!m_selectedKey.has_value())
        return -1;
    return m_visibleKeys.indexOf(*m_selectedKey);
}

bool DefectReviewViewModel::isTypeEnabled(inspection::InspectionDefectType type) const
{
    return inspection::inspectionMaskHasDefect(m_validDefectType, type);
}

bool DefectReviewViewModel::isTypeChecked(inspection::InspectionDefectType type) const
{
    return inspection::inspectionMaskHasDefect(m_visibleTypeMask, type);
}

void DefectReviewViewModel::setTypeChecked(inspection::InspectionDefectType type, bool checked)
{
    if (!isTypeEnabled(type))
        return;

    const auto bit = inspection::inspectionDefectTypeBit(type);
    if (checked)
        m_visibleTypeMask |= bit;
    else
        m_visibleTypeMask &= ~bit;

    rebuildVisibleKeys();
    emit sessionChanged();
    emit selectionChanged();
}

void DefectReviewViewModel::setOkFilterChecked(bool checked)
{
    if (m_showOkFilter == checked)
        return;
    m_showOkFilter = checked;
    rebuildVisibleKeys();
    emit sessionChanged();
    emit selectionChanged();
}

Qt::CheckState DefectReviewViewModel::allFiltersCheckState() const
{
    const int enabledDefects = countEnabledDefectBits(m_validDefectType);
    const int checkedDefects = countEnabledDefectBits(m_visibleTypeMask & m_validDefectType);
    const bool anyChecked    = m_showOkFilter || checkedDefects > 0;

    if (!anyChecked)
        return Qt::Unchecked;

    const bool allDefectsChecked =
        enabledDefects == 0 || checkedDefects == enabledDefects;
    if (m_showOkFilter && allDefectsChecked)
        return Qt::Checked;

    return Qt::PartiallyChecked;
}

Qt::CheckState DefectReviewViewModel::allDefectTypesCheckState() const
{
    const int enabledDefects = countEnabledDefectBits(m_validDefectType);
    if (enabledDefects == 0)
        return Qt::Unchecked;

    const int checkedDefects = countEnabledDefectBits(m_visibleTypeMask & m_validDefectType);
    if (checkedDefects == 0)
        return Qt::Unchecked;
    if (checkedDefects == enabledDefects)
        return Qt::Checked;
    return Qt::PartiallyChecked;
}

void DefectReviewViewModel::setAllFiltersChecked(bool checked)
{
    if (checked) {
        if (allFiltersCheckState() == Qt::Checked)
            return;
    } else if (allFiltersCheckState() == Qt::Unchecked) {
        return;
    }

    m_showOkFilter    = checked;
    m_visibleTypeMask = checked ? m_validDefectType : 0;
    rebuildVisibleKeys();
    emit sessionChanged();
    emit selectionChanged();
}

void DefectReviewViewModel::setAllDefectTypesChecked(bool checked)
{
    if (checked) {
        if (allDefectTypesCheckState() == Qt::Checked)
            return;
    } else if (allDefectTypesCheckState() == Qt::Unchecked) {
        return;
    }

    m_visibleTypeMask = checked ? m_validDefectType : 0;
    rebuildVisibleKeys();
    emit sessionChanged();
    emit selectionChanged();
}

void DefectReviewViewModel::selectLamp(const LampKey &key, bool centerMapViewport)
{
    if (!m_lampRecords.contains(key))
        return;
    const DefectReviewLampRecord &rec = m_lampRecords.value(key);
    if (lampIsAlgorithmOk(rec))
        return;
    m_selectedKey = key;
    emit selectionChanged();
    if (centerMapViewport)
        emit selectionCenterRequested();
}

void DefectReviewViewModel::selectVisibleRow(int row)
{
    if (row < 0 || row >= m_visibleKeys.size())
        return;
    selectLamp(m_visibleKeys[row], true);
}

void DefectReviewViewModel::moveSelection(int delta)
{
    if (m_visibleKeys.isEmpty())
        return;

    int idx = selectedVisibleIndex();
    if (idx < 0)
        idx = delta > 0 ? -1 : m_visibleKeys.size();

    idx = qBound(0, idx + delta, m_visibleKeys.size() - 1);
    selectLamp(m_visibleKeys[idx], true);
}

void DefectReviewViewModel::writeLampToWorkingCopy(const DefectReviewLampRecord &record)
{
    for (const DefectItemRef &ref : record.itemRefs) {
        if (ref.partitionIndex < 0 || ref.itemIndex < 0)
            continue;
        if (ref.partitionIndex >= m_workingCopy.defectPartitions.size())
            continue;
        auto &part = m_workingCopy.defectPartitions[ref.partitionIndex];
        if (ref.itemIndex >= part.defectItems.size())
            continue;
        auto &item              = part.defectItems[ref.itemIndex];
        item.rejudgeDefectType  = record.rejudgeDefectType;
        item.isRejudge          = record.isRejudge;
    }
}

void DefectReviewViewModel::setDirty(bool dirty)
{
    if (m_dirty == dirty)
        return;
    m_dirty = dirty;
    emit dirtyChanged(m_dirty);
}

void DefectReviewViewModel::toggleRejudgeDefectType(inspection::InspectionDefectType type)
{
    if (!m_selectedKey.has_value() || !isTypeEnabled(type))
        return;

    DefectReviewLampRecord &rec = m_lampRecords[*m_selectedKey];
    const auto              bit = inspection::inspectionDefectTypeBit(type);
    const bool              turningOn = (rec.rejudgeDefectType & bit) == 0;
    rec.rejudgeDefectType ^= bit;
    rec.isRejudge = true;

    rec.rejudgeMarkerOrder.removeAll(type);
    if (turningOn)
        rec.rejudgeMarkerOrder.append(type);

    writeLampToWorkingCopy(rec);
    setDirty(true);
    rebuildVisibleKeys();
    emit sessionChanged();
    emit selectionChanged();
}

void DefectReviewViewModel::markOk()
{
    if (!m_selectedKey.has_value())
        return;

    DefectReviewLampRecord &rec = m_lampRecords[*m_selectedKey];
    rec.rejudgeDefectType = 0;
    rec.isRejudge         = true;
    rec.rejudgeMarkerOrder.clear();
    writeLampToWorkingCopy(rec);
    setDirty(true);
    rebuildVisibleKeys();
    emit sessionChanged();
    emit selectionChanged();
}

void DefectReviewViewModel::markPending()
{
    if (!m_selectedKey.has_value())
        return;

    DefectReviewLampRecord &rec = m_lampRecords[*m_selectedKey];
    rec.rejudgeDefectType = 0;
    rec.isRejudge         = false;
    rec.rejudgeMarkerOrder.clear();
    writeLampToWorkingCopy(rec);
    setDirty(true);
    rebuildVisibleKeys();
    emit sessionChanged();
    emit selectionChanged();
}

void DefectReviewViewModel::submitCurrentAndNext()
{
    if (!m_selectedKey.has_value())
        return;

    DefectReviewLampRecord &rec = m_lampRecords[*m_selectedKey];
    if (!rec.isRejudge) {
        // Enter：认可当前缺陷（保留 F 键已改掩码；未改时沿用算法 defectType），仅置已复判
        if (rec.rejudgeDefectType == 0 && rec.defectType != 0)
            rec.rejudgeDefectType = rec.defectType;
        rec.isRejudge = true;
        writeLampToWorkingCopy(rec);
        setDirty(true);
        rebuildVisibleKeys();
    }

    const int idx = selectedVisibleIndex();
    emit sessionChanged();
    if (idx >= 0 && idx + 1 < m_visibleKeys.size())
        selectLamp(m_visibleKeys[idx + 1], true);
    else {
        emit selectionChanged();
        if (m_selectedKey.has_value())
            emit selectionCenterRequested();
    }
}

bool DefectReviewViewModel::confirmSave()
{
    if (!canConfirm())
        return false;

    const QString path = m_workingCopy.defectInfoFullFileName.trimmed();
    if (!path.isEmpty()) {
        QString error;
        if (!inspection::saveDefectInfoList(m_workingCopy, &error)) {
            emit confirmFailed(error);
            return false;
        }
    }

    qint64 historyId = m_historyRecordId;
    if (historyId < 1)
        historyId = inspection::InspectionHistoryCoordinator::instance().lastAppendedRecordId();

    QString dbError;
    bool    dbOk = false;
#if COMMON_ORM_ENABLED
    if (historyId > 0)
        dbOk = inspection::InspectionHistoryRepository::instance().updateRecord(historyId, m_workingCopy, &dbError);
    else
        dbOk = inspection::InspectionHistoryRepository::instance().updateRecordBySnTime(
            m_workingCopy.SN, m_workingCopy.detectTime, m_workingCopy, &dbError);
#else
    dbOk = true; // ORM disabled — skip persistence
#endif

    if (!dbOk && !dbError.isEmpty()) {
        qWarning() << "[DefectReview] history update failed:" << dbError;
        emit confirmFailed(dbError);
        return false;
    }

    m_baseline = m_workingCopy;
    setDirty(false);
    emit reviewConfirmed(m_workingCopy);
    emit sessionChanged();
    return true;
}

QImage DefectReviewViewModel::originImageForSelection() const
{
    const auto source = selectedChipImageSource(m_selectedKey, m_lampRecords, m_workingCopy);
    if (!source.has_value())
        return {};
    return cropPartitionImage(source->originPartitionPath, source->roi);
}

QImage DefectReviewViewModel::resultImageForSelection() const
{
    const auto source = selectedChipImageSource(m_selectedKey, m_lampRecords, m_workingCopy);
    if (!source.has_value())
        return {};
    return cropDetectPartitionImage(source->detectPartitionPath, source->roi);
}

QString DefectReviewViewModel::selectionPlaceholderText() const
{
    if (!m_selectedKey.has_value())
        return TR("aoi.review.select_defect");

    const DefectReviewLampRecord &rec = m_lampRecords.value(*m_selectedKey);
    const QString                 types = maskToDefectNames(rec.defectType);
  return QStringLiteral("# (%1,%2) %3 %4")
        .arg(rec.key.dieRow)
        .arg(rec.key.dieCol)
        .arg(chipTypeLabel(rec.chipType))
        .arg(types);
}

void DefectReviewViewModel::handleFunctionKey(int keyIndex)
{
    const auto type = defectTypeForFunctionKey(keyIndex);
    if (!type.has_value() || !isTypeEnabled(*type))
        return;
    toggleRejudgeDefectType(*type);
}

void DefectReviewViewModel::handleKey(Qt::Key key)
{
    switch (key) {
    case Qt::Key_F1:
    case Qt::Key_F2:
    case Qt::Key_F3:
    case Qt::Key_F4:
    case Qt::Key_F5:
    case Qt::Key_F6:
    case Qt::Key_F7:
    case Qt::Key_F8:
    case Qt::Key_F9:
    case Qt::Key_F10:
    case Qt::Key_F11:
        handleFunctionKey(static_cast<int>(key) - static_cast<int>(Qt::Key_F1));
        break;
    case Qt::Key_O:
        markOk();
        break;
    case Qt::Key_R:
        markPending();
        break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        submitCurrentAndNext();
        break;
    case Qt::Key_Up:
        moveSelection(-1);
        break;
    case Qt::Key_Down:
        moveSelection(1);
        break;
    default:
        break;
    }
}

} // namespace aoi::review
