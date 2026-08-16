#ifndef __DEMURA_LAYER_SELECTOR_DATA_H__
#define __DEMURA_LAYER_SELECTOR_DATA_H__


#include "meta_info_data.h"
#include "algo_mat_data.h"

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

	typedef struct
    {
        double L = 0.0;
        double x = 0.0;
        double y = 0.0;
    } Ca_Lxy;

    typedef struct
    {
        float correction_similarity_value = 0.95f;//相似性阈值。0-1范围
        float lumin_uniformity_th = 0.99f;//箱体间 亮度均匀性阈值。0-1范围
        float x_deviation_th = 0.001f;//箱体间 x坐标偏差阈值。大于0
		float y_deviation_th = 0.001f;//箱体间 y坐标偏差阈值。大于0
        float inner_lumin_uniformity_th = 0.99f;//箱体内 亮度均匀性阈值。0-1范围
        float inner_x_deviation_th = 0.004f;//箱体内 x坐标偏差阈值。大于0
        float inner_y_deviation_th = 0.004f;//箱体内 y坐标偏差阈值。大于0
        float w_export_correction_similarity_value = 0.95f;//输入最终w校正灰阶时的相似性阈值。0-1范围
        float rgb_export_correction_similarity_value = 0.95f;//输入最终rgb校正灰阶时的相似性阈值。0-1范围
        float judge_w_multi_level_correction_th = 0.1f;//判断w多层校正的相似性阈值。0-1范围
        float judge_rgb_multi_level_correction_th = 0.1f;//判断rgb多层校正的相似性阈值。0-1范围
		float uniformaty_ratio = 0.0005f; //均匀性计算时筛选掉的像素比例值。0-1范围
        float match_score = 0.74f; //模板匹配得分阈值。0-1范围
        float low_match_score = 0.20f; //模板匹配得分低阈值。0-1范围
        float per_strcut_intersect_ratio_th = 0.7f; //过渡灰阶融合时两轮廓相交比例。0-1范围
        float all_struct_intersection_ratio = 0.90f; //过渡灰阶融合时整张图的相交比例。0-1范围
    } DemuraLayerSelectorThreshold;
    typedef struct
    {
		int start_gray = 0;//拍图的起始灰阶
        int gray_num = 0;//拍图的灰阶数
        int gray_step = 0;//拍图的灰阶间隔
        float lumin_loss_ratio = 0.3f;//亮度衰减比例
        int lumin_loss_gray_index = 0;//亮度后最大灰阶索引
        double gamma = 2.8;//箱体gamma
        DemuraLayerSelectorThreshold threshold_info; //各检测算法的阈值结构体
    } DemuraLayerSelectorParam;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //
