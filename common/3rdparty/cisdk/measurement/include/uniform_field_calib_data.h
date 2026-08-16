#ifndef __UNIFORM_FIELD_CALIB_DATA_H__
#define __UNIFORM_FIELD_CALIB_DATA_H__

#include "meta_info_data.h"
#include "algo_mat_data.h"
#include "algo_mat.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	typedef enum
	{
		UniformFieldType_UNDEFINE = 0,
		UniformFieldType_COMPRESS = 1,       //压缩的文件
		UniformFieldType_NON_COMPRESS = 2,   //解压后的文件格式、仅包含一个光圈的相关属性
		UniformFieldType_TEMP = 3,             //中间步骤暗噪声系数计算
	}UniformFieldType;

    typedef struct
    {
		UniformFieldType type;
        Device device;
        AlgoColor color;
		int aperture;
		AlgoRect rect;
        AlgoMat x_k;//ALGO_16FC1
        AlgoMat y_k;
        AlgoMat z_k;
		AlgoMat x_b;//ALGO_8UC1
		AlgoMat y_b;
		AlgoMat z_b;
		AlgoMat x_kb_dark;//ALGO_32FC2暗噪声计算系数
		AlgoMat y_kb_dark;
		AlgoMat z_kb_dark;
		AlgoMat rectify_vig_; //下采样暗角矩阵ALGO_32FC1：按X滤片光圈由大到小，Y滤片各个光圈，z滤片各个光圈顺序拼接排列
		AlgoMat partition_wh; //分区区域大小ALGO_32SC1:(0,0)为区域宽,(0,1)为区域高，C3200orCC60均划分16个分区;
		AlgoMat k_labels_; //索引矩阵ALGO_8UC1：16个索引矩阵按列拼接，该矩阵宽为16，高为(分区宽*分区高);
		AlgoMat b_labels_; //索引矩阵ALGO_8UC1：16个索引矩阵按列拼接，该矩阵宽为16，高为(分区宽*分区高);
		AlgoMat k_centers_; //聚类中心矩阵ALGO_32FC1：16个聚类中心矩阵按列拼接;
		AlgoMat b_centers_; //聚类中心矩阵ALGO_32FC1：16个聚类中心矩阵按列拼接;
    } AlgoUniformField;
	
	typedef struct
	{
		uint8_t k_d_bitshift;
		uint8_t b_d_bitshift;
		uint8_t vig_bitshift;
		uint8_t k_center_bitshift;
		uint8_t b_center_bitshift;
		uint16_t b_center_base;
	}QuantifyParam;



#ifdef __cplusplus
}
#endif // __cplusplus
#endif