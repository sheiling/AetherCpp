#ifndef __VIEWING_ANGLE_MODEL_EMULATOR_DATA_H__
#define __VIEWING_ANGLE_MODEL_EMULATOR_DATA_H__

#include "meta_info_data.h"
#include "algo_mat_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus



// 定义坐标向量结构体
struct Coordinate {
	double x, y, z;
};


typedef struct
{
	AlgoColor  color; //记录颜色
	float led_interval; //记录点间距
	AlgoSize  box_size;//测评时箱体尺寸
	AlgoMat Angles;//采集的角度数组
	

	AlgoMat X_hori; //各通道的配光曲线数据
	AlgoMat X_vert;
	AlgoMat Y_hori;
	AlgoMat Y_vert;
	AlgoMat Z_hori;
	AlgoMat Z_vert;
}LightDistrib;


struct ViewPos
{
	float distance; //单位毫米
	float horizontal_angle;  //水平观看角度，单位°
	float vertical_angle;  //垂直观看角度，单位°
};

//配光曲线和颜色滤片封装，  输出的评分放在这里

#define  LIGHTCURVEANGLES   81


const float  PI = 3.141592653586f;



#ifdef __cplusplus
}
#endif // __cplusplus
#endif