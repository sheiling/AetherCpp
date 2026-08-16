#ifndef __CAMERA_CONTROLLER_HPP__
#define __CAMERA_CONTROLLER_HPP__

#include "ci_define.h"
#include "ci_declare.h"
#include "camera_controller_data.h"
#include "camera_controller.h"
#include "camera_controller_error.h"
#include "quality_estimator_data.h"
#include "quality_estimator_data.h"
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


	/************************************
	@brief  : 创建相机调节句柄
	@note   :
	@param  : out_camera_handle    相机调节句柄
	@return	: AlgoError
	@Author : Laihaokun@2022/06/10
	************************************/
	C_API AlgoError C_CALL CameraParamCreateHandle(AlgoHandle* out_camerapara_handle);
		

	/************************************
	@Brief		：	根据隔点成像状态与隔点尺寸推算逐点图像的采集状态
	@Note		：	输入handle包括有效RGBW参数时，可进行RGBW-XYZ的全量隔点参数推算。
					1）只有单灰阶参数时，默认check RGB，若输入handle单灰阶估计时基准是 W,则同时Check RGBW
					2）存在全灰阶参数时，根据SetEstiamteState去 Check，不设置时默认check RGB-Y

					重点：隔点推算时最小曝光时间与逐点不同，隔点最小曝光要大于逐点最小曝光，例如隔点150ms，逐点100ms（合适参数与相机、屏体刷新率有关）；
						该接口出参的handle是逐点的采集参数，不能够再继续用来推测后续业务的隔点参数。
						现阶段基于实验测算的经验值进行曝光时间衰减，此时曝光时间可能会超出最大曝光范围，暂不做处理
						后续拟计划基于灯点占比、隔点尺寸、采样率等估计灯点间混光模型，进行曝光时间推算
	@Parameter	: 	in_dot_size								--隔点尺寸，范围是1*1到6*6
	@Parameter	: 	in_out_camerapara_handle				--入参：合理的隔点采集参数，通过相机调节环节对隔点图像进行质量评估与相机参数分析得到
															  出参：基于隔点尺寸和成像状态（例如占空比）推算的逐点采集参数
	@Returns	:   AlgoError
	@Author		：	GuoHao@2024/03/18
	************************************/
	C_API AlgoError C_CALL CameraParamCalculateAllPointImageCaptureParamByDot(AlgoHandle in_out_camerapara_handle, AlgoSize in_dot_size);


	/************************************
	@Brief		：	根据逐点成像状态与隔点尺寸推算隔点图像的采集状态
	@Note		：	输入handle包括有效RGBW参数时，可进行RGBW-XYZ的全量隔点参数推算。
					1）只有单灰阶参数时，默认check RGBW-XYZ
					2）存在全灰阶参数时，根据SetEstiamteState去 Check，不设置时默认check RGB-Y

					重点：隔点推算时最小曝光时间与逐点不同，隔点最小曝光要大于逐点最小曝光，例如隔点150ms，逐点100ms（合适参数与相机、屏体刷新率有关）；
						该接口出参的handle是隔点的采集参数，不能够再继续用来推测后续业务的逐点参数。
						现阶段基于实验测算的经验值进行曝光时间衰减，此时曝光时间可能会超出最大曝光范围，暂不做处理（仅报错）
						后续拟计划基于灯点占比、隔点尺寸、采样率等估计灯点间混光模型，进行曝光时间推算
	@Parameter	: 	in_dot_size								--隔点尺寸，范围是1*1到6*6
	@Parameter	: 	in_out_camerapara_handle				--入参：合理的逐点采集参数，通过相机调节环节对逐点图像进行质量评估与相机参数分析得到
															  出参：基于隔点尺寸和成像状态（例如占空比）推算的隔点采集参数
	@Returns	:   AlgoError
	@Author		：	HuangZiMing@2025/06/09
	************************************/
	C_API AlgoError C_CALL CameraParamCalculateDotImageCaptureParamByAllPoint(AlgoHandle in_out_camerapara_handle, const AlgoSize in_dot_size);


	/************************************
	@Brief		：	设置公共参数
	@Note		：
	@Parameter	: 	camerapara_handle			--有效的handle
	@Parameter	: 	capture_capture_type_in		--采集方式，默认为点定位，可选超分成像
	@Parameter	: 	lum_dec_ratio_rgb_in		--rgb的衰减，为衰减了多少。 eg， {0.2f, 0.3f, 0.1f}表示红色亮度要衰减20%， 绿色要衰减30%
	@Parameter	: 	lum_dec_ratio_rgb_num		--lum_dec_ratio_rgb_in的个数
	@Parameter	: 	s_max_in					--灯点峰值上限
	@Parameter	: 	s_min_in					--灯点峰值下限
	@Parameter	: 	q_max_in					--灯点大小上限
	@Parameter	: 	q_min_in					--灯点大小上限
	@Parameter	: 	expotime_max_in				--曝光时间上限
	@Parameter	: 	expotime_min_in				--曝光时间下限
	@Parameter	: 	calib_grayscale_in			--输入rgb最高校正灰阶(默认-1，用于重置handle内部存的rgb最高灰阶)
	@Returns	:   AlgoError
	@Author		：	TianBo@2024/03/18
	************************************/
	C_API AlgoError C_CALL CameraParamSeCommonParam(AlgoHandle camerapara_handle, CamControllerCaptureType capture_capture_type_in, float* lum_dec_ratio_rgb_in, int lum_dec_ratio_rgb_num,
	    const float s_max_in, const float s_min_in, const float q_max_in, const float q_min_in, const int expotime_max_in, const int expotime_min_in, int calib_grayscale_in = -1);


	/************************************
	@Brief		：  设置增益参数
	@Note		：  不设置时，为不启用增益。  输入的gain为倍数，必须不小于1. 1表示不启用增益
	@Parameter	: 	camerapara_handle				--相机调节句柄
	@Parameter	: 	gain_open_expotime_in			--当曝光时间大于该参数时，启用增益。使用增益后，曝光时间也不超过该值. 单位ms
	@Parameter	: 	gain_max_in						--最大增益倍数，不小于1。 1表示不启用增益
	@Returns	:   AlgoError
	@Author		：	TianBo
	**************************************/
	C_API AlgoError C_CALL CameraParamSetGainParam(AlgoHandle camerapara_handle, int gain_open_expotime_in, float gain_max_in);



	/************************************
	@Brief		：	设置相机预调节类型
	@Note		：  （决定调光圈微焦还是调灰阶）CameraParamAnalyzePreconditioning
	@Parameter	: 	camerapara_handle			--有效的handle
	@Parameter	: 	in_camparam_adjustmethod	--相机预调节方法，默认ADJUSTMETHOD_KEEPGRAY 调光圈微焦，灰阶不变
	@Returns	:   AlgoError
	@Author		：	lvyang@2025/05/06
	************************************/
	C_API AlgoError C_CALL CameraParamSetAdjustType(AlgoHandle camerapara_handle,
		CamParamAdjustMethod in_camparam_adjustmethod = CamParamAdjustMethod::ADJUSTMETHOD_KEEPGRAY);

	/************************************
	@Brief		：  设置预调节参数
	@Note		：
	@Parameter	: 	camerapara_handle				--相机调节句柄
	@Parameter	: 	q_preconditoning_max_in			--实时调节过程中质量Q的最大值
	@Parameter	: 	q_preconditoning_min_in			--实时调节过程中质量Q的最小值
	@Parameter	: 	expotime_preconditoninge_max_in	--实时调节过程中曝光时间最大值（绿色Y）
	@Parameter	: 	expotime_preconditoninge_max_in --实时调节过程中曝光时间最小值（绿色Y）
	@Returns	:   AlgoError
	@Author		：	TianBo
	**************************************/
	C_API AlgoError C_CALL CameraParamSetPreconditioningParam(AlgoHandle camerapara_handle, float q_preconditoning_max_in, float q_preconditoning_min_in, int expotime_preconditoninge_max_in, int expotime_preconditoning_min_in);




	/************************************
	@Brief		：	预调节分析，根据CamParamAdjustMethod 选择不同的方案 默认为ADJUSTMETHOD_KEEPGRAY
	@Note		：  调节完毕后，使用该参数进行曝光推算。
					1、ADJUSTMETHOD_KEEPCAMPARAM 调节必须 按照RX BZ GY 顺序
					2、当类型为ADJUSTMETHOD_KEEPCAMPARAM ，在capture_param_in_out获取调节后的灰阶；
					3、当类型为	ADJUSTMETHOD_KEEPGRAY，camera_op_out指导相机调节动作
					4、相机采集参数统一在capture_param_in_out

	@Parameter	: 	camerapara_handle			--有效的handle
	@Parameter	: 	capture_param_in_out		--采集句柄，在这里拿曝光、增益、灰阶、颜色、滤片等信息。 ND暂不可用。 当未完成调节时，该参数包含下一次采集时的曝光和增益
	@Parameter	: 	quality_in					--capture_param_in_out参数下对应的质量参数
	@Parameter	: 	camera_op_out				--相机调节句柄，当类型为	ADJUSTMETHOD_KEEPGRAY，指导相机调节动作
	@Parameter	: 	is_finished					--是否调节完成。
	@Returns	:   AlgoError
	@Author		：	TianBo@2024/03/18
	************************************/
	C_API AlgoError C_CALL CameraParamAnalyzePreconditioning(AlgoHandle camerapara_handle, CamControllerCaptureParam* capture_param_in_out, AlgoPointQuality quality_in,
		CameraOperation* camera_op_out, bool* is_finished);


	/************************************
	@Brief		：	根据亮度衰减比例，预调节校正灰阶
	@Note		：  输入设置灰阶，输出预调节后的灰阶
	@Parameter	: 	in_out_capture_param		--仅输出灰阶。 eg:衰减比例10%,输出gamma*0.9后的灰阶；衰减比例-10%,输出gamma/0.9后的灰阶；
	@Parameter	: 	in_gamma_rgb				--gamma表 尺寸为256x3(wh)
	@Parameter	: 	in_lum_dec_ratio_rgb		-- rgb 亮度衰减比例; rgb必须相同
	@Returns	:   AlgoError
	@Author		：	lvyang@2025/03/17
	************************************/
	C_API AlgoError C_CALL CameraCaptureParamAdjust(const AlgoMat* in_gamma_rgb, const float* in_lum_dec_ratio_rgb,
		CamControllerCaptureParam* in_out_capture_param);


	/************************************
	@Brief		：	根据相机调节结果向下截断全灰阶校正灰阶
	@Note		：  相机实时分析结束后，全灰阶曝光估算前调用
	@Parameter	: 	in_gamma_rgb					--gamma表 尺寸为256x3(wh)
	@Parameter	: 	in_propose_max_gray				--相机实时分析结束后推荐的最高校正灰阶
	@Parameter	: 	in_out_multilayer_layer			--输入全灰阶采集灰阶，输出调整后的采集灰阶, 灰阶顺序由低到高
	@Parameter	: 	in_allow_min_gray				--允许的全灰阶最低灰阶
	@Parameter	: 	in_out_multilayer_layer_nums	--in_multilayer_layer灰阶个数
	@Returns	:   AlgoError
	@Author		：	lvyang@2025/05/06
	************************************/
	C_API AlgoError C_CALL CameraParamMultiLayerAdjust(const AlgoMat* in_gamma_rgb, const int in_propose_max_gray, int* in_out_multilayer_layer, int in_out_multilayer_layer_nums = 0);


	/************************************
	@Brief		：  设置全灰阶相关参数
	@Note		：  必须调用该接口
	@Parameter	: 	camerapara_handle				--相机调节句柄
	@Parameter	: 	in_rgb_gamma					--rgb gamma。 尺寸为256x3(wh)
	@Parameter	: 	in_multilayer_layer				--全灰阶采集灰阶
	@Parameter	: 	in_multilayer_layer_nums		--in_multilayer_layer灰阶个数
	@Parameter	: 	low_saturation_max_in			--低灰时的最大饱和度。   允许低灰时的饱和度低一些
	@Parameter	: 	low_saturation_min_in			--低灰时的最小饱和度。
	@Parameter	: 	low_grayscal_threshold_in		--低灰灰阶的分界线。
	@Parameter	: 	is_w_param_in					--输入参数模式 rgb和w 需设置两套参数（调两次该接口）。
	@Returns	:   AlgoError
	@Author		：	TianBo
	**************************************/
	C_API AlgoError C_CALL CameraParamSetMultiLayerParam(AlgoHandle camerapara_handle, AlgoMat* rgb_gamma_in, int* multilayer_layer_in, int multilayer_layer_nums_in,
		float low_saturation_max_in, float low_saturation_min_in, int low_grayscal_threshold_in, bool is_w_param_in = false);

	/************************************
	@Brief		：  设置需要进行全灰阶推算Check的分量
	@Note		：  注意：参数是RGBW-XYZ全量推算，这里state表示是否需要Check该分量,具体到某一层某一颜色某一滤片.
					不调用该接口时，，默认去推RGB-Y
					全量Check时参数范围会很窄，建议根据需要Check.

	@Parameter	: 	camerapara_handle		--相机调节句柄
	@Parameter	: 	in_estimate_option	    --推算选择，尺寸为n*4*3(wh),algomat结构体数组、去推算Check每层的RGBW-XYZ，1为Check该分量，0为不Check。
	@Parameter	: 	in_estimate_option_num	--输入in_estimate_option结构体数组大小。
	@Returns	:   AlgoError
	@Author		：	lvyang20250923
	**************************************/
	C_API AlgoError C_CALL CameraParamSetEstimateState(AlgoHandle camerapara_handle, const AlgoMat* in_estimate_option, int in_estimate_option_num);

	/************************************
	@Brief		：	相机参数推算，推算单灰阶的曝光时间
	@Note		：	根据RGBW-XYZ中任意单个分量去推算RGBW-XYZ 曝光增益 RZ除外 
					默认推算RGBW同灰阶参数; 推算前调CameraParamSetMultiLayerParam设置灰阶参数后可支持RGBW不同灰阶参数推算。
	@Parameter	: 	camerapara_handle			--有效的handle
	@Parameter	: 	capture_param_in			--采集句柄，包含曝光、增益、灰阶、颜色、滤片等信息。该参数应该是分析后符合要求返回的
	@Parameter	: 	quality_in					--capture_param_in参数下对应的质量参数
	@Returns	:   AlgoError
	@Author		：	TianBo@2024/03/18
	************************************/
	C_API AlgoError C_CALL CameraParamEstimateSingleGray(AlgoHandle camerapara_handle, CamControllerCaptureParam capture_param_in, AlgoPointQuality quality_in);


	/************************************
	@Brief		：	相机参数推算，推算单灰阶的曝光时间, 用于使用亮度分析好的handle来推算色度handle
	@Note		：	注意：如输入handle单灰阶参数RGB与W最高灰不同，这里推算的也是对应灰阶结果。
	@Parameter	: 	camerapara_handle				--有效的handle
	@Parameter	: 	cameraparam_singlegray_handle --采集句柄，包含曝光、增益、灰阶、颜色、滤片等信息。该参数应该是分析后符合要求返回的
	@Returns	:   AlgoError
	@Author		：	TianBo@2024/03/18
	************************************/
	C_API AlgoError C_CALL CameraParamEstimateSingleGrayUsingLumHandle(AlgoHandle camerapara_handle, AlgoHandle cameraparam_singlegray_handle);

	/************************************
	@Brief		：	根据已有单层相机参数和原始值目标值，推算目标值对应的新参数
	@Note		：	仅支持单层handle输入，同理输出单层的结果 
	@Parameter	: 	in_ori_val							-- 原始值 校正最高灰RGBW
	@Parameter	: 	in_tar_val							-- 目标值 校正最高灰RGBW
	@Parameter	: 	in_out_cameraparam_handle			-- 有效的handle。输入RGB参数应该是在屏体亮度不衰减分析后符合要求返回的(W不强制要求)。
														
	@Returns	:   AlgoError
	@Author		：	lvyang@2026/01/26
	************************************/
	C_API AlgoError C_CALL CameraParamInferenceByRGBWXYZ(const RGBWXYZ in_ori_val, const RGBWXYZ in_tar_val, AlgoHandle in_out_cameraparam_handle);

	/************************************
	@Brief		：	相机参数推算，推算单灰阶的曝光时间
	@Note		：	支持根据用户输入全灰阶推算State, 计算需要的全灰阶RGBW-XYZ任意分量。默认推RGB-Y。
	@Parameter	: 	camerapara_handle				--有效的handle
	@Parameter	: 	cameraparam_singlegray_handle	--有效的单灰阶推算句柄
	@Parameter	: 	camera_op_out					--相机调节参数，全灰阶可能会存在中间调节灰阶
	@Parameter	: 	interrupt_grayscale_out			--对应camera_op_out操作的灰阶
	@Parameter	: 	allow_capture_lowest_gray		--在不调节光圈的情况下，允许最低校正灰阶
	@Parameter	: 	allow_capture_highest_gray		--调整光圈后，允许最高校正灰阶 参考值（可能比当前最高灰大）
	@Parameter	: 	is_finished						--是否调节完成。 为false时，allow_capture_lowest_gray、allow_capture_highest_gray、interrupt_grayscale_out、camera_op_out有效。 为true时表示当前曝光参数推算完毕
	@Returns	:   AlgoError
	@Author		：	TianBo@2024/03/18
	************************************/
	C_API AlgoError C_CALL CameraParamEstimateMultiLayer(AlgoHandle camerapara_handle, AlgoHandle cameraparam_singlegray_handle, CameraOperation* camera_op_out, 
		int* interrupt_grayscale_out, int* allow_capture_lowest_gray, int* allow_capture_highest_gray, bool* is_finished);


	/************************************
	@Brief		：	获取曝光时间
	@Note		：	旧版本，仅支持RGBW同灰阶调节的场景，设置RGBW不同校正灰阶时，会报错
					单灰阶结果：RGB-XYZ OR W-XYZ    size = 3*3
					全灰阶结果：每层RGB-Y OR W-XYZ  size = n*3
					
	@Parameter	: 	camerapara_handle				--有效的handle
	@Parameter	: 	expotime_out					--获取的曝光时间
	@Parameter	: 	gain_out						--获取的增益，位置与曝光一致
	@Parameter	: 	saturation_out					--获取对应的饱和度
	@Parameter	: 	state_out						--获取对应的状态信息
	@Returns	:   AlgoError
	@Author		：	TianBo@2024/03/18
	************************************/
	[[deprecated("This function is deprecated and will be removed in a future version. plsease use CameraParamGetSingleExpotime replace this function ")]]
	C_API AlgoError C_CALL CameraParamGetExpotimeGain(AlgoHandle camerapara_handle, AlgoMat* expotime_out, AlgoMat* gain_out, AlgoMat* saturation_out, AlgoMat* state_out);
	
	/************************************
	@Brief		：	获取指定灰阶层的曝光时间
	@Note		：	格式为 RX RY RZ GX GY GZ BX BY BZ WX WY WZ 共计12个CamControllerCaptureParam 结构体

	@Parameter	: 	camerapara_handle				--有效的handle
	@Parameter	: 	in_gray_value					--输入需要获取参数的灰阶
	@Parameter	: 	capture_param_out				--输出采集参数，结构体数组指针
	@Parameter	: 	in_out_num					    --结构体数量, 小于12时按顺序裁剪后输出
	@Returns	:   AlgoError
	@Author		：	lvyang@2025/09/23
	************************************/
	C_API AlgoError C_CALL CameraParamGetSingleExpotime(AlgoHandle camerapara_handle, int in_gray_value, CamControllerCaptureParam * capture_param_out, int *in_out_num);
	
	/************************************
	@Brief		：	设置曝光时间与增益，支持两种模式
	@Note		：	单层灰阶输入格式为 RX RY RZ GX GY GZ BX BY BZ WX WY WZ 共计12个CamControllerCaptureParam 结构体
					多层灰阶输入格式为 12 * layer 个CamControllerCaptureParam，由低到高灰阶
	@Parameter	: 	camerapara_handle				--有效的handle
	@Parameter	: 	in_capture_param				--输入采集参数，结构体数组指针,输入顺序由低到高灰阶
	@Parameter	: 	in_num							--结构体数量, 必须为12的倍数
	@Returns	:   AlgoError
	@Author		：	lvyang@2026/01/29
	************************************/
	C_API AlgoError C_CALL CameraParamSetExpotimeGain(AlgoHandle camerapara_handle, CamControllerCaptureParam * in_capture_param, int in_num);

	/************************************
	@Brief		：	获取指定灰阶层的参数推算状态以及饱和度
	@Note		：	
	@Parameter	: 	camerapara_handle				--有效的handle
	@Parameter	: 	in_gray_value					--输入需要获取参数的灰阶
	@Parameter	: 	out_state						--输出指定灰阶层的参数推算状态， 4*3 Mat 每行代表颜色，每列代表滤片
	@Parameter	: 	out_saturation					--输出指定灰阶层的饱和度， 4*3 Mat
	@Returns	:   AlgoError
	@Author		：	lvyang@2025/09/23
	************************************/
	C_API AlgoError C_CALL CameraParamGetSingleState(AlgoHandle camerapara_handle, int in_gray_value, AlgoMat* out_state, AlgoMat* out_saturation);

	/************************************
	@Brief		：	设置参数推算状态以及饱和度，支持两种模式
					单层输入：in_gray_num=1,
					多层输入：in_gray_list与in_state和in_saturation顺序对应，由低到高灰阶
	@Note		：
	@Parameter	: 	camerapara_handle				--有效的handle
	@Parameter	: 	in_gray_list					--输入灰阶，多层输入时由低到高灰阶
	@Parameter	: 	in_gray_num						--输入灰阶数量 =1是表示只设置单层
	@Parameter	: 	in_state						--输入对应灰阶层的参数推算状态，in_gray_num 个 4*3 Mat
	@Parameter	: 	in_saturation					--输入对应灰阶层的饱和度， in_gray_num 个 4*3 Mat
	@Returns	:   AlgoError
	@Author		：	lvyang@2026/01/29
	************************************/
	C_API AlgoError C_CALL CameraParamSetState(AlgoHandle camerapara_handle, int *in_gray_list,int in_gray_num, AlgoMat* in_state , AlgoMat* in_saturation);


	/************************************
	@Brief		：	相机参数实时分析
	@Note		：
	@Parameter	: 	camerapara_handle			--有效的handle
	@Parameter	: 	capture_param_in_out		--采集句柄，包含曝光、增益、灰阶、颜色、滤片等信息。 ND暂不可用。 当未完成调节时，该参数包含下一次采集时的曝光和增益
	@Parameter	: 	quality_in					--capture_param_in_out参数下对应的质量参数
	@Parameter	: 	is_finished					--是否调节完成。
	@Returns	:   AlgoError
	@Author		：	TianBo@2024/03/18
	************************************/
	C_API AlgoError C_CALL CameraParamAnalyze(AlgoHandle camerapara_handle, CamControllerCaptureParam* capture_param_in_out, AlgoPointQuality quality_in, bool* is_finished);

	/************************************
	@brief  : 根据衰减推算相机曝光时间
	@note   : ...
	+--------------------------------
	@param  : camerapara_handle			-- 有效的handle
	@param  : in_camerapara_no_dec		-- 未衰减的handle
	@param  : dec						-- 衰减{rgb}
	@param  : n							-- dec数组长度
	+--------------------------------
	@return :
	@Author : YangShangqi@2024/04/02
	************************************/
	C_API AlgoError C_CALL CameraParamDec(AlgoHandle camerapara_handle, float* dec, int n = 3);

	/************************************
	@brief  : 设置单灰阶已经调节好的曝光参数
	@note   : ...
	+--------------------------------
	@param  : camerapara_handle			-- 有效的handle
	@param  : gary_lv					-- 调节灰阶
	@param  : rgb_xyz_expotime			-- 曝光时间
	@param  : rgb_xyz_gain				-- 增益
	@param  : rgb_xyz_saturation		-- 饱和度
	+--------------------------------
	@return :
	@Author : YangShangqi@202X/XX/XX
	************************************/
	C_API AlgoError C_CALL CameraParamSetSingleLayerExposure(AlgoHandle camerapara_handle, CamControllerCaptureParam* cam_param_in, AlgoPointQuality* q_in, int num);

	/************************************
	@brief  : 拷贝一个句柄中的数据到另一中
	@note   : ...
	+--------------------------------
	@param  : camerapara_handle    -- ...
	@param  : in_out_camerapara    -- ...
	+--------------------------------
	@return :
	@Author : YangShangqi@202X/XX/XX
	************************************/
	C_API AlgoError C_CALL CameraParamCopyTo(AlgoHandle camerapara_handle, AlgoHandle out_camerapara);


	C_API AlgoError C_CALL CameraParaRelease(AlgoHandle camerapara_handle);


	/************************************
	@brief  : 保存载入
	@note   : 后缀只能是.cameraparam  存储全量参数
	@param  : camerapara_handle  --有效的camerapara_handle句柄
	@param  : in_fullname   --后缀只能是.cameraparam
	@return	: AlgoError
	@Author : tianbo@2024/03/21
	************************************/
	C_API AlgoError C_CALL CameraParamSave(AlgoHandle camerapara_handle, const char* in_fullname);
	C_API AlgoError C_CALL CameraParamLoad(AlgoHandle camerapara_handle, const char* in_fullname);

	C_API AlgoError C_CALL CameraParamGetBufferSize(AlgoHandle meter_handle, uint64_t* out_len);
	C_API AlgoError C_CALL CameraParamToBuffer(AlgoHandle meter_handle, char* out_buffer, uint64_t* in_out_len);
	C_API AlgoError C_CALL CameraParamFromBuffer(AlgoHandle meter_handle, const char* in_buffer, uint64_t in_len);

	C_API void C_CALL CameraParamDestroyHandle(AlgoHandle camerapara_handle);



	//**********************************多色质量调节接口************************************************
	//目的：得到RGB成像占比符合要求的微焦状态。


	/************************************
	@Brief		：	多颜色预览调节参数设定,
	@Note		：	在灰阶推荐、估计过程中使用。
	@Parameter	: 	camerapara_handle				--有效的handle
	@Parameter	: 	in_gamma_table					--Set gamma参数。为采集过程中屏体的gamma，3x256(wh)，
	@Returns	:   AlgoError
	@Author		：	田博@2025/1/10
	*/
	C_API AlgoError C_CALL CameraParamSetGamma(AlgoHandle camerapara_handle, AlgoMat* in_gamma_table);



	/************************************
	@Brief		：	多颜色预览调节参数设定
	@Note		：	set当rgb同时调节时相关参数，包含基准灰阶对RGB的饱和度以及曝光时间调节的范围。 越宽泛，则后续调节灰阶的概率越小。
	@Parameter	: 	camerapara_handle				--有效的handle
	@Parameter	: 	calib_gray_scal					--校正灰阶，调节成功后，可直接输出用于后续调节的GY
	@Parameter	: 	in_rgb_s_max					--RGB在多色调节过程中的最大饱和度（灯点峰值）。长度必须为3，顺序为RGB
	@Parameter	: 	in_rgb_s_min					--RGB在多色调节过程中的最小饱和度（灯点峰值）。长度必须为3，顺序为RGB
	@Parameter	: 	in_num							--in_rgb_s_max的长度，现阶段必须为3
	@Parameter	: 	exptime_max						--RGB在多色调节过程中的最大曝光时间.默认建议100
	@Parameter	: 	exptime_max						--RGB在多色调节过程中的最小曝光时间.默认建议600
	@Returns	:   AlgoError
	@Author		：	田博@2025/1/10
	*/
	C_API AlgoError C_CALL CameraParamSetRGBAdjustParam(AlgoHandle camerapara_handle, int calib_gray_scal,
		float* in_rgb_s_max, float* in_rgb_s_min, int in_num, int exptime_max, int exptime_min);


	//RGB三色调节方案，最终可得到GY满足校正灰阶的峰值、占比、曝光时间的以及RB满足成像占比的成像状态
	/************************************
	@Brief		：	多颜色预览调节
	@Note		：	在返回ALGO_PASS时： 当is_finished = true，表示以调节完毕，可以使用out_gy_calib_gray_capture_param参数在校正灰阶分析GY。
									   当is_finished = false,有两种情况：
																case 0(camera_op_out.microfocal_op = MicroFocalOperation::MICROFOCAL_CHANGED): 表示当前状态无法调节，需要调节微焦。
																case 1(camera_op_out.microfocal_op = MicroFocalOperation::MICROFOCAL_NOCHANGED):表示当前状态可调节，根据输出调节曝光、增益、灰阶即可。

	@Parameter	: 	camerapara_handle				--有效的handle
	@Parameter	: 	in_out_current_capture_param	--各个颜色当前次采集状态，顺序必须为RGB。
	@Parameter	: 	in_qualitys						--in_out_current_capture_param采集状态下的成像质量，饱和度和峰值均为x100后的结果。
	@Parameter	: 	in_nums							--in_out_current_capture_param和in_qualitys的长度。目前必须为3。
	@Parameter	: 	camera_op_out					--指导相机调节. camera_op_out.microfocal_op = MicroFocalOperation::MICROFOCAL_CHANGED时，表示需要调节微焦或者光圈。
	@Parameter	: 	out_gy_calib_gray_capture_param	--输出的GY在校正灰阶下的相机采集参数。该参数在is_finished = true时有效
	@Parameter	: 	is_finished						--是否调节成功。
	@Returns	:   AlgoError
	@Author		：	田博@2025/1/10
	************************************/
	C_API AlgoError C_CALL CameraParamMultiColorPreconditioning(AlgoHandle camerapara_handle, CamControllerCaptureParam* in_out_current_capture_param,
		AlgoPointQuality* in_qualitys, int in_nums,
		CameraOperation* camera_op_out, CamControllerCaptureParam* out_gy_calib_gray_capture_param, bool* is_finished);

	/************************************
	@Brief		：	多颜色预览调节，初始化打屏灰阶
	@Note		：
	@Parameter	: 	in_gamma_table		        --屏体当前的gamma表
	@Parameter	: 	in_calib_gray		        --校正灰阶
	@Parameter	: 	in_multicolor_lum_ratio		--多颜色数目，暂时仅支持RGB 3个颜色， 表示打屏时RGB的亮度比例。默认可：红：绿：蓝 = 3：1：6
	@Parameter	: 	in_multicolor_number		--上述多颜色的数量，目前仅支持为3
	@Parameter	: 	out_multicolor_gray		    --输出RGB的灰阶，顺序为RGB。其中G为校正灰阶（目的为了确保曝光时间）。
	@Returns	:   AlgoError
	@Author		：	田博@2025/1/10
	************************************/
	C_API AlgoError C_CALL CameraParamGetMultiColorGray(AlgoMat* in_gamma_table, const int in_calib_gray, const float* in_multicolor_lum_ratio,
		const int in_multicolor_number, int* out_multicolor_gray);










	//旧接口

	/************************************
	@brief  : 创建相机调节句柄
	@note   :
	@param  : out_camera_handle    相机调节句柄
	@return	: AlgoError
	@Author : Laihaokun@2022/06/10
	************************************/
	C_API AlgoError C_CALL CameraParaCreateHandle(AlgoHandle* out_camerapara_handle);


	/************************************
	@Brief		：  配置当前相机参数
	@Note		：  可以输入为黑色
	@Parameter	: 	camerapara_handle       --相机调节句柄
	@Parameter	: 	Expotime                --当前曝光时间
	@Parameter	: 	Grayscale		        --当前打屏亮度 
	@Parameter	: 	Step                    --迭代次数 
	@Parameter	: 	color       	        --颜色，红绿蓝：
	@Parameter	: 	filter       	        --滤片，XYZ
    @Parameter	: 	lum_dec_ratio_in       	--亮度损失，>0,默认为1，即一次校正
	@Parameter	: 	Aperture_       	    --光圈，可不输入
	@Returns	:   AlgoError
	@Author		：	LaiHaokun@2021/08/02
	************************************/
	[[deprecated("This function is deprecated and will be removed in a future version. plsease use CameraParamSeCommonParam replace this function ")]]
	C_API AlgoError C_CALL CameraParaSetPara(AlgoHandle camerapara_handle, const int Expotime, const int Grayscale, const int Step, const AlgoColor color, const CamFilter filter, const float lum_dec_ratio_in, int Aperture_ = 0);



	//---------1 set lgcoef info

	/************************************
	@Brief		：	决策当前图像调节
	@Note		：
	@Parameter	: 	camerapara_handle       --相机调节句柄
	@Parameter	: 	quality_in_handle       --输入质量参数
	@Parameter	: 	Expotime_out		    --输出曝光时间
	@Parameter	: 	State_out		        --输出当前状态: 0:待继续调节；1：调节成功；2：屏幕过亮；3：屏幕过暗。
	@Parameter	: 	TargetQ_out		        --输出目标质量

	@Returns	:   AlgoError
	@Author		：	LaiHaokun@2021/08/02
	************************************/
	[[deprecated("This function is deprecated and will be removed in a future version. plsease use CameraParamAnalyzePreconditioning replace this function ")]]
	C_API AlgoError C_CALL CameraParaAnalyze(AlgoHandle camerapara_handle, const AlgoPointQuality *in_point_quality, int &Expotime_out, int &State_out, int &TargetQ_out);


	/************************************
	@Brief		：	自动调节时评估初始曝光时间
	@Note		：  当输入基准颜色为黑色时，基准曝光时间与绿色相同。调节自身颜色为黑色，滤片为XYZ时，曝光时间与基准曝光时间一致
	@Parameter	: 	camerapara_handle       --相机调节句柄
	@Parameter	: 	Expotime_in             --输入调节基准颜色滤片的曝光时间
	@Parameter	: 	color_in                --输入调节基准颜色
    @Parameter	: 	filter_in               --输入调节基准滤片
	@Parameter	: 	Expotime_out		    --输出曝光时间
	@Parameter	: 	State_out		        --输出当前状态： 0:待继续调节；1：调节成功；2：屏幕过亮；3：屏幕过暗。
	@Returns	:   AlgoError
	@Author		：	LaiHaokun@2021/08/02
	************************************/
	[[deprecated("This function is deprecated and will be removed in a future version. plsease use CameraParamEstimateSingleGray replace this function ")]]
	C_API AlgoError C_CALL CameraParaEstimate(AlgoHandle camerapara_handle, const int Expotime_in, const AlgoColor color_in, const CamFilter filter_in, int &Expotime_out, int &State_out);

	/************************************
    @Brief		：	设置成像质量区间
    @Note		：
    @Parameter	: 	S_max_in                --灯点峰值上限
	@Parameter	: 	S_min_in                --灯点峰值下限
    @Parameter	: 	Q_max_in                --灯点大小上限
    @Parameter	: 	Q_min_in                --灯点大小上限
    @Returns	:   AlgoError
    @Author		：	LaiHaokun@2021/08/02
    ************************************/
	C_API AlgoError C_CALL CameraParaSetQualityRange(AlgoHandle camerapara_handle, const float S_max_in, const float S_min_in, const float Q_max_in, const float Q_min_in);

	/************************************
    @Brief		：	设置曝光时间区间
    @Note		：
    @Parameter	: 	Expotime_max                --曝光时间上限
    @Parameter	: 	Expotime_min                --曝光时间下限
    @Returns	:   AlgoError
    @Author		：	LaiHaokun@2021/08/02
    ************************************/
	C_API AlgoError C_CALL CameraParaSetExpotimeRange(AlgoHandle camerapara_handle, const int Expotime_min, const int Expotime_max);


	C_API AlgoError C_CALL CameraParaGetQualityRange(AlgoHandle camerapara_handle, float &Qmin, float &Qmax, float &Smin, float &Smax);

	/************************************
	@Brief		：  配置当前相机参数（超分）
	@Note		：
	@Parameter	: 	camerapara_handle       --相机调节句柄
	@Parameter	: 	in_expotime             --当前曝光时间
	@Parameter	: 	in_grayscale		    --当前打屏亮度
	@Parameter	: 	in_color       	        --颜色，红绿蓝
	@Parameter	: 	in_filter       	    --滤片，XYZ
	@Parameter	: 	in_gain_db	       		--当前曝光下的增益, db
	@Returns	:   AlgoError
	@Author		：	TianBo@2022/06/21
	************************************/
	[[deprecated("This function is deprecated and will be removed in a future version. plsease use CameraParamSeCommonParam replace this function ")]]
	C_API AlgoError C_CALL CameraParaSetParaForSuperResolution(AlgoHandle camerapara_handle, int in_expotime, int in_grayscale, AlgoColor in_color, CamFilter in_filter, float in_gain_db = 0.f);




	/************************************
	@Brief		：  设置全灰阶相机调节的相关参数
	@Note		：
	@Parameter	: 	camerapara_handle			--相机调节句柄
	@Parameter	: 	in_rgb_gamma				--rgb gamma size为256x3（wh),type:ALGO_32SC1,目前仅支持rgb gamma一致
	@Parameter	: 	in_multilayer_layer			--全灰阶校正的灰阶，灰阶从低到高排列
	@Parameter	: 	in_multilayer_layer_nums    --in_multilayer_layer中数据个数
	@Parameter	: 	in_lum_dec_ratio			--rgb的衰减，为3长度的数组，亮度衰减，0-1的值，表示衰减了多少
	@Parameter	: 	in_low_saturation_max       --全灰阶低灰下的最高饱和度
	@Parameter	: 	in_low_saturation_min       --全灰阶低灰下的最低饱和度
	@Parameter	: 	in_gain_open       			--启用增益时的曝光时间，默认大于10s时启用增益（色度校正不启用增益）
	@Parameter	: 	in_gain_max       			--最大增益
	@Parameter	: 	in_is_iteration      			--是否是迭代全灰阶过程
	@Returns	:   AlgoError
	@Author		：	TianBo@2022/06/21
	************************************/
	[[deprecated("This function is deprecated and will be removed in a future version. plsease use CameraParamSetMultiLayerParam replace this function ")]]
	C_API AlgoError C_CALL CameraParaSetParaForMultiLayer(AlgoHandle camerapara_handle, AlgoMat* in_rgb_gamma, int* in_multilayer_layer, int in_multilayer_layer_nums,
		float* in_lum_dec_ratio, int in_num_dec, float in_low_saturation_max, float in_low_saturation_min, int in_gain_open = 10000, float in_gain_max = 4.f, bool in_is_iteration = true);



	/************************************
	@Brief		：  设置色度校正的曝光时间以及饱和度
	@Note		：
	@Parameter	: 	camerapara_handle			--相机调节句柄
	@Parameter	: 	in_expotime_rgb				--红绿蓝亮度的曝光时间
	@Parameter	: 	in_saturation_rgb			--绿蓝亮度对应的饱和度
	@Parameter	: 	in_num						--指针数据的长度，为3
	@Parameter	: 	in_grayscale				--当前设置参数对应的灰阶
	@Returns	:   AlgoError
	@Author		：	TianBo@2022/07/25
	************************************/
	C_API AlgoError C_CALL CameraParaSetExpotimeAndSaturation(AlgoHandle camerapara_handle, int* in_expotime_rgb, float* in_saturation_rgb, int in_num, int in_grayscale);



	/************************************
	@Brief		：  设置超分相机预览时的成像参数
	@Note		：
	@Parameter	: 	camerapara_handle				--相机调节句柄
	@Parameter	: 	in_Q_real_time_max				--实时调节过程中质量Q的最大值
	@Parameter	: 	in_Q_real_time_min				--实时调节过程中质量Q的最小值
	@Parameter	: 	in_expotime_real_time_max		--实时调节过程中曝光时间最大值（绿色Y）
	@Parameter	: 	in_expotime_real_time_min      	--实时调节过程中曝光时间最小值（绿色Y）
	@Returns	:   AlgoError
	@Author		：	TianBo@2022/06/21
	************************************/
	[[deprecated("This function is deprecated and will be removed in a future version. plsease use CameraParamSetPreconditioningParam replace this function ")]]
	C_API AlgoError C_CALL CameraParaSetRealTimeParaRange(AlgoHandle camerapara_handle, float in_Q_real_time_max, float in_Q_real_time_min, int in_expotime_real_time_max, int in_expotime_real_time_min);


	/************************************
	@Brief		：  超分成像过程中预览分析
	@Note		：
	@Parameter	: 	camerapara_handle				--相机调节句柄
	@Parameter	: 	quality_in						--当前质量参数
	@Parameter	: 	out_expotime					--下次预览时的曝光时间
	@Parameter	: 	CameraOperation					--需要对相机调节的动作
	@Parameter	: 	is_finished						--是否完成调节
	@Returns	:   AlgoError
	@Author		：	TianBo@2022/06/21
	************************************/
	[[deprecated("This function is deprecated and will be removed in a future version. plsease use CameraParamAnalyzePreconditioning replace this function ")]]
	C_API AlgoError C_CALL CameraParaAnalyzeRealTimeForSuperResolution(AlgoHandle camerapara_handle, AlgoPointQuality* in_quality, int* out_expotime, CameraOperation* out_camera_op, bool* is_finished);



	/************************************
	@Brief		：  色度校正曝光时间的估计
	@Note		：
	@Parameter	: 	camerapara_handle				--相机调节句柄
	@Returns	:   AlgoError
	@Author		：	TianBo@2022/06/21
	************************************/
	[[deprecated("This function is deprecated and will be removed in a future version. plsease use CameraParamEstimateSingleGrayUsingLumHandle replace this function ")]]
	C_API AlgoError C_CALL CameraParaEstimateForSuperResolution(AlgoHandle camerapara_handle);


	/************************************
	@Brief		：  全灰阶曝光时间的估计
	@Note		：
	@Parameter	: 	camerapara_handle				--相机调节句柄
	@Parameter	: 	out_camera_op					--需要对相机调节的动作
	@Parameter	: 	out_interrupt_grayscale			--参数估计中断的灰阶（即该灰阶需要调节光圈）
	@Parameter	: 	out_interrupt_color				--参数估计中断的COLOR
	@Parameter	: 	is_finished						--参数估计是否已经完成
	@Returns	:   AlgoError
	@Author		：	TianBo@2022/06/21
	************************************/
	[[deprecated("This function is deprecated and will be removed in a future version. plsease use CameraParamEstimateMultiLayer replace this function ")]]
	C_API AlgoError C_CALL CameraParaEstimateForMultiLayerSuperResolution(AlgoHandle camerapara_handle,
		CameraOperation* out_camera_op, int* out_interrupt_grayscale, AlgoColor* out_interrupt_color, bool* is_finished);




	/************************************
	@Brief		：  超分色度参数实际分析过程
	@Note		：
	@Parameter	: 	camerapara_handle				--相机调节句柄
	@Parameter	: 	in_quality						--当前的质量参数
	@Parameter	: 	out_expotime					--下一次迭代时的曝光时间
	@Parameter	: 	is_finished						--是否调节ok
	@Returns	:   AlgoError
	@Author		：	TianBo@2022/06/21
	************************************/
	C_API AlgoError C_CALL CameraParaAnalyzeForSuperResolution(AlgoHandle camerapara_handle, AlgoPointQuality* in_quality, int* out_expotime, bool* is_finished);


	/************************************
	@Brief		：  超分全灰阶参数实际分析过程
	@Note		：
	@Parameter	: 	camerapara_handle				--相机调节句柄
	@Parameter	: 	in_quality						--当前的质量参数
	@Parameter	: 	out_expotime					--下一次迭代时的曝光时间
	@Parameter	: 	out_gain						--下一次迭代时的增益
	@Parameter	: 	out_camera_op					--需要对相机调节的动作
	@Parameter	: 	is_finished						--是否调节ok
	@Returns	:   AlgoError
	@Author		：	TianBo@2022/06/21
	************************************/
	[[deprecated("This function is deprecated and will be removed in a future version. plsease use CameraParamAnalyze replace this function ")]]
	C_API AlgoError C_CALL CameraParaAnalyzeForMultiLayerSuperResolution(AlgoHandle camerapara_handle,
		AlgoPointQuality* in_quality, int* out_expotime, float* out_gain_db, CameraOperation* out_camera_op, bool* is_finished);


	/************************************
	@Brief		：  获取超分色度曝光时间
	@Note		：
	@Parameter	: 	camerapara_handle				--相机调节句柄
	@Parameter	: 	out_rgb_xyz_expotime			--曝光时间，为3x3矩阵，第一行表示Rx,Ry,Rz的曝光时间。type:ALGO_32SC1
	@Parameter	: 	out_rgb_xyz_state				--曝光时间对应的状态，为3x3矩阵，第一行表示Rx,Ry,Rz的状态。type:ALGO_32SC1
	@Parameter	: 	in_is_deep_copy
	@Returns	:   AlgoError
	@Author		：	TianBo@2022/06/21
	************************************/
	[[deprecated("This function is deprecated and will be removed in a future version. plsease use CameraParamGetSingleExpotime replace this function ")]]
	C_API AlgoError C_CALL CameraParaGetExpotimeForSuperResolution(AlgoHandle camerapara_handle, AlgoMat* out_rgb_xyz_expotime, AlgoMat* out_rgb_xyz_state, int in_is_deep_copy = 1);

	/************************************
	@Brief		：  获取超分全灰阶曝光时间
	@Note		：
	@Parameter	: 	camerapara_handle					--相机调节句柄
	@Parameter	: 	out_rgb_multilayer_expotime			--曝光时间，为3xn矩阵，n表示灰阶数量，第一行表示R从低灰到高亮衰减后的曝光时间. type:ALGO_32SC1
	@Parameter	: 	out_rgb_multilayer_gain				--曝光时间对应的增益,单位为db，为3xn矩阵，n表示灰阶数量，第一行表示R从低灰到高亮的增益，与曝光时间一一对应.type:ALGO_32FC1
	@Parameter	: 	out_rgb_multilayer_state			--曝光时间对应的状态，为3xn矩阵，n表示灰阶数量，第一行表示R从低灰到高亮的状态，与曝光时间一一对应.type:ALGO_32SC1
	@Parameter	: 	out_rgb_multilayer_expotime_fisrt	--第一次迭代（未衰减）rgb的曝光时间.type:ALGO_32SC1
	@Parameter	: 	in_is_deep_copy
	@Returns	:   AlgoError
	@Author		：	TianBo@2022/06/21
	************************************/
	[[deprecated("This function is deprecated and will be removed in a future version. plsease use CameraParamGetSingleExpotime replace this function ")]]
	C_API AlgoError C_CALL CameraParaGetExpotimeForMultiLayerSuperResolution(AlgoHandle camerapara_handle,
		AlgoMat* out_rgb_multilayer_expotime, AlgoMat* out_rgb_multilayer_gain, AlgoMat* out_rgb_multilayer_state, AlgoMat* out_rgb_multilayer_expotime_fisrt, int in_is_deep_copy = 1);


	C_API AlgoError C_CALL CameraParaRelease(AlgoHandle camerapara_handle);


	/************************************
	@Brief		：	保存/载入当前数据
	@Note		：
	@Parameter	: 	in_fullname	            --目录
	@Returns	:   AlgoError
	@Author		：	LaiHaokun@2021/08/02
	************************************/

	C_API void C_CALL CameraParaDestroyHandle(AlgoHandle camerapara_handle);




    /*//////////////////////////////   数码相机AE算法相关    //////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////*/

    /************************************
    @brief  : 设置相机曝光时间列表
    @note   : max_idx、min_idx为默认值时，参数范围限制为列表本身
    +--------------------------------
    @param  : camerapara_handle     -- 相机调节句柄
    @param  : Expotime_list         -- 曝光时间列表（升序排列）
    @param  : num_Expotime_list     -- 曝光时间列表长度
    @param  : max_idx               -- 参数限制（最大值）
    @param  : min_idx               -- 参数限制（最小值）
    +--------------------------------
    @return :
    @Author : YangShangqi@2023/07/12
    ************************************/
    C_API AlgoError C_CALL CameraSetExposureTimeList(AlgoHandle camerapara_handle, float* Expotime_list, int num_Expotime_list, const int max_idx=INT16_MAX, const int min_idx=0);

    /************************************
    @brief  : 设置相ISO档位列表
    @note   : max_idx、min_idx为默认值时，参数范围限制为列表本身
    +--------------------------------
    @param  : camerapara_handle     -- 相机调节句柄
    @param  : Expotime_list         -- ISO间列表（升序排列）
    @param  : num_Expotime_list     -- ISO列表长度
    @param  : max_idx               -- 参数限制（最大值）
    @param  : min_idx               -- 参数限制（最小值）
    +--------------------------------
    @return :
    @Author : YangShangqi@2023/07/12
    ************************************/
    C_API AlgoError C_CALL CameraSetIsoList(AlgoHandle camerapara_handle, float* ISO_list, int num_ISO_list, const int max_idx = INT16_MAX, const int min_idx = 0);

    /************************************
    @brief  : 设置相光圈档位列表
    @note   : max_idx、min_idx为默认值时，参数范围限制为列表本身
    +--------------------------------
    @param  : camerapara_handle     -- 相机调节句柄
    @param  : Expotime_list         -- 光圈列表（升序排列）(其中元素为光圈档位对应的相对进光量)；EG: F8 F11
    @param  : num_Expotime_list     -- 光圈列表长度
    @param  : max_idx               -- 参数限制（最大值）
    @param  : min_idx               -- 参数限制（最小值）
    +--------------------------------
    @return :
    @Author : YangShangqi@2023/07/12
    ************************************/
    C_API AlgoError C_CALL CameraSetApertureList(AlgoHandle camerapara_handle, float* Aperture_list, int num_Aperture_list, const int max_idx = INT16_MAX, const int min_idx = 0);


    /************************************
    @brief  : 数码相机自动曝光算法
    @note   : 算法优先级 T > ISO > A，不收敛时会搜索所有档位，按照 饱和度S、曝光T、ISO、光圈A优先级顺序输出最佳选择
    +--------------------------------
    @param  : camerapara_handle     -- 相机调节句柄
	@param  : in_point_quality      -- 输入灯点质量，饱和度和灯点占比
	@param  : Expotime_out			-- 输出曝光时间
	@param  : ISO_out				-- 输出ISO
	@param  : Aperture_out			-- 输出光圈
	@param  : State_out			    -- 分析结果状态：
										0是初始化状态，1是完成调节；2是算法不收敛，屏体偏亮；3是算法不收敛，屏体偏暗
										State_out != 1 时返回 err 重新调节
    +--------------------------------
    @return :
    @Author : YangShangqi@2023/07/12
    ************************************/
    C_API AlgoError C_CALL CameraExposureAnalyze(AlgoHandle camerapara_handle, const AlgoPointQuality *in_point_quality, int &Expotime_out, int &ISO_out, int &Aperture_out, int &State_out);





#ifdef __cplusplus
}
#endif // __cplusplus
#endif