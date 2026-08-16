#ifndef __SURFACE_TEMPERATURE_CALIB_DATA_H__
#define __SURFACE_TEMPERATURE_CALIB_DATA_H__

#include "meta_info_data.h"
#include "algo_mat_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
	enum SurfaceTempCalibType
	{
		SurfaceTempCalibType_Point = 0,  //逐点标定系数（默认模式--箱体自动线方案）
		SurfaceTempCalibType_Global = 1,  //Global标定系数
		SurfaceTempCalibType_Cabinet = 2,  //单箱复用标定系数（全屏推荐方案）
		SurfaceTempCalibType_Module = 3,  //单模组复用标定系数
	};
    typedef struct
    {
        AlgoColor color;//颜色
		AlgoMat x_data;//X滤片对应系数
		AlgoMat y_data;//Y滤片对应系数
		AlgoMat z_data;//Z滤片对应系数
		AlgoMat tar_temperature;//校正基准温度
		SurfaceTempCalibType calib_type;//系数标定模式
    }AlgoSurfTempCalib;
	enum SurfaceTempRectifyType
	{
		SurfaceTempRectifyType_UnchangedChroma = 0,  //保持色坐标不变
		SurfaceTempRectifyType_ChangedChroma = 1,  //色坐标改变
	};
	
	
#ifdef __cplusplus
}
#endif // __cplusplus
#endif