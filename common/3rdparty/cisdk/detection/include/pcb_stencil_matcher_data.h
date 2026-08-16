#ifndef __PCB_STENCIL_MATCHER_DATA_H__
#define __PCB_STENCIL_MATCHER_DATA_H__

#include "ci_data.h"
#include "algo_mat_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


    //我们定义
    typedef enum
    {
		TOTAL_DEVIATION  = 1,        //表示差值总和的匹配方法，为当前内部默认的匹配方法，也是当前唯一的匹配方法，算法内部目前只开发了此方法。
		SINGLE_DIFF_DEVIATION = 2,   //表示只计算差异值的方法，该方法的检测结果输出是mat数据，外部根据需求设置
		GAUSS_DEVIATION  = 3,        //表示高斯统计匹配方法，当前内部未集成，若设置则内部报错，为预置的枚举


    }PCBStencilMatchFlag;//PCB钢网匹配策略方式

	typedef struct 
	{
		AlgoPoint2f plat_coord;   //相机取图时的机械位置

	}PCBStencilSpaceInfo;  //PCB钢网匹配空间信息

	typedef struct 
	{
		int image_width;         //相机拍摄图像的宽
		int image_height;        //相机拍摄图像的高
		AlgoMat convert_matrix;  //坐标转换矩阵，两行两列

	}PCBStencilCameraInfo; //PCB钢网匹配相机信息

	typedef enum 
	{
		COORD_TYPE_RIGHT_UP = 1,   //机械坐标类型1，x轴向右，y轴向上，内部默认的坐标类型
		COORD_TYPE_LEFT_UP = 2,    //机械坐标类型2，x轴向左，y轴向上。
		COORD_TYPE_RIGHT_DOWN = 3, //机械坐标类型3，x轴向右，y轴向下
		COORD_TYPE_LEFT_DOWN = 4,  //机械坐标类型4，x轴向左，y轴向下
	}PCBStencilCoordType;  //PCB钢网匹配机械坐标类型



#ifdef __cplusplus
}
#endif // __cplusplus



#endif