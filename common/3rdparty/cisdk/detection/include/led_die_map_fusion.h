#ifndef _LED_DIE_MAP_FUSION_H_
#define _LED_DIE_MAP_FUSION_H_

#include "ci_declare.h"
#include "ci_data.h"
#include "led_die_map_fusion_data.h"
#include "led_die_map_fusion_error.h"
#include "die_bonding_defect_detector_data.h"
#include "algo_mat.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

	/*********************************************
	@brief   : 创建LedDieMap句柄
	@note    :
	@param   : fushion_handle
	@return  : AlgoError
	@Author  : shimai@2024/04/29
	*********************************************/
	C_API AlgoError C_CALL LedDieMapCreatHandle(AlgoHandle* fushion_handle);


	/*******************************************
	@brief   : 设置相机信息
	@note    :
	@param   : fushion_handle       有效的融合句柄
	@param   : in_cam_info          输入的相机信息
	@return  : AlgoError
	@Author  : shimai@2024/04/29
	********************************************/
	C_API AlgoError C_CALL LedDieMapSetCamInfo(AlgoHandle fushion_handle, const LedDieMapCameraInfo* in_cam_info);


	/*******************************************
	@brief   : 设置Mark点的坐标信息
	@note    :
	@param   : fushion_handle       有效的融合句柄
	@param   : in_panel_coordinate  输入的灯板坐标信息坐标
	@return  : AlgoError
	@Author  : shimai@2024/04/29
	********************************************/
	C_API AlgoError C_CALL LedDieMapSetMarkCoordinate(AlgoHandle fushion_handle, const LedDieMapMarkCoordinate* in_panel_coordinate);


	/*******************************************
	@brief   : 设置灯板信息
	@note    :
	@param   : fushion_handle       有效的融合句柄
	@param   : in_panel_info        输入的灯板信息
	@return  : AlgoError
	@Author  : shimai@2024/04/29
	********************************************/
	C_API AlgoError C_CALL LedDieMapSetPanelInfo(AlgoHandle fushion_handle, const LedDieMapPanelInfo* in_panel_info);


	/******************************************
	@brief   : 插入一条需要融合的数据
	@note    :
	@param   : fushion_handle       有效的融合句柄
	@param   : in_chip_group_info   在当前图像下的检测结果
	@param   : in_data_nums         检测出的芯片组数目
	@param   : in_cam_coordinate_mach 相机采集位置的机械坐标
	@return  : AlgoError
	@Author  : shimai@2024/04/29
	*******************************************/
	C_API AlgoError C_CALL LedDieMapInsertData(AlgoHandle fushion_handle, const DieBondingChipGroupInfo* in_chip_group_info, int in_data_nums,  AlgoPoint2f in_cam_coordinate_mach);
	

	/*******************************************
	@brief   : 对检测结果进行数据融合
	@note    :
	@param   : fushion_handle       有效的融合句柄
	@param   : fuse_with_panel_space是否通过生成的灯板融合数据(true表示通过预置灯板信息来进行融合，false表示对任意数据进行融合(不推荐))
	@param   : out_data_rows        输出融合后数据的行数
	@param   : out_data_cols        输出融合后数据的列数
	@return  : AlgoError
	@Author  : shimai@2024/04/29
	********************************************/
	C_API AlgoError C_CALL LedDieMapFuse(AlgoHandle fushion_handle, int* out_data_rows, int* out_data_cols, bool fuse_with_panel_space = true);


	/*******************************************
	@brief   : 获取指定行列上像素的信息
	@note    :
	@param   : fushion_handle       有效的融合句柄
	@param   : in_row               输入的行信息
	@param   : in_col               输入的列信息
	@param   : out_chip_group_info  输出对应像素的信息
	@return  : AlgoError
	@Author  : shimai@2024/04/29
	********************************************/
	C_API AlgoError C_CALL LedDieMapGetChipGroupInfo(AlgoHandle fushion_handle, int in_row, int in_col, DieBondingChipGroupInfo* out_chip_group_info);


	/*******************************************
	@brief   : 获取全部像素的信息
	@note    :
	@param   : fushion_handle       有效的融合句柄
	@param   : out_chip_group_info  输出全部像素的信息
	@param   : out_nums             输出像素个数
	@return  : AlgoError
	@Author  : shimai@2024/04/29
	********************************************/
	C_API AlgoError C_CALL LedDieMapGetAllChipGroupInfo(AlgoHandle fushion_handle, DieBondingChipGroupInfo* out_chip_group_info, int* out_nums);


	/*******************************************
	@brief   : 生成Map图
	@note    :
	@param   : fushion_handle       有效的融合句柄
	@param   : out_map_image        输出的Map图
	@return  : AlgoError
	@Author  : shimai@2024/04/29
	********************************************/
	C_API AlgoError C_CALL LedDieMapGenerateMap(AlgoHandle fushion_handle, AlgoMat* out_map_image);


	/*******************************************
	@brief   : 保存Map图
	@note    :
	@param   : fushion_handle       有效的融合句柄
	@param   : in_map_root          Map图的保存路径
	@return  : AlgoError
	@Author  : shimai@2024/04/29
	********************************************/
	C_API AlgoError C_CALL LedDieMapExportMap(AlgoHandle fushion_handle, const char* in_map_root);


	/*********************************************
	@brief   : 获取buffer大小
	@note    :
	@param   : fushion_handle       有效的融合句柄
	@param   : out_len              buffer大小
	@return  : AlgoError
	@Author  : shimai@2024/04/29
	**********************************************/
	C_API AlgoError C_CALL LedDieMapGetBufferSize(AlgoHandle fushion_handle, uint64_t* out_len);


	/******************************************
	@brief   : 将数据导出buffer里
	@note    :
	@param   : fushion_handle      有效的融合句柄
	@param   : out_buffer          输出数据信息的buffer空间
	@param   : in_out_len          buffer空间大小
	@return  : AlgoError
	@Author  : shimai@2024/04/29
	*******************************************/
	C_API AlgoError C_CALL LedDieMapToBuffer(AlgoHandle fushion_handle, char* out_buffer, uint64_t* in_out_len);


	/************************************
	@brief    : 从buffer中导入数据到句柄
	@note     :
	@param    : fushion_handle      有效的融合句柄
	@param    : in_buffer           buffer
	@param    : in_len              buffer大小
	@return   : AlgoError
	@Author   : shimai@2024/04/29
	************************************/
	C_API AlgoError C_CALL LedDieMapFromBuffer(AlgoHandle fushion_handle, const char* in_buffer, uint64_t in_len);


	/***********************************************
	@brief   : 载入检测结果
	@note    : 后缀名只能是.cobaoi
	@param   : fushion_handle     有效的融合句柄
	@param   : in_fullname        后缀只能是.cobaoi
	@return  : AlgoError
	@Author  : shimai@2024/04/29
	************************************************/
	C_API AlgoError C_CALL LedDieMapLoad(AlgoHandle fushion_handle, const char* in_fullname);


	/***********************************************
	@brief   : 保存检测结果
	@note    : 后缀名只能是.cobaoi
	@param   : fushion_handle     有效的融合句柄
	@param   : in_fullname        后缀只能是.cobaoi
	@return  : AlgoError
	@Author  : shimai@2024/04/29
	************************************************/
	C_API AlgoError C_CALL LedDieMapSave(AlgoHandle fushion_handle, const char* in_fullname);


	/***********************************************
	@brief   : 句柄指向的对象的内存空间释放，句柄指向的对象还在
	@note    : 内存释放
	@param   : fushion_handle    有效的融合句柄
	@return  : AlgoError
	@Author  : shimai@2024/04/29
	************************************************/
	C_API AlgoError C_CALL LedDieMapRelease(AlgoHandle fushion_handle);


	/*******************************************
	@brief   : 空间内存释放
	@note    : 句柄指向的对象销毁，此句柄已经无效，请配合handle=nullptr
	@param   : fushion_handle      有效的融合句柄
	@return  :
	@Author  : shimai@2024/04/29
	********************************************/
	C_API void C_CALL LedDieMapDestroyHandle(AlgoHandle fushion_handle);


#ifdef __cplusplus
}
#endif
#endif