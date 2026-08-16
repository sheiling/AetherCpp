#ifndef __LUM_ACCURACY_OPTIMIZER_DATA_H__
#define __LUM_ACCURACY_OPTIMIZER_DATA_H__

#include "meta_info_data.h"
#include "algo_mat_data.h"

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus
    typedef struct
    {
        AlgoColor color;
        AlgoSize psf_size; // 求解的最终PSF尺寸
        AlgoMat psf_x;     // x滤片标定得到的psf模型
        AlgoMat psf_y;     // y滤片标定得到的psf模型
        AlgoMat psf_z;     // z滤片标定得到的psf模型
    } AlgoLumOptimizerData;

#ifdef __cplusplus
}
#endif // __cplusplus
#endif