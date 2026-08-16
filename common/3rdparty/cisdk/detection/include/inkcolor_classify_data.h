#ifndef __INKCOLOR_CLASSIFY_DATA_H__
#define __INKCOLOR_CLASSIFY_DATA_H__

#include "algo_mat_data.h"
#include "meta_info_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	typedef enum
	{
		CS_CAM_RGB = 0,
		CS_CIE_Lch = 1,
		CS_CIE_Lab = 2,
		CS_CIE_XYZ = 3,
	}ColorSpace; //需要计算的颜色类型

	typedef enum
	{
		ON_CALIBRARE = 0,   // 打开补偿
		OFF_CALIBRARE = 1,  // 关闭补偿
	}LightCalibrate; //是否进行光源补偿

	typedef enum
	{
		ON_MONIT = 0,   // 打开补偿
		OFF_MONIT = 1,  // 关闭补偿
	}MonitPositionFlag;  // 相机位置监控开关。 


	typedef enum
	{
		MATRIX_CALIB = 0,  // XYZ矩阵计算补偿
		RATIO_CALIB = 1,   // XYZ比率计算补偿
	}DevCalibrateMethod;


	 //变量缩合之后的值
	typedef struct
	{
		float brightness;  //  L / XYZ空间的 Y  /R   // 变量小写
		float color1;  // c / a  / X  /G
		float color2; // h / b  /  z  /B
		//float ReflectControl;
		float panel_blance;
	}InkColorFeature;



#ifdef __cplusplus
}
#endif // __cplusplus
#endif