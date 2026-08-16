#ifndef __DL_CHIP_DETECTOR_H__
#define __DL_CHIP_DETECTOR_H__

#include <stdint.h>
#include "ci_declare.h"
#include "ci_define.h"
#include "dl_chip_detector_errors.h"
#include "dl_chip_detector_data.h"
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
	C_API AlgoHandle C_CALL DLChipCreateHandle();


	/************************************
	 @brief  : 加载粗定位模型
	 @note   :
	 @param  : chip_handle
	 @param  : in_model_file		 --onnx模型地址
	 @param  : in_trt_file			 --trt模型地址
	 @param  : in_key				 --密钥
	 @param  : in_engine			 --0：onnxruntime-cpu，1:onnxruntime-gpu，2:tensorrt-gpu。可拓展。
	 @return : AlgoError
	 @Author : zhoujun@2024/09/29
	 ************************************/
	C_API AlgoError C_CALL DLChipLoadLocateModel(AlgoHandle chip_handle, const std::string in_model_file, const std::string in_trt_file, const std::string in_key, int in_engine = 0);


	/************************************
	 @brief  : 加载精定位模型
	 @note   :
	 @param  : chip_handle
	 @param  : in_model_file		 --onnx模型地址
	 @param  : in_trt_file			 --trt模型地址
	 @param  : in_key				 --密钥
	 @param  : in_engine			 --0：onnxruntime-cpu，1:onnxruntime-gpu，2:tensorrt-gpu。可拓展。
	 @return : AlgoError
	 @Author : zhoujun@2024/05/28
	 ************************************/
	C_API AlgoError C_CALL DLChipLoadModel(AlgoHandle chip_handle, const std::string in_model_file, const std::string in_trt_file, const std::string in_key, int in_engine = 0);


	/************************************
	 @brief  : 加载区域剔除检查模型
	 @note   : 该模型为二分类模型，用于检查芯片区域是否被剔除过
	 @param  : chip_handle
	 @param  : in_model_file		 --onnx模型地址
	 @param  : in_trt_file			 --trt模型地址
	 @param  : in_key				 --密钥
	 @param  : in_engine			 --0：onnxruntime-cpu，1:onnxruntime-gpu，2:tensorrt-gpu。可拓展。
	 @return : AlgoError
	 @Author : songshaojun@2025/08/12
	 ************************************/
	C_API AlgoError C_CALL DLChipLoadRegionIntactModel(AlgoHandle chip_handle, const std::string in_model_file, const std::string in_trt_file, const std::string in_key, int in_engine = 0);



	/************************************
	 @brief  : 设置chip相关参数
	 @note   :
	 @param  : chip_handle
	 @param  : in_chip_type          --chip类型 ,R/G/B
	 @param  : in_chip_align         --chip整组排列类型
	 @param  : in_chip_size          --介于焊盘和芯片之间的宽高，如焊盘为250*100，芯片为160*60，可设为200*80
	 @param  : in_chip_distance      --chip相邻焊盘的距离
	 @return : AlgoError
	 @Author : zhoujun@2024/05/28
	 ************************************/
	C_API AlgoError C_CALL DLChipSetInfo(AlgoHandle chip_handle, ChipDetectRGBType in_chip_type, ChipDetectRGBAlignType in_chip_align, const AlgoSize* in_chip_size, const int in_chip_distance);


	/************************************
	 @brief  : 设置深度学习相关参数
	 @note   :
	 @param  : chip_handle
	 @param  : in_th			    --深度学习阈值,在0到1之间
	 @return : AlgoError
	 @Author : zhoujun@2024/05/28
	 ************************************/
	C_API AlgoError C_CALL DLChipSetThresh(AlgoHandle chip_handle, const DLChipThresh in_th);


	/************************************
	 @brief  : 设置裁剪的目标区域
	 @note   :
	 @param  : chip_handle
	 @param  : in_roi				--要裁剪的区域
	 @return : AlgoError
	 @Author : zhoujun@2024/05/28
	 ************************************/
	C_API AlgoError C_CALL DLChipSetROI(AlgoHandle chip_handle,  const AlgoSize *in_roi);


	 /************************************
	  @brief    : 确定chip中心及偏移角度
	  @note     :
	  @param    : chip_handle         --有效的chip句柄
	  @param    : in_image            --需要定位的图像
	  @param    : out_chip_center     --chip中心相对于图像中心的偏移量
	  @param    : out_solder_center   --solder中心相对于图像中心的偏移量
	  @param    : out_chip_angle      --chip的偏移角度，相对于X轴（后续可调整）
	  @return   : AlgoError
	  @Modified : haopanxia@2022/09/30
	 ************************************/
	C_API AlgoError C_CALL DLChipDetect(AlgoHandle chip_handle, AlgoMat* in_img, AlgoPoint2f* out_chip_center, AlgoPoint2f* out_solder_center, float* out_chip_angle);


	  /************************************
	  @brief    : 检查芯片区域是否被剔除过
	  @note     : 若芯片区域被剔除过，输出out_removed_flag = 1，芯片区域没有被剔除过，输出out_removed_flag = 0
	  @param    : chip_handle              --输入量，有效的chip句柄
	  @param    : in_img                   --输入量，需要检测的图像
      @param    : in_chip_solder_center    --输入量，焊盘中心坐标
	  @param    : in_check_roi_size        --输入量，检测区域roi大小
	  @param    : out_removed_flag         --输出量，芯片区域是否被剔除过标识符，若芯片区域被剔除过，out_removed_flag = 1；芯片区域没有被剔除过，输出out_removed_flag = 0
	  @return   : AlgoError
	  @Modified : songshaojun@2025/08/12
	 ************************************/
	C_API AlgoError C_CALL DLChipDetectRegionIntact(AlgoHandle chip_handle, const AlgoMat* in_img, const AlgoPoint2f in_chip_solder_center, const AlgoSize in_check_roi_size, int* out_removed_flag);

	/************************************
	 @brief  : 导出输出结果
	 @note   :
	 @param  : chip_handle
	 @param  : out_img_file			  --输出图像的地址
	 @return : AlgoError
	 @Author : zhoujun@2024/05/28
	 ************************************/
	C_API AlgoError C_CALL DLChipExportResultPng(AlgoHandle chip_handle, const std::string &out_img_file);


	/************************************
	 @brief  : 获取结果的size
	 @note   :
	 @param  : chip_handle
	 @param  : out_len				 --获取的结果bbox的size
	 @return : AlgoError
	 @Author : zhoujun@2024/05/28
	 ************************************/
	//C_API AlgoError C_CALL DLChipGetBBoxSize(AlgoHandle chip_handle, uint64_t *out_len);


	 /************************************
	  @brief  : 获取结果
	  @note   :
	  @param  : chip_handle
	  @param  : in_bbox				  --获取的结果bbox
	  @return : AlgoError
	  @Author : zhoujun@2024/05/28
	  ************************************/
	//C_API AlgoError C_CALL DLChipGetBBox(AlgoHandle chip_handle, DetectionResult*in_bbox);


	 /************************************
	  @brief  : 获取buffer大小
	  @note   :
	  @param  : chip_handle          --当前有效句柄
	  @param  : out_len              --buffer大小
	  @return : AlgoError
	  @Author :zhoujun@2024/05/28
	  ************************************/
	C_API AlgoError C_CALL DLChipGetBufferSize(AlgoHandle chip_handle, uint64_t* out_len);


	 /************************************
	  @brief  : 数据导出到buffer
	  @note   :
	  @param  : chip_handle      
	  @param  : out_buffer         --已经申请好的空间
	  @param  : in_out_len         --外部申请好的空间大小,会用真实大小修改
	  @return : AlgoError
	  @Author : zhoujun@2024/05/28
	 ************************************/
	C_API AlgoError C_CALL DLChipToBuffer(AlgoHandle chip_handle, char* out_buffer, uint64_t* in_len);


	/************************************
	 @brief  : 将buffer数据导入
	 @note   :
	 @param  : chip_handle   
	 @param  : in_buffer
	 @param  : in_len
	 @return : AlgoError
	 @Author : zhoujun@2024/05/28
	************************************/
	C_API AlgoError C_CALL DLChipFromBuffer(AlgoHandle chip_handle, const char* in_buffer, uint64_t in_len);


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
	C_API AlgoError C_CALL DLChipRelease(AlgoHandle chip_handle);
	C_API void C_CALL DLChipDestroyHandle(AlgoHandle chip_handle);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif