#pragma once

/**
 * @file die_bonding_chip_detect.h
 * @brief 固晶芯片检测算法单元（传统 + DL，经 AlgorithmBase::Execute 调用）
 */

#include <common/base/algorithm/algorithm_base.h>

#include <vision_algo/units/die_bonding/die_bonding_chip_detect_data.h>

#include <die_bonding_defect_detector.h>
#include <model_processor.h>

namespace vision_algo {

class DieBondingChipDetect final : public common::algorithm::AlgorithmBase
{
public:
    std::string Name() const override;
    common::errors::ErrorPtr Execute(common::algorithm::AlgorithmExecuteData &data) override;

    static common::errors::ErrorPtr runDetect(const DieBondingChipDetectInput::DetectFields &in,
                                              DieBondingChipDetectOutput::DetectResult &out,
                                              void *nativeDetectHandle);

private:
    common::errors::ErrorPtr setDetectData(AlgoHandle handle,
                                           const DieBondingChipDetectInput::DetectFields &in);

    common::errors::ErrorPtr chipDetect(AlgoHandle handle, ::AlgoMat *dstMat, int alignType,
                                        std::vector<DieBondingChipGroupInfo> &groups, int &outRow,
                                        int &outCol);

    common::errors::ErrorPtr chipDetectDL(AlgoHandle handle, ::AlgoMat *dstMat,
                                          const DieBondingChipDetectInput::DetectFields &in,
                                          std::vector<DieBondingChipGroupInfo> &groups,
                                          int &outRow, int &outCol, int &groupNum);

    bool saveDetectImage(AlgoHandle handle, const std::string &filename, bool isDeepLearning);

    bool isOk(const DieBondingChipDetectInput::DetectFields &in,
              DieBondingChipGroupInfo *chip_group_infos, int groupNum);

    std::string m_imageIndex;
    int         m_iImageIndex = 0;
    bool        m_isPolarityDetect = false;
    bool        m_isDefectClassify = false;
};

} // namespace vision_algo
