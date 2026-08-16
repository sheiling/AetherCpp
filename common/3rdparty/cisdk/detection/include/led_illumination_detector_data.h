#ifndef __LED_ILLUMINATION_DETECTOR_DATA_H__
#define __LED_ILLUMINATION_DETECTOR_DATA_H__

#include "ci_data.h"
#include "algo_mat.h"
#include "led_meter_inspector_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	typedef struct 
	{
		//死灯阈值
		float b_dead_th;         //蓝色画面下，死灯阈值。含义：与正常灯的对比值，范围(0,1),若设置为0.4f,则代表亮度低于正常灯的40%才会被判定为死灯
		float g_dead_th;         //绿色画面下，死灯阈值。含义：与正常灯的对比值，范围(0,1),若设置为0.4f,则代表亮度低于正常灯的40%才会被判定为死灯
		float r_dead_th;         //红色画面下，死灯阈值。含义：与正常灯的对比值，范围(0,1),若设置为0.4f,则代表亮度低于正常灯的40%才会被判定为死灯
		//暗灯阈值			      
		float b_dim_th;		     //蓝色画面下，暗灯阈值。含义：与正常灯的对比值，范围(0,1),若设置为0.8f,则代表亮度低于正常灯的80%才会被判定为暗灯
		float g_dim_th;		     //绿色画面下，暗灯阈值。含义：与正常灯的对比值，范围(0,1),若设置为0.8f,则代表亮度低于正常灯的80%才会被判定为暗灯
		float r_dim_th;		     //红色画面下，暗灯阈值。含义：与正常灯的对比值，范围(0,1),若设置为0.8f,则代表亮度低于正常灯的80%才会被判定为暗灯
		//亮灯阈值			      
		float b_bright_th;       //蓝色画面下，亮灯阈值。含义：与正常灯的对比值，范围(1,2),若设置为1.2f,则代表亮度高于正常灯的112%才会被判定为亮灯
		float g_bright_th;	     //绿色画面下，亮灯阈值。含义：与正常灯的对比值，范围(1,2),若设置为1.2f,则代表亮度高于正常灯的112%才会被判定为亮灯
		float r_bright_th;	     //红色画面下，亮灯阈值。含义：与正常灯的对比值，范围(1,2),若设置为1.2f,则代表亮度高于正常灯的112%才会被判定为亮灯
		//种子点筛选对比度阈值
		float b_seed_ratio_th;   //蓝色画面下，种子点筛选阈值。含义：与正常灯的对比值，范围(0,1),若设置为0.1f,则代表与正常灯亮度对比度在10%以上的才会进行筛选
		float g_seed_ratio_th;	 //绿色画面下，种子点筛选阈值。含义：与正常灯的对比值，范围(0,1),若设置为0.1f,则代表与正常灯亮度对比度在10%以上的才会进行筛选
		float r_seed_ratio_th;	 //红色画面下，种子点筛选阈值。含义：与正常灯的对比值，范围(0,1),若设置为0.1f,则代表与正常灯亮度对比度在10%以上的才会进行筛选
	}LEDIlluminationDetectThresh;
	


	typedef enum
	{
		BG_MODE = 1,              //计算背景替换值时，取背景范围内灰度值的众数  
		BG_SORT_MEDIAN = 2,		  //计算背景替换值时，取背景范围内灰度值的中间值
	}LEDIlluminationDetectMehtod;//计算背景方法选择


#ifdef __cplusplus
}
#endif // __cplusplus



#endif