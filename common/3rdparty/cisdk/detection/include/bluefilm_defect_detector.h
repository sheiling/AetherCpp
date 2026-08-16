#ifndef __BLUEFILM_DEFECT_DETECTOR_H__
#define __BLUEFILM_DEFECT_DETECTOR_H__

#include "ci_declare.h"
#include "ci_data.h"
#include "algo_mat.h"
#include "bluefilm_defect_detector_data.h"
#include "bluefilm_defect_detector_error.h"

#ifdef __cplusplus
extern "C" 
{
#endif //__cplusplus

	/*********************************************
	@brief   : 创建BluefilmDefectDetector句柄
	@note    : 
	@param   : detector_handle
	@return  : AlgoError
	@Author  : wushaoshuang@2024/07/30
	*********************************************/
	C_API AlgoError C_CALL BluefilmDefectCreateHandle(AlgoHandle* detector_handle);

	/*********************************************
	@brief   : 设置检测对比度阈值
	@note    : 设置检测对比度阈值，该阈值表示目标与背景亮度的差异度阈值，单位为%，如想检测差异度10%以上的斑点，那么该值设置10.0。内部默认为10%，外部根据需求调整,该值不能小于0。
	@param   : detector_handle         有效的检测句柄
	@param   : in_contrast_thresh      输入的对比度阈值
	@return  : AlgoError
	@Author  : wushaoshuang@2024/07/30
	*********************************************/
	C_API AlgoError C_CALL BluefilmDefectSetContrastThresh(AlgoHandle detector_handle, const float* in_contrast_thresh);

	/*********************************************
	@brief   : 设置面积检测阈值
	@note    : 设置面积检测阈值，即脏污面积阈值，多少面积以下的脏污直接忽略，外部根据需求设置，内部默认为5*5=25。
	@param   : detector_handle         有效的检测句柄
	@param   : in_area_thresh          输入的面积阈值
	@return  : AlgoError
	@Author  : wushaoshuang@2024/07/30
	*********************************************/
	C_API AlgoError C_CALL BluefilmDefectSetAreaThresh(AlgoHandle detector_handle, const int* in_area_thresh);

	/*********************************************
	@brief   : 设置滤波核尺寸
	@note    : 设置滤波核的大小，表示算法兼容目标边缘渐变的程度，内部默认为101，图像大小为1280*1024时一般无需修改，根据图像大小或检测ROI大小按照比例调整。
	@param   : detector_handle         有效的检测句柄
	@param   : in_filter_size          输入的滤波核大小
	@return  : AlgoError
	@Author  : wushaoshuang@2024/07/30
	*********************************************/
	C_API AlgoError C_CALL BluefilmDefectSetFilterSize(AlgoHandle detector_handle, const int* in_filter_size);

	/*********************************************
	@brief   : 设置检测参数
	@note    : 设置检测相关参数，详见bluefilm_defect_detector_data.h中的BluefilmDefectParameter结构体。
	@param   : detector_handle         有效的检测句柄
	@param   : in_parameter            输入的参数结构体，BluefilmDefectParameter结构体
	@return  : AlgoError
	@Author  : wushaoshuang@2024/07/30
	*********************************************/
	C_API AlgoError C_CALL BluefilmDefectSetParameter(AlgoHandle detector_handle, const BluefilmDefectParameter* in_parameter);

	/*********************************************
	@brief   : 获取对比度检测阈值
	@note    : 获取对比度检测阈值，外部根据需求选择调用
	@param   : detector_handle         有效的检测句柄
	@param   : out_thresh              输出的对比度阈值
	@return  : AlgoError
	@Author  : wushaoshuang@2024/07/30
	*********************************************/
	C_API AlgoError C_CALL BluefilmDefectGetContrastThresh(AlgoHandle detector_handle, float* out_thresh);

	/*********************************************
	@brief   : 获取面积检测阈值
	@note    : 获取面积检测阈值，外部根据需求选择调用
	@param   : detector_handle         有效的检测句柄
	@param   : out_area_thresh         输出的面积阈值
	@return  : AlgoError
	@Author  : wushaoshuang@2024/07/30
	*********************************************/
	C_API AlgoError C_CALL BluefilmDefectGetAreaThresh(AlgoHandle detector_handle, int* out_area_thresh);

	/*********************************************
	@brief   : 获取滤波核尺寸
	@note    : 获取滤波核的大小，外部根据需求选择调用
	@param   : detector_handle         有效的检测句柄
	@param   : out_filter_size         输出的滤波核大小
	@return  : AlgoError
	@Author  : wushaoshuang@2024/07/30
	*********************************************/
	C_API AlgoError C_CALL BluefilmDefectGetFilterSize(AlgoHandle detector_handle, int* out_filter_size);

	/*********************************************
	@brief   : 获取检测参数
	@note    : 获取检测相关参数，外部根据需求选择调用,详见bluefilm_defect_detector_data.h中的BluefilmDefectParameter结构体。
	@param   : detector_handle         有效的检测句柄
	@param   : out_parameter           输入的参数结构体，BluefilmDefectParameter结构体
	@return  : AlgoError
	@Author  : wushaoshuang@2024/07/30
	*********************************************/
	C_API AlgoError C_CALL BluefilmDefectGetParameter(AlgoHandle detector_handle, BluefilmDefectParameter* out_parameter);

	/*********************************************
	@brief   : 蓝膜脏污检测主接口
	@note    : 蓝膜脏污检测主要接口，检测结果通过接口BluefilmDefectGetResult获取。
	@param   : detector_handle         有效的检测句柄
	@param   : in_image                输入的待检测图像
	@param   : in_roi_rect             输入的待检测图像区域，默认无，即整幅图像进行检测，如果设置，则内部仅对该区域内的脏污进行检测。外部根据实际情况设置
	@param   : out_defect_num          输出的脏污数量
	@return  : AlgoError
	@Author  : wushaoshuang@2024/07/30
	*********************************************/
	C_API AlgoError C_CALL BluefilmDefectDetect(AlgoHandle detector_handle, AlgoMat* in_image, int* out_defect_num, AlgoRect* in_roi_rect = nullptr);

	/*********************************************
	@brief   : 获取脏污检测结果
	@note    : 获取脏污检测结果，即检测的脏污特征，包括脏污轮廓，面积、差异度、轮廓质心等，详见bluefilm_defect_detector_data.h中的BluefilmDefectFeature结构体。使用前需要根据脏污数量new一个BluefilmDefectFeature数组
	@param   : detector_handle         有效的检测句柄
	@param   : out_defect_feature      输出脏污特征数据，需要外部new一个BluefilmDefectFeature数组
	@return  : AlgoError
	@Author  : wushaoshuang@2024/07/30
	*********************************************/
	C_API AlgoError C_CALL BluefilmDefectGetResult(AlgoHandle detector_handle, BluefilmDefectFeature* out_defect_feature);

	/*********************************************
	@brief   : 导出检测结果图像
	@note    : 将检测结果绘制在原始图像上导出，该部分图像仅供观测参考，外部根据需求调用
	@param   : detector_handle         有效的检测句柄
	@param   : in_png_path             输入的图像存储路径
	@return  : AlgoError
	@Author  : wushaoshuang@2024/07/30
	*********************************************/
	C_API AlgoError C_CALL BluefilmDefectExportPng(AlgoHandle detector_handle, const char* in_png_path);

	/*********************************************
	@brief   : 获取buffer大小
	@note    :
	@param   : detector_handle         有效的检测句柄
	@param   : out_len                 buffer大小
	@return  : AlgoError
	@Author  : wushaoshuang@2024/07/30
	*********************************************/
	C_API AlgoError C_CALL BluefilmDefectGetBufferSize(AlgoHandle detector_handle, uint64_t* out_len);

	/*********************************************
	@brief   : 将数据导入buffer
	@note    :
	@param   : detector_handle         有效的检测句柄
	@param   : out_buffer              输出检测参数的buffer空间
	@param   : in_out_len              buffer空间大小
	@return  : AlgoError
	@Author  : wushaoshuang@2024/07/30
	*********************************************/
	C_API AlgoError C_CALL BluefilmDefectToBuffer(AlgoHandle detector_handle, char* out_buffer, uint64_t* in_out_len);

	/*********************************************
	@brief   : 将buffer数据读取到BluefilmDefect
	@note    :
	@param   : detector_handle         有效的检测句柄
	@param   : in_buffer               buffer
	@param   : in_len                  buffer大小
	@return  : AlgoError
	@Author  : wushaoshuang@2024/07/30
	*********************************************/
	C_API AlgoError C_CALL BluefilmDefectFromBuffer(AlgoHandle detector_handle, const char* in_buffer, uint64_t in_len);

	/*********************************************
	@brief   : 载入BluefilmDefect文件
	@note    :
	@param   : detector_handle         有效的检测句柄
	@param   : in_load_path            输入的文件路径，后缀必须为.bluefilm
	@return  : AlgoError
	@Author  : wushaoshuang@2024/07/30
	*********************************************/
	C_API AlgoError C_CALL BluefilmDefectLoad(AlgoHandle detector_handle, const char* in_load_path);

	/*********************************************
	@brief   : 保存BluefilmDefect文件
	@note    :
	@param   : detector_handle         有效的检测句柄
	@param   : in_save_path            输入要保存的文件路径，后缀必须为.bluefilm
	@return  : AlgoError
	@Author  : wushaoshuang@2024/07/30
	*********************************************/
	C_API AlgoError C_CALL BluefilmDefectSave(AlgoHandle detector_handle, const char* in_save_path);



/****************************************晶粒放置位置预规划相关接口**************************************/
	/*********************************************
	@brief   : 设置芯片放置参数
	@note    : 设置芯片放置参数，根据实际情况设置，详见bluefilm_defect_detect_data.h
	@param   : detector_handle         有效的检测句柄
	@param   : in_place_param          输入的芯片放置参数，详见bluefilm_defect_detect_data.h中结构体DiePlaceParameter
	@return  : AlgoError
	@Author  : wushaoshuang@2024/08/16
	*********************************************/
	C_API AlgoError C_CALL DiePlaceSetParam(AlgoHandle detector_handle, DiePlaceParameter* in_place_param);

	/*********************************************
	@brief   : 芯片放置位置计算主接口
	@note    : 芯片放置位置计算主接口，输出为一系列的芯片预放置位置，按照摆放顺序输出
	@param   : detector_handle         有效的检测句柄
	@param   : in_place_roi            输入的芯片预放置ROI
	@param   : out_place_point         输出的芯片预放置坐标,以1行n列的2通道的AlgoMat数据保存，n表示有n个输出坐标，第一个通道表示坐标的x值，第二个通道表示坐标的y值
	@return  : AlgoError
	@Author  : wushaoshuang@2024/08/16
	*********************************************/
	C_API AlgoError C_CALL DiePlacePlan(AlgoHandle detector_handle, AlgoRect* in_place_roi, AlgoMat* out_place_point);

	/*********************************************
	@brief   : 导出map示意图（外部选择性调用）
	@note    : 导出map示意图，用于人眼观测芯片总体规划放置的形状，仅供外部参考
	@param   : detector_handle         有效的检测句柄
	@param   : in_png_path             输入的图像存储路径
	@return  : AlgoError
	@Author  : wushaoshuang@2024/08/16
	*********************************************/
	C_API AlgoError C_CALL DiePlaceExportPng(AlgoHandle detector_handle, const char* in_png_path);

	/*********************************************
	@brief   : 获取芯片放置参数（外部选择性调用）
	@note    : 获取芯片放置参数
	@param   : detector_handle         有效的检测句柄
	@param   : out_place_param         输出的芯片放置参数，详见bluefilm_defect_detect_data.h中结构体DiePlaceParameter
	@return  : AlgoError
	@Author  : wushaoshuang@2024/08/16
	*********************************************/
	C_API AlgoError C_CALL DiePlaceGetParam(AlgoHandle detector_handle, DiePlaceParameter* out_place_param);

	/*********************************************
	@brief   : 获取芯片放置规划buffer大小
	@note    :
	@param   : detector_handle         有效的检测句柄
	@param   : out_len                 buffer大小
	@return  : AlgoError
	@Author  : wushaoshuang@2024/08/16
	*********************************************/
	C_API AlgoError C_CALL DiePlaceGetBufferSize(AlgoHandle detector_handle, uint64_t* out_len);

	/*********************************************
	@brief   : 将芯片放置数据导入buffer
	@note    :
	@param   : detector_handle         有效的检测句柄
	@param   : out_buffer              输出检测参数的buffer空间
	@param   : in_out_len              buffer空间大小
	@return  : AlgoError
	@Author  : wushaoshuang@2024/08/16
	*********************************************/
	C_API AlgoError C_CALL DiePlaceToBuffer(AlgoHandle detector_handle, char* out_buffer, uint64_t* in_out_len);

	/*********************************************
	@brief   : 将芯片放置buffer数据读取到BluefilmDefect
	@note    :
	@param   : detector_handle         有效的检测句柄
	@param   : in_buffer               buffer
	@param   : in_len                  buffer大小
	@return  : AlgoError
	@Author  : wushaoshuang@2024/08/16
	*********************************************/
	C_API AlgoError C_CALL DiePlaceFromBuffer(AlgoHandle detector_handle, const char* in_buffer, uint64_t in_len);

	/*********************************************
	@brief   : 载入DiePlace文件
	@note    :
	@param   : detector_handle         有效的检测句柄
	@param   : in_load_path            输入的文件路径，后缀必须为.dieplace
	@return  : AlgoError
	@Author  : wushaoshuang@2024/08/16
	*********************************************/
	C_API AlgoError C_CALL DiePlaceLoad(AlgoHandle detector_handle, const char* in_load_path);

	/*********************************************
	@brief   : 保存DiePlace文件
	@note    :
	@param   : detector_handle         有效的检测句柄
	@param   : in_save_path            输入要保存的文件路径，后缀必须为.dieplace
	@return  : AlgoError
	@Author  : wushaoshuang@2024/08/16
	*********************************************/
	C_API AlgoError C_CALL DiePlaceSave(AlgoHandle detector_handle, const char* in_save_path);


	/*********************************************
	@brief   : 空间内存释放
	@note    : 句柄指向的对象销毁，此句柄已经无效，请配合handle=nullptr
	@param   : detector_handle         有效的检测句柄
	@return  :
	@Author  : wushaoshuang@2024/07/30
	*********************************************/
	C_API void C_CALL BluefilmDefectDestroyHandle(AlgoHandle detector_handle);

	/*********************************************
	@brief   : 句柄指向的对象的内存空间释放，句柄指向的对象还在
	@note    : 内存释放
	@param   : detector_handle         有效的检测句柄
	@return  : AlgoError
	@Author  : wushaoshuang@2024/07/30
	*********************************************/
	C_API AlgoError C_CALL BluefilmDefectRelease(AlgoHandle detector_handle);


#ifdef __cplusplus
}
#endif
#endif