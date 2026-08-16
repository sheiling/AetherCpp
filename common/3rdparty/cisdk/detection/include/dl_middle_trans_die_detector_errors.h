#ifndef __DL_MIDDLE_TRANS_DIE_DETECTOR_ERROR_H__
#define __DL_MIDDLE_TRANS_DIE_DETECTOR_ERROR_H__

//basic error---------------------------------------------- 1-99
#include "ci_basic_error.h"
#include "ci_frontend_modname.h"
#include "ci_detection_modname.h"

#define ALGOMOD_DL_MIDDLE_TRANS_DIE_DETECTOR_INPUT_EMPTY                        ERROR_CODE(ALGOMOD_DL_MIDDLE_TRANS_DIE_DETECTOR,100) //数据为空
#define ALGOMOD_DL_MIDDLE_TRANS_DIE_DETECTOR_INPUT_MISMATCH                     ERROR_CODE(ALGOMOD_DL_MIDDLE_TRANS_DIE_DETECTOR,101) //数据不匹配
#define ALGOMOD_DL_MIDDLE_TRANS_DIE_DETECTOR_NO_TARGET						    ERROR_CODE(ALGOMOD_DL_MIDDLE_TRANS_DIE_DETECTOR,103) //此处无目标

#define ALGOMOD_DL_MIDDLE_TRANS_DIE_DETECTOR_MATCH_CONTOUR_EMPTY                ERROR_CODE(ALGOMOD_DL_MIDDLE_TRANS_DIE_DETECTOR,102)  //没有匹配到芯片





#endif