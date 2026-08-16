#ifndef _PAD_FINE_LOCATOR_DATA_H_
#define _PAD_FINE_LOCATOR_DATA_H_
#include "ci_data.h"
#include "algo_mat.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
	// 探针x，y坐标，roi的宽高
	/*typedef struct
	{
		int prode_coordinate_x;
		int prode_coordinate_y;
		int roi_width;
		int roi_height;
	}RoiInfo;*/
	// 探针坐标，roi的宽高
	typedef struct
	{
		AlgoPoint prode_coordinate;
		int roi_width;
		int roi_height;
	}RoiInfo;
	
	// 从相机坐标到载台坐标的转换矩阵,通过结构体形式输入
	typedef struct
	{
		float row_0_col_0;
		float row_0_col_1;
		float row_1_col_0;
		float row_1_col_1;
	}cam2plat_transform_struct;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif


