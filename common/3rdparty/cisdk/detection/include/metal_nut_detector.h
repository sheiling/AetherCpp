#ifndef __METAL_NUT_DETECTOR_H__
#define __METAL_NUT_DETECTOR_H__

#include "ci_data.h"
#include "ci_declare.h"
#include "meta_info_data.h"
#include "metal_nut_detector_data.h"
#include "metal_nut_detector_error.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

	/**************************************
	@brief   : 创建MetalNutDetector句柄
	@note    :
	@param   : out_detect_handle
	@return  : AlgoError
	@Author  : songshaojun@2023/01/07
	**************************************/
	C_API AlgoError C_CALL MetalNutDetectCreateHandle(AlgoHandle* out_detect_handle);

	/**************************************
	@brief   : 设置底壳螺母ROI
	@note    : 一次性可将所有ROI传入
	@param   : metal_nut_detect_handle
	@param   : in_mask， AlgoRect类型,存放了所有的ROI
	@param   : roi_num,roi的个数，int类型
	@return  : AlgoError
	@Author  : songshaojun@2023/01/07
	**************************************/
	C_API AlgoError C_CALL MetalNutDetectSetROI(AlgoHandle metal_nut_detect_handle, const AlgoRect* in_roi,const int roi_num);

	/**************************************
	@brief   : 获取底壳螺母Mask个数，查看设置了几个mask
	@note    : 
	@param   : metal_nut_detect_handle
	@param   : out_mask_num ，mask个数，int类型
	@return  : AlgoError
	@Author  : songshaojun@2023/01/09
	**************************************/
	//C_API AlgoError C_CALL NutDetectorGetMaskNum(AlgoHandle metal_nut_detect_handle, int *out_mask_num);


	/**************************************
	@brief   : 检测底壳螺母是否存在
	@note    : 返回值为int*类型，，0代表没有螺母，1代表有螺母；数字的位置代表了螺母的位置
	@param   : metal_nut_detect_handle
	@param   : in_image 输入的待检测图像，AlgoMat类型
	@param   : out_result 返回的检测结果，int*类型
	@return  : AlgoError
	@Author  : songshaojun@2023/01/07
	**************************************/
	C_API AlgoError C_CALL MetalNutDetect(AlgoHandle metal_nut_detect_handle, const AlgoMat* in_image, int *out_result);


	/**************************************
	@brief   : 获取检测结果。当没有检测结果时，只将roi区域绘制在输入图像上；当有检测结果时，将检测结果也同步绘制在输入图像上；
	@note    : 该功能可供排查错误，调试时使用
	@param   : metal_nut_detect_handle
	@param   : in_img 输入的图像，AlgoMat类型，将检测结果绘制在哪张图像上
	@param   : detect_result，int*类型，该输入可为空，如果为空，则不绘制检测结果
	@param   : in_dir 输出的结果路径 char*类型
	@return  : AlgoError
	@Author  : songshaojun@2023/01/09
	**************************************/
	C_API AlgoError C_CALL MetalNutDetectExportResult(AlgoHandle metal_nut_detect_handle, const AlgoMat* in_img,const int* detect_result, const char* in_dir);


	/**************************************
	@brief   : 设置底壳螺母检测参数
	@note    :
	@param   : metal_nut_detect_handle
	@param   : in_thre ，类型为AlgoMetalNutThreshold
	@return  : AlgoError
	@Author  : songshaojun@2023/01/07
	**************************************/
	C_API AlgoError C_CALL MetalNutDetectSetThresh(AlgoHandle nut_detect_handle,const AlgoMetalNutThreshold* in_thre);

	/**************************************
	@brief   : 获取底壳螺母检测参数
	@note    : 
	@param   : nut_detect_handle
	@param   : out_thresh ，返回的阈值，AlgoMetalNutThreshold类型
	@return  : AlgoError
	@Author  : songshaojun@2023/01/07
	**************************************/
	C_API AlgoError C_CALL MetalNutDetectGetThresh(AlgoHandle nut_detect_handle, AlgoMetalNutThreshold *out_thresh);

	/************************************
	@brief    : 获取buffer大小
	@note     :
	@param    : nut_detect_handle          有效的nut_detect句柄
	@param    : out_len，buffer大小，uint64_t* 类型
	@return   : AlgoError
	@Modified : songshoajun@2023/01/08
	************************************/
	C_API AlgoError C_CALL MetalNutDetectGetBufferSize(AlgoHandle nut_detect_handle, uint64_t* out_len);

	/************************************
	@brief    : 将NutDetect里面的数据导出到buffer
	@note     :
	@param    : nut_detect_handle       有效的nut_detect句柄
	@param    : out_buffer				已经申请好申请空间
	@param    : in_out_len          外部申请好的空间大小,会用真实大小修改
	@return   : AlgoError
	@Modified : songshaojun@2023/01/08
	************************************/
	C_API AlgoError C_CALL MetalNutDetectToBuffer(AlgoHandle nut_detect_handle, char* out_buffer, uint64_t* in_out_len);

	/************************************
	@brief    : 将buffer数据导入到NutDetect
	@note     :
	@param    : nut_detect_handle          有效的nut_detect句柄
	@param    : in_buffer
	@param    : in_len
	@return   : AlgoError
	@Modified : songshaojun@2023/01/08
	************************************/
	C_API AlgoError C_CALL MetalNutDetectFromBuffer(AlgoHandle nut_detect_handle, const char* in_buffer, uint64_t in_len);

	/***********************************************
	@brief   : 句柄指向的对象的内存空间释放，句柄指向的对象还在
	@note    : 内存释放
	@param   : release_handle
	@return  : AlgoError
	@Author  : songshaojun@2023/01/07
	************************************************/
	C_API AlgoError C_CALL MetalNutDetectRelease(AlgoHandle release_handle);

	/**************************************
	@brief   : 空间内存释放
	@note    : 句柄指向的对象销毁，此句柄已经无效，请配合handle = nullptr
	@param   : nut_detect_handle
	@return  : void
	@Author  : songshaojun@2023/01/07
	**************************************/
	C_API void C_CALL MetalNutDetectDestroyHandle(AlgoHandle nut_detect_handle);


	

#ifdef __cplusplus
}
#endif
#endif 
