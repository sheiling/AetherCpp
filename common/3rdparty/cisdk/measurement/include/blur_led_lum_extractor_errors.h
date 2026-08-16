#ifndef __LED_BLUR_LED_LUM_EXTRACTOR_ERRORS_H__
#define __LED_BLUR_LED_LUM_EXTRACTOR_ERRORS_H__

//basic error---------------------------------------------- 1-99
//#include "ci_errors.h"
#include "ci_basic_error.h"
#include "ci_frontend_modname.h"

#define ALGOBLUR__FAILED_FIND_AREA							 ERROR_CODE(ALGOMOD_BLUR_LED_LUM_EXTRACTOR,100) //未找到ROI. 可能时输入的图像无内容
//#define ALGOBLUR__ERROR_CORNERS_NUM_NOT_4_3                  ERROR_CODE(ALGOMOD_BLUR_LED_LUM_EXTRACTOR,101) //角点数量必须是3或者4。 被細化
#define ALGOBLUR__ERROR_CHESS_INFO_MISMATCH_WITH_LED_SIZE    ERROR_CODE(ALGOMOD_BLUR_LED_LUM_EXTRACTOR,102) //棋盘格的参数（个数、尺寸）与屏幕分辨率不相符
//#define ALGOBLUR__ERROR_AREA_VALID_HEIGHT_WIDTH_ERROR        ERROR_CODE(ALGOMOD_BLUR_LED_LUM_EXTRACTOR,103) //区域有效的高或宽异常，最边缘处棋盘格的问题。 被細化
#define ALGOBLUR__ERROR_CORNERS_MISMATCH_TIRANGLE			 ERROR_CODE(ALGOMOD_BLUR_LED_LUM_EXTRACTOR,104) //输入角点数不满足三角区校正条件,角点数小于3。用于三角区校正，角点数应该不小于3
#define ALGOBLUR__ERROR_NO_EACH_ROW_POINT_NUMS				 ERROR_CODE(ALGOMOD_BLUR_LED_LUM_EXTRACTOR,105) //没有每行灯点个数信息。需提前set灯点数据。 用于非大表的球型屏光通量提取。
//#define ALGOBLUR__ERROR_POINTS_DATA_MISMATCHED				 ERROR_CODE(ALGOMOD_BLUR_LED_LUM_EXTRACTOR,106) //输入灯点数据与shot不匹配
#define ALGOBLUR__ERROR_CANNT_FIND_POINTS_ORDER				 ERROR_CODE(ALGOMOD_BLUR_LED_LUM_EXTRACTOR,107) //未发现输入角点的顺序，输入角点差异过大。  三角区校正方式，该错误码未启用。
#define ALGOBLUR__ERROR_DEBLUR_ERROR						 ERROR_CODE(ALGOMOD_BLUR_LED_LUM_EXTRACTOR,108) //deblur数据异常。仅在超分去PSF场景生效。可能是由于输入的NSR小于0，或者单个棋盘格的尺寸小于10，或棋盘格模块中PSF生成失败所导致。
#define ALGOBLUR__ERROR_EDGECORR_PARAM_ERROR				 ERROR_CODE(ALGOMOD_BLUR_LED_LUM_EXTRACTOR,109) //基于大表的球形屏边缘修正输入的入参无效。   这种情况不会出现，属于过度判断。
#define ALGOBLUR__ERROR_TOP_BOT_NUMS_MISMATCH				 ERROR_CODE(ALGOMOD_BLUR_LED_LUM_EXTRACTOR,110) //设置的首行/尾行灯点个数大于屏幕分辨率。非大表的光通量提取中会使用。
#define ALGOBLUR__ERROR_INPUT_POINT_NUMS_MISMATCH			 ERROR_CODE(ALGOMOD_BLUR_LED_LUM_EXTRACTOR,111) //输入的角点个数信息不符合预期。 一般用于非棋盘格的超分成通量提取，角点数必须是3或4.   3为三角区的光通量提取-暂未进行应用。
#define ALGOBLUR__ERROR_COL_INVALID				 			 ERROR_CODE(ALGOMOD_BLUR_LED_LUM_EXTRACTOR,112) //待校正区域识别结果的列项有效高度小于特定有效的值，导致该位置无法提取有效果的光通量。主要用于非棋盘格的模糊光通量提取。
#define ALGOBLUR__ERROR_CHESS_ROW_COL_VALID_NUMS_ERROR		 ERROR_CODE(ALGOMOD_BLUR_LED_LUM_EXTRACTOR,113) //某个棋盘格区域识别结果的有效行/列数量为0（真实高度/宽度不小于当前棋盘格高度/宽度的0.75倍即为有效高度），导致该位置无法提取有效果的光通量。可能是由于边缘棋盘格成像畸形，如该棋盘格内大部分的宽度/高度远远小于整体的宽度/高度时可能会出现。  基本不会出现该问题。 可通过将成像调锐减小这种情况的出现。
#define ALGOBLUR__ERROR_SOME_CHESS_COL_INVALID		     	 ERROR_CODE(ALGOMOD_BLUR_LED_LUM_EXTRACTOR,114) //某个棋盘格区域识别结果的当前行/列无效（真实高度/宽度的长度为0时认为无效），导致该位置无法提取有效果的光通量。 由于有前处理，正常不会出现该问题。主要用于在球型屏提取边缘处棋盘格位置对应的的光通量
#define ALGOBLUR__ERROR_CHESS_HANDLE_MISMATCH				 ERROR_CODE(ALGOMOD_BLUR_LED_LUM_EXTRACTOR,115) //输入的棋盘格handle类型不匹配。eg：使用常规chess用于球型屏的光通量提取。


#endif