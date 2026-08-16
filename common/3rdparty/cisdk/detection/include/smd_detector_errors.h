#ifndef __SMD_DETECTOR_ERRORS_H__
#define __SMD_DETECTOR_ERRORS_H__

//basic error---------------------------------------------- 1-99
#include "ci_basic_error.h"
#include "ci_frontend_modname.h"
#include "ci_detection_modname.h"



//smd_detector_errors---------------------------------------
#define ALGOERROR_SMD_DETECTOR_DATA_EMPTY                    ERROR_CODE(ALGOMOD_SMD_DETECTOR,100) //输入数据为空
#define ALGOERROR_SMD_DETECTOR_ROI_EMPTY                     ERROR_CODE(ALGOMOD_SMD_DETECTOR,101) //分割得到的ROI区域为空
#define ALGOERROR_SMD_DETECTOR_BACK_EMPTY                    ERROR_CODE(ALGOMOD_SMD_DETECTOR,102) //灯板背景区域未提取到
#define ALGOERROR_SMD_DETECTOR_LED_EMPTY                     ERROR_CODE(ALGOMOD_SMD_DETECTOR,103) //灯点数据未提取到
#define ALGOERROR_SMD_DETECTOR_LED_SORT_FAILED               ERROR_CODE(ALGOMOD_SMD_DETECTOR,104) //灯点按照行列排序失败

#define ALGOERROR_SMD_DETECTOR_TARGET_EMPTY                  ERROR_CODE(ALGOMOD_SMD_DETECTOR,105) //目标为空
#define ALGOERROR_SMD_DETECTOR_MODEL_INPUT_EMPTY             ERROR_CODE(ALGOMOD_SMD_DETECTOR,106) //模板图像输入为空
#define ALGOERROR_SMD_DETECTOR_TEMPLATE_NAME_NOT_EXIST       ERROR_CODE(ALGOMOD_SMD_DETECTOR,107) //模板名字不存在

#endif