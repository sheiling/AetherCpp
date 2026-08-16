#ifndef __SHARPNESS_AUTOFOCUS_DATA_H__
#define __SHARPNESS_AUTOFOCUS_DATA_H__

#include "ci_data.h"
#include "algo_mat.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	typedef enum
	{
		TENENGRAD = 0,        //基于一介梯度
		LAPLACIAN = 1,        //基于二阶梯度
		FREQUENCY = 2,        //基于信息熵
		FUZZY_ENTROPY = 3,   //基于频域信息
	} SharpnessMethod;//清晰度评价方法




#ifdef __cplusplus
}
#endif // __cplusplus



#endif