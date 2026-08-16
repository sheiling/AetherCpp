#ifndef __DEMURA_LAYER_SELECTOR_ERRORS_H__
#define __DEMURA_LAYER_SELECTOR_ERRORS_H__

// basic error---------------------------------------------- 1-99
#include "ci_basic_error.h"
#include "ci_frontend_modname.h"
#include "ci_detection_modname.h"

#define ALGO_LUMINANCE_LOSS_LOWER_THAN_LOWEREST_GRAY(module) ERROR_CODE(module, 100) //衰减后亮度小于采集的最低灰阶。灰阶数增多
#define ALGO_CAPTURE_GRAY_NOT_ENOUGH(module) ERROR_CODE(module, 101) //采集的灰阶数小于1




#endif