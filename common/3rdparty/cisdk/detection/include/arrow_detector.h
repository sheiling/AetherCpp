#ifndef __ARROW_DETECTOR_H__
#define __ARROW_DETECTOR_H__

#include "ci_data.h"
#include "ci_declare.h"
#include "meta_info_data.h"
#include "arrow_detector_data.h"
#include "arrow_detector_error.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

	/**************************************
	@brief   : 创建Arrow句柄
	@note    :
	@param   : out_detector_handle
	@return  : AlgoError
	@Author  : songshaojun@2023/01/08
	**************************************/
	C_API AlgoError C_CALL ArrowCreateHandle(AlgoHandle* out_detector_handle);

	/**************************************
	@brief   : 设置箭头检测阈值
	@note    : 可根据不同的检测类型（PCB或底壳），设置不同的入参阈值
	@param   : arrow_handle
	@param   : in_threshold， 检测箭头方向的参数，类型为AlgoArrowThreshold
	@return  : AlgoError
	@Author  : songshaojun@2023/01/08
	**************************************/
	C_API AlgoError C_CALL ArrowSetThresh(AlgoHandle arrow_handle, const AlgoArrowThreshold *in_threshold);

	/**************************************
	@brief   : 获取箭头检测检测参数
	@note    :
	@param   : arrow_handle
	@param   :  ，out_thresh 返回的阈值，AlgoArrowThreshold类型
	@return  : AlgoError
	@Author  : songshaojun@2023/01/08
	**************************************/
	C_API AlgoError C_CALL ArrowGetThresh(AlgoHandle arrow_handle, AlgoArrowThreshold *out_thresh);

	/************************************
	@brief    : 从model模块中设置pcb模板
	@note     :
	@param    : arrow_handle         有效的ArrowDetoetor句柄
	@param    : in_model_handle       model句柄
	@param    : in_arrow_label 的模板方向
	@param    : in_template_name  模板名字,生成model文件时有设置
	@return   : AlgoError
	@Modified : songshaojun@2023/01/09
	************************************/
	C_API AlgoError C_CALL ArrowSetTemplate(AlgoHandle arrow_handle, AlgoHandle in_model_handle, const char* in_template_name, const AlgoArrowLabel *in_arrow_label);

	/************************************
	@brief    : 从model文件中载入模板
	@note     :
	@param    : arrow_handle               有效的ArrowDetoet句柄
	@param    : in_arrow_template_files    arrow模板文件
	@param    : in_template_name            模板名字
	@return   : AlgoError
	@Modified : songshaojun@2023/01/09
	************************************/
	C_API AlgoError C_CALL ArrowLoadTemplate(AlgoHandle arrow_handle, const char* in_arrow_template_files, const char* in_template_name);

	/**************************************
	@brief   : 设置箭头检测ROI
	@note    : 箭头检测一般只传入一个ROI
	@param   : arrow_handle
	@param   : in_roi 输入， AlgoRect类型
	@return  : AlgoError
	@Author  : songshaojun@2023/01/09
	**************************************/
	C_API AlgoError C_CALL ArrowSetROI(AlgoHandle arrow_handle, const AlgoRect* in_roi);

	/************************************
	@brief    : 检测箭头方向
	@note     : 使用该箭头方向代表检测图像的箭头朝向
	@param    : arrow_handle       有效的arrow_detector句柄
	@param    : in_image            待检测图像
	@param    : out_result          检测结果，AlgoArrowLabel类型，返回箭头的方向
	@return   : AlgoError
	@Modified : songshaojun@2023/01/09
	************************************/
	C_API AlgoError C_CALL ArrowDetectDirection(AlgoHandle arrow_handle, const AlgoMat* in_image,AlgoArrowLabel* out_result);

	/************************************
	@brief    : 获取检测buffer大小
	@note     :
	@param    : arrow_handle          有效的arrow_detector句柄
	@param    : out_len                buffer大小
	@return   : AlgoError
	@Modified : songshaojun@2023/01/09
	************************************/
	C_API AlgoError C_CALL ArrowGetBufferSize(AlgoHandle arrow_handle, uint64_t* out_len);

	/************************************
	@brief    : 将Arrow里面的数据导出到buffer
	@note     :
	@param    : arrow_handle       有效的arrow_detector句柄
	@param    : out_buffer          已经申请好申请空间
	@param    : in_out_len          外部申请好的空间大小,会用真实大小修改
	@return   : AlgoError
	@Modified : songshaojun@2023/01/09
	************************************/
	C_API AlgoError C_CALL ArrowToBuffer(AlgoHandle arrow_handle, char* out_buffer, uint64_t* in_out_len);

	/************************************
	@brief    : 将buffer数据导入到Arrow
	@note     :
	@param    : arrow_handle          有效的arrow_detector句柄
	@param    : in_buffer
	@param    : in_len
	@return   : AlgoError
	@Modified : songshaojun@2023/01/09
	************************************/
	C_API AlgoError C_CALL ArrowFromBuffer(AlgoHandle arrow_handle, const char* in_buffer, uint64_t in_len);

	/***********************************************
	@brief   : 句柄指向的对象的内存空间释放，句柄指向的对象还在
	@note    : 内存释放
	@param   : release_handle
	@return  : AlgoError
	@Author  : songshaojun@2023/01/08
	************************************************/
	C_API AlgoError C_CALL ArrowRelease(AlgoHandle release_handle);

	/**************************************
	@brief   : 空间内存释放
	@note    : 句柄指向的对象销毁，此句柄已经无效，请配合handle = nullptr
	@param   : arrow_handle
	@return  : void
	@Author  : songshaojun@2023/01/08
	**************************************/
	C_API void C_CALL ArrowDestroyHandle(AlgoHandle arrow_handle);

	
#ifdef __cplusplus
}
#endif
#endif 
