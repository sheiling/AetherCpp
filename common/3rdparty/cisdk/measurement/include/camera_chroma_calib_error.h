#ifndef __CAMERA_CHROMA_CALIB_ERROR_H__
#define __CAMERA_CHROMA_CALIB_ERROR_H__

//basic error---------------------------------------------- 1-99
//#include "ci_errors.h"
#include "ci_basic_error.h"
#include "ci_frontend_modname.h"

#define ALGOCAMERACHROMACALIB__ERROR_COLOR_TYPE  ERROR_CODE(ALGOMOD_CAMERA_CHROMA_CALIB, 100) //输入AlgoColor类型不对
#define ALGOCAMERACHROMACALIB__INVALID_CAMERAALIGN_INPUT ERROR_CODE(ALGOMOD_CAMERA_CHROMA_CALIB, 101) ////CameraAlign函数point或radius参数无效

#endif