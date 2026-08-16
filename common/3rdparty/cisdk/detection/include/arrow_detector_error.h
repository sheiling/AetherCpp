#ifndef __ARROW_DETECTOR_ERROE_H__
#define __ARROW_DETECTOR_ERROE_H__

//basic error---------------------------------------------- 1-99
#include "ci_basic_error.h"
#include "ci_frontend_modname.h"
#include "ci_detection_modname.h"

#define ALGOERROR_ARROW_DETECTOR_MATCH_FAILED(module)             ERROR_CODE(module,201)  //检测失败
#define ALGOERROR_ARROW_DETECTOR_MODEL_TYPE_MISMATCH(module)      ERROR_CODE(module,202)  //模板类型不匹配

#endif