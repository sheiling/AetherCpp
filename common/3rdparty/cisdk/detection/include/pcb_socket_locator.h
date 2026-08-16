#ifndef __PCB_SOCKET_LOCATOR_H__
#define __PCB_SOCKET_LOCATOR_H__

#include "ci_declare.h"
#include "ci_data.h"
#include "pcb_socket_locator_data.h"
#include "pcb_socket_locator_error.h"
#include "algo_mat.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

	/*********************************************
	@brief   : 创建PcbSocketLocator句柄
	@note    : 
	@param   : locator_handle
	@return  : AlgoeError
	@Author  : wushaoshuang@2023/03/20
	*********************************************/
	C_API AlgoError C_CALL PcbSocketCreateHandle(AlgoHandle* locator_handle);

	///********************************************
	//@brief   : 设置定位阈值
	//@note    : 
	//@param   : locator_handle   有效的定位句柄
	//@param   : in_thresh        灯板对插孔定位阈值
	//@return  : AlgoError
	//@Author  : wushaoshuang@2023/03/20
	//*********************************************/
	//C_API AlgoError C_CALL PcbSocketSetThresh(AlgoHandle locator_handle, PcbSocketThresh* in_thresh);

	///**********************************************
	//@brief   : 设置灯板对插孔类型
	//@note    :
	//@param   : locator_handle   有效的定位句柄
	//@param   : in_socket_type   灯板对插孔类型flag，内部默认为0，详见pcb_socket_locator_data.h
	//@return  : AlgoError
	//@Author  : wushaoshuang@2023/03/27
	//***********************************************/
	//C_API AlgoError C_CALL PcbSocketSetType(AlgoHandle locator_handle, PcbSocketType* in_socket_type);

	///**********************************************
	//@brief   : 设置灯板对插孔定位方法
	//@note    :
	//@param   : locator_handle   有效的定位句柄
	//@param   : in_method_flag   定位方法标志，0为外接矩形定位(内部默认)，1为轮廓质心定位，详见pcb_socket_locator_data.h
	//@return  : AlgoError
	//@Author  : wushaoshuang@2023/03/27
	//***********************************************/
	//C_API AlgoError C_CALL PcbSocketSetMethod(AlgoHandle locator_handle, PcbSocketMethod* in_locate_method);

	///*********************************************
	//@brief   : 设置定位目标明暗转换关系
	//@note    :
	//@param   : locator_handle   有效的定位句柄
	//@param   : in_inverse_flag  是否转换标志，0为不进行明暗转换，1为进行明暗转换，内部默认不进行明暗转换，详见pcb_socket_locator_data.h
	//@return  : AlgoError
	//@Author  : wushaoshuang@2023/03/27
	//**********************************************/
	//C_API AlgoError C_CALL PcbSocketSetGrayInverse(AlgoHandle locator_handle, PcbSocketGrayInverse* in_gray_inverse);

	///**********************************************
	//@brief   : 获取当前定位阈值和相关参数
	//@note    : 
	//@param   : locator_handle     有效的定位句柄
	//@param   : out_thresh         返回的定位阈值
	//@param   : out_socket_type    返回的灯板对插孔类型
	//@param   : out_locate_method  返回的定位方法flag
	//@param   : out_gray_inverse   返回的明暗转换flag
	//@return  : AlgoError
	//@Author  : wushaoshuang@2023/03/23
	//***********************************************/
	//C_API AlgoError C_CALL PcbSocketGetParameter(AlgoHandle locator_handle,
	//	PcbSocketThresh* out_thresh, PcbSocketType* out_socket_type,
	//	PcbSocketMethod* out_locate_method, PcbSocketGrayInverse* out_gray_inverse);

	/********************************************
	@brief   : 设置模板
	@note    : 配合model模块使用，详情见DemoPcbSocketLocate()
	@param   : locator_handle    有效的定位句柄
	@param   : model_handle      model句柄
	@param   : in_template_name  模板名字
	@return  : AlgoError
	@Author  : wushaoshuang@2023/03/20
	*********************************************/
	C_API AlgoError C_CALL PcbSocketSetTemplate(AlgoHandle locator_handle, AlgoHandle model_handle, 
		const char* in_template_name);

	/******************************************
	@brief   : 灯板对插孔定位接口
	@note    :
	@param   : locator_handle       有效的定位句柄
	@param   : in_image             需要定位的图像
	@param   : out_locate_result    对插孔定位的结果结构体，详见pcb_socket_locator_data.h
	@param   : out_match_value      输出匹配度值，可作为外部筛选条件，根据此值外部设置阈值筛选定位结果，经综合测试，该值在0.5以上时定位结果较为可靠，不同灯板类型可设置不同的值
	@return  : AlgoError
	@Author  : wushaoshuang@2023/03/20
	*******************************************/
	C_API AlgoError C_CALL PcbSocketLocate(AlgoHandle locator_handle,const AlgoMat* in_image, 
		PcbSocketResult* out_locate_result, float* out_match_value);

	/*******************************************
	@brief   : 输出定位的结果图
	@note    : 输出为png图像
	@param   : locator_handle       有效的定位句柄
	@param   : out_png_filename     图像保存路径
	@return  : AlgoError
	@Author  : wushaoshaung@2023/03/20
	********************************************/
	C_API AlgoError C_CALL PcbSocketExportResultPng(AlgoHandle locator_handle, const char* out_png_filename);

	/********************************************
	@brief   : 获取定位的轮廓点数量
	@note    :
	@param   : locator_handle       有效的定位句柄
	@param   : out_num              输出点数量
	@return  : AlgoError
	@Author  : wushaoshuang@2023/03/22
	********************************************/
	C_API AlgoError C_CALL PcbSocketGetBorderSize(AlgoHandle locator_handle, int* out_num);

	/*********************************************
	@brief   : 获取定位的轮廓点集
	@note    : 使用前必须用PcbSocketGetBorderSize获取到轮廓点数量，用法见demo
	@param   : locator_handle       有效的定位句柄
	@param   : out_locate_border    定位的轮廓点集
	@return  : AlgoError     
	@Author  : wushaoshuang@2023/03/22
	**********************************************/
	C_API AlgoError C_CALL PcbSocketGetBorder(AlgoHandle locator_handle, AlgoPoint* out_border);

	/*********************************************
	@brief   : 获取buffer大小
	@note    :
	@param   : locator_handle       有效的定位句柄
	@param   : out_len              buffer大小
	@return  : AlgoError
	@Author  : wushaoshuang@2023/03/20
	**********************************************/
	C_API AlgoError C_CALL PcbSocketGetBufferSize(AlgoHandle locator_handle, uint64_t* out_len);

	/******************************************
    @brief   : 将定位数据导入buffer里
    @note    :
    @param   : locator_handle      有效的定位句柄
    @param   : out_buffer          输出检测参数的buffer空间
    @param   : in_out_len          buffer空间大小
    @return  : AlgoError
    @Author  : wushaoshuang@2023/03/20
    *******************************************/
	C_API AlgoError C_CALL PcbSocketToBuffer(AlgoHandle locator_handle, char* out_buffer, uint64_t* in_out_len);

	/************************************
    @brief    : 将buffer数据导入到PcbSocket
    @note     :
    @param    : locator_handle          有效的定位句柄
    @param    : in_buffer               buffer
    @param    : in_len                  buffer大小
    @return   : AlgoError
    @Author  : wushaoshuang@2023/03/20
    ************************************/
	C_API AlgoError C_CALL PcbSocketFromBuffer(AlgoHandle locator_handle, const char* in_buffer, uint64_t in_len);

	/*******************************************
    @brief   : 空间内存释放
    @note    : 句柄指向的对象销毁，此句柄已经无效，请配合handle=nullptr
    @param   : locator_handle
    @return  :
    @Author  : wushaoshuang@2023/03/20
    ********************************************/
	C_API void C_CALL PcbSocketDestroyHandle(AlgoHandle locator_handle);

	/***********************************************
    @brief   : 句柄指向的对象的内存空间释放，句柄指向的对象还在
    @note    : 内存释放
    @param   : locator_handle    需要释放的句柄
    @return  : AlgoError
    @Author  : wushaoshuang@2023/03/20
    ************************************************/
	C_API AlgoError C_CALL PcbSocketRelease(AlgoHandle locator_handle);




#ifdef __cplusplus
}
#endif
#endif 