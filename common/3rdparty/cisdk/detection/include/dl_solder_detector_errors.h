#ifndef __DL_SOLDER_DETECTOR_ERROR_H__
#define __DL_SOLDER_DETECTOR_ERROR_H__

//basic error---------------------------------------------- 1-99
#include "ci_basic_error.h"
#include "ci_frontend_modname.h"
#include "ci_detection_modname.h"

#define ALGOMOD_DL_SOLDER_DETECTOR_INPUT_EMPTY                        ERROR_CODE(ALGOMOD_DL_SOLDER_DETECTOR,100) //数据为空
#define ALGOMOD_DL_SOLDER_DETECTOR_INPUT_MISMATCH                     ERROR_CODE(ALGOMOD_DL_SOLDER_DETECTOR,101) //数据不匹配
#define ALGOMOD_DL_SOLDER_DETECTOR_NO_TARGET						  ERROR_CODE(ALGOMOD_DL_SOLDER_DETECTOR,102) //此处无目标

#define ALGOMOD_DL_SOLDER_DETECTOR_SOLDER_MISSING                     ERROR_CODE(ALGOMOD_DL_SOLDER_DETECTOR,200) //焊盘缺失


#endif