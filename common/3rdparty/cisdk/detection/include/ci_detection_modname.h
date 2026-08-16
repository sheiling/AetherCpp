#ifndef __CI_DETECTION_MODNAME_H__
#define __CI_DETECTION_MODNAME_H__

//检测模块：模块编号范围应在128~511之间，不能超出该范围
#define		ALGOMOD_LED_METER_INSPECTOR                 128             //灯点点亮缺陷检测
#define		ALGOMOD_MURA_DETECTOR                       129             //mura检测
#define		ALGOMOD_FLASH_DETECTOR                      130             //闪屏闪点检测
#define		ALGOMOD_MURA_SALTPEPPER_INSPECTOR           131             //椒盐mura检测
#define		ALGOMOD_INKCOLOR_CLASSIFIER                 132             //墨色分选
#define		ALGOMOD_PCB_INKCOLOR_CLASSIFIER             133             //pcb墨色分选
#define		ALGOMOD_LUMCHROMA_CLASSIFIER                134             //亮色度分选
#define		ALGOMOD_LED_INSPECTOR                       135             //灯点外观检测
#define		ALGOMOD_MODEL_PROCESSOR                     136             //model处理模块
#define		ALGOMOD_UNIFORMITY_DETECTOR                 137             //均匀性检测
#define		ALGOMOD_CHIP_DETECTOR                       138             //灯点芯片检测
#define		ALGOMOD_SOLDER_DETECTOR                     139             //焊锡检测
#define		ALGOMOD_DIE_DETECTOR                        140             //wafer晶片检测
#define		ALGOMOD_MARKER_DETECTOR                     141             //marker点检测
#define  	ALGOMOD_CAMERA_ADJUSTMENT                   142             //设备相机调节
#define   	ALGOMOD_CAMERA_CHROMA_CALIBRATION           143             //设备相机原位色度标定
#define     ALGOMOD_CAMERA_DISTORTION_CALIBRATION       144             //设备相机原位畸变标定
#define    	ALGOMOD_CAMERA_VIGNETTING_CALIBRATION       145             //设备相机原位渐晕标定
#define    	ALGOMOD_SMD_DETECTOR                        146             //SMD直线度检测
#define     ALGOMOD_ARROW_DETECTOR                      147             //箭头检测
#define     ALGOMOD_METAL_NUT_DETECTOR                  148             //螺母检测
#define     ALGOMOD_PCB_SOCKET_LOCATEOR                 149             //pcb对插孔定位
#define     ALGOMOD_WAFER_CELL_PAD_LOCATEOR             150             //microLED cell检测
#define     ALGOMOD_MICRO_DETECTOR                      151             //microLED 缺陷检测
#define     ALGOMOD_WAFER_ANGLE_DETECTOR                152             //wafer角度检测
#define     ALGOMOD_UNFORMITY_MEASURER				    153             //均匀性检测,仪器端
#define	    ALGOMOD_PAD_FINE_LOCATOR                    154             //pad精定位
#define	    ALGOMOD_DIE_BONDING_DEFECT_LOCATOR          155             //固晶缺陷检测
#define	    ALGOMOD_WAFER_IMAGE_STITCHER                156             //WAFER图像融合拼接
#define	    ALGOMOD_GOLDEN_SAMPLE_GENERATOR             157             //最优模板生成
#define	    ALGOMOD_LED_DIE_MAP_FUSION                  158             //AOI检测结果融合和MAP图生成
#define	    ALGOMOD_CAMERA_SPECKLE_DETECTOR             159             //相机亮暗斑检测
#define	    ALGOMOD_PCB_STENCIL_MATCHER                 160             //PCB钢网匹配
#define	    ALGOMOD_CAMERA_LENS_CONTROLLER              161             //相机镜头控制
#define	    ALGOMOD_CABINET_EFFECT_DETECTOR             162             //箱体效果检测
#define	    ALGOMOD_DL_CHIP_DETECTOR				    163             //深度学习芯片焊盘检测
#define	    ALGOMOD_DL_SOLDER_DETECTOR				    164             //深度学习焊盘检测
#define	    ALGOMOD_DL_MIDDLE_TRANS_DIE_DETECTOR	    165             //深度学习中转位检测
#define	    ALGOMOD_DL_WAFER_DIE_DETECTOR			    166             //深度学习wafer位检测
#define	    ALGOMOD_DL_DIE_BONDING_DETECTOR             167             //深度学习固晶检测
#define	    ALGOMOD_DL_PCB_COMPONENT_CHECKER            168             //深度学习PCB元件检查
#define	    ALGOMOD_HALO_DETECTOR                       169             //背光光型检测
#define	    ALGOMOD_BLUEFILM_DEFECT_DETECTOR            170             //蓝膜脏污检测
#define	    ALGOMOD_DIE_DEFECT_DETECTOR                 171             //晶圆外观缺陷检测
#define	    ALGOMOD_DRIVER_DEFECT_DETECTOR			    172             //驱动IC显示效果检测
#define	    ALGOMOD_DL_CABINET_DEFECT_DETECTOR          173             //深度学习BOE箱体外观检测
#define     ALGOMOD_SUBPIXEL_DISPLAY_EVALUATOR          174             //子像素显示效果测评
#define     ALGOMOD_INKCOLOR_CALIBRATION                175             //墨色标定
#define     ALGOMOD_WAFER_DEFECT_DETECTOR               176             //晶圆缺陷检测
#define     ALGOMOD_SPATIAL_FUSION_CELL_LOCATOR         177             //pad信息整合保存
//#define     ALGOMOD_CURVE_FITTING                       178             //曲面拟合
#define     ALGOMOD_DEMURA_LAYER_SELECTOR               179             //校正灰阶选择
#define     ALGOMOD_LED_ILLUMINATION_DETECTOR           180             //点亮检测
#define     ALGOMOD_FORWARD_INFERENCE                   181             //基础推理模块
#define     ALGOMOD_COLORED_LED							182				//全彩化智能检测模块
#define     ALGOMOD_PRE_DEMURA_EVALUATOR                183             //校正准入优化
#define     ALGOMOD_THERMAL_MODEL_EMULATOR              184             //热力特性评估
#define     ALGOMOD_VEIWING_ANGLE_MODEL_EMULATOR        185             //视角特性测量和评估
#define     ALGOMOD_DISPLAY_UNIFORMITY_SIMULATOR        186             //校正后效果预览
//#define     ALGOMOD_LUMCHROMA_DETECTOR                  187             //亮色度检测(自动线检测2.0)
#define     ALGOMOD_DISPLAY_UNIFORMITY_EVALUATOR        188             //显示屏均匀性评价
#define     ALGOMOD_SHARPNESS_AUTOFOCUS                 189             //基于清晰度评价的自动对焦
#define 	ALGOMOD_LOWGRAY_DISPLAY_TUNING				190				//基于驱动IC的智能调参
#define 	ALGOMOD_CALIBRATION_SCHEME_RECOMMENDER		191				//校正方案推荐
#define     ALGOMOD_DISPLAY_PERFORMANCE_EVALUATOR       192             //显示量化库
#define     ALGOMOD_CABINET_CONSTRUCTOR				    193             //全自动箱体构造

#endif
