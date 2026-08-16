#ifndef _LED_DIE_MAP_FUSION_ERROR_H_
#define _LED_DIE_MAP_FUSION_ERROR_H_

#include "ci_basic_error.h"
#include "ci_frontend_modname.h"
#include "ci_detection_modname.h"

#define ALGOERROR_LED_DIE_MAP_FUSION_ERROR_TRANSMAT_EMPTY                            ERROR_CODE(ALGOMOD_LED_DIE_MAP_FUSION, 101) // 转换矩阵为空
#define ALGOERROR_LED_DIE_MAP_FUSION_ERROR_DATA_NOT_CORRESPOND_CAM_POS_NUM           ERROR_CODE(ALGOMOD_LED_DIE_MAP_FUSION, 102) // 数据数目与相机采集位置数目不匹配
#define ALGOERROR_LED_DIE_MAP_FUSION_ERROR_DATA_CAN_NOT_FILL_IN_PANEL                ERROR_CODE(ALGOMOD_LED_DIE_MAP_FUSION, 103) // 数据无法填入灯板空间
#define ALGOERROR_LED_DIE_MAP_ROW_OR_COL_NUM_ERROR                                   ERROR_CODE(ALGOMOD_LED_DIE_MAP_FUSION, 104) // 数据行列数有误


#endif // !_PAD_FINE_LOCATOR_ERROR_H_
