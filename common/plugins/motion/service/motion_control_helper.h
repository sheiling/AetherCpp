#pragma once

#include <common/core/device/motionControl/motion_controller.h>

#include <QHash>
#include <QPair>
#include <QString>
#include <QStringList>
#include <QVector>

#include <memory>

namespace motion {

/** DeviceManager.MotionController.DeviceIo 单路配置 */
struct MotionIoPointConfig
{
    QString name;
    int     port       = 0;
    bool    isOutput   = false;
    bool    highActive = false;
};

struct IoCombineOutput
{
    QString name;
    bool    set = false;
};

struct IoCombineAction
{
    QString                  name;
    QVector<IoCombineOutput> outputs;
};

struct IoCombineGroup
{
    QString                  group;
    QVector<IoCombineAction> actions;
};

/** 从 exe 目录 config/device_config.json 读取 / 写回运控 IO 配置 */
class MotionDeviceConfigReader
{
public:
    struct Snapshot
    {
        QVector<MotionIoPointConfig> deviceIo;
        QStringList                  motionCardNames;
    };

    /** 一次读取 device_config.json，解析 DeviceIo 与 Card */
    static Snapshot loadSnapshot();
    static QVector<MotionIoPointConfig> loadDeviceIo();
    static QStringList                  loadMotionCardNames();
    /** 批量更新 DeviceIo 的 port/logic；读写均使用 ordered_json 保持字段与键序 */
    static bool updateDeviceIoBatch(const QHash<QString, QPair<int, int>> &portLogicByName);

    static void partitionByDirection(const QVector<MotionIoPointConfig> &all,
                                     QVector<MotionIoPointConfig>       *inputs,
                                     QVector<MotionIoPointConfig>       *outputs);
};

/** 按 DI/DO 方向筛选 IO 配置 */
QVector<MotionIoPointConfig> filterIoConfigsByDirection(const QVector<MotionIoPointConfig> &all,
                                                        bool                                  isOutput);

/** 从 exe 目录 config/ioCombine.json 读取联动调试配置 */
QVector<IoCombineGroup> loadIoCombineConfig();

std::shared_ptr<common::device::MotionController> motionControllerOrNull();

/** 写入单路 IO，失败时输出 qWarning */
bool writeIo(const QString &ioName, bool value, const char *logContext);

/** 按联动配置批量写入 IO */
void writeIoCombineOutputs(const QVector<IoCombineOutput> &outputs, const char *logContext);

/**
 * 读取硬件端口位图并映射为 IO 名称 → 有效状态。
 * @param readDoPorts true 读 DO，false 读 DI
 */
QHash<QString, bool> queryIoStates(const std::shared_ptr<common::device::MotionController> &motion,
                                   const QStringList                  &cardNames,
                                   const QVector<MotionIoPointConfig> &configs,
                                   bool                                readDoPorts);

} // namespace motion
