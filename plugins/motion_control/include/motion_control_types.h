#pragma once

/**
 * @file motion_control_types.h
 * @brief MotionControl 插件对外数据类型（轴/IO 分组与运行时状态）
 */

#include "motion_control_plugin_global.h"

#include <QHash>
#include <QMap>
#include <QString>
#include <QStringList>

namespace motion_control_plugin {

/** 单轴运行时快照 */
struct MOTION_CONTROL_PLUGIN_EXPORT AxisRuntimeState
{
    QString name;
    QString moduleName;
    double  cmdPos      = 0.0;
    double  fbPos       = 0.0;
    bool    enabled     = false;
    bool    alarm       = false;
    bool    inPosition  = false;
    bool    origin      = false;
    bool    hardPositiveLimit = false;
    bool    hardNegativeLimit = false;
    bool    softPositiveLimit = false;
    bool    softNegativeLimit = false;
    bool    positiveLimit = false; ///< 正限位（硬或软）
    bool    negativeLimit = false; ///< 负限位（硬或软）
};

/** 单路 IO 运行时快照 */
struct MOTION_CONTROL_PLUGIN_EXPORT IoRuntimeState
{
    QString name;
    QString moduleName;
    bool    isOutput = false;
    bool    value    = false;
};

/** 模块名 → 轴名列表 */
using AxisModuleMap = QMap<QString, QStringList>;
/** 模块名 → IO 名列表 */
using IoModuleMap = QMap<QString, QStringList>;

/** 轴静态配置（对应 DeviceManager MotionController 轴参数，供设置页读写） */
struct MOTION_CONTROL_PLUGIN_EXPORT AxisConfigData
{
    QString name;
    QString moduleName;
    QString cardName;
    bool    enabled          = true;
    int     axisNo           = 0;
    int     axisType         = 1;
    int     unitType         = 0;
    int     encoderType      = 0;
    int     driverType       = 0;
    int     homeDir          = 1;
    int     homeEdgeType     = 3;
    int     homeVelMode      = 1;
    int     homePinLogic     = -1;
    int     homeSourceSel    = 1;
    QString homeMode         = QStringLiteral("Origin_FindHome");
    double  homeOffset       = 0.0;
    double  homeHighVel      = 50.0;
    double  homeLowVel       = 5.0;
    double  homeTacc         = 0.5;
    double  pulseUnit        = 1.0;
    double  pulseEquivalent  = -1.0;
    double  negativeLimit    = 0.0;
    double  positiveLimit    = 0.0;
    double  errorBandPluse   = 0.0;
    int     sourceSelLimit   = 1;
    int     softwareLimitAction = 0;
    double  startVel            = 2.0;
    double  stopVel             = 2.0;
    double  maxVel              = 50.0;
    double  tacc                = 0.5;
    double  tdec                = 0.5;
    double  sPara               = 0.2;
};

/** IO 静态配置（对应 MotionController DeviceIo，供 IO 参数页读写） */
struct MOTION_CONTROL_PLUGIN_EXPORT IoConfigData
{
    QString name;
    QString moduleName;
    QString cardName;
    bool    isOutput = false; ///< type==1 为输出
    int     port     = 0;
    int     logic    = 0;     ///< 0=低有效，1=高有效
    int     extend   = 0;
    int     node     = 0;
};

} // namespace motion_control_plugin
