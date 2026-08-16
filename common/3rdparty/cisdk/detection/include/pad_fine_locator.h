#ifndef _PAD_FINE_LOCATOR_H_
#define _PAD_FINE_LOCATOR_H_

#include "ci_declare.h"
#include "ci_data.h"
#include "pad_fine_locator_data.h"
#include "pad_fine_locator_error.h"
#include "algo_mat.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

	/*********************************************
	@brief   : 创建PadFineLocateLocator句柄
	@note    :
	@param   : locator_handle
	@return  : AlgoError
	@Author  : shimai@2023/12/21
	*********************************************/
	C_API AlgoError C_CALL PadFineLocateCreateHandle(AlgoHandle* locator_handle);

	/******************************************
	@brief   : pad中心在ROI中的坐标定位
	@note    :
	@param   : locator_handle       有效的定位句柄
	@param   : in_image             需要检测的图像
	@param   : out_pad_coordinate   pad点中心在选定的roi中的坐标
	@return  : AlgoError
	@Author  : shimai@2023/12/21
	*******************************************/
	C_API AlgoError C_CALL PadFineLocatePadCenterLocate(AlgoHandle locator_handle, const AlgoMat* in_image, AlgoPoint* out_pad_coordinate);
	
	/*******************************************
	@brief   : 计算探针与Pad点中心的实际距离
	@note    :
	@param   : locator_handle       有效的定位句柄
	@param   : in_coordinate        输入的坐标
	@param   : out_x_distance       x轴距离
	@param   : out_y_distance       Y轴距离
	@return  : AlgoError
	@Author  : shimai@2023/12/21
	********************************************/
	C_API AlgoError C_CALL PadFineLocateCalculateDistance(AlgoHandle locator_handle, AlgoPoint* in_coordinate, float* out_x_distance, float* out_y_distance);


	/*******************************************
	@brief   : 设定roi左上角坐标与宽高信息
	@note    :
	@param   : locator_handle       有效的定位句柄
	@param   : in_roi_info          输入的roi相关信息，输入为一个Rect范围
	@return  : AlgoError
	@Author  : shimai@2023/12/21
	********************************************/
	C_API AlgoError C_CALL PadFineLocateSetRoiInfo(AlgoHandle locator_handle, const AlgoRect* in_roi_info);

	/*******************************************
	@brief   : 设定转换矩阵
	@note    :
	@param   : locator_handle       有效的定位句柄
	@param   : in_transform_matrix  设定的转换矩阵
	@return  : AlgoError
	@Author  : shimai@2023/12/21
	********************************************/
	C_API AlgoError C_CALL PadFineLocateSetTransformMatrix(AlgoHandle locator_handle, const AlgoMat* in_transform_matrix);
	
	/*******************************************
	@brief   : 设定定位阈值
	@note    : 
	@param   : locator_handle       有效的定位句柄
	@param   : in_thresh            设定的阈值
	@return  : AlgoError
	@Author  : shimai@2023/12/21
	********************************************/
	C_API AlgoError C_CALL PadFineLocateSetThresh(AlgoHandle locator_handle, float* in_thresh);
	
	/*******************************************
	@brief   : 设定匹配模板
	@note    : 配合model模块使用，具体操作见demo
	@param   : locator_handle       有效的定位句柄
	@param   : model_handle         model句柄
	@param   : in_template_name     模版名
	@return  : AlgoError
	@Author  : shimai@2023/12/21
	********************************************/
	C_API AlgoError C_CALL PadFineLocateSetTemplate(AlgoHandle locator_handle, AlgoHandle model_handle, const char* in_template_name);
	
	/*******************************************
	@brief   : 获取ROI信息，包括探针在图像中的坐标以及ROI宽高
	@note    : 
	@param   : locator_handle       有效的定位句柄
	@param   : out_roi_info         输出的roi信息
	@return  : AlgoError
	@Author  : shimai@2023/12/21
	********************************************/
	C_API AlgoError C_CALL PadFineLocateGetRoiInfo(AlgoHandle locator_handle, AlgoRect* out_roi_info);

	/*******************************************
	@brief   : 获取转换矩阵 
	@note    :
	@param   : locator_handle       有效的定位句柄
	@param   : out_trans_mat        获取的转换矩阵信息
	@return  : AlgoError
	@Author  : shimai@2023/12/21
	********************************************/
	// C_API AlgoError C_CALL PadFineLocateGetTransformMatrix(AlgoHandle locator_handle, AlgoMat* out_trans_mat);

	/*******************************************
	@brief   : 获取匹配阈值
	@note    :
	@param   : locator_handle       有效的定位句柄
	@param   : out_thresh           获取的阈值信息
	@return  : AlgoError
	@Author  : shimai@2023/12/21
	********************************************/
	C_API AlgoError C_CALL PadFineLocateGetThresh(AlgoHandle locator_handle, float* out_thresh);

	/*******************************************
	@brief   : 导出定位结果图像
	@note    :
	@param   : locator_handle       有效的定位句柄
	@param   : out_png_filename     图像保存路径 
	@return  : AlgoError
	@Author  : shimai@2023/12/21
	********************************************/
	C_API AlgoError C_CALL PadFineLocateExportLocateResultPng(AlgoHandle locator_handle, const char* out_png_filename);

	/*********************************************
	@brief   : 获取buffer大小
	@note    :
	@param   : locator_handle       有效的定位句柄
	@param   : out_len              buffer大小
	@return  : AlgoError
	@Author  : shimai@2023/12/21
	**********************************************/
	C_API AlgoError C_CALL PadFineLocateGetBufferSize(AlgoHandle locator_handle, uint64_t* out_len);

	/******************************************
	@brief   : 将检测数据导入buffer里
	@note    :
	@param   : locator_handle      有效的定位句柄
	@param   : out_buffer          输出检测参数的buffer空间
	@param   : in_out_len          buffer空间大小
	@return  : AlgoError
	@Author  : shimai@2023/12/21
	*******************************************/
	C_API AlgoError C_CALL PadFineLocateToBuffer(AlgoHandle locator_handle, char* out_buffer, uint64_t* in_out_len);

	/************************************
	@brief    : 从buffer中导入数据到PadLocate
	@note     :
	@param    : locator_handle      有效的定位句柄
	@param    : in_buffer           buffer
	@param    : in_len              buffer大小
	@return   : AlgoError
	@Author   : shimai@2023/12/21
	************************************/
	C_API AlgoError C_CALL PadFineLocateFromBuffer(AlgoHandle locator_handle, const char* in_buffer, uint64_t in_len);

	/*******************************************
	@brief   : 空间内存释放
	@note    : 句柄指向的对象销毁，此句柄已经无效，请配合handle=nullptr
	@param   : locator_handle
	@return  :
	@Author  : shimai@2023/12/21
	********************************************/
	C_API void C_CALL PadFineLocateDestroyHandle(AlgoHandle locator_handle);

	/***********************************************
	@brief   : 句柄指向的对象的内存空间释放，句柄指向的对象还在
	@note    : 内存释放
	@param   : locator_handle    需要释放的句柄
	@return  : AlgoError
	@Author  : shimai@2023/12/21
	************************************************/
	C_API AlgoError C_CALL PadFineLocateRelease(AlgoHandle locator_handle);

#ifdef __cplusplus
}
#endif
#endif