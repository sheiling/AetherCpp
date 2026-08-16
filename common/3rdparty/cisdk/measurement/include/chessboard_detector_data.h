#ifndef __CHESSBOARD_DETECTOR_DATA_H__
#define __CHESSBOARD_DETECTOR_DATA_H__

#include "ci_data.h"
#include "algo_mat_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	enum ChessBoardMethod
	{
		CHESSBOARD_RECTANGLE_METHOD = 0,	//常规方式，仅做框识别
		CHESSBOARD_SPHERICAL_METHOD = 1,	//球型屏识别
		CHESSBOARD_RECTANGLE_PSF_METHOD = 2,	//常规方式，识别框，且提取PSF
	};
	
	enum ChessBoardConvexHullDetectionSiwtch
	{
		ConvexHullDetectionOpen = 0,		//做凸包检测
		ConvexHullDetectionClose = 1,		//不做凸包检测
	};


	typedef struct {
		AlgoMat located_quadrangle; //区域
		int located_type;  //0为常规屏棋盘格检测结果，1为球形屏检测结果
		AlgoMat mask; //ALGO_8UC1
	}AlgoChessBoard;



    /************************************
    @brief  : 棋盘格信息
    @note   : 
    @Author : YangShangqi@2023/08/24
    ************************************/
    typedef struct {
        int single_area_size_out;   ///> 非最后行列棋盘格尺寸
        int end_col_width_out;      ///> 最后一列棋盘格宽
        int end_row_height_out;     ///> 最后一行棋盘格高
        AlgoSize led_rect_size;     ///> 球形屏拉直成直角梯形后的最小外接矩形尺寸
        AlgoSize checkerboard_size_out; ///> 棋盘格block尺寸
        AlgoMat  blocks_rc;             ///> led 每行灯点每个block灯点数
    } ChessboardInfo; 


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // 
