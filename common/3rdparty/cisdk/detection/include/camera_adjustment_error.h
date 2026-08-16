#ifndef __CAMERA_ADJUSTMENT_ERROR_H__
#define __CAMERA_ADJUSTMENT_ERROR_H__

//basic error---------------------------------------------- 1-99
//#include "ci_errors.h"
#include "ci_basic_error.h"
#include "ci_frontend_modname.h"
#include "ci_detection_modname.h"


#define ALGOERROR_CAMERA_ADJUSTMENT_CVMAT_EMPTY                           ERROR_CODE(ALGOMOD_CAMERA_ADJUSTMENT,50)  //cv::Mat数据为空
#define ALGOERROR_CAMERA_ADJUSTMENT_CORNERS_NUM_NOT_4                     ERROR_CODE(ALGOMOD_CAMERA_ADJUSTMENT,51) //输入的角点个数不为4
#define ALGOERROR_CAMERA_ADJUSTMENT_ERROR_MAT_TYPE                        ERROR_CODE(ALGOMOD_CAMERA_ADJUSTMENT,52) //mat类型不对

#endif