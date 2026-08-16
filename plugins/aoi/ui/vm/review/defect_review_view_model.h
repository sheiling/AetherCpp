#pragma once

/**
 * @file defect_review_view_model.h
 * @brief 缺陷复判 ViewModel：筛选、选中、内存复判缓存与确认落盘
 */

#include "defect_review_types.h"

#include "inspection_types.h"

#include "viewmodel.h"

#include <optional>

#include <Qt>

#include <QHash>
#include <QObject>
#include <QImage>
#include <optional>

namespace aoi::review {

/**
 * @brief 缺陷复判会话状态机
 *
 * - `workingCopy` 承载全部复判编辑；快捷键与确认按钮仅作用于该副本。
 * - 板级展示以复判字段为准（`isRejudge` / `rejudgeDefectType`）。
 */
class DefectReviewViewModel : public ViewModel
{
    Q_OBJECT

public:
    explicit DefectReviewViewModel(bool bindBridge = true, QObject *parent = nullptr);

    void setHistoryRecordId(qint64 id) { m_historyRecordId = id; }
    qint64 historyRecordId() const { return m_historyRecordId; }
    /** 历史弹窗可无 JSON 路径仍允许确认落库 */
    void setRequireFilePathForConfirm(bool required) { m_requireFilePathForConfirm = required; }

    void setDieGrid(const DefectReviewDieGrid &grid);
    void loadReviewSession(const inspection::DefectInfoList &data);

    inspection::DefectInfoList workingDefectData() const { return m_workingCopy; }
    bool                       isDirty() const { return m_dirty; }

    DefectReviewDieGrid dieGrid() const { return m_dieGrid; }
    inspection::InspectionDefectTypeMask validDefectTypeMask() const { return m_validDefectType; }
    inspection::InspectionDefectTypeMask visibleTypeMask() const { return m_visibleTypeMask; }

    bool hasTypeChecked() const { return m_visibleTypeMask != 0 || m_showOkFilter; }

    bool isOkFilterChecked() const { return m_showOkFilter; }
    bool canConfirm() const
    {
        return m_dirty
               && (!m_requireFilePathForConfirm
                   || !m_workingCopy.defectInfoFullFileName.trimmed().isEmpty());
    }

    const QHash<LampKey, DefectReviewLampRecord> &lampRecords() const { return m_lampRecords; }
    const QHash<LampKey, DefectDieChipInfo>     &dieChipLayout() const { return m_dieChipLayout; }
    const QVector<LampKey>                       &visibleLampKeys() const { return m_visibleKeys; }

    std::optional<LampKey> selectedKey() const { return m_selectedKey; }
    int                    selectedVisibleIndex() const;

    bool isTypeEnabled(inspection::InspectionDefectType type) const;
    bool isTypeChecked(inspection::InspectionDefectType type) const;

    Qt::CheckState allFiltersCheckState() const;
    Qt::CheckState allDefectTypesCheckState() const;

    QImage originImageForSelection() const;
    QImage resultImageForSelection() const;
    QString selectionPlaceholderText() const;

public slots:
    void setTypeChecked(inspection::InspectionDefectType type, bool checked);
    void setOkFilterChecked(bool checked);
    void setAllFiltersChecked(bool checked);
    void setAllDefectTypesChecked(bool checked);
    void selectLamp(const LampKey &key, bool centerMapViewport = false);
    void selectVisibleRow(int row);
    void moveSelection(int delta);
    void toggleRejudgeDefectType(inspection::InspectionDefectType type);
    void markOk();
    void markPending();
    void submitCurrentAndNext();
    bool confirmSave();
    void handleKey(Qt::Key key);

signals:
    void sessionChanged();
    void selectionChanged();
    /** 列表/方向键/Enter 等导航选中后，请求 MAP 将当前项居中（MAP 点击选中不触发） */
    void selectionCenterRequested();
    void dirtyChanged(bool dirty);
    void reviewConfirmed(const inspection::DefectInfoList &data);
    void confirmFailed(const QString &message);

private:
    void bindActiveBridge();
    void rebuildIndex();
    void rebuildDieChipLayout();
    void rebuildVisibleKeys();
    void applyDieGridFromData();
    void writeLampToWorkingCopy(const DefectReviewLampRecord &record);
    void setDirty(bool dirty);
    void handleFunctionKey(int keyIndex);

    inspection::DefectInfoList                 m_baseline;
    inspection::DefectInfoList                 m_workingCopy;
    DefectReviewDieGrid                        m_dieGrid;
    DefectReviewDieGrid                        m_pendingDieGrid;
    inspection::InspectionDefectTypeMask       m_validDefectType = 0;
    inspection::InspectionDefectTypeMask       m_visibleTypeMask = 0;
    bool                                       m_showOkFilter    = false;
    QHash<LampKey, DefectReviewLampRecord>     m_lampRecords;
    QHash<LampKey, DefectDieChipInfo>          m_dieChipLayout;
    QVector<LampKey>                           m_visibleKeys; ///< 缺陷列表与键盘导航；不含算法 OK 晶粒
    std::optional<LampKey>                     m_selectedKey;
    bool                                       m_dirty = false;
    bool                                       m_bindBridge                  = true;
    bool                                       m_requireFilePathForConfirm   = true;
    qint64                                     m_historyRecordId             = -1;
};

} // namespace aoi::review
