#ifndef __VIGNETTING_CALIB_DATA_H__
#define __VIGNETTING_CALIB_DATA_H__

#include "meta_info_data.h"
#include "algo_mat_data.h"
#include "algo_mat.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

    typedef struct
    {
        Device device;
        AlgoColor color;
        AlgoMat x_data;//ALGO_32FC1
        AlgoMat y_data;
        AlgoMat z_data;
		AlgoMat x_kb_dark;//ALGO_32FC2暗噪声计算系数
		AlgoMat y_kb_dark;
		AlgoMat z_kb_dark;
		AlgoMat vigMat_resize_; //下采样暗角矩阵ALGO_32FC1：按X滤片光圈22、16、11、8、5.6、4、2.8、2，Y滤片各个光圈，z滤片各个光圈顺序拼接排列
		AlgoMat partition_wh; //分区区域大小ALGO_32SC1:(0,0)为区域宽,(0,1)为区域高，C3200orCC60均划分16个分区;
		AlgoMat k_labels_; //索引矩阵ALGO_8UC1：16个索引矩阵按列拼接，该矩阵宽为16，高为(分区宽*分区高);
		AlgoMat b_labels_; //索引矩阵ALGO_8UC1：16个索引矩阵按列拼接，该矩阵宽为16，高为(分区宽*分区高);
		AlgoMat k_centers_; //聚类中心矩阵ALGO_32FC1：16个聚类中心矩阵按列拼接;
		AlgoMat b_centers_; //聚类中心矩阵ALGO_32FC1：16个聚类中心矩阵按列拼接;
    } AlgoVignetting;
	//vig文件中新老数据类型不混用

	//typedef struct
	//{
	//	Device device;
	//	AlgoColor color;
	//	AlgoMat x_data;//ALGO_32FC1
	//	AlgoMat y_data;
	//	AlgoMat z_data;
	//	AlgoMat x_kb_dark;//ALGO_32FC2
	//	AlgoMat y_kb_dark;
	//	AlgoMat z_kb_dark;
	//	AlgoMat* x_kb_bright;
	//	AlgoMat* y_kb_bright;
	//	AlgoMat* z_kb_bright;
	//	int		 data_len;
	//} AlgoVignetting_;

	//void CreatAlgoVignetting_(AlgoVignetting_* in,int len) {
	//	AlgoVignetting_->x_kb_bright = malloc(sizeof(AlgoMat)*len);
	//	AlgoVignetting_->y_kb_bright = malloc(sizeof(AlgoMat)*len);
	//	AlgoVignetting_->z_kb_bright = malloc(sizeof(AlgoMat)*len);
	//};
	//void Release(AlgoVignetting_* in) {

	//	if(!AlgoVignetting_->x_kb_bright){
	//		free(AlgoVignetting_->x_kb_bright);
	//		AlgoVignetting_->x_kb_bright = NULL;
	//	}
	//	if (!AlgoVignetting_->y_kb_bright) {
	//		free(AlgoVignetting_->y_kb_bright);
	//		AlgoVignetting_->y_kb_bright = NULL;
	//	}
	//	if (!AlgoVignetting_->z_kb_bright) {
	//		free(AlgoVignetting_->z_kb_bright);
	//		AlgoVignetting_->z_kb_bright = NULL;
	//	}
	//}

#ifdef __cplusplus
}
#endif // __cplusplus
#endif