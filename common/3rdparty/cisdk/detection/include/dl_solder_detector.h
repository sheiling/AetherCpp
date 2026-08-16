#ifndef __DL_SOLDER_DETECTOR_H__
#define __DL_SOLDER_DETECTOR_H__

#include <stdint.h>
#include "ci_declare.h"
#include "ci_define.h"
#include "dl_solder_detector_data.h"
#include "dl_solder_detector_errors.h"
#include "algo_mat.h"
#include <vector>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	/************************************
	 @brief  : 初始化句柄
	 @note   :
	 @param  :
	 @return : AlgoHandle
	 @Author : zhoujun@2024/05/28
	 ************************************/
	C_API AlgoHandle C_CALL DLSolderCreateHandle();


	/************************************
	 @brief  : 加载模型
	 @note   :
	 @param  : solder_handle
	 @param  : in_model_file			 --onnx模型地址
	 @param  : in_trt_file				 --trt模型地址
	 @param  : in_key					 --密钥
	 @param  : in_engine				 --0：onnxruntime-cpu，1:onnxruntime-gpu，2:tensorrt-gpu。可拓展。
	 @return : AlgoError
	 @Author : zhoujun@2024/05/28
	 ************************************/
	C_API AlgoError C_CALL DLSolderLoadModel(AlgoHandle solder_handle, const std::string in_model_file, const std::string in_trt_file, const std::string in_key, int in_engine);


	/************************************
	 @brief  : 加载模型
	 @note   :
	 @param  : solder_handle
	 @param  : in_model_file			 --onnx模型地址
	 @param  : in_trt_file				 --trt模型地址
	 @param  : in_key					 --密钥
	 @param  : in_engine				 --0：onnxruntime-cpu，1:onnxruntime-gpu，2:tensorrt-gpu。可拓展。
	 @return : AlgoError
	 @Author : zhoujun@2024/05/28
	 ************************************/
	C_API AlgoError C_CALL DLSolderLoadDefectDetectModel(AlgoHandle solder_handle, const std::string in_model_file, const std::string in_trt_file, const std::string in_key, int in_engine);


	/************************************
	 @brief  : 设置solder相关参数
	 @note   :
	 @param  : solder_handle
	 @param  : in_solder_direction       --HORIAONTAL = 1, 水平方向，即焊盘一左一右;VERTICAL = 2, 垂直方向，即焊盘一上一下
	 @param  : in_solder_size			 --焊盘的大小
	 @return : AlgoError
	 @Author : zhoujun@2024/05/28
	 ************************************/
	C_API AlgoError C_CALL DLSolderSetInfo(AlgoHandle solder_handle, SolderDirection in_solder_direction,AlgoPoint2f in_solder_size);


	/************************************
	 @brief  : 设置深度学习相关参数
	 @note   :
	 @param  : solder_handle
	 @param  : in_th					 --深度学习阈值，取值均在0到1之间
	 @return : AlgoError
	 @Author : zhoujun@2024/05/28
	 ************************************/
	C_API AlgoError C_CALL  DLSolderSetThresh(AlgoHandle solder_handle, const SolerThresh in_th);


	/************************************
	@brief    : 焊盘整体及左右两个位置定位（共输出三个位置信息）
	@note     :
	@param    : solder_handle            --有效的solder句柄
	@param    : in_image                 --需要定位的图像
	@param    : out_pad_center           --检测得到的pad中心像素坐标
	@param    : out_pad_angle            --检测得到的pad的偏移角度
	@param    : out_left_pad_center      --检测到的左侧焊盘中心像素坐标
	@param    : out_right_pad_center     --检测到的右侧焊盘中心像素坐标
	@return   : AlgoError
	@Modified : haopanxia@2022/09/30
	************************************/
	C_API AlgoError C_CALL DLSolderDetect(AlgoHandle solder_handle, const AlgoMat* in_image,
		AlgoPoint2f* out_pad_center, float* out_pad_angle, AlgoPoint2f* out_left_pad_center, AlgoPoint2f* out_right_pad_center);


	/************************************
	 @brief  : 输出结果
	 @note   :
	 @param  : solder_handle
	 @param  : out_cls_res				--焊盘的分类结果
	 @return : AlgoError
	 @Author : zhoujun@2024/09/13
	 ************************************/
	C_API AlgoError C_CALL DLSolderDefectDetect(AlgoHandle solder_handle, DefectDetection out_cls_res);


	/************************************
	 @brief  : 输出结果
	 @note   :
	 @param  : solder_handle
	 @param  : out_img_file				--输出图像的地址
	 @return : AlgoError
	 @Author : zhoujun@2024/05/28
	 ************************************/
	C_API AlgoError C_CALL DLSolderExportResultPng(AlgoHandle solder_handle, const std::string &out_img_file);
	

	/************************************
	 @brief  : 获取结果的size
	 @note   :
	 @param  : solder_handle
	 @param  : len						--获取结果的长度
	 @return : AlgoError
	 @Author : zhoujun@2024/05/28
	 ************************************/
	//C_API AlgoError C_CALL DLSolderGetBBoxSize(AlgoHandle solder_handle, uint64_t *len);


	/************************************
	 @brief  : 获取结果
	 @note   :
	 @param  : solder_handle
	 @param  : in_bbox						 --获取的结果bbox
	 @return : AlgoError
	 @Author : zhoujun@2024/05/28
	 ************************************/
	//C_API AlgoError C_CALL DLSolderGetBBox(AlgoHandle solder_handle, DetectionResult*in_bbox);


	/************************************
	 @brief  : 获取buffer大小
	 @note   :
	 @param  : solder_handle			--当前有效句柄
	 @param  : out_len					--buffer大小
	 @return : AlgoError
	 @Author : zhoujun@2024/05/28
	 ************************************/
	C_API AlgoError C_CALL DLSolderGetBufferSize(AlgoHandle solder_handle, uint64_t* out_len);


	/************************************
	@brief  : 将数据导出到buffer
	@note   :
	@param  : solder_handle				--句柄
	@param  : out_buffer				--已经申请好申请空间
	@param  : in_out_len				--外部申请好的空间大小,会用真实大小修改
	@return	: AlgoError
	@Author : zhoujun@2024/05/28
	************************************/
	C_API AlgoError C_CALL DLSolderToBuffer(AlgoHandle solder_handle, char* out_buffer, uint64_t* in_out_len);


	/************************************
	@brief  : 将buffer数据导入
	@note   :
	@param  : solder_handle   
	@param  : in_buffer
	@param  : in_len
	@return	: AlgoError
	@Author : zhoujun@2024/05/28
	************************************/
	C_API AlgoError C_CALL DLSolderFromBuffer(AlgoHandle solder_handle, const char* in_buffer, uint64_t in_len);


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
	C_API AlgoError C_CALL DLSolderRelease(AlgoHandle solder_handle);
	C_API void C_CALL DLSolderDestroyHandle(AlgoHandle solder_handle);


#ifdef __cplusplus
}
#endif // __cplusplus
#endif