#ifndef __LED_COLORED_LED_INSPECTOR_H__
#define __LED_COLORED_LED_INSPECTOR_H__


#include "ci_define.h"
#include "ci_declare.h"
#include "colored_led_inspector_data.h"
#include "colored_led_inspector_errors.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	/************************************
	@brief  : 创建ColoredLed句柄
	@note   :
	@param  : in_colored_led_handle			--有效的colored_led句柄，不能为空
	@return : AlgoError
	@Author : zhongao@2024/12/19
	************************************/
	C_API AlgoError C_CALL ColoredLedCreateHandle(AlgoHandle* in_colored_led_handle);
	
	/************************************
	@brief  : 设定待检测灯板的宽高
	@note   :
	@param  : in_colored_led_handle			--有效的colored_led句柄，不能为空
	@param  : in_panel_resolution			--输入待检测灯板的分辨率，其中
											  in_panel_resolution.width为灯板的宽，in_panel_resolution.height为灯板的高
	@return : AlgoError
	@Author : zhongao@2024/12/19
	************************************/
	C_API AlgoError C_CALL ColoredLedSetPanelResolution(AlgoHandle in_colored_led_handle, AlgoSize& in_panel_resolution);

	/************************************
	@brief  : 设定待检测灯板的排布方式
	@note   :
	@param  : in_colored_led_handle			--有效的colored_led句柄，不能为空
	@param  : in_layout						--输入待检测灯板的排布方式，不能为LED_LAYOUT_UNDEFINED
	@return : AlgoError
	@Author : zhongao@2025/01/03
	************************************/
	C_API AlgoError C_CALL ColoredLedSetLedLayout(AlgoHandle in_colored_led_handle, LedLayout& in_layout);

	/************************************
	@brief  : 设定灯点中心在图像的像素间隔
	@note   :
	@param  : in_colored_led_handle			--有效的colored_led句柄，不能为空
	@param  : in_pixel_dotpitch				--输入相邻灯点的像素点间距，其中
											  in_pixel_dotpitch.width为相邻灯点x方向的点间距，in_pixel_dotpitch.height为相邻灯点y方向的点间距
	@return : AlgoError
	@Author : zhongao@2024/12/19
	************************************/
	C_API AlgoError C_CALL ColoredLedSetPixelDotPitch(AlgoHandle in_colored_led_handle, AlgoSize2f& in_pixel_dotpitch);

	/************************************
	@brief  : 设定相机中心的在机械坐标系下的空间坐标
	@note   :
	@param  : in_colored_led_handle			--有效的colored_led句柄，不能为空
	@param  : in_spatial_coords				--输入相机中心的在机械坐标系下的空间坐标
	@return : AlgoError
	@Author : zhongao@2024/12/23
	************************************/
	C_API AlgoError C_CALL ColoredLedSetSpatialCoords(AlgoHandle in_colored_led_handle, AlgoPoint2f& in_spatial_coords);

	/************************************
	@brief  : 设定图像与机械坐标系的映射矩阵
	@note   : 根据mark点在图像与机械坐标系的运动向量计算机械坐标映射矩阵
	@param  : in_colored_led_handle			--有效的colored_led句柄，不能为空
	@param  : in_vector_img					--输入mark点在图像坐标系下的运动向量((x_img_1-x_img_0) , (y_img_1-y_img_0))
	@param  : in_vector_mach				--输入相机中心在机械坐标系下的运动向量((x_mach_1-x_mach_0) , (y_mach_1-y_mach_0))，未强制要求量纲
	@return : AlgoError
	@Author : zhongao@2024/12/19
	************************************/
	C_API AlgoError C_CALL ColoredLedSetMappingMatrix(AlgoHandle in_colored_led_handle, const AlgoPoint2f& in_vector_img, const AlgoPoint2f& in_vector_mach);

	/************************************
	@brief  : 全彩化图像按照HSV分割出红绿蓝灰度shot图像
	@note   :
	@param  : in_colored_led_handle			--有效的colored_led句柄，不能为空
	@param  : in_shot_handle				--输入原始全彩化图像shot句柄
	@param  : out_shot_red_handle			--输出HSV分割后的红色灰度图像shot句柄
	@param  : out_shot_green_handle			--输出HSV分割后的绿色灰度图像shot句柄
	@param  : out_shot_blue_handle			--输出HSV分割后的蓝色灰度图像shot句柄
	@return : AlgoError
	@Author : zhongao@2024/12/23
	************************************/
	C_API AlgoError C_CALL ColoredLedSplitColor(AlgoHandle in_colored_led_handle, AlgoHandle in_shot_handle, AlgoHandle out_shot_red_handle, AlgoHandle out_shot_green_handle, AlgoHandle out_shot_blue_handle);

	/************************************
	@brief  : 推断图像中灯板的分辨率
	@note   :
	@param  : in_colored_led_handle			--有效的colored_led句柄，不能为空
	@param  : in_shot_handle				--输入单色shot图像句柄
	@param  : in_out_point_finder_handle	--输入输出滤除边缘灯点后的point_finder句柄
	@param  : out_wh						--输出局部图像中滤除边缘灯点后的灯点分辨率wh
	@return : AlgoError
	@Author : zhongao@2024/12/23
	************************************/
	C_API AlgoError C_CALL ColoredLedGetResolution(AlgoHandle in_colored_led_handle, AlgoHandle in_shot_handle, AlgoHandle in_out_point_finder_handle, AlgoSize& out_wh);

	/************************************
	@brief  : 将局部单色图像的光通量与缺陷检测结果存入本模块中
	@note   :
	@param  : in_colored_led_handle			--有效的colored_led句柄，不能为空
	@param  : in_mapper_handle				--输入point_mapper句柄
	@param  : in_meter_handle				--输入led_meter句柄
	@param  : in_inspector_handle			--输入led_meter_inspector句柄
	@param  : in_camera_center				--输入当前视野下相机的物理坐标中心
	@param  : in_color						--输入当前颜色通道
	@return : AlgoError
	@Author : zhongao@2024/12/19
	************************************/
	C_API AlgoError C_CALL ColoredLedSaveResult(AlgoHandle in_colored_led_handle, AlgoHandle in_mapper_handle, AlgoHandle in_meter_handle, AlgoHandle in_inspector_handle, AlgoColor in_color);

	/************************************
	@brief  : 将所有局部图像的光通量、缺陷检测结果进行拼接
	@note   : 1）该接口调用需要保证所有局部图像结果都计算完毕并存入本模块中
			  2）要求局部图像将整个待检测灯板拍全，相邻局部图像间重叠行列数在3-5行/列之间
	@param  : in_colored_led_handle			--有效的colored_led句柄，不能为空
	@return : AlgoError
	@Author : zhongao@2024/12/19
	************************************/
	C_API AlgoError C_CALL ColoredLedMergeResult(AlgoHandle in_colored_led_handle);

	/************************************
	@brief  : 按照缺陷类型输出红色灯点的灯点位置并存放在List中
	@note   : 
	@param  : in_colored_led_handle			--有效的colored_led句柄，不能为空
	@param  : in_defect_type				--输入的缺陷类型
	@param  : out_defect_loc_list			--输出对应缺陷类型的位置列表
	@param  : out_defect_num				--输出对应缺陷类型的个数
	@return : AlgoError
	@Author : zhongao@2024/12/23
	************************************/
	C_API AlgoError C_CALL ColoredLedGetRedResultList(AlgoHandle in_colored_led_handle, uint8_t in_defect_type, AlgoPoint * out_defect_loc_list, int * out_defect_num);

	/************************************
	@brief  : 按照缺陷类型输出绿色灯点的灯点位置并存放在List中
	@note   :
	@param  : in_colored_led_handle			--有效的colored_led句柄，不能为空
	@param  : in_defect_type				--输入的缺陷类型
	@param  : out_defect_loc_list			--输出对应缺陷类型的位置列表
	@param  : out_defect_num				--输出对应缺陷类型的个数
	@return : AlgoError
	@Author : zhongao@2024/12/23
	************************************/
	C_API AlgoError C_CALL ColoredLedGetGreenResultList(AlgoHandle in_colored_led_handle, uint8_t in_defect_type, AlgoPoint * out_defect_loc_list, int * out_defect_num);

	/************************************
	@brief  : 按照缺陷类型输出蓝色灯点的灯点位置并存放在List中
	@note   :
	@param  : in_colored_led_handle			--有效的colored_led句柄，不能为空
	@param  : in_defect_type				--输入的缺陷类型
	@param  : out_defect_loc_list			--输出对应缺陷类型的位置列表
	@param  : out_defect_num				--输出对应缺陷类型的个数
	@return : AlgoError
	@Author : zhongao@2024/12/23
	************************************/
	C_API AlgoError C_CALL ColoredLedGetBlueResultList(AlgoHandle in_colored_led_handle, uint8_t in_defect_type, AlgoPoint * out_defect_loc_list, int * out_defect_num);

	/************************************
	@brief  : 载入和保存ColoredLed的检测参数
	@note   : 
	@param  : in_colored_led_handle			--有效的colored_led句柄，不能为空
	@param  : in_fullname					--文件名后缀必须为.whiteinspect
	@return : AlgoError
	@Author : zhongao@2024/12/19
	************************************/
	C_API AlgoError C_CALL ColoredLedSave(AlgoHandle in_colored_led_handle, const char* in_fullname);
	C_API AlgoError C_CALL ColoredLedLoad(AlgoHandle in_colored_led_handle, const char* in_fullname);

	/************************************
	@brief  : 绘制并保存拼接后的死暗亮灯缺陷检测结果图
	@note   : 该接口会在对应的文件夹路径下生成以下可视化图像
				r_lum_dead.png r_lum_dark.png r_lum_bright.png
				g_lum_dead.png g_lum_dark.png g_lum_bright.png
				b_lum_dead.png r_lum_dark.png b_lum_bright.png
	@param  : in_colored_led_handle			--有效的colored_led句柄，不能为空
	@param  : in_save_dir					--输入存放图像的文件夹路径，如D:\\test 注意：文件夹后面不用添加其他斜线
	@return : AlgoError
	@Author : zhongao@2024/12/19
	************************************/
	C_API AlgoError C_CALL ColoredLedExportResultImg(AlgoHandle in_colored_led_handle, const char* in_save_dir);

	/************************************
	@brief  : 获取红灯全灯板拼接后的光通量Mat
	@note   :
	@param  : in_colored_led_handle			--有效的colored_led句柄，不能为空
	@param  : out_lum_mat					--输出红灯全灯板拼接后的光通量Mat
	@return : AlgoError
	@Author : zhongao@2024/12/19
	************************************/
	C_API AlgoError C_CALL ColoredLedGetRedLumMat(AlgoHandle in_colored_led_handle, AlgoMat* out_lum_mat);

	/************************************
	@brief  : 获取绿灯全灯板拼接后的光通量Mat
	@note   :
	@param  : in_colored_led_handle			--有效的colored_led句柄，不能为空
	@param  : out_lum_mat					--输出绿灯全灯板拼接后的光通量Mat
	@return : AlgoError
	@Author : zhongao@2024/12/19
	************************************/
	C_API AlgoError C_CALL ColoredLedGetGreenLumMat(AlgoHandle in_colored_led_handle, AlgoMat* out_lum_mat);

	/************************************
	@brief  : 获取蓝灯全灯板拼接后的光通量Mat
	@note   :
	@param  : in_colored_led_handle			--有效的colored_led句柄，不能为空
	@param  : out_lum_mat					--输出蓝灯全灯板拼接后的光通量Mat
	@return : AlgoError
	@Author : zhongao@2024/12/19
	************************************/
	C_API AlgoError C_CALL ColoredLedGetBlueLumMat(AlgoHandle in_colored_led_handle, AlgoMat* out_lum_mat);

	/************************************
	@brief  : 获取红灯全灯板拼接后的缺陷检测结果Mat
	@note   :
	@param  : in_colored_led_handle			--有效的colored_led句柄，不能为空
	@param  : out_result_mat				--输出红灯全灯板拼接后的缺陷检测结果Mat
	@return : AlgoError
	@Author : zhongao@2024/12/19
	************************************/
	C_API AlgoError C_CALL ColoredLedGetRedResultMat(AlgoHandle in_colored_led_handle, AlgoMat* out_result_mat);

	/************************************
	@brief  : 获取绿灯全灯板拼接后的缺陷检测结果Mat
	@note   :
	@param  : in_colored_led_handle			--有效的colored_led句柄，不能为空
	@param  : out_result_mat				--输出绿灯全灯板拼接后的缺陷检测结果Mat
	@return : AlgoError
	@Author : zhongao@2024/12/19
	************************************/
	C_API AlgoError C_CALL ColoredLedGetGreenResultMat(AlgoHandle in_colored_led_handle, AlgoMat* out_result_mat);

	/************************************
	@brief  : 获取蓝灯全灯板拼接后的缺陷检测结果Mat
	@note   :
	@param  : in_colored_led_handle			--有效的colored_led句柄，不能为空
	@param  : out_result_mat				--输出蓝灯全灯板拼接后的缺陷检测结果Mat
	@return : AlgoError
	@Author : zhongao@2024/12/19
	************************************/
	C_API AlgoError C_CALL ColoredLedGetBlueResultMat(AlgoHandle in_colored_led_handle, AlgoMat* out_result_mat);

	/************************************
	@brief  : 获取buffer大小
	@note   : 
	@param  : in_colored_led_handle			--有效的colored_led句柄，不能为空
	@param  : out_len						--输出buffer大小
	@return : AlgoError
	@Author : zhongao@2024/12/19
	************************************/
	C_API AlgoError C_CALL ColoredLedGetBufferSize(AlgoHandle in_colored_led_handle, uint64_t* out_len);

	/************************************
	@brief  : 获取buffer大小
	@note   :
	@param  : in_colored_led_handle			--有效的colored_led句柄，不能为空
	@param  : out_buffer					--外部已经申请好的外部buffer空间
	@param  : out_len						--外部申请好的buffer空间大小，会根据模块的真实大小进行修改
	@return : AlgoError
	@Author : zhongao@2024/12/19
	************************************/
	C_API AlgoError C_CALL ColoredLedToBuffer(AlgoHandle in_colored_led_handle, char* out_buffer, uint64_t* out_len);

	/************************************
	@brief  : 获取buffer大小
	@note   :
	@param  : in_colored_led_handle			--有效的colored_led句柄，不能为空
	@param  : in_buffer						--导入的外部buffer空间
	@param  : in_len						--导入的申请buffer空间大小
	@return : AlgoError
	@Author : zhongao@2024/12/19
	************************************/
	C_API AlgoError C_CALL ColoredLedFromBuffer(AlgoHandle in_colored_led_handle, char* in_buffer, uint64_t in_len);

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
	C_API AlgoError C_CALL ColoredLedRelease(AlgoHandle in_colored_led_handle);
	C_API void C_CALL ColoredLedDestroyHandle(AlgoHandle in_colored_led_handle);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif