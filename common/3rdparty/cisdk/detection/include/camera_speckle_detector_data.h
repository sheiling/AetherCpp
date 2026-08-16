#ifndef __CAMERA_SPECKLE_DETECTOR_DATA_H__
#define __CAMERA_SPECKLE_DETECTOR_DATA_H__

#include "ci_data.h"
#include "algo_mat_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	typedef struct
	{
		float contrast_thresh; //对比度检测阈值，该对比度即亮暗差异度阈值，单位为%，如想检测差异度1%以上的斑点，那么该值设置1.0。内部默认为1%，外部根据需求调整,该值不能小于0,内部默认0.5。
		float edge_thresh;    //边缘检测阈值，该值为控制图像边缘斑点的检出情况，内部默认为0.3，调节范围为大于0，该值的调节步长为0.1，当图像边缘出现过检时（边缘黑边被检出时），则增大该值，当图像边缘出现漏检时（边缘上的明显斑点未检出时），则减小该值。
		int area_thresh;       //面积检测阈值，即亮暗斑面积阈值，多少面积以下的亮暗斑直接忽略，外部根据需求设置，内部默认为3*3=9。
		int filter_size;       //滤波核的大小，表示算法兼容目标边缘渐变的程度，内部默认为201，该值一般无需修改，属于预置参数,若检测图像中存在大面积斑点未检出时，调大该值，步长以50来调试。
	}CameraSpeckleThresh;

	typedef struct 
	{
		int speckle_type = 2;        //检测目标类型，0为仅检测暗斑，1为仅检测亮斑，2为亮暗斑都检测
		int img_scale_num = 1;       //图像放缩次数，每次缩小为前一次的一半，可以加快算法运算速度，但会丢失部分精度。外部根据需求设置，内部默认为1次，0时不缩放，以原图检测。
		float back_expand_radio=1.5; //背景扩充比例，即计算差异时，取斑点候选区域大小的多少倍来当作背景区域求背景均值，内部默认为1.5倍，外部根据情况设置即可，该值必须大于1.
		AlgoRect speckle_rect;   //所需检测的斑点在图像中的区域，即只在该rect区域内进行斑点检测，其他位置的忽略不计，内部默认该区域为整幅待测图像大小，外部根据需求设置即可。

	}CameraSpeckleParameter;//亮暗斑检测参数

	typedef struct 
	{
		int area_max;             //斑点区域面积的最大值
		int area_min;             //斑点区域面积的最小值
		int A_failed_num;         //A类不合格区域的数量
		int B_failed_num;         //B类不合格区域的数量
		float A_failed_diff_max;  //A类不合格差异度的最大值
		float A_failed_diff_min;  //A类不合格差异度的最小值
		float B_failed_diff_max;  //B类不合格差异度的最大值
		float B_failed_diff_min;  //B类不合格差异度的最小值
		float single_rect_diff;   //单区域差异度
		    
	}CameraJudgeStandard;//相机判断标准，根据实际需求设定，该结构体仅表示一个面积区间，若有多个面积区间，请以数组形式传入接口。注：该结构体内的区间表示全部默认为左闭右开区间，如面积最大值为100，最小值为50，那么所表示的面积区间为[50,100)，其他的雷同。

	typedef struct 
	{
		AlgoMat contour_Point;  //表示每个亮暗斑的轮廓点，以n行两列的AlgoMat形式储存，第一列数据为轮廓点的x值，第二列数据为轮廓点的y值
		int contours_area;      //表示轮廓的面积，即当前斑点的面积
		float contrast;         //表示当前亮暗斑的差异度，单位为%
		int center_x;           //质心x
		int center_y;           //质心y

	}CameraSpeckleFeature; //每个亮暗斑的特征



#ifdef __cplusplus
}
#endif // __cplusplus
#endif