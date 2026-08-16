#pragma once

#include <common/base/algorithm/algorithm_base.h>

#include <vision_algo/units/chip_detector/chip_detector_data.h>

namespace vision_algo {

class ChipDetector final : public common::algorithm::AlgorithmBase
{
public:
    std::string Name() const override;
    common::errors::ErrorPtr Execute(common::algorithm::AlgorithmExecuteData &data) override;

    static common::errors::ErrorPtr runDetect(const ChipDetectorInput::Fields &in,
                                              ChipDetectorOutput::Fields &out);
};

} // namespace vision_algo
