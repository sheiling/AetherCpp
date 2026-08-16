#ifndef __LED_IRREGULAR_CONTOUR_SEARCHER_DATA_H__
#define __LED_IRREGULAR_CONTOUR_SEARCHER_DATA_H__

#include "algo_mat_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	typedef enum
	{
		IrregularContourType_ROTATE_RECT = 0,	// 旋转矩形，可以用于球身
		IrregularContourType_TRAPE = 1,			// 梯形，可能用于球顶
		IrregularContourType_TRIANGLE = 2,		// 三角形，可能用于球顶
		IrregularContourType_QUADRANGLE = 3,	// 四边形
	} IrregularContourType;//灯点框类型

    typedef struct
    {
        int ir_contour_type;		// 灯点框类型
        AlgoMat state;				// 灯点状态ALGO_8UC1
		int corner_length;			// 异形灯点框的角点个数
        AlgoMat* corners;			// 灯点框ALGO_32FC2，矩阵个数为corner_length
    }AlgoLedIrContour;


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // 
