#ifndef __MURA_DEFECT_DETECTOR_H__
#define __MURA_DEFECT_DETECTOR_H__

#include "ci_data.h"
#include "ci_declare.h"
#include "algo_mat.h"
#include "mura_defect_detector_data.h"
#include "mura_defect_detector_error.h"


#ifdef __cplusplus
extern "C" {
#endif //__cplusplus
	/*************************************************
	@brief   : 创建MuraDefect句柄
	@note    :
	@param   : inspect_handle
	@return  : AlgoError
	@Author  : wushaoshuang@2023/06/04
	**************************************************/
	C_API AlgoError C_CALL MuraDefectCreateHandle(AlgoHandle* inspect_handle);

	/*************************************************
	@brief   : 异常点检测接口
	@note    : 检测是否存在异常点
	@param   : inspect_handle       有效的检测句柄
	@param   : in_image             待检测数据，AlgoMat类型
	@param   : in_diff_thresh       异常点检测阈值，数据差异大于此阈值时，表示存在异常点
	@param   : out_result           输出检测结果，为bool类型值，表示是否存在异常点
	@return  : AlgoError
	@Author  : wushaoshuang@2023/06/04
	**************************************************/
	C_API AlgoError C_CALL MuraDefectAbnormalPoint(AlgoHandle inspect_handle, AlgoMat* in_image, double* in_diff_thresh, bool* out_result);

	/*************************************************
	@brief   : 异常块检测接口
	@note    : 检测是否存在异常块，输出块之间的差异值及异常块的数量
	@param   : inspect_handle       有效的检测句柄
	@param   : in_image             待检测数据，AlgoMat类型
	@param   : in_diff_thresh       异常块检测阈值，块之间的差异值（离散程度）大于此阈值时，表示存在异常块
	@param   : in_col_num           输入的列分块数量  
	@param   : in_row_num           输入的行分块数量
	@param   : out_diff_value       输出的块之间的差异值（离散程度）
	@param   : out_block_num        输出的异常块数量，与检测阈值相关
	@return  : AlgoError
	@Author  : wushaoshuang@2023/06/04
	**************************************************/
	C_API AlgoError C_CALL MuraDefectAbnormalBlock(AlgoHandle inspect_handle, AlgoMat* in_image, double* in_diff_thresh, int* in_col_num, int* in_row_num, double* out_diff_value, int* out_block_num);

	/*************************************************
	@brief   : 获取异常块索引
	@note    : out_index  存储屏体上块的位置索引 i*col+j
	@param   : inspect_handle       有效的检测句柄
	@param   : in_num               异常块的数量，从MuraDefectAbnormalBlock中获得
	@param   : out_index            输出的异常块的索引，表示屏幕上第几个块，从第一行第一列开始算
	@return  : AlgoError
	@Author  : wushaoshuang@2023/06/04
	**************************************************/
	C_API AlgoError C_CALL MuraDefectGetBlockIndex(AlgoHandle inspect_handle, int* in_num, int* out_index);

	/*******************************************
	@brief   : 空间内存释放
	@note    : 句柄指向的对象销毁，此句柄已经无效，请配合handle=nullptr
	@param   : inspect_handle
	@return  :
	@Author  : wushaoshuang@2023/06/04
	********************************************/
	C_API void C_CALL MuraDefectDestroyHandle(AlgoHandle inspect_handle);

	/***********************************************
	@brief   : 句柄指向的对象的内存空间释放，句柄指向的对象还在
	@note    : 内存释放
	@param   : inspect_handle    需要释放的句柄
	@return  : AlgoError
	@Author  : wushaoshuang@2023/06/04
	************************************************/
	C_API AlgoError C_CALL MuraDefectRelease(AlgoHandle inspect_handle);

#ifdef __cplusplus
}
#endif
#endif
