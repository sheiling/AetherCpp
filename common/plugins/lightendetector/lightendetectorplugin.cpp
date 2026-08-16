/**
 * @file lightedetectorplugin.cpp
 * @brief LightenDetector 插件入口类实现。
 */

#include "lightendetectorplugin.h"

#include <QDebug>

// 插件初始化入口；当前仅输出日志，步骤注册由 StepFactory 静态注册完成。
bool LightenDetectorPlugin::initialize(QString *errorString)
{
    Q_UNUSED(errorString)
    qDebug() << "[LightenDetectorPlugin] initialize.";
    return true;
}
