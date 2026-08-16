#ifndef __SHARPNESS_AUTOFOCUS_ERRORS_H__
#define __SHARPNESS_AUTOFOCUS_ERRORS_H__


#include "ci_basic_error.h"
#include "ci_frontend_modname.h"
#include "ci_detection_modname.h"


#define ALGOSHARPNESS__ERROR_METHOD                    ERROR_CODE(ALGOMOD_SHARPNESS_AUTOFOCUS,101)  //输入的计算方法错误
#define ALGOSHARPNESS__ERROR_IMG_NUM                   ERROR_CODE(ALGOMOD_SHARPNESS_AUTOFOCUS,102)  //输入的样本数量错误或样本数量与坐标数量不相等
#define ALGOSHARPNESS__ERROR_INPUT_EMPTY               ERROR_CODE(ALGOMOD_SHARPNESS_AUTOFOCUS,103)  //输入的样本为空,或数据为空
#define ALGOSHARPNESS__ERROR_IMG_TYPE_ERROR            ERROR_CODE(ALGOMOD_SHARPNESS_AUTOFOCUS,104)  //输入的样本类型错误，只支持8U或16U
#define ALGOSHARPNESS__ERROR_RESULT_NUM                ERROR_CODE(ALGOMOD_SHARPNESS_AUTOFOCUS,105)  //坐标数量小于等于2									
#define ALGOSHARPNESS__ERROR_FIT_CURVE                 ERROR_CODE(ALGOMOD_SHARPNESS_AUTOFOCUS,106)  //拟合错误，检查输入数据是否正确									
#define ALGOSHARPNESS__ERROR_THREAD_NUM                ERROR_CODE(ALGOMOD_SHARPNESS_AUTOFOCUS,107)  //输入的CPU并行线程数错误，请检查是否输入的值小于1或设置的并行线程数超过了CPU最大线程数								
#define ALGOSHARPNESS__ERROR_ROI                       ERROR_CODE(ALGOMOD_SHARPNESS_AUTOFOCUS,108)  //ROI错误									


#endif