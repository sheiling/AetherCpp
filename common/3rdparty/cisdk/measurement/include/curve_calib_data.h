#ifndef __CURVE_CALIB_DATA_H__
#define __CURVE_CALIB_DATA_H__

#include "meta_info_data.h"
#include "algo_mat_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

    typedef struct
    {
        AlgoColor color;//颜色
        AlgoMat x_data;//不同分量的标定曲面信息 ALGO_32FC1
        AlgoMat y_data;
        AlgoMat z_data;
    }AlgoCurveCalib;

	enum CurveCalibFitingMethod
	{
		//POLYFIT_2 =2,
		//POLYFIT_3 = 3,
		POLYFIT_2_3 = 5,  //高低阶拟合
		POLYFIT_BLOCK = 6,	//block拉齐亮度后高低阶拟合
		POLYFIT_FILTER = 7,	//滤波曲面
		POLYFIT_AVERAGEPULL_Adaptive_3 = 8,   //均值拉齐后，3阶迭代拟合
		POLYFIT_AVERAGEPULL_Adaptive_4 = 9,   //均值拉齐后，4阶迭代拟合
	};

#ifdef __cplusplus
}
#endif // __cplusplus
#endif