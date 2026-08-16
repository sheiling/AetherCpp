#ifndef __BLUR_LED_LUM_EXTRACTOR_H__
#define __BLUR_LED_LUM_EXTRACTOR_H__

#include "ci_define.h"
#include "ci_declare.h"
#include "ci_data.h"
#include "blur_led_lum_extractor_errors.h"
#include "led_lum_extractor_data.h"
#include "chessboard_detector_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	/************************************
	@brief  : 生成棋盘格及相关参数
	@note   :
	@param  : led_wh_in					led的分辨率，wh
	@param  : camera_wh_in				相机分辨率，wh
	@param  : gray_val_in				生成棋盘格的灰度值，0-255，8bit
	@param  : checkerboard_size_out		输出，棋盘格的亮暗块数量，wh
	@param  : red_checkerboard_out		输出，红色棋盘格图像，bgr
	@param  : green_checkerboard_out	输出，绿色棋盘格图像，bgr
	@param  : blue_checkerboard_out		输出，蓝色棋盘格图像，bgr
	@param  : single_area_size_out		输出，棋盘格每个区域的分辨率
	@param  : end_col_width_out			输出，棋盘格最后一列的宽度
	@param  : end_row_height_out		输出，棋盘格最后一行的高度
	@return	: AlgoError
	@Author : tianbo@2021/10/18
	************************************/
	C_API AlgoError C_CALL BlurLedLumGenerateChessboard(const AlgoSize* in_led_wh, const AlgoSize* in_camera_wh, const int in_gray_val,
		AlgoSize* out_checkerboard_size, AlgoMat* out_rgbw_chessboard, int* out_single_area_size, int* out_end_col_width, int* out_end_row_height);


	C_API AlgoError C_CALL BlurLedLumCreateHandle(AlgoHandle* out_blur_lum_handle);

	///************************************
	//@brief  : 光通量提取
	//@note   :
	//@param  : blur_lum_handle   有效的基于模糊成像的光通量句柄
	//@param  : in_pure_shot		有效的shot句柄 详见algo_shot.h中定义
	//@param  : CornerPoint		图像四个角点坐标，顺序为左上、右上、右下、左下
	//@param  : in_num			表示CornerPoint中点的数量
	//@param  ：topLedNum			可选参数，针对球型屏上下边灯点不一特定的参数，表示上边的实际灯点数
	//@param  ：bottomLedNum		可选参数，针对球型屏上下边灯点不一特定的参数，表示下边的实际灯点数
	//@param  ：rowsBatch			可选参数，默认为32.及表示在灯点行数变化的过程中，变化的最小间隔是32
	//@return	: AlgoError
	//@Author : tianbo@2021/07/08
	//************************************/
	C_API AlgoError C_CALL BlurLedLumExtract(AlgoHandle blur_lum_handle, const AlgoHandle in_pure_shot, const AlgoPoint*  CornerPoint, int in_num,
		int topLedNum = -1, int bottomLedNum = -1, int rowsBatch = 96);


	///************************************
	//@brief  : 光通量提取
	//@note   :
	//@param  : blur_lum_handle		有效的基于模糊成像的光通量句柄
	//@param  : in_nsr				输入的nsr,默认0.03.
	//@param  : in_outline_ratio	deblur后异常点判断阈值，默认8.
	//@return	: AlgoError
	//@Author : tianbo@2023/07/26
	//************************************/
	C_API AlgoError C_CALL BlurLedLumSetParamDeblur(AlgoHandle blur_lum_handle, float in_nsr, int in_outline_ratio);




    /************************************
    @brief  : 光通量提取 ( 含Transformer接口)
    @note   : 含Transformer接口
    @param  : blur_lum_handle           有效的基于模糊成像的光通量句柄
    @param  : chessboard_handle         有效的棋盘格检测句柄
    @param  : transformer_handle        有效的transformer句柄
    @param  : end_col_width             棋盘格最后一列的宽
    @param  : end_row_height            棋盘格最后一行的高
    @return : AlgoError
    @Author : YangShangqi@2022/11/23
    ************************************/
    C_API AlgoError C_CALL BlurLedLumExtractTrans(AlgoHandle blur_lum_handle,
        const AlgoHandle in_chessboard_handle,
        const AlgoHandle in_transformer_handle,
        int in_end_col_width, int in_end_row_height);


    /************************************
    @brief  : 光通量提取
    @note   : 用于SMD快速亮度校正技术（在提取光通量后会使用高斯滤波对光通量进行平滑）
              针对于由于数码相机欠采样产生的十字亮暗线
    @param  : blur_lum_handle           有效的基于模糊成像的光通量句柄
    @param  : chessboard_handle         有效的棋盘格检测句柄
    @param  : transformer_handle        有效的transformer句柄
    @param  : end_col_width             棋盘格最后一列的宽
    @param  : end_row_height            棋盘格最后一行的高
    @return : AlgoError
    @Author : YangShangqi@2022/11/23
    ************************************/
    C_API AlgoError C_CALL BlurLedLumExtractBlurPro(AlgoHandle blur_lum_handle,
        const AlgoHandle in_chessboard_handle,
        const AlgoHandle in_transformer_handle,
        int in_end_col_width, int in_end_row_height);



	/************************************
	@brief  : （球型屏三角区）光通量提取
	@note   :  
	@param  : blur_lum_handle           有效的基于模糊成像的光通量句柄
	@return : AlgoError
	@Author : tianbo@2023/02/20
	************************************/
	C_API AlgoError C_CALL BlurLedLumExtracTriangle(AlgoHandle blur_lum_handle);


	/************************************
	@brief  : （球型屏三角区）光通量提取
	@note   :
	@param  : blur_lum_handle           有效的基于模糊成像的光通量句柄
	@param  : corner_points_in          三角区的角点，有顺序。{P_0， p_1， p_2, p_3...},其中p0表示成像的顶点。p1-pn表示从输入源的左侧到右侧显示的角点。
	@param  : num_points_in             角点的个数
	@param  : each_row_points_in        每一行最基本单元的实际灯点比例。eg，某区域是由100x100分辨率的板子拼接的，其第一行实际灯点个数为40，则改行对应的值为40/100=0.4。
	@param  : num_rows_in				采集区域有多少行，与each_row_points_in数据的长度一致。
	@param  : is_top					当前三角区是否在顶端，在顶端为true，反之为false。
	@return : AlgoError
	@Author : tianbo@2023/02/20
	************************************/
	C_API AlgoError C_CALL BlurLedLumSetParamTriangle(AlgoHandle blur_lum_handle, AlgoPoint* corner_points_in, int num_points_in, float* each_row_points_in, int num_rows_in, bool is_top);

	///************************************
	//@brief  : 光通量提取(将transfromer后的Shot.image resize后作为光通量lum)
	//@note   :
	//@param  : blur_lum_handle   有效的基于模糊成像的光通量句柄
	//@param  : in_pure_shot	  有效的shot句柄 详见algo_shot.h中定义
	//@param  : in_led_size		  屏体实际分辨率
	//@Author : qianxiaotong@2023/8/24
	//************************************/
	C_API AlgoError C_CALL BlurLedLumExtractFromImage(AlgoHandle blur_lum_handle, const AlgoHandle in_pure_shot, const AlgoSize in_led_size);

	///************************************
	//@brief  : SetImage (目前仅提取球形屏光通量之前需要单独SetImage)
	//@note   :
	//@param  : blur_lum_handle   有效的基于模糊成像的光通量句柄
	//@param  : in_pure_shot	  有效的shot句柄 详见algo_shot.h中定义
	//@return	: AlgoError
	//@Author : tianbo@2021/07/08
	//************************************/
	C_API AlgoError C_CALL BlurLedLumSetImage(AlgoHandle blur_lum_handle, const AlgoHandle in_pure_shot);


	///************************************
	//@brief  : Set球形屏参数 (目前仅提取球形屏光通量之前需要单独Set参数)
	//@note   :
	//@param  : blur_lum_handle		有效的基于模糊成像的光通量句柄
	//@param  : in_top_led_num		采集区域最上方实际灯点数占比（相对于显示分辨率的占比），eg，显示分辨率为50，实际最上方仅有10个灯点，则该数值为0.2
	//@param  : in_bottom_led_num	采集区域最下方实际灯点数占比（相对于显示分辨率的占比）
	//@param  : in_rows_batch		多少行灯板抽点数不变
	//@return	: AlgoError
	//@Author : tianbo@2021/07/08
	//************************************/
	C_API AlgoError C_CALL BlurLedLumSetParamSpherical(AlgoHandle blur_lum_handle, float in_top_led_num = 1.f,
		float in_bottom_led_num = 1.f, int in_rows_batch = 96);


	///************************************
	//@brief  : 球形屏光通量提取
	//@note   :
	//@param  : blur_lum_handle   有效的基于模糊成像的光通量句柄
	//@param  : chessboard_handle	有效的chessboard句柄 
	//@param  : in_num			表示CornerPoint中点的数量
	//@param  ：topLedNum			可选参数，针对球型屏上下边灯点不一特定的参数，表示上边的实际灯点数
	//@param  ：bottomLedNum		可选参数，针对球型屏上下边灯点不一特定的参数，表示下边的实际灯点数
	//@param  ：rowsBatch			可选参数，默认为32.及表示在灯点行数变化的过程中，变化的最小间隔是32
	//@return	: AlgoError
	//@Author : tianbo@2021/07/08
	//C_API AlgoError C_CALL BlurLedLumExtractSpherical(AlgoHandle blur_lum_handle, const AlgoHandle chessboard_handle, 
	//	int in_end_col_width, int in_end_row_height);
    C_API AlgoError C_CALL BlurLedLumExtractSpherical(AlgoHandle blur_lum_handle, const AlgoHandle chessboard_handle,
        int in_end_col_width, int in_end_row_height, AlgoHandle trans_handle = nullptr);



    /************************************
    @brief  : 设置边缘修正宽度
    @note   :   + 在基于大表的球形屏光通量提取中，设置此参数后开启边缘修正功能
                + 不设置此参数不开启边缘修正
                + 参数<=0时不开启边缘修正
    +--------------------------------
    @param  : blur_lum_handle            有效的基于模糊成像的光通量句柄
    @param  : num_in                     边缘修正宽度
    @param  : 参数            参数说明
    +--------------------------------
    @return :
    @Author : YangShangqi@2023/08/24
    ************************************/
    C_API AlgoError C_CALL BlurLedLumSetParamEdgeCorrNum(AlgoHandle blur_lum_handle, const int num_in);


    /************************************
    @brief  : 球形屏光通量提取(有大表的流程)
    @note   : 注释
    +--------------------------------
    @param  : blur_lum_handle               有效的基于模糊成像的光通量句柄
    @param  : chessboard_handle             有效的chessboard句柄
    @param  : trans_handle                  有效的transform句柄
    @param  : single_area_size_out          棋盘格block尺寸（非最后行类棋盘格一块的size）
    @param  : end_col_width_out             棋盘格最后列的宽
    @param  : end_row_height_out            棋盘格最后行的高
    @param  : led_rect_size                 真实LED梯形的最小外接矩形size
    @param  : blocks_rc                     棋盘格每行的分辨率
    +--------------------------------
    @return :
    @Author : YangShangqi@2023/07/12
    ************************************/
    C_API AlgoError C_CALL BlurLedLumExtractSphericalTab(AlgoHandle blur_lum_handle, const AlgoHandle chessboard_handle, const AlgoHandle trans_handle, const ChessboardInfo chess_info_in, const AlgoMat Tab_in);

	///************************************
	//@brief  : 光通量提取
	//@note   :
	//@param  : blur_lum_handle   有效的基于模糊成像的光通量句柄
	//@param  ：region_handle		区域定位句柄
	//@param  : in_pure_shot		有效的shot句柄 详见algo_shot.h中定义
	//@param  : in_chessboard_shot		有效的采集到棋盘格的句柄
	//@param  : in_chessboard_size		表示棋盘格的亮暗块个数，wh
	//@param  : in_end_col_width		棋盘格最后一列的宽度
	//@param  : in_end_row_height		棋盘格最后一行的高度
	//@Author : tianbo@2021/10/12
	//************************************/
	C_API AlgoError C_CALL BlurLedLumExtractPro(AlgoHandle blur_lum_handle, AlgoHandle region_handle, const AlgoHandle in_pure_shot,
		const AlgoHandle in_chessboard_shot, AlgoSize in_chessboard_size, int in_end_col_width, int in_end_row_height);

	
	/************************************
	@brief  : 获取光通量
	@note   :
	@param  : blur_lum_handle    有效的基于模糊成像的光通量句柄
	@param  : out_lum			导出的结构体,必须已经初始化完毕
	@param  : in_is_deep_copy   是否深拷贝, 默认浅拷贝  1=深拷贝;0=浅拷贝
	@return	: AlgoError
	@Author : tianbo@2021/07/08
	************************************/
	C_API AlgoError C_CALL BlurLedLumGet(AlgoHandle blur_lum_handle, AlgoLedLum* out_lum, int in_is_deep_copy = 0);



	/************************************
	@brief  : 载入和保存灯点光通量数据
	@note   : 文件名后缀必须是.lum
	@param  : blur_lum_handle    有效的基于模糊成像的光通量句柄
	@param  : in_fullname   文件名后缀必须是.lum
	@return	: AlgoError
	@Author : tianbo@2021/06/21
	************************************/
	C_API AlgoError C_CALL BlurLedLumLoad(AlgoHandle blur_lum_handle, const char* in_fullname);
	C_API AlgoError C_CALL BlurLedLumSave(AlgoHandle blur_lum_handle, const char* in_fullname);


	/************************************
	@brief  : 导出灯点光通量结果图
	@note   :
	@param  : blur_lum_handle   有效的基于模糊成像的光通量句柄
	@param  : in_img_fullname   文件名
	@param  : in_is_color       是否为彩色  1=彩色,0=灰度
	@return	: AlgoError
	@Author : tianbo@2021/06/21
	************************************/
	C_API AlgoError C_CALL BlurLedLumExportImage(AlgoHandle blur_lum_handle, const char* in_img_fullname, int in_is_color = 0);


	/************************************
	@brief  : 获取buffer大小
	@note   :
	@param  : blur_lum_handle  当前有效句柄
	@param  : out_len     buffer大小
	@return	: AlgoError
	@Author : tianbo@2021/07/06
	************************************/
	C_API AlgoError C_CALL BlurLedLumGetBufferSize(AlgoHandle blur_lum_handle, uint64_t* out_len);

	/************************************
	@brief  : 将lum里面的数据导出到buffer
	@note   :
	@param  : blur_lum_handle     有效的lum句柄
	@param  : out_buffer     已经申请好申请空间
	@param  : in_out_len     外部申请好的空间大小,会用真实大小修改
	@return	: AlgoError
	@Author : tianbo@2021/07/01
	************************************/
	C_API AlgoError C_CALL BlurLedLumToBuffer(AlgoHandle blur_lum_handle, char* out_buffer, uint64_t* in_out_len);


	/************************************
	@brief  : 将buffer数据导入到lum
	@note   :
	@param  : blur_lum_handle   有效的lum句柄
	@param  : in_buffer
	@param  : in_len
	@return	: AlgoError
	@Author : tianbo@2021/07/01
	************************************/
	C_API AlgoError C_CALL BlurLedLumFromBuffer(AlgoHandle blur_lum_handle, const char* in_buffer, uint64_t in_len);




	/************************************
	Release  句柄指向的对象的内存空间释放, 句柄指向的对象还在
	Destroy  句柄指向的对象销毁, 此句柄已经无效, 请配合handle=nullptr;

	handle=new OBJ;
	OBJ.Set(a_lot_data);

	Release 相当于:
		OBJ.Release();
	Destroy 相当于:
		OBJ.release();
		delete handle;
	************************************/
	C_API AlgoError C_CALL BlurLedLumRelease(AlgoHandle blur_lum_handle);
	C_API void C_CALL BlurLedLumDestroyHandle(AlgoHandle blur_lum_handle);

	

#ifdef __cplusplus
}
#endif // __cplusplus
#endif