#ifndef __CMF_CALIB_DATA_H__
#define __CMF_CALIB_DATA_H__

#include "meta_info_data.h"
#include "algo_mat_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	enum CMFStandard
	{
		CMF1931 = 0,  // use 1931 2°observer CMF 
		CMF1964 = 1,  // use 1964 10°observer CMF
		CMF2006_2 = 2, // use 2006 2°observer CMF
		CMF2006_10 = 3,  // use 2006 10°observer CMF
		CMF2012_0 = 4, // use 2012 0°observer CMF
		CMF2012_1 = 5,  // use 2012 1°observer CMF
		CMF2012_2 = 6, // use 2012 2°observer CMF
		CMF2012_3 = 7, // use 2012 3°observer CMF
		CMF2012_4 = 8, // use 2012 4°observer CMF
		CMF2012_5 = 9, // use 2012 5°observer CMF
		CMF2012_6 = 10, // use 2012 6°observer CMF
		CMF2012_7 = 11, // use 2012 7°observer CMF
		CMF2012_8 = 12, // use 2012 8°observer CMF
		CMF2012_9 = 13, // use 2012 9°observer CMF
		CMF2012_10 = 14, // use 2012 10°observer CMF
		CMF2012_m = 15,  // use mean of 2012 0°to 10°observer CMF
		CMFnew1 = 16,  // use 2012 observer CMF1
		CMFnew2 = 17,  // use 2012 observer CMF2
		CMFnew3 = 18,  // use 2012 observer CMF3
		CMFnew4 = 19,  // use 2012 observer CMF4
		CMFnew5 = 20,  // use 2012 observer CMF5
		CMF2012_0_0 = 21, // use 2012 0.0°observer CMF
		CMF2012_0_1 = 22,  // use 2012 0.1°observer CMF
		CMF2012_0_2 = 23, // use 2012 0.2°observer CMF
		CMF2012_0_3 = 24, // use 2012 0.3°observer CMF
		CMF2012_0_4 = 25, // use 2012 0.4°observer CMF
		CMF2012_0_5 = 26, // use 2012 0.5°observer CMF
		CMF2012_0_65 = 27, // use 2012 0.65°observer CMF
		CMF2012_0_8 = 28, // use 2012 0.8°observer CMF
		CMF2012_1_0 = 29, // use 2012 1.0°observer CMF
		CMF2012_1_2 = 30, // use 2012 1.2°observer CMF
		CMF2012_1_45 = 31, // use 2012 1.45°observer CMF
		CMF2012_1_7 = 32, // use 2012 1.7°observer CMF
		CMF2012_2_0 = 33, // use 2012 2°observer CMF
		CMF2012_n1 = 34, // use 2012 -1°observer CMF
		CMF2012_n0_7 = 35, // use 2012 -0.7°observer CMF
		CMF2012_n0_5 = 36, // use 2012 -0.5°observer CMF
		CMF2012_n0_3 = 37, // use 2012 -0.3°observer CMF
		CMF2012_n0_1 = 38, // use 2012 -0.1°observer CMF
		CMF2012_n0_3n = 39, // use 2012 -0.3°observer CMF 人为调整
		CMF2012_n0_5n = 40, // use 2012 -0.5°observer CMF 人为调整
	};

	enum SpectraParamType				//生成的光谱参数类型
	{
		PARAM_UNDEFINE = 0,		//未定义
		HALF_WIDTH = 1,			//半波宽
		DOMINANT_WAVE = 2,		//主波长
		CURVE_RATIO = 3			//曲率
	};

	typedef struct
	{
		AlgoColor color;				//颜色
		SpectraParamType type;			//光谱参数类型
		float min;						//左区间
		float max;						//右区间
		float step;						//光谱步长
	} CamCmfGenParam;



#ifdef __cplusplus
}
#endif // __cplusplus
#endif