#ifndef __SHOT_UTILS_H__
#define __SHOT_UTILS_H__

#include "ci_data.h"
#include "ci_define.h"
#include "ci_declare.h"


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
	
	/************************************
	@brief  : 获取异形箱体对应标准箱体曲面的ROI区域
	@note   : 要求校正过程中异形箱体与标准箱体必须对齐边角放置
	@param  : in_standard_shot_handle			--标准箱体的shot句柄，需要有part信息
	@param  : in_to_be_solved_shot_handle		--异形箱体的shot句柄，需要有part信息
	@param  : out_curve_ROI						--异形箱体对应的ROI区域
	@return	: AlgoError
	@Author : GuoHao@2024/08/01
	************************************/
	C_API AlgoError C_CALL ShotUtilsRelativeLocate(AlgoHandle in_standard_shot_handle, AlgoHandle in_to_be_solved_shot_handle, AlgoRect* out_curve_roi);

	/************************************
	@brief  : 计算shot图像在指定roi区域内的均值
	@note   : in_roi值全为0时会计算全局均值
	@param  : in_standard_shot_handle			--shot句柄
	@param  : in_rect							--指定roi区域，(x,y)为左上角坐标，width为矩形宽，height为矩形高
	@param  : out_mean							--输出roi区域内的均值
	@param  : in_is_exp_norm					--是否需要做曝光时间的归一化，false不做归一化，true做归一化，默认为false
	@return	: AlgoError
	@Author : ZhongAo@2025/07/17
	************************************/
	C_API AlgoError C_CALL ShotUtilsGetMeanRect(AlgoHandle in_standard_shot_handle, AlgoRect in_rect, float* out_mean, bool in_is_exp_norm=false);


#ifdef __cplusplus
};
#endif // __cplusplus
#endif