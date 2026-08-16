#ifndef __CAMERA_VIGNETTING_CALIBRATION_DATA_H__
#define __CAMERA_VIGNETTING_CALIBRATION_DATA_H__
#include "algo_mat_data.h"
#include "meta_info_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	typedef struct
	{
		//设备信息
		Device device_info;
		//标定矩阵
		AlgoMat r_x_data;//ALGO_32FC1
		AlgoMat g_y_data;
		AlgoMat b_z_data;
		AlgoMat gray_data;
		//黑图
		AlgoMat bg_data;
		//标定区域
		AlgoRect roi;
	} AlgoCamVigCalib;


#ifdef __cplusplus
}
#endif // __cplusplus
#endif