#ifndef __UNFORMITY_MEASURER_H__
#define __UNFORMITY_MEASURER_H__

#include "ci_data.h"
#include "ci_declare.h"
#include "unformity_measurer_data.h"
#include "unformity_measurer_error.h"
#include "algo_mat.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

	/***************************************
	@brief   : 计算灰度数据特征
	@note    : 若计算全局数据特征，in_region_rect参数不需要输入，若计算局部区域特征，请输入in_region_rect区域信息
	@param   : in_date                输入的灰度数据
	@param   : in_region_rect         输入的局部区域rect信息，默认为空，可根据需求输入
	@param   : out_feature            输出的灰度数据特征结构体
	@return  : AlgoError
	@Author  : wushaoshuang@2023/11/21
	****************************************/
	C_API AlgoError C_CALL UnformityCalcGrayFeature(AlgoMat* in_data, GrayDataFeature* out_feature, AlgoRect* in_region_rect = nullptr);

	/***************************************
	@brief   : 计算数据直方图信息
	@note    : 若计算全局数据直方图，in_region_rect参数不需要输入，若计算局部区域直方图，请输入in_region_rect区域信息
	@param   : in_date              输入的数据
	@param   : in_region_rect       输入的区域rect信息,默认为空，可根据需求输入
	@param   : in_histogram_param   输入的直方图参数结构体
	@param   : out_histogram_data   输出的直方图信息，以n*2（2行n列）大小的AlgoMat数据类型给出，从上往下，第一行数据为直方图的x值，第二行数据为直方图的y值
	@return  : AlgoError
	@Author  : wushaoshuang@2023/11/21
	****************************************/
	C_API AlgoError C_CALL UnformityCalcHistogram(AlgoMat* in_data, HistogramParam* in_histogram_param, AlgoMat* out_histogram_data, AlgoRect* in_region_rect = nullptr);

	/***************************************
	@brief   : 计算多点间的灰度特征
	@note    :
	@param   : in_date              输入的灰度数据
	@param   : in_multi_rect        输入的多区域信息，需要外部new分配内存
	@param   : in_rect_num          输入多区域的数量，外部new的内存空间大小（即有多少个区域）
	@param   : out_MultiRect_result 输出多区域间的灰度特征
	@return  : AlgoError
	@Author  : wushaoshuang@2023/11/21
	****************************************/
	C_API AlgoError C_CALL UnformityCalcMultiRegionFeature(AlgoMat* in_data, AlgoRect* in_multi_rect, int in_rect_num, GrayDataFeature* out_MultiRect_result);

	/***************************************
	@brief   : 计算亮色度数据特征
	@note    : 输入的数据为L、x、y数据，若计算全局数据特征，in_region_rect参数不需要输入，若计算局部区域特征，请输入in_region_rect区域信息
	@param   : in_L_date              输入的整屏L数据
	@param   : in_x_date              输入的整屏x数据
	@param   : in_y_date              输入的整屏y数据
	@param   : in_region_rect         输入的区域rect信息,默认为空，可根据需求输入
	@param   : out_chroma_feature     输出的色度特征
	@return  : AlgoError
	@Author  : wushaoshuang@2023/11/21
	****************************************/
	C_API AlgoError C_CALL UnformityCalcChromaFeature(AlgoMat* in_L_data, AlgoMat* in_x_data, AlgoMat* in_y_data, ChromaDataFeature* out_chroma_result, AlgoRect* in_region_rect = nullptr);

	/***************************************
	@brief   : 计算主波长数据
	@note    : 输入的数据为L、x、y数据，若计算全局主波长，in_region_rect参数不需要输入，若计算局部主波长，请输入in_region_rect区域信息
	@param   : in_L_date                输入的L数据
	@param   : in_x_date                输入的x数据
	@param   : in_y_date                输入的y数据
	@param   : in_threads_num           输入的线程数，根据实际情况输入，默认为1，线程数越多，计算主波长越快，设置为8时，3200相机分辨率数据计算主波长时间可缩短至3-4s。
	@param   : in_region_rect           输入的区域rect信息,默认为空，可根据需求输入
	@param   : out_dominant_waveLength  输出的主波长数据
	@return  : AlgoError
	@Author  : wushaoshuang@2023/11/21
	****************************************/
	C_API AlgoError C_CALL UnformityCalcDWaveLength(AlgoMat* in_L_data, AlgoMat* in_x_data, AlgoMat* in_y_data, AlgoMat* out_dominant_waveLength, int in_threads_num = 1, AlgoRect* in_region_rect = nullptr);

	/***************************************
	@brief   : 计算多点间的亮色度信息
	@note    : 输入的数据为L、x、y数据
	@param   : in_L_date                输入的整屏L数据
	@param   : in_x_date                输入的整屏x数据
	@param   : in_y_date                输入的整屏y数据
	@param   : in_multi_rect            输入的多区域rect信息，需要外部new内存空间
	@param   : in_rect_num              输入的多区域数量信息，外部new内存空间的大小
	@param   : out_MultiRect_feature    输出的多区域间的色度特征
	@return  : AlgoError
	@Author  : wushaoshuang@2023/11/22
	****************************************/
	C_API AlgoError C_CALL UnformityCalcMultiRectChromaFeature(AlgoMat* in_L_data, AlgoMat* in_x_data, AlgoMat* in_y_data, AlgoRect* in_multi_rect, int in_rect_num, ChromaDataFeature* out_MultiRect_feature);

	/***************************************
	@brief   : 计算多点间的主波长数据
	@note    : 输入的数据为L、x、y数据
	@param   : in_L_date                输入的整屏L数据
	@param   : in_x_date                输入的整屏x数据
	@param   : in_y_date                输入的整屏y数据
	@param   : in_multi_rect            输入的多区域rect信息，需要外部new内存空间
	@param   : in_rect_num              输入的多区域数量信息，外部new内存空间的大小
	@param   : out_dominant_waveLength  输出的各区域的主波长均值，以1行n列的AlgoMat类型返回，n代表n个点，各位置的值代表各区域的主波长均值
	@param   : in_threads_num           输入的线程数，根据实际情况输入，默认为1，线程数越多，计算主波长越快，设置为8时，3200相机分辨率数据计算主波长时间可缩短至3-4s。
	@return  : AlgoError
	@Author  : wushaoshuang@2023/11/22
	****************************************/
	C_API AlgoError C_CALL UnformityCalcMultiRectDWaveLength(AlgoMat* in_L_data, AlgoMat* in_x_data, AlgoMat* in_y_data, AlgoRect* in_multi_rect, int in_rect_num, AlgoMat* out_dominant_waveLength, int in_threads_num = 1);

	/****************************************
	@brief   : 计算Lxy数据
	@note    : 输入的数据为X、Y、Z数据，输出为L、x、y数据，若计算全局数据，in_region_rect参数不需要输入，若计算局部数据，请输入in_region_rect区域信息
	@param   : in_X_date                输入的X数据
	@param   : in_Y_date                输入的Y数据
	@param   : in_Z_date                输入的Z数据
	@param   : in_region_rect           输入的区域rect信息,默认为空，可根据需求输入
	@param   : out_L_data               输出的亮度数据
	@param   : out_x_data               输出的色度x数据
	@param   : out_y_data               输出的色度y数据
	@return  : AlgoError
	@Author  : wushaoshuang@2023/12/22
	*****************************************/
	C_API AlgoError C_CALL UnformityCalcLxyData(AlgoMat* in_X_data, AlgoMat* in_Y_data, AlgoMat* in_Z_data, AlgoMat* out_L_data,AlgoMat* out_x_data, AlgoMat* out_y_data, AlgoRect* in_region_rect = nullptr);

	/****************************************
	@brief   : 计算多区域Lxy数据
	@note    : 输入的数据为X、Y、Z数据
	@param   : in_X_date                输入的X数据
	@param   : in_Y_date                输入的Y数据
	@param   : in_Z_date                输入的Z数据
	@param   : in_multi_rect            输入的多区域rect信息，需要外部new内存空间
	@param   : in_rect_num              输入的多区域数量信息，外部new内存空间的大小
	@param   : out_L_data               输出的多区域亮度数据
	@param   : out_x_data               输出的多区域色度x数据
	@param   : out_y_data               输出的多区域色度y数据
	@return  : AlgoError
	@Author  : wushaoshuang@2023/12/22
	*****************************************/
	C_API AlgoError C_CALL UnformityCalcMultiRectLxyData(AlgoMat* in_X_data, AlgoMat* in_Y_data, AlgoMat* in_Z_data, AlgoRect* in_multi_rect, int in_rect_num, AlgoMat* out_L_data, AlgoMat* out_x_data, AlgoMat* out_y_data);

	/****************************************
	@brief   : 计算色温数据
	@note    : 输入的数据为x、y数据
	@param   : in_x_date                输入的x数据
	@param   : in_y_date                输入的y数据
	@param   : in_region_rect           输入的区域rect信息,默认为空，可根据需求输入
	@param   : out_CCT_data             输出的色温数据
	@return  : AlgoError
	@Author  : wushaoshuang@2023/12/27
	*****************************************/
	C_API AlgoError C_CALL UnformityCalcCCT(AlgoMat* in_x_data, AlgoMat* in_y_data, AlgoMat* out_CCT_data, AlgoRect* in_region_rect = nullptr);

	/****************************************
	@brief   : 计算多区域色温数据
	@note    : 输入的数据为x、y数据
	@param   : in_x_date                输入的x数据
	@param   : in_y_date                输入的y数据
	@param   : in_multi_rect            输入的多区域rect信息，需要外部new内存空间
	@param   : in_rect_num              输入的多区域数量信息，外部new内存空间的大小
	@param   : out_CCT_data             输出的色温
	@return  : AlgoError
	@Author  : wushaoshuang@2023/12/27
	*****************************************/
	C_API AlgoError C_CALL UnformityCalcMultiRectCCT(AlgoMat* in_x_data, AlgoMat* in_y_data, AlgoRect* in_multi_rect, int in_rect_num, AlgoMat* out_CCT_data);

#ifdef __cplusplus
}
#endif
#endif
