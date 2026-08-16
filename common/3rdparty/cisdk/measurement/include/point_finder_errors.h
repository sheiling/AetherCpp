#ifndef __POINT_FINDER_ERRORS_H__
#define __POINT_FINDER_ERRORS_H__

#include "ci_basic_error.h"
#include "ci_frontend_modname.h"

#define ALGOPOINTFIND__ERROR_THREDNUM					ERROR_CODE(ALGOMOD_POINT_FINDER,100) //设置并行线程数错误，要求输入的线程数 > 1；该错误码出现在接口PointFindSetDefaultThreadsNum，请检查入参范围是否正确
#define ALGOPOINTFIND__ERROR_MATCH_FLAG					ERROR_CODE(ALGOMOD_POINT_FINDER,101) //设置匹配方法错误，要求输入的匹配方法为0，1，2，3，4，5，6，前5个匹配核分别对应opencv模板匹配0-5匹配核，6对应原有的模板匹配算法；该错误码出现在接口PointFindSetDefaultMatchFlag，请检查入参范围是否正确
#define ALGOPOINTFIND__ERROR_DOWNSAMPLE					ERROR_CODE(ALGOMOD_POINT_FINDER,102) //设置下采样率错误，要求输入的下采样率范围为 0.05f - 1.0f；该错误码出现在接口PointFindSetDefaultDownsampling，请检查入参范围是否正确
#define ALGOPOINTFIND__ERROR_SUBPIXEL_FLAG				ERROR_CODE(ALGOMOD_POINT_FINDER,103) //设置亚像素定位flg错误，要求输入的亚像素flg为0，1；0表示不使用亚像素计算，1表示进行亚像素计算；该错误码出现在接口PointFindSetDefaultSubpixelFlag，请检查入参范围是否正确
#define ALGOPOINTFIND__ERROR_METHOD						ERROR_CODE(ALGOMOD_POINT_FINDER,104) //设置点定位方法错误，要求输入的点定位方法为0，1；0表示模板匹配算法，1表示直接灰度分割算法；该错误码出现在接口PointFindSetDefaultMethod，请检查入参范围是否正确
#define ALGOPOINTFIND__ERROR_TEMPLATE_WH				ERROR_CODE(ALGOMOD_POINT_FINDER,105) //设置模板错误, 要求输入的模板行列数均 > 3；该错误码出现在接口PointFindSetTemplateWh，请检查入参范围是否正确
#define ALGOPOINTFIND__ERROR_TEMPLATE_EMPTY				ERROR_CODE(ALGOMOD_POINT_FINDER,106) //设置模板错误, 输入的模板mat是一个空图像；该错误码出现在接口PointFindSetTemplate，请检查输入的模板图像是否为空
#define ALGOPOINTFIND__ERROR_INPUT_EMPTY				ERROR_CODE(ALGOMOD_POINT_FINDER,107) //输入图像为空，可能是输入的img/shot/roi/shot.pattern/roi.xywh/shot.pattern.lighted_led_wh/mask为空；该错误码主要是由于各接口输入(.amat .shot .roi)等为空，请根据调用的接口逐一检查；对于接口PointFindLocate，则需要额外检查shot->panel->wh与roi->xywh是否为空
#define ALGOPOINTFIND__ERROR_ROI_EXCEED					ERROR_CODE(ALGOMOD_POINT_FINDER,108) //输入Roi区域错误，要求输入的roi区域不能超过输入的图像尺寸；该错误码出现在接口PointFindLocate、PointFindLocateInRect、PointFindLocateInMask，请检查输入的roi是否超出图像边界
#define ALGOPOINTFIND__ERROR_WRITE_IMG					ERROR_CODE(ALGOMOD_POINT_FINDER,109) //写入图像失败，要求存储路径内存充足；该错误码出现在接口PointFindExportLocImg、PointFindExportSubpixelLocImg，请检查存储路径所在磁盘是否内存充足
#define ALGOPOINTFIND__ERROR_MASK_FAILED_MATCH_IMG		ERROR_CODE(ALGOMOD_POINT_FINDER,110) //输入Mask与输入图像的尺寸不匹配，要求输入的mask与输入图像的尺寸保持一致；该错误码出现在接口PointFindLocateInMask，请检查输入的mask的尺寸是否一致
#define ALGOPOINTFIND__ERROR_MASK_NOCONTOUR_POINT		ERROR_CODE(ALGOMOD_POINT_FINDER,111) //使用Mask匹配时，Mask中所有位置全为0，要求输入的mask中不能全为0；该错误码出现在接口PointFindLocateInMask，请检查输入的mask数据是否为全0
#define ALGOPOINTFIND__ERROR_TEMPLATEPARAM				ERROR_CODE(ALGOMOD_POINT_FINDER,112) //设置模板参数范围错误，详见point_finder_errors.h中的TemplateParam结构体数值范围设定
#define ALGOPOINTFIND__ERROR_POSTPROCPARAM				ERROR_CODE(ALGOMOD_POINT_FINDER,113) //设置后处理参数范围错误，详见point_finder_errors.h中的PostProcParam结构体数值范围设定

#endif
