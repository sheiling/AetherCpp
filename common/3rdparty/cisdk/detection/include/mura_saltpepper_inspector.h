#ifndef __MURA_SALTPEPPER_INSPECTOR_H__
#define __MURA_SALTPEPPER_INSPECTOR_H__
#include "ci_data.h"
#include "ci_declare.h"
#include "meta_info_data.h"
#include "mura_saltpepper_inspector_data.h"
#include "mura_saltpepper_inspector_error.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

	/**************************************
	@brief   : 创建MuraSaltpepperInspect句柄
	@note    :
	@param   : inspector_handle
	@return  : AlgoError
	@Author  : wushaoshuang@2022/09/29
	**************************************/
	C_API AlgoError C_CALL MuraSaltpepperInspectCreateHandle(AlgoHandle* inspector_handle);

	/**************************************
	@brief   : 灯板光通量图麻点缺陷检测
	@note    : 从光通量图上检测LED灯板上是否存在麻点缺陷：纯色图（R,G,B）
	@param   : inspector_handle  有效的inspector句柄，不能为空
	@param   : in_meter_handle 输入光通量提取结果句柄
	@param   : out_mura_saltpepper_level 输出麻点缺陷程度表征值
	@return  : AlgoError
	@Author  : wushaoshuang@2022/09/29
	***************************************/
	//C_API AlgoError C_CALL MuraSaltpepperInspectFromPanelMeter(AlgoHandle inspector_handle, const AlgoHandle in_meter_handle, float& out_mura_saltpepper_level);

	/****************************************
	@brief   : shot图像麻点精细检测
	@note    : 针对屏幕图像进行麻点检测，此接口为精细检测，在快速检测结果达不到检测要求时可用此接口检测。
	@param   : inspector_handle  有效的inspect句柄，不能为空
	@param   : in_shot_handle    有效的shot句柄，详见algo_shot.h中定义
	@param   : in_corner_point4   区域的四个角点坐标，顺序依次为顺时针，左上、右上、右下、左下
	@return  : AlgoError
	@Author  : wushaoshuang@2022/12/12
	****************************************/
	C_API AlgoError C_CALL MuraSaltpepperAccuracyInspectFromShot(AlgoHandle inspector_handle, const AlgoHandle in_shot_handle, AlgoPoint in_corner_point4[4]);

	/***************************************
	@brief   : shot图像麻点快速检测
	@note    : 针对屏幕图像进行麻点表征，此接口为快速检测，麻点检测时首先推荐使用此接口，此接口检测达不到要求时再使用精细检测接口
	@param   : inspector_handle  有效的inspect句柄，不能为空
	@param   : in_shot_handle  有效的shot句柄，详见algo_shot.h中定义
	@param   : in_corner_point4   区域的四个角点坐标，顺序依次为顺时针，左上、右上、右下、左下
	@return  : AlgoError
	@Author  : wushaoshuang@2022/12/12
	****************************************/
	C_API AlgoError C_CALL MuraSaltpepperFastInspectFromShot(AlgoHandle inspector_handle, const AlgoHandle in_shot_handle, AlgoPoint in_corner_point4[4]);

	/***************************************
	@brief   : 屏幕meter图像麻点精细检测
	@note    : 针对屏幕光通量图像进行麻点检测，此接口为精细检测，在快速检测结果达不到检测要求时可用此接口检测。
	@param   : inspector_handle  有效的检测句柄，不能为空
	@param   : in_meter_handle  有效的meter句柄，不能为空。
	@return  : AlgoError
	@Author  : wushaoshuang@2022/12/16
	****************************************/
	C_API AlgoError C_CALL MuraSaltpepperAccuracyInspectFromMeter(AlgoHandle inspector_handle, const AlgoHandle in_meter_handle);

	/***************************************
	@brief   : 屏幕meter图像麻点快速检测
	@note    : 针对屏幕光通量图像进行麻点表征，此接口为快速检测，麻点检测时首先推荐使用此接口，此接口检测达不到要求时再使用精细检测接口
	@param   : inspector_handle  有效的检测句柄，不能为空。
	@param   : in_meter_handle  有效的meter句柄，不能为空。
	@return  : AlgoError
	@Author  : wushaoshuang@2022/12/16
	****************************************/
	C_API AlgoError C_CALL MuraSaltpepperFastInspectFromMeter(AlgoHandle inspector_handle, const AlgoHandle in_meter_handle);

	/***************************************
	@brief   : 设置检测参数
	@brief   : 设置检测参数，检测参数部分为外部必须设定，部分为内部默认，具体详见mura_saltpepper_inspector_data.h中入参说明
	@param   : inspecor_handle  有效的检测句柄，不能为空
	@param   : in_parameter    输入的参数
	@return  : AlgoError
	@Author  : wushaoshuang@2022/12/28
	***************************************/
	C_API AlgoError C_CALL MuraSaltpepperInspectSetParameter(AlgoHandle inspector_handle, MuraSaltpepperInspectParameter* in_parameter);

	/****************************************
	@brief   : 输出检测参数
	@note    : 输出检测参数，外部需要时调用，可获取当前检测参数
	@param   : inspector_handle  有效的inspect句柄，不能为空
	@param   : out_parameter   输出的检测参数
	@return  : AlgoError
	@Author  : wushaoshuang@2022/12/12
	****************************************/
	C_API AlgoError C_CALL MuraSaltpepperInspectGetParameter(AlgoHandle inspector_handle, MuraSaltpepperInspectParameter* out_parameter);

	/****************************************
	@brief   : 输出检测结果
	@note    : 输出检测结果，灯板光通量检测输出表征值，屏幕检测输出检测结果图像
	@param   : inspector_handle  有效的inspect句柄，不能为空
	@param   : out_detect_result  输出的检测结果
	@return  : AlgoError
	@Author  : wushaoshuang@2022/12/12
	*****************************************/
	C_API AlgoError C_CALL MuraSaltpepperInspectGetResult(AlgoHandle inspector_handle, MuraSaltpepperInspectResult* out_detect_result);

    /******************************************
	@brief   : 设定检测阈值
	@note    : 设定检测阈值，具体详见mura_saltpepper_inspector_data.h中阈值说明
	@param   : inspector_handle  有效的inspector句柄，不能为空
	@param   : in_thresh   输入的阈值
	@return  : AlgoError
	@Author  : wushaoshuang@2022/09/29
	*******************************************/
	C_API AlgoError C_CALL MuraSaltpepperInspectSetThresh(AlgoHandle inspector_handle, const MuraSaltpepperInspectThresh* in_thresh);

	/*****************************************
	@brief   : 输出阈值
	@note    : 输出阈值,外部需要时调用，可获取当前检测阈值
	@param   : inspector_handle 有效的inspector句柄，不能为空
	@param   : out_thresh  输出的阈值
	@return  : AlgoError
	@Author  : wushaoshuang@2022/09/29
	******************************************/
	C_API AlgoError C_CALL MuraSaltpepperInspectGetThresh(AlgoHandle inspector_handle, MuraSaltpepperInspectThresh* out_thresh);

	/*****************************************
	@brief   : 获取检测参数所占buffer大小
	@note    : 
	@param   : inspector_handle 有效的inspector句柄，不能为空
	@param   : out_len  输出的buffer尺寸大小
	@return  : AlgoError
	@Author  : wushaoshuang@2022/12/29
	******************************************/
	C_API AlgoError C_CALL MuraSaltpepperInspectGetParameterBufferSzie(AlgoHandle inspector_handle, uint64_t* out_len);

	/******************************************
	@brief   : 将检测参数放至buffer里
	@note    : 
	@param   : inspector_handle 有效的inspector句柄，不能为空
	@param   : out_buffer  输出检测参数的buffer空间
	@param   : out_len   buffer尺寸大小
	@return  : AlgoError
	@Author  : wushaoshuang@2022/12/29
	*******************************************/
	C_API AlgoError C_CALL MuraSaltpepperInspectParameterToBuffer(AlgoHandle inspector_handle, char* out_buffer, uint64_t* out_len);

	/******************************************
	@brief   : 从buffer中获取检测参数至检测句柄
	@note    :
	@param   : inspector_handle 有效的inspector句柄，不能为空
	@param   : in_buffer  检测参数的buffer空间
	@param   : in_len    buffer尺寸大小
	@return  : AlgoError
	@Author  : wushaoshuang@2022/12/29
	*******************************************/
	C_API AlgoError C_CALL MuraSaltpepperInspectParameterFromBuffer(AlgoHandle inspector_handle, const char* in_buffer, uint64_t in_len);

	/*******************************************
	@brief   : 空间内存释放
	@note    : 句柄指向的对象销毁，此句柄已经无效，请配合handle=nullptr
	@param   : inspector_handle
	@return  : 
	@Author  : wushaoshuang@2022/09/29
	********************************************/
	C_API void C_CALL MuraSaltpepperInspectDestroyHandle(AlgoHandle inspector_handle);

	/***********************************************
	@brief   : 句柄指向的对象的内存空间释放，句柄指向的对象还在
	@note    : 内存释放
	@param   : release_handle
	@return  : AlgoError
	@Author  : wushaoshuang@2022/09/29
	************************************************/
	C_API AlgoError C_CALL MuraSaltpepperInspectRelease(AlgoHandle release_handle);

#ifdef __cplusplus
}
#endif
#endif 
