#ifndef __WAFER_ANGLE_DETECTOR_H__
#define __WAFER_ANGLE_DETECTOR_H__

#include "ci_data.h"
#include "wafer_angle_detector_data.h"
#include "wafer_angle_detector_error.h"
#include "algo_mat.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

	/*********************************************
	@brief   : 创建WaferAngleDetector句柄
	@note    :
	@param   : detector_handle
	@return  : AlgoError
	@Author  : shimai@2023/09/16
	*********************************************/
	C_API AlgoError C_CALL WaferAngleCreateHandle(AlgoHandle* detector_handle);

	/******************************************
	@brief   : wafer角度检测
	@note    :
	@param   : detector_handle      有效的检测句柄
	@param   : in_image             需要检测的图像
	@param   : out_degree		    检测得到的wafer需要旋转的角度，正数为逆时针，负数为顺时针
	@return  : AlgoError
	@Author  : shimai@2023/09/16
	*******************************************/
	C_API AlgoError C_CALL WaferAngleDetect(AlgoHandle detector_handle, const AlgoMat* in_image, float* out_degree);

	/*******************************************
	@brief   : 输出检测的轮廓图像
	@note    : 输出为png图像
	@param   : detector_handle      有效的检测句柄
	@param   : out_png_filename     图像保存路径
	@return  : AlgoError
	@Author  : shimai@2023/09/20
	********************************************/
	C_API AlgoError C_CALL WaferAngleExportContourPng(AlgoHandle detector_handle, const char* out_png_filename);

	/*********************************************
	@brief   : 获取buffer大小
	@note    :
	@param   : detector_handle      有效的检测句柄
	@param   : out_len              buffer大小
	@return  : AlgoError
	@Author  : shimai@2023/09/16
	**********************************************/
	C_API AlgoError C_CALL WaferAngleGetBufferSize(AlgoHandle detector_handle, uint64_t* out_len);

	/******************************************
	@brief   : 将检测数据导入buffer里
	@note    :
	@param   : detector_handle     有效的检测句柄
	@param   : out_buffer          输出检测参数的buffer空间
	@param   : in_out_len          buffer空间大小
	@return  : AlgoError
	@Author  : shimai@2023/09/16
	*******************************************/
	C_API AlgoError C_CALL WaferAngleToBuffer(AlgoHandle detector_handle, char* out_buffer, uint64_t* in_out_len);

	/************************************
	@brief    : 从buffer中导入数据到WaferAngle
	@note     :
	@param    : detector_handle     有效的检测句柄
	@param    : in_buffer           buffer
	@param    : in_len              buffer大小
	@return   : AlgoError
	@Author   : shimai@2023/09/16
	************************************/
	C_API AlgoError C_CALL WaferAngleFromBuffer(AlgoHandle detector_handle, const char* in_buffer, uint64_t in_len);

	/*******************************************
	@brief   : 空间内存释放
	@note    : 句柄指向的对象销毁，此句柄已经无效，请配合handle=nullptr
	@param   : detector_handle
	@return  :
	@Author  : shimai@2023/09/16
	********************************************/
	C_API void C_CALL WaferAngleDestroyHandle(AlgoHandle detector_handle);

	/***********************************************
	@brief   : 句柄指向的对象的内存空间释放，句柄指向的对象还在
	@note    : 内存释放
	@param   : detector_handle    需要释放的句柄
	@return  : AlgoError
	@Author  : shimai@2023/09/16
	************************************************/
	C_API AlgoError C_CALL WaferAngleRelease(AlgoHandle detector_handle);

#ifdef __cplusplus
}
#endif
#endif