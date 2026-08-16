#ifndef __SUBPIXEL_DISPLAY_EVALUATOR_DATA_H__
#define __SUBPIXEL_DISPLAY_EVALUATOR_DATA_H__

#include "ci_data.h"
#include "algo_mat_data.h"

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

	typedef enum 
	{
		TO_SHARPNESS = 0, //表示当前模糊程度不符合要求，模糊程度超过了目标范围，接下来焦距往清晰的方向调节
		TRUE = 1,         //表示模糊程度符合要求，相机焦距调节正确
		TO_BLUR = 2, //表示当前模糊程度不符合要求，模糊程度达不到目标范围，接下来继续往模糊的方向调节

	}BlurEvaluateResult; //相机模糊程度评价结果


	typedef enum 
	{
		THREE_LAYOUT=1, //表示子像素排布方式为三灯排布
		FOUR_LAYOUT=2,  //表示子像素排布方式为四灯排布

	}SubpixelLayoutType; //子像素屏体排布方式

	typedef struct 
	{
		float sharpness;   //清晰度评价权重
		float color_error; //颜色错误评价权重
		float sawtooth;    //锯齿评价权重

	}EvaluateWeight;  //测评权重

	typedef struct 
	{
		AlgoMat X_data;  //X滤片数据
		AlgoMat Y_data;  //Y滤片数据
		AlgoMat Z_data;  //Z滤片数据

	}AlgoMatXYZData;  //XYZ数据

	typedef struct 
	{
		float X;  //屏幕标准的X数据，一般为光枪采集的数据
		float Y;  //屏幕标准的Y数据，一般为光枪采集的数据
		float Z;  //屏幕标准的Z数据，一般为光枪采集的数据
	}StandardXYZ; //标准的XYZ数据，一般为光枪采集的数据

	typedef struct 
	{
		AlgoSize actual_resolution;   //实际分辨率。该分辨率是一个相对值，若外部设置理论分辨率，则该值是针对外部设置理论分辨率计算的相对值，外部不设置理论分辨率时，内部默认理论分辨率为2k
		AlgoPoint2f actual_ratio;     //实际显示比率。即该分辨率相对基准显示的倍率，如该值为0.8时，表示显示效果为实相素的80%
		float evaluate_value;         //清晰度显示评价值。该值越大表示清晰度越差，越小表示清晰度越好。//后面考虑反转
	}SharpnessResult; //清晰度评价结果

	typedef struct 
	{
		SharpnessResult sharpness_result; //清晰度评价结果
		float color_error_result;        //颜色错误评价结果
		float swatooth_result;           //锯齿评价结果

	}EvaluateResult;  //评价结果


#ifdef __cplusplus
}
#endif // __cplusplus
#endif 