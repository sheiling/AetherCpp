#ifndef __DL_MIDDLE_TRANS_DIE_DETECTOR_H__
#define __DL_MIDDLE_TRANS_DIE_DETECTOR_H__

#include <stdint.h>
#include "ci_declare.h"
#include "ci_define.h"
#include "dl_middle_trans_die_detector_data.h"
#include "dl_middle_trans_die_detector_errors.h"
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
	C_API AlgoHandle C_CALL DLMiddleTransDieCreateHandle();


	/************************************
	 @brief  : 加载定位模型
	 @note   :
	 @param  : die_handle
	 @param  : in_model_file		 --onnx模型地址
	 @param  : in_trt_file			 --trt模型地址
	 @param  : in_key              --密钥
	 @param  : in_engine           --0：onnxruntime-cpu，1:onnxruntime-gpu，2:tensorrt-gpu。可拓展。默认为0
	 @return : AlgoError
	 @Author : zhoujun@2024/05/28
	 ************************************/
	C_API AlgoError C_CALL DLMiddleTransDieLoadLocateModel(AlgoHandle die_handle, const std::string in_model_file, const std::string in_trt_file, const std::string in_key, int in_engine=0);


	/************************************
	 @brief  : 加载检测模型
	 @note   :
	 @param  : die_handle
	 @param  : in_model_file		 --onnx模型地址
	 @param  : in_trt_file			 --trt模型地址
	 @param  : in_key              --密钥
	 @param  : in_engine           --0:onnxruntime-cpu，1:onnxruntime-gpu，2:tensorrt-gpu。可拓展。默认为0
	 @return : AlgoError
	 @Author : zhoujun@2024/05/28
	 ************************************/
	C_API AlgoError C_CALL DLMiddleTransDieLoadDetectModel(AlgoHandle die_handle, const std::string in_model_file, const std::string in_trt_file, const std::string in_key, int in_engine=0);



	/************************************
	 @brief  : 加载检测模型
	 @note   :
	 @param  : die_handle
	 @param  : in_model_file		 --onnx模型地址
	 @param  : in_trt_file			 --trt模型地址
	 @param  : in_key              --密钥
	 @param  : in_engine           --0:onnxruntime-cpu，1:onnxruntime-gpu，2:tensorrt-gpu。可拓展。默认为0
	 @return : AlgoError
	 @Author : zhoujun@2024/09/26
	 ************************************/
	C_API AlgoError C_CALL DLMiddleTransDieLoadPolarityModel(AlgoHandle die_handle, const std::string in_model_file, const std::string in_trt_file, const std::string in_key, int in_engine = 0);


	/************************************
	 @brief  : 设置深度学习相关参数
	 @note   :
	 @param  : die_handle
	 @param  : in_th			  --深度学习阈值，取值在0到1之间
	 @return : AlgoError
	 @Author : zhoujun@2024/05/28
	 ************************************/
	C_API AlgoError C_CALL DLMiddleTransDieSetThresh(AlgoHandle die_handle, const MiddleThresh in_th);


	/************************************
	@brief    : 中转位处die精细定位（只计算一个die）
	@note     :
	@param    : die_handle        --有效的die句柄
	@param    : in_image          --需要定位的图像
	@param    : out_die_center    --中转位die中心与图像中心的像素偏移量（x=列，y=行）
	@param    : out_die_angle     --芯片的偏移角度，单位/°
	@return   : AlgoError
	@Modified : zhoujun@2024/05/28
	************************************/
	C_API AlgoError C_CALL DLMiddleTransDieDetect(AlgoHandle die_handle, const AlgoMat* in_image,
		AlgoPoint2f* out_die_center, float* out_die_angle);

	/************************************
	@brief    : 中转位处die精细定位（只计算一个die）
	@note     :
	@param    : die_handle        --有效的die句柄
	@param    : in_image          --需要定位的图像
	@param    : in_die_center    --中转位die中心与图像中心的像素偏移量（x=列，y=行）
	@param    : in_die_angle     --芯片的偏移角度，单位/°
	@param    : out_pola          --极性
	@return   : AlgoError
	@Modified : zhoujun@2024/05/28
	************************************/
	C_API AlgoError C_CALL DLMiddleTransDiePolarityDetect(AlgoHandle die_handle, const AlgoMat* in_image,
		AlgoPoint2f* in_die_center, float* in_die_angle, Polarity out_pola);


	/************************************
	@brief    : 导出结果
	@note     :
	@param    : die_handle        --有效的die句柄
	@param    : out_img_file      --需要定位的图像
	@return   : AlgoError
	@Modified : zhoujun@2024/05/28
	************************************/
	C_API AlgoError C_CALL DLMiddleTransDieExportResultPng(AlgoHandle die_handle, const std::string &out_img_file);


	/************************************
	 @brief  : 获取结果的size
	 @note   :
	 @param  : die_handle
	 @param  : out_len             --获取的结果bbox的size
	 @return : AlgoError
	 @Author : zhoujun@2024/05/28
	 ************************************/
	C_API AlgoError C_CALL DLMiddleTransDieGetBBoxSize(AlgoHandle die_handle, uint64_t *out_len);


	/************************************
	 @brief  : 获取结果
	 @note   :
	 @param  : die_handle
	 @param  : in_bbox            --获取的结果in_bbox
	 @return : AlgoError
	 @Author : zhoujun@2024/05/28
	 ************************************/
	//C_API AlgoError C_CALL DLMiddleTransDieGetBBox(AlgoHandle die_handle, DetectionResult*in_bbox);


	/************************************
	 @brief  : 获取buffer大小
	 @note   :
	 @param  : die_handle       --当前有效句柄
	 @param  : out_len          --buffer大小
	 @return : AlgoError
	 @Author : zhoujun@2024/05/28
	 ************************************/
	//C_API AlgoError C_CALL DLMiddleTransDieGetBufferSize(AlgoHandle die_handle, uint64_t* out_len);


	/************************************
	 @brief  : 数据导出到buffer
	 @note   :
	 @param  : die_handle
	 @param  : out_buffer       --已经申请好申请空间
	 @param  : in_out_len       --外部申请好的空间大小,会用真实大小修改
	 @return : AlgoError
	 @Author : zhoujun@2024/05/28
	************************************/
	C_API AlgoError C_CALL DLMiddleTransDieToBuffer(AlgoHandle die_handle, char* out_buffer, uint64_t* in_out_len);


	/************************************
	 @brief  : 将buffer数据导入
	 @note   :
	 @param  : die_handle
	 @param  : in_buffer
	 @param  : in_len
	 @return : AlgoError
	 @Author : zhoujun@2024/05/28
	************************************/
	C_API AlgoError C_CALL DLMiddleTransDieFromBuffer(AlgoHandle die_handle, const char* in_buffer, uint64_t in_len);


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
	C_API AlgoError C_CALL DLMiddleTransDieRelease(AlgoHandle die_handle);
	C_API void C_CALL DLMiddleTransDieDestroyHandle(AlgoHandle die_handle);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif