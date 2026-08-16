#ifndef __MAPPER_UTILS_ERRORS_H__
#define __MAPPER_UTILS_ERRORS_H__

//basic error---------------------------------------------- 1-99
//#include "ci_errors.h"
#include "ci_basic_error.h"
#include "ci_frontend_modname.h"

#define ALGOMAPPERUTILS__SPHERICAL_ROW_LED_NUM_TOO_FEW			ERROR_CODE(ALGOMOD_MAPPER_UTILS, 100) // 球形屏某行真实灯点个数过少（少于2个）
#define ALGOMAPPERUTILS__SUBAREA_LED_NUM_TOO_FEW				ERROR_CODE(ALGOMOD_MAPPER_UTILS, 101) // 此错误码出现于接口MapperUtilsRectifyPos中，可能的原因①由于包边或者区域内死灯数目超边界，导致区域划分后子区域内点亮灯点个数过少（少于5个）；②畸变一致的参数设置过小，导致子区域本身就较小
#define ALGOMAPPERUTILS__POINTMAPPER_EMPTY						ERROR_CODE(ALGOMOD_MAPPER_UTILS, 102) // 输入的point_mapper信息为空，请检查输入的point_mapper句柄是否为空指针，或者句柄内的结果为空
#define ALGOMAPPERUTILS__INTERPOLATION_METHOD_ERROR				ERROR_CODE(ALGOMOD_MAPPER_UTILS, 103) // 输入的interpolation_method不符合要求，请参考include\mapper_utisl_data.h的数据结构检查入参
#define ALGOMAPPERUTILS__PARTTION_ERROR							ERROR_CODE(ALGOMOD_MAPPER_UTILS, 104) // 输入的shot分区信息有误，请检查分区rect是否处于屏体screen_wh内
#define ALGOMAPPERUTILS__LEDPANEL_SIZE_NOT_SAME					ERROR_CODE(ALGOMOD_MAPPER_UTILS, 106) // 输入的灯板分辨率不一致，请检查屏体灯板信息和入参是否符合要求
#define ALGOMAPPERUTILS__SHOT_EMPTY								ERROR_CODE(ALGOMOD_MAPPER_UTILS, 107) // 输入的shot信息为空，请检查输入的shot句柄是否为空指针，或者句柄内的结果为空
#define ALGOMAPPERUTILS__LEDLAYOUT_MISMATCH						ERROR_CODE(ALGOMOD_MAPPER_UTILS, 108) // 输入的灯点排布信息错误与期望值错误
#define ALGOMAPPERUTILS__ERROR_IMGSIZE_MISHMATCH_ROI_OR_MAP		ERROR_CODE(ALGOMOD_MAPPER_UTILS, 109) // 使用标准Map结果生成新Map结果时，输入的shot图像大小与roi大小不匹配或与Map结果不匹配
#define ALGOMAPPERUTILS__ERROR_POINTMAPPER_INFO					ERROR_CODE(ALGOMOD_MAPPER_UTILS, 110) // 输入的mapper结果中不符合结果，若调用标准Map生成新Map，则需行或列都大于2
#define ALGOMAPPERUTILS__SHOT_CAM_INFO_EMPTY					ERROR_CODE(ALGOMOD_MAPPER_UTILS, 111) // 输入的shot中，cam信息为空
#define ALGOMAPPERUTILS__ROI_EMPTY								ERROR_CODE(ALGOMOD_MAPPER_UTILS, 112) // 输入的roi信息为空，请检查输入的roi句柄是否为空指针，或者句柄内的结果为空
#define ALGOMAPPERUTILS__SHOT_DEVICE_MISMATCH                   ERROR_CODE(ALGOMOD_MAPPER_UTILS, 113) // 此错误码出现于接口MapperUtilsRectifyPos中，原因为输入的shot中，device对应的相机信息未测试，当前仅支持3200&TPM6100
#define ALGOMAPPERUTILS__CORNER_NO_EXIST						ERROR_CODE(ALGOMOD_MAPPER_UTILS, 114) // mapper的边缘灯点状态为no exist。针对mapper，正常不可能出现的

#endif