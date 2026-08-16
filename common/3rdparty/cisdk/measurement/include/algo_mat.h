#ifndef __ALGO_MAT_H__
#define __ALGO_MAT_H__

#include "ci_declare.h"
#include "algo_mat_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus




    /************************************
    @brief  : new 一个AlgoMat,默认值new一个空实体
    @note   : 和AlgoMatDelete配对使用
    @param  : rows     行数
    @param  : cols     列数
    @param  : type     类型
    @param  : init_val 初始值, 如果是多通道,每个值都是init_val, 例如32FC3, 每个元素都是 (0.f 0.f 0.f)
    @return : AlgoError
    @Author : chengjiayi@2022/08/01
    ************************************/
    C_API AlgoMat* C_CALL AlgoMatNew(int rows = 0, int cols = 0, AlgoMatType type = ALGO_8UC1, double init_val = 0.0);






    /************************************
    @brief  : delete new的AlgoMat
    @note   : 和AlgoMatNew配对使用,使用完注意置空,algo_img=nullptr
    @param  : algo_img
    @return	: AlgoError
    @Author : chengjiayi@2021/07/15
    ************************************/
    C_API void C_CALL AlgoMatDelete(AlgoMat* algo_img);



    /************************************
    @brief  : 创建一个AlgoMat实体, 默认值创建一个空实体
    @note   : 和AlgoMatFree配对使用
    @param  : rows     行数
    @param  : cols     列数
    @param  : type     类型
    @param  : init_val 初始值, 如果是多通道,每个值都是init_val, 例如32FC3, 每个元素都是 (0.f 0.f 0.f)
    @return : AlgoError
    @Author : chengjiayi@2022/08/01
    ************************************/
    C_API AlgoMat C_CALL AlgoMatCreate(int rows = 0, int cols = 0, AlgoMatType type = ALGO_8UC1, double init_val = 0.0);

	/************************************
	@brief  : 从内存创建一个AlgoMat实体, 先申请空间, 然后从目标内存拷贝过来, 构造algomat
	@note   : 1. 区别点: AlgoMatCreateShadow不申请内存, 直接用给定的内存构造algomat
			  2. 使用AlgoMatFree释放

	@param  : rows     行数
	@param  : cols     列数
	@param  : type     类型
	@param  : p_data   内存指针, 长度应该为rows*cols*一个元素的大小(字节)
	@param  : mem_size p_data内存大小, 单位字节
	@return : AlgoMat  异常时候,返回空algomat, 可以用AlgoMatIsEmpty进行判定
	@Author : chengjiayi@2023/11/24
	************************************/
	C_API AlgoMat C_CALL AlgoMatCreateFromMem(int rows, int cols, AlgoMatType type, const char* p_data, unsigned int mem_size);


    /************************************
    @brief  : 利用内存直接构造一个AlgoMat, 没有空间被申请, 但是AlgoMat中除本身结构体外, 没有分配任何内存空间
    @note   : 不能调用AlgoMatDelete或者AlgoMatFree去释放
    @param  : rows      行数
    @param  : cols      列数
    @param  : type      类型
    @param  : p_data    内存指针, 长度应该为rows*cols*一个元素的大小(字节)
    @return : AlgoError
    @Author : chengjiayi@2022/08/22
    ************************************/
    C_API AlgoMat C_CALL AlgoMatCreateShadow(int rows, int cols, AlgoMatType type, const char* p_data);



    /************************************
    @brief  : 释放AlgoMat实体空间
    @note   : 和AlgoMatCreate配对使用
    @param  : algo_img
    @return	: AlgoError
    @Author : chengjiayi@2021/07/15
    ************************************/
    C_API void C_CALL AlgoMatFree(AlgoMat* algo_img);



    /************************************
    @brief  : 检查AlgoMat 头
    @note   : 空的AlgoMat也没问题
    @param  : p_algo_img
    @return	: int            0=ok,1=error
    @Author : chengjiayi@2021/07/15
    ************************************/
    C_API int C_CALL AlgoMatCheckHeader(const AlgoMat* p_algo_img);


    /************************************
    @brief  : 检查数据有效性
    @note   : 
    @param  : p_algo_img
    @return	: int  0=非空,1=空
    @Author : chengjiayi@2021/07/15
    ************************************/
    C_API int C_CALL AlgoMatIsEmpty(const AlgoMat* p_algo_img);



    /************************************
    @brief  : 将src的数据拷贝到dst,深拷贝
    @note   : 
    @param  : src
    @param  : dst
    @return	: void
    @Author : chengjiayi@2022/01/25
    ************************************/
    C_API void C_CALL AlgoMatCopy(const AlgoMat* src, AlgoMat* dst);


    /************************************
    @brief  : 获取AlgoMat 元素
    @note   : 确保AlgoMat有效
    @param  : p_algo_img    AlgoMat
    @param  : r             元素行位置.0-base
    @param  : c             元素列位置.0-base
    @param  : idx_channel   哪个通道  例如 8UC3,一个元素含3个通道float, (r,c)表征哪个元素, idx_channel表征一个元素的哪个通道
    @return	: double        输出时按照double输出, 具体请根据AlgoMat类型再次转换下
    @Author : chengjiayi@2022/01/25
    ************************************/
    C_API double C_CALL AlgoMatGet(const AlgoMat* p_algo_img, int r, int c, int idx_channel = 0);


    
    /************************************
    @brief  : 设置AlgoMat 元素值
    @note   : 确保AlgoMat有效
    @param  : p_algo_img    AlgoMat
    @param  : val           设置值,输入时按照double传入, 但实际给AlgoMat赋值时,会将double转为相应类型
    @param  : r             元素行位置.0-base
    @param  : c             元素列位置.0-base
    @param  : idx_channel   哪个通道  例如 8UC3,一个元素含3个通道uchar, (r,c)表征哪个元素, idx_channel表征一个元素的哪个通道
    @return	: void
    @Author : chengjiayi@2021/07/15
    ************************************/
    C_API void C_CALL AlgoMatSet(AlgoMat* p_algo_img, double val, int r, int c, int idx_channel = 0);
    


    /************************************
    @brief  : 打印出矩阵到控制台
    @note   : 
    @param  : src       目标矩阵
    @param  : row_scale 打印前row_scale行,为-1时全部打印
    @param  : col_scale
    @param  : channel   选择哪个通道,-1时候,三个通道全部打印
    @return	: AlgoError
    @Author : chengjiayi@2022/03/01
    ************************************/
    C_API void C_CALL AlgoMatPrint(const AlgoMat* src, int row_scale = -1, int col_scale = -1, int channel = -1);



#ifdef __cplusplus
}
#endif // __cplusplus
#endif