#pragma once

#include "pagewidget.h"

#include "motion_control_helper.h"

#include <QHash>
#include <QString>

class QButtonGroup;
class QPushButton;
class QStackedWidget;
class QTimer;
class QWidget;

namespace motion {

class IoCombineDebugWidget;
class IoPointWidget;

/** 运控调试页：竖向主导航 + 二级 Tab（单点调试 / 联动调试） */
class MotionDebugView : public PageWidget
{
    Q_OBJECT

public:
    explicit MotionDebugView(QWidget *parent = nullptr);

    /** true：显示列头「读入/写出」按钮及 IO 点完整表单；false：隐藏 */
    void setShowFullDetails(bool show);

protected:
    void onLoad() override;
    void applyTheme() override;

private:
    void buildLayout();
    void bindPermissions();
    void applyPermissions();
    void rebuildSinglePointIoPage(bool reloadFromDisk = true);
    void refreshSinglePointIoStates();
    void loadIoColumnFromConfig(bool isOutput);
    void saveIoColumnToConfig(bool isOutput);

    QWidget *buildIoColumn(const QString &columnTitle,
                           const QVector<MotionIoPointConfig> &points,
                           bool isOutput,
                           QWidget *parent,
                           QHash<QString, IoPointWidget *> *pointRegistry,
                           QPushButton **outReadBtn,
                           QPushButton **outWriteBtn);
    QWidget *buildPlaceholderPage(const QString &title, const QString &hint);

    void reloadDeviceIoCache();
    void applyIoStatesToPoints(const QHash<QString, IoPointWidget *> &points,
                               const QHash<QString, bool>              &states);

    QWidget        *m_primaryNav        = nullptr;
    QButtonGroup   *m_primaryGroup      = nullptr;
    QWidget        *m_subTabBar         = nullptr;
    QButtonGroup   *m_subTabGroup       = nullptr;
    QStackedWidget *m_ioSubStack        = nullptr;
    QStackedWidget *m_mainStack         = nullptr;
    QWidget        *m_mainPanel         = nullptr;
    QWidget              *m_singlePointIoPage = nullptr;
    IoCombineDebugWidget *m_ioCombinePage     = nullptr;
    QTimer         *m_ioPollTimer       = nullptr;
    QHash<QString, IoPointWidget *> m_inputPoints;
    QHash<QString, IoPointWidget *> m_outputPoints;
    QPushButton *m_diReadBtn  = nullptr;
    QPushButton *m_diWriteBtn = nullptr;
    QPushButton *m_doReadBtn  = nullptr;
    QPushButton *m_doWriteBtn = nullptr;
    bool         m_showFullDetails = false;

    QVector<MotionIoPointConfig> m_deviceIoConfigs;
    QStringList                  m_motionCardNames;
};

} // namespace motion
