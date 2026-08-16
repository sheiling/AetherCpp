#ifndef __ALGO_LED_METER_ERRORS_H__
#define __ALGO_LED_METER_ERRORS_H__

//basic error---------------------------------------------- 1-99
//#include "ci_errors.h"
#include "ci_basic_error.h"
#include "ci_frontend_modname.h"

#define ALGOLEDMETER__MISMATCH_LIGHTED_WH                   ERROR_CODE(ALGOMOD_ALGOLEDMETER,100) //点亮灯点的个数不一致
#define ALGOLEDMETER__DUPLICATE_ADD							ERROR_CODE(ALGOMOD_ALGOLEDMETER,101) //重复添加数据
#define ALGOLEDMETER__METAINFO_INCOMPLETED					ERROR_CODE(ALGOMOD_ALGOLEDMETER,102) //输入的元信息不全
#define ALGOLEDMETER__ERROR_MULTI_CHANNELS                  ERROR_CODE(ALGOMOD_ALGOLEDMETER,103) //输入的是多通道
#define ALGOLEDMETER__MISMATCH_SERIES                       ERROR_CODE(ALGOMOD_ALGOLEDMETER,104) //输入和前面系列不匹配
#define ALGOLEDMETER__INVALID_ALIGN_INPUT                   ERROR_CODE(ALGOMOD_ALGOLEDMETER,105) //Align函数point或radius参数无效
#define ALGOLEDMETER__AREA_MERGE_WHRONG                     ERROR_CODE(ALGOMOD_ALGOLEDMETER,106) //子区域宽高不对
#define ALGOLEDMETER__LIGHTDARK_RITO_ERROR                  ERROR_CODE(ALGOMOD_ALGOLEDMETER,107)  //亮暗点检测阈值错误
#define ALGOLEDMETER__ERROR_QUANTIY                         ERROR_CODE(ALGOMOD_ALGOLEDMETER,108)  //输入meter数量异常
#define ALGOLEDMETER__ERROR_COLROW                          ERROR_CODE(ALGOMOD_ALGOLEDMETER,109)  //meter拼接行列数异常
#define ALGOLEDMETER__ERROR_OPERATE_BUFFER                  ERROR_CODE(ALGOMOD_ALGOLEDMETER,110)  //meter业务操作记录数据错误

#endif