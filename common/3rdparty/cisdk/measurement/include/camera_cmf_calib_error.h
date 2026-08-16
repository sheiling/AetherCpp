#ifndef __CAMERA_CMF_CALIB_ERROR_H__
#define __CAMERA_CMF_CALIB_ERROR_H__

//basic error---------------------------------------------- 1-99
//#include "ci_errors.h"
#include "ci_basic_error.h"
#include "ci_frontend_modname.h"

#define ALGOMOD_CAMERA_CMF_CALIB 11111111

#define ALGOCAMERACMFCALIB__ERROR_COLOR_TYPE  ERROR_CODE(ALGOMOD_CAMERA_CMF_CALIB, 100) //输入AlgoColor类型不对
#define ALGOCAMERACMFCALIB__INVALID_CAMERAALIGN_INPUT ERROR_CODE(ALGOMOD_CAMERA_CMF_CALIB, 101) ////CameraAlign函数point或radius参数无效
#define ALGOCAMERACMFCALIB__UNSET_RGB_METER ERROR_CODE(ALGOMOD_CAMERA_CMF_CALIB, 105) /// 未设置RGB meter

#endif