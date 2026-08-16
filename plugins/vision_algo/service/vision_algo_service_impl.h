#pragma once

/**
 * @file vision_algo_service_impl.h
 * @brief IVisionAlgoService 插件内实现（委托 common::algorithm::AlgorithmFactory）
 */

#include "ivision_algo_service.h"

namespace vision_algo {

class VisionAlgoServiceImpl final : public IVisionAlgoService
{
    Q_OBJECT

public:
    explicit VisionAlgoServiceImpl(QObject *parent = nullptr);

    void initialize();

    bool isReady() const override;

    bool isUnitAvailable(const std::string &unitName) const override;

    common::errors::ErrorPtr execute(const std::string &unitName,
                                     common::algorithm::AlgorithmExecuteData &data) override;

    std::future<common::errors::ErrorPtr> executeAsync(
        const std::string &unitName, common::algorithm::AlgorithmExecuteData *data) override;

private:
    bool m_ready = false;
};

} // namespace vision_algo
