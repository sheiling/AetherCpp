#ifndef __PCB_SOCKET_LOCATOR_DATA_H__
#define __PCB_SOCKET_LOCATOR_DATA_H__

#include "ci_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	typedef enum
	{
		//灯板对插孔类型（type）
		PANEL_SOCKET_TYPE_0 = 0,  //目前需求的三种对插孔类型（BOE1.5、1.25、0.9）均可兼容，算法内部默认，目前不用修改，后续根据算法升级会增加其他可选类型
		PANEL_SOCKET_TYPE_1 = 1,  //后续预留，当前用不到
		PANEL_SOCKET_TYPE_2 = 2,  //后续预留，当前用不到

	}PcbSocketType; 

	typedef enum 
	{
		//定位方法（method）
		LOCATE_RECTANGLE = 0,  //使用外接矩形定位对插孔中心，算法默认的定位方法，一般不用修改
		LOCATE_BARYCENTER = 1, //使用轮廓的质心定位对插孔中心，算法的备用定位方法。

	}PcbSocketMethod;

	typedef enum 
	{
		//定位目标是否明暗转换（GrayInverse）
		LOCATE_LIGHT = 0, // 不进行明暗转换，内部默认值
		LOCATE_DARK = 1,  //进行明暗转换
	}PcbSocketGrayInverse;



	typedef struct
	{
		//灯板对插孔定位抗干扰阈值
		float remove_noise_thresh; //需要算法抗毛刺或灰尘的程度阈值，默认为3.5，最大不要超过4.5，该值越大，算法定位越不稳定。
		//轮廓分割阈值
		int segment_thresh;      //轮廓分割阈值，默认为6，可根据对插孔在图像中的大小进行调节，推荐值为3~12，对插孔占比越大，该值越大，越小时相反。一般无需修改

	}PcbSocketThresh;    //灯板对插孔定位阈值结构体


	typedef struct 
	{
		//输出对插孔位置偏离图像中心的坐标，坐标原点在图像中心。
		AlgoPoint2f pcb_socket_center;

		//输出对插孔的偏转角度，顺时针旋转为负，逆时针旋转为正，单位°//水平
		float pcb_socket_angle;

	}PcbSocketResult; //灯板对插孔定位的输出结果

#ifdef __cplusplus
}

#endif // __cplusplus

#endif

