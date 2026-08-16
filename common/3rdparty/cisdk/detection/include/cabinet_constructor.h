#ifndef __CABINET_CONSTRUCTOR__H__
#define __CABINET_CONSTRUCTOR__H__


#include "ci_declare.h"
#include "ci_data.h"
#include "meta_info_enum.h"
#include "cabinet_constructor_data.h"
#include "cabinet_constructor_error.h"
#include "algo_mat.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

	/*********************************************
	@brief   : 创建检测句柄
	@note    :
	@param   : detector_handle
	@return  : AlgoError
	@Author  : songshaojun@2026/01/30
	************************************/
	C_API AlgoError C_CALL CabinetConstructCreateHandle(AlgoHandle* out_construct_handle);


	/************************************
	@brief  : 设置箱体信息
	@note   : 
	@param  : construct_handle,							输入量, 有效的构造句柄
	@param  : in_cabinet_size,							输入量, 当前箱体灯点数，如常见480*270；
	@param  : in_module_num,							输入量, 当前箱体由多少个模组组成，如常见4*2；
	@param  : in_single_module_group_data_num,          输入量, 每个模组被几个数据组均分，如常见2；
	@param  : in_group_data_directioin,					输入量, 数据组方向，0代表水平，1代表垂直，如=1，代表在垂直方向将模组分为N个数据组。  
	@return	: AlgoError
	@Author : songshaojun@2026/01/30
	************************************/
	C_API AlgoError C_CALL CabinetConstructSetCabinetInfo(AlgoHandle construct_handle, const AlgoSize in_cabinet_size, const AlgoSize in_module_num,
		const int in_single_module_group_data_num, const int in_group_data_directioin);


	/************************************
	@brief  : 设置默认检测模式
	@note   : 当不调用该接口时，默认拍摄状态为逐灯清晰拍摄，基于光通量数据检测
	@param  : construct_handle,							输入量, 有效的构造句柄
	@param  : in_capture_type,							输入量, 送入检测画面是基于光通量数据还是shot数据
														in_capture_type = ALGOMOD_ALGOLEDMETER代表基于Meter数据,逐灯清晰拍摄
														in_capture_type = ALGOMOD_ALGOSHOT代表基于Shot数据，模糊拍摄
	@return	: AlgoError
	@Author : songshaojun@2026/01/30
	************************************/
	C_API AlgoError C_CALL CabinetConstructSetDefaultMode(AlgoHandle construct_handle, const int in_capture_type);


	/************************************
	@brief  : 设置箱体点亮后的信息，包含箱体以纯绿色画面点亮时的Meter信息或shot信息
	@note   : 默认输入的点亮信息为Meter类型
			  当拍摄模式为模糊拍摄（ALGOMOD_ALGOSHOT）时,不需要调用该接口；
			  in_lighted_type参数、in_roi_handle参数为预留参数；
	@param  : construct_handle,							输入量, 有效的构造句柄
	@param  : in_lighted_info,							输入量, 箱体纯色-绿色画面下，提取的Meter的Handle或ShotHandle
	@param  : in_lighted_type,							输入量, in_lighted_info的类型是什么，ALGOMOD_ALGOLEDMETER或ALGOMOD_ALGOSHOT
	@param  : in_roi_handle,							输入量, 当前箱体的ROIhandle
	@return	: AlgoError
	@Author : songshaojun@2026/01/30
	************************************/
	C_API AlgoError C_CALL CabinetConstructSetLightedInfo(AlgoHandle construct_handle, const AlgoHandle in_lighted_info,
		int in_lighted_type = ALGOMOD_ALGOLEDMETER,const AlgoHandle in_roi_handle = nullptr);


	/************************************
	@brief  : 获取构造箱体时首次下发的数据组顺序及打屏pattern
	@note   : 可根据默认参数，决定返回pattern的灰阶及颜色
	@param  : construct_handle,							输入量, 有效的构造句柄
	@param  : out_data_order,							输出量, 返回的数据组顺序
	@param  : out_pattern,								输出量, 返回的上屏pattern
	@param  : in_gray_level,							输入量, 期望返回的上屏pattern的灰阶，默认200灰阶
	@param  : in_color,									输入量, 期望返回的上屏pattern的颜色，默认绿色，推荐绿色
	@return	: AlgoError
	@Author : songshaojun@2026/01/30
	************************************/
	C_API AlgoError C_CALL CabinetConstructInitDataConfig(AlgoHandle construct_handle, AlgoMat* out_data_order, AlgoMat* out_pattern, 
		 int in_gray_level = 200, AlgoColor in_color = COLOR_GREEN);


	/************************************
	@brief  : 根据输入画面，检测当前画面，是否具备自动调整的条件，若具备则输出调整至正确显示所需要的数据组顺序
	@note   : 接口调用后，需判断标志位out_can_auto_construct，若为true，则可继续进行，否则无法继续构造
	@param  : construct_handle,							输入量, 有效的构造句柄
	@param  : in_detect,								输入量, 初始的数据组及打屏pattern下发后，采集到的检测画面,采集到的meter或shot
	@param  : out_can_auto_construct,					输出量, 当前屏体是否能够进行自动箱体构造，
																out_data_group_order=true，可以自动构造，流程继续进行
																out_data_group_order=false，不可以自动构造，流程终止，检查屏体状态
	@param  : out_is_order_equal_init,					输出量, 当前屏体的数据组索引顺序是否与初始下发的一致，
																out_is_order_equal_init=true，与初始化下发的数据组顺序一致，可直接进入BUS检查，不需要再次下发
																out_is_order_equal_init=false，与初始化下发的数据组顺序不一致，需要再次下发
	@param  : out_data_group_order,						输出量, 输出的当前屏体正确的数据组顺序
	@return	: AlgoError
	@Author : songshaojun@2026/01/30
	************************************/    
	C_API AlgoError C_CALL CabinetConstructGetDataGroupOrder(AlgoHandle construct_handle, const AlgoHandle in_detect, 
		bool* out_can_auto_construct, bool *out_is_order_equal_init, AlgoMat *out_data_group_order);
	

	/************************************
	@brief  : 检测当前显示画面是否与输入画面一致
	@note   : 
	@param  : construct_handle,							输入量, 有效的构造句柄
	@param  : in_detect,								输入量, 正确数据组顺序下发后，采集到的检测画面
	@param  : in_pattern,								输入量, 当前的屏上pattern
	@param  : out_is_correct,							输出量, 当前箱体是否与输入画面一致的flag
																out_is_correct = true,显示画面一致;
																out_is_correct = flase,显示画面不一致;																												
	@return	: AlgoError
	@Author : songshaojun@2026/01/30
	************************************/
	C_API AlgoError C_CALL CabinetConstructCheckDisplayCorrect(AlgoHandle construct_handle, const AlgoHandle in_detect, AlgoMat *in_pattern, bool* out_is_correct);
	

	/************************************
	@brief  : 生成用于BUS检查的特殊校正系数及初始化BUS索引
	@note   : 
	@param  : construct_handle,							输入量, 有效的构造句柄
	@param  : out_init_bus_order,						输出量, 当前箱体进行BUS检查，所需下发的初始化BUS索引顺序
	@param  : out_pattern_coeff_handle,					输出量, 当前箱体进行BUS检查，所需要上屏的特殊校正系数句柄	
	@param  : in_data_group_order,						输入量, 当前箱体正确的数据组索引序号，若不输入，则默认使用算法内部计算值
	@return	: AlgoError
	@Author : songshaojun@2026/01/30
	************************************/
	C_API AlgoError C_CALL CabinetConstructGenBUSCoeff(AlgoHandle construct_handle,  AlgoMat *out_init_bus_order, AlgoHandle out_pattern_coeff_handle, AlgoMat *in_data_group_order = nullptr);


	/************************************
	@brief  : 特殊校正系数上屏后，使用相机采集并检测当前画面，输出当前箱体的正确BUS顺序
	@note   : 
	@param  : construct_handle,							输入量, 有效的构造句柄
	@param  : in_detect,								输入量, 特殊校正系数上屏后，采集到的检测画面
	@param  : out_is_order_equal_init,					输出量, 当前屏体的bus索引顺序是否与初始下发的一致，
																out_is_order_equal_init=true，与初始化下发的BUS顺序一致，不需要再次下发
																out_is_order_equal_init=false，与初始化下发的BUS顺序不一致，需要再次下发
	@param  : out_BUS_order,							输出量, 当前箱体正确的BUS顺序
	@return	: AlgoError
	@Author : songshaojun@2026/01/30
	************************************/  
	C_API AlgoError C_CALL CabinetConstructGetBUSOrder(AlgoHandle construct_handle, const AlgoHandle in_detect, bool* out_is_order_equal_init, AlgoMat *out_BUS_order);


	/************************************
	@brief  : 输出计算过程中部分可视化结果
	@note   : 支持检测接口CabinetConstructGetDataGroupOrder、CabinetConstructGetBUSOrder计算后调用
				CabinetConstructExportResultPng接口将结果落盘
				CabinetConstructExportResultMat接口将结果返回在内存中
	@param  : construct_handle,							输入量, 有效的构造句柄
	@param  : out_export_root,							输入量, 导出可视化结果存储落盘路径
	@param  : out_export_mat,							输入输出量, 导出可视化结果,存储在送入的AlgoMat中
	@return	: AlgoError
	@Author : songshaojun@2026/01/30
	************************************/
	C_API AlgoError C_CALL CabinetConstructExportResultPng(AlgoHandle construct_handle, const char *out_export_root);
	C_API AlgoError C_CALL CabinetConstructExportResultMat(AlgoHandle construct_handle, AlgoMat *out_export_mat);



	/************************************
	@Brief	：载入和保存
	@Note	：文件名后缀必须是.CabinetConstruct
			  存储内容包括:屏幕信息，
	@param	: construct_handle,		输入量, 有效的构造句柄
	@param	: in_fullname,			输入量, 文件名后缀必须是.CabinetConstruct
	@return : AlgoError
	@Author : songshaojun@2026/01/30
	************************************/
	C_API AlgoError C_CALL CabinetConstructLoad(AlgoHandle construct_handle, const char* in_fullname);
	C_API AlgoError C_CALL CabinetConstructSave(AlgoHandle construct_handle, const char* in_fullname);



	/************************************
	@brief  : 获取buffer大小
	@note   :
	@param	: construct_handle,		输入量, 有效的构造句柄
	@param  : out_len, 输出量, buffer大小
	@return	: AlgoError
	@author : songshaojun@2025/06/25
	************************************/
	C_API AlgoError C_CALL CabinetConstructGetBufferSize(AlgoHandle construct_handle, uint64_t* out_len);


	/************************************
	@brief  : 将handle里面的数据导出到buffer
	@note   :
	@param	: construct_handle,		输入量, 有效的构造句柄
	@param  : out_buffer			已经申请好申请空间
	@param  : in_out_len			外部申请好的空间大小,会用真实大小修改
	@return	: AlgoError
	@author : songshaojun@2025/06/25
	************************************/
	C_API AlgoError C_CALL CabinetConstructToBuffer(AlgoHandle construct_handle, char* out_buffer, uint64_t* in_out_len);


	/************************************
	@brief  : 将buffer数据导入到handle
	@note   :
	@param	: construct_handle,		输入量, 有效的构造句柄
	@param  : in_buffer
	@param  : in_len
	@return	: AlgoError
	@author : songshaojun@2025/06/25
	************************************/
	C_API AlgoError C_CALL CabinetConstructFromBuffer(AlgoHandle construct_handle, const char* in_buffer, uint64_t in_len);



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
    C_API AlgoError C_CALL CabinetConstructRelease(AlgoHandle construct_handle);
    C_API void C_CALL CabinetConstructDestroyHandle(AlgoHandle construct_handle);


#ifdef __cplusplus
}
#endif
#endif