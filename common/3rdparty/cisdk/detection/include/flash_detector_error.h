#ifndef __FLASH_DETECTOR_ERROR_H__
#define __FLASH_DETECTOR_ERROR_H__


#include "ci_basic_error.h"
#include "ci_frontend_modname.h"
#include "ci_detection_modname.h"

#define ALGOFLASHDETECTOR_INDEX_ERROR                           ERROR_CODE(ALGOMOD_FLASH_DETECTOR,100) // 输入索引错误
#define ALGOFLASHDETECTOR_ROES_COLS_ERROR                       ERROR_CODE(ALGOMOD_FLASH_DETECTOR,101) // 输入灯板行列数错误
#define ALGOFLASHDETECTOR_PANEL_ID_ERROR						ERROR_CODE(ALGOMOD_FLASH_DETECTOR,102) // 输入灯板数量错误
#define ALGOFLASHDETECTOR_FILE_PATH_ERROR						ERROR_CODE(ALGOMOD_FLASH_DETECTOR,103) // 文件路径错误


#endif