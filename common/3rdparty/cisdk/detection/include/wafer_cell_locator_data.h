#ifndef __WAFER_CELL_LOCATOR_DATA_H__
#define __WAFER_CELL_LOCATOR_DATA_H__

#include "ci_data.h"
#include "algo_mat.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
	
	////cell错位信息
	//typedef enum
	//{
	//	VERTICAL = 0,       //垂直方向
	//	HORIZONTAL = 1	    //水平方向
	//}WaferCellPadDirection;
	////输入的检测阈值信息
	//typedef struct
	//{
	//	int segment_th;       //分割阈值
	//	float match_th;       //模板匹配阈值
	//	int merge_dis_th;    //垂直方向两张样本pad组合并的阈值，小于该值认为上下两张样本的pad组是同一组
	//	float mech_move_acc;    //机械重叠精度,像素数
	//	AlgoPoint2f vertex_roi;  //检测顶点用到的ROI的宽度和高度
	//}WaferCellPadTh;    //WaferCellPad Cell定位阈值
	//输入的Cell的信息

	typedef struct
	{                     
		AlgoSize cell_size;                             //cell的大小,像素大小
		AlgoSize cell_gap_size;  				        //cell间隔的尺寸,像素大小
		int cell_pad_num;          				        //每个cell共有几组pad
		int pad_num;			   				        //每组pad共有几个pad点
		AlgoSize max_pad_size;   				        //最大Pad的大小
		int cell_staggered_direction;                   //cell错行的朝向,水平错行还是垂直错行,0代表垂直方向存在错行，1代表水平方向存在错行
		int pad_direction;                              //cell上pad的排列方式，0代表垂直方向排列，1代表水平方向排列
	}WaferCellPadInfo;     //wafer上 Cell的信息


	typedef struct
	{
		float in_sample_rows;       //线扫相机每次传输过来的图像行数    
		float in_v_offset;  	   //垂直方向上重叠量，即上下两张样本重叠的像素数
		float in_h_offset;         //水平方向上的偏移量，即线扫相机水平方向上移动一次的距离，像素数
		int in_data_rows;		   //当前样本是第几行
		int in_data_cols;   	  // 当前样本是第几列                      
	}WaferCellOffSetInfo;     //偏移量信息  备注：如果要转换为实际物理坐标，WaferCellOffSetInfo结构体内所有的值也都需要为物理坐标



	//在此处定义传出去的cell结果
	typedef struct
	{
		AlgoMat vertex;                  //当前corner的顶点信息
		AlgoMat pad;                     //当前corner的pad信息

	}WaferCellCornerData;                //一个cell的其中一个顶点的信息

	typedef struct
	{
		int cell_idx;                    //当前cell是所有检测到的cell里面的第几个
		int cell_rows;                   //当前cell在所有检测到的cell里面位于第几行
		int cell_cols;                   //当前cell在所有检测到的cell里面位于第几列
		int corner_num;                  //一个cell中，检测到的顶点的数量
		AlgoPoint2f cell_center;         //cell的中心坐标
		WaferCellCornerData corner0;     //cell左上角顶点
		WaferCellCornerData corner1;     //cell右上角顶点
		WaferCellCornerData corner2;     //cell左下角顶点
		WaferCellCornerData corner3;     //cell右下角顶点
	}WaferOneCell;
	//WaferCellInfo


#ifdef __cplusplus
}
#endif // __cplusplus



#endif