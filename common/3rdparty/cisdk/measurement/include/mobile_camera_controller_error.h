#ifndef __MOBILE_CAMERA_CONTROLLER_HPP_ERROR_H__
#define __MOBILE_CAMERA_CONTROLLER_HPP_ERROR_H__

//basic error---------------------------------------------- 1-99
#include "ci_basic_error.h"
#include "ci_frontend_modname.h"
//#include "ci_errors.h"


// coeff --------------------------------------------------

#define ALGOCAMERAPARA__ERROR_INPUT_PARAMETER_WRONG             ERROR_CODE(ALGOMOD_MOBILE_CAMERA_CONTROLLER,100)  //输入的参数错误
#define ALGOCAMERAPARA__ERROR_INPUT_COLOR_WRONG                 ERROR_CODE(ALGOMOD_MOBILE_CAMERA_CONTROLLER,101)  //输入的屏体颜色不正确
#define ALGOCAMERAPARA__ERROR_INPUT_QUALITY_RANGE_WRONG         ERROR_CODE(ALGOMOD_MOBILE_CAMERA_CONTROLLER,102)  //输入的参数范围不正确
#define ALGOCAMERAPARA__ERROR_ROI_NOTFIND_ERROR                 ERROR_CODE(ALGOMOD_MOBILE_CAMERA_CONTROLLER,103)  //屏体ROI区域未找到或屏体区域成像占比过小
#define ALGOCAMERAPARA__ERROR_RESULT_ERROR						ERROR_CODE(ALGOMOD_MOBILE_CAMERA_CONTROLLER,104)  //当前条件下AE无法成功


#endif