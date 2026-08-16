#ifndef __UNFORMITY_MEASURER_DATA_H__
#define __UNFORMITY_MEASURER_DATA_H__

#include "ci_data.h"
#include "algo_mat_data.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus


	//***************************************输出结构体**********************
	typedef struct 
	{
		float mean;       //均值
		float variance;   //方差
		float max;        //最大值
		float min;        //最小值
		float uniformity; //均匀性,百分比，最大值为100，最小为0，即一组数据中数值完全一致时，均匀性为0。该值越大数据均匀性越差。

	}GrayDataFeature; //灰度数据特征

	typedef struct
	{
		float L_max;        //L数据最大值
		float L_min;        //L数据最小值
		float L_mean;       //L数据均值
		float L_variance;   //L数据方差
		float x_max;        //x数据最大值
		float x_min;        //x数据最小值
		float x_mean;       //x数据均值
		float x_variance;   //x数据方差
		float y_max;        //y数据最大值
		float y_min;        //y数据最小值
		float y_mean;       //y数据均值
		float y_variance;   //y数据方差
		float L_uniformity; //亮度均匀性，百分比，最大值为100，最小为0，即一组数据中数值完全一致时，均匀性为0。该值越大数据均匀性越差。
		float delta_uv;     //色差(色度均匀性)

	}ChromaDataFeature; //色度均匀性特征

	//***************************************输入预置量*******************************
	typedef struct 
	{
		int x_coord_min = 0;    //直方图x坐标最小值，即灰度级归一化的最小值，默认为0，可根据实际情况更改
		int x_coord_max = 63;   //直方图x坐标最大值，即灰度级归一化的最大值，默认为63（2^6-1），可根据实际情况更改
		int y_coord_min = 0;    //直方图y坐标最小值，即每个灰度级出现的数量归一化的最小值，默认为0，可根据实际情况更改
		int y_cootd_max = 100;  //直方图y坐标最大值，即每个灰度级出现的数量归一化的最大值，默认为100，可根据实际情况更改

	}HistogramParam;   //直方图参数；举例说明：一组数据中最小值为50，最大值为110，设置x_coord_min = 0，x_coord_max = 63，
	                                     //y_coord_min = 0，y_cootd_max = 100，则：内部会将该组数据归一化至0-63后，
	                                     //再统计直方图（即每个数据出现的次数），之后再将每个数据出现的次数归一化至0-100后输出。





#ifdef __cplusplus
}
#endif //__cplusplus
#endif