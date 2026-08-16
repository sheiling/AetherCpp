#ifndef __DISPLAY_UNIFORMITY_SIMULATOR_DATA_H__
#define __DISPLAY_UNIFORMITY_SIMULATOR_DATA_H__

#include "ci_data.h"
#include "algo_mat.h"
#include "ci_define.h"
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
	typedef enum
	{
		REC_709 = 0,                         // Rec.709色域 
		DISPLAY_P3 = 1,                      // Display P3 色域
		ADOBE_RGB = 2,                       // Adobe_RGB 色域
		REC_2020 = 3,                        // Rec.2020 色域
		CUSTOM = 10,                         // 定制色域
	}DisplayTargetGamut; // 目标显示色域

	typedef enum
	{
		THERMAL_CALIB_V1 = 0,                // 热力校正1.0版本(任意温度使用同一校正系数) 
		THERMAL_CALIB_V2 = 1,                // 热力校正2.0版本(手动调节拉杆调整校正系数) 
		THERMAL_CALIB_V3 = 2,                // 热力校正3.0版本(自适应进行温度补偿) 

	}DisplayThermalCalibMethod; // 热力校正版本


	typedef enum
	{
		INDIVIDUAL_STATS,         // 各自独立均值
		GLOBAL_STATS,             // 全局混合均值
		FIXED_TARGET              // 使用固定目标值
	}DisplayChromaEnhanceMethod;

#ifdef __cplusplus
}
#endif __cplusplus
#endif