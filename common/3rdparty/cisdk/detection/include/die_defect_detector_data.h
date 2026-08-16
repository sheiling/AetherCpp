#ifndef __DIE_DEFECT_DETECTOR_DATA_H__
#define __DIE_DEFECT_DETECTOR_DATA_H__

#include "ci_data.h"
#include "algo_mat_data.h"

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

	typedef enum 
	{
		EDGE_DETECT=0,   //边缘检测，即当检测的缺陷边界超过对应缺陷的设定ROI边缘时，认为存在缺陷，不超过时认为无缺陷
		AREA_DETECT=1,   //面积检测，即当检测到的芯片面积与正常芯片面积比值大于阈值时，认为存在缺陷，不超过时认为无缺陷。
		ALL_DETECT=2,    //边缘检测与面积检测共同判断，两者均满足时才为缺陷。内部默认为该方法

	}DieDefectDetectMethod; //缺陷检测方法(注：该方法仅针对双胞缺陷以及破损缺陷来设置，内部脏污缺陷不用设置)

	typedef enum 
	{
		BLACK_BACKGROUND = 0,  //相比而言，芯片本身亮度大于背景亮度，即黑底白目标
		WHITE_BACKGROUND = 1,  //相比而言，芯片本身亮度低于背景亮度，即白低黑目标

	}DieBackgroundType;  //芯片背景类型

	typedef struct
	{
		bool twin_detect_flag = true;      //多胞检测flag，为true时表示检测多胞缺陷，为false时表示不检测多胞缺陷
		bool damage_detect_flag = true;    //破损检测flag，为true时表示检测破损缺陷，为false时表示不检测破损缺陷
		bool dirty_detect_flag = true;     //脏污检测flag，为true时表示检测脏污缺陷，为false时表示不检测脏污缺陷，仅检测脏污时，需要单独调用脏污检测ROI接口设置独立的检测ROI

	}DieDefectDetectList; //芯片缺陷检测项

	typedef struct
	{
		float x_coord;  //输入的芯片位置x坐标，坐标原点必须在图像中心
		float y_coord;  //输入的芯片位置y坐标，坐标原点必须在图像中心
		float angle;    //输入的芯片角度
	}DieLocateInfo;     //输入待检测芯片的位置信息。

	typedef enum 
	{
		UNKNOWN_DEFECT = -1,   //表示无法检测
		NONE_DEFECT = 0,      //表示无缺陷
		DAMAGE_DEFECT = 1,    //表示破损缺陷
		TWIN_DEFECT = 2,      //表示多胞缺陷
		DIRTY_DEFECT = 3,     //表示脏污缺陷

	}DieDefectDetectResult;  //芯片缺陷检测结果

#ifdef __cplusplus
}
#endif // __cplusplus
#endif 