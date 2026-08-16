#ifndef __LED_POINT_LOCATOR_ERRORS_H__
#define __LED_POINT_LOCATOR_ERRORS_H__

//basic error---------------------------------------------- 1-99
//#include "ci_errors.h"
#include "ci_basic_error.h"
#include "ci_frontend_modname.h"

#define ALGOLEDPOINT__ERROR_TARGET_WH                       ERROR_CODE(ALGOMOD_LED_POINT_LOCATOR,100) //点亮的灯点行列错误
#define ALGOLEDPOINT__ERROR_FIND_SEED                       ERROR_CODE(ALGOMOD_LED_POINT_LOCATOR,101) //点定位找不到锚点
#define ALGOLEDPOINT__INVALID_LED_LAYOUT                    ERROR_CODE(ALGOMOD_LED_POINT_LOCATOR,102) //灯点排布类型错误
#define ALGOLEDPOINT__SEARCH_OUT_IMG_BOUNDS                 ERROR_CODE(ALGOMOD_LED_POINT_LOCATOR,103) //灯点搜寻超过图像边界
#define ALGOLEDPOINT__EMPTY_DATA                            ERROR_CODE(ALGOMOD_LED_POINT_LOCATOR,104) //shot为空
#define ALGOLEDPOINT__CORNER_NO_VALID_POINT                 ERROR_CODE(ALGOMOD_LED_POINT_LOCATOR,105) //四角点匹配求loc时，有效四角点的角点数量小于1。角点死灯过多或灯板偏移过大 
#define ALGOLEDPOINT__ERROR_INPUT_RANGE_WRONG               ERROR_CODE(ALGOMOD_LED_POINT_LOCATOR,106) //基于任意灯点排布的点定位算法参数越界
#define ALGOLEDPOINT__ERROR_INPUT_PARITY_WRONG              ERROR_CODE(ALGOMOD_LED_POINT_LOCATOR,107) //基于任意灯点排布的点定位算法参数奇偶错误


#endif