#ifndef __SOLDER_DETECTOR_DATA_H__
#define __SOLDER_DETECTOR_DATA_H__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


	typedef enum
	{
		HORIAONTAL   = 1,         //水平方向，即焊盘一左一右
		VERTICAL     = 2,         //垂直方向，即焊盘一上一下
		SURROUND_PAD_MIP = 3,     //四个焊盘均匀分布，MIP灯珠焊盘形式

	}SolderDirection;





	typedef struct
	{
		float in_match_thresh;
		float in_low_thresh;
		float in_high_thresh;
		float in_pad_area_thresh  ;

	}SolderDetectThresh;



	typedef enum
	{	
		NO_DEFECT       = 0,
		BRIDGE_DEFECT   = 1,        //连锡缺陷
		DIRTY_DEFECT    = 2,        //残渣缺陷
		PAD_LOST        = 3,        //焊盘缺失
		GLUE_DEFECT     = 4,        //胶残留
		OXIDATE_DEFECT  = 5,        //氧化发黄
		CHIP_BROKEN     = 6,        //残晶电极


	}SolderDefectType;//缺陷类型








#ifdef __cplusplus
}
#endif // __cplusplus



#endif