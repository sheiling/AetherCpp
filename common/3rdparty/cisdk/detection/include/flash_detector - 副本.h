#ifndef __FLASH_DETECTOR_H__
#define __FLASH_DETECTOR_H__

#include "ci_define.h"
#include "ci_declare.h"
#include "roi_detector.h"
#include "flash_detector_error.h"
#include "flash_detector_data.h"
#include "algo_mat_data.h"
#include "meta_info_data.h"
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	/************************************
   @Brief		：	创建闪屏闪点检测句柄.
   @Note		：
   @Parameter	: 	flash_detector_handle -- 闪烁检测句柄
   @Returns		:   AlgoError
   @Author		：	mumeng@2022/8/24
   ************************************/
	C_API AlgoError C_CALL FlashDetectCreateHandle(AlgoHandle* flash_detector_handle);


	/************************************
   @Brief		：	设置闪屏闪点缺陷判断阈值
   @Note		：
   @Parameter	: 	flash_detector_handle  -- 闪烁检测句柄
   @Parameter	: 	detect_thresh          -- 阈值结构体，元素顺序：闪屏阈值、闪线阈值、闪点图像二值化阈值、判定闪点的像素个数阈值
   @Returns		:   AlgoError
   @Author		：	mumeng@2022/8/30
   ************************************/
	C_API AlgoError C_CALL FlashDetectSetThresh(AlgoHandle flash_detector_handle,
		DetectThresh* detect_thresh);


	/************************************
   @Brief		：	设置灯板的ROI4角点，生成mask列表，放在检测句柄中
   @Note		：
   @Parameter	: 	flash_detector_handle -- 闪烁检测句柄
   @Parameter	: 	in_img				  -- 相机拍得图像
   @Parameter	: 	in_screen4corner      -- 屏幕在图像区域中的4角点
   @Parameter	: 	in_panel_num		  -- 灯板数量
   @Parameter	: 	in_panel_rows		  -- 灯板的总行数
   @Parameter	: 	in_panel_cols		  -- 灯板的总列数
   @Parameter	: 	in_shrink_pixel_num	  -- 每个灯板mask的ROI区域往内缩小的像素个数
   @Returns		:   AlgoError
   @Author		：	mumeng@2025/12/24
   ************************************/
	C_API AlgoError C_CALL FlashDetectCalcPanelInfo(AlgoHandle flash_detector_handle, const AlgoMat* in_img, const AlgoPoint* in_screen4corner,
		PanelInfo* panel_parm, const int in_shrink_pixel_num);


   /************************************
   @Brief		：	生成灯板mask列表，放在检测句柄中
   @Note		：
   @Parameter	: 	flash_detector_handle -- 闪烁检测句柄
   @Parameter	: 	in_img				  -- 相机拍得图像
   @Returns		:   AlgoError
   @Author		：	mumeng@2026/3/3
   ************************************/
	C_API AlgoError C_CALL FlashDetectCalcPanelMask(AlgoHandle flash_detector_handle, const AlgoMat* in_img);

	/************************************
   @Brief		：	获取灯板区域定位结果，将区域画在灯板图上
   @Note		：
   @Parameter	: 	flash_detector_handle -- 闪烁检测句柄
   @Parameter	: 	in_img_file_path	  -- 保存结果图的路径
   @Returns		:   AlgoError
   @Author		：	mumeng@2026/3/3
   ************************************/
	C_API AlgoError C_CALL FlashDetectExportPanelMask(AlgoHandle flash_detector_handle, const char* in_img_file_path);

	/************************************
   @Brief		：	获取灯板信息
   @Note		：
   @Parameter	: 	flash_detector_handle -- 闪烁检测句柄
   @Parameter	: 	panel_parm            -- 获取到的灯板信息结构体
   @Returns		:   AlgoError
   @Author		：	mumeng@2025/12/24
   ************************************/
	C_API AlgoError C_CALL FlashDetectGetPanelInfo(AlgoHandle flash_detector_handle, PanelInfo* panel_parm);


	/************************************
   @Brief		：	将灯板的mask列表保存到本地文件中.
   @Note		：
   @Parameter	: 	flash_detector_handle   -- 闪烁检测句柄
   @Parameter	: 	in_panel_mask_file		-- 保存文件的路径，文件后缀为 .fdm
   @Returns		:   AlgoError
   @Author		：	mumeng@2022/9/5
   ************************************/
	C_API AlgoError C_CALL FlashDetectSavePanelMaskList(AlgoHandle flash_detector_handle,
		const char* in_panel_mask_file);



	/************************************
   @Brief		：	从本地文件读取灯板的mask列表.
   @Note		：
   @Parameter	: 	flash_detector_handle   -- 闪烁检测句柄
   @Parameter	: 	in_panel_mask_file	    -- 保存文件的路径，文件后缀为 .fdm
   @Returns		:   AlgoError
   @Author		：	mumeng@2022/9/5
   ************************************/
	C_API AlgoError C_CALL FlashDetectLoadPanelMaskList(AlgoHandle flash_detector_handle,
		const char* in_panel_mask_file);


	/************************************
   @Brief		：	获取灯板mask列表所占buffer的大小.
   @Note		：
   @Parameter	: 	flash_detector_handle   -- 闪烁检测句柄
   @Parameter	: 	out_len					-- 得到的buffer尺寸大小
   @Returns		:   AlgoError
   @Author		：	mumeng@2022/9/1
   ************************************/
	C_API AlgoError C_CALL FlashDetectGetPanelMaskListBufferSize(AlgoHandle flash_detector_handle, uint64_t* out_len);


	/************************************
   @Brief		：	将灯板mask列表放至buffer里.
   @Note		：
   @Parameter	: 	flash_detector_handle   -- 闪烁检测句柄
   @Parameter	: 	out_buffer				-- 输出mask的buffer空间
   @Parameter	: 	out_len					-- buffer尺寸大小
   @Returns		:   AlgoError
   @Author		：	mumeng@2022/9/1
   ************************************/
	C_API AlgoError C_CALL FlashDetectPanelMaskListToBuffer(AlgoHandle flash_detector_handle, char* out_buffer, uint64_t* out_len);



	/************************************
   @Brief		：	从buffer中获取灯板mask列表至检测句柄
   @Note		：
   @Parameter	: 	flash_detector_handle   -- 闪烁检测句柄
   @Parameter	: 	in_buffer				-- 输出mask的buffer空间
   @Parameter	: 	in_len					-- buffer尺寸大小
   @Returns		:   AlgoError
   @Author		：	mumeng@2022/9/1
   ************************************/
	C_API AlgoError C_CALL FlashDetectPanelMaskListFromBuffer(AlgoHandle flash_detector_handle, const char* in_buffer, uint64_t in_len);


	/************************************
   @Brief		：	载入当前图像.
   @Note		：
   @Parameter	: 	flash_detector_handle -- 闪烁检测句柄
   @Parameter	: 	in_img_current        -- 采集的当前帧图像
   @Returns		:   AlgoError
   @Author		：	mumeng@2022/9/5
   ********************************/
	C_API AlgoError C_CALL FlashDetectSetImage(AlgoHandle flash_detector_handle, const AlgoMat* in_img_current);



	/************************************
   @Brief		：	闪线检测.
   @Note		：
   @Parameter	: 	flash_detector_handle -- 闪烁检测句柄
   @Parameter	: 	in_panel_num		  -- 屏幕灯板数量
   @Parameter	: 	out_detect_ret_list	  -- 检测结果列表, 1 = 对应编号灯板有闪线； 0 = 对应编号灯板无闪线  注意：列表长度等于灯板数量  
   @Returns		:   AlgoError
   @Author		：	mumeng@2022/9/1
   ************************************/
	C_API AlgoError C_CALL FlashDetectLine(AlgoHandle flash_detector_handle, int* out_result_per_panel);



	/************************************
   @Brief		：	闪屏检测.
   @Note		：
   @Parameter	: 	flash_detector_handle -- 闪烁检测句柄
   @Parameter	: 	out_detect_ret	      -- 该屏幕的检测结果, 1 代表有闪屏，0 代表没闪屏
   @Returns		:   AlgoError
   @Author		：	mumeng@2022/9/5
   ************************************/
	C_API AlgoError C_CALL FlashDetectScreen(AlgoHandle flash_detector_handle,int* out_detect_ret);



	/************************************
   @Brief		：	闪点检测.
   @Note		：
   @Parameter	: 	flash_detector_handle -- 闪烁检测句柄
   @Parameter	: 	in_panel_num		  -- 屏幕灯板数量
   @Parameter	: 	out_detect_ret_list   -- 该屏幕的检测结果   1 = 对应编号灯板有闪点； 0 = 对应编号灯板无闪点  注意：列表长度等于灯板数量  
   @Returns		:   AlgoError
   @Author		：	mumeng@2022/9/5
   ************************************/
	C_API AlgoError C_CALL FlashDetectPoint(AlgoHandle flash_detector_handle, int* out_detect_ret_list);



	/************************************
   @Brief		：	闪点检测.
   @Note		：
   @Parameter	: 	flash_detector_handle -- 闪烁检测句柄
   @Parameter	: 	in_panel_num		  -- 屏幕灯板数量
   @Parameter	: 	in_panel_id			  -- 检测的第几个灯板，从 0 开始
   @Returns		:   AlgoError
   @Author		：	mumeng@2026/03/03
   ************************************/
	C_API AlgoError C_CALL FlashDetectPanel(AlgoHandle flash_detector_handle, int* out_detect_ret_list);



	/************************************
   @Brief		：	获取相机位置框
   @Note		：
   @Parameter	: 	flash_detector_handle -- 闪烁检测句柄
   @Parameter	: 	in_img				  -- 输入当前拍摄图像
   @Parameter	: 	out_draw_img_path	  -- 输出画有位置框的图像
   @Returns		:   AlgoError
   @Author		：	mumeng@2026/03/05
   ************************************/
	C_API AlgoError C_CALL FlashDetectExportPanelContoursImg(AlgoHandle flash_detector_handle, AlgoMat* in_img, const char* out_draw_img_path);



	/************************************
   @Brief		：	将缓存的图像释放.
   @Note		：
   @Parameter	: 	flash_detector_handle -- 闪烁检测句柄
   @Returns		:   AlgoError
   @Author		：	mumeng@2022/8/25
   ********************************/
	C_API AlgoError C_CALL FlashDetectResetImageBuffer(AlgoHandle flash_detector_handle);


	/************************************
   @Brief		：	将检测句柄中，缓存的中间图像释放.
   @Note		：
   @Parameter	: 	flash_detector_handle -- 闪烁检测句柄
   @Returns		:   AlgoError
   @Author		：	mumeng@2022/9/10
   ********************************/
	C_API AlgoError C_CALL FlashDetectRelease(AlgoHandle flash_detector_handle);



	/************************************
   @Brief		：	将flash_detector_handle句柄销毁.
   @Note		：
   @Parameter	: 	flash_detector_handle -- 闪烁检测句柄
   @Returns		:   void
   @Author		：	mumeng@2022/8/25
   ********************************/
	C_API void C_CALL FlashDetectDestroyHandle(AlgoHandle flash_detector_handle);


	/***********************************
	Release  句柄指向的对象的内存空间释放, 句柄指向的对象还在
	Destroy  句柄指向的对象销毁, 此句柄已经无效, 请配合handle = nullptr;
	**********************************/



#ifdef __cplusplus
}
#endif // __cplusplus
#endif
