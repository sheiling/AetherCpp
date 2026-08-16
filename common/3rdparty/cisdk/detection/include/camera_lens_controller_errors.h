#ifndef __CAMERA_LENS_CONTROLLER_ERRORS_H__
#define __CAMERA_LENS_CONTROLLER_ERRORS_H__

// basic error---------------------------------------------- 1-99
#include "ci_basic_error.h"
#include "ci_frontend_modname.h"
#include "ci_detection_modname.h"


#define CAMERA_LENS_CONTROLLER__ERROR_RECT                                     \
    ERROR_CODE(ALGOMOD_CAMERA_LENS_CONTROLLER, 101) // roi区域信息错误

#define CAMERA_LENS_CONTROLLER__AE_FAIL                                        \
    ERROR_CODE(ALGOMOD_CAMERA_LENS_CONTROLLER, 102) // AE自动曝光失败错误

#define CAMERA_LENS_CONTROLLER__AE_FIND_INIT_POINT_FAIL                        \
    ERROR_CODE(ALGOMOD_CAMERA_LENS_CONTROLLER,                                 \
               103) // AE_失败 : 大步长搜索失败，无法找到初始点

#define CAMERA_LENS_CONTROLLER__AE_INPUT_IMG_ERR                               \
    ERROR_CODE(ALGOMOD_CAMERA_LENS_CONTROLLER, 104) // AE_失败 : 输入图像错误

#define CAMERA_LENS_CONTROLLER__AE_INPUT_DETEC_IMG_ERR                         \
    ERROR_CODE(ALGOMOD_CAMERA_LENS_CONTROLLER,                                 \
               105) // AE_失败 : 输入试探图像错误

#define CAMERA_LENS_CONTROLLER__AE_EXCEEDED_MAX_ITER_NUM                       \
    ERROR_CODE(ALGOMOD_CAMERA_LENS_CONTROLLER,                                 \
               106) // AE_失败 : 超出最大迭代步数

#define CAMERA_LENS_CONTROLLER__AE_EXCEEDED_MAX_PARA                           \
    ERROR_CODE(ALGOMOD_CAMERA_LENS_CONTROLLER,                                 \
               107) // AE_失败 : 预估内参超出极限，输出极限相机参数


#endif