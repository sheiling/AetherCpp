#ifndef __CHESSBOARD_DETECTOR_ERRORS_H__
#define __CHESSBOARD_DETECTOR_ERRORS_H__

//basic error---------------------------------------------- 1-99
//#include "ci_errors.h"
#include "ci_basic_error.h"
#include "ci_frontend_modname.h"

//#define ALGOCHESSBOARD__INPUT_NULLPTR							ERROR_CODE(ALGOMOD_CHESSBOARD_DETECTOR,100) //该错误码以进行细化
//#define ALGOCHESSBOARD__FAILED_FIND_AREA						ERROR_CODE(ALGOMOD_CHESSBOARD_DETECTOR,101) //未找到ROI。 该错误码以进行细化。
#define ALGOCHESSBOARD__FAILED_FIND_MATCHED_AMOUNT_AREA			ERROR_CODE(ALGOMOD_CHESSBOARD_DETECTOR,102) //未找到匹配数量的区域。如实际棋盘格采集的块数与输入的棋盘格个数不一致？或成像质量或与糊导致棋盘格之间粘连
#define ALGOCHESSBOARD__ERROR_NO_LED_SIZE						ERROR_CODE(ALGOMOD_CHESSBOARD_DETECTOR,103) //未获取到点亮灯点分辨率，可能是没有先SetImage shot。
#define ALGOCHESSBOARD__INPUT_INDEX_OVERSTEPPED					ERROR_CODE(ALGOMOD_CHESSBOARD_DETECTOR,104) //找棋盘格位置超过块数的边界（找结果），在提取光通量时可能爆出该问题。该问题一般不会出现
#define ALGOCHESSBOARD__CHESS_CORNER_WRONG						ERROR_CODE(ALGOMOD_CHESSBOARD_DETECTOR,105) //某个棋盘格的位置角点非矩形/四边形，形变过大。一般由棋盘格识别结果错位导致-如棋盘格完全错位。该问题在提取光通量时出现。或因为棋盘格排序出现异常，如图像曲率过大，棋盘格透视变化后同列格子非垂直分布。
#define ALGOCHESSBOARD__ERROR_AREA_VALID_HEIGHT_WIDTH_ERROR		ERROR_CODE(ALGOMOD_CHESSBOARD_DETECTOR,106) //某个棋盘格区域识别结果的有效行/列数量为0（真实高度/宽度不小于当前棋盘格高度/宽度的0.75倍即为有效高度），导致该位置无法提取有效果的光通量。该问题在计算棋盘格PSF时出现，可能是边缘处成像成明显非矩形形状（一边长一边短），基本不会出现，若出现建议成像调锐利。
#define ALGOCHESSBOARD__ERROR_LED_SIZE_WRONG					ERROR_CODE(ALGOMOD_CHESSBOARD_DETECTOR,107) //棋盘格规则（尺寸、各块）与输入的灯点分辨率信息不相符.
//#define ALGOCHESSBOARD__ORDER_WRONG							ERROR_CODE(ALGOMOD_CHESSBOARD_DETECTOR,108) //调用的顺序不对
#define ALGOCHESSBOARD__WRONG_MODULE_SIZE						ERROR_CODE(ALGOMOD_CHESSBOARD_DETECTOR,109) //校正区域灯点分辨率不是通过接口ChessboardDetectorSetModuleSize设置的mozu_size的整数倍。在设置了模组尺寸做精细修边时启用该判断。
#define ALGOCHESSBOARD__WRONG_DEGAP_SWITCH						ERROR_CODE(ALGOMOD_CHESSBOARD_DETECTOR,110) //设置修缝开关时入参错误，设置凸包边参数无效。
#define ALGOCHESSBOARD__ERROR_CHESS_SHOT_NOTHING				ERROR_CODE(ALGOMOD_CHESSBOARD_DETECTOR,111) //棋盘格shot未识别到任何内容。可能采集的为黑图
#define ALGOCHESSBOARD__ERROR_SINLE_CHESS_INVALID				ERROR_CODE(ALGOMOD_CHESSBOARD_DETECTOR,112) //无法通过4个点拟合某个棋盘格，可能存在显示异常的单个棋盘格。（基本不会出现）。
#define ALGOCHESSBOARD__ERROR_CHESS_SIZE_TOO_SMALL				ERROR_CODE(ALGOMOD_CHESSBOARD_DETECTOR,113) //单棋盘格尺寸过小，不支持做PSF（仅在计算PSF时校验）。单个棋盘格的分辨率不小于15x15.。
#define ALGOCHESSBOARD__ERROR_MASK_ERROR						ERROR_CODE(ALGOMOD_CHESSBOARD_DETECTOR,114) //识别出来的mask错误，导致有效的rect内无东西。可能与周围有严重混光有管

#endif