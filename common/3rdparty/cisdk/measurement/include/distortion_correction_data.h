#ifndef __DISTORTION_CORRECTION_DATA_H__
#define __DISTORTION_CORRECTION_DATA_H__
#include <string>
#include "ci_data.h"
#include "algo_mat_data.h"

typedef struct
{
	AlgoMat camera_matrix;//相机内参
	AlgoMat dist_coeffs;//畸变矫正系数
}AlgoDistortion;

#endif