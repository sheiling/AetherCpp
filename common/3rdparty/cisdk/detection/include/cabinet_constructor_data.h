#ifndef _CABINET_CONSTRUCTOR_DATA_H_
#define _CABINET_CONSTRUCTOR_DATA_H_

#include "ci_data.h"
#include "algo_mat.h"

//#define USE_CUDA 0

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
	
	//用于识别的标识码类别
	typedef enum
	{
		BARCODE_1D_CUSTOM = 1,     // 用来对模组或数据组进行标识的识别码类别，1维条形码，表现形式为为1*N或N*1;
		BARCODE_2D_ARUCO = 2,      // 用来对模组或数据组进行标识的识别码类别，2维矩阵码-Aruco，表现形式为N*N;
	}CabinetConstructMarkerType;


#ifdef __cplusplus
}
#endif // __cplusplus

#endif


