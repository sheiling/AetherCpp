#ifndef __CURVE_CALIB_ERROR_H__
#define __CURVE_CALIB_ERROR_H__

//basic error---------------------------------------------- 1-99
//#include "ci_errors.h"
#include "ci_basic_error.h"
#include "ci_frontend_modname.h"

#define ALGOCURVECALIB__ERROR_CIRCLE_NUM                  ERROR_CODE(ALGOMOD_CURVE_CALIB, 100) //标定圆个数错误
#define ALGOCURVECALIB__ERROR_PATITION_SIZE               ERROR_CODE(ALGOMOD_CURVE_CALIB, 101) //分区大小错误
#define ALGOCURVECALIB__ERROR_RADIUS                      ERROR_CODE(ALGOMOD_CURVE_CALIB, 102) //输入半径错误
#define ALGOCURVECALIB__ERROR_FILE_NUM                    ERROR_CODE(ALGOMOD_CURVE_CALIB, 103) //公共系数生成文件数量不对
#define ALGOCURVECALIB__ERROR_CIE_TYPE_ERROR              ERROR_CODE(ALGOMOD_CURVE_CALIB, 104) //cie type err
#define ALGOCURVECALIB__ERROR_COLOR_MISMATCH              ERROR_CODE(ALGOMOD_CURVE_CALIB, 105) //meter的颜色不一致
#define ALGOCURVECALIB__ERROR_INVALID_CURVEF              ERROR_CODE(ALGOMOD_CURVE_CALIB, 106) //曲面数据存在nan或inf
#define ALGOCURVECALIB__ERROR_INVALID_PULLPOINT           ERROR_CODE(ALGOMOD_CURVE_CALIB, 107) //手拉曲面的坐标点异常
#define ALGOCURVECALIB__ERROR_STATE_EMPTY				  ERROR_CODE(ALGOMOD_CURVE_CALIB, 108) //state为空
#define ALGOCURVECALIB__ERROR_INVALID_MODESIZE		      ERROR_CODE(ALGOMOD_CURVE_CALIB, 109) //灯板尺寸不能被meter分辨率整除


#endif