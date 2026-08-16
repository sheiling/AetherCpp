#ifndef __CAMERA_CMF_CALIB_DATA_H__
#define __CAMERA_CMF_CALIB_DATA_H__

#include "meta_info_data.h"
#include "algo_mat_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	typedef struct
	{
		float wid_range_r[3] = { 3.0f, 1.0f, 30.0f };
		float cur_range_r[3] = { 50.0f, 5.0f, 50.0f };
		float wav_range_r[3] = { 610.0f, 1.0f, 640.0f };
		float wid_range_g[3] = { 15.0f, 1.0f, 34.0f };
		float cur_range_g[3] = { 3.0f, 2.0f, 9.0f };
		float wav_range_g[3] = { 500.0f, 1.0f, 560.0f };
		float wid_range_b[3] = { 8.0f, 1.0f, 27.0f };
		float cur_range_b[3] = { 3.0f, 2.0f, 9.0f };
		float wav_range_b[3] = { 430.0f, 1.0f, 490.0f };

	}CmfCalibGenParam;


#ifdef __cplusplus
}
#endif // __cplusplus
#endif