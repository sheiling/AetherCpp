#ifndef __SPATIAL_FUSION_CELL_LOCATOR_DATA_H__
#define __SPATIAL_FUSION_CELL_LOCATOR_DATA_H__

#include "ci_data.h"
#include "algo_mat.h"
#include "wafer_defect_detector_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus



	typedef enum
	{
		PLANE_ARRAY_CAMERA    = 1,  
		LINEAR_ARRAY_CAMERA   = 2,

	}CamType; //相机类型





	typedef struct
	{
		AlgoSize cam_size;                    //相机尺寸,拍出来图像的分辨率大小；【备注】：如果为线阵相机，则将高度设置为每次传输过来的图像的高度
		AlgoMat trans_mat1;                   //相机对应的坐标转换矩阵1;
		AlgoMat trans_mat2;                   //相机对应的坐标转换矩阵2；【备注】：如果为面阵相机，则可将trans_mat1设置为空;
		AlgoPoint2f cam_plat_coordinate;       //相机在空间载台坐标系中的位置
	}CamCoordInfo;   //相机信息




	typedef struct
	{
		AlgoSize2f cell_size;           //cell的大小
		AlgoSize2f cell_gap_size;  		//cell间隔的尺寸
		int cell_pad_num;          		//每个cell共有几组pad
		int pad_num;			   		//每组pad共有几个pad点
		int cell_staggered_direction;   //cell错行的朝向,水平错行还是垂直错行,0代表垂直方向存在错行，1代表水平方向存在错行
		int pad_direction;              //cell上pad的排列方式，0代表垂直方向排列，1代表水平方向排列
	}WaferCellInfo;     //wafer信息


	//关于CellPad定位的数据结构
	typedef struct
	{
		AlgoMat vertex;                  //当前corner的顶点信息
		AlgoMat pad;                     //当前corner的pad信息,algomat的行数<=6

	}CornerData;                //一个cell的其中一个顶点的信息

	typedef struct
	{
		WaferDefectFeature defect_feature;      //一个缺陷的特征    
		AlgoMat defect_contour;                 //一个缺陷特征对应的轮廓信息        
	}WaferDefectInfo;                           //检测到的一个缺陷的信息


	typedef struct
	{
		int cell_idx;                                //当前cell是所有检测到的cell里面的第几个
		int cell_rows;                               //当前cell在所有检测到的cell里面位于第几行
		int cell_cols;                               //当前cell在所有检测到的cell里面位于第几列
		int corner_num;                              //一个cell中，检测到的顶点的数量 ，当corner_num==3时，没有检测到的顶点内部所有值均为-1
		AlgoPoint2f cell_center;                     //cell的中心坐标
		CornerData corner0;                          //cell左上角顶点
		CornerData corner1;                          //cell右上角顶点
		CornerData corner2;                          //cell右下角顶点
		CornerData corner3;                          //cell左下角顶点
		int cell_defect_num;                         //cell缺陷的数量		
	}WaferOneCellInfo; 
   
	typedef struct
	{
		int cell_idx;                            //该cell的索引
		int cell_defect_num;                     //该cell上含有缺陷的个数
		WaferDefectInfo cell_defect_result;      //该cell上所有缺陷的结果
	}WaferOneCellDefectInfo;


	typedef struct
	{
		int cell_idx;                                //该cell的索引
		int cell_illumination_dead_num;              //cell点亮后，死灯个数
		int cell_illumination_dim_num;			     //cell点亮后，暗灯个数
		int cell_illumination_bright_num;		     //cell点亮后，亮灯个数
		AlgoMat cell_illumination_dead_points;       //cell点亮后，死灯结果
		AlgoMat cell_illumination_dim_points;        //cell点亮后，暗灯结果
		AlgoMat cell_illumination_bright_points;     //cell点亮后，亮灯结果
	}WaferOneCellIlluminationInfo;


	/*方式1
	备注：AlgoMat cell_illumination_result  存储了cell的点亮结果
	cell_illumination_result是一个n行3列的AlgoMat；
	第n行第一个坐标代表灯点的x坐标；
	第n行第二个坐标代表灯点的y坐标；
	第n行第三个坐标代表灯点的缺陷标识符，标识符与led_meter_inspector_data.h中的枚举类型LEDDefectFlag相对应；
	方式2：
	直接将点亮结果拿过来，放进去
	*/



	typedef struct
	{
		int in_sample_rows;      //线扫相机每次传输过来的图像行数，像素数    
		int in_overlap_num;      //前后两张样本的重叠量，像素数
		int in_data_row;	     //当前样本是第几行             
	}OffSetInfo;                 //偏移量信息 






#ifdef __cplusplus
}
#endif // __cplusplus



#endif