#ifndef __QUALITY_ESTIMATOR_ERRORS_H__
#define __QUALITY_ESTIMATOR_ERRORS_H__

//basic error---------------------------------------------- 1-99
//#include "ci_errors.h"
#include "ci_basic_error.h"
#include "ci_frontend_modname.h"

#define ALGOQUALITY__CHECK_ITEMS_AND_THRESHOLD_MISMATCH     ERROR_CODE(ALGOMOD_QUALITY_ESTIMATOR,100)	//质量模块检查项和设置阈值不匹配
#define ALGOQUALITY__SET_THRESH_FIRST                       ERROR_CODE(ALGOMOD_QUALITY_ESTIMATOR,101)	//检查前先设置阈值
#define ALGOQUALITY__SLIDEWIN_TOO_SMALL                     ERROR_CODE(ALGOMOD_QUALITY_ESTIMATOR,102)	//roi 区域太小或者灯点行列不对, 导致灯点窗口<1x1

//手机亮度修缝质量评估错误码
#define ALGOQUALITY_GRID_TOO_SMALL							ERROR_CODE(ALGOMOD_QUALITY_ESTIMATOR,103)//找到灯板数量较少，可能是光环境不合适、环境光过亮或灯点采样率过大
#define ALGOQUALITY_GRID_TOO_BIG							ERROR_CODE(ALGOMOD_QUALITY_ESTIMATOR,104)//找到灯板数量较多，可能是存在杂物干扰
#define ALGOQUALITY_SCREEN_IS_BAD							ERROR_CODE(ALGOMOD_QUALITY_ESTIMATOR,105)//请检查成像区域内屏体是否完整或是否存在与灯板接近的矩形干扰
#define ALGOQUALITY_INPUT_RANGE_WRONG						ERROR_CODE(ALGOMOD_QUALITY_ESTIMATOR,106)//设置的质量评价相关参数范围错误

#define ALGOQUALITY_NO_VALID_POINTS							ERROR_CODE(ALGOMOD_QUALITY_ESTIMATOR,107)//有效点数过少，超过不同区域灯点峰值均值的0.3倍认为是有效点。计算超分质量时可能会出现。基本不会出现该问题
#define ALGOQUALITY_PT_PEAK_VAL_ERROR						ERROR_CODE(ALGOMOD_QUALITY_ESTIMATOR,108)//灯点峰值存在错误，此时最亮的block过曝且最暗的block欠曝，说明相机曝光怎么调都无法满足要求
#define ALGOQUALITY__ROI_ERROR								ERROR_CODE(ALGOMOD_QUALITY_ESTIMATOR,109)//ROI错误，可能是ROI过于靠近图像边缘导致

#endif