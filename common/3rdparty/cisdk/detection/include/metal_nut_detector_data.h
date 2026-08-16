#ifndef __METAL_NUT_DETECTOR_DATA_H__
#define __METAL_NUT_DETECTOR_DATA_H__

#include "algo_mat_data.h"
#include "ci_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


	typedef struct 
	{
		//可调整的阈值
		int shell_nut_area_thre;            //判断底壳螺母是否存在的面积阈值，默认值280
		float shell_nut_distance_thre;    //判断螺母是否处于中心的阈值,默认值50
		double bw_ratio;                  //生成自适应分割阈值的倍数
	}AlgoMetalNutThreshold;


#ifdef __cplusplus
}
#endif // __cplusplus
#endif