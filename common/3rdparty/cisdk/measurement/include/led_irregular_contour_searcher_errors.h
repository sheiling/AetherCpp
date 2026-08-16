#ifndef __LED_IRREGULAR_CONTOUR_SEARCHER_ERRORS_H__
#define __LED_IRREGULAR_CONTOUR_SEARCHER_ERRORS_H__

//basic error----------------------------------------------1-99
//#include "ci_errors.h"
#include "ci_basic_error.h"
#include "ci_frontend_modname.h"


#define ALGOLEDIRREGULARCONTOUR__EMPTY_SHOT                          ERROR_CODE(ALGOMOD_LED_IRREGULAR_COUTOUR_SEARCHER,100)		// shot为空，没办法重新定位灯点中心或者绘制搜索框图像
#define ALGOLEDIRREGULARCONTOUR__SPHERICAL_ROW_LED_NUM_TOO_FEW       ERROR_CODE(ALGOMOD_LED_IRREGULAR_COUTOUR_SEARCHER,101)		// 球形屏某行真实灯点个数过少（少于三个）
#define ALGOLEDIRREGULARCONTOUR__SPHERICAL_IR_CONTOUR_TYPE_ERROR     ERROR_CODE(ALGOMOD_LED_IRREGULAR_COUTOUR_SEARCHER,102)		// 异形框的类型不正确   



#endif