#ifndef __CAMERA_DISTORTION_CALIBRATION_DATA_H__
#define __CAMERA_DISTORTION_CALIBRATION_DATA_H__
#include "algo_mat_data.h"
#include "meta_info_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	typedef struct
	{
		//设备信息
		Device device_info;
		//畸变标定矩阵
		AlgoMat dist_calib_matrix; //畸变标定矩阵：ALGO_32FC1
		AlgoMat dist_coeff_matrix; //畸变系数矩阵：ALGO_32FC1

	} AlgoCamDistCalib;


#ifdef __cplusplus
}
#endif // __cplusplus
#endif