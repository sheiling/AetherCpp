#ifndef __LUMCHROMA_DETECTOR_H__
#define __LUMCHROMA_DETECTOR_H__

#include "ci_data.h"
#include "ci_declare.h"
#include "meta_info_data.h"
#include "lumchroma_detector_data.h"
#include "lumchroma_detector_error.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

	/************************************
	@brief  : 创建module_detect_handle句柄
	@note   :
	@param  : lum_chroma_detect_handle
	@return : AlgoError
	@Author : lipengpeng@2025/04/22
	************************************/
	C_API AlgoError C_CALL LumChromaDetectCreateHandle(AlgoHandle* lum_chroma_detect_handle);

	/************************************
	@brief  : 亮暗点检测
	@note   : 板内检测：检测LED模组内过亮或过暗的灯点:RGBW图
	@param  : lum_chroma_detect_handle         有效的句柄,不能为空
	@param  : in_color_meter_handle            输入光通量提取结果句柄,点定位后
	@param  : in_num                           输入光通量数量
	@return : AlgoError
	@Author : lipengpeng@2025/04/22
	************************************/
	C_API AlgoError C_CALL LumChromaDetectBrightAndDarkPoint(AlgoHandle lum_chroma_detect_handle, const AlgoHandle * in_color_meter_handle, const int in_num);

	/************************************
	@brief  : 亮暗块检测
	@note   : 板内检测：检测LED模组内过亮或过暗的块:RGBW图
	@param  : lum_chroma_detect_handle         效的句柄,不能为空
	@param  : in_color_meter_handle            输入光通量提取结果句柄,点定位后
	@param  : in_num                           输入光通量数量
	@return : AlgoError
	@Author : lipengpeng@2025/04/22
	************************************/
	C_API AlgoError C_CALL LumChromaDetectBrightAndDarkBlock(AlgoHandle lum_chroma_detect_handle, const AlgoHandle *in_color_meter_handle, const int in_num);

	/************************************
	@brief  : 亮色度表征计算---板间
	@note   : LED模组整体亮度色度表征
	@param  : lum_chroma_detect_handle         有效的句柄,不能为空
	@param  : in_color_meter_handle            输入有效的光通量句柄
	@param  : in_num                           输入光通量数量
	@param  : in_colorspace                    亮色度表征量颜色空间
	@param  : out_lumchroma_feature            输出计算得到中心亮色度表征值
	@return	: AlgoError
	@Author : lipengpeng@2025/04/22
	************************************/
	C_API AlgoError C_CALL  LumChromaDetectGetAreaLumChroma(AlgoHandle lum_chroma_detect_handle, const AlgoHandle* in_color_meter_handle, const int in_num,
		LumChromaDetectColorSpace in_colorspace, LumChromaDetectFeature* out_lumchroma_feature);

	/************************************
	@brief  : 检测基准计算---板间
	@note   : 标准模组样本若干，自动计算亮度色度标准值
	@param  : lum_chroma_detect_handle        有效的句柄,不能为空
	@param  : in_lumchroma_feature            输入亮色度表征值
	@param  : in_num                          输入样本数据数量
	@param  : out_lumchroma_standard          输出亮色度检测基准值
	@return	: AlgoError
	@Author : lipengpeng@2025/04/22
	************************************/
	C_API AlgoError C_CALL LumChromaDetectCalcStd(AlgoHandle lum_chroma_detect_handle, LumChromaDetectFeature* in_lumchroma_feature, const int in_num,
		LumChromaDetectFeature* out_lumchroma_standard);

	/************************************
	@brief  : 亮色度检测---板间
	@note   : 板间检测：检测板间亮度色度差异过大的LED模组
	@param  : lum_chroma_detect_handle        有效的句柄,不能为空
	@param  : in_lumchroma_feature            输入亮色度表征值
	@param  : in_lumchroma_standard           输入亮色度检测基准值
	@param  : in_color                        检测画面：RGBW
	@param  : out_lumchroma_diff              输出亮色度检测偏差：可通过检测挡位Check
	@param  : out_result_label                输出检测结果：0：代表OK；1：代表NG
	@return	: AlgoError
	@Author : lipengpeng@2025/04/22
	************************************/
	C_API AlgoError C_CALL LumChromaDetectConsistency(AlgoHandle lum_chroma_detect_handle, LumChromaDetectFeature* in_lumchroma_feature, LumChromaDetectFeature* in_lumchroma_standard,
		AlgoColor in_color, LumChromaDetectDiff *out_lumchroma_diff, int* out_result_label);

	/************************************
	@brief  : 亮色度检测---板内
	@note   : 板内检测：检测板内亮度色度差异过大的LED模组
	@param  : lum_chroma_detect_handle        有效的句柄,不能为空
	@param  : in_color_meter_handle           输入有效的光通量句柄
	@param  : in_num                          输入光通量数量
	@param  : in_colorspace                   亮色度表征量颜色空间
	@param  : in_areas                        检测区域：目前支支持分五区和九区
	@param  : in_color                        检测画面：RGBW
	@param  : out_inner_lumchroma_diff        输出板内亮色度检测偏差
	@param  : out_result_label                输出检测结果：0：代表OK；1：代表NG
	@return	: AlgoError
	@Author : lipengpeng@2025/08/04
	************************************/
	C_API AlgoError C_CALL LumChromaDetectInnerConsistency(AlgoHandle lum_chroma_detect_handle, const AlgoHandle* in_color_meter_handle, const int in_num,
		LumChromaDetectColorSpace in_colorspace, AlgoColor in_color, LumChromaDetectAreas in_areas, LumChromaDetectDiff *out_inner_lumchroma_diff, int* out_result_label);

	/************************************
	@brief  : 检测阈值设定
	@note   : 板内亮暗点
	@param  : lum_chroma_detect_handle        有效的句柄,不能为空
	@param  : in_lumchroma_point_thresh       输入板内亮暗点检测阈值
	@return : AlgoError
	@Author : lipengpeng@2025/04/22
	************************************/
	C_API AlgoError C_CALL LumChromaDetectPointSetThresh(AlgoHandle lum_chroma_detect_handle, const LumChromaDetectPointThresh* in_lumchroma_point_thresh);

	/************************************
	@brief  : 检测阈值获取
	@note   : 板内亮暗点
	@param  : lum_chroma_detect_handle        有效的句柄,不能为空
	@param  : out_lumchroma_point_thresh      输出板内亮暗点检测阈值
	@return : AlgoError
	@Author : lipengpeng@2025/04/22
	************************************/
	C_API AlgoError C_CALL LumChromaDetectPointGetThresh(AlgoHandle lum_chroma_detect_handle, LumChromaDetectPointThresh* out_lumchroma_point_thresh);

	/************************************
	@brief  : 检测阈值设定
	@note   : 板内亮暗块
	@param  : lum_chroma_detect_handle        有效的句柄,不能为空
	@param  : in_lumchroma_block_thresh       输入板内亮暗块检测阈值
	@return : AlgoError
	@Author : lipengpeng@2025/04/22
	************************************/
	C_API AlgoError C_CALL LumChromaDetectBlockSetThresh(AlgoHandle lum_chroma_detect_handle, const LumChromaDetectBlockThresh* in_lumchroma_block_thresh);

	/************************************
	@brief  : 检测阈值获取
	@note   : 板内亮暗块
	@param  : lum_chroma_detect_handle        有效的句柄,不能为空
	@param  : out_lumchroma_block_thresh      输出板内亮暗块检测阈值
	@return : AlgoError
	@Author : lipengpeng@2025/04/22
	************************************/
	C_API AlgoError C_CALL LumChromaDetectBlockGetThresh(AlgoHandle lum_chroma_detect_handle, LumChromaDetectBlockThresh* out_lumchroma_block_thresh);

	/************************************
	@brief  : 检测阈值设定
	@note   : 板间亮色度差异
	@param  : lum_chroma_detect_handle        有效的句柄,不能为空
	@param  : in_lumchroma_consis_thresh      输入板间检测阈值
	@return : AlgoError
	@Author : lipengpeng@2025/04/22
	************************************/
	C_API AlgoError C_CALL LumChromaDetectConsisSetThresh(AlgoHandle lum_chroma_detect_handle, const LumChromaDetectConsisThresh* in_lumchroma_consis_thresh);

	/************************************
	@brief  : 检测阈值获取
	@note   : 板间亮色度差异
	@param  : lum_chroma_detect_handle        有效的句柄,不能为空
	@param  : out_lumchroma_consis_thresh     输出板间检测阈值
	@return : AlgoError
	@Author : lipengpeng@2025/04/22
	************************************/
	C_API AlgoError C_CALL LumChromaDetectConsisGetThresh(AlgoHandle lum_chroma_detect_handle, LumChromaDetectConsisThresh* out_lumchroma_consis_thresh);

	/************************************
	@brief  : 检测标准设定
	@note   :
	@param  : lum_chroma_detect_handle        有效的句柄,不能为空
	@param  : in_lumchroma_standard           输入检测标准值
	@return : AlgoError
	@Author : lipengpeng@2025/04/22
	************************************/
	C_API AlgoError C_CALL LumChromaDetectSetStandard(AlgoHandle lum_chroma_detect_handle, LumChromaDetectFeature* in_lumchroma_standard);

	/************************************
	@brief  : 检测标准获取
	@note   :
	@param  : lum_chroma_detect_handle        有效的句柄,不能为空
	@param  : out_lumchroma_standard          获取检测标准值
	@return : AlgoError
	@Author : lipengpeng@2025/04/22
	************************************/
	C_API AlgoError C_CALL LumChromaDetectGetStandard(AlgoHandle lum_chroma_detect_handle, LumChromaDetectFeature* out_lumchroma_standard);

	/************************************
	@brief  : 图像检测结果Mat数据转List
	@note   :
	@param  : lum_chroma_detect_handle        有效的句柄,不能为空
	@param  : in_filter_defect_type           输入想要的缺陷标识符：示例1.比如输入亮块标识符，则获取亮块缺陷的结果
	@param  : out_defect_loc_list             输出缺陷灯点位置List
	@param  : out_defect_num                  输出缺陷灯点数量
	@param  : in_ignore_defect_type           输入不想要的缺陷标识符,默认值LUM_CHROMA_DETECT_FLAG_UNINIT：示例2.比如输入暗点标识符，则既是暗点又是亮点的结果忽略
	@return : AlgoError
	@Author : lipengpeng@2025/04/22
	************************************/
	C_API AlgoError C_CALL LumChromaDetectGetResultList(AlgoHandle lum_chroma_detect_handle, LumChromaDetectFlag in_filter_defect_type,
		AlgoPoint* out_defect_loc_list, int* out_defect_num, LumChromaDetectFlag in_ignore_defect_type = LUM_CHROMA_DETECT_FLAG_UNINIT);


	/************************************
	@brief  : 获取缺陷灯点表征值
	@note   : 仅支持板内亮暗点
	@param  : lum_chroma_detect_handle        有效的句柄,不能为空
	@param  : in_defect_loc_list              输出缺陷灯点位置List
	@param  : in_defect_num                   输出缺陷灯点数量
	@param  : out_defect_data                 获取缺陷灯点表征值
	@return : AlgoError
	@Author : lipengpeng@2025/06/13
	************************************/
	C_API AlgoError C_CALL LumChromaDetectGetResultData(AlgoHandle lum_chroma_detect_handle, AlgoPoint* in_defect_loc_list,
		int in_defect_num, float* out_defect_data);


	/************************************
	@brief  : 空间内存释放
	@note   : 句柄指向的对象销毁,此句柄已经无效,请配合handle=nullptr
	@param  : lum_chroma_detect_handle        有效的句柄,不能为空
	@return : AlgoError
	@Author : lipengpeng@2025/04/22
	************************************/
	C_API void C_CALL LumChromaDetectDestroyHandle(AlgoHandle lum_chroma_detect_handle);

	/************************************
	@brief  : 句柄指向的对象的内存空间释放,句柄指向的对象还在
	@note   : 内存释放
	@param  : lum_chroma_detect_handle        有效的句柄,不能为空
	@return : AlgoError
	@Author : lipengpeng@2025/04/22
	************************************/
	C_API AlgoError C_CALL LumChromaDetectDestroyRelease(AlgoHandle lum_chroma_detect_handle);


	///********************buffer相关的***************************
	/************************************
	@brief  : 载入和保存lum_chroma_detector的阈值
	@note   : 文件名后缀必须为.meterdefect
	@param	: lum_chroma_detect_handle		  有效的句柄
	@param  : in_meter_detect_fullname        文件后缀必须为.meterdefect
	@return : AlgoError
	@Author : lipengpeng@2025/04/22
	************************************/
	C_API AlgoError C_CALL LumChromaDetectLoad(AlgoHandle lum_chroma_detect_handle, const char* in_meter_detect_fullname);
	C_API AlgoError C_CALL LumChromaDetectSave(AlgoHandle lum_chroma_detect_handle, const char* in_meter_detect_fullname);

	/************************************
	@brief  : 获取检测buffer大小
	@note   :
	@param	: lum_chroma_detect_handle		  有效的句柄
	@param  : out_len                         buffer大小
	@return : AlgoError
	@Author : lipengpeng@2025/04/22
	************************************/
	C_API AlgoError C_CALL LumChromaDetectGetBufferSize(AlgoHandle lum_chroma_detect_handle, uint64_t* out_len);

	/************************************
	@brief  : 将lum_chroma_detector里面的数据导出到buffer
	@note   :
	@param	: lum_chroma_detect_handle		  有效的句柄
	@param  : out_buffer                      已经申请好申请空间
	@param  : in_out_len                      外部申请好的空间大小,会用真实大小修改
	@return : AlgoError
	@Author : lipengpeng@2025/04/22
	************************************/
	C_API AlgoError C_CALL LumChromaDetectToBuffer(AlgoHandle lum_chroma_detect_handle, char* out_buffer, uint64_t* in_out_len);

	/************************************
	@brief  : 将buffer数据导入到lum_chroma_detector
	@note   :
	@param	: lum_chroma_detect_handle		  有效的句柄
	@param  : in_buffer
	@param  : in_len
	@return : AlgoError
	@Author : lipengpeng@2025/04/22
	************************************/
	C_API AlgoError C_CALL LumChromaDetectFromBuffer(AlgoHandle lum_chroma_detect_handle, const char* in_buffer, uint64_t in_len);


#ifdef __cplusplus
}
#endif // __cplusplus
#endif