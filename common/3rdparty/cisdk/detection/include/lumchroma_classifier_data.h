#ifndef __LUMCHROMA_CLASSIFIER_DATA_H__
#define __LUMCHROMA_CLASSIFIER_DATA_H__
#include "algo_mat_data.h"
#include "meta_info_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
	//1.颜色空间计算类型
	typedef enum
	{
		CS_CAM_RGB = 0,
		CS_CIE_Lch = 1,
		CS_CIE_Lab = 2,
		CS_CIE_XYZ = 3,
	}ColorSpace; //需要计算的颜色类型
	//2.表征量-输出使用
	typedef struct
	{
		float lum_main;//正面主相机亮度表征量
		float lum_side_1;//侧面相机1亮度表征量
		float lum_side_2;//侧面相机2亮度表征量
		float chroma_main;//主相机偏色表征量
		float chroma_side_1;//侧面相机1偏色表征量
		float chroma_side_2;//侧面相机2偏色表征量

	}Colorcast;
	//3.分类阈值
	typedef struct
	{
		//软件分类框参数
		float thr_col_num;//分类框列数
		float thr_row_num;//分类框行数
		float thr_left_value;//分类框最左值-对应side_1
		float thr_right_value;//分类框最右值-对应side_1
		float thr_bottom_value;//分类框最低值-对应side_2
		float thr_top_value;//分类框最高值-对应side_2
		//侧面相机亮度阈值参数
		float thr_lum_sideMin_1;//侧面相机1最小亮度阈值
		float thr_lum_sideMax_1;//侧面相机1最大亮度阈值
		float thr_lum_sideMin_2;//侧面相机2最小亮度阈值
		float thr_lum_sideMax_2;//侧面相机2最大亮度阈值
		float thr_lum_side_deviation;//侧面相机12亮度偏差
		//侧面相机色偏度阈值参数
		float thr_chroma_sideMin_1;//侧面相机1最小色偏度阈值
		float thr_chroma_sideMax_1;//侧面相机1最大色偏度阈值
		float thr_chroma_sideMin_2;//侧面相机2最小色偏度阈值
		float thr_chroma_sideMax_2;//侧面相机2最小色偏度阈值
		float thr_chroma_side_deviation;//侧面相机12色偏度偏差
		//正面主相机亮度色偏度阈值参数
		float thr_lum_mainMin;//正面主相机最小亮度阈值
		float thr_lum_mainMax;//正面主相机最大亮度阈值
		float thr_chroma_mainMin;//正面主相机最小色偏度阈值
		float thr_chroma_mainMax;//正面主相机最大色偏度阈值
		//正面与侧面相机偏差阈值
		float thr_lum_mainAndside1_deviation;//正面主相机与侧面相机1亮度偏差
		float thr_lum_mainAndside2_deviation;//正面主相机与侧面相机2亮度偏差
		float thr_chroma_mainAndside1_deviation;//正面主相机与侧面相机1色偏度偏差
		float thr_chroma_mainAndside2_deviation;//正面主相机与侧面相机2色偏度偏差

	}LumChromaClassifyThreshold;

#ifdef __cplusplus
}
#endif // __cplusplus
#endif