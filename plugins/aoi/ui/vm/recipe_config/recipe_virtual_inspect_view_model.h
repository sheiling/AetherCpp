#pragma once

/**
 * @file recipe_virtual_inspect_view_model.h
 * @brief 配方配置页虚拟调测子 ViewModel：AdHoc detectFov + CurrentInspectionSnapshot
 */

#include "data/dataDefinition/inspection_types.h"

#include <QImage>
#include <QObject>

class QThread;

namespace aoi::inspection {
class VirtualInspectRunner;
}

namespace aoi {

class RecipeConfigViewModel;

/**
 * @brief 虚拟调测：内存图 → AdHoc detectFov → 汇总快照；明细由复判页承担
 *
 * 产线运行中禁止发起；运行状态/进度/快照信号由 View 直连本 VM。
 */
class RecipeVirtualInspectViewModel : public QObject
{
    Q_OBJECT

public:
    explicit RecipeVirtualInspectViewModel(RecipeConfigViewModel *host, QObject *parent = nullptr);
    ~RecipeVirtualInspectViewModel() override;

    bool                           virtualInspecting() const { return m_virtualInspecting; }
    aoi::CurrentInspectionSnapshot virtualInspectSnapshot() const { return m_inspectSnapshot; }

    /** 非产线 Running / 扫描 flow 运行中时可虚拟调测 */
    bool canVirtualInspect(QString *blockedReason = nullptr) const;

public slots:
    void virtualInspectFromImage(const QImage &image);
    void cancelVirtualInspect();

signals:
    void virtualInspectingChanged();
    void virtualInspectSnapshotChanged();
    /** percent 0–100；-1 表示结束；stageKey 为 i18n key */
    void virtualInspectProgress(int percent, const QString &stageKey);

private slots:
    void onRunnerProgress(int percent, const QString &stageKey);
    void finishVirtualInspect(aoi::CurrentInspectionSnapshot snapshot, const QString &error);
    void tickDetectingAnimation();

private:
    void setVirtualInspecting(bool inspecting);
    void startDetectingAnimation();
    void stopDetectingAnimation();

    RecipeConfigViewModel            *m_host = nullptr;
    bool                              m_virtualInspecting = false;
    aoi::CurrentInspectionSnapshot    m_inspectSnapshot;

    QThread                          *m_runnerThread = nullptr;
    inspection::VirtualInspectRunner *m_runner       = nullptr;

    class QTimer *m_detectingTimer    = nullptr;
    int           m_detectingDotCount = 0;
};

} // namespace aoi
