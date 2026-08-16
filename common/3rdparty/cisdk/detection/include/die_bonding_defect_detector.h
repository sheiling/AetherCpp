#ifndef __DIE_BONDING_DEFECT_DETECTOR_H__
#define __DIE_BONDING_DEFECT_DETECTOR_H__


#include "ci_declare.h"
#include "ci_data.h"
#include "die_bonding_defect_detector_data.h"
#include "die_bonding_defect_detector_error.h"
#include "algo_mat.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

	/*********************************************
	@brief   : 创建DieBondingDefectCreateHandle检测句柄
	@note    :
	@param   : detector_handle
	@return  : AlgoError
	@Author  : shimai@2024/03/05
	*********************************************/
	C_API AlgoError C_CALL DieBondingCreateHandle(AlgoHandle* detector_handle);

	/******************************************
	@brief   : 设定参数信息
	@note    :
	@param   : detector_handle      有效的检测句柄
	@param   : in_parameter         灯板的参数信息
	@param   : in_align_type        RGB芯片大排列方式
	@return  : AlgoError
	@Author  : shimai@2024/03/05
	*******************************************/
	C_API AlgoError C_CALL DieBondingSetParameter(AlgoHandle detector_handle, const DieBondingParameter* in_parameter, 
		DieBondingChipAlignType in_align_type);
	
	/******************************************
	@brief   : 设定芯片组阈值
	@note    :
	@param   : detector_handle      有效的检测句柄
	@param   : in_thresh            设定芯片组相关的阈值
	@return  : AlgoError
	@Author  : shimai@2024/03/05
	*******************************************/
	C_API AlgoError C_CALL DieBondingSetGroupThresh(AlgoHandle detector_handle, const DieBondingGroupThreshold* in_thresh);

	/******************************************
	@brief   : 设定芯片相关阈值
	@note    :
	@param   : detector_handle      有效的检测句柄
	@param   : in_thresh            设定芯片相关的阈值
	@param   : in_chip_type         设定芯片类型
	@return  : AlgoError
	@Author  : shimai@2024/03/05
	*******************************************/
	C_API AlgoError C_CALL DieBondingSetChipThresh(AlgoHandle detector_handle, const DieBondingChipThreshold* in_thresh, 
		DieBondingChipType in_chip_type);

	/*******************************************
	@brief   : 设定芯片组的匹配模板
	@note    : 配合model模块使用，具体操作见demo
	@param   : detector_handle      有效的检测句柄
	@param   : model_handle         model句柄
	@param   : in_template_name     模版名
	@return  : AlgoError
	@Author  : shimai@2024/03/05
	********************************************/
	C_API AlgoError C_CALL DieBondingSetGroupTemplate(AlgoHandle detector_handle, AlgoHandle model_handle, 
		const char* in_template_name);

	/*******************************************
	@brief   : 设定芯片相关的匹配模板
	@note    : 配合model模块使用，具体操作见demo
	@param   : detector_handle      有效的检测句柄
	@param   : model_handle         model句柄
	@param   : in_template_name     模版名
	@param   : in_chip_type			芯片类型
	@param   : in_template_type		模板类型
	@return  : AlgoError
	@Author  : shimai@2024/03/05
	********************************************/
	C_API AlgoError C_CALL DieBondingSetChipTemplate(AlgoHandle detector_handle, AlgoHandle model_handle, 
		const char* in_template_name, DieBondingChipType in_chip_type, DieBondingTemplateType in_template_type);

	/*******************************************
	@brief   : 设定当前图像的索引（索引需要具有唯一性，且要与所属图像对应），用于后续数据融合时确定某一数据属于哪张图像
	@note    : 配合model模块使用，具体操作见demo
	@param   : detector_handle      有效的检测句柄
	@param   : in_image_index		输入的图像索引
	@return  : AlgoError
	@Author  : shimai@2024/05/07
	********************************************/
	C_API AlgoError C_CALL DieBondingSetImageIndex(AlgoHandle detector_handle, const char* in_image_index);

	/*******************************************
	@brief   : 设定脏污检测差分阈值
	@note    : 设定脏污检测差分阈值
	@param   : detector_handle      有效的检测句柄
	@param   : in_area_thresh       输入的脏污面积阈值，表示当前芯片上的脏污面积占模板芯片面积的值为阈值以上时，才认为是脏污，内部默认为0.05，即脏污面积为芯片面积的5%以上时，就存在脏污，外部可根据实际情况调整
	@param   : in_diff_thresh		输入的阈值,表示当前芯片与模板芯片的差异化为阈值以上时，认为存在脏污，以下时认为不存在，内部默认为80，外部一般无需修改
	@return  : AlgoError
	@Author  : wushaoshuang@2025/06/26
	********************************************/
	C_API AlgoError C_CALL DieBondingSetDirtyThresh(AlgoHandle detector_handle, const float* in_area_thresh, const int* in_diff_thresh);

	/*******************************************
	@brief   : 设定线程
	@note    : 设定当前检测时算法计算所使用的线程情况,包括是否使用线程，以及使用线程时的线程数量，内部默认使用线程且线程数为8
	@param   : detector_handle      有效的检测句柄
	@param   : in_threads_flag      输入的是否开启多线程标识，为true时表示开启多线程，false时表示不开启多线程并行
	@param   : in_threads_num		输入的线程数量，开启多线程时需要设置，内部默认为8个线程,不开启多线程并行时不用设置
	@return  : AlgoError
	@Author  : wushaoshuang@2025/06/27
	********************************************/
	C_API AlgoError C_CALL DieBondingSetThread(AlgoHandle detector_handle, const bool* in_threads_flag, const int in_threads_num = 8);

	/*******************************************
	@brief   : 设定芯片缺失检测阈值及参数
	@note    : 设定芯片缺失检测时的阈值以及对应参数，一般无需设置，特殊情况下可修改
	@param   : detector_handle      有效的检测句柄
	@param   : in_BG_thresh		    输入的B芯片和G芯片阈值（BG共用）R芯片暂时用不到，值一般在50-150之间，表示芯片与焊盘之间的亮度差异，内部默认为120，若存在正常芯片被误检测为漏固缺陷，则调大该值，若本身漏固但被误检测为其他缺陷，则减小该值，调整步长以10为单位
	@param   : in_ratio             输入的检测区域比例，一般为2-5之间，表示芯片缺失检测时对比的区域大小，默认为5，外部一般无需修改，若阈值调整不满足要求时，再调整该值，若存在正常芯片被误检测为漏固缺陷，则减小该值，若本身漏固但被误检测为其他缺陷，则增大该值，步长为1
	@return  : AlgoError
	@Author  : wushaoshuang@2025/06/27
	********************************************/
	C_API AlgoError C_CALL DieBoundingSetMissingThresh(AlgoHandle detector_handle,const int* in_BG_thresh, const int in_ratio = 5);

	/*******************************************
	@brief   : 设定异物检测阈值
	@note    : 设定异物检测时的阈值
	@param   : detector_handle      有效的检测句柄
	@param   : in_thresh		    输入的阈值，意义表示检测到的目标面积与焊盘面积的比值，该值内部默认为0.6，范围一般大于0.5，外部根据实际情况调节，当发现存在异物（或不完整芯片）被错检为缺失时，减小该值，若发现焊盘本身无芯片且无异物（本身为芯片缺失）被错检为异物时，则怎大该值，该值的调节步长为0.05
	@return  : AlgoError            
	@Author  : wushaoshuang@2025/07/26
	********************************************/
	C_API AlgoError C_CALL DieBoundingSetForeignThresh(AlgoHandle detector_handle, const float* in_thresh);

	/******************************************
	@brief   : 定位图像中全部芯片组的坐标（主要接口）
	@note    :
	@param   : detector_handle      有效的检测句柄
	@param   : in_image             需要进行芯片组定位的图像
	@return  : AlgoError
	@Author  : shimai@2024/03/04
	*******************************************/
	C_API AlgoError C_CALL DieBondingChipGroupLocate(AlgoHandle detector_handle, const AlgoMat* in_image);




	/******************************************
	@brief   : 计算芯片组的行列数
	@note    :
	@param   : locator_handle       有效的定位句柄
	@param   : out_row              芯片组行数
	@param   : out_col              芯片组列数
	@return  : AlgoError
	@Author  : shimai@2024/03/04
	*******************************************/
	C_API AlgoError C_CALL DieBondingCalGroupRowCol(AlgoHandle detector_handle, int* out_row, int* out_col);


	///******************************************
	//@brief   : 获取图像中全部芯片组坐标
	//@note    :
	//@param   : locator_handle       有效的定位句柄
	//@param   : out_coordinates      芯片组坐标数组
	//@return  : AlgoError
	//@Author  : shimai@2024/03/04
	//*******************************************/
	//C_API AlgoError C_CALL DieBondingGetGroupCoordinates(AlgoHandle detector_handle, AlgoRect * out_coordinates);


	/******************************************
	@brief   : 对图像中全部芯片进行检测（主要接口，需要进行芯片组定位DieBondingChipGroupLocate后调用）
	@note    :
	@param   : detector_handle      有效的检测句柄
	@param   : in_align_type        芯片的排列方式
	@return  : AlgoError
	@Author  : shimai@2024/03/05
	*******************************************/
	C_API AlgoError C_CALL DieBondingDetect(AlgoHandle detector_handle, DieBondingChipAlignType& in_align_type);


	/************************************
	@Brief		：	直线度偏移量检测 (主要接口 需要在DieBondingChipGroupLocate和DieBondingDetect后进行)
	@Note		：
	@Parameter	: 	detector_handle			     有效的检测句柄
	@Parameter	: 	out_offest_id_row		  -- 行方向，超出偏移范围的灯点编号，（第几列，第几行）
	@Parameter	: 	out_offest_row		      -- 行方向，灯点的实际直线度偏移量
	@Parameter	: 	out_num_row		          -- 行方向，超出检测阈值的灯点个数
	@Parameter	: 	out_offest_id_col		  -- 列方向，超出偏移范围的灯点编号，（第几列，第几行）
	@Parameter	: 	out_offest_row		      -- 行方向，灯点的实际直线度偏移量
	@Parameter	: 	out_num_col		          -- 列方向，超出检测阈值的灯点个数
	@Parameter	: 	detect_flag 	          -- 检测方向的标志
	@Returns	:   AlgoError
	@Author		：	shimai@2024/03/5 -->  haopanxia@2023/01/09
	************************************/
	C_API AlgoError C_CALL DieBondingLineOffestDetect(AlgoHandle detector_handle,
		AlgoPoint* out_offset_id_row, float* out_offset_row, int* out_num_row,
		AlgoPoint* out_offset_id_col, float* out_offset_col, int* out_num_col,
		DieBondingLineFlag detect_flag);


	///******************************************
	//@brief   : 获取检测后得到的全部芯片信息ChipInfo
	//@note    :
	//@param   : detector_handle      有效的检测句柄
	//@param   : out_chip_infos       全部chipinfo信息
	//@return  : AlgoError
	//@Author  : shimai@2024/03/05
	//*******************************************/
	//C_API AlgoError C_CALL DieBondingGetAllChipInfo(AlgoHandle detector_handle, DieBondingChipInfo* out_chip_infos);

	/******************************************
	@brief   : 获取检测后得到的全部像素(芯片组)信息
	@note    :
	@param   : detector_handle      有效的检测句柄
	@param   : out_chip_group_info  全部chipgroup信息
	@return  : AlgoError
	@Author  : shimai@2024/03/05
	*******************************************/
	C_API AlgoError C_CALL DieBondingGetAllChipGroupInfo(AlgoHandle detector_handle, DieBondingChipGroupInfo* out_chip_group_info);


	/***********************************************
	@brief   : 导出芯片检测检测结果图像（需要完成DieBondingDetect后进行，如果执行了DieBondingLineOffestDetect之后就会同时导出线性度检测的结果）
	@note    :
	@param   : detector_handle      有效的检测句柄
	@param   : out_image_filename   图像保存路径
	@return  : AlgoError
	@Author  : shimai@2024/03/04
	************************************************/
	C_API AlgoError C_CALL DieBondingExportDetectResult(AlgoHandle detector_handle, const char * out_image_filename);


	/*********************************************
	@brief   : 获取buffer大小
	@note    :
	@param   : detector_handle      有效的检测句柄
	@param   : out_len              buffer大小
	@return  : AlgoError
	@Author  : shimai@2024/03/05
	**********************************************/
	C_API AlgoError C_CALL DieBondingGetBufferSize(AlgoHandle detector_handle, uint64_t* out_len);


	/******************************************
	@brief   : 将数据导出buffer里
	@note    :
	@param   : detector_handle     有效的检测句柄
	@param   : out_buffer          输出检测参数的buffer空间
	@param   : in_out_len          buffer空间大小
	@return  : AlgoError
	@Author  : shimai@2024/03/04
	*******************************************/
	C_API AlgoError C_CALL DieBondingToBuffer(AlgoHandle detector_handle, char* out_buffer, uint64_t* in_out_len);



	///***********************************************
	//@brief   : 导出线性度检测结果图像（需要完成DieBondingLineOffestDetect后进行）
	//@note    :
	//@param   : detector_handle      有效的检测句柄
	//@param   : out_image_filename   图像保存路径
	//@return  : AlgoError
	//@Author  : shimai@2024/03/04
	//************************************************/
	//C_API AlgoError C_CALL DieBondingExportLineDetectResult(AlgoHandle detector_handle, const char* out_image_filename);


	/************************************
	@brief    : 从buffer中导入数据到句柄
	@note     :
	@param    : detector_handle      有效的检测句柄
	@param    : in_buffer           buffer
	@param    : in_len              buffer大小
	@return   : AlgoError
	@Author   : shimai@2024/03/04
	************************************/
	C_API AlgoError C_CALL DieBondingFromBuffer(AlgoHandle detector_handle, const char* in_buffer, uint64_t in_len);


	/***********************************************
	@brief   : 载入检测结果
	@note    : 后缀名只能是.chipinfo
	@param   : detector_handle    有效的检测句柄
	@param   : in_fullname        后缀只能是.chipinfo
	@return  : AlgoError
	@Author  : shimai@2024/03/14
	************************************************/
	C_API AlgoError C_CALL DieBondingLoad(AlgoHandle detector_handle, const char* in_fullname);

	/***********************************************
	@brief   : 保存检测结果
	@note    : 后缀名只能是.chipinfo
	@param   : detector_handle    有效的检测句柄
	@param   : in_fullname        后缀只能是.chipinfo
	@return  : AlgoError
	@Author  : shimai@2024/03/14
	************************************************/
	C_API AlgoError C_CALL DieBondingSave(AlgoHandle detector_handle, const char* in_fullname);
	
	/***********************************************
	@brief   : 句柄指向的对象的内存空间释放，句柄指向的对象还在
	@note    : 内存释放
	@param   : detector_handle    需要释放的句柄
	@return  : AlgoError
	@Author  : shimai@2024/03/04
	************************************************/
	C_API AlgoError C_CALL DieBondingRelease(AlgoHandle detector_handle);

	
	/*******************************************
	@brief   : 空间内存释放
	@note    : 句柄指向的对象销毁，此句柄已经无效，请配合handle=nullptr
	@param   : detector_handle      有效的检测句柄
	@return  :
	@Author  : shimai@2024/03/04
	********************************************/
	C_API void C_CALL DieBondingDestroyHandle(AlgoHandle detector_handle);

	///***********************************************
	//@brief   : 获取全部芯片组的坐标
	//@note    : 内存释放
	//@param   : detector_handle    有效的检测句柄
	//@param   : out_coordinates    全部芯片组的坐标
	//@return  : AlgoError
	//@Author  : shimai@2024/03/04
	//************************************************/
	////C_API AlgoError C_CALL DieBondingGetGroupCoordinates(AlgoHandle detector_handle, AlgoRect* out_coordinates);



	///******************************************
	//@brief   : 检测RGB芯片及对应芯片所在焊盘
	//@note    :
	//@param   : detector_handle      有效的检测句柄
	//@param   : in_image             需要检测的图像
	//@param   : in_rect              所属的芯片组的ROI范围
	//@param   : in_chip_type         待检测的芯片的类型
	//@return  : AlgoError
	//@Author  : shimai@2024/03/05
	//*******************************************/
	////C_API AlgoError C_CALL DieBondingSingleChipDetect(AlgoHandle detector_handle, const AlgoMat* in_image, const AlgoRect* in_rect, DieBondingChipType in_chip_type);

	///******************************************
	//@brief   : 定位图像中全部芯片组的坐标
	//@note    :
	//@param   : locator_handle       有效的定位句柄
	//@param   : in_image             需要检测的图像
	//@return  : AlgoError
	//@Author  : shimai@2024/03/04
	//*******************************************/
	////C_API AlgoError C_CALL DieBondingChipGroupLocate(AlgoHandle detector_handle, const AlgoMat* in_image);

#ifdef __cplusplus
}
#endif
#endif