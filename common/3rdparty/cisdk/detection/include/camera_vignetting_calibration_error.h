#ifndef __CAMERA_VIGNETTING_CALIBRATION_ERROR_H__
#define __CAMERA_VIGNETTING_CALIBRATION_ERROR_H__

//basic error---------------------------------------------- 1-99
#include "ci_basic_error.h"
#include "ci_frontend_modname.h"
#include "ci_detection_modname.h"

#define ALGOERROR_CAMERA_VIGNETTING_CALIBRATION_RATIO_ERROR                           ERROR_CODE(ALGOMOD_CAMERA_VIGNETTING_CALIBRATION,50)   //光路修正缩放比例小于1或大于64
#define ALGOERROR_CAMERA_VIGNETTING_CALIBRATION_CHANNELS_ERROR                        ERROR_CODE(ALGOMOD_CAMERA_VIGNETTING_CALIBRATION,51)   //图像通道不是3

#endif