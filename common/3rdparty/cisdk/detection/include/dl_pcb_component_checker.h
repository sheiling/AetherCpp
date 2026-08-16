#ifndef _DL_PCB_COMPONENT_CHECKER_H_
#define _DL_PCB_COMPONENT_CHECKER_H_

#include "ci_basic_error.h"
#include "ci_frontend_modname.h"
#include "ci_detection_modname.h"
#include "ci_data.h"
#include "algo_mat.h"
#include "dl_pcb_component_checker_data.h"
#include "dl_pcb_component_checker_errors.h"


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


/*********************************************
@brief   : 创建DLPcbComponent检测句柄
@note    :
@param   : checker_handle
@return  : AlgoError
@Author  : shimai@2024/08/19
*********************************************/
C_API AlgoError C_CALL DLPcbComponentCreateHandle(AlgoHandle* checker_handle);


/*********************************************
@brief   : 加载深度学习模型
@note    :
@param   : checker_handle          --检测句柄
@param   : in_model_file           --模型文件路径
@param   : in_key                  --模型密钥
@param   : img_sz                  --模型输入图像大小(不同模型对于输入图像的大小要求不同)
@param   : max_batch               --模型最大batch(模型支持对图像并行处理来加速，因此需要指定模型支持的最大batch大小)
@param   : in_engine               --模型推理引擎(0:ONNX_CPU, 1:ONNX_GPU, 2:TRT) 根据设备选择不同引擎，默认为CPU
                                     如果使用GPU加速，需要具备GPU环境，且需要安装好CUDA 和 CUDNN，否则会加载失败
                                     使用ONNX_CPU速度最慢，TRT速度最快
@return  : AlgoError
@Author  : shimai@2024/08/19
*********************************************/
C_API AlgoError C_CALL DLPcbComponentLoadModel(AlgoHandle checker_handle, const char* in_model_file, const char* in_key, int img_sz = 320, int max_batch = 1, int in_engine = 0);


/*********************************************
@brief   : 元件有无检测
@note    :
@param   : checker_handle               --检测句柄
@param   : in_image                     --输入图像
@param   : in_component_coordinates     --输入的元件坐标
@param   : in_component_num             --输入的元件数量
@return  : AlgoError
@Author  : shimai@2024/08/19
**********************************************/
C_API AlgoError C_CALL DLPcbComponentCheck(AlgoHandle checker_handle, const AlgoMat* in_image, const AlgoRect* in_component_coordinates, const int in_component_num);


/*********************************************
@brief   : 获取全部元件信息
@note    :
@param   : checker_handle          --检测句柄
@param   : out_component_infos     --输出元件检查结果
@return  : AlgoError
@Author  : shimai@2024/08/19
*********************************************/
C_API AlgoError C_CALL DLPcbComponentGetInfos(AlgoHandle checker_handle, DLPcbComponentInfo* out_component_infos);


/***********************************************
@brief   : 导出元件有无检测结果图像（需要完成DLPcbComponentCheck）
@note    :
@param   : checker_handle       有效的检测句柄
@param   : out_image_filename   图像保存路径
@return  : AlgoError
@Author  : shimai@2024/08/19
************************************************/
C_API AlgoError C_CALL DLPcbComponentExportResult(AlgoHandle checker_handle, const char* out_image_filename);


/*********************************************
@brief   : 获取buffer大小
@note    :
@param   : checker_handle       有效的检测句柄
@param   : out_len              buffer大小
@return  : AlgoError
@Author  : shimai@2024/08/19
**********************************************/
C_API AlgoError C_CALL DLPcbComponentGetBufferSize(AlgoHandle checker_handle, uint64_t* out_len);


/******************************************
@brief   : 将数据导出buffer里
@note    :
@param   : checker_handle      有效的检测句柄
@param   : out_buffer          输出检测参数的buffer空间
@param   : in_out_len          buffer空间大小
@return  : AlgoError
@Author  : shimai@2024/08/19
*******************************************/
C_API AlgoError C_CALL DLPcbComponentToBuffer(AlgoHandle checker_handle, char* out_buffer, uint64_t* in_out_len);


/************************************
@brief    : 从buffer中导入数据到句柄
@note     :
@param    : checker_handle      有效的检测句柄
@param    : in_buffer           buffer
@param    : in_len              buffer大小
@return   : AlgoError
@Author   : shimai@2024/08/19
************************************/
C_API AlgoError C_CALL DLPcbComponentFromBuffer(AlgoHandle checker_handle, const char* in_buffer, uint64_t in_len);


/***********************************************
@brief   : 载入检测结果
@note    : 后缀名只能是.componentinfo
@param   : checker_handle     有效的检测句柄
@param   : in_fullname        后缀只能是.componentinfo
@return  : AlgoError
@Author  : shimai@2024/08/19
************************************************/
C_API AlgoError C_CALL DLPcbComponentLoad(AlgoHandle checker_handle, const char* in_fullname);


/***********************************************
@brief   : 保存检测结果
@note    : 后缀名只能是.componentinfo
@param   : checker_handle     有效的检测句柄
@param   : in_fullname        后缀只能是.componentinfo
@return  : AlgoError
@Author  : shimai@2024/08/19
************************************************/
C_API AlgoError C_CALL DLPcbComponentSave(AlgoHandle checker_handle, const char* in_fullname);


/***********************************************
@brief   : 句柄指向的对象的内存空间释放，句柄指向的对象还在
@note    : 内存释放
@param   : checker_handle    需要释放的句柄
@return  : AlgoError
@Author  : shimai@2024/08/19
************************************************/
C_API AlgoError C_CALL DLPcbComponentRelease(AlgoHandle checker_handle);


/*******************************************
@brief   : 空间内存释放
@note    : 句柄指向的对象销毁，此句柄已经无效，请配合handle=nullptr
@param   : checker_handle      有效的检测句柄
@return  :
@Author  : shimai@2024/08/19
********************************************/
C_API void C_CALL DLPcbComponentDestroyHandle(AlgoHandle checker_handle);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif