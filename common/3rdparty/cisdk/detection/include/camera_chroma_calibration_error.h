#ifndef __CAMERA_CHROMA_CALIBRATION_ERROR_H__
#define __CAMERA_CHROMA_CALIBRATION_ERROR_H__

//basic error---------------------------------------------- 1-99
#include "ci_basic_error.h"
#include "ci_frontend_modname.h"
#include "ci_detection_modname.h"

#define ALGOERROR_CAMERA_CHROMA_CALIBRATION_CVMAT_EMPTY                           ERROR_CODE(ALGOMOD_CAMERA_CHROMA_CALIBRATION,50)  //cv::Mat数据为空
#define ALGOERROR_CAMERA_CHROMA_CALIBRATION_FAILURE_OPENFILE                      ERROR_CODE(ALGOMOD_CAMERA_CHROMA_CALIBRATION,51)  //打开文件错误

#endif