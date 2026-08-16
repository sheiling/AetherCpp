#ifndef __DRIVER_DEFECT_DETECTOR_H__
#define __DRIVER_DEFECT_DETECTOR_H__

#include "ci_data.h"
#include "ci_declare.h"
#include "meta_info_data.h"
#include "algo_shot_data.h"
#include "point_mapper_data.h"
#include "algo_led_meter_data.h"
#include "led_contour_searcher_data.h"
#include "driver_defect_detector_data.h"
#include "driver_defect_detector_error.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

	/**************************************
	@brief   : 创建句柄
	@note    : 
	@return  : AlgoHandle
	@Author  : songshaojun@2024/12/04
	**************************************/
	C_API AlgoHandle C_CALL DriverDefectCreateHandle();

	
	/**************************************
	@brief   : 设置驱动IC信息
	@note    : 
	@param   : driver_defect_handle 输入量，有效的驱动IC检测句柄
	@param   : in_led_driver_info   输入量，驱动IC信息，详情请见driver_defect_detector_data.h
	@return  : AlgoError
	@Author  : songshaojun@2024/12/04
	**************************************/
	C_API AlgoError C_CALL DriverDefectSetDriverInfo(AlgoHandle driver_defect_handle, const LEDDriverInfo* in_led_driver_info);


	/**************************************
	@brief   : 设置检测信息
	@note    : 包含箱体roi、提取到的箱体灯点轮廓、提取到的点定位信息；
	@param   : driver_defect_handle  输入量，有效的驱动IC检测句柄
	@param   : in_roi_handle				 输入量，roi的handle
	@param   : in_contour_handle			 输入量，轮廓的handle
	@param   : in_map_handle				 输入量，点定位的handle
	@return  : AlgoError
	@Author  : songshaojun@2024/12/04
	**************************************/
	C_API AlgoError C_CALL DriverDefectSetDetectInfo(AlgoHandle driver_defect_handle, const AlgoHandle in_roi_handle,const AlgoHandle in_contour_handle, const AlgoHandle in_map_handle);
	


	/**************************************
	@brief   : 设置鬼影检测阈值
	@note    : 若不调用该接口，则使用算法内部默认参数
	@param   : driver_defect_handle  输入量，有效的驱动IC检测句柄
	@param   : in_th				 输入量，鬼影阈值
	@return  : AlgoError
	@Author  : songshaojun@2024/12/04
	**************************************/
	C_API AlgoError C_CALL DriverDefectSetGhostingTh(AlgoHandle driver_defect_handle, const DriverDefectGhostingTh* in_th);


	/**************************************
	@brief   : 设置第一扫偏暗检测阈值
	@note    : 若不调用该接口，则使用算法内部默认参数
	@param   : driver_defect_handle  输入量，有效的驱动IC检测句柄
	@param   : in_th				 输入量，第一扫偏暗阈值
	@return  : AlgoError
	@Author  : songshaojun@2024/12/04
	**************************************/
	C_API AlgoError C_CALL DriverDefectSetFirstScanTh(AlgoHandle driver_defect_handle, const DriverDefectFirstScanTh* in_th);


	/**************************************
	@brief   : 设置耦合检测阈值
	@note    : 若不调用该接口，则使用算法内部默认参数
	@param   : driver_defect_handle  输入量，有效的驱动IC检测句柄
	@param   : in_th				 输入量，耦合阈值
	@return  : AlgoError
	@Author  : songshaojun@2024/12/04
	**************************************/
	C_API AlgoError C_CALL DriverDefectSetCouplingTh(AlgoHandle driver_defect_handle, const DriverDefectCouplingTh* in_th);


	/**************************************
	@brief   : 设置屏体的白色目标XYZ值
	@note    : 该接口服务于自动电流增益调节功能
	@param   : driver_defect_handle  输入量，有效的驱动IC检测句柄
	@param   : in_target_XYZ		 输入量，屏体要调整到的白色画面下的XYZ目标值
	@return  : AlgoError
	@Author  : songshaojun@2025/12/17
	**************************************/
	C_API AlgoError C_CALL DriverDefectSetTargetXYZ(AlgoHandle driver_defect_handle, const TriXYZ &in_target_XYZ);


	/**************************************
	@brief   : 设置当前屏体，进行自动电流调节时所用的基准电流增益值、及RGB电流增益所能调节的最大值和最小值，具体数值，不是百分比
	@note    : 该接口服务于自动电流增益调节功能
	@param   : driver_defect_handle  输入量，有效的驱动IC检测句柄
	@param   : in_base_rgb		     输入量，当前进行屏体RGB电流增益调节时选定的基准电流增益值
	@param   : in_min_rgb		     输入量，当前屏体RGB电流增益所能调节的最低下限值
	@param   : in_max_rgb		     输入量，当前屏体RGB电流增益所能调节的最高上限值
	@return  : AlgoError
	@Author  : songshaojun@2025/12/17
	**************************************/
	C_API AlgoError C_CALL DriverDefectSetGainLimits(AlgoHandle driver_defect_handle, const TriRGB &in_base_rgb, const TriRGB &in_min_rgb,const TriRGB &in_max_rgb);


	/**************************************
	@brief   : 获取达到目标值，所需要调节的RGB电流增益值
	@note    : 
	@param   : driver_defect_handle  输入量，有效的驱动IC检测句柄
	@param   : in_rgb_XYZ		     输入量，当前RGB电流增益下，R/G/B纯色画面下，使用色度计测量得到的XYZ值
	@param   : in_measurment_WXYZ    输入量，当前RGB电流增益下，W纯色画面下，使用色度计测量得到的XYZ值。
	@param   : in_current_r_gain	 输入量，当前情况下，已经写入屏体内部的R电流增益（若为首次开始调节，此处置为0，详细用法请参见demo）
	@param   : in_current_g_gain	 输入量，当前情况下，已经写入屏体内部的G电流增益（若为首次开始调节，此处置为0，详细用法请参见demo）
	@param   : in_current_b_gain	 输入量，当前情况下，已经写入屏体内部的B电流增益（若为首次开始调节，此处置为0，详细用法请参见demo）
	@param   : out_r_gain		     输出量，达到目标值，所需要调整的R通道电流增益实际值
	@param   : out_g_gain		     输出量，达到目标值，所需要调整的G通道电流增益实际值
	@param   : out_b_gain		     输出量，达到目标值，所需要调整的B通道电流增益实际值
	@return  : AlgoError
	@Author  : songshaojun@2025/12/17
	**************************************/
	C_API AlgoError C_CALL DriverDefectGetRGBCurrentGain(AlgoHandle driver_defect_handle, const RGBXYZ &in_rgb_XYZ, const TriXYZ &in_measurment_WXYZ,
		const float &in_current_r_gain_rate, const float &in_current_g_gain_rate, const float &in_current_b_gain_rate,
		float *out_r_gain_rate, float *out_g_gain_rate, float *out_b_gain_rate,
		float *out_r_gain, float *out_g_gain, float *out_b_gain);



	/**************************************
	@brief   : 黑屏检测
	@note    : 检测箱体是否为黑屏（熄灭）状态，如果是，返回值为true，否则为false
	@param   : driver_defect_handle  输入量，有效的驱动IC检测句柄
	@param   : in_shot				 输入量，相机拍摄的箱体shot
	@param   : out_is_dark			 输出量，是否为黑屏；
	@return  : AlgoError
	@Author  : songshaojun@2024/12/04
	**************************************/
	C_API AlgoError C_CALL DriverDefectCheckScrenStatus(AlgoHandle driver_defect_handle, const AlgoShot* in_shot, bool* out_is_dark);



	/**************************************
	@brief   : 设置鬼影检测需要的背景图像
	@note    : 所需图像为背景图像的光通量
	@param   : driver_defect_handle  输入量，有效的驱动IC检测句柄
	@param   : in_meter_bg_handle	 输入量，检测鬼影画面时，用到的背景图的meter
	@return  : AlgoError
	@Author  : songshaojun@2025/01/16
	**************************************/
	C_API AlgoError C_CALL DriverDefectSetGhostingBg(AlgoHandle driver_defect_handle, const AlgoHandle in_meter_bg_handle);



	/**************************************
	@brief   : 鬼影检测
	@note    : 检测箱体是否存在鬼影现象，及鬼影强弱
	@param   : driver_defect_handle  输入量，有效的驱动IC检测句柄
	@param   : in_flag				 输入量，检测行管鬼影还是驱动IC鬼影，0代表检测行管鬼影，1代表检测驱动IC鬼影
	@param   : in_shot_handle		 输入量，箱体shot的handle
	@param   : in_meter_handle		 输入量，由in_shot提取得到的meter的handle
	@param   : in_pattern			 输入量，打屏pattern
	@param   : out_result			 输出量，检测结果，GhostingResult类型详见driver_defect_detector_data.h
	@return  : AlgoError
	@Author  : songshaojun@2024/12/04
	**************************************/
	C_API AlgoError C_CALL DriverDefectDetectGhosting(AlgoHandle driver_defect_handle,const int in_flag,
		const AlgoHandle in_shot_handle, const AlgoHandle in_meter_handle, const AlgoMat* in_pattern, GhostingResult* out_result);


	/**************************************
	@brief   : 耦合检测
	@note    : 检测箱体是否存在高对比度耦合现象（暗块将周围带亮/暗，或亮块将周围带亮/暗），及耦合强弱
	@param   : driver_defect_handle  输入量，有效的驱动IC检测句柄
	@param   : in_flag				 输入量，检测亮块耦合还是暗块耦合，0代表检测亮块耦合，1代表检测暗块耦合
	@param   : in_shot_handle		 输入量，箱体shot的handle
	@param   : in_meter_handle		 输入量，由箱体shot提取得到的meter
	@param   : in_pattern			 输入量，打屏pattern
	@param   : out_result			 输出量，检测结果，CouplingResult类型详见driver_defect_detector_data.h
	@return  : AlgoError
	@Author  : songshaojun@2024/12/04
	**************************************/
	C_API AlgoError C_CALL DriverDefectDetectCoupling(AlgoHandle driver_defect_handle,const int in_flag,
		const AlgoHandle in_shot_handle, const AlgoHandle in_meter_handle, const AlgoMat* in_pattern, CouplingResult* out_result);

	/**************************************
	@brief   : 第一扫偏暗检测
	@note    : 检测箱体是否存在第一扫偏暗现象（第一扫偏暗/亮），及第一扫偏暗强弱
	@param   : driver_defect_handle  输入量，有效的驱动IC检测句柄
	@param   : in_shot_handle		 输入量，箱体shot的handle
	@param   : in_meter_handle		 输入量，由箱体shot提取得到的meter
	@param   : in_scan_idx_list		 输入量，第一扫的索引列表
	@param   : num					 输入量，第一扫的索引的数量
	@param   : out_result			 输出量，检测结果，FirstScanResult类型详见driver_defect_detector_data.h
	@return  : AlgoError
	@Author  : songshaojun@2024/12/04
	**************************************/
	C_API AlgoError C_CALL DriverDefectDetectFirstScan(AlgoHandle driver_defect_handle,
		const AlgoHandle in_shot_handle, const AlgoHandle in_meter_handle, const int* in_scan_idx_list, const int num , FirstScanResult* out_result);


	/**************************************
	@brief   : 设置/获取 第一扫偏暗检测结果
	@note    : 
	@param   : driver_defect_handle  输入量，有效的驱动IC检测句柄
	@param   : in_result			 输入量，设置进来的第一扫偏暗检测结果
	@param   : out_result		     输出量，吐出去的第一扫偏暗结果
	@return  : AlgoError
	@Author  : songshaojun@2024/12/04
	**************************************/
	C_API AlgoError C_CALL DriverDefectSetFirstScanResult(AlgoHandle driver_defect_handle, const FirstScanResult* in_result);
	C_API AlgoError C_CALL DriverDefectGetFirstScanResult(AlgoHandle driver_defect_handle, FirstScanResult* out_result);


	/**************************************
	@brief   : 设置/获取 耦合检测结果
	@note    : 
	@param   : driver_defect_handle  输入量，有效的驱动IC检测句柄
	@param   : in_result			 输入量，设置进来的耦合检测结果
	@param   : out_result		     输出量，吐出去的耦合结果
	@return  : AlgoError
	@Author  : songshaojun@2024/12/04
	**************************************/
	C_API AlgoError C_CALL DriverDefectSetCouplingResult(AlgoHandle driver_defect_handle, const CouplingResult* in_result);
	C_API AlgoError C_CALL DriverDefectGetCouplingResult(AlgoHandle driver_defect_handle, CouplingResult* out_result);


	/**************************************
	@brief   : 设置/获取 鬼影测结果
	@note    : 
	@param   : driver_defect_handle  输入量，有效的驱动IC检测句柄
	@param   : in_result			 输入量，设置进来的鬼影检测结果
	@param   : out_result		     输出量，吐出去的鬼影结果
	@return  : AlgoError
	@Author  : songshaojun@2024/12/04
	**************************************/
	C_API AlgoError C_CALL DriverDefectSetGhostingResult(AlgoHandle driver_defect_handle, const GhostingResult* in_result);
	C_API AlgoError C_CALL DriverDefectGetGhostingResult(AlgoHandle driver_defect_handle, GhostingResult* out_result);


	/************************************
    @brief    : 导出可视化检测结果图,仅供测试!!!
    @note     : 根据flag选择导出不同的结果
	@param    : driver_defect_handle     输入量，有效的驱动IC检测句柄
	@param    : in_flag  			     输入量，标志位，flag = 0代表返回鬼影可视化检测结果，flag=1代表返回耦合可视化检测结果，flag=2代表返回第一扫偏暗可视化结果；
	@param    : in_fullname		         输入量，结果图在硬盘上保存的位置
	@return   : AlgoError
	@Author   : songshaojun@2024/12/04
    ************************************/
    C_API AlgoError C_CALL DriverDefectExportResultPng(AlgoHandle driver_defect_handle, const int in_flag, const char* in_fullname);


	/************************************
    @Brief	：载入和保存检测结果
    @Note	：文件名后缀必须是.icdefect
    @param	: driver_defect_handle, 输入量, 有效的驱动IC检测句柄
    @param	: in_fullname,          输入量, 文件名后缀必须是.icdefect
    @return : AlgoError
	@Author : songshaojun@2024/12/04
    ************************************/
    C_API AlgoError C_CALL DriverDefectLoad(AlgoHandle driver_defect_handle, const char* in_fullname);
    C_API AlgoError C_CALL DriverDefectSave(AlgoHandle driver_defect_handle, const char* in_fullname);


	/************************************
	@brief  : 获取buffer大小
	@note   :
	@param	: driver_defect_handle, 输入量, 有效的驱动IC检测句柄
	@param  : out_len, 输出量, buffer大小
	@return	: AlgoError
	@Author : songshaojun@2024/12/04
	************************************/
	C_API AlgoError C_CALL DriverDefectGetBufferSize(AlgoHandle driver_defect_handle, uint64_t* out_len);


	/************************************
	@brief  : 将ledpoint里面的数据导出到buffer
	@note   :
	@param	: driver_defect_handle, 输入量, 有效的驱动IC检测句柄
	@param  : out_buffer        已经申请好申请空间
	@param  : in_out_len        外部申请好的空间大小,会用真实大小修改
	@return	: AlgoError
	@Author : songshaojun@2024/12/04
	************************************/
	C_API AlgoError C_CALL DriverDefectToBuffer(AlgoHandle driver_defect_handle, char* out_buffer, uint64_t* in_out_len);


	/************************************
	@brief  : 将buffer数据导入到driver defect detector
	@note   :
	@param	: driver_defect_handle, 输入量, 有效的驱动IC检测句柄
	@param  : in_buffer
	@param  : in_len
	@return	: AlgoError
	@Author : songshaojun@2024/12/04
	************************************/
	C_API AlgoError C_CALL DriverDefectFromBuffer(AlgoHandle driver_defect_handle, const char* in_buffer, uint64_t in_len);



	/**************************************
	@brief   : 空间内存释放
	@note    : 句柄指向的对象销毁，此句柄已经无效，请配合handle = nullptr
	@param   : 
	@return  : void
	@Author  : songshaojun@2023/01/07
	**************************************/
	C_API AlgoError C_CALL DriverDefectRelease(AlgoHandle driver_defect_handle);
	C_API void C_CALL DriverDefectDestroyHandle(AlgoHandle driver_defect_handle);


	

#ifdef __cplusplus
}
#endif
#endif 
