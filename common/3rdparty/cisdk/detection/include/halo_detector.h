#ifndef ___HALO_DETECTOR_H__
#define ___HALO_DETECTOR_H__

#include "ci_declare.h"
#include "ci_define.h"
#include "ci_data.h"
#include "halo_detector_errors.h"
#include "halo_detector_data.h"
#include "algo_mat.h"


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	/************************************
   @brief    : 创建光型检测句柄
   @note     :
   @return   : AlgoHandle
   @Modified : mumeng@2024/10/12
   ************************************/
	C_API AlgoHandle C_CALL HaloDetectCreateHandle();


	/************************************
	@brief    : 设置模型并行检测数量
	@param    : halo_detector_handle   有效的halo检测句柄
	@param    : in_thread_num		   一次检测数量,  常用 4
	@note     :
	@return   : AlgoError
	@Modified : mumeng@2025/06/25
	************************************/
	C_API AlgoError C_CALL HaloDetectSetThreadNum(AlgoHandle halo_detector_handle, const int in_thread_num);


	/************************************
	@brief    : 设置所有灯条的检测ROI区域
	@param    : halo_detector_handle   有效的halo检测句柄
	@param    : in_detect_rect		   检测区域rect
	@note     :
	@return   : AlgoError
	@Modified : mumeng@2024/08/23
	************************************/
	C_API AlgoError C_CALL HaloDetectSetRoi(AlgoHandle halo_detector_handle, const AlgoRect* in_detect_rect);


	/************************************
	@brief    : 制作模板
	@param    : halo_detector_handle   有效的halo检测句柄
	@param    : in_cam_img			   输入的当前相机采集的图像
	@param    : in_loc_template_rect   输入的检测区域rect
	@param    : in_template_flag	   模板名称 flag
	@param    : out_template_amat	   输出的模板amat图像
	@note     :
	@return   : AlgoError
	@Modified : mumeng@2025/11/3
	************************************/
	C_API AlgoError C_CALL HaloDetectMakeTemplate(AlgoHandle halo_detector_handle, AlgoMat* in_cam_img, 
		const AlgoRect* in_loc_template_rect, TemplateName in_template_flag, AlgoMat* out_template_amat);



	/************************************
	@brief    : 设置模板
	@param    : halo_detector_handle   有效的halo检测句柄
	@param    : in_template_img 	   输入的Algomat定位模板图像
	@param    : in_template_flag	   模板名称 flag
	@note     :
	@return   : AlgoError
	@Modified : mumeng@2025/11/3
	************************************/
	C_API AlgoError C_CALL HaloDetectSetTemplate(AlgoHandle halo_detector_handle, AlgoMat* in_template_img, TemplateName in_template_flag);


	/************************************
	@brief    : 获取模板
	@param    : halo_detector_handle   有效的halo检测句柄
	@param    : in_template_flag	   模板名称 flag
	@param    : out_locate_img	       输出的模板Algomat图像
	@note     :
	@return   : AlgoError
	@Modified : mumeng@2025/11/3
	************************************/
	C_API AlgoError C_CALL HaloDetectGetTemplate(AlgoHandle halo_detector_handle, TemplateName in_template_flag, AlgoMat* out_locate_img);



	/************************************
	@brief    : 设置灯条信息
	@param    : halo_detector_handle   有效的halo检测句柄
	@param    : in_light_type		   灯珠的类型 REFLECT——反射式灯珠  REFRACT——折射式灯珠
	@param    : in_light_array		   灯珠干扰点的排列方式
	@note     :
	@return   : AlgoError
	@Modified : mumeng@2025/11/6
	************************************/
	C_API AlgoError C_CALL HaloDetectSetLightInfo(AlgoHandle halo_detector_handle, const LightType in_light_type, const LightArray in_light_array);



	/************************************
	@brief    : 获取灯条信息
	@param    : halo_detector_handle   有效的halo检测句柄
	@param    : out_light_info		   输出的灯条信息结构体
	@note     :
	@return   : AlgoError
	@Modified : mumeng@2024/10/12
	************************************/
	C_API AlgoError C_CALL HaloDetectGetLightInfo(AlgoHandle halo_detector_handle, LightInfo* out_light_info);


	/************************************
	@brief    : 设置检测阈值
	@param    : halo_detector_handle   有效的halo检测句柄
	@param    : in_detect_th		   检测阈值，包括locate_thresh--定位阈值 die_light_th--死灯检测阈值 unity_thresh--偏位/浮高阈值   scrath_thresh--刮花阈值  blurred_thresh--雾化阈值
	@note     :
	@return   : AlgoError
	@Modified : mumeng@2024/10/12
	************************************/
	C_API AlgoError C_CALL HaloDetectSetDetectThresh(AlgoHandle halo_detector_handle, const DetectThresh in_detect_th);


	/************************************
	@brief    : 获取检测阈值
	@param    : halo_detector_handle   有效的halo检测句柄
	@param    : out_detect_th          输出的检测阈值
	@note     :
	@return   : AlgoError
	@Modified : mumeng@2024/10/12
	************************************/
	C_API AlgoError C_CALL HaloDetectGetDetectThresh(AlgoHandle halo_detector_handle, DetectThresh* out_detect_th);

	/************************************
	@brief    : 设备开始检测前死灯预判断
	@param    : halo_detector_handle   有效的halo检测句柄
	@param    : in_cam_mat		       相机拍摄图像
	@param    : in_halo_idx			   灯条序号索引
	@param    : in_idx_len		       序号索引长度
	@param    : in_die_th		       死等检测阈值  0-1  float
	@param    : out_die_label_mat	   输出的死灯检测标签矩阵
	@note     :
	@return   : AlgoError
	@Modified : mumeng@2025/8/11
	************************************/
	C_API AlgoError C_CALL HaloDetectPreJudgeDie(AlgoHandle halo_detector_handle, AlgoMat* in_cam_mat,
		int* in_halo_idx, int in_idx_len, const float in_die_th, AlgoMat* out_die_label_mat);



	/************************************
	@brief    : 检测死等暗灯
	@param    : halo_detector_handle   有效的halo检测句柄
	@param    : in_labelret_mat	   灯条的检测结果标签矩阵
	@param    : in_drawret_mat	   检测结果图像
	@param    : in_die_th		   死灯判断阈值
	@param    : out_labelret_mat   输出的结果标签矩阵
	@param    : out_drawret_mat	   输出的检测结果图像
	@note     :
	@return   : AlgoError
	@Modified : mumeng@2025/6/14
	************************************/
	C_API AlgoError C_CALL HaloDetectDetectDieLight(AlgoHandle halo_detector_handle, AlgoMat* in_labelret_mat, AlgoMat* in_drawret_mat, float in_die_th,
		AlgoMat* out_labelret_mat, AlgoMat* out_drawret_mat);


	/************************************
	@brief    : 雾化检测
	@param    : halo_detector_handle   有效的halo检测句柄
	@param    : in_labelret_mat		   输入标签结果矩阵
	@param    : in_drawret_mat		   输入画缺陷的map图
	@param    : out_labelret_mat	   输出检测更新后的标签结果矩阵
	@param    : out_drawret_mat		   输出检测更新后缺陷map图像
	@note     :
	@return   : AlgoError
	@Modified : mumeng@2025/10/24
	************************************/
	C_API AlgoError C_CALL HaloDetectDetectBlurred(AlgoHandle halo_detector_handle, AlgoMat* in_labelret_mat, AlgoMat* in_drawret_mat,
		AlgoMat* out_labelret_mat, AlgoMat* out_drawret_mat);



	/************************************
	@brief    : 检测刮花
	@param    : halo_detector_handle   有效的halo检测句柄
	@param    : out_detect_mat		   输出只有刮花的检测结果矩阵
	@note     :
	@return   : AlgoError
	@Modified : mumeng@2025/1/2
	************************************/
	C_API AlgoError C_CALL HaloDetectDetectScrath(AlgoHandle halo_detector_handle, AlgoMat* out_detect_mat);



	/************************************
	@brief    : 载入深度学习模型——深度学习检测
	@param    : halo_detector_handle   有效的halo检测句柄
	@param    : in_model_file	      onnx格式的深度学习训练模型文件路径
	@param    : in_trt_file	          trt格式的深度学习训练模型文件路径
	@param    : in_key	              深度学习文件的密钥
	@param    : in_engine	          推理引起的falg,  0：onnxruntime-cpu，1:onnxruntime-gpu，2:tensorrt-gpu
	@param    : in_model_type	      当前模型文件功能的falg，包含：LOACTION_MODEL；TWO_CALSS_MODEL；MULTI_CLASS_MODEL,
	@param    : in_model_name	      当前模型是什么模型，包含：YOLO 和 ANOMLIB,  默认入参是YOLO
	@note     :
	@return   : AlgoError
	@Modified : mumeng@2025/6/20
	************************************/
	C_API AlgoError C_CALL HaloDetectLoadModel(AlgoHandle halo_detector_handle, const char* in_model_file, const char* in_trt_file,
		const char* in_key, int in_engine, ModelType in_model_type, ModelName in_model_name = YOLO);



	/************************************
	@brief    : 加载SVM模型
	@param    : halo_detector_handle  有效的halo检测句柄
	@param    : in_svm_model_file	  SVM模型的路径， 文件名为 .xml
	@param    : in_scaler_file	      SVM数据标准化参数， 文件名为 .yml
	@note     :
	@return   : AlgoError
	@Modified : mumeng@2025/12/22
	************************************/
	C_API AlgoError C_CALL HaloDetectLoadSVMModel(AlgoHandle halo_detector_handle, const char* in_svm_model_file, const char* in_scaler_file);


	/************************************
	@brief    : 设置置信度阈值——深度学习检测
	@param    : halo_detector_handle   有效的halo检测句柄
	@param    : in_confidence_th	   置信度阈值，locate_th--定位置信度，two_class_th--二分类置信度，multi_class_th多分类置信度
	@note     :
	@return   : AlgoError
	@Modified : mumeng@2024/12/29
	************************************/
	C_API AlgoError C_CALL HaloDetectSetConfidenceThresh(AlgoHandle halo_detector_handle, const DetectConfidenceThresh in_confidence_th);



	/************************************
	@brief    : 获取置信度阈值——深度学习检测
	@param    : halo_detector_handle   有效的halo检测句柄
	@param    : in_confidence_th	   置信度阈值，locate_th--定位置信度，two_class_th--二分类置信度，multi_class_th多分类置信度
	@note     :
	@return   : AlgoError
	@Modified : mumeng@2024/1/10
	************************************/
	C_API AlgoError C_CALL HaloDetectGetConfidenceThresh(AlgoHandle halo_detector_handle, DetectConfidenceThresh* out_confidence_th);



	/************************************
	@brief    : 使用深度学习的方法进行灯珠定位
	@param    : halo_detector_handle			有效的halo检测句柄
	@param    : in_img_scr						待检测的图像
	@param    : out_halo_point_mat	            定位坐标矩阵
	@param    : out_loc_draw_img	            画有定位结果的图像
	@note     :
	@return   : AlgoError
	@Modified : mumeng@2025/6/13
	************************************/
	C_API AlgoError C_CALL HaloDetectModelLocation(AlgoHandle halo_detector_handle, AlgoMat* in_img_src, AlgoMat* out_halo_point_mat, AlgoMat* out_loc_draw_img);



	/************************************
	@brief    : 对定位后的位置进行校准
	@param    : halo_detector_handle			有效的halo检测句柄
	@param    : in_img_scr						待检测的图像
	@param    : in_halo_point_mat	            校准前定位坐标矩阵
	@param    : in_correct_roi_list	            需要校准的区域坐标rect
	@param    : list_len						rect列表长度
	@param    : out_correct_point_mat			输出校准后的坐标点矩阵
	@param    : out_correct_img					输出校准定位图像
	@note     :
	@return   : AlgoError
	@Modified : mumeng@2025/6/13
	************************************/
	C_API AlgoError C_CALL HaloDetectCorrectPoint(AlgoHandle halo_detector_handle, AlgoMat* in_img_src, AlgoMat* in_halo_point_mat, AlgoRect* in_correct_roi_list, int list_len,
		AlgoMat* out_correct_point_mat, AlgoMat* out_correct_img);


	/************************************
	@brief    : 对灯点进行排布
	@param    : halo_detector_handle			有效的halo检测句柄
	@param    : in_point_mat					未排布的灯点坐标
	@param    : in_rows_num						灯珠的行数
	@param    : in_cols_num						灯珠的列数
	@param    : out_point_map					排布后的灯点坐标，矩阵的第几行列，对应灯珠行列，（像素坐标）
	@note     :
	@return   : AlgoError
	@Modified : mumeng@2025/11/6
	************************************/
	C_API AlgoError C_CALL  HaloDetectPointMapping(AlgoHandle halo_detector_handle, AlgoMat* in_point_mat, 
		const int in_rows_num, const int in_cols_num, AlgoMat* out_point_map);


	/************************************
	@brief    : 合并定位结果矩阵
	@param    : halo_detector_handle			有效的halo检测句柄
	@param    : in_img_relult_list				放有多张图像定位结果矩阵列表
	@param    : in_list_lens					矩阵列表长度
	@param    : in_merge_method					矩阵拼接的方法
	@param    : out_merge_result				输出拼接后的结果矩阵
	@note     :
	@return   : AlgoError
	@Modified : mumeng@2025/6/14
	************************************/
	C_API AlgoError C_CALL HaloDetectMergePointMap(AlgoHandle halo_detector_handle, AlgoMat*  in_img_pointmap_list, const int in_list_lens,
		MergeMethod in_merge_method, AlgoMat* out_merge_result);



	/************************************
	@brief    : 设置定位结果map
	@param    : halo_detector_handle			有效的halo检测句柄
	@param    : in_point_map					输入的灯珠定位结果map
	@note     :
	@return   : AlgoError
	@Modified : mumeng@2025/6/14
	************************************/
	C_API AlgoError C_CALL HaloDetectSetPonitMap(AlgoHandle halo_detector_handle, AlgoMat* in_point_map);




	/************************************
	@brief    : 获取定位结果map
	@param    : halo_detector_handle			有效的halo检测句柄
	@param    : out_point_map					输入的灯珠定位结果map
	@note     :
	@return   : AlgoError
	@Modified : mumeng@2025/6/14
	************************************/
	C_API AlgoError C_CALL HaloDetectGetPonitMap(AlgoHandle halo_detector_handle, AlgoMat* out_point_map);



	/************************************
	@brief    : 计算图像质量评价标准
	@param    : halo_detector_handle			有效的halo检测句柄
	@param    : in_img_src						待评价光型图像
	@param    : in_id_list						当前点亮图像行数列表
	@param    : in_list_len						行数列表长度
	@param    : out_score_mat					输出光型的质量评价分数
	@note     :
	@return   : AlgoError
	@Modified : mumeng@2025/7/01
	************************************/
	C_API AlgoError C_CALL HaloDetectCalcImageScore(AlgoHandle halo_detector_handle, AlgoMat* in_img_src, int* in_id_list, int in_list_len,
		AlgoMat* out_score_mat, AlgoMat* out_score_img);




	/************************************
	@brief    : 当前图像缺陷检测——深度学习检测
	@param    : halo_detector_handle   有效的halo检测句柄
	@param    : in_img_src			   相机拍摄的图像
	@param    : in_id_list			   点亮的灯条的序号，右下角为原点，序号从 1 开始。
	@param    : in_id_list			   灯条序号list的长度
	@note     :
	@return   : AlgoError
	@Modified : mumeng@2025/6/14
	************************************/
	C_API AlgoError C_CALL HaloModelDefectDetect(AlgoHandle halo_detector_handle, AlgoMat* in_img_src, int* in_id_list, int in_list_len);


	/************************************
	@brief    : 输入一个光型图检测——深度学习检测,内部自用接口，软件端不调用
	@param    : halo_detector_handle   有效的halo检测句柄
	@param    : in_img_src			   单个光型图像
	@param    : out_ret_img			   输出的画有缺陷结果的图
	@param    : out_contours_img	   二分类复判的轮廓图像图像
	@param    : out_ret_label		   检测结果标签
	@note     :
	@return   : AlgoError
	@Modified : mumeng@2025/07/29
	************************************/
	C_API AlgoError C_CALL HaloModelDefectOneHaloImg(AlgoHandle halo_detector_handle, AlgoMat* in_img_src,
		AlgoMat* out_ret_img, AlgoMat* out_contours_img, int* out_ret_label);



	/************************************
	@brief    : 获取检测结果矩阵
	@param    : halo_detector_handle   有效的halo检测句柄
	@param    : out_labelret_mat	   标签结果矩阵   ① 矩阵的行号和列号是对应灯珠的行列号；②矩阵有三通道  C1--x;C2--y;C3--缺陷类型 C4--光型灰度值
	@param    : out_drawret_img	       画有检测结果图像
	@note     :
	@return   : AlgoError
	@Modified : mumeng@2025/6/15
	************************************/
	C_API AlgoError C_CALL HaloDetectGetResult(AlgoHandle halo_detector_handle, AlgoMat* out_labelret_mat, AlgoMat* out_drawret_img);



	/************************************
	@brief    : 拼接不同相机的检测结果矩阵
	@param    : halo_detector_handle		有效的halo检测句柄
	@param    : in_detect_relult_mat_list   需要拼接的结果矩阵列表
	@param    : in_list_lens				矩阵列表的长度
	@param    : in_merge_method				结果拼接的方法，MERGE_HORZCAT--水平串联拼接， MERGE_VERTCAT--垂直串联拼接
	@param    : out_merge_result_mat		输出拼接结果矩阵
	@note     :
	@return   : AlgoError
	@Modified : mumeng@2025/6/14
	************************************/
	C_API AlgoError C_CALL HaloDetectMergeResult(AlgoHandle halo_detector_handle, AlgoMat* in_detect_relult_mat_list, const int in_list_lens,
		const MergeMethod in_merge_method, AlgoMat* out_merge_result_mat);



	/************************************
	@brief    : 结果对其，将灯条的检测结果向右对其
	@param    : halo_detector_handle		有效的halo检测句柄
	@param    : in_detect_relult_mat_list   需要拼接的结果矩阵列表
	@param    : in_list_lens				矩阵列表的长度
	@param    : in_merge_method				结果拼接的方法，MERGE_HORZCAT--水平串联拼接， MERGE_VERTCAT--垂直串联拼接
	@param    : out_merge_result_mat		输出拼接结果矩阵
	@note     :
	@return   : AlgoError
	@Modified : mumeng@2025/6/14
	************************************/
	C_API AlgoError C_CALL  HaloDetectAlignResult(AlgoHandle halo_detector_handle, AlgoMat* in_out_label_mat, AlgoMat* in_out_drawret_img,
		AlgoMat* out_label_mat, AlgoMat* out_drawret_img);


	/************************************
	@brief    : 从相机拍摄的图像中，扣除光型图像
	@param    : halo_detector_handle			有效的halo检测句柄
	@param    : in_cam_img						相机拍摄的原图
	@param    : in_id_list						灯条编号索引
	@param    : in_list_len					    灯条索引的长度
	@param    : out_halo_img_list				输出的光型图像列表
	@param    : in_halo_num						输入有多少个光型图像
	@note     :
	@return   : AlgoError
	@Modified : mumeng@2025/7/23
	************************************/
	C_API AlgoError C_CALL  HaloDetectGetHaloImg(AlgoHandle halo_detector_handle, AlgoMat* in_cam_img, 
		int* in_id_list, int in_list_len, AlgoMat* out_halo_img_list, int in_halo_num);




	/************************************
	@brief    : 对检测结果图像进行切割
	@param    : halo_detector_handle		有效的halo检测句柄
	@param    : in_drawret_img		        输入的大的检测结果图像
	@param    : in_block_num		        输入需要切分几块
	@param    : out_cut_img_list			输出的图像块列表
	@note     :
	@return   : AlgoError
	@Modified : mumeng@2025/6/16
	************************************/
	C_API AlgoError C_CALL  HaloDetectSegmentResult(AlgoHandle halo_detector_handle, AlgoMat* in_drawret_img, const int in_block_num, AlgoMat* out_cut_img_list);


	/************************************
	@brief    : 获取buffer大小
	@note     :
	@param    : halo_detector_handle   有效的halo句柄
	@param    : out_len                buffer大小
	@return   : AlgoError
	@Modified : mumeng@024/10/17
	************************************/
	C_API AlgoError C_CALL HaloDetectGetBufferSize(AlgoHandle halo_detector_handle, uint64_t* out_len);


	/************************************
	@brief    : 将HaloDetect里面的数据导出到buffer
	@note     :
	@param    : halo_detector_handle         有效的halo句柄
	@param    : out_buffer					 已经申请好申请空间
	@param    : in_out_len					 外部申请好的空间大小,会用真实大小修改
	@return   : AlgoError
	@Modified : mumeng@024/10/17
	************************************/
	C_API AlgoError C_CALL HaloDetectToBuffer(AlgoHandle halo_detector_handle, char* out_buffer, uint64_t* in_out_len);


	/************************************
	@brief    : 将buffer数据导入到HaloDetect
	@note     :
	@param    : halo_detector_handle         有效的halo句柄
	@param    : in_buffer
	@param    : in_len
	@return   : AlgoError
	@Modified :mumeng@024/10/17
	************************************/
	C_API AlgoError C_CALL HaloDetectFromBuffer(AlgoHandle halo_detector_handle, const char* in_buffer, uint64_t in_len);


	/*********************************************
	@brief   : 保存HaloDetect文件
	@note    :
	@param   : halo_detector_handle				有效的halo句柄
	@param   : in_save_path					    输入的文件路径，后缀必须为.hdf
	@return  : AlgoError
	@Author  : mumeng@024/10/17
	*********************************************/
	C_API AlgoError C_CALL HaloDetectSave(AlgoHandle halo_detector_handle, const char* in_save_path);


	/*********************************************
	@brief   : 载入HaloDetect文件
	@note    :
	@param   : halo_detector_handle				有效的halo句柄
	@param   : in_load_path						输入的文件路径，后缀必须为.hdf
	@return  : AlgoError
	@Author  : mumeng@024/10/17
	*********************************************/
	C_API AlgoError C_CALL HaloDetectLoad(AlgoHandle halo_detector_handle, const char* in_load_path);


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
	C_API AlgoError C_CALL HaloDetectRelease(AlgoHandle halo_detector_handle);
	C_API void C_CALL HaloDetectDestroyHandle(AlgoHandle halo_detector_handle);



	//C_API AlgoError C_CALL cvMatToAlgoMatTest(AlgoMat* out_mat);




#ifdef __cplusplus
}
#endif // __cplusplus
#endif