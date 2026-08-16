#ifndef __CI_FRONTEND_MODNAME_H__
#define __CI_FRONTEND_MODNAME_H__


//基础模块：CI框架前段链路基础模块，模块编号范围0-128，不可超出该范围--25个模块，共26个错误码-ci_base分为log和mat
#define	    ALGOMOD_ALGOLOG                         0             //日志模块
#define		ALGOMOD_ALGOMAT                         1             //AlgoMat
#define		ALGOMOD_UTILS                           2             //utils模块
#define		ALGOMOD_ALGOSHOT                        3             //shot模块
#define		ALGOMOD_ROI_DETECTOR                    4             //区域定位
#define		ALGOMOD_QUALITY_ESTIMATOR               5             //质量评价
#define		ALGOMOD_LED_POINT_LOCATOR               6             //点定位
#define		ALGOMOD_LED_COUTOUR_SEARCHER            7             //灯点框模块
#define		ALGOMOD_LED_LUM_EXTRACTOR               8             //光通量提取
#define		ALGOMOD_ALGOLEDMETER                    9             //ledmeter模块
#define		ALGOMOD_VIGNETTING_CALIB               10             //渐晕标定模块
#define		ALGOMOD_BLUR_LED_LUM_EXTRACTOR         11             //模糊光通量提取
#define		ALGOMOD_GEOMETRY_TRANSFORMER           12             //几何变换
#define		ALGOMOD_CAMERA_CONTROLLER              13             //相机控制
//#define		ALGOMOD_MOBILE_CAMERA_CONTROLLER       14             //手机相机控制  --不属于
#define  	ALGOMOD_UNIFORM_FIELD_CALIB            15             //均匀场标定
#define   	ALGOMOD_POINT_FINDER                   16             //灯点搜索模块
#define     ALGOMOD_CHESSBOARD_DETECTOR            17             //棋盘格检测
#define    	ALGOMOD_COLORIMETER_CALIB              18             //色度计标定
#define    	ALGOMOD_EDGE_CALIB                     19             //边缘校正
#define     ALGOMOD_CURVE_CALIB                    20             //曲面特征标定修正
#define     ALGOMOD_CAMERA_CHROMA_CALIB            21             //相机色度标定    
#define     ALGOMOD_CAMERA_LUMINANCE_CALIBRATION   22             //相机亮度标定
#define     ALGOMOD_LUM_ACCURACY_OPTIMIZER         23             //光通量PSF
#define     ALGOMOD_POINT_IRREGULAR_MAPPER         24             //异形屏灯点排列模块
#define     ALGOMOD_POING_MAPPER                   25             //点定位匹配
#define     ALGOMOD_SHOT_UTILS                     26             //shot工具
#define     ALGOMOD_POINT_MAPPER                   27             //灯点排列模块
#define     ALGOMOD_MAPPER_UTILS                   28             //mapper结果工具库
#define     ALGOMOD_LED_IRREGULAR_COUTOUR_SEARCHER 29			  //多边形框搜索模块
#define     ALGOMOD_MOBILE_CAMERA_CONTROLLER       30			  //移动端相机调节模块
#define     ALGOMOD_DISTOTTION_CORRECTION          31             //相机畸变标定
#define     ALGOMOD_CMF_CALIB					   32			  //多光谱标定
#define     ALGOMOD_THERMAL_IMAGER				   33			  //温度数据模块
#define		ALGOMOD_SURFACE_TEMPERATURE_CALIB      34             //环境温度补偿 
#define     ALGOMOD_LUMCHROMA_DETECTOR             35             //亮色度检测(自动线检测)
#define     ALGOMOD_STABILITY_MONITOR              36             //稳定性监控模块

#endif
