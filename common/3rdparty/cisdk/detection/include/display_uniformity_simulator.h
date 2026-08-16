#ifndef __DISPLAY_UNIFORMITY_SIMULATOR_H__
#define __DISPLAY_UNIFORMITY_SIMULATOR_H__

#include "algo_mat.h"
#include "ci_data.h"
#include "ci_declare.h"
#include "meta_info_data.h"
#include "display_uniformity_simulator_data.h"
#include "display_uniformity_simulator_error.h"


#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

	/*********************************************
	@brief   : 创建CalibEffectionSimulator(校正效果仿真)句柄
	@note    :
	@param   : calibration_handle
	@return  : AlgoError
	@Author  : shimai@2025/07/01
	*********************************************/
	C_API AlgoError C_CALL CalibEffectionCreateHandle(AlgoHandle* calibration_handle);


	/*********************************************
	@brief   : 应用校正方案
	@note    :
	@param   : calibration_handle
	@param   : in_grayscale          输入的meter的灰阶
	@param   : in_out_r_xyz_meter    输入输出(校正前后)的r的xyzmeter的灰阶
	@param   : in_out_g_xyz_meter    输入输出(校正前后)的g的xyzmeter的灰阶
	@param   : in_out_b_xyz_meter    输入输出(校正前后)的b的xyzmeter的灰阶
	@param   : 
	@return  : AlgoError
	@Author  : shimai@2025/07/01
	*********************************************/
	C_API AlgoError C_CALL  CalibEffectionApply(AlgoHandle calibration_handle, int in_grayscale,  AlgoHandle* in_out_r_xyz_meter, AlgoHandle* in_out_g_xyz_meter, AlgoHandle* in_out_b_xyz_meter, bool is_white = false);


	/*********************************************
	@brief   : 设置亮度校正系数
	@note    :
	@param   : calibration_handle       对应的校正效果句柄
	@param   : in_calib_grayscales      亮度校正的灰阶(在哪些灰阶分别做的亮度校正，校正灰阶按从小到大顺序输入)
	@param   : in_lum_coeff             亮度校正系数(亮度校正系数与亮度校正灰阶一一对应)
	@param   : calib_layer              校正层数(在几个灰阶下做了校正，校正层数应该与校正灰阶数与校正系数的个数相对应)
	@return  : AlgoError
	@Author  : shimai@2025/07/01
	*********************************************/
	C_API AlgoError C_CALL  CalibEffectionSetLumCoeff(AlgoHandle calibration_handle, int* in_calib_grayscales, AlgoHandle* in_lum_coeff, int calib_layers);


	/*********************************************
	@brief   : 设置白色校正系数
	@note    :
	@param   : calibration_handle       对应的校正效果句柄
	@param   : in_calib_grayscales      白色校正的灰阶(在哪些灰阶分别做的白色校正，校正灰阶按从小到大顺序输入)
	@param   : in_white_coeff           白色校正系数(白色校正系数与白色校正灰阶一一对应)
	@param   : calib_layer              校正层数(在几个灰阶下做了校正，校正层数应该与校正灰阶数与校正系数的个数相对应)
	@return  : AlgoError
	@Author  : shimai@2025/07/01
	*********************************************/
	C_API AlgoError C_CALL  CalibEffectionSetWhiteCoeff(AlgoHandle calibration_handle, int* in_calib_grayscales, AlgoHandle* in_white_coeff, int calib_layers);


	/*********************************************
	@brief   : 设置色度校正系数
	@note    :
	@param   : calibration_handle       对应的校正效果句柄
	@param   : in_calib_grayscale       色度校正的灰阶
	@param   : in_chroma_coeff          色度校正系数
	@param   : is_high_precision_coef   是否为高精度系数(对于全灰阶校正，如果涉及多个灰阶的亮度校正时，这里应该是一个单独的色度系数，参数值应该为false，
	                                    如果是高精度校正或者只做了白色的全灰阶， 那么这里就是一个高精度系数，参数值就应该为true)
	@return  : AlgoError
	@Author  : shimai@2025/07/01
	*********************************************/
	C_API AlgoError C_CALL  CalibEffectionSetChromaCoeff(AlgoHandle calibration_handle, int in_calib_grayscale, AlgoHandle in_chroma_coeff, bool is_high_precision_coef = false);


	/*********************************************
	@brief   : 设置Gamma表
	@note    :
	@param   : calibration_handle       对应的校正效果句柄
	@param   : gamma_table              gamma表

	@return  : AlgoError
	@Author  : shimai@2025/07/01
	*********************************************/
	C_API AlgoError C_CALL  CalibEffectionSetGammaTable(AlgoHandle calibration_handle, AlgoMat* gamma_table);


	/*********************************************
	@brief   : 计算热力校正系数
	@note    :
	@param   : calibration_handle       对应的校正效果句柄
	@param   : in_hot_w_xyz             热屏下w的xyz数据
	@param   : in_cold_w_xyz            冷屏下w的xyz数据
	@param   : in_cold_r_xyz            冷屏下r的xyz数据
	@param   : in_cold_g_xyz            冷屏下g的xyz数据
	@param   : in_cold_b_xyz            冷屏下b的xyz数据
	@return  : AlgoError
	@Author  : shimai@2025/12/12
	*********************************************/
	C_API AlgoError C_CALL  CalibEffectionComputeThermalCoeff(AlgoHandle calibration_handle, AlgoHandle* in_cold_w_xyz, AlgoHandle* in_cold_r_xyz, AlgoHandle* in_cold_g_xyz, AlgoHandle* in_cold_b_xyz, AlgoHandle* in_hot_w_xyz, int in_data_num = 3);



	/*********************************************
	@brief   : 计算热力校正系数
	@note    :
	@param   : calibration_handle       对应的校正效果句柄
	@param   : in_r_xyz                 输入r的xyz数据
	@param   : in_g_xyz                 输入g的xyz数据
	@param   : in_b_xyz                 输入b的xyz数据
	@param   : in_calib_method          输入热力校正方式
	@param   : in_alpha                 输入拉杆系数(如果选了热力2.0就必须输入，范围0-1)
	@param   : in_cold_temp             输入初始状态的热力图像(如果选了热力3.0就必须输入)
	@param   : in_hot_temp              输入热平衡状态的热力图像(如果选了热力3.0就必须输入)
	@param   : in_curr_temp             输入当前状态的热力图像(如果选了热力3.0就必须输入)
	@return  : AlgoError
	@Author  : shimai@2025/12/12
	*********************************************/
	C_API AlgoError C_CALL  CalibEffectionApplyThermalCoeff(AlgoHandle calibration_handle, AlgoHandle* in_r_xyz, AlgoHandle* in_g_xyz, AlgoHandle* in_b_xyz, DisplayThermalCalibMethod in_calib_method, float in_alpha = -1.f, AlgoMat* in_cold_temp = NULL, AlgoMat* in_hot_temp = NULL, AlgoMat* in_curr_tem = NULL, int in_data_num = 3);

	
	/*********************************************
	@brief   : 将校正方案储存到本地，后缀名为.calib_effection
	@note    :
	@param   : calibration_handle       对应的校正效果句柄
	@param   : in_fullname              文件名
	
	@return  : AlgoError
	@Author  : shimai@2025/07/01
	*********************************************/
	C_API AlgoError C_CALL  CalibEffectionSave(AlgoHandle calibration_handle, const char* in_fullname);

	/*********************************************
	@brief   : 从本地读取校正方案到句柄，后缀名为.calib_effection
	@note    :
	@param   : calibration_handle       对应的校正效果句柄
	@param   : in_fullname              文件名
	
	@return  : AlgoError
	@Author  : shimai@2025/07/01
	*********************************************/
	C_API AlgoError C_CALL  CalibEffectionLoad(AlgoHandle calibration_handle, const char* in_fullname);

	/*********************************************
	@brief   : 销毁校正效果句柄
	@note    :
	@param   : calibration_handle       对应的校正效果句柄
	@return  : AlgoError
	@Author  : shimai@2025/07/01
	*********************************************/
	C_API void C_CALL  CalibEffectionDestroyHandle(AlgoHandle calibration_handle);


	/*********************************************
	@brief   : 创建DisplayResultSimulator句柄
	@note    :
	@param   : simulator_handle
	@return  : AlgoError
	@Author  : shimai@2025/04/18
	*********************************************/
	C_API AlgoError C_CALL DisplayCreateHandle(AlgoHandle* simulator_handle);

	/*********************************************
	@brief   : 获取任意灰阶下的meter
	@note    :
	@param   : simulator_handle     输入句柄
	@param   : in_meters            输入meter，可以输一个或多个，根据输入的全部meter插值生成要观测的灰阶
	@param   : data_num             输入的meter个数
	@param   : in_grayscale         输入要预览的灰阶
	@param   : in_gamma_table       输入gamma表
	@param   : out_meter            输出的meter
	@return  : AlgoError
	@Author  : shimai@2025/04/24
	*********************************************/
	C_API AlgoError C_CALL DisplayGetAnyGrayScaleMeter(AlgoHandle simulator_handle, AlgoHandle* in_meters, const int data_num, const int in_grayscale, const AlgoMat* in_gamma_table, AlgoHandle out_meter);

	/*********************************************
	@brief   : 设置峰值白色XYZ
	@note    :
	@param   : simulator_handle     输入句柄
	@param   : SetPeakWhite         峰值白色的XYZ数据(光枪值)
	@return  : AlgoError
	@Author  : shimai@2025/04/24
	*********************************************/
	C_API AlgoError C_CALL DisplaySetPeakWhite(AlgoHandle simulator_handle, TriXYZ in_xyz);

	/*********************************************
	@brief   : 设置自定义显示色域
	@note    :
	@param   : simulator_handle     输入句柄
	@param   : in_rgbw_lxy          目标显示色域的三基色和白色的亮色度信息(光枪值)
	@return  : AlgoError
	@Author  : shimai@2025/04/24
	*********************************************/
	C_API AlgoError C_CALL DisplaySetCustomGamut(AlgoHandle simulator_handle, RGBWLxy in_rgbw_lxy);

	/*********************************************
	@brief   : 获取拟合的效果图像(支持1通道或3通道数据输入，1通道输入输出为灰度图像，3通道输入输出为彩色图像)
	@note    :
	@param   : simulator_handle						输入句柄
	@param   : in_meter_handle						输入光通量 (2025/07/18更新，将入参类型从AlgoHandle转为AlgoHandle*)
	@param   : out_image							输出拟合的效果图像
	@param   : in_channel							输出数据通道数，只能为1或3，需要与in_meter_handle的长度对应
	@param   : in_gamut								要显示的色域(支持标准色域与自定义色域（仅支持输入三通道数据时使用）
													如果是自定义色域需要先设置色域信息，默认为Rec.709)
	@param   : in_mode								转换模式mode0速度快，偏色严重，mode1速度慢，偏色轻微，默认为mode1（仅支持输入三通道数据时使用）
	@param   : use_max_white_norm                   是否用峰值白色亮度(光枪值)进行标准化（如果是则不同灰阶间的结果会反映灰阶间亮度差异，需要设置峰值亮色度信息，即调用
	                                                如果是否则只输出相对亮度信息）（仅支持输入三通道数据时使用）
	@return  : AlgoError
	@Author  : shimai@2025/04/18
	@Author  : shimai@2025/07/18    更新，把生成灰度图像和彩色图像进行集成
	*********************************************/
	C_API AlgoError C_CALL DisplayGetSimulateImage(AlgoHandle simulator_handle, AlgoHandle* in_meter_handle, AlgoMat* out_image, int in_channels = 1, DisplayTargetGamut in_gamut = DisplayTargetGamut::REC_709, int in_mode = 1, bool use_max_white_norm = false);

	/*********************************************
	@brief   : 获取亮度增强图像
	@note    :
	@param   : simulator_handle     输入句柄
	@param   : out_img              输出的亮度增强图像，用于展示亮度的不均匀性(必须先使用DisplayGetSimulateImage接口生成过彩色图像)
	@return  : AlgoError
	@Author  : shimai@2025/07/18
	*********************************************/
	C_API AlgoError C_CALL DisplayGetLumEnhanceImage(AlgoHandle simulator_handle, AlgoMat* out_img, AlgoMat* out_color_bar, float* out_min_lum, float* out_max_lum);

	/*********************************************
	@brief   : 获取颜色增强图像
	@note    :
	@param   : simulator_handle     输入句柄
	@param   : out_image            输出的颜色增强图像，用于展示色偏的不均匀性(必须先使用DisplayGetSimulateImage接口生成过彩色图像)
	@param   : in_max_hue_diff      输入最大色相角的差(单位：角度°，范围0-180)
	@param   : in_min_saturation    输入小饱和度(范围0-1)
	@param   : in_max_saturation    输入大饱和度(范围0-1)
	@return  : AlgoError
	@Author  : shimai@2025/07/18
	*********************************************/
	C_API AlgoError C_CALL DisplayGetChromaEnhanceImage(AlgoHandle simulator_handle, AlgoMat* out_img, int in_max_hue_diff = 15, float in_min_saturation = 0.f, float in_max_saturation = 1.f);


	/*********************************************
	@brief   : 导出拟合的效果图像
	@note    :
	@param   : in_img               输入要导出的图像
	@param   : out_image_path       导出图像的路径(支持jpg png格式)
	@return  : AlgoError
	@Author  : shimai@2025/04/18
	*********************************************/
	C_API AlgoError C_CALL DisplayExportImage(AlgoMat* in_img, const char* in_img_path);



	/*********************************************
	@brief   : 导出Lxy数据(需要获取了RGB图像即输入了XYZ数据之后才能调用)
	@note    :
	@param   : simulator_handle     输入句柄
	@param   : out_lxy_mat			输出的Lxy数据
	@return  : AlgoError
	@Author  : shimai@2025/09/23
	*********************************************/
	C_API AlgoError C_CALL DisplayGetLxy(AlgoHandle simulator_handle, AlgoMat* out_lxy_mat);


	/*********************************************
	@brief   : 获取多个亮度增强图像
	@note    :
	@param   : simulator_handle     输入句柄
	@param	 : in_meters            输入的meter(必须把XYZ数据都输入)
	@param   : in_data_num          输入meter数量(输入的XYZ组数 * 3， 例如：输入了一组XYZ就输入3，输入了两组XYZ就输入6)
	@param   : out_mats             输出的亮度增强图像
	@param   ：out_color_bar        输出的colorbar
	@param   : out_min_val          输出的bar范围的最小值
	@param   : out_max_val          输出的bar范围的最大值
	@return  : AlgoError
	@Author  : shimai@2025/09/23
	*********************************************/
	C_API AlgoError C_CALL DisplayGetMultipleLumEnhanceImage(AlgoHandle simulator_handle, AlgoHandle* in_meters, int in_data_num, AlgoMat* out_mats, AlgoMat* out_colorbar, float* out_min_val, float* out_max_val);


	/*********************************************
	@brief   : 导出多个色度增强图像
	@note    :
	@param   : simulator_handle     输入句柄
	@param	 : in_meters            输入的meter(必须把XYZ数据都输入)
	@param   : in_data_num          输入meter数量(输入的XYZ组数 * 3， 例如：输入了一组XYZ就输入3，输入了两组XYZ就输入6)
	@param   : out_mats             输出的色度增强图像
	@param   ：in_lum_gain          输入对亮度增强的增益，默认为10
	@param   ：in_chroma_gain       输入对色度增强的增益，默认为20
	@param   ：in_lum_limit         输入对亮度调整的上限，默认为40
	@param   ：in_chroma_limit      输入对色度增强的增益，默认为60
	@return  : AlgoError
	@Author  : shimai@2026/01/09
	*********************************************/
	C_API AlgoError C_CALL DisplayGetMultipleChromaEnhanceImage(AlgoHandle simulator_handle, AlgoHandle * in_meters, int in_data_num, AlgoMat * out_mats, float in_lum_gain = 10.f, float in_chroma_gain = 20.f, float in_lum_limit = 40.f, float in_chroma_limit = 60.f);

	
	
	/*********************************************
	@brief   : 销毁句柄
	@note    :
	@param   : simulator_handle			    入参：要销毁的句柄
	@Author  : shimai@2025/03/18
	*********************************************/
	C_API void C_CALL DisplayDestroyHandle(AlgoHandle simulator_handle);


	// /*********************************************
	// @brief   : 应用亮色度校正系数
	// @note    :
	// @param   : simulator_handle           输入句柄
	// @param   : in_out_r_meter_handles     输入——输出r的光通量(按照xyz顺序输入)
	// @param   : in_out_g_meter_handles     输入——输出g的光通量(按照xyz顺序输入)
	// @param   : in_out_b_meter_handles     输入——输出b的光通量(按照xyz顺序输入)
	// @param   : in_chroma_coeff            输入亮色度校正系数
	// @return  : AlgoError
	// @Author  : shimai@2025/04/23
	// *********************************************/
	// C_API AlgoError C_CALL UniformitySimulatorApplyChromaCoeff(AlgoHandle simulator_handle, AlgoHandle* in_out_r_meter_handles, AlgoHandle* in_out_g_meter_handles, AlgoHandle* in_out_b_meter_handles, AlgoHandle in_chroma_coeff);


	// /*********************************************
	// @brief   : 叠加色度系数和亮度系数(将亮度系数和色度系数融合乘一个色度系数，支持未做色度校正，直接把亮度系数转换为色度系数)
	// @note    :
	// @param   : in_out_chroma_coeff     输入输出色度系数(最终会把两个系数融合)
	// @param   : in_lum_coeff            输入亮度校正系数
	// @return  : AlgoError
	// @Author  : shimai@2025/04/24
	// *********************************************/
	// C_API AlgoError C_CALL UniformitySimulatorMergeChromaAndLumCoeff(AlgoHandle in_out_chroma_coeff, AlgoHandle in_lum_coeff);


	// /*********************************************
	// @brief   : 叠加色度系数和热力系数(将亮度系数和热力系数融合乘一个色度系数，支持未做色度校正，直接把热力系数转换为色度系数)
	// @note    :
	// @param   : in_out_chroma_coeff        输入输出色度系数(最终会把两个系数融合)
	// @param   : in_thermal_coeff            输入热力校正系数
	// @return  : AlgoError
	// @Author  : shimai@2025/04/24
	// *********************************************/
	// C_API AlgoError C_CALL DisplayMergeChromaAndThermalCoeff(AlgoHandle in_out_chroma_coeff, AlgoHandle in_thermal_coeff);


	// /*********************************************
	// @brief   : 热力系数温度补偿
	// @note    :
	// 
	// 
	// @param   : in_thermal_coeff          输入热力校正系数
	// @param   : in_init_thermal           输入初始的热力状态
	// @param   : in_balance_thermal        输入热平衡的热力状态
	// @param   : in_current_thermal        输入当前的热力状态
	// @param   : out_thermal_coeff         输出补偿后的热力校正系数    
	// 
	// @return  : AlgoError
	// @Author  : shimai@2025/04/24
	// *********************************************/
	// C_API AlgoError C_CALL DisplayThermalCoeffTemperatureCompensation(AlgoHandle in_thermal_coeff, AlgoMat* in_init_thermal, AlgoMat* in_balance_thermal, AlgoMat* in_current_thermal, AlgoHandle out_thermal_coeff);

#ifdef __cplusplus
}
#endif
#endif 
