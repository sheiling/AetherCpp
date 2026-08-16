#ifndef _DL_DIE_BONDING_DETECTOR_H_
#define _DL_DIE_BONDING_DETECTOR_H_

#include "ci_data.h"
#include "algo_mat.h"
#include "die_bonding_defect_detector_data.h"
#include "die_bonding_defect_detector_error.h"
#include "dl_die_bonding_detector_data.h"
#include "dl_die_bonding_detector_errors.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


/*********************************************
@brief   : 创建DLDieBondingCerateHandle检测句柄
@note    :
@param   : detector_handle
@return  : AlgoError
@Author  : shimai@2024/08/16
*********************************************/
C_API AlgoError C_CALL DLDieBondingCreateHandle(AlgoHandle* detector_handle);


/*********************************************
@brief   : 加载深度学习模型
@note    :
@param   : detector_handle         --检测句柄
@param   : in_model_file           --模型文件路径
@param   : in_key                  --模型密钥
@param   : in_model_type           --模型类型(由于本模块包含多个功能，需要多个模型，因此需要指定模型类型)
@param   : img_sz                  --模型输入图像大小(不同模型对于输入图像的大小要求不同)
@param   : max_batch               --模型最大batch(模型支持对图像并行处理来加速，因此需要指定模型支持的最大batch大小)
@param   : in_engine               --模型推理引擎(0:ONNX_CPU, 1:ONNX_GPU, 2:TRT) 根据设备选择不同引擎，默认为CPU
                                     如果使用GPU加速，需要具备GPU环境，且需要安装好CUDA 和 CUDNN，否则会加载失败
                                     使用ONNX_CPU速度最慢，TRT速度最快
@return  : AlgoError
@Author  : shimai@2024/08/16
*********************************************/
C_API AlgoError C_CALL DLDieBondingLoadModel(AlgoHandle detector_handle, const char* in_model_file, const char* in_key, DLDieBondingModelType in_model_type, int max_batch = 1, int in_engine = 0);


/*********************************************
@brief   : 设置待检测灯板相关参数
@note    :
@param   : detector_handle         --检测句柄
@param   : in_parameter            --灯板参数
@return  : AlgoError
@Author  : shimai@2024/08/16
*********************************************/
C_API AlgoError C_CALL DLDieBondingSetParameter(AlgoHandle detector_handle,  const DLDieBondingParameter* in_parameter);


/*******************************************
@brief   : 设定当前图像的索引（索引需要具有唯一性，且要与所属图像对应），用于后续数据融合时确定某一数据属于哪张图像
@note    : 
@param   : detector_handle      有效的检测句柄
@param   : in_image_index		输入的图像索引
@return  : AlgoError
@Author  : shimai@2024/08/16
********************************************/
C_API AlgoError C_CALL DLDieBondingSetImageIndex(AlgoHandle detector_handle, const char* in_image_index);


/*******************************************
@brief   : 设定芯片组置信度阈值
@note    : 该接口仅针对芯片组设置置信度阈值
@param   : detector_handle      有效的检测句柄
@param   : in_group_thresh		输入芯片组的置信度阈值
@return  : AlgoError
@Author  : shimai@2024/12/04
********************************************/
C_API AlgoError C_CALL DLDieBondingSetGroupConfThresh(AlgoHandle detector_handle, float in_group_thresh);

/*******************************************
@brief   : 设定RGB芯片置信度阈值
@note    : 该接口仅针对RGB单个芯片设置置信度阈值
@param   : detector_handle      有效的检测句柄
@param   : in_chip_thresh		输入芯片的置信度阈值
@return  : AlgoError
@Author  : wushaoshuang@2025/07/30
********************************************/
C_API AlgoError C_CALL DLDieBondingSetChipConfThresh(AlgoHandle detector_handle, float in_chip_thresh);

/*******************************************
@brief   : 设置缺陷分类阈值
@note    : 设置缺陷分类阈值，该阈值有两个，一个为芯片阈值，一个为焊盘阈值，表示的意义为：当芯片置信度大于该芯片阈值时，则认为是正常芯片，不进行后续的缺陷分类，反之进行缺陷分类，当焊盘置信度大于该焊盘阈值时，则认为时正常焊盘，不进行后续的缺陷分类，反之进行缺陷分类；该两个阈值的主要目的是使正常的芯片及焊盘不参与缺陷分类以提高检测效率
@param   : detector_handle      有效的检测句柄
@param   : in_chip_thresh		输入芯片阈值，范围为0-1，一般推荐0.8，调节方法为：当有正常芯片被检测为缺陷时，则该值过大，应减小该阈值，当缺陷芯片被检测为正常时，则该值太小，应增大该阈值，调节步长推荐为0.1；内部默认为0.8
@param   : in_solder_thresh		输入焊盘阈值，范围为0-1，一般推荐0.8，调节方法为：当有正常焊盘被检测为缺陷时，则该值过大，应减小该阈值，当缺陷焊盘被检测为正常时，则该值太小，应增大该阈值，调节步长推荐为0.1；内部默认为0.8
@return  : AlgoError
@Author  : wushaoshuang@2025/09/06
********************************************/
C_API AlgoError C_CALL DLDieBondingSetDefectThresh(AlgoHandle detector_handle, const float in_chip_thresh, const float in_solder_thresh);


/*******************************************
@brief   : 设置锡膏检测阈值
@note    : 设置锡膏检测阈值，该值表示锡膏检测的置信度，该值范围为0-1，默认为0.25，外部可根据实际检测情况调整，调节步长为0.05，当锡膏区域过检时，调大该值，漏检时调小该值。
@param   : detector_handle      有效的检测句柄
@param   : in_solder_paste_thresh		输入阈值
@return  : AlgoError
@Author  : wushaoshuang@2026/02/09
********************************************/
C_API AlgoError C_CALL DLDieBondingSetSolderPasteThresh(AlgoHandle detector_handle, const float in_solder_paste_thresh);

/*********************************************
@brief   : 获取芯片组数量
@note    : 获取检测到的芯片组数量
@param   : detector_handle         --检测句柄
@param   : out_num                 --输出检测出的芯片组数量
@return  : AlgoError
@Author  : wushaoshuang@2025/08/02
**********************************************/
C_API AlgoError C_CALL DLDieBondingGetGroupNum(AlgoHandle detector_handle, int* out_num);

/*********************************************
@brief   : 芯片组定位
@note    :
@param   : detector_handle         --检测句柄
@param   : in_image                --输入图像
@param   : out_row                 --输出检测出的芯片组行数目
@param   : out_col                 --输出检测出的芯片组列数目
@param   : crop_img_flag           --是否分块进行定位(将图像裁剪成多份进行定位，如果图像中的芯片组数量大于100个时建议开启把这里设置为true)
@return  : AlgoError
@Author  : shimai@2024/08/16
**********************************************/
C_API AlgoError C_CALL DLDieBondingChipGroupLocate(AlgoHandle detector_handle, const AlgoMat* in_image, int* out_row, int* out_col, bool crop_img_flag = false);


/*********************************************
@brief   : 芯片检测
@note    :
@param   : detector_handle         --检测句柄
@param   : is_defect_classify      --是否进行缺陷分类
@param   : is_polarity_detect      --是否进行极性检测
@param   : is_solder_paste_detect  --是否进行锡膏检测
@return  : AlgoError
@Author  : shimai@2024/08/16
**********************************************/
C_API AlgoError C_CALL DLDieBondingDetect(AlgoHandle detector_handle, const bool& is_defect_classify, const bool& is_polarity_detect, const bool& is_solder_paste_detect);


/*********************************************
@brief   : 获取全部芯片组信息(芯片组信息中存储了检测结果)
@note    :
@param   : detector_handle         --检测句柄
@param   : out_chip_group_info     --输出芯片检测结果
@return  : AlgoError
@Author  : shimai@2024/08/16
*********************************************/
C_API AlgoError C_CALL DLDieBondingGetAllChipGroupInfo(AlgoHandle detector_handle, DieBondingChipGroupInfo* out_chip_group_info);


/***********************************************
@brief   : 导出芯片检测检测结果图像（需要完成DLDieBondingDetect）
@note    :
@param   : detector_handle      有效的检测句柄
@param   : out_image_filename   图像保存路径
@return  : AlgoError
@Author  : shimai@2024/08/16
************************************************/
C_API AlgoError C_CALL DLDieBondingExportDetectResult(AlgoHandle detector_handle, const char* out_image_filename);

/***********************************************
@brief   : 获取检测检测结果图像（需要完成DLDieBondingDetect）
@note    : 获取检测检测结果图像，以AlgoMat类型输出检测结果图像，外部获取后可以自由处理，外部也可直接存储，存储效率较DLDieBondingExportDetectResult有大大提升
@param   : detector_handle      有效的检测句柄
@param   : out_image            输出的检测结果图，以AlgoMat数据输出
@return  : AlgoError
@Author  : wushaoshuang@2025/09/24
************************************************/
C_API AlgoError C_CALL DLDieBondingGetResultImage(AlgoHandle detector_handle, AlgoMat* out_image);

/*********************************************
@brief   : 获取buffer大小
@note    :
@param   : detector_handle      有效的检测句柄
@param   : out_len              buffer大小
@return  : AlgoError
@Author  : shimai@2024/08/16
**********************************************/
C_API AlgoError C_CALL DLDieBondingGetBufferSize(AlgoHandle detector_handle, uint64_t* out_len);


/******************************************
@brief   : 将数据导出buffer里
@note    :
@param   : detector_handle     有效的检测句柄
@param   : out_buffer          输出检测参数的buffer空间
@param   : in_out_len          buffer空间大小
@return  : AlgoError
@Author  : shimai@2024/08/16
*******************************************/
C_API AlgoError C_CALL DLDieBondingToBuffer(AlgoHandle detector_handle, char* out_buffer, uint64_t* in_out_len);


/************************************
@brief    : 从buffer中导入数据到句柄
@note     :
@param    : detector_handle      有效的检测句柄
@param    : in_buffer           buffer
@param    : in_len              buffer大小
@return   : AlgoError
@Author   : shimai@2024/08/16
************************************/
C_API AlgoError C_CALL DLDieBondingFromBuffer(AlgoHandle detector_handle, const char* in_buffer, uint64_t in_len);


/***********************************************
@brief   : 载入检测结果
@note    : 后缀名只能是.chipgroupinfo
@param   : detector_handle    有效的检测句柄
@param   : in_fullname        后缀只能是.chipgroupinfo
@return  : AlgoError
@Author  : shimai@2024/08/16
************************************************/
C_API AlgoError C_CALL DLDieBondingLoad(AlgoHandle detector_handle, const char* in_fullname);


/***********************************************
@brief   : 保存检测结果
@note    : 后缀名只能是.chipgroupinfo
@param   : detector_handle    有效的检测句柄
@param   : in_fullname        后缀只能是.chipgroupinfo
@return  : AlgoError
@Author  : shimai@2024/08/16
************************************************/
C_API AlgoError C_CALL DLDieBondingSave(AlgoHandle detector_handle, const char* in_fullname);


/***********************************************
@brief   : 句柄指向的对象的内存空间释放，句柄指向的对象还在
@note    : 内存释放
@param   : detector_handle    需要释放的句柄
@return  : AlgoError
@Author  : shimai@2024/08/16
************************************************/
C_API AlgoError C_CALL DLDieBondingRelease(AlgoHandle detector_handle);


/*******************************************
@brief   : 空间内存释放
@note    : 句柄指向的对象销毁，此句柄已经无效，请配合handle=nullptr
@param   : detector_handle      有效的检测句柄
@return  :
@Author  : shimai@2024/08/16
********************************************/
C_API void C_CALL DLDieBondingDestroyHandle(AlgoHandle detector_handle);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif