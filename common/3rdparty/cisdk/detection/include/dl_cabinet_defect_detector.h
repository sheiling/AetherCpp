#ifndef ____CABINET_DEFECT_DETECTOR_H__
#define ____CABINET_DEFECT_DETECTOR_H__

#include "ci_declare.h"
#include "ci_define.h"
#include "ci_data.h"
#include "dl_cabinet_defect_detector_data.h"
#include "dl_cabinet_defect_detector_errors.h"
#include "algo_mat.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


	/************************************
	@brief  : 创建检测句柄
	@note   :
	@return : AlgoHandle
	@author : songshaojun@2024/12/19
	************************************/
	C_API AlgoHandle C_CALL DLCabinetDefectCreateHandle();


	/************************************
	 @brief  : 加载模型
	 @note   :
	 @param  : detect_handle         输入量，有效的检测句柄
	 @param  : in_model_file		 输入量，onnx模型地址
	 @param  : in_trt_file			 输入量，trt模型地址
	 @param  : in_key				 输入量，密钥
	 @param  : in_model_type		 输入量，载入的模型类型，详情请参考dl_cabinet_defect_detector_data.h中DLCabinetDefectModelType
	 @param  : in_engine			 输入量，使用的推理引擎，其中，0：onnxruntime-cpu，1:onnxruntime-gpu，2:tensorrt-gpu。
	 @return : AlgoError
	 @author : songshaojun@2024/12/19
	 ************************************/
	C_API AlgoError C_CALL DLCabinetDefectLoadModel(AlgoHandle detect_handle, const char* in_model_file, const char* in_trt_file, const char* in_key, const DLCabinetDefectModelType& in_model_type, int in_engine);


	/************************************
	 @brief  : 设置深度学习相关阈值
	 @note   :
	 @param  : detect_handle         输入量，有效的检测句柄
	 @param  : in_locate_dl_th		 输入量，定位模型用到的深度学习阈值，详情请参考dl_cabinet_defect_detector_data.h中DLCabinetDefectDLThresh
	 @param  : in_segment_dl_th		 输入量，分割模型用到的深度学习阈值，详情请参考dl_cabinet_defect_detector_data.h中DLCabinetDefectDLThresh	 
	 @return : AlgoError
	 @author : songshaojun@2024/12/19
	 ************************************/
	C_API AlgoError C_CALL  DLCabinetDefectSetDLThresh(AlgoHandle detect_handle, const DLCabinetDefectDLThresh in_locate_dl_th, const DLCabinetDefectDLThresh in_segment_dl_th);
	

	/************************************
	 @brief  : 设置算法检测相关参数
	 @note   : 
	 @param  : detect_handle         输入量，有效的检测句柄
	 @param  : in_th				 输入量，算法检测相关阈值，详情请参考dl_cabinet_defect_detector_data.h中DLCabinetDefectDetectThresh
	 @return : AlgoError
	 @author : songshaojun@2024/12/19
	 ************************************/
	C_API AlgoError C_CALL  DLCabinetDefectSetMergeThresh(AlgoHandle detect_handle, const DLCabinetDefectMergeThresh in_th);


	/************************************
	 @brief  : 设置图像重叠数量
	 @note   : 重叠分为宽度方向上重叠和高度方向上重叠，单位为像素
	 @param  : detect_handle         输入量，有效的检测句柄
	 @param  : in_overlap_size		 输入量，重复拍摄时，每次的重叠像素数
	 @return : AlgoError
	 @author : songshaojun@2024/12/19
	 ************************************/
	C_API AlgoError C_CALL  DLCabinetDefectSetPixelOverlapNum(AlgoHandle detect_handle, const AlgoSize in_overlap_size);



	/************************************
	@brief   : 检测箱体缺陷
	@note    : 具体检测哪种类型缺陷，根据标志位确定，一次可检测多种类型缺陷
			   注意：该接口不再支持检测划痕，划痕检测请使用新街口DLCabinetDefectDetectScratches
	@param   : detect_handle         输入量，有效的检测句柄
	@param   : in_image              输入量，需要检测的图像
	@param   : in_roi_rect           输入量，roi
	@param   : in_ignored_roi_list   输入量，ignoredROI，不期望参与检测的区域list
	@param   : in_ignored_roi_num    输入量，不期望参与检测的区域的数量
	@param   : in_diet_size          输入量，期望检出大小，小于该尺寸不检出
	@param   : in_detect_type        输入量，需要检测的缺陷类型，详情请参考dl_cabinet_defect_detector_data.h中DLCabinetDefectDetectType
	@param   : out_result            输出量，检测结果
	@return  : AlgoError
	@author  : songshaojun@2025/01/10
	************************************/
	C_API AlgoError C_CALL DLCabinetDefectDetect(AlgoHandle detect_handle, const AlgoMat* in_image, const DLCabinetDefectDetectType* in_detect_list, AlgoRect in_roi_rect, 
		AlgoRect* in_ignored_roi_list, const int in_ignored_roi_num, const AlgoSize in_diet_size, const int in_defect_num, AlgoMat* out_result);


	/************************************
	@brief   : 检测箱体划痕缺陷
	@note    : 该接口只检测"SCRATCHES"这一种缺陷
	@param   : detect_handle         输入量，有效的检测句柄
	@param   : in_image              输入量，需要检测的图像
	@param   : in_roi_rect           输入量，roi
	@param   : in_ignored_roi_list   输入量，ignoredROI，不期望参与检测的区域list
	@param   : in_ignored_roi_num    输入量，不期望参与检测的区域的数量
	@param   : in_diet_size          输入量，期望检出大小，小于该尺寸不检出
	@param   : out_bbox_result       输出量，返回的划痕bbox结果
	@param   : out_mask_result       输出量，返回的划痕mask结果
	@return  : AlgoError
	@author  : songshaojun@2025/01/10
	************************************/
	C_API AlgoError C_CALL DLCabinetDefectDetectScratches(AlgoHandle detect_handle, const AlgoMat* in_image, AlgoRect in_roi_rect, 
		AlgoRect* in_ignored_roi_list, const int in_ignored_roi_num, const AlgoSize in_diet_size,AlgoMat* out_bbox_result, AlgoMat* out_mask_result);
	//C_API AlgoError C_CALL DLCabinetDefectDetectMask(AlgoHandle detect_handle, const AlgoMat* in_image, AlgoMat* out_bbox_result, AlgoMat* out_mask_result);

	

	/************************************
	@brief   : 拼接缺陷结果
	@note    : 拼接其他缺陷+划痕缺陷
	@param   : in_image1               输入量，待合并图像1
	@param   : in_image2               输入量，待合并图像2
	@param   : in_flag                 输入量，标志位，1=垂直，2=水平
	@param   : out_stitch_result       输出量，返回的合并结果
	@return  : AlgoError
	@author  : songshaojun@2025/01/10
	************************************/
	C_API AlgoError C_CALL DLCabinetDefectStitchMat(const AlgoMat* in_image1, const AlgoMat* in_image2,const int in_flag, AlgoMat* out_stitch_result);

	/************************************
	@brief   : 合并已检测到的所有缺陷
	@note    : 与DLCabinetDefectMergeSameTypeResult接口配合使用
	@param   : detect_handle         输入量，有效的检测句柄
	@param   : in_result_list        输入量，需要合并的结果的list
	@param   : in_result_nums        输入量，有几个需要合并的结果，对应于list的大小
	@param   : out_img               输出量，合并后的结果
	@return  : AlgoError
	@author  : songshaojun@2024/12/19
	************************************/
	C_API AlgoError C_CALL DLCabinetDefectMergeAllTypeResult(AlgoHandle detect_handle, const AlgoMat* in_result_list, const int in_result_num, AlgoMat* out_img);


	/************************************
	@brief   : 拼接图像
	@note    : 用于拼接多次采集的图像,或拼接检测结果图像
	@param   : detect_handle         输入量，有效的检测句柄
	@param   : in_img_order          输入量，图像索引信息，详情请参考dl_cabinet_defect_detector_data.h中DLCabinetDefectMergeInfo
	@param   : in_img_list           输入量，需要合并的图像的list
	@param   : in_img_nums           输入量，有几个需要合并的图像，对应于list的大小
	@param   : zoom_sacle            输入量，缩放尺寸,zoom_sacle<1代表缩小图像，zoom_sacle>1代表放大图像，=1代表原尺寸
	@param   : out_img               输出量，拼接后的图像
	@return  : AlgoError
	@author  : songshaojun@2024/12/19
	************************************/
	C_API AlgoError C_CALL DLCabinetDefectStitchImage(AlgoHandle detect_handle, const DLCabinetDefectMergeInfo& in_img_order, const AlgoMat* in_img_list, const int in_img_num, const float in_zoom_sacle, AlgoMat* out_img);




	/************************************
	@brief   : 在拼接好的大图上，绘制NG目标出现的大致区域
	@note    : 
	@param   : detect_handle                        输入量，有效的检测句柄
	@param   : in_img_order						    输入量，图像索引信息，详情请参考dl_cabinet_defect_detector_data.h中DLCabinetDefectMergeInfo，与DLCabinetDefectStitchImage接口中in_img_order一致
	@param   : in_out_stitched_img                  输入输出量，已经调用DLCabinetDefectStitchImage接口拼接好的大图,即DLCabinetDefectStitchImage接口吐出的out_img变量。NG区域将直接绘制在该图像上
	@param   : in_zoom_sacle                        输入量，在调用DLCabinetDefectStitchImage接口绘制拼接大图时的缩放尺寸，与DLCabinetDefectStitchImage接口中in_zoom_sacle保持一致
	@param   : in_merged_result                     输入量，在调用DLCabinetDefectMergeAllTypeResult接口后吐出的所有合并结果，即：DLCabinetDefectMergeAllTypeResult接口中的out_img变量
	@param   : in_stitched_image_00point_mech       输入量，in_out_stitched_img图像左下角(0,0)点的机械坐标，即拼接之前
	@param   : in_pixel_size					    输入量, 像元尺寸
	@return  : AlgoError
	@author  : songshaojun@2025/04/11
	************************************/
	C_API AlgoError C_CALL DLCabinetDefectDrawNGImg(AlgoHandle detect_handle, const DLCabinetDefectMergeInfo& in_img_order, AlgoMat* in_out_stitched_img, const float in_zoom_sacle, const AlgoMat* in_merged_result,const AlgoPoint2f in_stitched_image_00point_mech, const float in_pixel_size);



	/************************************
	 @brief  : 导出当前检测结果，导出结果为落盘图像
	 @note   : 1、若送入的图像及其结果为单次拍摄的，则可用于导出单张样本的可视化检测结果；
			   2、若送入的图像及其结果为拼接后的，则可用于导出拼接后的所有结果的可视化结果；
			   注意：若送入的图像结果为拼接后的，请确保结果与图像都进行了相对应尺寸的平移和缩放操作；
	 @param  : detect_handle         输入量，有效的检测句柄
	 @param  : in_img				 输入量，需要绘制导出的样本
	 @param  : in_result			 输入量，需要绘制导出的样本的结果
	 @param  : out_img_root			 输入量，结果的存储路径
	 @return : AlgoError
	 @author : songshaojun@2025/01/10
	 ************************************/
	C_API AlgoError C_CALL DLCabinetDefectExportResultPng(AlgoHandle detect_handle, const AlgoMat* in_img, const AlgoMat* in_result, const char* out_img_root);


	/************************************
	 @brief  : 导出当前检测结果，导出结果为Mat形式
	 @note   : 1、若送入的图像及其结果为单次拍摄的，则可用于导出单张样本的可视化检测结果；
			   2、若送入的图像及其结果为拼接后的，则可用于导出拼接后的所有结果的可视化结果；
			   注意：若送入的图像结果为拼接后的，请确保结果与图像都进行了相对应尺寸的平移和缩放操作；
	 @param  : detect_handle         输入量，有效的检测句柄
	 @param  : in_img				 输入量，需要绘制导出的样本
	 @param  : in_result			 输入量，需要绘制导出的样本的结果
	 @param  : out_img			     输入量，用于接收结果的mat，导出的结果存储在该mat中
	 @return : AlgoError
	 @author : songshaojun@2025/01/15
	 ************************************/
	C_API AlgoError C_CALL DLCabinetDefectExportResultMat(AlgoHandle detect_handle, const AlgoMat* in_img, const AlgoMat* in_result, AlgoMat* out_img);


	/************************************
	 @brief  : 导出当前检测结果，导出结果为落盘图像
	 @note   : 1、若送入的图像及其结果为单次拍摄的，则可用于导出单张样本的可视化检测结果；
			   2、若送入的图像及其结果为拼接后的，则可用于导出拼接后的所有结果的可视化结果；
			   注意：若送入的图像结果为拼接后的，请确保结果与图像都进行了相对应尺寸的平移和缩放操作；
	 @param  : detect_handle         输入量，有效的检测句柄
	 @param  : in_img				 输入量，需要绘制导出的样本
	 @param  : in_bbox_result		 输入量，需要绘制导出的样本的bbox结果
	 @param  : in_mask_result		 输入量，需要绘制导出的样本的mask结果
	 @param  : out_img_root			 输入量，结果的存储路径
	 @param  : transparency			 输入量，绘制的mask结果透明度，范围[0,1],1代表不透明，0代表完全透明。该参数默认值为0.2f
	 @return : AlgoError
	 @author : songshaojun@2025/01/10
	 ************************************/
	C_API AlgoError C_CALL DLCabinetDefectExportSegmentResultPng(AlgoHandle detect_handle, const AlgoMat* in_img, const AlgoMat* in_bbox_result, const AlgoMat* in_mask_result, const char* out_img_root, const float transparency = 0.2f);


	/************************************
	 @brief  : 导出当前检测结果，导出结果为Mat形式
	 @note   : 1、若送入的图像及其结果为单次拍摄的，则可用于导出单张样本的可视化检测结果；
			   2、若送入的图像及其结果为拼接后的，则可用于导出拼接后的所有结果的可视化结果；
			   注意：若送入的图像结果为拼接后的，请确保结果与图像都进行了相对应尺寸的平移和缩放操作；
	 @param  : detect_handle         输入量，有效的检测句柄
	 @param  : in_img				 输入量，需要绘制导出的样本
	 @param  : in_bbox_result		 输入量，需要绘制导出的样本的bbox结果
	 @param  : in_mask_result		 输入量，需要绘制导出的样本的mask结果
	 @param  : out_img			     输入量，用于接收结果的mat，导出的结果存储在该mat中
	 @param  : transparency			 输入量，绘制的mask结果透明度，范围[0,1],1代表不透明，0代表完全透明。该参数默认值为0.2f
	 @return : AlgoError
	 @author : songshaojun@2025/01/15
	 ************************************/
	C_API AlgoError C_CALL DLCabinetDefectExportSegmentResultMat(AlgoHandle detect_handle, const AlgoMat* in_img, const AlgoMat* in_bbox_result, const AlgoMat* in_mask_result, AlgoMat* out_img, const float transparency = 0.2f);



	/************************************
	 @brief  : 保存和载入
	 @note   : 内部只存储了阈值参数
	 @param  : detect_handle              输入量，有效的检测句柄
	 @param	 : in_cabinetdefect_fullname, 输入量, 文件名后缀必须是.cabinetdefect
	 @return : AlgoError
	 @author : songshaojun@2025/02/13
	 ************************************/
	C_API AlgoError C_CALL DLCabinetDefectSave(AlgoHandle detect_handle,  const char* in_cabinetdefect_fullname);
	C_API AlgoError C_CALL DLCabinetDefectLoad(AlgoHandle detect_handle,  const char* in_cabinetdefect_fullname);


	/************************************
    @brief  : 获取buffer大小
    @note   :
    @param  : detect_handle, 输入量 有效的排列句柄
    @param  : out_len, 输出量, buffer大小
    @return	: AlgoError
	 @author : songshaojun@2025/02/13
    ************************************/
    C_API AlgoError C_CALL DLCabinetDefectGetBufferSize(AlgoHandle detect_handle, uint64_t* out_len);

	/************************************
	@brief  : 将ledpoint里面的数据导出到buffer
	@note   :
	@param  : detect_handle   有效的detect_handle句柄
	@param  : out_buffer        已经申请好申请空间
	@param  : in_out_len        外部申请好的空间大小,会用真实大小修改
	@return	: AlgoError
	 @author : songshaojun@2025/02/13
	************************************/
	C_API AlgoError C_CALL DLCabinetDefectToBuffer(AlgoHandle detect_handle, char* out_buffer, uint64_t* in_out_len);

	/************************************
	@brief  : 将buffer数据导入到ledpoint
	@note   :
	@param  : detect_handle   有效的detect_handle句柄
	@param  : in_buffer
	@param  : in_len
	@return	: AlgoError
	 @author : songshaojun@2025/02/13
	************************************/
	C_API AlgoError C_CALL DLCabinetDefectFromBuffer(AlgoHandle detect_handle, const char* in_buffer, uint64_t in_len);


	C_API AlgoError C_CALL DLCabinetDefectRelease(AlgoHandle detect_handle);
	C_API void C_CALL DLCabinetDefectDestroyHandle(AlgoHandle detect_handle);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif