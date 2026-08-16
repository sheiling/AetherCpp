#ifndef __DL_CHIP_DETECTOR_ERROR_H__
#define __DL_CHIP_DETECTOR_ERROR_H__

//basic error---------------------------------------------- 1-99
#include "ci_basic_error.h"
#include "ci_frontend_modname.h"
#include "ci_detection_modname.h"
#define ALGOMOD_DL_CHIP_DETECTOR_DATA_EMPTY								 ERROR_CODE(ALGOMOD_DL_CHIP_DETECTOR,101) //数据为空
#define ALGOMOD_DL_CHIP_DETECTOR_INPUT_MISMATCH							 ERROR_CODE(ALGOMOD_DL_CHIP_DETECTOR,102) //数据不匹配
#define ALGOMOD_DL_CHIP_DETECTOR_ROI_IMG_SIZE_WRONG                      ERROR_CODE(ALGOMOD_DL_CHIP_DETECTOR,103) //ROI区域超出图像范围 
#define ALGOMOD_DL_CHIP_DETECTOR_SOLDER_MISSING							 ERROR_CODE(ALGOMOD_DL_CHIP_DETECTOR,104) //焊盘缺失且无芯片，无法检测
#define ALGOMOD_DL_CHIP_DETECTOR_NO_TARGET								 ERROR_CODE(ALGOMOD_DL_CHIP_DETECTOR,105) //此处无目标


#define ALGOMOD_DL_CHIP_DETECTOR_CHIP_EMPTY								 ERROR_CODE(ALGOMOD_DL_CHIP_DETECTOR,200)  //检测到焊盘但是此处无芯片
#endif