#ifndef __ALGO_LED_METER_DATA_H__
#define __ALGO_LED_METER_DATA_H__

#include "meta_info_data.h"
#include "algo_mat_data.h"
#include "ci_measurement_enum.h"
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

    typedef struct
    {
        Partition* part;//分区信息
        LedPanel* panel;//面板信息
        Layer* layer;//层信息
        Capture* capture;//拍摄信息

        AlgoColor color;//颜色
        CIEType type;//CIE_XYZ类型
        AlgoMat state;//灯点状态 ALGO_8UC1
        AlgoMat pos;//亚像素位置 ALGO_32FC2
        AlgoMat chroma;//色度分量 ALGO_32FC1
    }AlgoLedMeter;

	enum LedMeterAddMethod
	{
		LEDMETERADDMETHOD_ORI = 0,//不同位置数据不需要归一到loc(0,0)位置的均值上 ，直接Add即可
		LEDMETERADDMETHOD_NORMALIZATION = 1,//不同位置数据归一到loc(0,0)位置的均值上
	};//点定位隔点数据合并方法


	typedef enum
	{
		MOTION_PATH_S_HORIZONTAL_LEFT2RIGHT_FIRST = 0,    //s型路线，起点在左侧，先水平向右移动，再垂直向下移动
		MOTION_PATH_S_HORIZONTAL_RIGHT2LEFT_FIRST = 1,    //s型路线，起点在右侧，先水平向左移动，再垂直向下移动
		MOTION_PATH_I_HORIZONTAL_LEFT2RIGHT_FIRST = 2,    //直线型，起点在左侧，先水平向右移动，再垂直向下移动
		MOTION_PATH_I_HORIZONTAL_RIGHT2LEFT_FIRST = 3,    //直线型，起点在右侧，先水平向左移动，再垂直向下移动
		MOTION_PATH_S_VERTICAL_UP2DOWN_FIRST = 4,         //s型路线，起点在上，先垂直向下移动，再水平向右移动
		MOTION_PATH_S_VERTICAL_DOWN2UP_FIRST = 5,         //s型路线，起点在下，先垂直向上移动，再水平向右移动
		MOTION_PATH_I_VERTICAL_UP2DOWN_FIRST = 6,         //直线型，起点在上，先垂直向下移动，再水平向右移动
		MOTION_PATH_I_VERTICAL_DOWN2UP_FIRST = 7,         //直线型，起点在下，先垂直向上移动，再水平向右移动
		/*
		* s型路线示例
		*  开始--------------->
		*					  |
		*  <----------------- ⬇
		* |
		* ⬇---------------->结束
		*
		* 直线型路线示例
		* 开始---------------->
		* <-------------------
		* |
		* |
		* ⬇------------------>
		* <------------------
		* |
		* |
		* ⬇------------------>
		* <----------------结束
		*
		*/
	}AlgoLedMeterMotionPathType;

	
	typedef struct
	{
		//运动路径
		AlgoLedMeterMotionPathType motion_path;  //设备运动路径
		int horizontal_shot_num;     //水平方向上的总计拍摄次数
		int vertical_shot_num;		 //垂直方向上的总计拍摄次数
		int horizontal_overlap_num;  //水平方向上重叠灯珠的数目
		int vertical_overlap_num;    //垂直方向上重叠灯珠的数目
		AlgoSize led_wh;             //整张led灯板的灯点数，w*h(列*高)
	}AlgoLedMeterMergeInfo;


	typedef struct
	{
		//运动路径
		AlgoLedMeterMotionPathType motion_path;  //设备运动路径
		int horizontal_shot_num;     //水平方向上的总计拍摄次数
		int vertical_shot_num;		 //垂直方向上的总计拍摄次数
		int horizontal_overlap_num;  //水平方向上重叠灯珠的数目
		int vertical_overlap_num;    //垂直方向上重叠灯珠的数目
		AlgoSize led_wh;             //整张led灯板的灯点数，w*h(列*高)
	}MergeInfo;
	

	enum LedMeterAbnormalPointType
	{
		AbnormalPointType_LightPoint = 0,
		AbnormalPointType_DarkPoint = 1,
	};//异常亮暗点类型


	enum LedMeterAbnormalModType
	{
		AbnormalModType_Point = 0, //亮暗点  //名字加前缀
		AbnormalModType_Spot = 1,  //亮暗斑
	};//异常点类型

	


#define LEDMETER_OPERATE_BUFFER_SIZE 1048576 // 1MB=1024*1024

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // 
