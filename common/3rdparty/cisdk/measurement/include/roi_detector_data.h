#ifndef __ROI_DETECTOR_DATA_H__
#define __ROI_DETECTOR_DATA_H__

#include "ci_data.h"
#include "algo_mat_data.h"
#include "ci_measurement_enum.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


    typedef struct 
	{
        AlgoSize img_wh;//图像大小
        AlgoRect xywh; //区域
        AlgoMat mask; //ALGO_8UC1
    }AlgoROI;

	typedef enum
	{
		MASKFLAG_CONVEX_HULL = 0,			// 凸包补全的mask，适用于矩形屏体，该模块默认的mask类型
		MASKFLAG_TRUE_SHAPE = 1,			// 真实形状的mask，适用于异形屏
	} MaskFlag;			// 屏体区域掩膜的MASK类别

	typedef enum
	{
		ROI_FLAG_FAST                  = 0b10000000,
		ROI_FLAG_NORMAL                = 0b00000000,                //常规算法
        ROI_FLAG_HIGH_PRECISION        = 0b00010000,//0x10//16      //普通业务场景高精度（代替快速算法）

		ROI_FLAG_LARGE_INTERVAL_LED    = 0b00000001,//0x01//1       //大间隔场景
		ROI_FLAG_UNEVEN_GRAY_SCALE     = 0b00000010,//0x02//2       //灰度不均匀场景
		ROI_FLAG_AMBIENT_LIGHT         = 0b00000100,//0x04//4       //干扰光场景
		ROI_FLAG_UNEVEN_GRAY_AMBIENT   = 0b00001000,//0x08//8       //灰度不均匀+干扰光场景

		ROI_FLAG_LARGE_INTERVAL_HIGH_PRECISION = ROI_FLAG_LARGE_INTERVAL_LED | ROI_FLAG_HIGH_PRECISION,//大间隔+高精度
		ROI_FLAG_UNEVEN_GRAY_HIGH_PRECISION = ROI_FLAG_UNEVEN_GRAY_SCALE | ROI_FLAG_HIGH_PRECISION,//灰度不均匀+高精度
		ROI_FLAG_AMBIENT_LIGHT_HIGH_PRECISION = ROI_FLAG_AMBIENT_LIGHT | ROI_FLAG_HIGH_PRECISION,	//干扰光+高精度
		ROI_FLAG_AMBIENT_UNEVEN_HIGH_PRECISION = ROI_FLAG_UNEVEN_GRAY_AMBIENT | ROI_FLAG_HIGH_PRECISION,	//灰度不均匀+干扰光+高精度

	}RoiFlag;
	

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // 
