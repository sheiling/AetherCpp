#pragma once

/**
 * @file ivision_algo_service.h
 * @brief 视觉算法插件对外服务接口（由 ExtensionSystem 注册为全局对象）
 *
 * 算法单元统一经 execute(unitName, AlgorithmExecuteData) 调度；
 * 各单元操作由入参 Packet 中的 op / 字段定义。
 */

#include "vision_algo_plugin_global.h"
#include "vision_algo/core/algorithm_unit_type.h"

#include <common/base/algorithm/algorithm_packet.h>
#include <common/utility/errors/include/error.h>

#include <QObject>

#include <future>
#include <string>

namespace vision_algo {

class VISION_ALGO_EXPORT IVisionAlgoService : public QObject
{
    Q_OBJECT

public:
    explicit IVisionAlgoService(QObject *parent = nullptr) : QObject(parent) {}
    ~IVisionAlgoService() override = default;

    /** 插件 initialize 完成后为 true（AlgorithmFactory 已注册） */
    virtual bool isReady() const = 0;

    /** 判断指定算法单元是否已在 AlgorithmFactory 注册 */
    virtual bool isUnitAvailable(const std::string &unitName) const = 0;

    /** 按 AlgorithmUnitType 判断单元是否可用（语法糖） */
    bool isUnitAvailable(AlgorithmUnitType type) const
    {
        return isUnitAvailable(unitTypeName(type));
    }

    /**
     * @brief 同步执行算法单元
     * @param unitName 与 REGISTER_ALGORITHM 一致，如 kUnitDieBondingChipDetect
     * @param data     inputs / outputs 为各单元 Packet
     */
    virtual common::errors::ErrorPtr execute(const std::string &unitName,
                                             common::algorithm::AlgorithmExecuteData &data) = 0;

    /** 异步执行（param 所指 Packet 须在 future 完成前保持有效） */
    virtual std::future<common::errors::ErrorPtr> executeAsync(
        const std::string &unitName, common::algorithm::AlgorithmExecuteData *data) = 0;
};

} // namespace vision_algo
