#ifndef __WAFER_IMAGE_STITCHER_DATA_H__
#define __WAFER_IMAGE_STITCHER_DATA_H__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
#include "ci_data.h"
#include "algo_mat.h"


	//typedef enum
	//{
	//	PLANE_ARRAY_CAMERA = 1,
	//	LINEAR_ARRAY_CAMERA = 2,
	//
	//}CamType; //相机类型
	//
	//
	//typedef struct
	//{
	//	AlgoSize cam_size;                      //相机尺寸,拍出来图像的分辨率大小；【备注】：如果为线阵相机，则将高度设置为每次传输过来的图像的高度
	//	AlgoMat trans_mat1;                     //相机对应的坐标转换矩阵1;
	//	AlgoMat trans_mat2;                     //相机对应的坐标转换矩阵2；【备注】：如果为面阵相机，则将该参数可为一个空的mat;
	//	AlgoPoint2f cam_plat_coordinate;        //相机在空间载台坐标系中的位置
	//
	//}CamCoordInfo;   //相机信息
	typedef struct
	{
		int in_sample_rows;      //线扫相机每次传输过来的图像行数，像素数    
		int in_data_row;	     //当前样本是第几行             
	}OffSetPara;                 //偏移量信息 









	 


#ifdef __cplusplus
}
#endif // __cplusplus



#endif