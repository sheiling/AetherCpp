#ifndef __DL_SOLDER_DETECTOR_DATA_H__
#define __DL_SOLDER_DETECTOR_DATA_H__
#include <string>
#include"solder_detector_data.h"
#include "ci_data.h"
#include "algo_mat_data.h"

typedef struct
{
	float iou_th;         //iou阈值    取值范围在0到1之间，建议取0.1，若较大会出现多余的误检框
	float confidence_th;  //置信度阈值  取值范围在0到1之间，建议取0.5，若较大会出现漏检状况
}SolerThresh;

enum DefectDetection {
	NORMAL = 1,              //正常
	RESIDUAL = 2,            //残留
	SOLDER_BRIDGE = 3,       //连桥
	CRY_RESIDUAL = 4         //残晶   
};
#endif