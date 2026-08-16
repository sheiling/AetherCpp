#ifndef __COLORIMETER_CALIB_ERROR_H__
#define __COLORIMETER_CALIB_ERROR_H__

//basic error---------------------------------------------- 1-99
//#include "ci_errors.h"
#include "ci_basic_error.h"
#include "ci_frontend_modname.h"


//#define ALGOCOLORIMETER__ERROR_RATIO_ERROR                           ERROR_CODE(ALGOMOD_COLORIMETER_CALIB,100) //光路修正缩放比例小于1或大于64
#define ALGOCOLORIMETERCALIB__ERROR_SPECKLEREGION_EXIST                ERROR_CODE(ALGOMOD_COLORIMETER_CALIB,100) //箱体多光道标定输入数据存在亮暗块，会导致标定系数不准确
#define ALGOCOLORIMETERCALIB__ERROR_INVALID_COLORIMETER                ERROR_CODE(ALGOMOD_COLORIMETER_CALIB,101) //标定系数存在nan或inf
#define ALGOCOLORIMETERCALIB__ERROR_MISMATCH_COLORIMETER_FILTER        ERROR_CODE(ALGOMOD_COLORIMETER_CALIB,102) //这个错误表明，标定文件的分量都不统一，
                                                                                                                 //比如输入3个标定文件，一个存储的是x_data(其余分量为空),一个存储的是y_data,一个存储的是z_data，这种情况就会报错

#endif