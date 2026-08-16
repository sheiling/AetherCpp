#ifndef __COLORIMETER_CALIB_DATA_H__
#define __COLORIMETER_CALIB_DATA_H__

#include "meta_info_data.h"
#include "algo_mat_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
	typedef enum 
	{
		COLORIMETERCALIB_MEASURE = 0,		//色度计标定：标定测量端， 即将两个色度计标定到一致
		COLORIMETERCALIB_TARGET = 1,		//色度计标定：标定最终结果，即通过修改最终的目标而保证两个色度计起到一样效果的作用。
		COLORIMETERCALIB_MULTICAMERA_AFTERCALIBRATION = 2,  //相机多光道标定，校正后标定，系数上屏后采集数据进行的标定
		COLORIMETERCALIB_MULTICAMERA_ORIGINALDATA = 3      //相机多光道标定，标定原始值，利用校正过程中的数据进行标定（无需额外的采集操作）
	}ColorimeterCalibType;

	typedef struct
	{
		AlgoColor color;//颜色
		AlgoMat x_data;//不同分量的标定系数 ALGO_32FC1
		AlgoMat y_data;
		AlgoMat z_data;
		int colorimeter_calib_type;
	}AlgoColorimeterCalib;

	/*typedef struct
	{
		Capture 
		Device
	}ID;*/

#ifdef __cplusplus
}
#endif // __cplusplus
#endif