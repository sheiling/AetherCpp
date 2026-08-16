#ifndef __ALGO_SHOT_ERRORS_H__
#define __ALGO_SHOT_ERRORS_H__

//basic error---------------------------------------------- 1-99
//#include "ci_errors.h"
#include "ci_basic_error.h"
#include "ci_frontend_modname.h"

#define ALGOSHOT__INVALID_CAM_TYPE                          ERROR_CODE(ALGOMOD_ALGOSHOT,100) //相机类型错误    
#define ALGOSHOT__MISMATCH                                  ERROR_CODE(ALGOMOD_ALGOSHOT,101) //shot类型和数据不匹配
#define ALGOSHOT__NOT_IN_SAME_SET                           ERROR_CODE(ALGOMOD_ALGOSHOT,102) //数据不在相同的拍摄序列
#define ALGOSHOT__MISTMATCH_PATTERN_PARTITION_INFO          ERROR_CODE(ALGOMOD_ALGOSHOT,103)//数据中点亮的灯点宽高错误
#define ALGOSHOT__ERROR_PATTERN_INFO                        ERROR_CODE(ALGOMOD_ALGOSHOT,104) //Shot pattern信息不全/错误
#define ALGOSHOT__ERROR_DEVICE_INFO                         ERROR_CODE(ALGOMOD_ALGOSHOT,105) //Shot device信息不全/错误
#define ALGOSHOT__ERROR_PANEL_INFO                          ERROR_CODE(ALGOMOD_ALGOSHOT,106) //Shot panel信息不全/错误
#define ALGOSHOT__ERROR_CAPTURE_INFO                        ERROR_CODE(ALGOMOD_ALGOSHOT,107) //Shot capture信息不全/错误
#define ALGOSHOT__ERROR_PARTITION_INFO                      ERROR_CODE(ALGOMOD_ALGOSHOT,108) //Shot partition信息不全/错误
#define ALGOSHOT__ERROR_MAT_TYPE                            ERROR_CODE(ALGOMOD_ALGOSHOT,109) //Shot mat类型不对
#define ALGOSHOT__ERROR_CIE_TYPE                            ERROR_CODE(ALGOMOD_ALGOSHOT,110) //Shot CIEtype类型不对
#define ALGOSHOT__ERROR_COLOR_INFO                          ERROR_CODE(ALGOMOD_ALGOSHOT,111) //Shot color信息不全/错误
#define ALGOSHOT__ERROR_BAYER_TYPE                          ERROR_CODE(ALGOMOD_ALGOSHOT,112) //Shot bayer信息错误
#define ALGOSHOT__ERROR_RECT                                ERROR_CODE(ALGOMOD_ALGOSHOT,113) //Shot 区域信息错误
#define ALGOSHOT__ERROR_OPERATE_BUFFER                      ERROR_CODE(ALGOMOD_ALGOSHOT,114) //Shot 业务操作记录数据错误



#endif