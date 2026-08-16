#ifndef __MURA_SALTPEPPER_INSPECTOR_DATA_H__
#define __MURA_SALTPEPPER_INSPECTOR_DATA_H__

#include "algo_mat_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	typedef struct
	{
		////灯板光通量图检测阈值
		//float thr_mura_saltpepperMin_R;//红色画面最小阈值
		//float thr_mura_saltpepperMax_R;//红色画面最大阈值
		//float thr_mura_saltpepperMin_G;//绿色画面最小阈值 
		//float thr_mura_saltpepperMax_G;//绿色画面最大阈值
		//float thr_mura_saltpepperMin_B;//蓝色画面最小阈值
		//float thr_mura_saltpepperMax_B;//蓝色画面最大阈值
		//float thr_mura_saltpepperMin_W;//白色画面最小阈值
		//float thr_mura_saltpepperMax_W;//白色画面最大阈值
		//屏幕图像检测阈值
		float thr_mura_saltpepper_detect; //精细检测重要阈值，即控制检出麻点的阈值，进而控制表征值大小。快速检测不需要此阈值，不用设置。
		float thr_mura_saltpepper_region; //结果绘制阈值，即控制检出区域的阈值。精细检测与快速检测均需要设置

	}MuraSaltpepperInspectThresh;//麻点mura缺陷检测阈值结构体


	typedef struct 
	{
		//屏幕检测入参
		float in_width_length; //被测屏幕的实际宽度，单位：cm     注：外部必须输入
		float in_hight_length; //被测屏幕的实际高度，单位：cm     注：外部必须输入
		float in_actual_distance; //相机拍摄的实际距离，单位：cm     注：外部必须输入
		int in_light_or_dark_type; //检测类型，0为暗麻点，1为亮麻点，2为亮暗均检测  注：内部默认为0，即暗麻点检测。
		int in_LED_rows_num;  //拼屏灯板的排数，即有多少行灯板，即分区检测数量的行数    注：内部默认为1
		int in_LED_cols_num;  //拼屏灯板的列数，即有多少列灯板，即分区检测数量的列数    注：内部默认为1
		float in_back_ratio;  //对比度背景区域因子                注：默认为1.5，不建议修改

	}MuraSaltpepperInspectParameter; //麻点mura缺陷检测入参结构体


	typedef struct 
	{
		//检测结果输出
		AlgoMat out_saltpepper_level; //屏幕检测输出结果数据，以AlgoMat形式保存，其中每个元素的位置代表检测的麻点区域位置，元素值为对应区域的表征值，元素值为0时代表当前区域在当前阈值下未检出麻点。
		AlgoMat out_img;  //屏幕检测输出结果图像，图像为输入图像增强变换后的图，图像上方绘制的信息为当前阈值下检测出的麻点区域及对应的表征值。
		AlgoMat out_img_input; //屏幕检测输出结果图像，图像为输入图像原图，图像上方绘制的信息为当前阈值下检测出的麻点区域及对应的表征值。
	}MuraSaltpepperInspectResult; //麻点mura缺陷检测结果输出结构体

	typedef enum
	{
		//检测类型：亮暗麻点
		MURA_SALTPEPPER_INSPECT_DARK = 0, //暗麻点检测，内部默认检测暗麻点
		MURA_SALTPEPPER_INSPECT_LINGT = 1,  //亮麻点检测
		MURA_SALTPEPPER_INSPECT_ALL = 2,  //亮暗麻点均检测
	}MuraSaltpepperInspectFlag;

#ifdef __cplusplus
}
#endif // __cplusplus
#endif