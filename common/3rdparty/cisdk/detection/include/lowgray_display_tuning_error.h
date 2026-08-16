#ifndef __LOWGRAY_DISPLAY_TUNING_ERROR_H__
#define __LOWGRAY_DISPLAY_TUNING_ERROR_H__

#include "ci_basic_error.h"
#include "ci_frontend_modname.h"
#include "ci_detection_modname.h"

#define IC_PARAM_DATA_EMPTY(module)         ERROR_CODE(ALGOMOD_LOWGRAY_DISPLAY_TUNING, 101)		//输入图像数据为空
#define IC_PARAM_PARAM_EMPTY(module)        ERROR_CODE(ALGOMOD_LOWGRAY_DISPLAY_TUNING, 102)		//输入参数为空
#define IC_PARAM_LEN_ERROR(module)          ERROR_CODE(ALGOMOD_LOWGRAY_DISPLAY_TUNING, 103)		//输入参数长度异常
#define IC_PARAM_VALUE_ERROR(module)		ERROR_CODE(ALGOMOD_LOWGRAY_DISPLAY_TUNING, 104)		//输入参数数值异常
#define IC_PARAM_RESULT_EMPTY(module)       ERROR_CODE(ALGOMOD_LOWGRAY_DISPLAY_TUNING, 104)		//输出结果为空
#define IC_PARAM_METER_VALUE_ERROR(module)	ERROR_CODE(ALGOMOD_LOWGRAY_DISPLAY_TUNING, 105)		//解析meter图像数据最大值异常



#endif // !__LOWGRAY_DISPLAY_TUNING_ERROR_H__
