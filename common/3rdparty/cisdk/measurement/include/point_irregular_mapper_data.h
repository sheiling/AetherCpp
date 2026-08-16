#ifndef __point_irregular_mapper_DATA_H__
#define __point_irregular_mapper_DATA_H__

#include "ci_data.h"
#include "algo_mat_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


	typedef enum
	{
		POINTIRMAP_STATE_FLAG_NONEXIST = 0b00000000,		// 物理不存在
		POINTIRMAP_STATE_FLAG_DEAD = 0b00000001,			// 死灯标志  
		POINTIRMAP_STATE_FLAG_LIVE = 0b00000010,			// 活灯标志
		POINTIRMAP_STATE_FLAG_ISOLATED = 0b00000011,		// 未排序标志
	} PointIrMapFlag;										// 灯点状态枚举变量：大表上不存在的是物理不存在；不亮的、find漏掉的是死灯；正常排列的灯点是活灯；未排列的是未排序；


	typedef enum {
		AUXILIARY_ROW = 0,									// 辅助行，用于2025.04交付的异形灯点排列算法优化
		AUXILIARY_COL = 1,									// 辅助列，用于2025.04交付的异形灯点排列算法优化
		AUXILIARY_NORMAL = 2,									// 常规逐点灯点坐标，死灯补全时可能用到
		AUXILIARY_EDGE = 3,									// 辅助边缘，对应分区上下左右的包络，用于箱体工具测试画面check点位图（大表）是否正确
	}AuxiliaryMapperType;									// 用于辅助mapper的pattern图像类型


	typedef enum
	{
		IRREGULAR_TYPE_POINT_RECT = 0,				// 灯点是规则的按照矩形排布，即灯点都同行同列，如经裁切后的常规矩形屏
		IRREGULAR_TYPE_POINT_ROW = 1,			// 灯点是规则的按照行排布，即灯点都同行不同列，行点间距基本一致，如球形屏  
	}IrregularType;										// 异形屏体的种类


	typedef struct
	{
		AlgoMat state;										// 灯点状态  ALGO_8UC1
		AlgoMat led_point_map;								// 灯点亚像素位置 ALGO_32FC2
		IrregularType ir_type;								// 异形屏体的种类
	}AlgoPointIrMap;										// 灯点排列结果数据结构体



#ifdef __cplusplus
}
#endif // __cplusplus

#endif // 
