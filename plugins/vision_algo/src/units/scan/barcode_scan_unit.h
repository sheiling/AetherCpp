#pragma once

/**
 * @file barcode_scan_unit.h
 * @brief 条码/二维码扫描（ZXing，经 AlgorithmBase::Execute）
 */

#include <common/base/algorithm/algorithm_base.h>

namespace vision_algo {

class BarcodeScanUnit final : public common::algorithm::AlgorithmBase
{
public:
    std::string Name() const override;
    common::errors::ErrorPtr Execute(common::algorithm::AlgorithmExecuteData &data) override;
};

} // namespace vision_algo
