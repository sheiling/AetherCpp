#ifndef __DISTORTION_CORRECTION_ERROR_H__
#define __DISTORTION_CORRECTION_ERROR_H__

//basic error---------------------------------------------- 1-99
#include "algo_log.h"
#include "ci_basic_error.h"
#include "ci_frontend_modname.h"

#define ALGOPOINTMAP__ERROR_EMPTY_INPUT_IMG           ERROR_CODE(ALGOMOD_DISTOTTION_CORRECTION,100)  //输入的图像/数据为空
#define ALGOPOINTMAP__ERROR_INPUT_INVALID             ERROR_CODE(ALGOMOD_DISTOTTION_CORRECTION,101)  //输入的数据错误，请检查输入是否正确***
#define ALGOPOINTMAP__ERROR_INPUT_IMG_SIZE_INVALID         ERROR_CODE(ALGOMOD_DISTOTTION_CORRECTION,102)  //输入的图像存在尺寸不一致



#endif