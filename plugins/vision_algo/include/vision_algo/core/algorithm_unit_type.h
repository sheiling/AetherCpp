#pragma once

/**
 * @file algorithm_unit_type.h
 * @brief vision_algo 命名空间下的算法公共类型（几何、色度、元信息、单元 Param）
 *
 * 本头文件定义 `vision_algo::` 前缀类型，与 CISDK 全局 `::` 类型（ci_data.h、meta_info_data.h 等）
 * 字段布局一一对应，但避免在对外 API 中直接暴露 CISDK 头文件。
 * 与 CISDK 的互转见 `src/core/algo_ci_data_convert.h` / `algo_ci_data_convert.cpp`（`vision_algo::cisdk_convert`）。
 */

#include <common/core/device/imageCapture/image_capture.h>

#include <nlohmann/json.hpp>

#include <cstdint>
#include <memory>
#include <string>

namespace vision_algo {

// --- 枚举（均附带 NLOHMANN_JSON_SERIALIZE_ENUM）---

/** 算法单元类型标识 */
enum class AlgorithmUnitType : std::uint8_t
{
	Unknown = 0,
	MarkPointLocate,
	ChipDetector,
	DieBondingChipDetect,
	BarcodeScan,
	ModelProcessor,
};
NLOHMANN_JSON_SERIALIZE_ENUM(AlgorithmUnitType, {
	{ AlgorithmUnitType::Unknown, "Unknown" },
	{ AlgorithmUnitType::MarkPointLocate, "MarkPointLocate" },
	{ AlgorithmUnitType::ChipDetector, "ChipDetector" },
	{ AlgorithmUnitType::DieBondingChipDetect, "DieBondingChipDetect" },
	{ AlgorithmUnitType::BarcodeScan, "BarcodeScan" },
	{ AlgorithmUnitType::ModelProcessor, "ModelProcessor" },
})

/** 算法单元注册名（与 REGISTER_ALGORITHM / AlgorithmFactory key 一致） */
constexpr const char *kUnitMarkPointLocate      = "MarkPointLocate";
constexpr const char *kUnitChipDetector         = "ChipDetector";
constexpr const char *kUnitDieBondingChipDetect = "DieBondingChipDetect";
constexpr const char *kUnitBarcodeScan          = "BarcodeScan";
constexpr const char *kUnitModelProcessor       = "ModelProcessor";

/** 将 AlgorithmUnitType 映射为 AlgorithmFactory 注册名 */
inline const char *unitTypeName(AlgorithmUnitType type)
{
	switch (type) {
	case AlgorithmUnitType::MarkPointLocate: return kUnitMarkPointLocate;
	case AlgorithmUnitType::ChipDetector: return kUnitChipDetector;
	case AlgorithmUnitType::DieBondingChipDetect: return kUnitDieBondingChipDetect;
	case AlgorithmUnitType::BarcodeScan: return kUnitBarcodeScan;
	case AlgorithmUnitType::ModelProcessor: return kUnitModelProcessor;
	default: return "";
	}
}

/**
 * 各算法单元 execute 入参公共图像上下文（原 AlgorithmInputBase）
 * - processImage：工步/配方传入图像（Bayer 原图或 RGB process 图）
 * - imageCapture：非空时单元内可调用 makeRgbForProcess / convertForDisplay
 */
struct VisionImageContext
{
	std::shared_ptr<common::device::ImageInfo> processImage; ///< 待处理图像（Bayer 或 RGB）
	common::device::ImageCapture              *imageCapture = nullptr; ///< 非空时可单元内 debayer

	std::string imageIndex;   ///< 图像索引字符串（如 FOV 编号）
	int         iImageIndex = 0; ///< 图像序号（整型）
	std::string sn;           ///< 板级序列号

	bool        saveImageFlag  = false; ///< 是否保存检测结果图
	bool        saveMatFlag    = false; ///< 是否保存 AlgoMat 文件
	bool        saveDefectFlag = false; ///< 是否保存缺陷图
	std::string defectImageDir;         ///< 缺陷图输出目录
	std::string detectImageFileName;    ///< 检测图保存路径
	std::string orignImageFileName;     ///< 原图保存路径
	std::string amatFileName;           ///< AlgoMat 保存路径
};

/** 各算法单元 execute 出参公共元信息 */
struct VisionResultMeta
{
	std::string imageIndex; ///< 与入参对应的图像索引
	bool        ok = true;  ///< 本帧/本步检测是否通过
};

/** 色度分量类型，对应 CISDK CIEType */
enum class AlgoCIEType : int
{
	CIE_UNDEFINED = 0x00000000,
	CIE_X         = 0x00000001,
	CIE_Y         = 0x00000002,
	CIE_Z         = 0x00000004,
	CIE_XYZ       = 0x00000007,
	CAM_R         = 0x00000010,
	CAM_G         = 0x00000020,
	CAM_B         = 0x00000040,
	CAM_RGB       = 0x00000070,
};
NLOHMANN_JSON_SERIALIZE_ENUM(AlgoCIEType, {
	{ AlgoCIEType::CIE_UNDEFINED, "CIE_UNDEFINED" },
	{ AlgoCIEType::CIE_X, "CIE_X" },
	{ AlgoCIEType::CIE_Y, "CIE_Y" },
	{ AlgoCIEType::CIE_Z, "CIE_Z" },
	{ AlgoCIEType::CIE_XYZ, "CIE_XYZ" },
	{ AlgoCIEType::CAM_R, "CAM_R" },
	{ AlgoCIEType::CAM_G, "CAM_G" },
	{ AlgoCIEType::CAM_B, "CAM_B" },
	{ AlgoCIEType::CAM_RGB, "CAM_RGB" },
})

/** Bayer 排列类型，对应 CISDK BayerType */
enum class AlgoBayerType : int
{
	COLOR_BayerNone = 0,
	COLOR_BayerBG   = 46,
	COLOR_BayerGB   = 47,
	COLOR_BayerRG   = 48,
	COLOR_BayerGR   = 49,
};
NLOHMANN_JSON_SERIALIZE_ENUM(AlgoBayerType, {
	{ AlgoBayerType::COLOR_BayerNone, "COLOR_BayerNone" },
	{ AlgoBayerType::COLOR_BayerBG, "COLOR_BayerBG" },
	{ AlgoBayerType::COLOR_BayerGB, "COLOR_BayerGB" },
	{ AlgoBayerType::COLOR_BayerRG, "COLOR_BayerRG" },
	{ AlgoBayerType::COLOR_BayerGR, "COLOR_BayerGR" },
})

/** 相机型号，对应 CISDK CameraType */
enum class AlgoCameraType : int
{
	CAM_UNDEFINED           = 0,
	CAM_16_BIT_DEPTH        = 16,
	CAM_14_BIT_DEPTH        = 14,
	CAM_12_BIT_DEPTH        = 12,
	CAM_10_BIT_DEPTH        = 10,
	CAM_8_BIT_DEPTH         = 8,
	CAM_C1200               = 120016,
	CAM_C3200               = 320012,
	CAM_TPM6100             = 610016,
	CAM_SC6100              = 610116,
	CAM_TP2600              = 260016,
	CAM_TP2000              = 200012,
	CAM_MOBILE_MI11         = 77771110,
	CAM_MER_2000_5GC        = 68200012,
	CAM_MER_503_20GM        = 68050310,
	CAM_MER_503_20GM_DEPTH8 = 68050308,
};
NLOHMANN_JSON_SERIALIZE_ENUM(AlgoCameraType, {
	{ AlgoCameraType::CAM_UNDEFINED, "CAM_UNDEFINED" },
	{ AlgoCameraType::CAM_16_BIT_DEPTH, "CAM_16_BIT_DEPTH" },
	{ AlgoCameraType::CAM_14_BIT_DEPTH, "CAM_14_BIT_DEPTH" },
	{ AlgoCameraType::CAM_12_BIT_DEPTH, "CAM_12_BIT_DEPTH" },
	{ AlgoCameraType::CAM_10_BIT_DEPTH, "CAM_10_BIT_DEPTH" },
	{ AlgoCameraType::CAM_8_BIT_DEPTH, "CAM_8_BIT_DEPTH" },
	{ AlgoCameraType::CAM_C1200, "CAM_C1200" },
	{ AlgoCameraType::CAM_C3200, "CAM_C3200" },
	{ AlgoCameraType::CAM_TPM6100, "CAM_TPM6100" },
	{ AlgoCameraType::CAM_SC6100, "CAM_SC6100" },
	{ AlgoCameraType::CAM_TP2600, "CAM_TP2600" },
	{ AlgoCameraType::CAM_TP2000, "CAM_TP2000" },
	{ AlgoCameraType::CAM_MOBILE_MI11, "CAM_MOBILE_MI11" },
	{ AlgoCameraType::CAM_MER_2000_5GC, "CAM_MER_2000_5GC" },
	{ AlgoCameraType::CAM_MER_503_20GM, "CAM_MER_503_20GM" },
	{ AlgoCameraType::CAM_MER_503_20GM_DEPTH8, "CAM_MER_503_20GM_DEPTH8" },
})
/** 镜头型号，对应 CISDK LensType */
enum class AlgoLensType : int
{
	LENS_UNDEFINED   = 0,
	LENS_FIXED_35MM  = 35,
	LENS_FIXED_50MM  = 50,
	LENS_FIXED_80MM  = 80,
	LENS_FIXED_100MM = 100,
	LENS_FIXED_135MM = 135,
	LENS_FIXED_8MM   = 8,
};
NLOHMANN_JSON_SERIALIZE_ENUM(AlgoLensType, {
	{ AlgoLensType::LENS_UNDEFINED, "LENS_UNDEFINED" },
	{ AlgoLensType::LENS_FIXED_35MM, "LENS_FIXED_35MM" },
	{ AlgoLensType::LENS_FIXED_50MM, "LENS_FIXED_50MM" },
	{ AlgoLensType::LENS_FIXED_80MM, "LENS_FIXED_80MM" },
	{ AlgoLensType::LENS_FIXED_100MM, "LENS_FIXED_100MM" },
	{ AlgoLensType::LENS_FIXED_135MM, "LENS_FIXED_135MM" },
	{ AlgoLensType::LENS_FIXED_8MM, "LENS_FIXED_8MM" },
})

/** 相机滤片，对应 CISDK CamFilter */
enum class AlgoCamFilter : int
{
	FILTER_UNDEFINED = 0b00000000,
	FILTER_X         = 0b00000001,
	FILTER_Y         = 0b00000010,
	FILTER_Z         = 0b00000100,
	FILTER_DN        = 0b00001000,
	FILTER_XYZ       = 0b00000111,
};
NLOHMANN_JSON_SERIALIZE_ENUM(AlgoCamFilter, {
	{ AlgoCamFilter::FILTER_UNDEFINED, "FILTER_UNDEFINED" },
	{ AlgoCamFilter::FILTER_X, "FILTER_X" },
	{ AlgoCamFilter::FILTER_Y, "FILTER_Y" },
	{ AlgoCamFilter::FILTER_Z, "FILTER_Z" },
	{ AlgoCamFilter::FILTER_DN, "FILTER_DN" },
	{ AlgoCamFilter::FILTER_XYZ, "FILTER_XYZ" },
})

/** 打屏颜色，对应 CISDK AlgoColor */
enum class AlgoColor : int
{
	COLOR_BLACK = 0b00000000,
	COLOR_RED   = 0b00000100,
	COLOR_GREEN = 0b00000010,
	COLOR_BLUE  = 0b00000001,
	COLOR_WHITE = 0b00000111,
};
NLOHMANN_JSON_SERIALIZE_ENUM(AlgoColor, {
	{ AlgoColor::COLOR_BLACK, "COLOR_BLACK" },
	{ AlgoColor::COLOR_RED, "COLOR_RED" },
	{ AlgoColor::COLOR_GREEN, "COLOR_GREEN" },
	{ AlgoColor::COLOR_BLUE, "COLOR_BLUE" },
	{ AlgoColor::COLOR_WHITE, "COLOR_WHITE" },
})

/** LED 面板类型，对应 CISDK LedPanelType */
enum class AlgoLedPanelType : int
{
	LED_PANEL_UNDEFINED = 0,
	LED_PANEL_COB       = 1,
	LED_PANEL_SMD       = 2,
	LED_PANEL_COG       = 3,
};
NLOHMANN_JSON_SERIALIZE_ENUM(AlgoLedPanelType, {
	{ AlgoLedPanelType::LED_PANEL_UNDEFINED, "LED_PANEL_UNDEFINED" },
	{ AlgoLedPanelType::LED_PANEL_COB, "LED_PANEL_COB" },
	{ AlgoLedPanelType::LED_PANEL_SMD, "LED_PANEL_SMD" },
	{ AlgoLedPanelType::LED_PANEL_COG, "LED_PANEL_COG" },
})

/** LED 灯点排布，对应 CISDK LedLayout */
enum class AlgoLedLayout : int
{
	LED_LAYOUT_UNDEFINED    = 0b0000,
	LED_LAYOUT_RECTANGLE    = 0b0100,
	LED_LAYOUT_TRIANGLE_ROW = 0b0001,
	LED_LAYOUT_TRIANGLE_COL = 0b0010,
	LED_LAYOUT_TRIANGLE     = 0b0011,
};
NLOHMANN_JSON_SERIALIZE_ENUM(AlgoLedLayout, {
	{ AlgoLedLayout::LED_LAYOUT_UNDEFINED, "LED_LAYOUT_UNDEFINED" },
	{ AlgoLedLayout::LED_LAYOUT_RECTANGLE, "LED_LAYOUT_RECTANGLE" },
	{ AlgoLedLayout::LED_LAYOUT_TRIANGLE_ROW, "LED_LAYOUT_TRIANGLE_ROW" },
	{ AlgoLedLayout::LED_LAYOUT_TRIANGLE_COL, "LED_LAYOUT_TRIANGLE_COL" },
	{ AlgoLedLayout::LED_LAYOUT_TRIANGLE, "LED_LAYOUT_TRIANGLE" },
})

/** LED 亚像素排列，对应 CISDK LedSubpixelType */
enum class AlgoLedSubpixelType : int
{
	LED_SUBPIXEL_UNDEFINED = 0,
	LED_SUBPIXEL_SINGLE    = 1,
	LED_SUBPIXEL_RGB_STD   = 123,
};
NLOHMANN_JSON_SERIALIZE_ENUM(AlgoLedSubpixelType, {
	{ AlgoLedSubpixelType::LED_SUBPIXEL_UNDEFINED, "LED_SUBPIXEL_UNDEFINED" },
	{ AlgoLedSubpixelType::LED_SUBPIXEL_SINGLE, "LED_SUBPIXEL_SINGLE" },
	{ AlgoLedSubpixelType::LED_SUBPIXEL_RGB_STD, "LED_SUBPIXEL_RGB_STD" },
})

/** 隔点类型，对应 CISDK PatternType */
enum class AlgoPatternType : int
{
	PATTERN_UNDEFINED = 0,
	PATTERN_GRID      = 1,
};
NLOHMANN_JSON_SERIALIZE_ENUM(AlgoPatternType, {
	{ AlgoPatternType::PATTERN_UNDEFINED, "PATTERN_UNDEFINED" },
	{ AlgoPatternType::PATTERN_GRID, "PATTERN_GRID" },
})

/** 矩阵元素位深，对应 CISDK AlgoMatDepth */
enum class AlgoMatDepth : int
{
	ALGO_8U  = 0,
	ALGO_8S  = 1,
	ALGO_16U = 2,
	ALGO_16S = 3,
	ALGO_32S = 4,
	ALGO_32F = 5,
	ALGO_64F = 6,
};
NLOHMANN_JSON_SERIALIZE_ENUM(AlgoMatDepth, {
	{ AlgoMatDepth::ALGO_8U, "ALGO_8U" },
	{ AlgoMatDepth::ALGO_8S, "ALGO_8S" },
	{ AlgoMatDepth::ALGO_16U, "ALGO_16U" },
	{ AlgoMatDepth::ALGO_16S, "ALGO_16S" },
	{ AlgoMatDepth::ALGO_32S, "ALGO_32S" },
	{ AlgoMatDepth::ALGO_32F, "ALGO_32F" },
	{ AlgoMatDepth::ALGO_64F, "ALGO_64F" },
})
/** 矩阵通道与位深组合，对应 CISDK AlgoMatType */
enum class AlgoMatType : int
{
	ALGO_8UC1  = 0,
	ALGO_8SC1  = 1,
	ALGO_16UC1 = 2,
	ALGO_16SC1 = 3,
	ALGO_32SC1 = 4,
	ALGO_32FC1 = 5,
	ALGO_64FC1 = 6,
	ALGO_8UC2  = 8,
	ALGO_8SC2  = 9,
	ALGO_16UC2 = 10,
	ALGO_16SC2 = 11,
	ALGO_32SC2 = 12,
	ALGO_32FC2 = 13,
	ALGO_64FC2 = 14,
	ALGO_8UC3  = 16,
	ALGO_8SC3  = 17,
	ALGO_16UC3 = 18,
	ALGO_16SC3 = 19,
	ALGO_32SC3 = 20,
	ALGO_32FC3 = 21,
	ALGO_64FC3 = 22,
	ALGO_8UC4  = 24,
	ALGO_8SC4  = 25,
	ALGO_16UC4 = 26,
	ALGO_16SC4 = 27,
	ALGO_32SC4 = 28,
	ALGO_32FC4 = 29,
	ALGO_64FC4 = 30,
};
NLOHMANN_JSON_SERIALIZE_ENUM(AlgoMatType, {
	{ AlgoMatType::ALGO_8UC1, "ALGO_8UC1" },
	{ AlgoMatType::ALGO_8SC1, "ALGO_8SC1" },
	{ AlgoMatType::ALGO_16UC1, "ALGO_16UC1" },
	{ AlgoMatType::ALGO_16SC1, "ALGO_16SC1" },
	{ AlgoMatType::ALGO_32SC1, "ALGO_32SC1" },
	{ AlgoMatType::ALGO_32FC1, "ALGO_32FC1" },
	{ AlgoMatType::ALGO_64FC1, "ALGO_64FC1" },
	{ AlgoMatType::ALGO_8UC2, "ALGO_8UC2" },
	{ AlgoMatType::ALGO_8SC2, "ALGO_8SC2" },
	{ AlgoMatType::ALGO_16UC2, "ALGO_16UC2" },
	{ AlgoMatType::ALGO_16SC2, "ALGO_16SC2" },
	{ AlgoMatType::ALGO_32SC2, "ALGO_32SC2" },
	{ AlgoMatType::ALGO_32FC2, "ALGO_32FC2" },
	{ AlgoMatType::ALGO_64FC2, "ALGO_64FC2" },
	{ AlgoMatType::ALGO_8UC3, "ALGO_8UC3" },
	{ AlgoMatType::ALGO_8SC3, "ALGO_8SC3" },
	{ AlgoMatType::ALGO_16UC3, "ALGO_16UC3" },
	{ AlgoMatType::ALGO_16SC3, "ALGO_16SC3" },
	{ AlgoMatType::ALGO_32SC3, "ALGO_32SC3" },
	{ AlgoMatType::ALGO_32FC3, "ALGO_32FC3" },
	{ AlgoMatType::ALGO_64FC3, "ALGO_64FC3" },
	{ AlgoMatType::ALGO_8UC4, "ALGO_8UC4" },
	{ AlgoMatType::ALGO_8SC4, "ALGO_8SC4" },
	{ AlgoMatType::ALGO_16UC4, "ALGO_16UC4" },
	{ AlgoMatType::ALGO_16SC4, "ALGO_16SC4" },
	{ AlgoMatType::ALGO_32SC4, "ALGO_32SC4" },
	{ AlgoMatType::ALGO_32FC4, "ALGO_32FC4" },
	{ AlgoMatType::ALGO_64FC4, "ALGO_64FC4" },
})

/** 点定位隔点数据合并方法，对应 CISDK LedMeterAddMethod */
enum class AlgoLedMeterAddMethod : int
{
	LEDMETERADDMETHOD_ORI           = 0,
	LEDMETERADDMETHOD_NORMALIZATION = 1,
};
NLOHMANN_JSON_SERIALIZE_ENUM(AlgoLedMeterAddMethod, {
	{ AlgoLedMeterAddMethod::LEDMETERADDMETHOD_ORI, "LEDMETERADDMETHOD_ORI" },
	{ AlgoLedMeterAddMethod::LEDMETERADDMETHOD_NORMALIZATION, "LEDMETERADDMETHOD_NORMALIZATION" },
})

/** 设备运动路径，对应 CISDK AlgoLedMeterMotionPathType */
enum class AlgoLedMeterMotionPathType : int
{
	MOTION_PATH_S_HORIZONTAL_LEFT2RIGHT_FIRST = 0,
	MOTION_PATH_S_HORIZONTAL_RIGHT2LEFT_FIRST = 1,
	MOTION_PATH_I_HORIZONTAL_LEFT2RIGHT_FIRST = 2,
	MOTION_PATH_I_HORIZONTAL_RIGHT2LEFT_FIRST = 3,
	MOTION_PATH_S_VERTICAL_UP2DOWN_FIRST      = 4,
	MOTION_PATH_S_VERTICAL_DOWN2UP_FIRST      = 5,
	MOTION_PATH_I_VERTICAL_UP2DOWN_FIRST      = 6,
	MOTION_PATH_I_VERTICAL_DOWN2UP_FIRST      = 7,
};
NLOHMANN_JSON_SERIALIZE_ENUM(AlgoLedMeterMotionPathType, {
	{ AlgoLedMeterMotionPathType::MOTION_PATH_S_HORIZONTAL_LEFT2RIGHT_FIRST, "MOTION_PATH_S_HORIZONTAL_LEFT2RIGHT_FIRST" },
	{ AlgoLedMeterMotionPathType::MOTION_PATH_S_HORIZONTAL_RIGHT2LEFT_FIRST, "MOTION_PATH_S_HORIZONTAL_RIGHT2LEFT_FIRST" },
	{ AlgoLedMeterMotionPathType::MOTION_PATH_I_HORIZONTAL_LEFT2RIGHT_FIRST, "MOTION_PATH_I_HORIZONTAL_LEFT2RIGHT_FIRST" },
	{ AlgoLedMeterMotionPathType::MOTION_PATH_I_HORIZONTAL_RIGHT2LEFT_FIRST, "MOTION_PATH_I_HORIZONTAL_RIGHT2LEFT_FIRST" },
	{ AlgoLedMeterMotionPathType::MOTION_PATH_S_VERTICAL_UP2DOWN_FIRST, "MOTION_PATH_S_VERTICAL_UP2DOWN_FIRST" },
	{ AlgoLedMeterMotionPathType::MOTION_PATH_S_VERTICAL_DOWN2UP_FIRST, "MOTION_PATH_S_VERTICAL_DOWN2UP_FIRST" },
	{ AlgoLedMeterMotionPathType::MOTION_PATH_I_VERTICAL_UP2DOWN_FIRST, "MOTION_PATH_I_VERTICAL_UP2DOWN_FIRST" },
	{ AlgoLedMeterMotionPathType::MOTION_PATH_I_VERTICAL_DOWN2UP_FIRST, "MOTION_PATH_I_VERTICAL_DOWN2UP_FIRST" },
})

// --- 基础几何 / 色度 / 矩阵（对应 ci_data.h、algo_mat_data.h）---
/** 浮点二维坐标，对应 CISDK ::AlgoPoint2f */
struct AlgoPoint2f
{
	float x = 0.f; ///< 横坐标
	float y = 0.f; ///< 纵坐标
};

/** 整型二维坐标，对应 CISDK ::AlgoPoint */
struct AlgoPoint
{
	int x = 0; ///< 横坐标
	int y = 0; ///< 纵坐标
};

/** 整型矩形，对应 CISDK ::AlgoRect */
struct AlgoRect
{
	int x      = 0; ///< 左上角横坐标
	int y      = 0; ///< 左上角纵坐标
	int width  = 0; ///< 宽度
	int height = 0; ///< 高度
};

/** 浮点矩形，对应 CISDK ::AlgoRect2f */
struct AlgoRect2f
{
	float x      = 0.f; ///< 左上角横坐标
	float y      = 0.f; ///< 左上角纵坐标
	float width  = 0.f; ///< 宽度
	float height = 0.f; ///< 高度
};

/** 整型尺寸，对应 CISDK ::AlgoSize */
struct AlgoSize
{
	int width  = 0; ///< 宽度
	int height = 0; ///< 高度
};

/** 浮点尺寸，对应 CISDK ::AlgoSize2f */
struct AlgoSize2f
{
	float width  = 0.f; ///< 宽度
	float height = 0.f; ///< 高度
};

/** CIE Lab 色度分量，对应 CISDK TriLab */
struct AlgoTriLab
{
	float L = 0.f; ///< 亮度 L*
	float a = 0.f; ///< 色度 a*
	float b = 0.f; ///< 色度 b*
};

/** CIE Lch 色度分量，对应 CISDK TriLch */
struct AlgoTriLch
{
	float L = 0.f; ///< 亮度 L
	float c = 0.f; ///< 色度 c
	float h = 0.f; ///< 色相 h
};

/** CIE Lxy 色度分量，对应 CISDK TriLxy */
struct AlgoTriLxy
{
	float L = 0.f; ///< 亮度 L
	float x = 0.f; ///< 色度 x
	float y = 0.f; ///< 色度 y
};

/** CIE XYZ 色度分量，对应 CISDK TriXYZ */
struct AlgoTriXYZ
{
	float X = 0.f; ///< X 分量
	float Y = 0.f; ///< Y 分量
	float Z = 0.f; ///< Z 分量
};

/** RGB 色度分量，对应 CISDK TriRGB */
struct AlgoTriRGB
{
	float R = 0.f; ///< 红通道
	float G = 0.f; ///< 绿通道
	float B = 0.f; ///< 蓝通道
};

/** RGBW 四通道色度，对应 CISDK RGBW */
struct AlgoRGBW
{
	float R = 0.f; ///< 红通道
	float G = 0.f; ///< 绿通道
	float B = 0.f; ///< 蓝通道
	float W = 0.f; ///< 白通道
};

/** 三通道 Lxy 色度，对应 CISDK RGBLxy */
struct AlgoRGBLxy
{
	AlgoTriLxy R; ///< 红通道 Lxy
	AlgoTriLxy G; ///< 绿通道 Lxy
	AlgoTriLxy B; ///< 蓝通道 Lxy
};

/** 四通道 Lxy 色度，对应 CISDK RGBWLxy */
struct AlgoRGBWLxy
{
	AlgoTriLxy R; ///< 红通道 Lxy
	AlgoTriLxy G; ///< 绿通道 Lxy
	AlgoTriLxy B; ///< 蓝通道 Lxy
	AlgoTriLxy W; ///< 白通道 Lxy
};

/** 四通道 XYZ 色度，对应 CISDK RGBWXYZ */
struct AlgoRGBWXYZ
{
	AlgoTriXYZ R; ///< 红通道 XYZ
	AlgoTriXYZ G; ///< 绿通道 XYZ
	AlgoTriXYZ B; ///< 蓝通道 XYZ
	AlgoTriXYZ W; ///< 白通道 XYZ
};

/** 三通道 XYZ 色度，对应 CISDK RGBXYZ */
struct AlgoRGBXYZ
{
	AlgoTriXYZ R; ///< 红通道 XYZ
	AlgoTriXYZ G; ///< 绿通道 XYZ
	AlgoTriXYZ B; ///< 蓝通道 XYZ
};

/** 矩形四角浮点坐标（vision_algo 辅助，由 AlgoRect 推导） */
struct AlgoCornerPoint
{
	AlgoPoint2f p1; ///< 左上角
	AlgoPoint2f p2; ///< 右上角
	AlgoPoint2f p3; ///< 右下角
	AlgoPoint2f p4; ///< 左下角
};

/** 矩阵描述，对应 CISDK ::AlgoMat */
struct AlgoMat
{
	int            rows  = 0;                          ///< 矩阵行数
	int            cols  = 0;                          ///< 矩阵列数
	AlgoMatType    type  = AlgoMatType::ALGO_8UC1;     ///< 矩阵类型
	unsigned int   len   = 0;                          ///< 数据字节数（depth_in_byte * rows * cols）
	unsigned char *data  = nullptr;                    ///< 矩阵元素指针
};

// --- 复合 / 业务 Param ---
/** 设备信息，对应 CISDK Device */
struct AlgoDevice
{
	std::string    device_id;                              ///< 设备 id（Device::id）
	std::string    lens_id;                                ///< 镜头编号（Device::lens_id）
	AlgoCameraType cam_type  = AlgoCameraType::CAM_UNDEFINED; ///< 相机类型（Device::cam_type）
	AlgoLensType   lens_type = AlgoLensType::LENS_UNDEFINED;   ///< 镜头类型（Device::lens_type）
};

/** 拍摄参数，对应 CISDK Capture */
struct AlgoCapture
{
	int           exposure   = 0;                                      ///< 曝光时间 ms（Capture::exposure）
	int           gain       = 0;                                      ///< 相机增益（Capture::gain）
	int           aperture   = 0;                                      ///< 光圈（Capture::aperture）
	int           microfocus = 0;                                      ///< 微焦，单位 mm（Capture::microfocus）
	AlgoCamFilter filter     = AlgoCamFilter::FILTER_UNDEFINED;      ///< XYZ 滤片（Capture::filter）
};

/** 打屏层信息，对应 CISDK Layer */
struct AlgoLayer
{
	int   index        = 0;   ///< layer 索引（Layer::index）
	float linear_norms = 0.f; ///< 多层归一化线性亮度百分比 0-1（Layer::linear_norms）
	int   value        = 0;   ///< 打屏亮度（Layer::value）
};

/** LED 面板信息，对应 CISDK LedPanel */
struct AlgoLedPanel
{
	AlgoLedPanelType    type          = AlgoLedPanelType::LED_PANEL_UNDEFINED;       ///< 面板类型（LedPanel::type）
	AlgoLedLayout       layout        = AlgoLedLayout::LED_LAYOUT_UNDEFINED;         ///< 灯点排布（LedPanel::layout）
	AlgoLedSubpixelType subpixel_type = AlgoLedSubpixelType::LED_SUBPIXEL_UNDEFINED; ///< 亚像素排列（LedPanel::subpixel_type）
	AlgoSize            wh;                                                          ///< 灯板大小（LedPanel::wh）
	float               pixel_pitch = 0.f;                                           ///< 灯点间距 mm（LedPanel::pixel_pitch）
};

/** 隔点打屏信息，对应 CISDK Pattern */
struct AlgoPattern
{
	AlgoPatternType type = AlgoPatternType::PATTERN_UNDEFINED; ///< 隔点类型（Pattern::type）
	AlgoSize        interval;                                  ///< 隔点宽高（Pattern::interval）
	AlgoPoint       loc;                                       ///< 点亮隔点中的行列位置（Pattern::loc）
	AlgoSize        lighted_led_wh;                            ///< 点亮灯点的行数与列数（Pattern::lighted_led_wh）
};

/** 分区信息，对应 CISDK Partition */
struct AlgoPartition
{
	AlgoRect xywh;       ///< 分区矩形（Partition::xywh）
	AlgoSize screen_wh;  ///< 屏幕大小（Partition::screen_wh）
};

/** 单次拍摄数据，对应 CISDK ::AlgoShot（含堆分配子对象，仅可移动） */
struct AlgoShot
{
	AlgoPartition *part    = nullptr; ///< 分区信息（AlgoShot::part）
	AlgoLedPanel  *panel   = nullptr; ///< 面板信息（AlgoShot::panel）
	AlgoLayer     *layer   = nullptr; ///< 层信息（AlgoShot::layer）
	AlgoPattern   *pattern = nullptr; ///< 打点信息（AlgoShot::pattern）
	AlgoColor     *color   = nullptr; ///< 打屏颜色（AlgoShot::color）
	AlgoDevice    *device  = nullptr; ///< 设备信息（AlgoShot::device）
	AlgoCapture   *capture = nullptr; ///< 拍摄信息（AlgoShot::capture）

	AlgoRect    rect;                                    ///< 图像位置（AlgoShot::rect）
	AlgoCIEType type = AlgoCIEType::CIE_UNDEFINED;       ///< 色度分量类型（AlgoShot::type）
	AlgoMat     image;                                   ///< 图像数据，通常 ALGO_16UC1（AlgoShot::image）

	AlgoShot() = default;
	~AlgoShot();
	AlgoShot(AlgoShot &&other) noexcept;
	AlgoShot &operator=(AlgoShot &&other) noexcept;
	AlgoShot(const AlgoShot &)            = delete;
	AlgoShot &operator=(const AlgoShot &) = delete;
};

/** 灯点计量数据，对应 CISDK ::AlgoLedMeter（含堆分配子对象，仅可移动） */
struct AlgoLedMeter
{
	AlgoPartition *part    = nullptr; ///< 分区信息（AlgoLedMeter::part）
	AlgoLedPanel  *panel   = nullptr; ///< 面板信息（AlgoLedMeter::panel）
	AlgoLayer     *layer   = nullptr; ///< 层信息（AlgoLedMeter::layer）
	AlgoCapture   *capture = nullptr; ///< 拍摄信息（AlgoLedMeter::capture）

	AlgoColor   color = AlgoColor::COLOR_BLACK;          ///< 颜色（AlgoLedMeter::color）
	AlgoCIEType type  = AlgoCIEType::CIE_UNDEFINED;      ///< CIE/相机色度类型（AlgoLedMeter::type）
	AlgoMat     state;                                   ///< 灯点状态 ALGO_8UC1（AlgoLedMeter::state）
	AlgoMat     pos;                                     ///< 亚像素位置 ALGO_32FC2（AlgoLedMeter::pos）
	AlgoMat     chroma;                                  ///< 色度分量 ALGO_32FC1（AlgoLedMeter::chroma）

	AlgoLedMeter() = default;
	~AlgoLedMeter();
	AlgoLedMeter(AlgoLedMeter &&other) noexcept;
	AlgoLedMeter &operator=(AlgoLedMeter &&other) noexcept;
	AlgoLedMeter(const AlgoLedMeter &)            = delete;
	AlgoLedMeter &operator=(const AlgoLedMeter &) = delete;
};

/** 隔点数据合并参数，对应 CISDK AlgoLedMeterMergeInfo */
struct AlgoLedMeterMergeInfo
{
	AlgoLedMeterMotionPathType motion_path =
		AlgoLedMeterMotionPathType::MOTION_PATH_S_HORIZONTAL_LEFT2RIGHT_FIRST; ///< 设备运动路径
	int      horizontal_shot_num    = 1; ///< 水平方向总拍摄次数
	int      vertical_shot_num      = 1; ///< 垂直方向总拍摄次数
	int      horizontal_overlap_num = 0; ///< 水平方向重叠灯珠数
	int      vertical_overlap_num   = 0; ///< 垂直方向重叠灯珠数
	AlgoSize led_wh;                       ///< 整张灯板灯点数 w×h（列×行）
};

} // namespace vision_algo