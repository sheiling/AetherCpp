#ifndef __ARROW_DETECTOR_DATA_H__
#define __ARROW_DETECTOR_DATA_H__

#include "algo_mat_data.h"
#include "ci_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	typedef enum //箭头的朝向方向
	{
		UP = 0,             //箭头方向向上
		RIGHT = 1,			//箭头方向向右
		DOWN = 2,			//箭头方向向下
		LEFT = 3,			//箭头方向向左
		NONE = 4
	}AlgoArrowLabel;

	typedef struct
	{
		double match_tamplate_thre;
		double misjudgment_thre;

	}AlgoArrowThreshold;





#ifdef __cplusplus
}
#endif // __cplusplus
#endif