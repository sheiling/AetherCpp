#ifndef __MAPPER_UTILS_H__
#define __MAPPER_UTILS_H__

#include "ci_define.h"
#include "ci_declare.h"
#include "mapper_utils_data.h"
#include "mapper_utils_errors.h"
#include "geometry_transformer.h"


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	/************************************
	@brief  :  初始化句柄
	@note   :
	@param  : out_mapper_utils_handle    有效的MapperUtils句柄 详见mapper_utils.h定义
	@param  : out_mapper_utils_handle    有效的MapperUtils句柄 详见mapper_utils.h定义
	@return	: AlgoError
	@Author : huangziming@2024/09/25
	************************************/
	C_API AlgoError C_CALL MapperUtilsCreateHandle(AlgoHandle* out_mapper_utils_handle);


	/************************************
	@brief  : 根据隔点的排列结果，通过划分畸变&形变基本一致的子区域，结合统计信息对排列坐标进行修正
	@note   : 使用场景：
				  应用范围：常规屏
				  业务场景：灯点成像模糊导致finder结果有偏差，只针对超分2.0业务
	@param  : in_out_point_map_handle                  待修正的某一颜色的隔点排列结果
	@param  : in_pure_shot                             有效的shot句柄，和待补全的颜色为同一颜色，必须包含panel,partition,pattern,image参数
														   其中传入的pattern信息与隔点信息一致，如2*2隔点，取左上角点，对应pattern loc={0,0},interval={2,2}
														   partition中的xywh为处理包边后的真实点亮的分区区域信息，screen_wh为全屏尺寸信息
														   注：若模组分辨率不一致，需要将panel的宽高设置为partition的宽高
	@param  : in_subarea_length						   认为畸变&形变基本一致时，在相机成像面所占最大像素数目,即范围内的灯点可视为畸变形变基本一致
															数值越大，受畸变&形变的影响越大，算法内部使用时超出模组尺寸则进行截断
															数值越小，受畸变&形变的影响越小，但会导致用于修正的灯点数目过少，算法会抛出error
															此参数与镜头和光圈相关，CC60-50经验值为400，C3200-50经验值为300
															推荐使用默认值0，此时会根据相机类型使用默认值；一旦设置此参数，则以设置参数为准,要求不能小于0
	@return	: AlgoError
	@Author : HuangZiming@2025/08/06
	************************************/
	C_API AlgoError C_CALL MapperUtilsRectifyPos(AlgoHandle in_out_point_map_handle, const AlgoHandle in_pure_shot, const int in_subarea_length = 0);

	/************************************
	@brief  : 根据隔点的排列结果，通过插值的方式补全对应逐点灯点排列结果
	@note   : 使用场景：
				  当前支持常规矩形屏&曲面屏，包边、模组分辨率不一致场景
				  通过单张隔点图插值补全获取逐点图灯点排列，加速校正效率
			  注：接口对曲面屏的算法边界与mapper模块一致
	@param  : in_out_point_map_handle_need_fill        待补全的某一颜色的隔点排列结果
	@param  : in_pure_shot                             有效的shot句柄，和待补全的颜色为同一颜色，必须包含panel,partition,pattern,image参数
														   其中传入的pattern信息与隔点信息一致，如2*2隔点，取左上角点，对应pattern loc={0,0},interval={2,2}
														   partition中的xywh为处理包边后的真实点亮的分区区域信息，screen_wh为全屏尺寸信息
														   注：若模组分辨率不一致，需要将panel的宽高设置为partition的宽高
	@param  : in_interpolation_method                  灯点补全方法，当前支持 LOCAL_LINEAR_FIT,即局部区域内线性插值方法，针对直线排布的灯点类型
																				LOCAL_SPLINE_FIT,即样条插值补全方法，针对弯曲排布的灯点类型，当前曲面屏数据较少，效果有风险
																	默认使用LOCAL_LINEAR_FIT
	@param  : in_point_map_handle_type                 灯点排列类型，当前仅支持 ALGOMOD_POINT_MAPPER， 即常规矩形屏灯点排列结果
	@return	: AlgoError
	@Author : HuangZiming@2025/04/14
	************************************/
	C_API AlgoError C_CALL MapperUtilsFillIntervalByInterpolation(AlgoHandle in_out_point_map_handle_need_fill, const AlgoHandle in_pure_shot, 
		const InterpolationMethod in_interpolation_method = InterpolationMethod::LOCAL_LINEAR_FIT, 
		const int in_point_map_handle_type = ALGOMOD_POINT_MAPPER);


	/************************************
	@brief  : 根据一个像素内的某个颜色的逐点排列结果，推断其他颜色的逐点灯点排列结果
	@note   : 当前仅支持整像素的RGB紧密排列时利用某种颜色推测其他某一颜色，例如在球型屏中已有绿色逐点排列结果，结合红色的单张隔点点定位结果,可以获取得到红色的逐点排列结果
	@param  : point_map_handle_need_predict            待推测的其他某一颜色的逐点排列结果
	@param  : in_point_find_handle                     有效的其他某一颜色隔点搜索句柄， 和待推测的其他某一颜色为同一颜色
	@param  : in_pattern_wh                            其他颜色隔点pattern宽高
	@param  : in_pattern_loc                           其他颜色隔点在隔点pattern中对应的位置
	@param  : in_point_map_handle_mapped			   排列好的某个颜色的逐点排布句柄
	@param  : in_point_map_handle_type                 灯点排列类型，当前仅支持 ALGOMOD_POINT_IRREGULAR_MAPPER
	@return	: AlgoError
	@Author : HuangZiming@2024/12/13
	************************************/
	C_API AlgoError C_CALL MapperUtilsInferenceMapByOffset(AlgoHandle point_map_handle_need_predict, AlgoHandle in_point_find_handle,
		AlgoSize in_pattern_wh, AlgoPoint in_pattern_loc, AlgoHandle in_point_map_handle_mapped, const int in_point_map_handle_type = ALGOMOD_POINT_IRREGULAR_MAPPER);

	/************************************
	@brief  :  基于shot的image，重新计算并更新灯点的亚像素中心
	@note   :  业务场景：
					在球形屏隔点超分校正中，由于灯点重复性/RGB预测误差等原因，得到的亚像素中心和用于提取光通量的逐点图像中的灯点中心可能相差1-2个像素，
					此时需要该接口，基于逐点图像修正亚像素中心后再计算搜索框。

	@param  :  in_ledpoint_need_relocate				--待推测的其他某一颜色的逐点排列结果
	@param  :  in_pure_shot								--有效的其他某一颜色隔点搜索句柄， 和待推测的其他某一颜色为同一颜色
	@param  :  in_point_type                            --灯点排列类型，当前仅支持 ALGOMOD_POINT_IRREGULAR_MAPPER
	@return	:  AlgoError
	@Author :  GuoHao@2024/12/17
	************************************/
	C_API AlgoError C_CALL MapperUtilsRelocateSubPiXelByImage(AlgoHandle in_ledpoint_need_relocate, const AlgoHandle in_pure_shot, int in_point_type = ALGOMOD_POINT_IRREGULAR_MAPPER);


	/************************************
	@Brief		：	使用标准点定位结果进行进行点定位。 目前仅适用于模组校正的场景
	@Note		：	in_standard_ledpoint_handle必须保证所有的灯点都是live，及需调用PointMapCheckAllStateLive确认是否符合standard的要求
	@Parameter	: 	in_standard_map_handle       输入量，有效的标准map句柄，其中不能有一颗死灯
	@Parameter	: 	in_shot                      输入量，有效的shot句柄,待定位的shot      详见algo_shot.h中定义
	@Parameter	: 	in_region                    输入量，有效的区域定位句柄，标准roi区域， 详见roi_detector.h中定义
	@Parameter	: 	in_out_mapping_handle        输出量，基于标准Map输出的当前Shot的Mapping结果，存储形式为AlgoHandle
	@Parameter	: 	in_search_radius             输入量，表示四角匹配的区域半径，搜索范围为正方形,单位是灯点
												 [note]: 默认值 = 4，越大，则鲁棒性越好，但耗时变大。可支持最大的模组的位置偏移与该参数有关
														 
	@Returns	:   AlgoError
	@Author		：	songshaojun@2025/05/30
	************************************/
	C_API AlgoError C_CALL MapperUtilsMappingUseStanderMap(const AlgoHandle in_standard_map_handle, const AlgoHandle in_shot, const AlgoHandle in_region, AlgoHandle in_out_mapping_handle, const int in_search_radius = 4);



	/************************************
	@Brief		：	得到mapper四个角点的坐标，必须是存在的（死灯也为存在）
	@Note		：	
	@Parameter	: 	in_map_handle				 输入量，有效的标准map句柄，其中不能有一颗死灯
	@Parameter	: 	out_points					 输出量，四个角点的坐标，从左上、右上、右下、左下
	@Parameter	: 	nums						 points的个数
	@Returns	:   AlgoError
	@Author		：	songshaojun@2025/05/30
	************************************/
	C_API AlgoError C_CALL MapperUtilsGetConnerPoints(AlgoHandle in_map_handle, AlgoPoint2f* out_points, int* nums);


	/************************************
	@Brief		：	得到当前mapper需要放大的倍率，默认放大后单个灯占不高于12x12个像素。若原始超过default则不进行放大
	@Note		：	
	@Parameter	: 	in_map_handle				 输入量，有效的标准map句柄
	@Parameter	: 	out_ratio					 输出量，需要放大的倍率
	@Parameter	: 	in_default_max_sampling		 放大后每个led占的pix个数
	@Returns	:   AlgoError
	@Author		：	songshaojun@2025/05/30
	************************************/
	C_API AlgoError C_CALL MapperUtilsGetScalarRatio(AlgoHandle in_map_handle, float* out_ratio, float in_default_max_sampling = 12.f);


	/************************************
	@Brief		：	基于transform对mapper结果的loc进行坐标映射
	@Note		：
	@Parameter	: 	in_map_handle				 输入输出量，有效的标准map句柄。
	@Parameter	: 	in_transfomer_handle		 有效的透视句柄，
	@Returns	:   AlgoError
	@Author		：	songshaojun@2025/05/30
	************************************/
	C_API AlgoError C_CALL MapperUtilsTransformLoc(AlgoHandle in_out_map_handle, AlgoHandle in_transfomer_handle);





	/************************************
    @brief  : 句柄指向的对象的内存空间释放
    @note   : 句柄指向的对象还在
			 Release 相当于: OBJ.Release();
    @param  : mapper_utils_handle
    @return : AlgoError
    @Author : huangziming@2024/12/13
    ************************************/
	C_API AlgoError C_CALL MapperUtilsRelease(AlgoHandle mapper_utils_handle);

	/************************************
	@brief  : 句柄指向的对象销毁
	@note   : 此句柄已经无效, 请配合handle=nullptr使用
			  Destroy 相当于:
				OBJ.release();
				delete handle;
	@param  : mapper_utils_handle       
	@return : AlgoError
	@Author : huangziming@2024/12/13
	************************************/
	C_API void C_CALL MapperUtilsDestroyHandle(AlgoHandle mapper_utils_handle);
    







#ifdef __cplusplus
};
#endif // __cplusplus
#endif
