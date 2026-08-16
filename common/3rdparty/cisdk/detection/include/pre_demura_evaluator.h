#ifndef __PRE_DEMURA_EVALUATOR_H__
#define __PRE_DEMURA_EVALUATOR_H__

#include "ci_define.h"
#include "ci_declare.h"
#include "pre_demura_evaluator_data.h"
#include "pre_demura_evaluator_error.h"
#include "camera_controller_data.h"
#include "roi_detector.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


    /************************************
    @brief  : 创建predemura句柄
    @note   :
    @param  : out_predemura_handle     目标值句柄
    @param  : in_methods
    @return	: AlgoError
    @Author : chengjiayi@2021/07/06
    ************************************/
    C_API AlgoError C_CALL PreDemuraCreateHandle(AlgoHandle* out_predemura_handle);


	/************************************
	@brief  : 计算箱体的最大温差
	@note   :
	@param  : in_img  输入的热力图像
	@param  : in_rect 输入箱体在热力图中的范围rect
	@param  : out_max_temp_diff 输出箱体最大温差
	@param  : out_current_max_temp 当前箱体的最大温度
	@return	: AlgoError
	@Author : shimai@2025/03/27
		************************************/
	C_API AlgoError C_CALL PreDemuraCalcCabinetMaxTempeatureDiff(AlgoMat* in_img, AlgoRect in_rect, double *out_max_temp_diff, double* out_current_max_temp = NULL);



	/************************************
	@brief  : 计算箱体热成像仪图像的最大温度差异
	@note   :
	@param  : in_data					 热成像图数据
	@param  : rows						 热成像图数据的rows
	@param  : cols						 热成像图数据的cols
	@param  : out_maxTemperatureDiff     输出，热成像仪数据中箱体范围中的最大温度差异
	@param  : in_Temperature_thresh      筛选箱体区域时的温度阈值，默认为35°
	@return	: AlgoError
	@Author : yaof@2025/02/25
	************************************/
	C_API AlgoError C_CALL PreDemuraCalBoxMaxTemperatureDiff(double* in_data, int rows, int cols, double *out_maxTemperatureDiff, int in_Temperature_thresh = 35);



	/************************************
	@brief  : 计算箱体的最大温升
	@note   :
	@param  : in_init_thermal      输入初始热力图像
	@param  : in_current_thermal   输入当前状态的热力状态
	@param  : out_max_temp_diff    输出箱体最大温差
	@return	: AlgoError
	@Author : shimai@2025/03/27
	************************************/
	C_API AlgoError C_CALL PreDemuraCalbinetMaxDeltaTemp(AlgoMat* in_init_thermal, AlgoMat* in_current_thermal, double* max_delta_tempeature);



	/************************************
	@brief  : 计算箱体热成像仪图像的最大温度差异
	@note   :
	@param  : in_data					 热成像图数据
	@param  : rows						 热成像图数据的rows
	@param  : cols						 热成像图数据的cols
	@param  : out_maxTemperatureDiff     输出，热成像仪数据中箱体范围中的最大温度差异
	@param  : in_Temperature_thresh      筛选箱体区域时的温度阈值，默认为35°
	@return	: AlgoError
	@Author : yaof@2025/02/25
	************************************/
	C_API AlgoError C_CALL PreDemuraCalcModuleMaxTemperatureDiff(double* in_data, int data_num, int mode_num, double *out_maxTemperatureDiff);



	/************************************
	@brief  : 计算箱体的线性度偏差
	@note   :
	@param  : in_meter_handles       入参：输入数据（所有灰阶的Y的光通量Meter数据）
	@param  : in_len			     入参：输入的meter数据的数目
	@param  : gamma_table:           入参：输入的gamma表
	@param  : in_bit_depth,          入参：输入源位深(8,10,12bit...)
	@param  : in_part_weight:		 入参：每个灰阶的权重，默认均为1
	@param  : in_start_gray:		 入参：开始灰阶
	@param  : in_end_gray:			 入参：截止灰阶
	@param  : out_lum:				 出参：每个灰阶的理想亮度值
	@param  : out_diff:				 出参：每个灰阶与理想亮度偏差值
	@param  : out_maxdiff:		     出参：最大偏差值
	@param  : out_mindiff:		     出参：最小偏差值
	@param	: out_avgdiff:		     出参：平均偏差值
	@Author : yaof@20250225
	************************************/
	C_API AlgoError C_CALL PreDemuraCalcGrayscaleLinearityDiff(
		AlgoHandle *meter_handles, int len, double *gamma_table, int bit_depth,
		float *part_weight,
		int start_gray, int end_gray, double *out_lum, double *out_diff,
		double &maxdiff, double &mindiff, double &avgdiff);


	/************************************
	@brief  : 计算屏体的RGBW耦合差异
	@note   :
	@param  : demura_layer_selector_handle          -- DemuraLayerSelector句柄
	@param  : in_RY_meter							-- RXmeter句柄
	@param  : in_GY_meter							-- GYmeter句柄
	@param  : in_BY_meter							-- BYmeter句柄
	@param  : in_WY_meter                           -- WY_meter句柄
	@param  : in_mod_cols						    -- 箱体灯板的列数
	@param  : in_mod_rows						    -- 箱体灯板的列数
	@param  : out_max_coupling                      -- 输出灯板间RGBW最大耦合差异
	@return	: AlgoError
	@Author : yaof@2025/02/25
	************************************/
	C_API AlgoError C_CALL PreDemuraCalcRGBWcouplingDiff(
		AlgoHandle in_RY_meter,
		AlgoHandle in_GY_meter, AlgoHandle in_BY_meter, AlgoHandle in_WY_meter,
		int in_mod_cols, int in_mod_rows,
		double *out_max_coupling
	);


	/************************************
	   @brief  : 计算屏体的亮暗差异
	   @note   :
	   @param  : demura_layer_selector_handle      -- DemuraLayerSelector句柄
	   @param  : in_meter						   -- 输入屏体 Y meter句柄
	   @param  : out_max_lumdiff                   --输出灯珠最大亮度差异
	   @param  : out_lum_cv                        --输出灯珠的变异系数
	   @param  : out_lum_uniformity                --输出屏体的均匀性
	   @return	: AlgoError
	   @Author : yaof 2025/02/25
	   ************************************/
	   //C_API AlgoError C_CALL PreDemuraCalcLumMaxdiff(
	   //	AlgoHandle in_meter, double *out_max_lumdiff, double* out_lum_cv, double* out_lum_uniformity);
	   /*************************************************
	  @brief  :计算屏体的亮暗差异
	  @note：
	  @param: demura_layer_selector_handle      ---DemuraLayerSelector句柄
	  @param: In_Meter                          ---输入屏体的Y meter 句柄
	  @param: Out_Max_Mumdiff_Without_Outliers  ---输出剔除异常值后的灯珠最大亮度差异
	  @param: Out_Max_Lumdiff_With_Outliers     ---输出未剔除异常值的灯珠最大亮度差异
	  @param: Out_Max_Without_Outliers          ---输出剔除异常值后的最大亮度
	  @param: Out_Min_Without_Outliers          ---输出剔除异常值后的最小亮度
	  @param: Out_Avg_Without_Outliers          ---输出剔除异常值后的平均亮度
	  @param: Out_Max_With_Outliers             ---输出未剔除异常值的最大亮度
	  @param: Out_Min_With_Outliers             ---输出未剔除异常值的最小亮度
	  @param: Out_Avg_With_Outliers             ---输出未剔除异常值的平均亮度
	  @param: Segment_Count                     ---输入分段数,默认10
	  @param: LED_Counts                        ---输出每段的灯点数量
	  @param: Brightness_Ranges                 ---输出分段后的亮度范围，用于绘制直方图，分为N段时，这里设置为N+1
	  @param: filePath                          ---输出逐灯点的亮度数据、存放路径，用以绘制仿真图
	  @return: AlgoError
	  @Author: Zhangguomeng 2026/03/18
	   **************************************************/
	C_API AlgoError PreDemuraCalcLumMaxdiff(AlgoHandle In_Meter, double *Out_Max_Mumdiff_Without_Outliers, double *Out_Max_Lumdiff_With_Outliers,
		double *Out_Max_Without_Outliers, double *Out_Min_Without_Outliers, double *Out_Avg_Without_Outliers,
		double *Out_Max_With_Outliers, double *Out_Min_With_Outliers, double *Out_Avg_With_Outliers,
		int Segment_Count, int LED_Counts[], double *Brightness_Ranges, const std::string& filePath);

	/************************************
    @brief  : 计算各灰阶图像的相似程度
    @note   :
    @param  : meter_handles                   -- 输入meter的句柄数组
    @param  : in_meter_num					 -- 输入meter数量
    @param  : in_base_gray                    -- 比较时的基准灰阶
    @param  : out_similar_score               --各灰阶与基准灰阶的相似度，灰阶顺序为从0开始递增：0，1，2，3，……
	  										 相似度分数取值范围：0~1
    @return	: AlgoError
    @Author : yaof
    ************************************/
	C_API AlgoError C_CALL PreDemuraCalcGrayscaleSimilarity(
		AlgoHandle *meter_handles,
		int in_meter_num, int in_base_gray, float *out_similar_score);



	/************************************
	@brief  : 计算各灰阶间的Mura一致性
	@note   :
	@param  : meter_handles                   -- 输入meter的句柄数组
	@param  : in_meter_num					  -- 输入meter数量
	@param  : in_module_rows				  -- 输入模组行数
	@param  : in_module_cols				  -- 输入模组列数
	@param  : out_max_mura_difference         -- 各灰阶间最大Mura偏差
											 
	@return	: AlgoError
	@Author : shimai@2025.12.04
	************************************/
	C_API AlgoError C_CALL PreDemuraCalcGrayscaleMuraUniformity(AlgoHandle *meter_handles, int in_meter_num, int in_module_rows, int in_module_cols, float *out_max_mura_difference);



	/************************************
	  @brief  : 计算屏体的波长分布
	  @note   :
	  @param  : in_data							 --输入的波长数据
	  @param  : int width, int height,           --输入数据的宽、高
	  @param  : int mode_cols, int mode_rows,    --输入，灯板的行数、列数
	  @param  : WaveDistributionPath，           --输入输出，波长分布图片的路径
	  @param  : out_histogram_mat,               --输出，波长的直方图分布，N*2的矩阵， 分别表示波长、此波长内灯珠数量
	  @param  : out_calc_score,                  --输出，校正分数，取值范围0 至 1.0

	  @return : AlgoError
	  @Author : yaof
	  ************************************/
	// C_API AlgoError C_CALL PreDemuraCalcWaveDistribution(int *in_data, int width, int height,
	// 	int mode_cols, int mode_rows, const char *WaveDistributionPath, AlgoMat *out_histogram_mat, double *out_maxwave_diff,
	// 	double *out_calc_score);

	/************************************
	  @brief  : 计算屏体的波长分布
	  @note   :
	  @param  : in_data							 --输入的波长数据
	  @param  : int width, int height,           --输入数据的宽、高
	  @param  : int mode_cols, int mode_rows,    --输入，灯板的行数、列数
	  @param  : out_histogram_mat                --输出，主波长分布直方图(每个波长对应的像素数)
	  @param  : out_max_wave_diff                --输出，最大波长差异
	  @param  : out_rob_max_wave_diff            --输出，去除异常波长后的最大波长差异
	  @param  : out_calc_score,                  --输出，校正分数，取值范围0 至 1.0
	  @param  : out_distribution_img,            --输出，主波长分布图(屏幕上每个像素的波长分布情况)
	  @param  : out_color_bar,					 --输出，color_bar
	  @param  : out_min_wave_len,				 --输出，最小波长
	  @param  : out_max_wave_len,				 --输出，最大波长
	  @param  : out_avg_wave_len,				 --输出，平均波长
	  2026/01/20新增内容如下：
	  @param  : out_rob_min_wave_len,			 --输出，去除异常波长后的最小波长
	  @param  : out_rob_max_wave_len,			 --输出，去除异常波长后的最大波长
	  @param  : out_rob_avg_wave_len,            --输出，去除异常波长后的平均波长
	  @param  : outline_ratio,				     --输出，异常点比例

	  @return : AlgoError
	  @Author : yaof 2026/3/24 张国梦修改
	  ************************************/
	C_API AlgoError C_CALL PreDemuraCalcWaveDistribution(int *in_data, int width, int height,
		int mode_cols, int mode_rows, AlgoMat *out_histogram_mat, double *out_max_wave_diff, double *out_rob_max_wave_diff,
		double *out_calc_score, AlgoMat* out_distribution_img, AlgoMat* out_color_bar, double* out_min_wave_len, double* out_max_wave_len, double* out_avg_wave_len, double* out_rob_min_wave_len = NULL, double* out_rob_max_wave_len = NULL, double* out_rob_avg_wave_len = NULL, double* outline_ratio = NULL);


	/************************************
	@brief  : 计算箱体的亮度、色温随温度的变化率
	@note   :
	@param  : in_init_thermal		    输入初始热力图像
	@param  : in_curr_thermal		    输入当前热力图像
	@param  : in_rect				    输入箱体在热力图中的范围
	@param  : in_init_x_meter		    输入初始X光通量
	@param  : in_init_y_meter		    输入初始y光通量
	@param  : in_init_z_meter		    输入初始z光通量
	@param  : in_curr_x_meter		    输入当前X光通量
	@param  : in_curr_y_meter		    输入当前y光通量
	@param  : in_curr_z_meter		    输入当前z光通量
	@param  : out_lum_delta_ratio       输出箱体亮度随温度的变化率
	@param  : out_color_temp_delta      输出色温随温度的变化量
	@return	: AlgoError
	@Author : shimai@2025/03/27
	************************************/
	C_API AlgoError C_CALL PreDemuraCalcLumAndColorTempDeltaWithTemperature(AlgoMat* in_init_thermal, AlgoMat* in_curr_thermal, AlgoRect in_rect,
		AlgoHandle in_init_x_meter, AlgoHandle in_init_y_meter, AlgoHandle in_init_z_meter, 
		AlgoHandle in_curr_x_meter, AlgoHandle in_curr_y_meter, AlgoHandle in_curr_z_meter,
		double* out_lum_delta_ratio, double* out_color_temp_delta);


	/************************************
	@brief  : 计算各个灰阶的相机采集参数
	@note   :
	@param  : in_baseline_capture_param	     输入基准灰阶下的采集参数(即在基准灰阶下采集的R_x, R_y, G_x, G_y, G_z的曝光时间等内容, )
	@param  : in_capture_grayscale		     输入要采集的灰阶(如30, 40, 50....)
	@param  : in_capture_num		         输入要采集的图像个数, 要采几个就输入几，
	                                         in_capture_num数须和in_baseline_capture_param中的数据个数相对应)

	@param  : in_grayscale_num		         输入要采集的灰阶个数
	@param  : in_gamma_table		         输入gamma表支持行数为1或为3的gamma表，即RGB共享同一gamma或不同gamma
	@param  : out_capture_param		         输出的各个灰阶下的采集参数
	@param  : in_min_exposure		         输入曝光时间推荐下限(此上下限是建议取值范围，实际计算出来的曝光时间可能会超出这一范围)，默认值为100
	@param  : in_max_exposure		         输入曝光时间推荐上限(此上下限是建议取值范围，实际计算出来的曝光时间可能会超出这一范围)，默认值100000
	@param  : in_max_gain		             输入最大增益倍数(建议不超过4)，默认值为4
	@param  : in_nd_ratio                    输入当前相机的全部nd倍率
	@param  : in_nd_num                      输入当前相机的ND镜数量
	@return	: AlgoError
	@Author : shimai@2025/04/07
	************************************/
	C_API AlgoError C_CALL PreDemuraCalcCamCapturePara(CamControllerCaptureParam* in_baseline_capture_param, int* in_capture_grayscale, int in_capture_num, int in_grayscale_num, AlgoMat* in_gamma_table, CamControllerCaptureParam* out_capture_param, float* in_nd_ratio = NULL, int in_nd_num = 1, int in_min_exposure = 100, int in_max_exposure = 10000, float in_max_gain = 4.0f);


	/************************************
	@brief  : 检测箱体的位置
	@note   :
	@param  : in_shot_handle							 输入采集的箱体shot图像
	@param  : in_roi_handle								 用来对图像中的箱体进行定位
	@param  : out_is_position_proper					 输出箱体摆放位置是否符合要求(是否端正)
	@param  : PreDremuraCabinetPositionOperation         输出要如何操作箱体位置(如果摆放位置不符合要求时看这个，符合就不用看了)
	@param  : in_length_ratio_threshold					 输入对边长度差异阈值(如果大于这个值，会被判定为梯形)，默认值为1%,即对边的长度差异不能超过1%
	@param  : in_angle_threshold						 输入角度差异阈值(如果大于这个值，会被认为没有放平，存在选择)，默认值为1，即箱体旋转角度不能超过1°
	@param  : in_center_distance_threshold				 输入中心距离阈值(如果大于这个值，会认为箱体没有放在视野中心位置)
	@return	: AlgoError
	@Author : shimai@2025/04/07
	************************************/
	C_API AlgoError C_CALL PreDemuraDetectCabinetPosition(AlgoHandle in_shot_handle, AlgoHandle in_roi_handle, bool* out_is_position_proper, PreDremuraCabinetPositionOperation* out_operation, const float in_length_ratio_threshold = 0.01f, const float in_angle_threshold = 1.0f, const float in_center_distance_threshold = 200.0f);


	/************************************
	@brief  : 标准化meter
	@note   : 由于各个灰阶采集时的曝光时间，增益，nd镜倍率等不同，导致最终不同灰阶获得的meter的亮度基本一致，无法比较各个灰阶之间的亮度差异
			  通过使用相机采集到的灰阶亮度以及采集时的参数，将灰阶亮色度标准化
	@param  : in_out_meter_handle			要标准化的meter(直接在原本的meter上处理)
	@param  : in_capture_param				对应灰阶的采集参数		 
	@return	: AlgoError
	@Author : shimai@2025/04/07
	************************************/
	C_API AlgoError C_CALL PreDemuraNormalizeMeter(AlgoHandle* in_out_meter, CamControllerCaptureParam* in_capture_param, int data_num);

	

	/************************************
	@brief  : 通过rgb的光通量得到w的光通量
	@note   : 需要先调用PreDemuraNormalizeMeter()将meter标准化

	@param  : in_rgb_meter		   输入的rgb的meter
	@param  : out_w_meter		   输出w的meter
	@return	: AlgoError
	@Author : shimai@2025/04/18
	************************************/
	C_API AlgoError C_CALL PreDemuraRgbLumToW(AlgoHandle in_r_meter, AlgoHandle in_g_meter, AlgoHandle in_b_meter, AlgoHandle out_w_meter);


	/************************************
	@brief  : 将色温(CCT)转换为色坐标(XY)(适用色温范围4000k - 25000k)
	
	@param  : input_cct		输入的色温值
	@param  : out_x		    输出的色坐标的小x
	@param  : out_y		    输出的色坐标的小y
	@return	: AlgoError
	@Author : shimai@2025/05/29
	************************************/
	C_API AlgoError C_CALL PreDemuraCCToXY(const float input_cct, float* out_x, float* out_y);


	/************************************
	@brief  : 计算采集的gamma1亮度和理想gamma1亮度偏差是否符合要求
	
	@param  : in_gamma1_lum       输入：gamma1的实测亮度
	@param  : in_gamma_max	      输入：gamma表最大值
	@param  : in_max_lum_val	  输入：最高灰阶对应的亮度值
	@param  : out_lum_diff  	  输出：实际1灰阶亮度和理论的差异
	@param  : out_if_qualified	  输出：偏差是否符合要求
	@return	: AlgoError
	@Author : shimai@2025/05/29
	************************************/
	C_API AlgoError C_CALL PreDemuraCalcGamma1LumDiff(const float in_gamma1_lum, const float in_gamma_max, const float in_max_lum_val, float* out_lum_diff, bool* out_if_qualified);


	/************************************
	@brief  : 计算采集的gamma1亮度和理想gamma1亮度偏差是否符合要求

	@param  : in_gamma1_lum       输入：gamma1的实测亮度
	@param  : in_gamma_max	      输入：gamma表最大值
	@param  : in_max_lum_val	  输入：最高灰阶对应的亮度值
	@param  : out_lum_diff  	  输出：实际1灰阶亮度和理论的差异
	@param  : out_if_qualified	  输出：偏差是否符合要求
	@return	: AlgoError
	@Author : shimai@2025/05/29
	************************************/
	C_API AlgoError C_CALL PreDemuraCalcGamma1LumDiff(const float in_gamma1_lum, const float in_gamma_max, const float in_max_lum_val, float* out_lum_diff, bool* out_if_qualified);


	/************************************
	@brief  : 计算校正距离
	
	@param  : in_cam_info					 输入：校正相机信息
	@param  : LensInfo						 输入：校正镜头信息
	@param  : in_point_pxl_num_in_img		 输入：灯点在图像中的像素数(n * n), 如5*5等
	@param  : in_led_pitch  				 输出：Led点间距
	@param  : in_screen_size	             输出：校正的屏幕尺寸
	@param  : out_can_calibrate	             输出：当前能否正常进行校正
	@param  : out_min_calib_distance	     输出：最小校正距离
	@param  : out_max_calib_distance	     输出：最远校正距离
	@param  : inset_factor					 输入：内缩比例：屏幕在相机视野中的最大占比, 默认为0.85

	@return	: AlgoError
	@Author : shimai@2025/08/08
	************************************/
	C_API AlgoError C_CALL PreDemuraCalcCalibrationDistance(CameraInfo in_cam_info, LensInfo in_lens_info,     // 相机和镜头相关参数
		float in_point_pxl_num_in_img, float in_led_pitch, AlgoSize in_screen_size,                     // 屏幕相关参数
		bool* out_can_calibrate, float* out_min_calib_distance, float* out_max_calib_distance, float in_inset_factor = 0.85f);   // 输出


	/************************************
	@brief  : 计算RGB耦合系数(合成W时RGB的具体配比系数)

	@param  : in_r_meter					 输入：r的xyz光通量meter
	@param  : in_g_meter				     输入：g的xyz光通量meter
	@param  : in_b_meter				     输入：b的xyz光通量meter
	@param  : in_w_meter				     输入：w的xyz光通量meter
	@param  : in_data_num                    输入：meter数量
	@param  : out_couple_coeff		         输出：耦合系数(合成W时的RGB的具体配比)
	

	@return	: AlgoError
	@Author : shimai@2025/09/11
	************************************/
	C_API AlgoError C_CALL PreDemuraComputeRgbCoupleCoeff(AlgoHandle* in_r_meter, AlgoHandle* in_g_meter, AlgoHandle* in_b_meter, AlgoHandle* in_w_meter, int in_data_num, AlgoMat* out_couple_coeff);



	/************************************
	@brief  : 应用RGB耦合系数(计算合成的W中RGB各自的XYZ的情况)
	
	@param  : in_r_meter				输入输出：r的xyz光通量meter，会输出叠加耦合系数后的光通量
	@param  : in_g_meter				输入输出：g的xyz光通量meter，会输出叠加耦合系数后的光通量
	@param  : in_b_meter				输入输出：b的xyz光通量meter，会输出叠加耦合系数后的光通量
	@param  : in_data_num    				输入：数据数量
	@param  : in_couple_coeff		        输入：耦合系数(合成W时的RGB的具体配比)
	
	@return	: AlgoError
	@Author : shimai@2025/09/11
	************************************/
	C_API AlgoError C_CALL PreDemuraApplyRgbCoupleCoeff(AlgoHandle* in_out_r_meter, AlgoHandle* in_out_g_meter, AlgoHandle* in_out_b_meter, int in_data_num, AlgoMat* in_couple_coeff, AlgoHandle* out_w_r_xyz, AlgoHandle* out_w_g_xyz, AlgoHandle* out_w_b_xyz);


	/************************************
	@brief  : 计算亮度均匀性

	@param  : in_meter					输入：光通量meter
	@param  : in_module_rows			输入：箱体中灯板的行数
	@param  : in_module_cols			输入：箱体中灯板的列数
	@param  : out_module_uniformity    	输出：各个灯板的均匀性
	@param  : out_cabinet_uniformity    输出：整个箱体的均匀性
	@param  : in_n_subregions    		输入：在测试灯板均匀性时选择的子区域个数(n点法测量，默认为9)
	@param  : in_subregion_pixel_num	输入：单个子区域的像素宽高，默认为10，即每个子区域选10*10共100个像素

	@return	: AlgoError
	@Author : shimai@2026/01/15
	************************************/
	//C_API AlgoError C_CALL PreDemuraCalcLumUniformity(AlgoHandle in_meter, int in_module_rows, int in_module_cols, float* out_module_uniformity, float* out_cabinet_uniformity, int in_n_subregions = 9,int in_subregion_pixel_num = 10);
	/**
	* 计算亮度均匀性
	*
	*
	* param in_meter						输入：光通量meter
	* param in_module_rows					输入：箱体中灯板的行数
	* param in_module_cols					输入：箱体中灯板的列数
	* param out_module_uniformity	  	    输出：各个灯板的均匀性
	* param out_cabinet_uniformity			输出：整个箱体的均匀性
	* param out_module_subregion_luminance  输出：各灯板内选中区域的亮度值数组
	* param out_module_mean_luminance		输出：各灯板的亮度均值数组
	* param in_n_subregions					输入：在测试灯板均匀性时选择的子区域个数(n点法测量，默认为9)
	* param in_subregion_pixel_num			输入：单个子区域的像素宽高，默认为10，即每个子区域选10*10共100个像素
	* return AlgoError						返回算法执行错误码
	* Author zhangguomeng@2026/03/23        新增各灯板内选中区域，各灯板亮度值的输出
	*/
	C_API AlgoError C_CALL PreDemuraCalcLumUniformity(AlgoHandle in_meter, int in_module_rows, int in_module_cols,
		float* out_module_uniformity, float* out_cabinet_uniformity,
		float* out_module_subregion_luminance, float* out_module_mean_luminance,
		int in_n_subregions = 9, int in_subregion_pixel_num = 10);


	/************************************
	* 计算色度非均匀性
	*
	* @param Is_Red                                 输入：判断屏体输入颜色是否为红色，0 代表不是红色，1 代表是红色
	* @param Red_Z                                  输入：当 Is_Red 为 1 时，使用的 Z 通道默认值,建议使用光枪标定的红色Z值
	* @param X_Meter                                输入：X 通道的数据
	* @param Y_Meter                                输入：Y 通道的数据
	* @param Z_Meter                                输入：非红色时使用的 Z 通道数据
	* @param In_Module_Rows                         输入：箱体中灯板的行数
	* @param In_Module_Cols                         输入：箱体中灯板的列数
	* @param Out_Module_Color_Non_Uniformity        输出：各个灯板的色度非均匀性
	* @param Out_Cabinet_Color_Non_Uniformity       输出：整个箱体的色度非均匀性
	* @param Out_Panel_Subregion_U_Prime_Avg        输出：每块灯板内各子区域均值 u' 的数组，长度为灯板数量 * 子区域个数
	* @param Out_Panel_Subregion_V_Prime_Avg        输出：每块灯板内各子区域均值 v' 的数组，长度为灯板数量 * 子区域个数
	* @param Out_Panel_U_Avg                        输出：每块灯板内所有色坐标 u 的均值数组，长度为灯板数量
	* @param Out_Panel_V_Avg                        输出：每块灯板内所有色坐标 v 的均值数组，长度为灯板数量
	* @param In_N_Subregions                        输入：在测试灯板色度非均匀性时选择的子区域个数(n点法测量，默认为 9)
	* @param In_Subregion_Pixel_Num                 输入：单个子区域的像素宽高，默认为 10，即每个子区域选 10*10 共 100 个像素
	* @return AlgoError                             返回算法执行错误码
	* @Author zhangguommeng@2026/03/20
	************************************/
	C_API AlgoError C_CALL PreDemuraCalcChromaNonUniformity(int Is_Red, double Red_Z, AlgoHandle X_Meter, AlgoHandle Y_Meter, AlgoHandle Z_Meter, int In_Module_Rows, int In_Module_Cols, float* Out_Module_Color_Non_Uniformity, float* Out_Cabinet_Color_Non_Uniformity,
		double* Out_Panel_Subregion_U_Prime_Avg, double* Out_Panel_Subregion_V_Prime_Avg, double* Out_Panel_U_Avg, double* Out_Panel_V_Avg, int In_N_Subregions = 9, int In_Subregion_Pixel_Num = 10);

	/************************************
	* 标定光通量Meter
	*
	*
	* param in_meter								输入：原始光通量meter
	* param target									输入：标定目标值
	* param out_calibrated_meter					输出：标定后的光通量meter
	* return AlgoError								返回算法执行错误码
	* Author zhangguommeng@2026/03/24
	*/
	C_API AlgoError C_CALL PreDemuraCalibrateMeter(AlgoHandle in_meter, float target, AlgoHandle out_calibrated_meter);

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
    C_API AlgoError C_CALL  PreDemuraRelease(AlgoHandle target_handle);
    C_API void C_CALL PreDemuraDestroyHandle(AlgoHandle target_handle);



	/***
 * Function Name		: CalTemperatureInThermalImager
 * Description     		: 计算热成像图具体位置的温度
 *
 * Parameters:
 * in_data:						IN,   热成像图数据
 * rows:						IN,   热成像图数据的rows
 * cols:						IN,   热成像图数据的cols
 * out_average_T:				OUT,  多个计算区域的平均温度值数组指针
 * in_center_x:					IN,   多个计算区域的中心点x坐标数组指针，列。默认为空，此时为1个点，且为中心点的x
 * in_center_y:					IN,   多个计算区域的中心点y坐标数组指针，行。默认为空，此时为1个点，且为中心点的y
 * in_point_num					IN,   计算区域的数量，默认值为1；
 * in_rect_width:				IN,   计算区域的宽，默认值为20；
 * return  :					0：success   -1：输入数据为空 -2：区域参数设置异常  -3：区域大小异常
 * Author  : lichaoping
 *
 */

 // C_API AlgoError C_CALL CalTemperatureInThermalImager(double* in_data, int rows, int cols, double *out_average_T, int* in_center_x = NULL, int* in_center_y = NULL, int in_point_num = 1, int in_rect_width = 20);


#ifdef __cplusplus
}
#endif // __cplusplus
#endif