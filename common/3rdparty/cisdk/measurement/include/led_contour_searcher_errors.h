#ifndef __LED_CONTOUR_SEARCHER_ERRORS_H__
#define __LED_CONTOUR_SEARCHER_ERRORS_H__

//basic error----------------------------------------------1-99
//#include "ci_errors.h"
#include "ci_basic_error.h"
#include "ci_frontend_modname.h"


#define ALGOLEDCONTOUR__ERROR_EMPTY_SHOT                          ERROR_CODE(ALGOMOD_LED_COUTOUR_SEARCHER,100)  //输入的shot信息为空，请检查输入的shot信息,可能存在如下两种情况：1)shot的handle的指针为空；2)shot内的图像为空；
#define ALGOLEDCONTOUR__ERROR_EMPTY_LED_POINT                     ERROR_CODE(ALGOMOD_LED_COUTOUR_SEARCHER,101)  //输入的点定位信息为空，请检查输入的点定位信息,可能存在如下三种情况:1)点定位handle的指针为空；2)点定位内loc/state为空；3)点定位内search_radius参数为空；
#define ALGOLEDCONTOUR__ERROR_EMPTY_LEDLAYOUT                     ERROR_CODE(ALGOMOD_LED_COUTOUR_SEARCHER,102)  //输入的灯点排布信息为空，请检查输入的灯点排布信息LEDLayout；
#define ALGOLEDCONTOUR__ERROR_WRONG_THREAD_NUMS                   ERROR_CODE(ALGOMOD_LED_COUTOUR_SEARCHER,103)  //输入的CPU并行线程数错误，请检查是否输入的值小于1或设置的并行线程数超过了CPU最大线程数
#define ALGOLEDCONTOUR__ERROR_WRONG_METHOD                        ERROR_CODE(ALGOMOD_LED_COUTOUR_SEARCHER,104)  //输入的搜索框方法错误，请检查方法参数是否大于3或小于0；												
#define ALGOLEDCONTOUR__ERROR_WRONG_SHRINK                        ERROR_CODE(ALGOMOD_LED_COUTOUR_SEARCHER,105)  //输入的灯点框的缩放参数错误，请检查参数是否在如下范围内：1)宽度范围缩放：大于0.01f,小于2.f;2)高度范围缩放：大于0.01f,小于2.f;
#define ALGOLEDCONTOUR__ERROR_EMPTY_CONTOUR_RESULT                ERROR_CODE(ALGOMOD_LED_COUTOUR_SEARCHER,106)  //本handle内的contour信息为空，请检查输入的contour信息，可能存在如下两种情况：1)contour的handle的指针为空；2)contour内的结果为空；
#define ALGOLEDCONTOUR__ERROR_CONTOUR_RESULT_OUTOF_IMG            ERROR_CODE(ALGOMOD_LED_COUTOUR_SEARCHER,107)  //计算得到的contour结果超出了图像边界，请检查ROI是否距离图像边界过紧或紧贴于图像上；


#endif