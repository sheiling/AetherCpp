#ifndef __DRIVER_DEFECT_DETECTOR_ERROE_H__
#define __DRIVER_DEFECT_DETECTOR_ERROE_H__

//basic error---------------------------------------------- 1-99
#include "ci_basic_error.h"
#include "ci_frontend_modname.h"
#include "ci_detection_modname.h"

#define ALGODRIVERDEFECT__ERROR_EMPTY_ROI                 ERROR_CODE(ALGOMOD_DRIVER_DEFECT_DETECTOR,100) //输入的roi句柄为空或输入的roi信息为空；
#define ALGODRIVERDEFECT__ERROR_ROI_OUTOF_IMG             ERROR_CODE(ALGOMOD_DRIVER_DEFECT_DETECTOR,101) //输入的roi超出图像边界，请检查输入的roi信息；
#define ALGODRIVERDEFECT__ERROR_EMPTY_MAP				  ERROR_CODE(ALGOMOD_DRIVER_DEFECT_DETECTOR,102) //输入的mapper句柄为空，或输入的mapper信息为空，请检查输入的map信息中，state/loc是否为空；
#define ALGODRIVERDEFECT__ERROR_EMPTY_IMG				  ERROR_CODE(ALGOMOD_DRIVER_DEFECT_DETECTOR,103) //输入的图像信息为空，请检查输入的shot句柄是否为空，或检查输入的img或shot信息中，image是否为空；
#define ALGODRIVERDEFECT__ERROR_EMPTY_PATTERN			  ERROR_CODE(ALGOMOD_DRIVER_DEFECT_DETECTOR,104) //输入的pattern图像（用于检测驱动IC显示缺陷的特殊pattern）为空；
#define ALGODRIVERDEFECT__ERROR_EMPTY_LUM				  ERROR_CODE(ALGOMOD_DRIVER_DEFECT_DETECTOR,105) //输入的光通量图为空，请见输入的meter句柄是否为空，或输入的meter图像为空；
#define ALGODRIVERDEFECT__ERROR_EMPTY_CONTOUR			  ERROR_CODE(ALGOMOD_DRIVER_DEFECT_DETECTOR,106) //输入的轮廓信息为空，或轮廓句柄为空
#define ALGODRIVERDEFECT__ERROR_EMPTY_FIRSTLINE_IDX		  ERROR_CODE(ALGOMOD_DRIVER_DEFECT_DETECTOR,107) //输入的第一扫索引为空，请检查输入的第一扫索引数量；
#define ALGODRIVERDEFECT__ERROR_FIRSTLINE_IDX			  ERROR_CODE(ALGOMOD_DRIVER_DEFECT_DETECTOR,108) //输入的第一扫索引超出边界，请检查输入的第一扫索引是否存在小于0的值，或第一扫索引是否大于箱体宽度-1或箱体高度-1;例如，箱体宽度为480，当行管垂直方向扫描时，输入第一扫索引大于等于480；
#define ALGODRIVERDEFECT__ERROR_SCAN_DIRECTION			  ERROR_CODE(ALGOMOD_DRIVER_DEFECT_DETECTOR,109) //输入的扫描方向错误，请检查输入的驱动信息LEDDriveInfo中的scan_direction是否等于0或1；或scan_direction与scan_flash_direction不匹配
#define ALGODRIVERDEFECT__ERROR_FLAG					  ERROR_CODE(ALGOMOD_DRIVER_DEFECT_DETECTOR,110) //输入标志位错误，鬼影/耦合检测时，请检查标志位是否为0或1；导出结果图像时，请检查标志位是否为0或1或2；
#define ALGODRIVERDEFECT__ERROR_THREHSOLD				  ERROR_CODE(ALGOMOD_DRIVER_DEFECT_DETECTOR,111) //输入的阈值不在规定范围内，请检查阈值设置是否在合理范围内；
#define ALGODRIVERDEFECT__ERROR_WRITE_IMG                 ERROR_CODE(ALGOMOD_DRIVER_DEFECT_DETECTOR,112) //写入图像失败，存在如下可能的原因：1）输入的存储路径下，无内存空间写入该图像；2)输入存储路径错误或为空，请检查输入路径是否正确；3)输入的原始图像为空，请检查输入图像是否正确；	
#define ALGODRIVERDEFECT__ERROR_EMPTY_RESULT              ERROR_CODE(ALGOMOD_DRIVER_DEFECT_DETECTOR,113) //空的检测结果，请检查是否进行了检测
#define ALGODRIVERDEFECT__ERROR_EMPTY_GHOSTING_BG         ERROR_CODE(ALGOMOD_DRIVER_DEFECT_DETECTOR,114) //空的鬼影背景图，请检查输入的鬼影背景图是否为空
#define ALGODRIVERDEFECT__ERROR_GHOSTING_BG               ERROR_CODE(ALGOMOD_DRIVER_DEFECT_DETECTOR,115) //鬼影背景图与鬼影待检测图尺寸不一致,或鬼影背景图全为0
#define ALGODRIVERDEFECT__ERROR_IMPUT_IMG                 ERROR_CODE(ALGOMOD_DRIVER_DEFECT_DETECTOR,116) //输入图像异常，请检查输入图像数据是否都为0
#define ALGODRIVERDEFECT__ERROR_TRAGETXYZ                 ERROR_CODE(ALGOMOD_DRIVER_DEFECT_DETECTOR,117) //输入的目标XYZ错误，检查XYZ数值
#define ALGODRIVERDEFECT__ERROR_MAX_ITERS                 ERROR_CODE(ALGOMOD_DRIVER_DEFECT_DETECTOR,118) //已达最大迭代次数，请检查输入目标是否可达
#define ALGODRIVERDEFECT__ERROR_COUPLING_REGION           ERROR_CODE(ALGOMOD_DRIVER_DEFECT_DETECTOR,119) //输入的耦合pattern可能存在错误，请检查输入




#endif