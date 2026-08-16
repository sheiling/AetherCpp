#ifndef __LED_LUM_EXTRACTOR_ERRORS_H__
#define __LED_LUM_EXTRACTOR_ERRORS_H__

//basic error---------------------------------------------- 1-99
//#include "ci_errors.h"
#include "ci_basic_error.h"
#include "ci_frontend_modname.h"

#define ALGOLEDLUM__ERROR_INVALID_CONTOUR_TYPE                ERROR_CODE(ALGOMOD_LED_LUM_EXTRACTOR,100) //输入的contour内参不匹配，请检查contour的类型是否为如下四种内的一种：CIRCLE_FIXED、CIRCLE_FLOAT、RECT_FIXED、RECT_FLOAT
#define ALGOLEDLUM__ERROR_WRONG_METHOD					      ERROR_CODE(ALGOMOD_LED_LUM_EXTRACTOR,101) //输入的提取光通量方法不对，请检查输入的方法是否不在LedLumMethod中定义；
#define ALGOLEDLUM__ERROR_EMPTY_SHOT					      ERROR_CODE(ALGOMOD_LED_LUM_EXTRACTOR,102) //输入的shot信息为空，请检查输入的shot信息,可能存在如下两种情况：1)shot的handle的指针为空；2)shot内的图像为空；
#define ALGOLEDLUM__ERROR_MISMATCH_CHANNEL_NUM                ERROR_CODE(ALGOMOD_LED_LUM_EXTRACTOR,103) //输入的shot通道数不对，本模块需要单通道shot，请检查输入的shot是否为单通道；
#define ALGOLEDLUM__ERROR_EMPTY_CONTOUR                       ERROR_CODE(ALGOMOD_LED_LUM_EXTRACTOR,104) //输入的contour信息为空，请检查输入的contour信息，可能存在如下两种情况：1)contour的handle的指针为空；2)contour内的结果为空；
#define ALGOLEDLUM__ERROR_EMPTY_ROI                           ERROR_CODE(ALGOMOD_LED_LUM_EXTRACTOR,105) //输入的roi信息为空，请检查输入的roi信息，可能存在如下两种情况：1)roi的handle的指针为空；2)roi内的结果为空；																					
#define ALGOLEDLUM__ERROR_DC_PARA                             ERROR_CODE(ALGOMOD_LED_LUM_EXTRACTOR,106) //输入的dc值错误，请检查输入的dc值范围在如下范围：1）dc_dtat>0;2)dc_width >0;
#define ALGOLEDLUM__ERROR_DC_REGION_OUTOF_SHOTROI             ERROR_CODE(ALGOMOD_LED_LUM_EXTRACTOR,107) //输入的dc范围超过了shot的区域，请检查shot的roi区域和dc的roi区域是否匹配；
#define ALGOLEDLUM__ERROR_EMPTY_RESULT                        ERROR_CODE(ALGOMOD_LED_LUM_EXTRACTOR,108) //计算结果为空，请检查本handle内，lum（光通量结果）或state（灯点状态结果）是否为空
#define ALGOLEDLUM__ERROR_INVALID_IR_CONTOUR_TYPE             ERROR_CODE(ALGOMOD_LED_LUM_EXTRACTOR,109) //输入的ir_contour内参不匹配，可能是ir_contour的类型type不正确或者暂时没有对应的提取方法
#define ALGOLEDLUM__ERROR_EMPTY_IR_CONTOUR                    ERROR_CODE(ALGOMOD_LED_LUM_EXTRACTOR,110) //输入的ir_contour信息为空，请检查输入的ir_contour信息，可能存在如下两种情况：1)ir_contour的handle的指针为空；2)ir_contour内的结果为空；
#define ALGOLEDLUM__ERROR_PERSPECTIVE_TRANSFORM               ERROR_CODE(ALGOMOD_LED_LUM_EXTRACTOR,111) //点阵超分拉伸变换后灯点框超画布尺寸边界，请尝试减小校正距离增大灯点采样率来规避
#define ALGOLEDLUM__ERROR_CONTOUR_EXCEED_BORDER               ERROR_CODE(ALGOMOD_LED_LUM_EXTRACTOR,112) //提光通量时，contour超过了图像的边界


#endif