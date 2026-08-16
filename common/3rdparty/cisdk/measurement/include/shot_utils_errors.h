#ifndef __SHOT_UTILS_ERRORS_H_
#define __SHOT_UTILS_ERRORS_H_

#include "ci_basic_error.h"
#include "ci_frontend_modname.h"

#define SHOTUTILS__ERROR_CONTOUR_NUM		          ERROR_CODE(ALGOMOD_SHOT_UTILS, 100) //异形屏体对齐计算的图像屏体轮廓数量不为1							  
#define SHOTUTILS__ERROR_MIN_DISTANCE		          ERROR_CODE(ALGOMOD_SHOT_UTILS, 101) //异形屏体与标准箱体之间最近角的距离过大，请检查是否对齐或者手动输入相对位置	


#endif