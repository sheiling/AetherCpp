#ifndef __CHESSBOARD_DETECTOR_H__
#define __CHESSBOARD_DETECTOR_H__

#include "ci_define.h"
#include "ci_declare.h"
#include "chessboard_detector_errors.h"
#include "chessboard_detector_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus



	C_API AlgoError C_CALL ChessboardDetectorCreateHandle(AlgoHandle* out_chessboard_detector_handle);


	///************************************
	//@brief  : set img
	//@note   :
	//@param  : chessboard_detector_handle    有效的棋盘格检测句柄
	//@param  : in_chessboard_shot			  有效点的棋盘格图像shot句柄，经过拉齐后的shot
	//@param  : in_img_shot					  有效点的校正图像shot句柄，经过拉齐后的shot
	//@return	: AlgoError
	//@Author : tianbo@2022/04/22
    //************************************/
	C_API AlgoError C_CALL ChessboardDetectorSetImage(AlgoHandle chessboard_detector_handle, const AlgoHandle in_chessboard_shot, const AlgoHandle in_img_shot);



	///************************************
	//@brief  : set 参数
	//@note   :
	//@param  : chessboard_detector_handle    有效的棋盘格检测句柄
	//@param  : in_chessboard_size			  棋盘格的块数size
	//@return	: AlgoError
	//@Author : tianbo@2022/04/22
	//************************************/
	C_API AlgoError C_CALL ChessboardDetectorSetParam(AlgoHandle chessboard_detector_handle,  AlgoSize in_chessboard_size);


	///************************************
	//@brief  : set 参数
	//@note   : 如果不set，默认不做精确识别操作
	//@param  : chessboard_detector_handle    有效的棋盘格检测句柄
	//@param  : in_module_size				  设置单个模组的大小，用于精确识别边缘
	//@return	: AlgoError
	//@Author :  dulonghui&2023.08.27
	//************************************/
	C_API AlgoError C_CALL ChessboardDetectorSetModuleSize(AlgoHandle chessboard_detector_handle, AlgoSize in_module_size);



	/************************************
	@brief  : 选择是否进行凸包检测（默认进行），数码相机修缝必须设置为不进行凸包检测；
	@note   : 凸包检测只适用于桶形畸变，因为数码相机畸变会随着焦段变化而变化，必须设为不进行凸包检测；
	@param  : chessboard_detector_handle			有效的棋盘格检测句柄
	@param  : in_convex_hull_detection_switch		凸包检测开关，修缝使用ConvexHullDetectionClose = 1（不做凸包检测）
	@return	: AlgoError
	@Author : by GuoHao, at 2024.05.20.
	************************************/
	C_API AlgoError C_CALL ChessboardDetectorSetConvexHullDetectionSiwtch(AlgoHandle chessboard_detector_handle, ChessBoardConvexHullDetectionSiwtch in_convex_hull_detection_switch);
	


    /************************************
    @brief  :  set 参数(Transformer 接口)
    @note   :
                1）当默认参数均为0时，为球形屏SetParam
                2) 当默认参数不为0时，为普通屏幕SetParam
    @param  : chessboard_detector_handle    有效的棋盘格检测句柄
    @param  : trans                         有效的transformer句柄
    @param  : in_chessboard_size			棋盘格的块数size
    @param  : single_area_size_in = 0       非最后一行列，棋盘格单块尺寸
    @param  : end_col_width_in = 0          棋盘格最后一列的宽
    @param  : end_row_height_in = 0         棋盘格最后一行的高
    @return : AlgoError
    @Author : YangShangqi@2022/11/22
    ************************************/
    C_API AlgoError C_CALL ChessboardDetectorSetParamTrans(AlgoHandle chessboard_detector_handle, AlgoHandle in_trans_handle, AlgoSize in_chessboard_size, int in_single_area_size = 0, int in_end_col_width = 0, int in_end_row_height = 0);



	///************************************
	//@brief  : 基于球形屏的棋盘格检测
	//@note   :
	//@param  : chessboard_detector_handle    有效的棋盘格检测句柄
	//@return	: AlgoError
	//@Author : tianbo@2022/04/22
	//************************************/
	C_API AlgoError C_CALL ChessboardDetectorFindBorderForSpherical(AlgoHandle chessboard_detector_handle);



    /************************************
    @brief  : 正常屏的棋盘格检测
    @note   : 注释
    @param  : chessboard_detector_handle    有效的棋盘格检测句柄
    @return	: AlgoError
    @Author : YangShangqi@2022/11/21
    ************************************/
    C_API AlgoError C_CALL ChessboardDetectorFindBorder(AlgoHandle chessboard_detector_handle);


	/************************************
	@brief  : 使用棋盘格来估计PSF
	@note   : 注释
	@param  : chessboard_detector_handle    有效的棋盘格检测句柄
	@return	: AlgoError
	@Author : tianbo@2023/06/13
	************************************/
	C_API AlgoError C_CALL ChessboardDetectorComputePSFFromChessboard(AlgoHandle chessboard_detector_handle);

	///************************************
	//@brief  : 获取棋盘格
	//@note   :
	//@param  : chessboard_detector_handle    有效的棋盘格检测句柄
	//@param  : out_chessboard			      导出的结构体,必须已经初始化完毕
	//@param  : in_is_deep_copy               是否深拷贝, 默认浅拷贝  1=深拷贝;0=浅拷贝
	//@return	: AlgoError
	//@Author : tianbo@2022/04/22
	//************************************/
	C_API AlgoError C_CALL ChessboardDetectorGet(AlgoHandle chessboard_detector_handle, AlgoChessBoard* out_chessboard, int in_is_deep_copy = 0);


	///************************************
	//@brief  : 导出棋盘格识别结果图像
	//@note   :
	//@param  : chessboard_detector_handle    有效的棋盘格检测句柄
	//@param  : in_img_fullname   文件名
	//@return	: AlgoError
	//@Author : tianbo@2022/10/10
	//************************************/
	C_API AlgoError C_CALL ChessboardDetectorExportImage(AlgoHandle chessboard_detector_handle, const char* in_img_fullname);


	///************************************
	//@brief  : 导出棋盘格估计PSF的结果图像
	//@note   :
	//@param  : chessboard_detector_handle    有效的棋盘格检测句柄
	//@param  : in_img_fullname   文件名
	//@return	: AlgoError
	//@Author : tianbo@2023/06/13
	//************************************/
	C_API AlgoError C_CALL ChessboardDetectorExportPSFImage(AlgoHandle chessboard_detector_handle, const char* in_img_fullname);


	///************************************
	//@brief  : 载入和保存棋盘格检测结果数据
	//@note   : 文件名后缀必须是.chessboard
	//@param  : chessboard_detector_handle    有效的棋盘格检测句柄
	//@param  : in_fullname   文件名后缀必须是.chessboard
	//@return	: AlgoError
	//@Author : tianbo@2022/04/22
	//************************************/
	C_API AlgoError C_CALL ChessboardDetectorLoad(AlgoHandle chessboard_detector_handle, const char* in_fullname);
	C_API AlgoError C_CALL ChessboardDetectorSave(AlgoHandle chessboard_detector_handle, const char* in_fullname);


	///************************************
	//@brief  : 获取buffer大小
	//@note   :
	//@param  : chessboard_detector_handle    有效的棋盘格检测句柄
	//@param  : out_len     buffer大小
	//@return	: AlgoError
	//@Author : tianbo@2021/07/06
	//************************************/
	C_API AlgoError C_CALL ChessboardDetectorGetBufferSize(AlgoHandle chessboard_detector_handle, uint64_t* out_len);

	///************************************
	//@brief  : 将chessboard里面的数据导出到buffer
	//@note   :
	//@param  : chessboard_detector_handle    有效的棋盘格检测句柄
	//@param  : out_buffer     已经申请好申请空间
	//@param  : in_out_len     外部申请好的空间大小,会用真实大小修改
	//@return	: AlgoError
	//@Author : tianbo@2021/07/01
	//************************************/
	C_API AlgoError C_CALL ChessboardDetectorToBuffer(AlgoHandle chessboard_detector_handle, char* out_buffer, uint64_t* in_out_len);


	///************************************
	//@brief  : 将buffer数据导入到chessboard
	//@note   :
	//@param  : chessboard_detector_handle    有效的棋盘格检测句柄
	//@param  : in_buffer
	//@param  : in_len
	//@return	: AlgoError
	//@Author : tianbo@2021/07/01
	//************************************/
	C_API AlgoError C_CALL ChessboardDetectorFromBuffer(AlgoHandle chessboard_detector_handle, const char* in_buffer, uint64_t in_len);




	///************************************
	//Release  句柄指向的对象的内存空间释放, 句柄指向的对象还在
	//Destroy  句柄指向的对象销毁, 此句柄已经无效, 请配合handle=nullptr;

	//handle=new OBJ;
	//OBJ.Set(a_lot_data);

	//Release 相当于:
	//	OBJ.Release();
	//Destroy 相当于:
	//	OBJ.release();
	//	delete handle;
	//************************************/
	C_API AlgoError C_CALL ChessboardDetectorRelease(AlgoHandle chessboard_detector_handle);
	C_API void C_CALL ChessboardDetectorDestroyHandle(AlgoHandle chessboard_detector_handle);



    /************************************
    @brief  : set 参数
    @note   : 依赖于达标信息的球形屏
    +--------------------------------
    @param  : chessboard_detector_handle    有效的棋盘格检测句柄
    @param  : trans                         有效的transformer句柄
    @param  : chess_info_in                 棋盘格信息
    @param  : 参数            参数说明
    +--------------------------------
    @return :
    @Author : YangShangqi@2023/08/24
    ************************************/
    C_API AlgoError C_CALL ChessboardDetectorSetParamForSphericalTab(AlgoHandle chessboard_detector_handle, AlgoHandle in_trans_handle, const ChessboardInfo* chess_info_in);


    /************************************
    @brief  : 根据球形屏分区处大表生成棋盘格图像及其规格
    @note   : 注释
    +--------------------------------
    @param  : Tab                       分区处的表
    @param  : camera_wh_in              相机分辨率
    @param  : gray_val_in               棋盘格的灰阶
    @param  : default_width_num_in      棋盘格密度控制参数 - 对于2k图像，生成棋盘格列数为 default_width_num_in,默认值32
    @param  : out_rgbw_chessboard       输出RGBW棋盘格图像
    @param  : chess_info_out            棋盘格信息（规格）
    +--------------------------------
    @return :
    @Author : YangShangqi@2023/07/12
    ************************************/
    C_API AlgoError C_CALL ChessboardGenerateCheckerboardTab(AlgoMat Tab, AlgoSize camera_wh_in, const int gray_val_in, const int default_width_num_in, AlgoMat* out_rgbw_chessboard_out, ChessboardInfo* chess_info_out);

    /************************************
    @brief  : 生成棋盘格图像及其规格
    @note   : 注释
    +--------------------------------
    @param  : in_led_wh                 LED分辨率
    @param  : camera_wh_in              相机分辨率
    @param  : gray_val_in               棋盘格的灰阶
    @param  : default_width_num_in      棋盘格密度控制参数 - 对于2k图像，生成棋盘格列数为 default_width_num_in,默认值32
    @param  : out_rgbw_chessboard       输出RGBW棋盘格图像数组 长度为 4
    @param  : chess_info_out            棋盘格信息（规格）
    +--------------------------------
    @return :
    @Author : YangShangqi@2023/07/12
    ************************************/
    C_API AlgoError C_CALL ChessboardGenerateCheckerboard(const AlgoSize* in_led_wh, const AlgoSize *camera_wh_in, const int gray_val_in, const int default_width_num_in, AlgoMat* out_rgbw_chessboard_out, ChessboardInfo* chess_info_out);


	/************************************
	@brief  : 生成隔点棋盘格图像及其规格
	@note   : 隔点超分链路生成棋盘格，用于源打屏
	+--------------------------------
	@param  : in_led_wh                 LED分辨率
	@param  : camera_wh_in              相机分辨率
	@param  : gray_val_in               棋盘格的灰阶
	@param  : default_width_num_in      棋盘格密度控制参数 - 对于2k图像，生成棋盘格列数为 default_width_num_in,默认值32
	@param  : interval_in				隔点间隔
	@param  : out_rgbw_chessboard       输出RGBW隔点棋盘格图像 纵向排列
	************************************
	以4*4隔点为例，隔点排列如下：
	1  5   9  13
	2  6  10  14
	3  7  11  15
	4  8  12  16
	************************************
	@param  : out_rgbw_chessboard_num   输出RGBW隔点棋盘格图像数量
	@param  : chess_info_out            棋盘格信息（规格）
	+--------------------------------
	@return :
	@Author : Wangyueyao@2024/03/19
	************************************/
	C_API AlgoError C_CALL ChessboardGenerateCheckerboardForInterval(const AlgoSize* in_led_wh, const AlgoSize *camera_wh_in, const int gray_val_in, const int default_width_num_in, const int interval_in, AlgoMat* out_rgbw_chessboard_out,int *out_rgbw_chessboard_num ,ChessboardInfo* chess_info_out);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif