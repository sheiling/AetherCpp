#ifndef __EDGE_CALIB_DATA_H__
#define __EDGE_CALIB_DATA_H__

#include "meta_info_data.h"
#include "algo_mat_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

    typedef struct
    {
        int edge_height;//边缘修正时候使用的边缘宽高
        int edge_width;
        AlgoColor color;//颜色 
        AlgoMat x_data;//不同分量的标定曲面信息 ALGO_32FC1
        AlgoMat y_data;
        AlgoMat z_data;
    }AlgoEdgeCalib;



	typedef struct
	{
		int BoxID;  //base 为0

		float edgeCenterRito[4];  //值为1表示正常， 非1异常， 顺序为左、右、上、下

	}EdgeCalibEdgeInfo;

#ifdef __cplusplus
}
#endif // __cplusplus
#endif