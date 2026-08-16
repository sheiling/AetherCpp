#ifndef __POINT_FINDER_DATA_H__
#define __POINT_FINDER_DATA_H__

#include "ci_data.h"
#include "algo_mat_data.h"
#include "roi_detector_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

    typedef enum
    {
		MATCH_SQDIFF = 0,
		MATCH_SQDIFF_NORMED = 1,
		MATCH_CCORR = 2,
		MATCH_CCORR_NORMED = 3,
		MATCH_CCOEFF = 4,
		MATCH_CCOEFF_NORMED = 5,
		MATCH_NCC = 6

    } PointFindMatchFlg;

	typedef struct 
	{
		float sigma_ratio;					//模板平滑比例，调节范围[0.1f-10.0f]，数值越小越平滑。-1为不设置，内部默认参数为1.0f
		float depression_side_ratio;		//模板的中心凹陷边长与模板边长的比例，调节范围[0.1f, 1.0f]，比值越小，中心凹陷的区域越小。-1为不设置，内部默认参数为-1.0f
		float depression_degree;			//模板中心凹陷程度，调节范围[0.f-1.0f]，比值越小，中心凹陷程度越低。-1为不设置，内部默认参数为-1.0f
		int demean_flag;					//模板是否去均值，去均值：1， 不去均值：0。-1为不设置，内部默认参数为1
		int circle_flag;					//模板是方形还是圆形，圆形:1，方形:0。-1为不设置，内部默认参数为0

	}TemplateParam;

	typedef struct
	{
		float binary_ratio;					//二值化分割阈值比例，调节范围[0.1f-10.0f]，数值越大灯点在图像所占面积减小，可能出现漏检情况；反之，灯点在图像所占面积增大，可能灯点粘连，造成大面积漏检。-1为不设置，内部默认参数为1.0f
		float gray_filter_max_ratio;		//灰度图干扰点滤除上界阈值，调节范围为[1.0f-5.0f]，内部默认为1.8f，该数值表示在灰度图上滤除大于灯点均值×1.8的干扰过亮灯/杂散点
		float gray_filter_min_ratio;		//灰度图干扰点滤除上界阈值，调节范围为[0.0f-1.0f]，内部默认为0.3f，该数值表示在灰度图上滤除小于灯点均值×0.3的干扰过暗灯/杂散点

	}PostProcParam;


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // 
