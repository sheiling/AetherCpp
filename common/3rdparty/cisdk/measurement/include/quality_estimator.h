#ifndef __QUALITY_ESTIMATOR_H__
#define __QUALITY_ESTIMATOR_H__

#include "ci_define.h"
#include "ci_declare.h"
#include "quality_estimator_data.h"
#include "quality_estimator_errors.h"
#include "ci_data.h"
#include "meta_info_enum.h"
#include "algo_mat_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	/************************************
	@brief  : 初始化质量句柄
	@note   :
	@param  : out_quality_handle
	@return	: AlgoError
	@Author : chengjiayi@2021/05/17
	************************************/
	C_API AlgoError C_CALL QualityCreateHandle(AlgoHandle* out_quality_handle);



	/************************************
	@Brief		：	计算区域质量
	@Note		：
	@Parameter	: 	quality_handle		-- 质量句柄
	@Parameter	: 	in_shot		        -- shot句柄
	@Parameter	: 	in_region		    -- 区域句柄
	@Parameter	: 	out_area_quality	-- 导出区域质量,请预先空间分配
	@Returns	:   AlgoError
	@Author		：	chengjiayi@2021/04/26
	************************************/
	C_API AlgoError C_CALL QualityEstimateArea(AlgoHandle quality_handle, const AlgoHandle in_shot, const AlgoHandle in_region, AlgoAreaQuality* out_area_quality);

	/************************************
	@brief  :  计算区域质量,shot不包含元信息的情况下
	@note   :  如果shot包含metainfo, 将会忽略掉shot包含的metainfo
	@param  : quality_handle        -- 质量句柄
	@param  : in_shot_no_metainfo   -- shot句柄,不包含元信息
	@param  : in_cam_depth          -- 相机位深
	@param  : in_region             -- 区域句柄
	@param  : out_area_quality      -- 导出区域质量,请预先new
	@return : AlgoError
	@Author : chengjiayi@2022/09/19
	************************************/
	C_API AlgoError C_CALL QualityEstimateAreaNoMetaInfo(AlgoHandle quality_handle, const AlgoHandle in_shot_no_metainfo, CameraType in_cam_depth,
		const AlgoHandle in_region, AlgoAreaQuality* out_area_quality);


	/************************************
	@Brief		：	设置灯点大小，即两颗灯点之间的像素距离，当不设置的时候，会默认使用矩形屏计算方法，球形屏业务必须调用；
	@Note		：	
					球形屏必须使用，常规屏选用
					会影响的接口有QualityEstimatePoint、QualityEstimatePointBlock

	@Parameter	: 	quality_handle				-- 质量句柄
	@Parameter	: 	in_point_wh					-- 单个灯点的大小，该值可参考接口ROIComputePointTemplateWh的出参out_point_wh
	@Returns	:   AlgoError
	@Author		：	GuoHao@2025/07/30
	************************************/
	C_API AlgoError C_CALL QualitySetPointTemplateWh(AlgoHandle quality_handle, AlgoSize2f in_point_wh);


	/************************************
	@Brief		：	计算灯点质量
	@Note		：
	@Parameter	: 	quality_handle				-- 质量句柄
	@Parameter	: 	in_shot						-- shot句柄
	@Parameter	: 	in_region					-- roi区域句柄
	@Parameter	: 	out_point_quality			-- 导出灯点质量,请预先空间分配
	@Returns	:   AlgoError
	@Author		：	chengjiayi@2021/04/26
	************************************/
	C_API AlgoError C_CALL QualityEstimatePoint(AlgoHandle quality_handle, const AlgoHandle in_shot, const AlgoHandle in_region, AlgoPointQuality* out_point_quality);


	/************************************
	@Brief		：	设置图像分区域评估时的子区域大小(以灯点为单位)
	@Note		：
					默认子区域大小为12*12灯点大小，该接口通常不需要调用即可；
					该接口会影响：1、点定位的灯点质量评价，对应接口QualityEstimatePointBlock；2、模糊成像的质量评价，对应接口？；

	@Parameter	: 	quality_handle				-- 质量句柄
	@Parameter	: 	in_block_size				-- 子区域大小，单位为灯点尺寸
	@Returns	:   AlgoError
	@Author		：	GupHao@2025/07/30
	************************************/
	C_API AlgoError C_CALL QualitySetBlockSize(AlgoHandle quality_handle, AlgoSize in_block_size);


	/************************************
	@Brief		：	将图像分区域，计算各区域灯点质量
	@Note		：
					该接口会将Shot.image分为若干个block，以block为单位进行质量评估，从而获得各个block的灯点成像质量；
					将屏体按照置block的大小划分为多个区域（block的大小不能被灯点总数整除也没关系），对每个block进行质量评估，并将结果以矩阵形式存储在公有变量中；
	@Parameter	: 	in_out_quality_handle		-- 质量句柄
	@Parameter	: 	in_shot						-- shot句柄
	@Parameter	: 	in_region					-- roi区域句柄
	@Returns	:   AlgoError
	@Author		：	GupHao@2025/07/30
	************************************/
	C_API AlgoError C_CALL QualityEstimatePointBlock(AlgoHandle in_out_quality_handle, const AlgoHandle in_shot, const AlgoHandle in_region);


	/************************************
	@brief		:	设置灯点质量阈值
	@note		:
					1、该接口设置的阈值可以与QualityCheck配合使用，来对质量评估的结果进行判断，是否满足阈值区间，并返回对应错误码；该功能软件基本没有在用；
					2、该接口设置的阈值，会用于QualityAssessmentPointFromBlock接口，对QualityEstimatePointBlock的结果进行判断，并输出最终图像整体的质量评价结果
						当各个block的灯点峰值的最大值、最小值同时超限会报错；
						当灯点占比最大值超过上限，会报错；
	@param		:	quality_handle				-- 质量句柄
	@param		:	in_min						-- 阈值下限
	@param		:	in_max						-- 阈值上限
	@param		:	in_is_check					-- 是否检查  >0.f 检查
	@return		:	AlgoError
	@Author		:	GuoHao@2025/07/30()
	************************************/
	C_API AlgoError C_CALL QualitySetPointThresh(AlgoHandle quality_handle, const AlgoPointQuality* in_min, const AlgoPointQuality* in_max, const AlgoPointQuality* in_is_check);


	/************************************
	@Brief		：	基于各区域灯点质量，进行整体评估
	@Note		：
					如果没有做分区域评估就调用该接口，会报错；

					该接口与QualityEstimatePointLocal、QualitySetPointThresh配合使用，当不调用QualitySetPointThresh时，默认该接口输出为各维度的均值；
					当使用QualitySetPointThresh时，评估逻辑为：
						1、基于in_is_check确定对哪些维度进行check，通常是灯点占比、灯点峰值、灯点均值这几个维度；
						2、pt_peak_val：灯点峰值，当所有block的质量都在min与max的区间内时，将会输出最大值（最亮的block的灯点峰值的均值）；
							为了避免异常点干扰会去掉最暗最亮的1%的block，会对最亮的三个block求均值；
						3、pt_hw_dutyratio，会输出最大值;
					    

	@Parameter	: 	in_quality_handle			-- 质量句柄
	@Parameter	: 	out_point_quality			-- 基于不同区域得到的最终质量评价结果
	@Returns	:   AlgoError
	@Author		：	GupHao@2025/07/30
	************************************/
	C_API AlgoError C_CALL QualityAssessmentPointFromBlock(AlgoHandle in_quality_handle, AlgoPointQuality* out_point_quality);


	/************************************
	@Brief		：	获取QualityEstimatePointBlock评估得到的各区域灯点质量
	@Note		：
					当handle内部存在按区域评估的结果时，获得到的是各个block的质量，形式为矩阵，矩阵大小与block的个数有关，假如屏幕点亮灯点个数为120*60，block尺寸为12*12，则矩阵大小为10*5；
					当handle内部不存在按区域评估的结果时（未调用QualityEstimatePointBlock接口，而是使用了QualityEstimatePoint或者未评估），会报错；
					当内部无质量评估结果时会报错

	@Parameter	: 	in_quality_handle			-- 质量句柄
	@Parameter	: 	out_blocks_point_quality	-- 灯点质量结构体；
	@param		:	is_deep_copy				-- 是否深拷贝, 默认浅拷贝  1=深拷贝;0=浅拷贝
	@Returns	:   AlgoError
	@Author		：	GupHao@2025/07/30
	************************************/
	C_API AlgoError C_CALL QualityGetPointQualityBlock(AlgoHandle in_quality_handle, AlgoPointQualityMat* out_blocks_point_quality, int in_is_deep_copy = 0);


	/************************************
	@Brief		：	存储功能，存储quality中的area_quality和point_quality
	@Note		：
	@Parameter	: 	in_quality_handle			-- 质量句柄
	@Parameter	: 	in_quality_fullname			-- 文件名，后缀是.quality
	@Returns	:   AlgoError
	@Author		：	GupHao@2025/07/30
	************************************/
	C_API AlgoError C_CALL QualitySave(AlgoHandle in_quality_handle, const char* in_quality_fullname);
	C_API AlgoError C_CALL QualityLoad(AlgoHandle in_quality_handle, const char* in_quality_fullname);



	/************************************
	@brief  : 计算灯点质量,shot不包含元信息的情况下
	@note   : 如果shot包含metainfo, 将会忽略掉shot包含的metainfo
	@param  : quality_handle        -- 质量句柄
	@param  : in_shot_no_metainfo   -- shot句柄,不包含元信息
	@param  : in_cam_depth          -- 相机位深
	@param  : in_lighted_led_wh     -- 屏幕点亮的列行数,注意虚拟像素的x2
	@param  : in_region             -- 区域句柄
	@param  : out_point_quality     -- 导出区域质量,请预先new
	@return : AlgoError
	@Author : chengjiayi@2022/09/19
	************************************/
	C_API AlgoError C_CALL QualityEstimatePointNoMetaInfo(AlgoHandle quality_handle, const AlgoHandle in_shot_no_metainfo, CameraType in_cam_depth,
		const AlgoSize* in_lighted_led_wh, const AlgoHandle in_region, AlgoPointQuality* out_point_quality);


	/************************************
	@Brief		：	基于任意灯点排布的点定位算法计算灯点质量，输出灯点占比和灯点峰值
	@Note		：
	@Parameter	: 	quality_handle		-- 质量句柄
	@Parameter	: 	in_shot		        -- shot句柄
	@Parameter	: 	out_point_quality	-- 导出灯点质量,请预先空间分配
	@Parameter  :   in_param            -- 输入参数设置，默认为空不输入，一般情况下不设置。参数包括：
					1.thresh				--自适应阈值补偿值，默认44。thresh增大，灯点在图像所占面积减小，可能出现漏检情况；
											  反之，灯点在图像所占面积增大，可能灯点粘连，多颗灯检测为一颗灯
					2.blockSize				--自适应阈值窗口，默认201，输入必须为奇数。blockSize大小取决于检测灯点所占像素个数，当超过默认201*201时才考虑适当增大
	                3.radius_med_min_ratio  --剔除非灯点最小比例，默认0.2。表示在灯点半径乘以最小和最大比例内的目标都视为灯点。
											  没有暗灯时可适当增大最小比例，精准定位正常发光灯点。
					4.radius_med_max_ratio	--剔除非灯点最大比例，默认1.5。
	@Parameter  :   in_param_num        -- 输入参数个数，默认为0，一般情况下不设置
										输入时，设置为4，对应输入in_param中
	@Returns	:   AlgoError
	@Author		：	wangyueyao@2024/06/04
	************************************/
	C_API AlgoError C_CALL QualityEstimatePointForAnyArrangement(AlgoHandle quality_handle, const AlgoHandle in_shot, AlgoPointQuality*out_point_quality, const float* in_param = nullptr, const int in_param_num = 0);


	/************************************
	@Brief		：	计算灯点质量
	@Note		：
	@Parameter	: 	quality_handle		-- 质量句柄
	@Parameter	: 	in_shot		        -- shot句柄
	@Parameter	: 	in_region		    --  区域句柄
	@Parameter	: 	out_point_quality	-- 导出灯点质量,请预先空间分配
	@Returns	:   AlgoError
	@Author		：	tianbo@2021/10/12
	************************************/
	C_API AlgoError C_CALL QualityEstimatePointForBlur(AlgoHandle quality_handle, const AlgoHandle in_shot, const AlgoHandle in_region, AlgoPointQuality*out_point_quality);


	/************************************
	@brief  : 计算灯点质量
	@note   :
	@param  : quality_handle        -- 质量句柄
	@param  : in_algomat            -- 有效的algomat
	@param  : in_cie_type           -- 颜色分量类型
	@param  : in_cam_depth          -- 相机位深
	@param  : in_lighted_led_wh     -- 屏幕点亮的列行数,注意虚拟像素的x2
	@param  : in_rect               -- Rect区域
	@param  : out_point_quality     -- 导出区域质量,请预先new
	@return : AlgoError
	@Author : lipengpeng@2023/01/12
	************************************/
	C_API AlgoError C_CALL QualityEstimatePointForAlgoMat(AlgoHandle quality_handle, const AlgoMat* in_algomat,
		CIEType in_cie_type, CameraType in_cam_depth, const AlgoSize* in_lighted_led_wh, const AlgoRect* in_rect,
		AlgoPointQuality* out_point_quality);



	/************************************
	@Brief		：	手机亮度修缝中计算网格成像质量
	@Note		：
	@Parameter	: 	quality_handle					// 质量句柄
	@Parameter	: 	入参：
					g_shot_in						// shot句柄
					int band_size;					// 打屏窄带宽度
					int edge_size;					// 打屏边栏宽度
					int hem_size;					// 打屏包边宽度
					float max_sampling_rate		    // 当前分区最大采样率
					出参：
					pt_sharpness_out				// 屏体整体锐度
					max_blur_quality_out			// 屏体最大锐度
					min_blur_quality_out			// 屏体最小锐度
					pt_avg_val_out					// 屏体整体曝光
					pt_peak_val_out					// 屏体最大曝光
					background_quality_out			// 环境光亮度质量
					background_avg_val_out			// 环境光曝光值
	@Returns	:   AlgoError
	@Author		：	GuoHao@2023/12/11
	************************************/
	C_API AlgoError C_CALL QualityEstimatePointForGrid(AlgoHandle quality_handle, const AlgoHandle g_shot_in, const int& band_size_in,
		const int& edge_size_in, const int& hem_size_in, const float& max_sampling_rate_in, float& pt_sharpness_out, float& max_blur_quality_out,
		float& min_blur_quality_out, float& pt_avg_val_out, float& pt_peak_val_out, float& background_quality_out, float& background_avg_val_out);

	/************************************
	@Brief		：	手机亮度修缝3.0：设置质量评价参数
	@Note		：
	@Parameter	: 	quality_handle					// 质量句柄
	@Parameter	: 	入参：
					areaAndRatioRangeType_in		// 阈值计算策略，默认为0，按照算法边界计算；1代表按照矩形框比例计算；根据业务选择即可；
					minThresholdRatio_in			// 最小阈值对应比例,策略为0，则最小算法采样率边界×该比例为下限；策略为1，则引导框采样率×该比例为下限
					maxThresholdRatio_in			// 最大阈值对应比例，同上
					maxSampleRate_in				// 算法边界最大采样率，推荐设为6.0
					minSampleRate_in				// 算法边界最小采样率，推荐设为3.0
					rectThreshold_in				// 矩形度阈值，暂定为0.8，该值越小，可斜拍检测角度越大，但是会带来误检和效果风险；
					binaryType_in					// 二值化方法选择，默认为0，otsu方法；1为局域自适应阈值二值化，用于复杂光环境，但是速度较慢，失焦时误差较大；
					resultType_in					// 质量评估返回结果类型，默认为1，0仅返回全局质量；1返回全局质量与角点；2返回全局质量、角点、灯板锐度；
					checkType_in					// 矩形二次check逻辑策略，暂时无效

	@Returns	:   AlgoError
	@Author		：	GuoHao@2024/04/09
	************************************/
	C_API AlgoError C_CALL QualityEstimatePointSetGridParam(AlgoHandle quality_handle, const int areaAndRatioRangeType_in, const float minThresholdRatio_in, const float maxThresholdRatio_in, const float maxSampleRate_in, const float minSampleRate_in,
		const float rectThreshold_in, const int binaryType_in, const int resultType_in, const int checkType_in);

	/************************************
	@Brief		：	手机亮度修缝3.0：逐灯板计算网格打屏的成像质量，并获取屏体角点
	@Note		：	需要注意set
	@Parameter	: 	quality_handle					// 质量句柄
	@Parameter	: 	入参：
					g_shot_in						// shot句柄
					band_size_in;					// 打屏窄带宽度
					edge_size_in;					// 打屏边栏宽度
					hem_size_in;					// 打屏包边宽度
					max_camera_sample_rate_in		// 当前分区相机最大采样率
					guidebox_sample_rate_in			// 引导框采样率
					出参：
					mean_sharpness_out				// 全局平均锐度
					max_sharpness_out				// 全局最大锐度
					min_sharpness_out				// 全局最小锐度
					mean_Expo_out					// 全局平均曝光
					peak_Expo_out					// 全局峰值曝光
					grid_corner_out					// 屏体角点
					panel_center_table_out			// 灯板中心点位置
					panel_sharpness_table_out		// 灯板锐度
	@Returns	:   AlgoError
	@Author		：	GuoHao@2024/04/09
	************************************/
	C_API AlgoError C_CALL QualityEstimatePointForGridPro(AlgoHandle quality_handle, const AlgoHandle g_shot_in, const int& band_size_in,
		const int& edge_size_in, const int& hem_size_in, const float& max_camera_sample_rate_in, const float & guidebox_sample_rate_in,
		float& mean_sharpness_out, float& max_sharpness_out, float & min_sharpness_out, float& mean_Expo_out, float& peak_Expo_out,
		AlgoPoint2f * grid_corner_out, AlgoPoint * maxMinSharpLoc_out, AlgoMat *boardSharpnessX_out, AlgoMat * boardSharpnessY_out);




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
	C_API AlgoError C_CALL QualityRelease(AlgoHandle quality_handle);
	C_API void C_CALL QualityDestroyHandle(AlgoHandle quality_handle);






	////////////////////////////////////  以下接口仅供不方便自行检查时候使用 ///////////////////////////////////////

	/************************************
	@brief  : 设置区域质量阈值
	@note   :
	@param  : quality_handle    -- 质量句柄
	@param  : in_min               -- 阈值下限
	@param  : in_max               -- 阈值上限
	@param  : in_is_check          -- 是否检查  >0.f 检查
	@return	: AlgoError
	@Author : chengjiayi@2021/08/30
	************************************/
	C_API AlgoError C_CALL QualitySetAreaThresh(AlgoHandle quality_handle, const AlgoAreaQuality* in_min,
		const AlgoAreaQuality* in_max, const AlgoAreaQuality* in_is_check);


	/************************************
	@Brief		：	载入质量阈值
	@Note		：	
	@Parameter	: 	quality_handle		--质量句柄
	@Parameter	: 	in_config_file		--质量配置文件
	@Parameter	: 	in_node_name		--质量文件读取哪个node
	@Returns	:   AlgoError
	@Author		：	chengjiayi@2021/04/26
	************************************/
	C_API AlgoError C_CALL QualityLoadThresh(AlgoHandle quality_handle, const char* in_config_file, const char* in_node_name);


	//使用算法内置阈值,即配置文件中<Default>
	C_API AlgoError C_CALL QualityDefaultThresh(AlgoHandle quality_handle);




	/************************************
	@Brief		：	根据载入的质量阈值进行检查
	@Note		：	这个函数主要是算法内部根据质量阈值进行比对并返回错误
	@Parameter	: 	quality_handle		--质量句柄
	@Parameter	: 	out_result		    --导出灯点检查结果, 请预先空间分配;out_result=nullptr时仅用于打印
	@Returns	:   AlgoError
	@Author		：	chengjiayi@2021/04/26
	************************************/
	C_API AlgoError C_CALL QualityCheck(AlgoHandle quality_handle, QualityResult* out_result = nullptr);




	/************************************
	@Brief		：	根据子区域对灯点质量进行评价（点定位）
	@Note		：	此函数用于区域ROI质量评估 ， 
					之前需要先通过 QualityEstimateGenerateRects 获取灯点质量评估区域集合
	@Parameter	: 	quality_handle		--质量句柄
	@Parameter	: 	in_shot				--输入图像（要求原图，不是ROI后的图像）
	@Returns	:   in_rects			--需要进行评价的子区域数组
	@Returns	:   rect_num			--子区域数组长度
	@Returns	:   out_point_quality	--导出灯点质量,请预先空间分配
	@Author		：	Yangshangqi @2024/07/16
	************************************/
	C_API AlgoError C_CALL QualityEstimatePointByRects(AlgoHandle quality_handle, const AlgoHandle in_shot, AlgoHandle in_region_roi, const AlgoRect* in_rects, int in_rect_num, AlgoPointQuality* out_point_quality);

	/************************************
	@Brief		：	根据子区域对灯点质量进行评价（超分成像）
	@Note		：	 此函数用于区域ROI质量评估 ， 
					 之前需要先通过 QualityEstimateGenerateRects 获取灯点质量评估区域集合
	@Parameter	: 	quality_handle		--质量句柄
	@Parameter	: 	in_shot				--输入图像（要求原图，不是ROI后的图像）
	@Returns	:   in_rects			--需要进行评价的子区域数组
	@Returns	:   rect_num			--子区域数组长度
	@Returns	:   out_point_quality	--导出灯点质量,请预先空间分配
	@Author		：	Yangshangqi @2024/07/16
	************************************/
	C_API AlgoError C_CALL QualityEstimatePointForBlurByRects(AlgoHandle quality_handle, const AlgoHandle in_shot,  const AlgoRect* in_rects, int in_rect_num, AlgoPointQuality* out_point_quality);

	/************************************
	@Brief		：	根据屏幕ROI生成灯点质量评估区域集合
	@Note		：  此函数用于区域ROI质量评估 ，中生成评估ROI,为之后的灯点质量评估提供质量评估区域集合
	@Parameter	: 	in_region_roi		--有效的ROI句柄
	@Parameter	: 	in_method			--生成ROI的排布规则：推荐用 1 
											+ 0 -> 均匀分布 49点；
											+ 1-> 5点稀疏加权分布 : 
	@Returns	:   out_rects			--输出的SUB_ROI数组: 长度为128，需要预先申请空间（灯点质量评估区域集合）
	@Returns	:   out_num				--子区域数组有效长度
	@Author		：	Yangshangqi @2024/07/16
	************************************/
	C_API AlgoError C_CALL QualityEstimateGenerateRects(AlgoHandle in_region_roi, int in_method, AlgoRect* out_rects, int *out_num);


#ifdef __cplusplus
}
#endif // __cplusplus
#endif