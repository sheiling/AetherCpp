#ifndef __LED_METER_INSPECTOR_H__
#define __LED_METER_INSPECTOR_H__

#include "ci_data.h"
#include "ci_declare.h"
#include "meta_info_data.h"
#include "led_meter_inspector_data.h"
#include "led_meter_inspector_error.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	/************************************
	@brief  : 创建MeterInspect句柄
	@note   :
	@param  : out_inspector_handle
	@param  : in_method  默认0x4440
	@return : AlgoError
	@Author : lipengpeng@2021/07/13
	************************************/
	C_API AlgoError C_CALL MeterInspectCreateHandle(AlgoHandle* out_inspector_handle, int in_method = 0x4440);

	/************************************
	@brief  : 死灯检测
	@note   : 检测LED屏幕上不亮的灯点:纯色图(R,G,B)
	@param  : inspector_handle  有效的inspector句柄,不能为空
	@param  : in_meter_handle  输入光通量提取结果句柄,点定位后
	@return : AlgoError
	@Author : lipengpeng@2022/02/28
	************************************/
	C_API AlgoError C_CALL MeterInspectDeadLed(AlgoHandle inspector_handle, const AlgoHandle in_meter_handle);

	/************************************
	@brief  : 暗灯检测
	@note   : 检测LED屏幕上过暗的灯点:纯色图(R,G,B)
	@param  : inspector_handle  有效的inspector句柄,不能为空
	@param  : in_meter_handle  输入光通量提取结果句柄,点定位后
	@return : AlgoError
	@Author : lipengpeng@2022/02/28
	************************************/
	C_API AlgoError C_CALL MeterInspectDarkLed(AlgoHandle inspector_handle, const AlgoHandle in_meter_handle);

	/************************************
	@brief  : 亮灯检测
	@note   : 检测LED屏幕上过亮的灯点:纯色图(R,G,B)
	@param  : inspector_handle  有效的inspector句柄,不能为空
	@param  : in_meter_handle  输入光通量提取结果句柄,点定位后
	@return : AlgoError
	@Author : lipengpeng@2022/02/28
	************************************/
	C_API AlgoError C_CALL MeterInspectBrightLed(AlgoHandle inspector_handle, const AlgoHandle in_meter_handle);

	/************************************
	@brief  : 连续暗灯连续亮灯检测
	@note   : 检测LED屏幕上过亮的灯点:纯色图(R,G,B)
	@param  : inspector_handle  有效的inspector句柄,不能为空
	@param  : in_meter_handle  输入光通量提取结果句柄,点定位后
	@return : AlgoError
	@Author : lipengpeng@2023/05/25
	************************************/
	C_API AlgoError C_CALL MeterInspectContinuousLed(AlgoHandle inspector_handle, const AlgoHandle in_meter_handle);

	/************************************
	@brief  : 黑屏检测
	@note   : 检测LED屏幕异常不亮：纯色图(R,G,B)
	@param  : inspector_handle  有效的inspector句柄,不能为空
	@param  : in_shot_handle_ori  原图输入shot句柄,定位前   详见algo_shot.h中定义
	@param  : in_shot_handle_black  黑图输入shot句柄,定位前  详见algo_shot.h中定义
	@param  : out_flag  输出是否黑屏标志值：0：不是黑屏 1:是黑屏
	@return : AlgoError
	@Author : lipengpeng@2021/07/13
	************************************/
	C_API AlgoError C_CALL MeterInspectBlackScreen(AlgoHandle inspector_handle, const AlgoHandle in_shot_handle_ori,
		const AlgoHandle in_shot_handle_black, int* out_flag);

	/************************************
	@brief  : 串灯检测:串死、串活
	@note   : 检测LED屏幕是否串灯:斜线纯色图(R,G,B)-建议斜扫画面为棋盘格形式
	@param  : inspector_handle  有效的inspector句柄,不能为空
	@param  : in_slash_meter_handle_1  输入斜扫画面1光通量提取结果句柄,点定位后
	@param  : in_slash_meter_handle_2  输入斜扫画面2光通量提取结果句柄,点定位后
	@param  : type  打屏Pattern方式
	@param  : step  步长
	@return : AlgoError
	@Author : lipengpeng@2022/02/28
	************************************/
	C_API AlgoError C_CALL MeterInspectLedMap(AlgoHandle inspector_handle, const AlgoHandle in_slash_meter_handle_1,
		const AlgoHandle in_slash_meter_handle_2, LedMapType type, int step);


	/************************************
	@brief  : 串灯检测:串死、串活
	@note   : 检测背光灯板是否有串死串活现象
	@param  : inspector_handle  有效的inspector句柄,不能为空
	@param  : in_slash_meter_handle_1  输入斜扫画面1光通量提取结果句柄,点定位后
	@param  : in_slash_meter_handle_2  输入斜扫画面2光通量提取结果句柄,点定位后
	@param  : type  打屏Pattern方式
	@param  : step  步长,默认设置为2
	@param  : grid_width   棋盘格中每个小格子的宽度
	@param  : grid_height  棋盘格的宽度小格子的高度
	@return : AlgoError
	@Author : songshaojun@2024/03/22
	************************************/
	C_API AlgoError C_CALL MeterInspectLedMapForBackLight(AlgoHandle inspector_handle, const AlgoHandle in_slash_meter_handle_1,
		const AlgoHandle in_slash_meter_handle_2, LedMapType type, int step,int grid_width,int grid_height);




	/************************************
	@brief  : 串色（错色）
	@note   : 检测LED屏幕是否串色:纯色图(R,G,B:串色)-不支持混色检测
	@param  : inspector_handle  有效的inspector句柄,不能为空
	@param  : in_meter_handle_R  输入R画面光通量提取结果句柄,点定位后
	@param  : in_meter_handle_G  输入G画面光通量提取结果句柄,点定位后
	@param  : in_meter_handle_B  输入B画面光通量提取结果句柄,点定位后
	@return : AlgoError
	@Author : lipengpeng@2022/02/28
	************************************/
	C_API AlgoError C_CALL MeterInspectColorVariation(AlgoHandle inspector_handle, const AlgoHandle in_meter_handle_R,
		const AlgoHandle in_meter_handle_G, const AlgoHandle in_meter_handle_B);

	/************************************
	@brief  : 色偏检测
	@note   : 检测LED灯点是否存在色偏:纯色图(R,G,B,W:色偏)
	@param  : inspector_handle  有效的inspector句柄,不能为空
	@param  : in_meter_handle_X  输入X通道光通量提取结果句柄,点定位后
	@param  : in_meter_handle_Y  输入X通道光通量提取结果句柄,点定位后
	@param  : in_meter_handle_Z  输入X通道光通量提取结果句柄,点定位后
	@return : AlgoError
	@Author : lipengpeng@2024/01/18
	************************************/
	C_API AlgoError C_CALL MeterInspectColorCast(AlgoHandle inspector_handle, const AlgoHandle in_meter_handle_X,
		const AlgoHandle in_meter_handle_Y, const AlgoHandle in_meter_handle_Z);

	/************************************
	@brief  : 亮度异常检测
	@note   : 检测LED屏幕亮度差异，将亮度差异大的灯板检测出来
	@param  : inspector_handle    有效的inspector句柄,不能为空
	@param  : in_meter_handle     输入光通量提取结果句柄,点定位后
	@param  : out_lum             输出灯板整体亮度值
	@param  : out_flag            输出是否亮度异常：0：亮度正常 1:亮度异常
	@return : AlgoError
	@Author : lipengpeng@2023/02/27
	************************************/
	C_API AlgoError C_CALL MeterInspectLumAbnormal(AlgoHandle inspector_handle, const AlgoHandle in_meter_handle,
		float* out_lum, int* out_flag);
		
	/************************************
	@brief  : 设置混色检测阈值
	@note   : 
	@param  : inspector_handle  有效的inspector句柄,不能为空
	@param  : mix_color_thresh  混色检测的阈值，详情请见led_meter_inspector_data.h中MeterInspectMixColorThresh结构体
	@return : AlgoError
	@Author : songshaojun@2024/01/25
	************************************/
	C_API AlgoError C_CALL MeterInspectSetMixColorThresh(AlgoHandle inspector_handle, const MeterInspectMixColorThresh *mix_color_thresh);

	/************************************
	@brief  : 获取混色检测阈值
	@note   : 
	@param  : inspector_handle       有效的inspector句柄,不能为空
	@param  : out_mix_color_thresh   返回的混色检测阈值
	@return : AlgoError
	@Author : songshaojun@2024/01/25
	************************************/
	C_API AlgoError C_CALL MeterInspectGetMixColorThresh(AlgoHandle inspector_handle, MeterInspectMixColorThresh *out_mix_color_thresh);

	/************************************
	@brief  : 混色检测主接口
	@note   : 检测LED屏幕是否存在混色，例如：亮蓝色画面，灯板上某点蓝色灯和绿色灯同时亮起；
	@param  : inspector_handle  有效的inspector句柄,不能为空
	@param  : in_meter_handle_R  输入检测画面R通道Meter结果句柄
	@param  : in_meter_handle_G  输入检测画面G通道Meter结果句柄
	@param  : in_meter_handle_B  输入检测画面B通道Meter结果句柄
	@return : AlgoError
	@Author : songshaojun@2024/01/25
	************************************/
	C_API AlgoError C_CALL MeterInspectMixColor(AlgoHandle inspector_handle, const AlgoHandle in_meter_handle_R,
		const AlgoHandle in_meter_handle_G, const AlgoHandle in_meter_handle_B);	
			
	/************************************
	@brief  : 阈值设定
	@note   : 阈值设定
	@param  : inspector_handle  有效的inspector句柄,不能为空
	@param  : in_thresh  输入的阈值
	@return : AlgoError
	@Author : lipengpeng@2021/07/13
	************************************/
	C_API AlgoError C_CALL MeterInspectSetThresh(AlgoHandle inspector_handle, const MeterInspectThresh* in_thresh);

	/************************************
	@brief  : 阈值输出
	@note   : 阈值输出
	@param  : inspector_handle  有效的inspector句柄,不能为空
	@param  : out_thresh  输出的阈值
	@return : AlgoError
	@Author : lipengpeng@2021/11/03
	************************************/
	C_API AlgoError C_CALL MeterInspectGetThresh(AlgoHandle inspector_handle, MeterInspectThresh* out_thresh);

	/************************************
	@brief  : 模块内部数据
	@note   : 图像数据：每个检测项完成后可调用此接口，获取对应的检测数据，比值系数与阈值紧密相关
	@param  : inspector_handle  有效的inspector句柄,不能为空
	@param  : out_inner_data  输出图像数据：一、死暗亮灯：0通道：检测模板数据；1通道：原图数据；2通道：原图数据与检测模板数据比值-阈值调节参考。
										   二、连续暗灯、连续亮灯：0通道：检测模板数据；2、原始通道数据；3、原图数据与检测模板数据比值。
										   三、串色：红色画面：0、1、2分别为R、G、B通道比值数据；绿色画面：0、1、2分别为G、B、R通道比值数据；红色画面：0、1、2分别为B、G、R通道比值数据；
										   四、串灯：0通道：斜扫画面1比值数据；1通道：斜扫画面2比值数据；
	@param  : is_deep_copy  是否深拷贝, 默认浅拷贝  1=深拷贝;0=浅拷贝
	@return : AlgoError
	@Author : lipengpeng@2021/11/03
	************************************/
	C_API AlgoError C_CALL MeterInspectGetInnerData(AlgoHandle inspector_handle, AlgoMat* out_inner_data, int in_is_deep_copy = 0);

	/************************************
	@brief  : 获取图像检测结果数据
	@note   : 检测结果Mat
	@param  : inspector_handle  有效的inspector句柄,不能为空
	@param  : out_result_mat  输出检测结果数据Mat
	@return : AlgoError
	@Author : lipengpeng@2022/02/28
	************************************/
	C_API AlgoError C_CALL MeterInspectGetResultMat(AlgoHandle inspector_handle, AlgoMat* out_result_mat);

	/************************************
	@brief  : 图像检测结果Mat数据转List
	@note   : 检测结果list,具有优先级功能
	@param  : inspector_handle  有效的inspector句柄,不能为空
	@param  : in_filter_defect_type  输入想要的缺陷标识符：示例1.比如输入串色标识符，则获取串色缺陷的结果
	@param  : out_defect_loc_list  输出缺陷灯点位置List
	@param  : out_defect_num  输出缺陷灯点数量
	@param  : in_ignore_defect_type  输入不想要的缺陷标识符,默认值LED_DEFECT_FLAG_UNINIT：示例2.比如输入死灯标识符，则既是死灯又是串色的结果忽略（示例1.2.对应）
	@return : AlgoError
	@Author : lipengpeng@2022/03/18
	************************************/
	C_API AlgoError C_CALL MeterInspectGetResultList(AlgoHandle inspector_handle, uint32_t in_filter_defect_type,
		AlgoPoint* out_defect_loc_list, int* out_defect_num, uint32_t in_ignore_defect_type = LED_DEFECT_FLAG_UNINIT);

	/************************************
	@brief  : 缺陷芯片位置编号输出
	@note   : 通过缺陷灯点找到对应缺陷芯片位置
	@param  : inspector_handle  有效的inspector句柄,不能为空
	@param  : in_defect_led_loc_list  输入缺陷灯点位置坐标：一般为连续性或大面积缺陷
	@param  : in_defect_led_num  输入缺陷灯点数量
	@param  : in_chipId_Mat  输入灯板芯片位置矩阵
	@param  : out_defect_chip_id_list  输出缺陷芯片编号
	@param  : out_defect_chip_num  输出缺陷芯片数量
	@return : AlgoError
	@Author : lipengpeng@2022/03/18
	************************************/
	C_API AlgoError C_CALL MeterInspectGetDefectChipId(AlgoHandle inspector_handle, const AlgoPoint* in_defect_led_loc_list,
		int in_defect_led_num, AlgoMat* in_chipId_Mat, int* out_defect_chip_id_list, int* out_defect_chip_num);

	/************************************
	@brief  : 空间内存释放
	@note   : 句柄指向的对象销毁, 此句柄已经无效, 请配合handle=nullptr
	@param  : inspector_handle
	@return : AlgoError
	@Author : lipengpeng@2021/07/13
	************************************/
	C_API void C_CALL MeterInspectDestroyHandle(AlgoHandle inspector_handle);

	///********************buffer相关的***************************
	/************************************
	@Brief		：	载入和保存MeterInspector的阈值
	@Note		：	文件名后缀必须为.meterinspect
	@Parameter	: 	inspector_handle		---有效的inspector句柄
	@Parameter	: 	in_locator_fullname		---文件后缀必须为.meterinspect
	@Returns	:   AlgoError
	@Author		：	songshaojun@2024/01/29
	************************************/
	C_API AlgoError C_CALL MeterInspectLoad(AlgoHandle inspector_handle, const char* in_meter_inspect_fullname);
	C_API AlgoError C_CALL MeterInspectSave(AlgoHandle inspector_handle, const char* in_meter_inspect_fullname);

	/************************************
	@brief    : 获取检测buffer大小
	@note     :
	@param    : inspector_handle          有效的inspector句柄
	@param    : out_len                   buffer大小
	@return   : AlgoError
	@Author	  ：	songshaojun@2024/01/29
	************************************/
	C_API AlgoError C_CALL MeterInspectGetBufferSize(AlgoHandle inspector_handle, uint64_t* out_len);

	/************************************
	@brief    : 将MeterInspector里面的数据导出到buffer
	@note     :
	@param    : inspector_handle       有效的inspector句柄
	@param    : out_buffer             已经申请好申请空间
	@param    : in_out_len             外部申请好的空间大小,会用真实大小修改
	@return   : AlgoError
	@Author	  ：	songshaojun@2024/01/29
	************************************/
	C_API AlgoError C_CALL MeterInspectToBuffer(AlgoHandle inspector_handle, char* out_buffer, uint64_t* in_out_len);

	/************************************
	@brief    : 将buffer数据导入到MeterInspector
	@note     :
	@param    : inspector_handle          有效的inspector句柄
	@param    : in_buffer
	@param    : in_len
	@return   : AlgoError
	@Author	  ：	songshaojun@2024/01/29
	************************************/
	C_API AlgoError C_CALL MeterInspectFromBuffer(AlgoHandle inspector_handle, const char* in_buffer, uint64_t in_len);
	
	/************************************
	@brief  : 缺陷融合算法
	@note   : 五个相机拍照检测融合算法
	@param  : inspector_handle  有效的inspector句柄,不能为空
	@param  : in_meter_handle  输入光通量提取结果句柄,点定位后
	@param  : method  算法融合策略：1：所有拍照检测结果取“并集”；2：每个相机拍照检测结果取部分结果，然后所有结果取“并集”
	@param  : in_led_vertical_list  正着拍照检测结果
	@param  : in_vertical_num  正着拍检测缺陷灯点个数
	@param  : in_led_long_edge_front_list  长边侧面拍照1检测结果
	@param  : in_long_side_front_num  长边侧面拍照1检测缺陷灯点个数
	@param  : in_led_long_edge_back_list  长边侧面拍照2检测结果
	@param  : in_long_side_back_num  长边侧面拍照2检测缺陷灯点个数
	@param  : in_led_short_edge_left_list  短边侧面拍照1检测结果
	@param  : in_short_side_left_num  短边侧面拍照1检测缺陷灯点个数
	@param  : in_led_short_edge_right_list  短边侧面拍照2检测结果
	@param  : in_short_side_right_num  短边侧面拍照2检测缺陷灯点个数
	@param  : out_led_merge_list  缺陷融合结果
	@param  : out_merge_num
	@return : AlgoError
	@Author : lipengpeng@2021/08/06
	************************************/
	C_API AlgoError C_CALL MeterInspectMerge(AlgoHandle inspector_handle, const AlgoHandle in_meter_handle,
		int method, const AlgoPoint* in_led_vertical_list, int in_vertical_num, const AlgoPoint* in_led_long_edge_front_list,
		int in_long_side_front_num, const AlgoPoint* in_led_long_edge_back_list, int in_long_side_back_num,
		const AlgoPoint* in_led_short_edge_left_list, int in_short_side_left_num, const AlgoPoint* in_led_short_edge_right_list,
		int in_short_side_right_num, AlgoPoint* out_led_merge_list, int* out_merge_num);

	/************************************
	@brief  : 连续大面积缺陷灯点
	@note   : 连续大面积缺陷灯点公共接口
	@param  : inspector_handle  有效的inspector句柄,不能为空
	@param  : in_meter_handle  输入光通量提取结果句柄,点定位后
	@param  : in_defect_led_loc_list  输入缺陷检测结果列表
	@param  : in_defect_num  输入缺陷灯点个数
	@param  : out_bigArea_led_loc_list  输出连续大面积缺陷灯点位置信息列表
	@param  : out_bigArea_num  输出连续大面积缺陷灯点个数
	@return : AlgoError
	@Author : lipengpeng@2021/08/06
	************************************/
	C_API AlgoError C_CALL MeterInspectContinuousBigArea(AlgoHandle inspector_handle, const AlgoHandle in_meter_handle,
		const AlgoPoint* in_defect_led_loc_list, int in_defect_num, AlgoPoint* out_bigArea_led_loc_list, int* out_bigArea_num);

	/************************************
	@brief  : 黑屏亮灯检测
	@note   : 检测LED屏幕0灰阶时常亮的灯点
	@param  : inspector_handle  有效的inspector句柄,不能为空
	@param  : in_meter_handle  输入光通量提取结果句柄,点定位后
	@param  : out_lights_led_loc_list  输出亮灯灯点位置
	@param  : out_num  输出亮灯灯点个数
	@return : AlgoError
	@Author : lipengpeng@2021/12/21
	************************************/
	C_API AlgoError C_CALL MeterInspectBlackScreenLights(AlgoHandle inspector_handle, const AlgoHandle in_meter_handle,
		AlgoPoint* out_lights_led_loc_list, int* out_num);

	/************************************
	@brief  : 灯点结果原图标注
	@note   : 灯点结果原图标注公共接口
	@param  : inspector_handle  有效的inspector句柄,不能为空
	@param  : in_img_fullname  图像文件名
	@param  : in_src_shot_handle  有效的原图shot句柄
	@param  : in_shot_handle  有效的G通道shot句柄
	@param  : in_ledpoint_handle  有效的ledpoint句柄
	@param  : in_result_led_loc_list  输入缺陷检测结果列表
	@param  : in_result_num  输入缺陷检测结果数量
	@param  : draw_type  缺陷标注类型(0=circle,4=rectangle)
	@param  : in_color  缺陷标注颜色(红绿蓝)
	@param  : in_scale  放大倍数
	@return : AlgoError
	@Author : lipengpeng@2021/12/29
	************************************/
	C_API AlgoError C_CALL MeterInspectDrawResultImage(AlgoHandle inspector_handle, const char* in_img_fullname,
		const AlgoHandle in_src_shot_handle, const AlgoHandle in_shot_handle, const AlgoHandle in_ledpoint_handle,
		const AlgoPoint* in_result_led_loc_list, int in_result_num, int draw_type, AlgoColor in_color, int in_scale);

	//沃格背光检测结果合并
	/************************************
	@brief  : 设置结果合并信息
	@note   : 适用于沃格背光检测设备，单张灯板分次拍摄，最终将所有检测结果合并
	@param  : inspector_handle  有效的inspector句柄,不能为空
	@param  : in_merge_info     输入的合并信息，详情请见MerterINspectMergeInfo结构体
	@return : AlgoError
	@Author : songshaojun@2024/02/03
	************************************/
	C_API AlgoError C_CALL MeterInspectSetMergeInfo(AlgoHandle inspector_handle, const MeterInspectMergeInfo *in_merge_info);

	/************************************
	@brief  : 合并检测结果
	@note   : 所有样本检测完毕后，再调用该接口进行merge
	@param  : inspector_handle  有效的inspector句柄,不能为空
	@param  : in_all_result     输入值，所有样本检测结果
	@param  : in_result_num     输入值，共有几张检测样本结果
	@param  : out_merge_result  输出值，合并后的检测结果
	@return : AlgoError
	@Author : songshaojun@2024/02/03
	************************************/
	C_API AlgoError C_CALL MeterInspectGetMergeResult(AlgoHandle inspector_handle, const AlgoMat *in_all_result,
		const int in_result_num, AlgoMat *out_merge_result);

	/************************************
	@brief  : 句柄指向的对象的内存空间释放, 句柄指向的对象还在
	@note   : 内存释放
	@param  : release_handle
	@return : AlgoError
	@Author : lipengpeng@2021/08/11
	************************************/
	C_API AlgoError C_CALL MeterInspectRelease(AlgoHandle release_handle);

	/************************************
	@brief  : 光通量融合Mask
	@note   : 融合mask创建
	@param  : inspector_handle  有效的inspector句柄,不能为空
	@param  : in_slash_meter_handle_1  输入第1幅斜扫画面光通量meter,点定位后
	@param  : type  打屏Pattern方式
	@param  : step  步长
	@param  : out_mask  代表第2幅斜扫画面正常亮灯点mask
	@return : AlgoError
	@Author : lipengpeng@2022/02/24
	************************************/
	C_API AlgoError C_CALL MeterInspectCreateMeterMask(AlgoHandle inspector_handle, const AlgoHandle in_slash_meter_handle_1,
		LedMapType type, int step, AlgoMat* out_mask);


#ifdef __cplusplus
}
#endif // __cplusplus
#endif