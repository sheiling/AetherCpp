#ifndef _DL_DIE_BONDING_DETECTOR_ERROR_H_
#define _DL_DIE_BONDING_DETECTOR_ERROR_H_

//basic error---------------------------------------------- 1-99
#include "ci_basic_error.h"
#include "ci_frontend_modname.h"
#include "ci_detection_modname.h"

#define ALGOERROR_DIE_BONDING_DETECTOR_INPUT_EMPTY                        ERROR_CODE(ALGOMOD_DL_DIE_BONDING_DETECTOR,100) //数据为空
#define ALGOERROR_DIE_BONDING_DETECTOR_INPUT_MISMATCH                     ERROR_CODE(ALGOMOD_DL_DIE_BONDING_DETECTOR,101) //数据不匹配
#define ALGOERROR_DIE_BONDING_DETECTOR_NUM_PAD_ERROR                      ERROR_CODE(ALGOMOD_DL_DIE_BONDING_DETECTOR,102) //焊盘数目有误
#define ALGOERROR_DIE_BONDING_DETECTOR_DATA_PROCESS_ERROR                 ERROR_CODE(ALGOMOD_DL_DIE_BONDING_DETECTOR,103) //数据处理错误
#define ALGOERROR_DIE_BONDING_DETECTOR_PARAMETER_ERROR                    ERROR_CODE(ALGOMOD_DL_DIE_BONDING_DETECTOR,104) //参数错误


#endif