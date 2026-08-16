#ifndef __CAMERA_LUMINANCE_CALIBRATION_DATA_H__
#define __CAMERA_LUMINANCE_CALIBRATION_DATA_H__

#include "algo_mat_data.h"
#include "meta_info_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	typedef struct
	{
		float calibcoeff;
		float luminance;
		float ROIx;
		float ROIy;
		float ROIw;
		float ROIh;
	}
	AlgoCameraLuminanceCalibration;

	typedef struct
	{
		float calibcoeff = 0;
		float apert = 0;
		float microfocus = 0;
		char* color = "";
		CIEType filter = CIE_UNDEFINED;
		float imgarearatio[9] = {0};
	}AlgoCameraLuminanceCalibData;

	typedef struct
	{
		float calibcoeff[3] = { 0 };
		float apert[3] = { 0 };
		float microfocus[3] = { 0 };
		char* color[3] = { "" };
		CIEType filter[3] = { CIE_UNDEFINED };
		float imgarearatio[3][9] = { 0 };
	}AlgoCameraLuminanceCalibchromaData;


#ifdef __cplusplus
}
#endif // __cplusplus
#endif