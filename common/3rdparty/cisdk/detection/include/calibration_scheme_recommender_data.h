#ifndef __CALIBRATION_SCHEME_RECOMMENDER_DATA_H__
#define __CALIBRATION_SCHEME_RECOMMENDER_DATA_H__

#include "meta_info_data.h"
#include "algo_mat_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
	typedef struct
	{
		bool lum_flag;              //是否进行亮度校正 false表示不进行，true表示进行
		bool chroma_flag;           //是否进行色度校正 false表示不进行，true表示进行
		bool white_flag;            //是否进行白色校正 false表示不进行，true表示进行

		AlgoMat lum_scheme;      //亮度 方案推荐 ALGO_8UC1  为2*N(h*w) 第一行为推荐的灰阶，第二行为对应灰阶需要的迭代次数
		AlgoMat white_scheme;    //白色 方案推荐 ALGO_8UC1  为2*N(h*w) 第一行为推荐的灰阶，第二行为对应灰阶需要的迭代次数
		AlgoMat chroma_scheme;    //色度 方案推荐 ALGO_8UC1 为1*1 是亮度校正最高灰阶

	}AlgoCalibrationSchemeRecommend;


#ifdef __cplusplus
}
#endif // __cplusplus

#endif