#ifndef __WAFER_DEFECT_DETECTOR_DATA_H__
#define __WAFER_DEFECT_DETECTOR_DATA_H__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
#include "ci_data.h"

	//关于空间坐标转换的数据
    typedef enum
    {
		MIDDLE_TOP               = 1,
		MIDDLE_CENTER            = 2,
		LEFT_TOP                 = 3,

    }WaferPosType;//位置类型，给定的机械坐标位置在图像的上边缘中心或者图像中心


	typedef enum
	{
		RIGHT_UPWARD                  = 1,//目前只针对空间坐标系向右进行了转换
		RIGHT_DOWNWARD                = 2,
		//LEFT_UPWARD                  = 3,
		//LEFT_DOWNWARD                = 4,

	}WaferSpaceCoordsType;//空间坐标系类型


	//关于缺陷评估的数据
	typedef enum
	{
		NORMAL     = 1,   //用于评估缺陷的每个数据评判时占比一致
		FAST       = 2,   //用对比度当做缺陷评判标准

	}WaferDefectMethod;//目前主要用于缺陷评定方式，后续也可扩展缺陷检测方式等


	typedef enum
	{
		ALL_DEFECT   = 0,    //该类别，不属于缺陷检出的任何类别，该类别用于区分缺陷图像输出的选择情况
		POINT_DEFECT = 1,    //点缺陷
		LINE_DEFECT  = 2,    //线缺陷
		BLOCK_DEFECT = 3,    //块缺陷

	}WaferDefectType;//缺陷类型



	typedef struct
	{
		int     in_multi_scale;      //尺度相关
		int     in_mask_size;        //mask
		int     in_contrast_ratio;   //对比度

	}WaferDefectDetectThresh;



	typedef struct
	{
		float in_point_parm;      //主要确认点缺陷的参数
		float in_aspect_ratio;    //主要用于确定线状缺陷，长宽比，一般用于区分线性缺陷，长宽比为>1的数值，一般为长边/短边
		float in_circle_ratio;    //主要用于确定块状缺陷， 圆形度范围，一般用于区分块状缺陷，圆形度范围为0-1，数值越高越类似块状缺陷

	}WaferDefectEvaluateParam;    //用于进行缺陷评估的具体参数


	typedef struct
	{
		WaferDefectType flag;          //当前轮廓的类型，主要类型为WaferDefectType定义，见上方
		float           score;         //当前轮廓的评价得分，评分范围0-1,1表示缺陷最严重
		float           mean_gray;	   //平均灰度值
		float           contrast;	   //对比度
		float           area;		   //像素面积
		float           len;		   //周长
		float           roundness;     //圆形度
		int             index;         //缺陷轮廓所在索引，从0开始
		AlgoRect        b_rect;        //最小外接正矩形

	} WaferDefectFeature;               //检测到的wafer缺陷特征结构体


	 


#ifdef __cplusplus
}
#endif // __cplusplus



#endif