#ifndef __DL_WAFER_DIE_DETECTOR_HPP__
#define __DL_WAFER_DIE_DETECTOR_HPP__
#include <stdint.h>
#include "ci_declare.h"
#include "ci_define.h"
#include "dl_wafer_die_detector_data.h"
#include "dl_wafer_die_detector_errors.h"
#include "algo_mat.h"
#include <vector>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


	C_API AlgoHandle C_CALL DLWaferDieCreateHandle();

	/************************************
	 @brief  : 加载模型
	 @note   :
	 @param  : die_handle
	 @param  : in_model_file	   --onnx模型地址
	 @param  : in_trt_file		   --trt模型地址
	 @param  : in_key              --密钥
	 @param  : in_engine           --0：onnxruntime-cpu，1:onnxruntime-gpu，2:tensorrt-gpu。可拓展。
	 @return : AlgoError
	 @Author : zhoujun@2024/05/28
	 ************************************/
	C_API AlgoError C_CALL DLWaferDieLoadModel(AlgoHandle die_handle, const std::string &in_model_file, const std::string& in_trt_file, const std::string &in_key, int in_engine =0);


	/************************************
	 @brief  : 设置深度学习阈值
	 @note   :
	 @param  : die_handle
	 @param  : in_th			 --深度学习阈值
	 @return : AlgoError
	 @Author : zhoujun@2024/05/28
	 ************************************/
	C_API AlgoError C_CALL DLWaferDieSetThresh(AlgoHandle die_handle, const WaferThresh in_th);


	/************************************
	 @brief  : 设置die相关参数
	 @note   :
	 @param  : die_handle
	 @param  : in_size			--单个die的size
	 @param  : in_threshold		--过曝的灰度阈值
	 @return : AlgoError
	 @Author : zhoujun@2024/05/28
	 ************************************/
	C_API AlgoError C_CALL DLWaferDieSetInfo(AlgoHandle die_handle, AlgoSize*in_size);


	/************************************
	 @brief  : 设置ROI区域
	 @note   :
	 @param  : die_handle
	 @param  : in_wafer_rect       --要裁剪的区域
	 @return : AlgoError
	 @Author : zhoujun@2024/05/28
	 ************************************/
	C_API AlgoError C_CALL DLWaferDieSetROI(AlgoHandle die_handle,const AlgoRect* in_wafer_rect);


	/************************************
	 @brief  : 图像处理
	 @note   :
	 @param  : die_handle
	 @param  : out_die_center    --roi中所有die中心与图像中心的像素偏移量（x=列，y=行）
	 @return : AlgoError
	 @Author : zhoujun@2024/05/28
	 ************************************/
	C_API AlgoError C_CALL DLWaferDieDetect(AlgoHandle die_handle, AlgoMat* in_img, AlgoMat* out_die_center);


	/************************************
	 @brief  : 输出检测结果图
	 @note   :
	 @param  : die_handle
	 @param  : out_img_file       --结果输出位置
	 @return : AlgoError
	 @Author : zhoujun@2024/05/28
	 ************************************/
	C_API AlgoError C_CALL DLWaferDieExportResultPng(AlgoHandle die_handle, const std::string &out_img_file);


	/************************************
	 @brief  : 获取结果bbox的size
	 @note   :
	 @param  : die_handle
	 @param  : out_len            --获取的结果bbox的size
	 @return : AlgoError
	 @Author : zhoujun@2024/05/28
	 ************************************/
	//C_API AlgoError C_CALL DLWaferDieGetBBoxSize(AlgoHandle die_handle, uint64_t *out_len);


	/************************************
	 @brief  : 获取结果
	 @note   :
	 @param  : die_handle
	 @param  : out_bbox           --获取的结果bbox
	 @return : AlgoError
	 @Author : zhoujun@2024/05/28
	 ************************************/
	//C_API AlgoError C_CALL DLWaferDieGetBBox(AlgoHandle die_handle, BBox*out_bbox);


	/************************************
	 @brief  : 获取buffer大小
	 @note   :
	 @param  : die_handle        --当前有效句柄
	 @param  : out_len           --buffer大小
	 @return : AlgoError
	 @Author : zhoujun@2024/05/28
	 ************************************/
	//C_API AlgoError C_CALL DLWaferDieGetBufferSize(AlgoHandle die_handle, uint64_t* out_len);


	/************************************
	 @brief  : 数据导出到buffer
	 @note   :
	 @param  : die_handle
	 @param  : out_buffer        --已经申请好申请空间
	 @param  : in_out_len        --外部申请好的空间大小,会用真实大小修改
	 @return : AlgoError
	 @Author :zhoujun@2024/05/28
	************************************/
	C_API AlgoError C_CALL DLWaferDieToBuffer(AlgoHandle die_handle, char* out_buffer, uint64_t* in_out_len);


	/************************************
	 @brief  : 将buffer数据导入
	 @note   :
	 @param  : die_handle
	 @param  : in_buffer
	 @param  : in_len
	 @return : AlgoError
	 @Author : zhoujun@2024/05/28
	************************************/
	C_API AlgoError C_CALL DLWaferDieFromBuffer(AlgoHandle die_handle, const char* in_buffer, uint64_t in_len);


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
	C_API AlgoError C_CALL DLWaferDieRelease(AlgoHandle die_handle);
	C_API void C_CALL DLWaferDieDestroyHandle(AlgoHandle die_handle);





	//C_API AlgoError C_CALL DLWaferDieGetRes(AlgoHandle die_handle, BBox*bbox);


	//C_API AlgoError C_CALL DLWaferDieDeployDestroyHandle(AlgoHandle die_handle);
	/************************************
	 @brief  : 设置滑窗裁剪相关参数
	 @note   :
	 @param  : die_handle
	 @param  : win_crop       --详见wafer_deployment_data.h定义
	 @return : AlgoError
	 @Author : zhoujun@2024/05/28
	 ************************************/






#ifdef __cplusplus
}
#endif // __cplusplus
#endif