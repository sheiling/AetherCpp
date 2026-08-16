#ifndef __CABINET_DEFECT_DETECTOR_DATA_H__
#define __CABINET_DEFECT_DETECTOR_DATA_H__

#include "ci_data.h"
#include "algo_mat.h"


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	typedef enum 
	{
		LOCATE = 1,              //定位
		LOCATE_OBB = 2,			 //定位（旋转框）
		CLASSIFY = 3,            //分类
		SEGMENT = 4,			 //分割
	}DLCabinetDefectModelType;

	typedef enum
	{
		SCREW_HOLE_OK = 0,			      //螺丝孔完好，没有破损，脏污  
		MAGNETIC_BOLT_OK = 1,			  //磁铁柱完好，没有破损，脏污
		LOCATE_BOLT_OK = 2,				  //定位柱完好，没有破损，脏污
		LABEL_OK = 3,					  //侧面电源标签完好，没有破损，脏污
		LOGO_OK = 4,					  //背面LOGO完整，每个字母都存在且每个字母都没有破损，脏污
		IC_OK = 5,						  //IC存在
		NAMEPLATE_OK = 6,                 //背面铭牌OK,且铭牌没有破损，划痕
		LOGO_B_OK = 7,					  //背面LOGO字母B存在，且字母完整，没有破损脏污
		LOGO_O_OK = 8,					  //背面LOGO字母O存在，且字母完整，没有破损脏污
		LOGO_E_OK = 9,                    //背面LOGO字母E存在，且字母完整，没有破损脏污	
		SCREW_INSTALLED = 10,             //螺丝孔上存在螺丝
		CAPACITOR_INSTALLED = 11,		  //该位置存在电容

		SCRATCH = 12,					  //划痕
		SCREW_HOLE_NG = 13,			      //螺丝孔出现破损，或脏污
		MAGNETIC_BOLT_NG = 14,   		  //磁铁柱出现破损，或脏污
		LOCATE_BOLT_NG = 15,	          //定位柱出现破损，或脏污
		METAL_DIRT = 16,				  //金属氧化或脏污(正面十字架/一字架，或侧面金属区域)
		IC_MISS = 17,					  //此处应有IC,但IC缺失
		LABEL_NG = 18,					  //侧面电源标签存在，但存在破损或脏污
		LABEL_MISS = 19,				  //侧面电源标签缺失
		LOGO_NG = 20,					  //背面LOGO的三个字母均存在，但存在字母上有破损或脏污
		LOGO_MISS = 21,					  //背面LOGO，B/O/E三个字母中，存在一个以上的缺失
		NAMEPLATE_NG = 22,				  //背面铭牌存在，但铭牌上存在脏污或划痕
		NAMEPLATE_MISS = 23,              //背面铭牌缺失
		LOGO_B_NG = 24,					  //背面LOGO字母B存在，但存在破损或脏污
		LOGO_O_NG = 25,					  //背面LOGO字母O存在，但存在破损或脏污
		LOGO_E_NG = 26,                   //背面LOGO字母E存在，但存在破损或脏污
		SIDE_DIRT = 27,					  //侧面金属区域脏污或氧化

		SCREW_MISS = 50,				  //螺丝缺失，即：当前位置应该存在一个螺丝，但没有安装螺丝
		SCREW_ADDITION = 51,			  //螺丝安装错误，即：当前位置安装了一个螺丝，但设备设计该位置是没有螺丝的
		MAGNETIC_BOLT_MISS = 52,		  //磁铁柱缺失，即：当前位置应该存在一个磁铁柱，但没有安装磁铁柱
		MAGNETIC_BOLT_ADDITION = 53,	  //磁铁住安装错误，即：当前位置安装了一个磁铁柱，但设备设计该位置是没有磁铁柱的
		SCREW_HOLE_MISS = 54,			  //螺丝孔缺失，即：当前位置应该存在一个螺丝孔，算法未检出或该螺丝孔未安装
	}DLCabinetDefectResult;

	typedef enum
	{
		SCREW = 1,			               //螺丝、螺丝孔
		MAGNETIC_BOLT = 2,			       //磁铁柱
		LOCATE_BOLT = 3,				   //定位柱
		LABEL = 4,						   //标签
		LOGO = 5,						   //LOGO
		IC = 6,							   //IC
		SCRATCHES = 7,					   //划痕
		DIRT = 8,					       //金属十字架/一字架脏污
		CABINET_NAMEPLATE = 9,             //箱体铭牌
		CAPACITOR = 10,					   //电容
	}DLCabinetDefectDetectType;
	

	typedef enum
	{
		X_2_RIGHT_Y_2_DOWN_S_X_FIRST = 1,
		X_2_RIGHT_Y_2_UP_S_X_FIRST = 2,
		X_2_LEFT_Y_2_DOWN_S_X_FIRST = 3,
		X_2_LEFT_Y_2_UP_S_X_FIRST = 4,
		X_2_RIGHT_Y_2_DOWN_S_Y_FIRST = 5,
		X_2_RIGHT_Y_2_UP_S_Y_FIRST = 6,
		X_2_LEFT_Y_2_DOWN_S_Y_FIRST = 7,
		X_2_LEFT_Y_2_UP_S_Y_FIRST = 8,
		X_2_RIGHT_Y_2_DOWN_I_X_FIRST = 9,
		X_2_RIGHT_Y_2_UP_I_X_FIRST = 10,
		X_2_LEFT_Y_2_DOWN_I_X_FIRST = 11,
		X_2_LEFT_Y_2_UP_I_X_FIRST = 12,
		X_2_RIGHT_Y_2_DOWN_I_Y_FIRST = 13,
		X_2_RIGHT_Y_2_UP_I_Y_FIRST = 14,
		X_2_LEFT_Y_2_DOWN_I_Y_FIRST = 15,
		X_2_LEFT_Y_2_UP_I_Y_FIRST = 16,
		/*
		* S型路线(X_2_RIGHT_Y_2_DOWN_S_X_FIRST示例)：
		* 开始--->--->--->--->
		*					|
		*    <---<---<---<---
		* |
		* --->--->--->--->结束
		*
		* I型路线（X_2_RIGHT_Y_2_DOWN_I_X_FIRST示例）
		*
		* 开始--->--->--->--->
		*  <---<---<---<---
		* |
		* --->--->--->--->
		* 结束 <---<---<---<---
		*/
	}DLCabinetDefectImageCapturePath;


	typedef struct
	{
		DLCabinetDefectImageCapturePath img_capture_path; //运动扫描路径，参考DLCabinetDefectImgOrder;
		AlgoSize merge_size;	           //样本是几行几列（宽度方向上有几张图，高度方向上有几张图）
	}DLCabinetDefectMergeInfo;

	typedef struct
	{
		float iou_th;			 //iou阈值,当两个框的重叠面积超过该值，取其中一个   
		float confidence_th;	 //置信度阈值  
	}DLCabinetDefectDLThresh;
	
	typedef struct
	{
		float mech_acc;			      //机械重复定位精度
		float border_gap;	          //边界附近结果裁切阈值，默认2个像素，结果距离图像边缘2个像素，剔除
	}DLCabinetDefectMergeThresh;      //合并阈值


#ifdef __cplusplus
}
#endif // __cplusplus



#endif