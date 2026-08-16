#ifndef __ROI_DETECTOR_ERRORS_H__
#define __ROI_DETECTOR_ERRORS_H__

//basic error---------------------------------------------- 1-99
//#include "ci_errors.h"
#include "ci_basic_error.h"
#include "ci_frontend_modname.h"


//roi_detector_errors---------------------------------------
#define ALGOROI__FAILED_FIND_AREA                           ERROR_CODE(ALGOMOD_ROI_DETECTOR,100) //未找到ROI
#define ALGOROI__ERROR_CORNERS_NUM_NOT_4                    ERROR_CODE(ALGOMOD_ROI_DETECTOR,101) //输入的角点个数不为4
#define ALGOROI__NEED_COMPUTE_TRANSEMAT_FIRST               ERROR_CODE(ALGOMOD_ROI_DETECTOR,102)//必须先计算变换矩阵
#define ALGOROI__CORNER_OUT_OF_IMAGE                        ERROR_CODE(ALGOMOD_ROI_DETECTOR,103)//角点超出图像范围
#define ALGOROI__CANNOT_FIND_4_CORNERS                      ERROR_CODE(ALGOMOD_ROI_DETECTOR,104)//未找到4个角点
#define ALGOROI__SAME_CORNER_POINTS                         ERROR_CODE(ALGOMOD_ROI_DETECTOR,105)//4个角点中2个相同
#define ALGOROI__SPLICE_POSITION_AREA                       ERROR_CODE(ALGOMOD_ROI_DETECTOR,106)//未按顺序寻找各块拼接屏左上角坐标
#define ALGOROI__SPLICE_NUMBER_AREA                         ERROR_CODE(ALGOMOD_ROI_DETECTOR,107)//找到拼接屏数量有误

//手机亮度修缝错误码
#define ALGOROI_ERROR_PANEL_NUMBER                          ERROR_CODE(ALGOMOD_ROI_DETECTOR,108) //找到黑色矩形(模组)数量有误

#define ALGOROI__ERROR_NUMBER_COLOR                         ERROR_CODE(ALGOMOD_ROI_DETECTOR,109)//待分区颜色数量有误
#define ALGOROI__ERROR_MODE                                 ERROR_CODE(ALGOMOD_ROI_DETECTOR,110)//分区模式有误

#define ALGOROI__ERROR_THERMALIMAGER_DATATYPE               ERROR_CODE(ALGOMOD_ROI_DETECTOR,111)//区域定位时thermal_imager数据类型错误，必须为ThermalImagerDataType_RAW



#endif