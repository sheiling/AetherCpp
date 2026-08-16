#ifndef __LED_CONTOUR_SEARCHER_DATA_H__
#define __LED_CONTOUR_SEARCHER_DATA_H__

#include "algo_mat_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

    typedef struct
    {
        int type;//搜寻灯点框方法
        AlgoMat state;//灯点状态ALGO_8UC1
        AlgoMat xywh;//灯点框 ALGO_32FC4
    }AlgoLedContour;

	typedef enum
	{
		RECT_FIXED = 0,				// 正矩形框，全局WH确定矩形（推荐）
		RECT_FLOAT = 1,				// 正矩形框，自适应WH确定矩形
		CIRCLE_FIXED = 2,			// 圆形框，全局大小一致，暂未开发相关实现代码
		CIRCLE_FLOAT = 3,			// 圆形框，自适应大小，可用于球形屏等场景
	}ContourType;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // 
