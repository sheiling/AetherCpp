#ifndef __TRANSFORMER_ERRORS_H__
#define __TRANSFORMER_ERRORS_H__

//basic error---------------------------------------------- 1-99
//#include "ci_errors.h"
#include "ci_basic_error.h"
#include "ci_frontend_modname.h"

#define ALGOTRANSFORM__TRANSFORM_MEMBER_ERROR     ERROR_CODE(ALGOMOD_GEOMETRY_TRANSFORMER,100)  //transform类数据成员错误
#define ALGOTRANSFORM__ERROR_CORNERS_NUM_NOT_4    ERROR_CODE(ALGOMOD_GEOMETRY_TRANSFORMER,101) //角点数量必须是4
#define ALGOTRANSFORM__PASSWORD_ERR               ERROR_CODE(ALGOMOD_GEOMETRY_TRANSFORMER,102) //密码错误
#define ALGOTRANSFORM__RESULT_IS_NAN_INF          ERROR_CODE(ALGOMOD_GEOMETRY_TRANSFORMER,103) //转换矩阵包含nan or inf，请检查输入角点是否异常



#endif