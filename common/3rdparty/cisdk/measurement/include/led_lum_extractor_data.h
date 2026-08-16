#ifndef __LED_LUM_EXTRACTOR_DATA_H__
#define __LED_LUM_EXTRACTOR_DATA_H__

#include "meta_info_data.h"
#include "algo_mat_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

    typedef struct
    {
        AlgoMat state;//灯点状态 ALGO_8UC1
        AlgoMat lum;//灯点光通量 ALGO_32FC1
    }AlgoLedLum;
	typedef enum 
	{
		LedLumMethod_SUM_METHOD = 0, //使用原点定位求和方法
		LedLumMethod_MEAN_METHOD = 1, //使用超分求均值方法
	}LedLumMethod;
	typedef enum
	{
		ContourRetrieveMethod_MASK_METHOD = 0,		// 使用mask确认搜索框区域的范围
		ContourRetrieveMethod_GEOMETRY_METHOD = 1,	// 基于几何学判断某像素是否位于搜索框内，目前仅适用于异形搜索框角点数量为4的情况
	}ContourRetrieveMethod;  // 框检索方法，指的是提取光通量的时候用什么方法确认某个像素是否在搜索框内，目前仅针对异形搜索框ir_contour



#ifdef __cplusplus
}
#endif // __cplusplus

#endif // 
