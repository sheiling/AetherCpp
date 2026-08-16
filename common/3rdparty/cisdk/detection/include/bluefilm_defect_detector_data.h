#ifndef __BLUEFILM_DEFECT_DETECTOR_DATA_H__
#define __BLUEFILM_DEFECT_DETECTOR_DATA_H__

#include "ci_data.h"
#include "algo_mat_data.h"

#ifdef __cplusplus
extern "C" 
{
#endif // __cplusplus

	typedef struct
	{
		int defect_type = 2;           //检测目标类型，0为仅检测暗脏污，1为仅检测亮脏污，2为亮暗脏污都检测。注：为0或1时，算法内部默认除了目标脏污外的其他区域均为背景区域，不再滤除其他相关区域，详见《算法测试和集成说明》
		float back_expand_radio = 1.5f; //背景扩充比例，即计算差异时，取脏污候选区域大小的多少倍来当作背景区域求背景均值，内部默认为1.5倍，外部根据情况设置即可，该值必须大于1,一般无需设置
		int img_scale_num = 0;         //图像缩小次数，该值越大图像下采样次数越多，算法效率越高，但检测精度会越差。内部默认为0，即不进行缩放，若为1时进行一次下采样。一般无需设置。

	}BluefilmDefectParameter;//蓝膜脏污检测参数

	typedef struct
	{
		AlgoMat contour_point;  //表示每个脏污的轮廓点，以n行两列的AlgoMat形式储存，第一列数据为轮廓点的x值，第二列数据为轮廓点的y值
		int contours_area;      //表示轮廓的面积，即当前脏污的面积
		float contrast;         //表示当前脏污的差异度，单位为%
		AlgoPoint centroid;     //每个脏污轮廓的质心

	}BluefilmDefectFeature; //每个脏污的特征


	typedef enum 
	{
		RECTANGLE_ROW_ORDER_PLACE = 1,  //晶粒放置按照矩形行顺序一行一行摆放
		RECTANGLE_S_TYPE_PLACE = 2,     //晶粒放置按照矩形S形摆放
		CIRCLE_ROW_ORDER_PLACE = 3,     //晶粒放置按照圆形行顺序一行一行摆放
		CIRCLE_S_TYPE_PLACE =4,        //晶粒放置按照圆形S形摆放
	}DiePlaceType; //蓝膜上的晶粒摆放类型

	typedef struct 
	{
		float place_col_spacing;   //表示芯片行方向摆放间隔（一个芯片中心行方向到下一个芯片中心的距离）
		float place_row_spacing;   //表示芯片列方向摆放间隔（一个芯片中心列方向到下一个芯片中心的距离）
		DiePlaceType place_type;   //芯片放置类型

	}DiePlaceParameter; //晶粒放置参数



#ifdef __cplusplus
}
#endif // __cplusplus
#endif 