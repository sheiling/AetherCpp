#ifndef __GEOMETRY_TRANSFORMER_H__
#define __GEOMETRY_TRANSFORMER_H__

#include "ci_define.h"
#include "ci_declare.h"
#include "ci_data.h"
#include "geometry_transformer.h"
#include "ci_measurement_enum.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


    /************************************
    @brief  : 创建一有效的Transformer句柄
    @note   : 相当于C++中实例化Transformer对象
    @param  : out_transform_handle       指针
    @return	: AlgoError
    @Author : YangShangqi@2022/11/19
    ************************************/
    C_API AlgoError C_CALL TransformerCreateHandle(AlgoHandle* out_transform_handle);

    /************************************
    @brief  : 句柄指向的对象的内存空间释放
    @note   : 句柄指向的对象还在
              Release 相当于: OBJ.Release();
    @param  : transform_handle       有效的Transform句柄
    @return : AlgoError
    @Author : YangShangqi@2022/11/19
    ************************************/
    C_API AlgoError C_CALL TransformerRelease(AlgoHandle transform_handle);


    /************************************
    @brief  : 句柄指向的对象销毁
    @note   : 此句柄已经无效, 请配合handle=nullptr使用
              Destroy 相当于:
                OBJ.release();
                delete handle;
    @param  : transform_handle       有效的Transform句柄
    @return : AlgoError
    @Author : YangShangqi@2022/11/19
    ************************************/
    C_API void C_CALL TransformerDestroyHandle(AlgoHandle transform_handle);


    /************************************
    @brief  : 计算用于 **超分成像** 的透视变化矩阵
    @note   : 
    @param  : transform_handle       有效的Transform句柄
    @param  : in_corner4             变换前的角点（要求等于4）
    @param  : in_img_wh              待变换图像的尺寸
    @param  : in_num                 输入角点个数
    @return : AlgoError
    @Author : YangShangqi@2022/11/19
    ************************************/
    // in_number
    C_API AlgoError C_CALL TransformerComputeTransMatBlur( AlgoHandle transform_handle, const AlgoPoint* in_corner4 ,const AlgoSize in_img_wh,int in_num);




	/************************************
	@brief  : 计算 **超分成像** 的透视变化矩阵目标点的参数
	@note   :
	@param  : transform_handle       有效的Transform句柄
	@param  : proportion_in          表示透视变化的目标点占整个区域的比例，默认0.85f
	@param  : ratio_mask_size_in     表示透视变化后的图像比例大小，默认1.6f
	@return : AlgoError
	@Author : tianbo@2023/08/19
	************************************/
	C_API AlgoError C_CALL TransformerSetParamForBlur(AlgoHandle transform_handle, const float in_proportion, const float in_ratio_mask_size);



	/************************************
	@brief  : 设置transform目标点基于最小外接矩形四点的缩放比例(基于四点中心的缩放)
	@note   :
	@param  : transform_handle       有效的Transform句柄
	@param  : in_scalar_ratio        缩放比例
	@return : AlgoError
	@Author : tianbo@2025/12/24
	************************************/
	C_API AlgoError C_CALL TransformerSetScalarRatio(AlgoHandle transform_handle, float in_scalar_ratio);


	/************************************
	@brief  : 在使用transformer做透变时的插值方式，不设置时默认cubic
	@note   :
	@param  : transform_handle       有效的Transform句柄
	@param  : in_inter_method        插值方式，在头文件ci_measurement_enum.h中定义，公共的。
	@return : AlgoError
	@Author : tianbo@2025/12/24
	************************************/
	C_API AlgoError C_CALL TransformerSetInterpolationMethod(AlgoHandle transform_handle, CiInterpolationMethod in_inter_method);

    /************************************
    @brief  : 计算透视变化矩阵
    @note   : 通过输入的四个角点的最小外接矩形确定变换后的角点
			  支持ScalarRatio参数，以左上角角点为原点进行缩放
    @param  : transform_handle       有效的Transform句柄
    @param  : in_corner4             变换前的角点（要求等于4）
    @param  : in_num                 输入角点个数
    @return : AlgoError
    @Author : YangShangqi@2022/11/19        ratio.           
    ************************************/
    C_API AlgoError C_CALL TransformerComputeTransMat(AlgoHandle transform_handle, const AlgoPoint* in_corner4,  int in_num);



	/************************************
	@brief  : 计算透视变化矩阵
	@note   : 通过输入的四个角点的最小外接矩形确定变换后的角点
			  支持ScalarRatio参数，以左上角角点为原点进行缩放
	@param  : transform_handle       有效的Transform句柄
	@param  : in_corner4             变换前的角点（要求等于4），排列必须是左上、右上、右下、左下
	@param  : in_num                 输入角点个数
	@return : AlgoError
	@Author : tianbo@2026/01/08        ratio.
	************************************/
	C_API AlgoError C_CALL TransformerComputeTransMat2f(AlgoHandle transform_handle, const AlgoPoint2f* in_corner4, int in_num);


    /************************************
    @brief  : 对图像进行透视变换
    @note   : 要求在计算透视变换矩阵之后
    @param  : transform_handle     有效的Transform句柄
    @param  : in_shot             有效的shot句柄，未进行透视变化的原始Shot
    @param  : out_shot             有效的shot句柄, 输出
    @return : AlgoError
    @Author : YangShangqi@2022/11/19
    ************************************/
    C_API AlgoError C_CALL TransformerTransImg(AlgoHandle transform_handle,const AlgoHandle in_shot, AlgoHandle out_shot);

    /************************************
    @brief  : 向buffer中写入数据
    @note   : 
    @param  : transform_handle      有效的Transform句柄
    @param  : out_buffer            buffer的指针
    @return : in_out_len            buffer的长度
    @Author : YangShangqi@2022/11/19
    ************************************/
    C_API AlgoError C_CALL TransformerToBuffer(AlgoHandle transform_handle,char* out_buffer, uint64_t* in_out_len);

    /************************************
    @brief  : 获取Transformer的buffer大小
    @note   : 
    @param  : transform_handle         有效的Transform句柄
    @param  : out_len                  buffer的长度(输出)
    @return : AlgoError
    @Author : YangShangqi@2022/11/19
    ************************************/
    C_API AlgoError C_CALL TransformerGetBufferSize(AlgoHandle transform_handle, uint64_t* out_len);


    /************************************
    @brief  : 从buffer中读取数据
    @note   : 注释
    @param  : transform_handle      有效的Transform句柄
    @param  : in_buffer             buffer的指针
    @param  : in_len                buffer的长度
    @return : AlgoError
    @Author : YangShangqi@2022/11/19
    ************************************/
    C_API AlgoError C_CALL TransformerFromBuffer(AlgoHandle transform_handle, char* in_buffer, uint64_t in_len);

     /************************************
     @brief  : 将transformer对象输出到 **.transformer 文件中
     @note   : 文件后缀为  .transformer
     @param  : transform_handle      有效的Transform句柄
     @param  : in_file_path          文件地址
     @return : AlgoError
     @Author : YangShangqi@2022/11/19
     ************************************/
    C_API AlgoError C_CALL TransformerSave(AlgoHandle transform_handle, char* in_file_path);


    /************************************
    @brief  : 从  **.transformer 文件中 读取数据
    @note   : 文件后缀为  .transformer
    @param  : transform_handle      有效的Transform句柄
    @param  : in_file_path          文件地址
    @return : AlgoError
    @Author : YangShangqi@2022/11/19
    ************************************/
    C_API AlgoError C_CALL TransformerLoad(AlgoHandle transform_handle, char* in_file_path);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // !__TRANSDORM_COMPUTE_H__
