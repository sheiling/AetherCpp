#ifndef _AOI_DATA_FUSION_DATA_H_
#define _AOI_DATA_FUSION_DATA_H_
#include "ci_data.h"
#include "algo_mat.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
	typedef struct
	{
		AlgoSize cam_size;                                        // 相机视野尺寸，即相机的分辨率
		AlgoMat cam_trans_mat;                                    // 相机的转换矩阵
	}LedDieMapCameraInfo;

	typedef struct
	{
		AlgoPoint2f mark0_coordinate_mach;                        // 当前灯板Mark0在机械坐标系中的坐标
		AlgoPoint2f mark1_coordinate_mach;                        // 当前灯板Mark1在机械坐标系中的坐标
	}LedDieMapMarkCoordinate;

	typedef struct
	{
		AlgoPoint2f dist_m0_m1;                               // 灯板从Mark0到Mark1的偏移
		AlgoPoint2f dist_m0_g00;                              // 灯板从Mark0到第一个像素中心的偏移
		float pixel_gap_row;                                  // 灯板上两行灯点的间距
		float pixel_gap_col;                                  // 灯板上两列灯点的间距
		int row_nums;                                         // 当前灯板上灯点行数
		int col_nums;                                         // 当前灯板上灯点列数
	}LedDieMapPanelInfo;
#ifdef __cplusplus
}
#endif // __cplusplus

#endif


