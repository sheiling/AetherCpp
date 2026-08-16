#pragma once

/**
 * @file virtual_inspect_runner.h
 * @brief 配方页虚拟调测：后台线程执行 AdHoc FOV 检测（detectFov + 信号回传）
 */

#include "data/dataDefinition/inspection_types.h"
#include "data/dataModel/recipe_config_model.h"

#include <QImage>
#include <QObject>
#include <QString>

namespace aoi::inspection {

/** 在系统临时目录生成唯一 PNG 路径（供 detect 引擎导出/原图落盘） */
bool makeTempResultImagePath(QString *outPath);

class VirtualInspectRunner : public QObject
{
    Q_OBJECT

public:
    explicit VirtualInspectRunner(QObject *parent = nullptr);
    ~VirtualInspectRunner() override;

public slots:
    /** VM 送内存图（QImage），经 detectFov(AdHoc) 执行单张检测 */
    void start(const RecipeDocument &recipe, const QImage &frame, const QString &sourceImagePath);
    void cancel();

    aoi::CurrentInspectionSnapshot lastSnapshot() const { return m_lastSnapshot; }
    QString                        lastError() const { return m_lastError; }

signals:
    void progressChanged(int percent, const QString &stageKey);
    /** error 非空表示失败/取消；成功时 snapshot 已填充，DefectInfoList 已由 Runner 上报复判 */
    void finished(aoi::CurrentInspectionSnapshot snapshot, QString error);

private:
    void bumpProgress(int percent, const QString &stageKey);
    void finishWithResult(aoi::CurrentInspectionSnapshot snapshot, QString error);

    bool                          m_cancelRequested = false;
    bool                          m_running         = false;
    aoi::CurrentInspectionSnapshot m_lastSnapshot;
    QString                        m_lastError;
};

} // namespace aoi::inspection

// Q_DECLARE_METATYPE moved to virtual_inspect_runner.cpp to avoid
// C2766/C2908 conflicts with Qt6's internal QMetaTypeId instantiation.
