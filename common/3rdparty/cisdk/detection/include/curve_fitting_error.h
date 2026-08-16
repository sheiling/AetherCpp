#ifndef __CURVE_FITTING_ERROR_H__
#define __CURVE_FITTING_ERROR_H__

//basic error---------------------------------------------- 1-99
//#include "ci_errors.h"
#include "ci_basic_error.h"
#include "ci_frontend_modname.h"
#include "ci_detection_modname.h"

#define ALGOCURVEFITTING__ERROR_INVALID_CURVEF                    ERROR_CODE(ALGOMOD_CURVE_FITTING,100) //曲面系数中存在inf或nan
#define ALGOCURVEFITTING__ERROR_INVALID_PULLPOINT                   ERROR_CODE(ALGOMOD_CURVE_FITTING,101) //手拉边界点坐标异常


#endif