 #ifndef __SMD_DETECTOR_DATA_H__
#define __SMD_DETECTOR_DATA_H__

#include "ci_data.h"
#include "algo_mat_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


	typedef struct
	{
		// 该部分输入全部为实际物理尺寸
		double in_pixel2mm;//图像中每个像素代表的实际尺寸
		float  in_led_distance;//两个灯点理想状态下的间隔尺寸,即点间距
		float  in_led_width;
		float  in_led_height;

	}SMDParam;


	typedef struct
	{
		float in_match_thresh;       //模板匹配阈值
		int   in_segment_thresh;     //灯点分割阈值
		float in_area_low_thresh;    //灯点面积判断低百分比阈值，一般为0.4-0.8之间
		float in_area_high_thresh;   //灯点面积判断高百分比阈值，一般为1.3-2之间
		float in_dis_thresh;         //灯点直线度偏移阈值，根据客户需求而定，调用SMDLineOffestDetect时会用到
		float in_gap_low_thresh;     //灯点间距低阈值，根据客户需求而定，调用SMDGapDetect时会用到
		float in_gap_high_thresh;    //灯点间距高阈值，根据客户需求而定，调用SMDGapDetect时会用到
		float in_angle_thresh;       //灯点角度判定阈值，输入正值，算法内部判断时，将正负范围内的都检出

	}SMDThresh;



	typedef enum
	{
		MODEL_METHOD         = 1,   //基于模板检测方法
		TRADITION_METHOD    = 2,   //基于传统检测方法

	}DetectMethodFlag;



	typedef enum
	{
		SMD_FLAG_ROW     = 0b00000001,     //只针对行进行检测
		SMD_FLAG_COL     = 0b00000010,     //只针对列进行检测
		SMD_FLAG_ROW_COL = 0b00000100,     //针对行、列都进行检测

	}DetectFlag;




	typedef enum
	{
		LEFT_TO_RIGHT         =     0b10000000,     //区域从左到右进行排序
		RIGHT_TO_LEFT         =     0b01000000,     //区域从右到左进行排序
		TOP_TO_BOTTOM         =     0b00100000,     //区域从上到下进行排序
		BOTTOM_TO_TOP         =     0b00010000,     //区域从下到上进行排序

	}SortFlag;





	

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // 
