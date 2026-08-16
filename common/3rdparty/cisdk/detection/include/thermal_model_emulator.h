#ifndef __THERMAL_MODEL_EMULATOR_H__
#define __THERMAL_MODEL_EMULATOR_H__

#include "algo_mat.h"
#include "ci_data.h"
#include "ci_declare.h"
#include "meta_info_data.h"
#include "thermal_model_emulator_data.h"
#include "thermal_model_emulator_error.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

	/*********************************************
	@brief   : 创建ThermalModlEmulator句柄
	@note    :
	@param   : locator_handle
	@return  : AlgoError
	@Author  : shimai@2025/03/18
	*********************************************/
	C_API AlgoError C_CALL ThermalCreateHandle(AlgoHandle* emulator_handle);

	
	/*********************************************
	@brief   : 选择箱体上一点，通过输入该点的当前温度及校正时的温度温度拟合当前的热力状态下箱体或拼成的全屏的亮色度
	@note    :
	@param   : emulator_handle      入参：句柄
	@param   : in_color             入参：箱体打屏画面对应颜色
	@param   : in_grayscale         入参：要观察的箱体灰阶
	@param   : in_loc               入参：参考点在箱体上的坐标
	@param   : in_tempeature        入参：参考点温度
	@param   : in_calib_temp        入参：校正时的温度
	@param   : in_calib_type        入参：校正类型，详情参加data.h
	@param   : in_r_xyz_meter       入参：采集到的r的xyz光通量，没有的通道就直接输入空meter就行
	@param   : in_g_xyz_meter       入参：采集到的g的xyz光通量，没有的通道就直接输入空meter就行
	@param   : in_b_xyz_meter       入参：采集到的b的xyz光通量，没有的通道就直接输入空meter就行
	@param   : out_meter_x          出参：输出的x的响应
	@param   : out_meter_y          出参：输出的y的响应 
	@param   : out_meter_z          出参：输出的z的响应
	@param   : full_screen_mode     入参：是否是全屏(如果是查看单箱体效果就选择false，如果输入全屏数据或者输入单箱数据想看拟合全屏效果则选择true)
	@return  : AlgoError
	@Author  : shimai@2025/03/18
	*********************************************/
	C_API AlgoError C_CALL ThermalEmulateLumByTemperature(AlgoHandle emulator_handle, 
		const AlgoColor in_color, const int in_grayscale, const AlgoPoint in_loc, const float in_calib_temp, const float in_tempeature, const ThermalCalibType in_calib_type, 
		const AlgoHandle* in_r_xyz_meter, const AlgoHandle* in_g_xyz_meter, const AlgoHandle* in_b_xyz_meter, 
		AlgoHandle out_meter_x, AlgoHandle out_meter_y, AlgoHandle out_meter_z, bool full_screen_mode = false);



	/*********************************************
    @brief   : 通过输入当前的亮屏时间以及校正时的时间，计算对应状态下箱体/或拼成的全屏的亮色度
    @note    :
    @param   : emulator_handle      入参：句柄
    @param   : in_color             入参：箱体打屏画面对应颜色
    @param   : in_grayscale         入参：要观察的箱体灰阶
    @param   : in_calib_time        入参：校正时的亮屏时间
    @param   : in_time              入参：要查看的亮屏时间
	@param   : in_r_xyz_meter       入参：采集到的r的xyz光通量，没有的通道就直接输入空meter就行
	@param   : in_g_xyz_meter       入参：采集到的g的xyz光通量，没有的通道就直接输入空meter就行
	@param   : in_b_xyz_meter       入参：采集到的b的xyz光通量，没有的通道就直接输入空meter就行
    @param   : out_meter_x          出参：输出的x的响应
    @param   : out_meter_y          出参：输出的y的响应
    @param   : out_meter_z          出参：输出的z的响应
    @param   : full_screen_mode     入参：是否是全屏(如果是查看单箱体效果就选择false，如果输入全屏数据或者输入单箱数据想看拟合全屏效果则选择true)
    @return  : AlgoError
    @Author  : shimai@2025/03/18
    *********************************************/
	C_API AlgoError C_CALL ThermalEmulateLumByTime(AlgoHandle emulator_handle, 
		const AlgoColor in_color, const int in_grayscale, const float in_calib_time, const float in_time, const ThermalCalibType in_calib_type, 
		const AlgoHandle* in_r_xyz_meter, const AlgoHandle* in_g_xyz_meter, const AlgoHandle* in_b_xyz_meter,
		AlgoHandle out_meter_x, AlgoHandle out_meter_y, AlgoHandle out_meter_z, bool full_screen_mode = false);


	/*********************************************
	@brief   : 读入转为AlgoMat格式的热成像仪获取的热力数据，通过手动输入ROI获取箱体在热力图中的位置
	@note    :
	@param   : emulator_handle			 入参：句柄
	@param   : in_thermal_image			 入参：输入的热力图像
	@param   : in_data_num			     入参：输入的热力数据的数量
	@param   : in_rect					 入参：箱体在对应图像中的范围
	@param   : in_process                入参：输入的亮色度是升温的过程中的哪个阶段
	@Author  : shimai@2025/03/18
	*********************************************/
	C_API AlgoError C_CALL ThermalSetThermalDataBySetRoi(AlgoHandle emulator_handle, const AlgoMat* in_thermal_image, const int in_data_num, const AlgoRect in_rect, const ThermalProcessState in_process);


	/*********************************************
	@brief   : 读入转为AlgoMat格式的热成像仪获取的热力数据，通过区域定位模块自动定位箱体所在区域
	@note    :
	@param   : emulator_handle			入参：句柄
	@param   : roi_handle			    入参：区域定位模块句柄
	@param   : in_thermal_image			入参：输入的热力图像
	@param   : in_data_num			    入参：输入的热力数据的数量
	@param   : in_process               入参：输入的亮色度是升温的过程中的哪个阶段
	@Author  : shimai@2025/03/18
	*********************************************/
	C_API AlgoError C_CALL ThermalSetThermalDataByAutoLocate(AlgoHandle emulator_handle, AlgoHandle roi_handle, const AlgoMat* in_thermal_image, const int in_data_num, const ThermalProcessState in_process);


	/*********************************************
	@brief   : 计算箱体上各点的温度随时间变化系数和亮色度随温度的衰减系数
	@note    :
	@param   : emulator_handle			 入参：句柄
	@param   : pointwise_mode			 入参：标志位开关：否使用逐点计算系数
	@Author  : shimai@2025/03/18
	*********************************************/
	C_API AlgoError C_CALL ThermalComputeCoef(AlgoHandle emulator_handle, const bool pointwise_mode = true);

	/*********************************************
	@brief   : 设置算法参数
	@note    :
	@param   : emulator_handle			 入参：句柄
	@param   : in_parameter			     入参：算法内部用到的参数
	@Author  : shimai@2025/03/18
	*********************************************/
	C_API AlgoError C_CALL ThermalSetParameter(AlgoHandle emulator_handle, const ThermalModelParameter in_parameter);
	

	/*********************************************
	@brief   : 设置箱体亮色度
	@note    :
	@param   : emulator_handle			 入参：句柄
	@param   : in_meter_r_xyz_handle	 入参：输入的r的xyz的响应值
	@param   : in_meter_g_xyz_handle	 入参：输入的g的xyz的响应值
	@param   : in_meter_b_xyz_handle	 入参：输入的b的xyz的响应值
	@param   : in_process			     入参：输入的亮色度是升温的过程中的哪个阶段
	@Author  : shimai@2025/03/18
	*********************************************/
	C_API AlgoError C_CALL ThermalSetCabinetLum(AlgoHandle emulator_handle, AlgoHandle* in_meter_r_xyz_handle, AlgoHandle* in_meter_g_xyz_handle, AlgoHandle* in_meter_b_xyz_handles, const ThermalProcessState in_process);

	
	/*********************************************
	@brief   : 获取箱体上某个点在某温度、灰阶状态下能达到温度范围
	@note    :
	@param   : emulator_handle			 入参：句柄
	@param   : in_color			         入参：箱体打屏画面对应颜色
	@param   : in_grayscale			     入参：箱体观察时的灰阶
	@param   : in_loc                    入参：参考点在箱体上的坐标
	@param   : out_temp_min			     出参：对应点在当前状态下的最低温度
	@param   : out_temp_max			     出参：对应点在当前状态下的最高温度
	@Author  : shimai@2025/03/18
	*********************************************/
	C_API AlgoError C_CALL ThermalGetTemperatureRange(AlgoHandle emulator_handle, const AlgoColor in_color, const int in_grayscale, const AlgoPoint in_loc,  float* out_temp_min, float* out_temp_max);
	

	/*********************************************
	@brief   : 获取参考点在某一温度状态下，全屏各个点的温度状态
	@note    :
	@param   : emulator_handle			 入参：句柄
	@param   : in_color			         入参：箱体打屏画面对应颜色
	@param   : in_grayscale			     入参：箱体观察时的灰阶
	@param   : in_loc                    入参：参考点在箱体上的坐标
	@param   : in_tempeature			 入参：对应点当前温度
	@param   : out_temp_state			 出参：当前全屏各点的温度
	@Author  : shimai@2025/03/18
	*********************************************/
	C_API AlgoError C_CALL ThermalGetTempState(AlgoHandle emulator_handle, const AlgoColor in_color, const int in_grayscale, const AlgoPoint in_loc, const float in_tempeature, AlgoMat* out_temp_state);
	

	/*********************************************
	@brief   : 输入一个热力图，输出伪彩色的热力图
	@note    :
	@param   : in_temp_state			 入参：输入的热力状态图
	@param   : out_pseudo_color_img		 出参：输出伪彩色热力状态图
	@param   : out_color_bar             出参：参考点在箱体上的坐标
	@param   : out_min_temp				 出参：对应全屏最低温度
	@param   : out_max_temp				 出参：当前全屏最高温度
	@Author  : shimai@2025/10/10
	*********************************************/
	C_API AlgoError C_CALL ThermalGetPseudoColorTempeatureState(AlgoMat* in_temp_state, AlgoMat* out_pseudo_color_img, AlgoMat* out_color_bar, float* out_min_temp, float* out_max_temp);


	/*********************************************
	@brief   : 获取箱体当前热力状态下温度的不均匀性
	@note    :
	@param   : emulator_handle			 入参：句柄
	@param   : out_temp_nonumiform	     出参：箱体当前状态下温度不均匀性
	@Author  : shimai@2025/03/18
	*********************************************/
	C_API AlgoError C_CALL ThermalGetTempeatureNonuniformity(AlgoHandle emulator_handle, TempeatureNonuniformity* in_out_temp_nonumiform);
	

	/*********************************************
	@brief   : 获取箱体当前状态相较于初始状态的升温的不均匀性
	@note    :
	@param   : emulator_handle			        入参：句柄
	@param   : out_delta_temp_nonumiform		出参：箱体当前状态相较于初始状态的升温的不均匀性
	@Author  : shimai@2025/03/18
	*********************************************/
	C_API AlgoError C_CALL ThermalGetDeltaTempNonuniformity(AlgoHandle emulator_handle, DeltaTempeatureNonuniformity* in_out_delta_temp_nonumiform);
	

	/*********************************************
	@brief   : 获取箱体当前热力状态下造成的亮色度的不均匀性
	@note    :
	@param   : emulator_handle			         入参：句柄
	@param   : out_luminance_nonumiform			 出参：箱体当前状态亮色度的不均匀性
	@Author  : shimai@2025/03/18
	*********************************************/
	C_API AlgoError C_CALL ThermalGetLuminanceNonuniformity(AlgoHandle emulator_handle, LuminanceNonuniformity* out_luminance_nonumiform);
	

	

	/*********************************************
	@brief   : 获取拟合的全屏/在当前状态下的效果图
	@note    :
	@param   : emulator_handle			    入参：句柄
	@param   : out_x_img					出参：输出的x的效果图
	@param   : out_y_img					出参：输出的y的效果图
	@param   : out_z_img					出参：输出的z的效果图
	@Author  : shimai@2025/03/18
	*********************************************/
	C_API AlgoError C_CALL ThermalGetEffectImage(AlgoHandle in_meter_handle, AlgoMat* out_effect_image);
	

	/*********************************************
	@brief   : 存储文件到本地
	@note    :
	@param   : emulator_handle			    入参：句柄
	@param   : in_fullname			        入参：文件名
	@Author  : shimai@2025/03/18
	*********************************************/
	C_API AlgoError C_CALL ThermalSave(AlgoHandle emulator_handle, const char* in_fullname);
	

	/*********************************************
	@brief   : 从本地导入数据
	@note    :
	@param   : emulator_handle			    入参：句柄
	@param   : in_fullname			        入参：文件名
	@Author  : shimai@2025/03/18
	*********************************************/
	C_API AlgoError C_CALL ThermalLoad(AlgoHandle emulator_handle, const char* in_fullname);
	

	/*********************************************
	@brief   : 从句柄导入数据到buffer
	@note    :
	@param   : emulator_handle			    入参：句柄
	@param   : in_out_len			        出参：buffer长度
	@Author  : shimai@2025/03/18
	*********************************************/
	C_API AlgoError C_CALL ThermalGetBufferSize(AlgoHandle emulator_handle, uint64_t* in_out_len);
	
	/*********************************************
	@brief   : 从buffer中导入数据到句柄
	@note    :
	@param   : emulator_handle			    入参：句柄
	@param   : in_buffer			        入参：buffer
	@param   : in_len			            入参：buffer长度
	@Author  : shimai@2025/03/18
	*********************************************/
	C_API AlgoError C_CALL ThermalFromBuffer(AlgoHandle emulator_handle, const char* in_buffer, uint64_t in_len);
	
	/*********************************************
	@brief   : 从句柄导入数据到buffer
	@note    :
	@param   : emulator_handle			    入参：句柄
	@param   : in_buffer			        入参：buffer
	@param   : in_len			            入参：buffer长度
	@Author  : shimai@2025/03/18
	*********************************************/
	C_API AlgoError C_CALL ThermalToBuffer(AlgoHandle emulator_handle, char* out_buffer, uint64_t* in_out_len);
	

	/*********************************************
	@brief   : 销毁句柄
	@note    :
	@param   : emulator_handle			    入参：要销毁的句柄
	@Author  : shimai@2025/03/18
	*********************************************/
	C_API void C_CALL ThermalDestroyHandle(AlgoHandle emulator_handle);

#ifdef __cplusplus
}
#endif
#endif 
