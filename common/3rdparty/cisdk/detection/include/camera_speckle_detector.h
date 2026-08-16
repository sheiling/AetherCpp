#ifndef __CAMERA_SPECKLE_DETECTOR_H__
#define __CAMERA_SPECKLE_DETECTOR_H__

#include "ci_declare.h"
#include "ci_data.h"
#include "algo_mat.h"
#include "camera_speckle_detector_data.h"
#include "camera_speckle_detector_error.h"


#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

	/*********************************************
	@brief   : 创建CameraSpeckleDetector句柄
	@note    :
	@param   : detector_handle
	@return  : AlgoError
	@Author  : wushaoshuang@2024/05/27
	*********************************************/
	C_API AlgoError C_CALL CameraSpeckleCreateHandle(AlgoHandle* detector_handle);

	/*********************************************
	@brief   : 设置对比度检测阈值
	@note    : 设置对比度检测阈值，该对比度即亮暗差异度阈值，单位为%，如想检测差异度1%以上的斑点，那么该值设置1.0。内部默认为1%，外部根据需求调整,该值不能小于0。
	@param   : detector_handle         有效的检测句柄
	@param   : in_contrast_thresh      输入的对比度阈值
	@return  : AlgoError
	@Author  : wushaoshuang@2024/05/27
	*********************************************/
	C_API AlgoError C_CALL CameraSpeckleSetContrastThresh(AlgoHandle detector_handle, float* in_contrast_thresh);

	/*********************************************
	@brief   : 设置面积检测阈值
	@note    : 设置面积检测阈值，即亮暗斑面积阈值，多少面积以下的亮暗斑直接忽略，外部根据需求设置，内部默认为3*3=9。
	@param   : detector_handle         有效的检测句柄
	@param   : in_area_thresh          输入的面积阈值
	@return  : AlgoError
	@Author  : wushaoshuang@2024/05/27
	*********************************************/
	C_API AlgoError C_CALL CameraSpeckleSetAreaThresh(AlgoHandle detector_handle, int* in_area_thresh);

	/*********************************************
	@brief   : 设置边缘检测阈值
	@note    : 设置边缘斑点检测阈值，该值为控制图像边缘斑点的检出情况，内部默认为0.3，调节范围为大于0，该值的调节步长为0.1，当图像边缘出现过检时（边缘黑边被检出时），则增大该值，当图像边缘出现漏检时（边缘上的明显斑点未检出时），则减小该值。
	@param   : detector_handle         有效的检测句柄
	@param   : in_edge_thresh          输入的面积阈值
	@return  : AlgoError
	@Author  : wushaoshuang@2026/01/07
	*********************************************/
	C_API AlgoError C_CALL CameraSpeckleSetEdgeThresh(AlgoHandle detector_handle, float* in_edge_thresh);

	/*********************************************
	@brief   : 设置滤波核尺寸
	@note    : 设置滤波核的大小，表示算法兼容目标边缘渐变的程度，内部默认为201，一般无需修改，属于预置参数。
	@param   : detector_handle         有效的检测句柄
	@param   : in_filter_size          输入的滤波核大小
	@return  : AlgoError
	@Author  : wushaoshuang@2024/05/27
	*********************************************/
	C_API AlgoError C_CALL CameraSpeckleSetFilterSize(AlgoHandle detector_handle, int* in_filter_size);

	/*********************************************
	@brief   : 设置参数
	@note    : 设置检测相关参数，详见camera_speckle_detector_data.h中的CameraSpeckleParameter结构体。
	@param   : detector_handle         有效的检测句柄
	@param   : in_parameter            输入的参数结构体，CameraSpeckleParameter结构体
	@return  : AlgoError
	@Author  : wushaoshuang@2024/05/27
	*********************************************/
	C_API AlgoError C_CALL CameraSpeckleSetParameter(AlgoHandle detector_handle, CameraSpeckleParameter* in_parameter);

	/*********************************************
	@brief   : 设置判断标准
	@note    : 设置检测合格的判断标准，即算法内部根据该标准来判断当前相机是否合格。详见camera_speckle_detector_data.h中的CameraJudgeStandard结构体。
	@param   : detector_handle         有效的检测句柄
	@param   : in_standard             输入的参数结构体，CameraJudgeStandard结构体，由于可能存在多个面积下的判断标准，因此需要外部new一个CameraJudgeStandard数组传入
	@param   : in_num                  输入的new的数组大小，表示有几个CameraJudgeStandard标准
	@return  : AlgoError
	@Author  : wushaoshuang@2024/05/27
	*********************************************/
	C_API AlgoError C_CALL CameraSpeckleSetStandard(AlgoHandle detector_handle, CameraJudgeStandard* in_standard, int in_num);

	/*********************************************
	@brief   : 获取对比度检测阈值
	@note    : 获取对比度检测阈值，外部根据需求选择调用
	@param   : detector_handle         有效的检测句柄
	@param   : out_thresh              输出的对比度阈值
	@return  : AlgoError
	@Author  : wushaoshuang@2024/05/27
	*********************************************/
	C_API AlgoError C_CALL CameraSpeckleGetContrastThresh(AlgoHandle detector_handle, float* out_thresh);

	/*********************************************
	@brief   : 获取面积检测阈值
	@note    : 获取面积检测阈值，外部根据需求选择调用
	@param   : detector_handle         有效的检测句柄
	@param   : out_area_thresh         输出的面积阈值
	@return  : AlgoError
	@Author  : wushaoshuang@2024/05/27
	*********************************************/
	C_API AlgoError C_CALL CameraSpeckleGetAreaThresh(AlgoHandle detector_handle, int* out_area_thresh);

	/*********************************************
	@brief   : 获取边缘检测阈值
	@note    : 获取边缘检测阈值，外部根据需求选择调用
	@param   : detector_handle         有效的检测句柄
	@param   : out_edge_thresh         输出的面积阈值
	@return  : AlgoError
	@Author  : wushaoshuang@2026/01/07
	*********************************************/
	C_API AlgoError C_CALL CameraSpeckleGetEdgeThresh(AlgoHandle detector_handle, float* out_edge_thresh);

	/*********************************************
	@brief   : 获取滤波核尺寸
	@note    : 获取滤波核的大小，外部根据需求选择调用
	@param   : detector_handle         有效的检测句柄
	@param   : out_filter_size          输出的滤波核大小
	@return  : AlgoError
	@Author  : wushaoshuang@2024/05/27
	*********************************************/
	C_API AlgoError C_CALL CameraSpeckleGetFilterSize(AlgoHandle detector_handle, int* out_filter_size);

	/*********************************************
	@brief   : 获取参数
	@note    : 获取检测相关参数，外部根据需求选择调用,详见camera_speckle_detector_data.h中的CameraSpeckleParameter结构体。
	@param   : detector_handle         有效的检测句柄
	@param   : out_parameter           输入的参数结构体，CameraSpeckleParameter结构体
	@return  : AlgoError
	@Author  : wushaoshuang@2024/05/27
	*********************************************/
	C_API AlgoError C_CALL CameraSpeckleGetParameter(AlgoHandle detector_handle, CameraSpeckleParameter* out_parameter);

	/*********************************************
	@brief   : 获取判断标准数量
	@note    : 获取检测合格的判断标准数量，供外部new数组时使用。
	@param   : detector_handle         有效的检测句柄
	@param   : out_num                 输出的数量
	@return  : AlgoError
	@Author  : wushaoshuang@2024/05/27
	*********************************************/
	C_API AlgoError C_CALL CameraSpeckleGetStandardNum(AlgoHandle detector_handle, int* out_num);

	/*********************************************
	@brief   : 获取判断标准
	@note    : 获取检测合格的判断标准，外部根据需求选择调用。详见camera_speckle_detector_data.h中的CameraJudgeStandard结构体。
	@param   : detector_handle         有效的检测句柄
	@param   : out_standard            输出的标准结构体，需要外部new一个CameraJudgeStandard结构体数组，大小与setStandard时设置的数组大小一致
	@return  : AlgoError
	@Author  : wushaoshuang@2024/05/27
	*********************************************/
	C_API AlgoError C_CALL CameraSpeckleGetStandard(AlgoHandle detector_handle, CameraJudgeStandard* out_standard);

	/*********************************************
	@brief   : 斑点检测主接口
	@note    : 斑点检测主要接口，该接口直接输出是否合格。
	@param   : detector_handle         有效的检测句柄
	@param   : in_image                输入的待检测图像
	@param   : out_result              输出的检测结果，一个bool类型的结果，True表示合格，False表示不合格
	@return  : AlgoError
	@Author  : wushaoshuang@2024/05/27
	*********************************************/
	C_API AlgoError C_CALL CameraSpeckleDetect(AlgoHandle detector_handle, AlgoMat* in_image, bool* out_result);

	/************************************
	@brief    : 获取指定RECT区域的图像模板
	@note     :
	@param    : in_algomat            输入的图像
	@param    : in_rect               目标rect， 包含起点坐标 x y 和区域宽高 with height
	@param    : in_rect_num           目标rect的数量
	@param    : out_algomat_temp      输出的ROI区域图像
	@return	  : AlgoError
	@Modified :	lipengpeng@2024/09/23
	************************************/
	C_API AlgoError C_CALL CameraSpeckleGetManualShieldTemp(AlgoMat * in_algomat, AlgoRect* in_rect, int in_rect_num, AlgoMat * out_algomat_temp);

	/************************************
	@brief    : 自动获取指定RECT区域的图像模板
	@note     : 首样生成
	@param    : detector_handle       有效的检测句柄
	@param    : in_expand_radio       目标区域大小扩充比例
	@param    : out_algomat_temp      输出的ROI区域图像
	@return	  : AlgoError
	@Modified :	lipengpeng@2024/10/09
	************************************/
	C_API AlgoError C_CALL CameraSpeckleGetAutoShieldTemp(AlgoHandle detector_handle, float in_expand_radio, AlgoMat * out_algomat_temp);

	/************************************
	@brief   : 爆亮爆暗检测结果筛选导出
	@note    :
	@param   : detector_handle         有效的检测句柄
	@param   : in_mask_tmpl            模版mask图像
	@param   : in_png_path             检测结果图像存储路径
	@return	 : AlgoError
	@Author  : lipengpeng@2024/09/11
	************************************/
	C_API AlgoError C_CALL CameraSpeckleDetectImgExport(AlgoHandle detector_handle, const AlgoMat* in_mask_tmpl, const char* in_png_path);

	/*********************************************
	@brief   : 获取斑点数量
	@note    : 获取检测的斑点数量，外部根据需求调用
	@param   : detector_handle         有效的检测句柄
	@param   : out_num                 输出斑点的数量
	@return  : AlgoError
	@Author  : wushaoshuang@2024/05/27
	*********************************************/
	C_API AlgoError C_CALL CameraSpeckleGetSpeckleNum(AlgoHandle detector_handle, int* out_num);

	/*********************************************
	@brief   : 获取斑点特征
	@note    : 获取斑点特征，即检测的斑点的结果，包括斑点轮廓，面积、差异度等，外部根据需求调用,使用前先使用CameraSpeckleGetSpeckleNum接口获取数量后，new一个CameraSpeckleFeature数组
	@param   : detector_handle         有效的检测句柄
	@param   : out_speckle_feature     输出斑点特征数据，需要外部new一个CameraSpeckleFeature数组
	@return  : AlgoError
	@Author  : wushaoshuang@2024/05/27
	*********************************************/
	C_API AlgoError C_CALL CameraSpeckleGetFeature(AlgoHandle detector_handle, CameraSpeckleFeature* out_speckle_feature);

	/*********************************************
	@brief   : 获取判断结果
	@note    : 获取判断结果，即判断的结果情况，包括各面积区间下A类有多少，B类有多少，外部根据需求调用，调用前必须先调用CameraSpeckleDetect接口
	@param   : detector_handle         有效的检测句柄
	@param   : out_judge_result        输出斑点特征数据，需要外部new一个AlgoPoint数组，数组大小为面积区间数量，即设置判断标准时new的CameraJudgeStandard数组大小。每个AlgoPoint保存的数据为A类与B类的不合格数量，AlgoPoint的x值对应A类数量，y值对应B类数量
	@return  : AlgoError
	@Author  : wushaoshuang@2024/05/27
	*********************************************/
	C_API AlgoError C_CALL CameraSpeckleGetJudgeResult(AlgoHandle detector_handle, AlgoPoint* out_judge_result);

	/*********************************************
	@brief   : 导出检测结果图像
	@note    : 将检测结果绘制在原始图像及增强图像上导出，该部分图像仅供观测参考，外部根据需求调用
	@param   : detector_handle         有效的检测句柄
	@param   : in_png_path             输入的图像存储路径
	@return  : AlgoError
	@Author  : wushaoshuang@2024/05/27
	*********************************************/
	C_API AlgoError C_CALL CameraSpeckleExportPng(AlgoHandle detector_handle, const char* in_png_path);

	/*********************************************
	@brief   : 获取buffer大小
	@note    :
	@param   : detector_handle         有效的检测句柄
	@param   : out_len                 buffer大小
	@return  : AlgoError
	@Author  : wushaoshuang@2024/05/27
	*********************************************/
	C_API AlgoError C_CALL CameraSpeckleGetBufferSize(AlgoHandle detector_handle, uint64_t* out_len);

	/*********************************************
	@brief   : 将数据导入buffer
	@note    :
	@param   : detector_handle         有效的检测句柄
	@param   : out_buffer              输出检测参数的buffer空间
	@param   : in_out_len              buffer空间大小
	@return  : AlgoError
	@Author  : wushaoshuang@2024/05/27
	*********************************************/
	C_API AlgoError C_CALL CameraSpeckleToBuffer(AlgoHandle detector_handle, char* out_buffer, uint64_t* in_out_len);

	/*********************************************
	@brief   : 将buffer数据读取到CameraSpeckle
	@note    :
	@param   : detector_handle         有效的检测句柄
	@param   : in_buffer               buffer
	@param   : in_len                  buffer大小
	@return  : AlgoError
	@Author  : wushaoshuang@2024/05/27
	*********************************************/
	C_API AlgoError C_CALL CameraSpeckleFromBuffer(AlgoHandle detector_handle, const char* in_buffer, uint64_t in_len);

	/*********************************************
	@brief   : 载入CameraSpeckle文件
	@note    :
	@param   : detector_handle         有效的检测句柄
	@param   : in_load_path            输入的文件路径，后缀必须为.speckle
	@return  : AlgoError
	@Author  : wushaoshuang@2024/05/27
	*********************************************/
	C_API AlgoError C_CALL CameraSpeckleLoad(AlgoHandle detector_handle, const char* in_load_path);

	/*********************************************
	@brief   : 保存CameraSpeckle文件
	@note    :
	@param   : detector_handle         有效的检测句柄
	@param   : in_save_path            输入要保存的文件路径，后缀必须为.speckle
	@return  : AlgoError
	@Author  : wushaoshuang@2024/05/27
	*********************************************/
	C_API AlgoError C_CALL CameraSpeckleSave(AlgoHandle detector_handle, const char* in_save_path);

	/*********************************************
	@brief   : 空间内存释放
	@note    : 句柄指向的对象销毁，此句柄已经无效，请配合handle=nullptr
	@param   : detector_handle         有效的检测句柄
	@return  :
	@Author  : wushaoshuang@2024/05/27
	*********************************************/
	C_API void C_CALL CameraSpeckleDestroyHandle(AlgoHandle detector_handle);

	/*********************************************
	@brief   : 句柄指向的对象的内存空间释放，句柄指向的对象还在
	@note    : 内存释放
	@param   : detector_handle         有效的检测句柄
	@return  : AlgoError
	@Author  : wushaoshuang@2024/05/27
	*********************************************/
	C_API AlgoError C_CALL CameraSpeckleRelease(AlgoHandle detector_handle);


#ifdef __cplusplus
}
#endif
#endif