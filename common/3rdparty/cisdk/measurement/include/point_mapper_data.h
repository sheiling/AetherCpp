#ifndef __POINT_MAPPER_DATA_H__
#define __POINT_MAPPER_DATA_H__

#include "ci_data.h"
#include "algo_mat.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	typedef enum
	{
		Point_FLAG_NONEXIST = 0b00000000,//物理不存在
		Point_FLAG_DEAD = 0b00000001,//死灯标志  
		Point_FLAG_LIVE = 0b00000010,//非死灯标志
		Point_FLAG_ISOLATED = 0b00000110,//孤立标志
	} PointFlag;//灯点状态


	// 2025-09-17，新标准
	typedef enum
	{
		POINTMAPMETHOD_LINE_FITTING = 0,		// 非标准排布（默认推荐），适用于大多数场景（包括矩形屏、亚像素、行列不等间距、轻微缺块、整行整列缺失、微弧形屏），灯点排布可以是矩形、三角形等；
		POINTMAPMETHOD_REGION_GROWTH = 1,	// 非矩形形状，适用于裁切屏体、大面积死灯等场景，要求屏体必须联通，不能有整行整列的缺失；灯点排布为矩形；
	} PointMapMethod;//灯点排列方法


	typedef struct
	{
		AlgoMat state;//灯点状态  ALGO_8UC1
		AlgoMat loc;//灯点亚像素位置 ALGO_32FC2
	} PointMapResult;

#define POINT_FLAG_CHECK_EXIST(flag) ((bool)flag)
#define POINT_FLAG_CHECK_DEAD(flag) (flag & Point_FLAG_DEAD)
#define POINT_FLAG_CHECK_LIVE(flag) (flag & Point_FLAG_LIVE)
#define POINT_FLAG_CHECK_ISOLATED(flag) (flag == Point_FLAG_ISOLATED)


#ifdef __cplusplus
}
#endif // __cplusplus



#endif