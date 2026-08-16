#ifndef __THERMAL_MODEL_EMULATOR_DATA_H__
#define __THERMAL_MODEL_EMULATOR_DATA_H__

#include "ci_data.h"
#include "algo_mat.h"
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


	typedef struct
	{
		int cabinet_width;                     // 单个箱体的宽
		int cabinet_height;                    // 单个箱体的高
		AlgoMat cabinet_gamma_table;           // 箱体的gamma表
		float sampling_interval;               // 热力图采样间隔
		int full_screen_cabinet_row;           // 全屏中箱体行数
		int full_screen_cabinet_col;		   // 全屏中箱体列数
		int panel_width;                       // 单个灯板的宽
		int panel_height;                      // 单个灯板的高
	}ThermalModelParameter;


	// 校正类型
	typedef enum
	{
		NO_CALIBRATION = 0,                // 不做校正
		MODULE_CALIBRATION = 1,            // 模组校正
		CABINET_CALIBRATION = 2,           // 箱体校正
		FULL_SCREEN_CALIBRARTION = 3,      // 全屏校正
	}ThermalCalibType;

	// 升温过程状态
	typedef enum
	{
		STATE_INITIAL = 0,    // 初始状态
		STATE_HEATING = 1,    // 升温状态
		STATE_BALANCE = 2     // 平衡状态
	}ThermalProcessState;

	// 箱体温度的不均匀性
	typedef struct
	{
		// 箱体温度极差
		float cabinet_temp_range;
		// 模组的平均温度离散系数
		float panel_temp_avg_coef_var;
		// 箱体的温度离散系数
		float cabinet_temp_coef_var;
	}TempeatureNonuniformity;

	// 箱体升温的不均匀性 
	typedef struct
	{
		// 箱体最小升温
		float min_temp_delta;
		// 箱体的平均升温
		float avg_temp_delta;
		// 箱体的最大升温
		float max_temp_delta;
		// 箱体升温的离散系数
		float delta_temp_coef_var;
	}DeltaTempeatureNonuniformity;


	typedef struct
	{
		// 当前状态下箱体的亮度极差
		float current_cabinet_bright_range;
		// 当前状态下箱体的亮度的离散系数
		float current_cabinet_bright_coef_var;
		// 全局最大亮度衰减，当前状态相比于初始状态衰减最大点的亮度衰减
		float global_max_bright_decay;
		// 全局平均亮度衰减，当前状态相比于初始状态平均每个点的亮度衰减
		float global_avg_bright_decay;
		// 当前状态下温度最高点和最低点的色度偏差
		AlgoPoint2f current_max_chromatic_aberration;
		// 全局情况下最大色偏点的色偏，当前状态相比于初始状态最大的色偏
		AlgoPoint2f global_max_chromatic_aberration;
		// 全局情况下每个点的平均色偏，当前状态相比于初始状态平均每个点的色偏
		AlgoPoint2f global_avg_chromatic_aberration;

	}LuminanceNonuniformity;

#ifdef __cplusplus
}
#endif __cplusplus
#endif