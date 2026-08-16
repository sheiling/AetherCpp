#ifndef __INKCOLOR_CALIBRATION_DATA_H__
#define __INKCOLOR_CALIBRATION_DATA_H__

#include "algo_mat_data.h"
#include "meta_info_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	typedef struct
	{
		float V1;  //  L / XYZ空间的 Y  /R   // 变量小写
		float V2;  // c / a  / X  /G
		float V3; // h / b  /  z  /B
	}MeasureVal;


	typedef enum
	{
		ChromaCalibration = 0,
		LightCalibration = 1,
		SystemCalibration = 2,
	}CalibrationMethod;

	typedef struct 
	{
		AlgoSize img_wh;
		int bit_depth;
	}ImgInfo;
	
#ifdef __cplusplus
}
#endif // __cplusplus
#endif