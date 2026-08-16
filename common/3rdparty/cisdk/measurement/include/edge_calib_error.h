#ifndef __EDGE_CALIB_ERROR_H__
#define __EDGE_CALIB_ERROR_H__

// basic error---------------------------------------------- 1-99
//#include "ci_errors.h"
#include "ci_basic_error.h"
#include "ci_frontend_modname.h"

#define ALGOEDGECALIB__ERROR_SIZE_EDGE_PAIR ERROR_CODE(ALGOMOD_EDGE_CALIB, 100) //匹配对的大小不对
#define ALGOEDGECALIB__ERROR_FILE_NUM ERROR_CODE(ALGOMOD_EDGE_CALIB, 101)       //公共系数生成文件数量不对
#define ALGOEDGECALIB__ERROR_SIZE_SCREEN ERROR_CODE(ALGOMOD_EDGE_CALIB, 102)    //拼接区域模组数量不对
#define ALGOEDGECALIB__ERROR_SIZE_SCREEN_LastBlock                                                                     \
    ERROR_CODE(ALGOMOD_EDGE_CALIB, 103) //拼接区域最后一行/列模组行、列数不对
#define ALGOEDGECALIB__ERROR_RATIO ERROR_CODE(ALGOMOD_EDGE_CALIB, 104)          //二次补偿衰减比例
#define ALGOEDGECALIB__ERROR_INVALID_EDGE ERROR_CODE(ALGOMOD_EDGE_CALIB, 105)   //生成的edge包含inf或nan
#define ALGOEDGECALIB__ERROR_EDGES_ABNORMAL ERROR_CODE(ALGOMOD_EDGE_CALIB, 106) //异常边缘过多
#define ALGOEDGECALIB__ERROR_DATA_ZERO ERROR_CODE(ALGOMOD_EDGE_CALIB, 107) //数据异常，有除零的情况发生
#define ALGOEDGECALIB__ERROR_STATE_EMPTY ERROR_CODE(ALGOMOD_EDGE_CALIB, 108) //灯点的state为空，不满足异性箱体处理要求
#define ALGOEDGECALIB__ERROR_CIE_TYPE ERROR_CODE(ALGOMOD_EDGE_CALIB, 109)  //输入meter的CIE_TYPE错误
#define ALGOEDGECALIB__ERROR_METER_NUM ERROR_CODE(ALGOMOD_EDGE_CALIB, 110) //输入meter数量错误
#define ALGOEDGECALIB__ERROR_RATIO_NUM ERROR_CODE(ALGOMOD_EDGE_CALIB, 111) //输入比例数量错误

#endif