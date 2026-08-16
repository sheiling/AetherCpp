#ifndef __GOLDEN_SAMPLE_GENERATOR_H__
#define __GOLDEN_SAMPLE_GENERATOR_H__

#include "ci_declare.h"
#include "ci_data.h"
#include "golden_sample_generator_data.h"
#include "golden_sample_generator_error.h"
#include "algo_mat.h"


#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

	/*********************************************
	@brief   : 创建GoldenSampleGenerator句柄
	@note    :
	@param   : generator_handle
	@return  : AlgoError
	@Author  : wushaoshuang@2024/04/09
	*********************************************/
	C_API AlgoError C_CALL GoldenSampleCreateHandle(AlgoHandle* generator_handle);

	/*********************************************
	@brief   : 从model文件设置模板
	@note    : 人工选择的初始模板，模板框选目标位置要求与后续匹配算法对模板的要求保持一致，该算法不支持对模板图像中目标的位置做调整。配合model模块使用。
	@param   : generator_handle     有效的模板生成句柄
	@param   : model_handle         有效的model句柄 
	@param   : in_template_name     模板名称
	@return  : AlgoError
	@Author  : wushaoshuang@2024/04/09
	*********************************************/
	C_API AlgoError C_CALL GoldenSampleSetTemplateFromModel(AlgoHandle generator_handle, AlgoHandle model_handle,
		const char* in_template_name);

	/*********************************************
	@brief   : 从图像设置模板
	@note    : 人工选择的初始模板，模板框选目标位置要求与后续匹配算法对模板的要求保持一致，该算法不支持对模板图像中目标的位置做调整。该接口输入直接为一个AlgoMat模板图像。
	@param   : generator_handle     有效的模板生成句柄
	@param   : in_template_img      输入的模板图像
	@return  : AlgoError
	@Author  : wushaoshuang@2024/04/15
	*********************************************/
	C_API AlgoError C_CALL GoldenSampleSetTemplateFromMat(AlgoHandle generator_handle, AlgoMat* in_template_img);


	/*********************************************
	@brief   : 设置待测样本目标类型
	@note    : 设置样本图像中目标的类型，即样本图像中是单个目标还是多个目标，如mark点样本、中转位样本等为单目标，wafer位样本等为多目标。内部默认为多目标样本
	@param   : generator_handle        有效的模板生成句柄
	@param   : in_sample_target_type   输入的样本目标类型枚举，SampleTargetType枚举，详见golden_sample_generator_data.h
	@return  : AlgoError
	@Author  : wushaoshuang@2024/04/09
	*********************************************/
	C_API AlgoError C_CALL GoldenSampleSetTargetType(AlgoHandle generator_handle, SampleTargetType* in_sample_target_type);

	/*********************************************
	@brief   : 设置目标匹配阈值
	@note    : 即设置样本中匹配值大于该阈值时认为是目标，小于该阈值时是非目标，建议与后续匹配算法中设置的阈值保持一致。
	@param   : generator_handle        有效的模板生成句柄
	@param   : in_thresh               输入的阈值结构体，GoldenSampleThresh枚举，详见golden_sample_generator_data.h
	@return  : AlgoError
	@Author  : wushaoshuang@2024/04/09
	*********************************************/
	C_API AlgoError C_CALL GoldenSampleSetThresh(AlgoHandle generator_handle, GoldenSampleThresh* in_thresh);

	/*********************************************
	@brief   : 生成最优模板
	@note    : 生成最优模板主接口
	@param   : generator_handle        有效的模板生成句柄
	@param   : in_image                输入单个/多个待测样本图像，需要外部new内存空间以存储待测样本
	@param   : in_image_num            输入的样本数量，与外部new的内存空间大小保持一致（即有多少个样本图像，多目标时只需一个样本图像，单目标时多个样本图像）
	@param   : out_golden_sample       输出的最优模板图像
	@return  : AlgoError
	@Author  : wushaoshuang@2024/04/09
	*********************************************/
	C_API AlgoError C_CALL GoldenSampleGenerate(AlgoHandle generator_handle, AlgoMat* in_image,int in_image_num, 
		AlgoMat* out_golden_sample);

	/*********************************************
	@brief   : 获取初始模板
	@note    : 获取初始模板,根据需求调用
	@param   : generator_handle        有效的模板生成句柄
	@param   : out_org_template        输出的原始模板图像
	@return  : AlgoError
	@Author  : wushaoshuang@2024/04/09
	*********************************************/
	C_API AlgoError C_CALL GoldenSampleGetTemplate(AlgoHandle generator_handle, AlgoMat* out_org_template);

	/*********************************************
	@brief   : 获取待测样本目标类型
	@note    : 获取待测样本目标类型，根据需求调用
	@param   : generator_handle        有效的模板生成句柄
	@param   : out_sample_target_type  输出的样本目标类型枚举
	@return  : AlgoError
	@Author  : wushaoshuang@2024/04/09
	*********************************************/
	C_API AlgoError C_CALL GoldenSampleGetTargetType(AlgoHandle generator_handle, SampleTargetType* out_sample_target_type);

	/*********************************************
	@brief   : 获取目标匹配阈值
	@note    : 获取目标匹配阈值，根据需求调用
	@param   : generator_handle        有效的模板生成句柄
	@param   : out_thresh              输出的阈值
	@return  : AlgoError
	@Author  : wushaoshuang@2024/04/09
	*********************************************/
	C_API AlgoError C_CALL GoldenSampleGetThresh(AlgoHandle generator_handle, GoldenSampleThresh* out_thresh);

	/*********************************************
	@brief   : 导出最优模板图像
	@note    : 保存最优模板结果图以及最优模板结果图在样本中的位置图像，根据需求调用
	@param   : generator_handle        有效的模板生成句柄
	@param   : in_png_path             输入的图像存储路径
	@return  : AlgoError
	@Author  : wushaoshuang@2024/04/09
	*********************************************/
	C_API AlgoError C_CALL GoldenSampleExportPng(AlgoHandle generator_handle, const char* in_png_path);

	/*********************************************
	@brief   : 获取buffer大小
	@note    : 
	@param   : generator_handle        有效的模板生成句柄
	@param   : out_len                 buffer大小
	@return  : AlgoError
	@Author  : wushaoshuang@2024/04/09
	*********************************************/
	C_API AlgoError C_CALL GoldenSampleGetBufferSize(AlgoHandle generator_handle, uint64_t* out_len);

	/*********************************************
	@brief   : 将数据导入buffer
	@note    :
	@param   : generator_handle        有效的模板生成句柄
	@param   : out_buffer              输出检测参数的buffer空间
	@param   : in_out_len              buffer空间大小
	@return  : AlgoError
	@Author  : wushaoshuang@2024/04/09
	*********************************************/
	C_API AlgoError C_CALL GoldenSampleToBuffer(AlgoHandle generator_handle, char* out_buffer, uint64_t* in_out_len);

	/*********************************************
	@brief   : 将buffer数据读取到GoldenSample
	@note    :
	@param   : generator_handle        有效的模板生成句柄
	@param   : in_buffer               buffer
	@param   : in_len                  buffer大小
	@return  : AlgoError
	@Author  : wushaoshuang@2024/04/09
	*********************************************/
	C_API AlgoError C_CALL GoldenSampleFromBuffer(AlgoHandle generator_handle, const char* in_buffer, uint64_t in_len);

	/*********************************************
	@brief   : 载入GoldeSample文件
	@note    :
	@param   : generator_handle        有效的模板生成句柄
	@param   : in_load_path            输入的文件路径，后缀必须为.sample
	@return  : AlgoError
	@Author  : wushaoshuang@2024/04/09
	*********************************************/
	C_API AlgoError C_CALL GoldenSampleLoad(AlgoHandle generator_handle, const char* in_load_path);

	/*********************************************
	@brief   : 保存GoldeSample文件
	@note    :
	@param   : generator_handle        有效的模板生成句柄
	@param   : in_save_path            输入要保存的文件路径，后缀必须为.sample
	@return  : AlgoError
	@Author  : wushaoshuang@2024/04/09
	*********************************************/
	C_API AlgoError C_CALL GoldenSampleSave(AlgoHandle generator_handle, const char* in_save_path);

	/*********************************************
	@brief   : 空间内存释放
	@note    : 句柄指向的对象销毁，此句柄已经无效，请配合handle=nullptr
	@param   : generator_handle        有效的模板生成句柄
	@return  : 
	@Author  : wushaoshuang@2024/04/09
	*********************************************/
	C_API void C_CALL GoldenSampleDestroyHandle(AlgoHandle generator_handle);

	/*********************************************
	@brief   : 句柄指向的对象的内存空间释放，句柄指向的对象还在
	@note    : 内存释放
	@param   : generator_handle        有效的模板生成句柄
	@return  : AlgoError
	@Author  : wushaoshuang@2024/04/09
	*********************************************/
	C_API AlgoError C_CALL GoldenSampleRelease(AlgoHandle generator_handle);



#ifdef __cplusplus
}
#endif
#endif