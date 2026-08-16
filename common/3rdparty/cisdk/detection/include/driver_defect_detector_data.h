#ifndef __DRIVER_DEFECT_DETECTOR_DATA_H__
#define __DRIVER_DEFECT_DETECTOR_DATA_H__


#include "ci_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	typedef struct
	{
		/*
		scan_direction:
		scan_direction = 0,扫描方向为水平方向,scan_direction = 1,扫描方向为垂直方向;

		scan_flash_direction:
		scan_flash_direction = 0,(配合垂直扫描方向)刷新方向为,从左到右,--->--->--->;
		scan_flash_direction = 1,(配合垂直扫描方向)刷新方向为,从右到左,<---<---<---;
		scan_flash_direction = 2,(配合水平扫描方向)新方向为,从上到下;
		scan_flash_direction = 3,(配合水平扫描方向)新方向为,从下到上;

		scan_nums：
		每个驱动IC可控制的IC灯点数量

		*/
		int scan_direction;        //行管扫描方向
		int scan_flash_direction;  //行管刷新方向		
		int scan_nums;		       //行管可控制的扫数
		 //未来可进行补充······
	}LEDDriverInfo;

	typedef struct
	{
		bool is_ghosting;           //是否检测到鬼影，若检测到鬼影，为true
		float feature1;             //鬼影特征值，值越大，代表鬼影现象越严重;
	}GhostingResult;

	typedef struct
	{
		bool is_coupling;           //是否检测到耦合，若检测到耦合，为true
		float feature1;				//耦合特征值，绝对值值越大，代表耦合现象越严重;
									//当耦合特征值为正数时,代表耦合区域比周围亮;当耦合特征值为负数时,代表耦合区域比周围暗;
	}CouplingResult;
	typedef struct
	{
		bool is_first_scan_defect;  //是否检测到第一扫异常现象，若检测到第一扫异常现象，为true
		float feature1;             //第一扫异常特征值，绝对值值越大，代表第一扫异常现象越严重;
									//当第一扫异常特征值为正数时,代表第一扫异常区域比周围亮;当第一扫异常特征值为负数时,代表第一扫异常区域比周围暗;
	}FirstScanResult;

	typedef struct
	{
		int detect_nums;					//【阈值】每个正常灯点旁边,参与鬼影计算的灯点数量;
											//1) 默认值2;
											//2) 调小，检测范围变小；
											//3）调大，检测范围变大；
		float match_th;						//【阈值】灯点得分，当大于该值,才认为灯点为亮起状态;
											//1) 默认值：R/G/B:0.3f W:0.4f；调节范围[0.3f,1.f]
											//2) 调小，检测越灵敏，存在误检的可能；
											//3) 调大，检测放宽松，存在漏检的可能；											
		float detect_th;					//【阈值】是否为鬼影的判断阈值，当鬼影特征值大于该值,才认为存在鬼影现象;
											//1) 默认值：R/G/B: 25.f;
											//2) 调小，检测越灵敏，存在误检的可能
											//3) 调大，检测放宽松，存在漏检的可能，
	}DriverDefectGhostingTh; //鬼影阈值

	typedef struct
	{
		float contrast_th;			         //第一扫区域的对比度
											 //contrast_th = 0.8f时,当第一扫区域与正常区域亮度差异大于0.8%时,认为该第一扫区域为异常区域
									         //1)默认值，0.8f，调节范围：（0，5.f]；
									         //2)调小，检测越灵敏，存在误检可能；
									         //3)调大，检测越宽松，存在漏检可能；
		float num_th;				         //第一扫异常区域数量阈值,当num_th = 0.5f,第一扫异常区域的数量大于50%时,认为全局存在第一扫异常现象;
									         //1）默认值，0.5f，调节范围：（0，1.f]
									         //2)调小，检测要求降低，即使很少数量的第一扫异常，也会被认为全局存在第一扫异常，抗干扰性降低；
									         //3)调大，检测要求提高，存在较多数量的第一扫异常，才会被认为全局存在第一扫异常，检测标准高；
	}DriverDefectFirstScanTh;//第一扫异常阈值


	typedef struct
	{
		float contrast_th;			         //耦合区域的对比度
											 //contrast_th = 0.85f时,当耦合与正常区域亮度差异大于0.85%时,认为该耦合区域为异常区域
											 //1)默认值，0.85f，调节范围：（0，5.f]，
									         //2)调小，检测越灵敏，存在误检可能；
									         //3)调大，检测越宽松，存在漏检可能；
		float num_th;				         //耦合区域数量阈值,当num_th = 0.5f,耦合区域的数量大于50%时,认为全局存在耦合现象;
									         //1）默认值，0.5f，调节范围：（0，1.f]
									         //2)调小，检测要求降低，即使很少数量的耦合，也会被认为全局存在耦合，抗干扰性降低；
									         //3)调大，检测要求提高，存在较多数量的耦合，才会被认为全局存在耦合，检测标准高；
									   	      
	}DriverDefectCouplingTh; //耦合阈值
	
#ifdef __cplusplus
}
#endif // __cplusplus
#endif