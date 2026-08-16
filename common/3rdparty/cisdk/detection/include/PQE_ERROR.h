#ifndef __PQE_ERROR_H__
#define __PQE_ERROR_H__

// basic error---------------------------------------------- 1-99
//#include "ci_errors.h"
#include "ci_basic_error.h"
#include "ci_detection_modname.h"

// clang-format off
#define ALGO_DISPLAY_PERFOEMANCE_EVALUATOR_DATA_LENCE                   ERROR_CODE(ALGOMOD_DISPLAY_PERFORMANCE_EVALUATOR,101)  //传入刷新率数据长度小于6帧采样点，不满足要求

// clang-format on

#endif