#ifndef __MAPPER_UTILS_DATA_H__
#define __MAPPER_UTILS_DATA_H__


#include "meta_info_data.h"
#include "algo_mat_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


	// 灯点偏移预测方法
	typedef enum
	{
		POINTIRMAP_OFFSET_PREDICT_POINT = 0b00000000, // 逐灯点计算偏移
		POINTIRMAP_OFFSET_PREDICT_PERSPECTIVE = 0b00000001, // 角点计算透视变换方法
	} OffsetPredictMethod;

	// 隔点灯点补全方法
	typedef enum
	{
		LOCAL_LINEAR_FIT = 0,	// 相邻灯点直线插值补全
		LOCAL_SPLINE_FIT = 1,   // 相邻灯点样条曲线插值补全
	} InterpolationMethod;     


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // 
