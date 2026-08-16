#ifndef __MURA_DATA_H__
#define __MURA_DATA_H__

#include "ci_data.h"
#include "algo_mat_data.h"


#ifdef __cplusplus
extern "C" {
#endif //__cplusplus
	typedef struct 
	{
		float mean_gray;		//mura平均灰度值
		float mean_gray_bg;		//背景区域平均灰度值
		float contrast;			//对比度
		float contrast_jnd;		//JND 对比度
		float mean_hue;			//mura平均色调角
		float mean_hue_bg;		//背景区域平均色调角
		float diff_hue;			//色调角偏差
		float area;				//实际物理尺寸，单位mm*mm
		int area_pixel;			//mura面积(pixel)
		float area_percent;		//实际mura面积占比
		int perimeter;			//mura周长，pixel
		float roundness;		//圆形度
		float rectangularity;	//矩形度
		float len_width_ratio;   //长宽比	

		float shape;            //mura形状
		float score;            //得分
		float semu;             //semu因子
		float dsemu;            //dsemu因子
		float locfator;         //位置因子
	}MuraFeature;               //mura特征结构体


	typedef struct 
	{
		float grade;            //整个屏体的mura等级，为所有mura评分的累加之和
	}MuraEvaluation;            //整个屏体的mura评估结果结构体

	typedef struct {
		AlgoPoint2f pos_pixel;				    //质心(pixel)
		AlgoPoint2f pos;						//实际物理位置坐标，单位mm
		AlgoRect rect;							//外接矩形(pixel)
		AlgoRect circle;						//外接圆形
		AlgoPoint* point_list;				    //mura边缘坐标
		int point_num;							//mura边缘坐标个数
	}MuraDescription;

	typedef struct
	{
		float diff_threshold;           //差分阈值，用于剔除背景与前景差异过小数据
		int area_threshold;           //mura面积筛选阈值，小于此面积的mura忽略
		float evaluation_threshold;   //全屏评价阈值，小于此阈值的mura得分不计入全屏评价
	}MuraThreshold;                   //mura评价阈值


	typedef struct 
	{
		int blocknum;           //分区个数
		float bg_ratio;         //背景区域尺寸倍率，bk_w = 1.5*w,bk_h = 1.5*h，内部默认为1.5
		int indent_pixel;       //内部缩进像素数,内部默认为10
		int detect_mode;        //检测模式，0-默认亮暗mura都检；1-暗；2-亮，内部默认亮暗都检
	}MuraDetectParam;           //mura检测参数

	typedef struct
	{
		float distance;         //相机拍摄距离，单位m,内部默认为1.5m
		float panel_w;          //被测显示屏实际物理尺寸，宽，单位：mm，内部默认为400mm
		float panel_h;          //被测显示屏实际物理尺寸，高，单位：mm，内部默认为200mm
		int panel_type;         //被测显示屏类型，0-unfefined;1-cob;2-cog;3-smd;4-miniLED backlight，内部默认为1：cob
	}MuraCaptureParam;          //mura拍摄参数


	typedef enum
	{
		RAW_DATA = 0,           //待检测数据从raw文件中读取的，非meter文件数据均认为是raw数据来源，如bmp数据等均可设置为raw
		METER_DATA = 1,         //待检测数据从meter文件中读取的

	}MuraDataType;         //mura检测数据来源类型

	typedef enum
	{
		Mode_Gray = 0,          //表示单通道检测模式,即灰度检测
		Mode_L = 1,             //在L通道上检测
		Mode_C = 2,             //在C通道上检测
		Mode_H = 3,             //在H通道上检测
		Mode_MIX = 4,           //LCH混合检测
		Mode_DE = 5,            //色差图上检测
	}MuraDetectMethod;          //检测方法


	typedef struct 
	{
		float k_score;           
		float k_contrast;
		float k_semu;
		float k_dsemu;
		float k_area;
		float k_location;
		float k_shape;
	}MuraEvaluWeight; //mura评价权重，内部依据该权重对mura进行测评


	typedef struct 
	{
		float lum_uniformity;    //亮度均匀性
		float chroma_uniformity; //色度均匀性
		float min_max_ratio;     //全局min/max
	}GlobalResult;  //全局均匀性检测结果

	typedef struct 
	{
		int block_row_num;       //行分区数量
		int block_col_num;       //列分区数量
		int downsamp_ratio = 5;  //下采样系数，默认为5,光通量数据时设为0

	}PartitionInfo;  //分区信息


	typedef struct 
	{
		int x;         //圆形中心点x坐标
		int y;         //圆形中心点y坐标
		int r;         //圆形区域半径值
	}CircularInfo;   //圆形区域信息,注意：输入的x、y坐标值必须大于半径值,x+r必须小于输入图像的宽，y+r必须小于输入图像的高


	typedef struct 
	{
		AlgoMat block_lum_uniformity;          //分区内亮度均匀性,AlgoMat行数、列数与分区行数、列数相一致，数值对应相应区域的亮度均匀性
		AlgoMat block_chroma_uniformity;       //分区内色度均匀性，AlgoMat行数、列数与分区行数、列数相一致，数值对应相应区域的色度均匀性
		AlgoMat block_lum_mean;                //分区内亮度均值，AlgoMat行数、列数与分区行数、列数相一致，数值对应相应区域的亮度均值
		AlgoMat block_chroma_mean_u;           //分区内色度均值u，AlgoMat行数、列数与分区行数、列数相一致，数值对应相应区域的u均值
		AlgoMat block_chroma_mean_v;           //分区内色度均值v，AlgoMat行数、列数与分区行数、列数相一致，数值对应相应区域的v均值
		AlgoMat block_chroma_mean_x;           //分区内色度均值x，AlgoMat行数、列数与分区行数、列数相一致，数值对应相应区域的x均值
		AlgoMat block_chroma_mean_y;           //分区内色度均值y，AlgoMat行数、列数与分区行数、列数相一致，数值对应相应区域的y均值
		AlgoMat block_min_max_ration;          //分区内最大/小值差异，AlgoMat行数、列数与分区行数、列数相一致，数值对应相应区域的最大值比最小值（均匀性的一种）
		float block_mutual_lum_uniformity;     //分区间亮度均匀性
		float block_mutual_chroma_uniformity;  //分区间色度均匀性
		float block_mutual_min_max_ratio;      //分区间min/max差异
		float deltaElab;                       //分区间的deltaElab
	}PartitionResult;  //分区均匀性检测结果

	typedef struct 
	{
		AlgoMat X_mean;                        //任意指定区域间的X均值，AlgoMat为一行n列数据，n列数据对应n个分区
		AlgoMat Y_mean;                        //任意指定区域间的Y均值，AlgoMat为一行n列数据，n列数据对应n个分区
		AlgoMat Z_mean;                        //任意指定区域间的Z均值，AlgoMat为一行n列数据，n列数据对应n个分区
		AlgoMat cx_mean;                       //任意指定区域间的x均值，AlgoMat为一行n列数据，n列数据对应n个分区
		AlgoMat cy_mean;                       //任意指定区域间的y均值，AlgoMat为一行n列数据，n列数据对应n个分区
		float multiregion_lum_uniformity;      //任意指定区域间的亮度均匀性
		
	}MultiRegionResult; //指定任意区域的均匀性检测结果  

	typedef struct 
	{
		float radiusRatio;			          //圆形区域的半径与面板实际宽度的比值
		float heightRatio;			          //高方向起始位置与面板实际高度的比值
		float widthRatio;			          //宽方向起始位置与面板实际宽度的比值
	}PointPositionInfo;  //九点的位置信息

	typedef struct 
	{
		AlgoMat nine_point_lum;                  //九点的亮度均值,AlgoMat为一行9列数据，9列数据对应9个区域数据
		AlgoMat nine_point_u;                    //九点的色度均值u，AlgoMat为一行9列数据，9列数据对应9个区域数据
		AlgoMat nine_point_v;                    //九点的色度均值v，AlgoMat为一行9列数据，9列数据对应9个区域数据
		float min_max_ratio_lum;                 //九点间的亮度均匀性
		float delta_uv;                          //九点间的色度均匀性
	}NinePointResult;   //指定九点的均匀性检测结果

	typedef struct 
	{
		int dim_num;                       //暗/死灯个数
		float dim_percentage;              //死/暗灯占百分比，单位%
		AlgoMat dim_position;			   //死/暗灯位置，AlgoMat为两行n列数据，第一行为死/暗灯x坐标，第二行为死/暗灯y坐标
		int bright_num;                    //亮点个数
		float bright_percentage;           //亮点所占百分比，单位%
		AlgoMat bright_position;           //亮点位置,AlgoMat为两行n列数据，第一行为亮灯x坐标，第二行为亮灯y坐标
	}AbnormalPointResult;

	typedef enum
	{
		DRAW_RED = 1,
		DRAW_GREEN = 2,
		DRAW_BLUE = 3,
		DRAW_WHITE = 4,
	}ExportImgColor;       //设置输出图像颜色

	typedef struct
	{
		float Lab_L;
		float Lab_a;
		float Lab_b;

	}DeltaERefer;     //计算DeltaE参考值

	typedef struct 
	{
		float max;     //最大值
		float min;     //最小值
		float mean;    //均值
	}DataInfoResult;  //统计数据信息

#ifdef __cplusplus
}
#endif //__cplusplus
#endif