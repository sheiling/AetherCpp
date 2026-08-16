#ifndef __DIE_DETECTOR_DATA_H__
#define __DIE_DETECTOR_DATA_H__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


    //返修机、点测机共用参数
    typedef enum
    {
        WAFER_DIE_R = 1,
		WAFER_DIE_G = 2,
		WAFER_DIE_B = 3,
		WAFER_DIE_MIP = 4,
    }WaferDieRGBType;//WAFER上的DIE类型


	//返修机相关阈值、参数
	typedef struct
	{
		float in_match_thresh;   //匹配阈值
		int in_segment_thresh;   //分割微调阈值
		float in_die_area_low_thresh;  //面积筛选的低阈值
		float in_die_area_high_thresh;  //面积筛选的高阈值
		
	}DieDetectThresh; //返修机检测阈值


	//点测机相关阈值、参数
	typedef enum
	{
		WAFER_DIE_NOT_EXIST = 0,  //0代表无晶粒
		WAFER_DIE_COMPLETE = 1,   //1代表完整晶粒
		WAFER_DIE_INCOMPLETE = -1,  //-1代表出现破损

	}DieDefectType;//WAFER上DIE的缺陷类型

	typedef struct
	{
		float match_select_thold;           //扫直的筛选图阈值
		float die_defect_score_thold;       //晶粒缺陷检测得分阈值
		float die_match_thold;   //点测匹配阈值
		int die_segment_thold;   //点测分割微调阈值
		float die_area_low_thold;  //面积筛选的低阈值
		float die_area_high_thold;  //面积筛选的高阈值
		int object_segment_thold; //找到最近晶粒或目标点的背景分割阈值
		float mark_die_match_thold; //对位点的筛选阈值
		int ptn_w;                   //将对位点图像在水平方向上分为几份
		int ptn_h;                   //将对位点图像在垂直方向上分为几份
	}DieProbeThresh; //点测机检测阈值


	typedef struct 
	{
		float x_coord;  //输入的芯片位置x坐标，坐标原点必须在图像中心
		float y_coord;  //输入的芯片位置y坐标，坐标原点必须在图像中心
		float angle;    //输入的芯片角度
	}DiePolarInputData; //输入待极性检测芯片的数据。

	typedef enum 
	{
		LEFT_RIGHT_LAY = 1, //极性标识在芯片上左右排布，内部默认为左右排布
		UP_DOWN_LAY = 2,    //极性标识在芯片上上下排布
	}DiePolarLayType;  //芯片极性排布方式

	typedef enum 
	{
		NONE_POLAR = 0,     //无法检出（可能极性标识不明显或存在干扰时的检测结果）
		CORRECT_POLAR = 1,  //极性正常（与输入模板的极性一致视为极性正常）
		OPPOSITE_POLAR = 2, //极性相反（与输入模板的极性相反视为极性相反）
	}DiePolarDetectResult;  //晶粒极性检测结果


#ifdef __cplusplus
}
#endif // __cplusplus



#endif