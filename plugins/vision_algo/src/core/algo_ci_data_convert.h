#pragma once

/**
 * @file algo_ci_data_convert.h
 * @brief vision_algo CI 类型与 CISDK 互转（仅 VisionAlgo 插件内部 include）
 */

#include <vision_algo/core/algorithm_unit_type.h>

#include <algo_led_meter_data.h>
#include <algo_shot_data.h>
#include <ci_data.h>
#include <meta_info_data.h>

#ifdef VISION_ALGO_HAS_CHIP
#    include <chip_detector_data.h>
#endif
#ifdef VISION_ALGO_HAS_DIE_BONDING
#    include <vision_algo/units/die_bonding/die_bonding_chip_detect_data.h>
#    include <die_bonding_defect_detector_data.h>
#endif

#include <vector>

namespace vision_algo::cisdk_convert {

// --- 基础几何 ---

::AlgoSize     toCisdkSize(const AlgoSize &size);
AlgoSize       fromCisdkSize(const ::AlgoSize &size);
::AlgoSize2f   toCisdkSize2f(const AlgoSize2f &size);
AlgoSize2f     fromCisdkSize2f(const ::AlgoSize2f &size);
::AlgoPoint    toCisdkPoint(const AlgoPoint &point);
AlgoPoint      fromCisdkPoint(const ::AlgoPoint &point);
::AlgoPoint2f  toCisdkPoint2f(const AlgoPoint2f &point);
AlgoPoint2f    fromCisdkPoint2f(const ::AlgoPoint2f &point);
::AlgoRect     toCisdkRect(const AlgoRect &rect);
AlgoRect       fromCisdkRect(const ::AlgoRect &rect);
::AlgoRect2f   toCisdkRect2f(const AlgoRect2f &rect);
AlgoRect2f     fromCisdkRect2f(const ::AlgoRect2f &rect);
AlgoCornerPoint cornerPointsFromRect(const AlgoRect &rect);

::AlgoMat toCisdkMat(const AlgoMat &mat);
AlgoMat   fromCisdkMat(const ::AlgoMat &mat);

// --- 色度分量 ---

TriLab     toCisdk(const AlgoTriLab &v);
AlgoTriLab fromCisdk(const TriLab &v);
TriLch     toCisdk(const AlgoTriLch &v);
AlgoTriLch fromCisdk(const TriLch &v);
TriLxy     toCisdk(const AlgoTriLxy &v);
AlgoTriLxy fromCisdk(const TriLxy &v);
TriXYZ     toCisdk(const AlgoTriXYZ &v);
AlgoTriXYZ fromCisdk(const TriXYZ &v);
TriRGB     toCisdk(const AlgoTriRGB &v);
AlgoTriRGB fromCisdk(const TriRGB &v);
RGBW       toCisdk(const AlgoRGBW &v);
AlgoRGBW   fromCisdk(const RGBW &v);
RGBLxy     toCisdk(const AlgoRGBLxy &v);
AlgoRGBLxy fromCisdk(const RGBLxy &v);
RGBWLxy    toCisdk(const AlgoRGBWLxy &v);
AlgoRGBWLxy fromCisdk(const RGBWLxy &v);
RGBWXYZ    toCisdk(const AlgoRGBWXYZ &v);
AlgoRGBWXYZ fromCisdk(const RGBWXYZ &v);
RGBXYZ     toCisdk(const AlgoRGBXYZ &v);
AlgoRGBXYZ fromCisdk(const RGBXYZ &v);

// --- 元信息 ---

Device       toCisdk(const AlgoDevice &device);
AlgoDevice   fromCisdk(const Device &device);
Capture      toCisdk(const AlgoCapture &capture);
AlgoCapture  fromCisdk(const Capture &capture);
Layer        toCisdk(const AlgoLayer &layer);
AlgoLayer    fromCisdk(const Layer &layer);
LedPanel     toCisdk(const AlgoLedPanel &panel);
AlgoLedPanel fromCisdk(const LedPanel &panel);
Pattern      toCisdk(const AlgoPattern &pattern);
AlgoPattern  fromCisdk(const Pattern &pattern);
Partition    toCisdk(const AlgoPartition &partition);
AlgoPartition fromCisdk(const Partition &partition);

void releaseAlgoShot(AlgoShot &shot);
AlgoShot fromCisdkShot(const ::AlgoShot &shot);
::AlgoShot toCisdkShot(const AlgoShot &shot);
void releaseCisdkShot(::AlgoShot &shot);

void releaseAlgoLedMeter(AlgoLedMeter &meter);
AlgoLedMeter fromCisdkLedMeter(const ::AlgoLedMeter &meter);
::AlgoLedMeter toCisdkLedMeter(const AlgoLedMeter &meter);
void releaseCisdkLedMeter(::AlgoLedMeter &meter);

::AlgoLedMeterMergeInfo toCisdk(const AlgoLedMeterMergeInfo &info);
AlgoLedMeterMergeInfo fromCisdk(const ::AlgoLedMeterMergeInfo &info);

// --- chip_detector（CISDK chip_detector_data.h）---

#ifdef VISION_ALGO_HAS_CHIP
ChipDetectRGBType      toCisdkRgbType(ChipDetectRgbType type);
ChipDetectRGBAlignType toCisdkRgbAlignType(ChipDetectRgbAlignType type);
ChipDetectThresh       toCisdkChipThreshold(const ChipDetectThreshold &thresh);
#endif

// --- die_bonding（CISDK die_bonding_defect_detector_data.h）---

#ifdef VISION_ALGO_HAS_DIE_BONDING
ChipDetectInfo fromCisdkChip(const DieBondingChipInfo &src);
ChipGroupDetectInfo fromCisdkGroup(const DieBondingChipGroupInfo &src);
std::vector<ChipGroupDetectInfo> fromCisdkGroups(const std::vector<DieBondingChipGroupInfo> &src);
DieBondingChipThreshold toCisdkThreshold(const ChipThreshold &src);
#endif

} // namespace vision_algo::cisdk_convert
