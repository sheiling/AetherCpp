#ifndef __CAMERA_CHROMA_CALIB_DATA_H__
#define __CAMERA_CHROMA_CALIB_DATA_H__

#include "meta_info_data.h"
#include "algo_mat_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

    typedef struct
    {
        AlgoColor color;      //颜色
        AlgoMat trans_matrix; //转换矩阵

    }AlgoCamCalib;


#ifdef __cplusplus
}
#endif // __cplusplus
#endif