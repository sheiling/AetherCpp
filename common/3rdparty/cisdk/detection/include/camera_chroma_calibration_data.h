#ifndef __CAMERA_CHROMA_CALIBRATION_DATA_H__
#define __CAMERA_CHROMA_CALIBRATION_DATA_H__
#include "algo_mat_data.h"
#include "meta_info_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	typedef struct
	{
		Device device_info;//设备信息
		AlgoMat chroma_calib_matrix;//颜色转换矩阵:ALGO_32FC1
	}AlgoCamChromaCalib;

#ifdef __cplusplus
}
#endif // __cplusplus
#endif