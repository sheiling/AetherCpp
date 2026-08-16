#ifndef ____WAFER_PAD_LOCATOR_H__
#define ____WAFER_PAD_LOCATOR_H__

#include "ci_declare.h"
#include "ci_define.h"
#include "ci_data.h"
#include "wafer_pad_locator_data.h"
#include "wafer_cell_locator_data.h"
#include "wafer_pad_locator_errors.h"
#include "algo_mat.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


    /************************************
    @brief    : 创建句柄
    @note     : 
    @return   : AlgoHandle
    @Modified : songshaojun@2023/07/03
    ************************************/
	C_API AlgoHandle C_CALL WaferPadCreateHandle();


	/**********************************************
	@brief   : 设置wafer上Cell信息
	@note    :
	@param   : locator_handle   有效的定位句柄
	@param   : in_pad_info      输入pad的信息，详见wafer_cell_pad_locator_data.h
	@return  : AlgoError
	@Author  : songshaojun@2023/07/02
	***********************************************/
	C_API AlgoError C_CALL WaferPadSetInfo(AlgoHandle locator_handle, WaferCellPadInfo* in_pad_info);


	/********************************************
	@brief   : 设置pad点分割阈值
	@note    : 
	@param   : locator_handle     有效的定位句柄
	@param   : in_segment_thresh  pad点分割阈值
	@return  : AlgoError
	@Author  : songshaojun@2023/07/03
	*********************************************/
	//C_API AlgoError C_CALL WaferCellPadSetSegmentTh(AlgoHandle locator_handle, int* in_segment_thresh);


	/********************************************
	@brief   : 设置cell顶点匹配阈值
	@note    : 
	@param   : locator_handle     有效的定位句柄
	@param   : in_match_thresh    cell顶点匹配阈值
	@return  : AlgoError
	@Author  : songshaojun@2023/07/03
	*********************************************/
	//C_API AlgoError C_CALL WaferCellPadSetMatchTh(AlgoHandle locator_handle, float* in_match_thresh);


	/********************************************
	@brief   : 设置顶点模板的接口
	@note    : 配合model模块使用
	@param   : locator_handle    有效的定位句柄
	@param   : model_handle      model句柄
	@param   : in_template_name  模板名字
	@param   : in_cell_vertex    输入模板中，cell顶点的坐标,cell顶点的坐标是以模板图像的左上角为（0，0）点为参考
	@return  : AlgoError
	@Author  : songshaojun@2023/07/03
	*********************************************/
	C_API AlgoError C_CALL  WaferPadSetTemplate(AlgoHandle locator_handle, AlgoHandle model_handle,
		const char* in_template_name, const AlgoPoint2f* in_cell_vertex);

	/********************************************
	@brief   : 设置定位阈值的接口
	@note    : 
	@param   : locator_handle    有效的定位句柄
	@param   : in_segment_thresh 分割阈值
	@param   : in_match_thresh   匹配阈值
	@return  : AlgoError
	@Author  : songshaojun@2023/11/17
	*********************************************/
	C_API AlgoError C_CALL  WaferPadSetLocateThreshold(AlgoHandle locator_handle, int in_segment_thresh, float in_match_thresh);






	/********************************************
	@brief   : 生成待检测图像
	@note    : 1、img1为第一张图像，img2为第二张图像,取img1部分数据拼接到img2上，拼接方式为垂直拼接；
	           2、从img1取多少拼接到img2上通过参数concat_num决定；
	@param   : locator_handle    有效的定位句柄
	@param   : img1              输入的第一张图像
	@param   : img2              输入的第二张图像
	@param   : concat_num        两张样本重叠的像素数
	@param   : out_locate_img    输出的待检测定位图像
	@return  : AlgoError
	@Author  : songshaojun@2023/08/17
	*********************************************/
	C_API AlgoError C_CALL  WaferPadGenLocateImage(AlgoHandle locator_handle, 
		const AlgoMat *img1, const AlgoMat *img2,const int *concat_num, AlgoMat *out_locate_img);


	/******************************************
	@brief   : 获取单张样本上共有几组pad点，cell顶点
	@note    : pad结果和cell顶点结果分开存储，但相互对应
	           pad结果的存储形式：
			   6N*6,6N中6代表每组pad点含有6个pad点，N代表共检测到了N组pad点，每一行存储的内容如下：
			   0 1 2 3 4    5 
			   x,y,w,h,α，index    x,y代表pad中心坐标，w,h代表pad宽高，α代表pad角度，index代表当前是第几组pad

			   cell顶点的结果存储形式
			   N*3，N代表共检测到了N个Cell顶点，每一行内容存储形式如下：
			   0 1   2 
			   x,y,index    x,y代表顶点坐标 index代表当前是第几个cell顶点

	@param   : locator_handle       有效的定位句柄
	@param   : in_image             输入待检测图像
	@param   : out_pad_result       检测到的pad组结果
	@param   : out_vertex_result    检测到的cell顶点结果
	@return  : AlgoError
	@Author  : songshaojun@2023/08/22
	*******************************************/
	C_API AlgoError C_CALL WaferPadGetLocateResult(AlgoHandle locator_handle,const AlgoMat* in_image, 
		AlgoMat* out_pad_result, AlgoMat* out_vertex_result);


	/******************************************
	@brief   : 导出单张样本的pad点定位结果
	@note    : 仅供测试使用!!! 
	@param   : locator_handle       有效的定位句柄
	@param   : out_result_dir       结果存放路径
	@Author  : songshaojun@2023/08/16
	*******************************************/
	C_API AlgoError C_CALL WaferPadExportPadPng(AlgoHandle locator_handle, const char* out_result_png);

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
	C_API AlgoError C_CALL WaferPadRelease(AlgoHandle locator_handle);
	C_API void C_CALL WaferPadDestroyHandle(AlgoHandle locator_handle);





#ifdef __cplusplus
}
#endif // __cplusplus
#endif