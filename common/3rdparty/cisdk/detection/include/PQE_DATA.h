#ifndef __PQE_DATA_H__
#define __PQE_DATA_H__

#include "ci_data.h"


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define M_PI (3.14159265358979323846)
#define MY_VECTOR_DEF_SIZE (1)
#define MES_MAX (100)
#define LOW_GRAY (16)
#define HIGH_GRAY (128)
#define MAX(X, Y) ((X)>(Y) ? (X) : (Y))
#define MIN(X, Y) ((X)<(Y) ? (X) : (Y))
#define CLIP(X, MIN, MAX)  ((X)>(MAX) ? (MAX) : ((X)<(MIN) ? (MIN) : (X)))

typedef enum
{
	OK,
	DATA_NULL,
	LARGE_OUTOF_RANGE,
	SMALL_OUTOF_RANGE,
	ERR,
}ReturnValue;

typedef struct lxy
{
	double l;
	double x;
	double y;
	double wave_length;
}S_LXY;

typedef struct LightPanelXYZ
{
	double Xr, Xg, Xb, Xw;
	double Yr, Yg, Yb, Yw;
	double Zr, Zg, Zb, Zw;
}LightPanelXYZ;

typedef struct LightPanelViewingAngle
{
	double Y_left;  //左视角
	double Y;        //中间位置
	double Y_right;  //右视角
}LightPanelViewingAngle;

typedef struct lab
{
	double l;
	double a;
	double b;
}S_LAB;

typedef struct itp
{
	double i;
	double t;
	double p;
}S_ITP;

typedef struct xyz
{
	double x;
	double y;
	double z;
}S_XYZ;

typedef struct thermaleffectparam
{
	double luminance;
	double temperature;
}S_Lumin_T;

typedef struct colorgamutparm
{
	double red_x;        //红点色坐标x
	double red_y;        //红点色坐标y
	double green_x;      //绿点色坐标x
	double green_y;      //绿点色坐标y
	double blue_x;       //蓝点色坐标x
	double blue_y;       //蓝点色坐标y
}S_COLOR_GAMUT;

typedef struct rgbvalue
{
	int r;
	int g;
	int b;
}S_RGB_VALUE;

typedef struct {
	int curSize;          // 已用的大小
	int maxSize;          // 数组最大存储大小
	double *data;          // 实际的数据地址
} S_VECTOR;

typedef struct  commcolorgamut
{
	S_COLOR_GAMUT rec709; //高清电视
	S_COLOR_GAMUT bt2020; //超高清
	S_COLOR_GAMUT p3; //超高清
	S_COLOR_GAMUT d65;//超高清
	commcolorgamut()
	{
		rec709.red_x = 0.640;
		rec709.green_x = 0.300;
		rec709.blue_x = 0.150;
		rec709.red_y = 0.330;
		rec709.green_y = 0.600;
		rec709.blue_y = 0.060;

		bt2020.red_x = 0.708;
		bt2020.green_x = 0.17;
		bt2020.blue_x = 0.131;
		bt2020.red_y = 0.292;
		bt2020.green_y = 0.797;
		bt2020.blue_y = 0.046;

		p3.red_x = 0.68;
		p3.green_x = 0.265;
		p3.blue_x = 0.15;
		p3.red_y = 0.32;
		p3.green_y = 0.69;
		p3.blue_y = 0.06;

		d65.red_x = 0.68;
		d65.green_x = 0.265;
		d65.blue_x = 0.15;
		d65.red_y = 0.32;
		d65.green_y = 0.69;
		d65.blue_y = 0.06;
	}
}S_COMM_COLORGAMUT;

typedef struct {
	double k;                      //整体线性度值
	double score;                  //整体加权
	S_VECTOR part_k;			  //分段线性度
	S_VECTOR part_k_diff_percent; //分段线性度值偏差
	S_VECTOR diff_percent;        //分段偏差百分比
	S_VECTOR mes_percent;         //均方误差
	S_VECTOR part_mes_percent;    //综合偏差
}S_Linearity;

typedef struct {
	int wavelen;	//波长
	float k;         //斜率
}Wave_Table;

typedef struct {
	float x;
	float y;
}Std_Light;


typedef struct {
	int start_gray; // 起始灰阶
	int end_gray;   // 结束灰阶
}Gray_Merge_Range;  // 灰阶融合范围

typedef struct
{
	bool                has_gray_inversion;           // 是否存在灰阶反跳
	bool                has_gray_merge_above8;        // 是否存在8灰阶以上的合并

	int                 gray_inversion_count;         // 跳灰灰阶数量
	int*                inversion_grays;              // 跳灰灰阶

	int                 gray_merge_count;             // 合并区段数量
	Gray_Merge_Range*   merge_gray_ranges;            // 区间合并的具体区段
}GrayScale_Lum_Order_Condition; // 灰阶亮度排序情况

typedef struct {
	int width;    // 箱体宽度灯点数量
	int height;   // 箱体高度灯点数量
} Pixel_Count;  // 箱体灯点数

/*
1.以箱体左上角为起始点，第一行/列表示为1
2.起始/结束的行/列灯点需要亮起
*/
typedef struct {
	int column_start; // 箱体宽起始位置
	int column_end;   // 箱体宽结束位置
	int row_start; // 箱体高起始位置
	int row_end;   // 箱体高结束位置
} Illuminated_Area;  // 箱体亮起灯点区域

typedef struct  // 刷新率计算参数 
{
	int min_avg_filter_width = 5;     // 均值滤波最小宽度
	int max_avg_filter_width = 50;    // 均值滤波最大宽度
	int max_peak_gap = 40;            // 最大峰间隔
	int max_std_gap = 80;             // 标准差判断阈值
	float min_lum = 0.5f;             // 最低亮度阈值，小于阈值返回帧频  
	float noise_thr = 0.25f;          // 噪声阈值
} RefreshParams;

// 采集设备刷新率
typedef enum
{
	ASTREIA_FREQUENCY = 186567,
	LF_FREQUENCY = 800000,
} RefreshSampleFrequency;

#ifdef __cplusplus
}
#endif // __cplusplus



#endif