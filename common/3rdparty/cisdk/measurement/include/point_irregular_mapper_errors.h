#ifndef __point_irregular_mapper_ERRORS_H__
#define __point_irregular_mapper_ERRORS_H__

//basic error---------------------------------------------- 1-99
//#include "ci_errors.h"
#include "ci_basic_error.h"
#include "ci_frontend_modname.h"

#define ALGOPOINTIRREGULARMAPPER__ERROR_FIND_POINT_NUM						ERROR_CODE(ALGOMOD_POINT_IRREGULAR_MAPPER,100)	// 该错误码出现在Compute计算裁切屏排列时，如果排列找到的灯点数量与finder输入不一样，存在漏灯或重复的情况会报错；
#define ALGOPOINTIRREGULARMAPPER__ERROR_POINT2I_TO_POINT2F_ERROR			ERROR_CODE(ALGOMOD_POINT_IRREGULAR_MAPPER,101)	// 该错误码出现在Compute接口中，先按照整像素去排列，当未找到对应的亚像素中心的时候会报错；
#define ALGOPOINTIRREGULARMAPPER__ERROR_IRREGULAR_TYPE						ERROR_CODE(ALGOMOD_POINT_IRREGULAR_MAPPER,102)	// IrregularType错误，该错误码出现在SetIrregularType接口，请检查设置的IrregularType是否符合include/data.h规范
#define ALGOPOINTIRREGULARMAPPER__ERROR_PATTERN_TYPE                        ERROR_CODE(ALGOMOD_POINT_IRREGULAR_MAPPER,103)	// 该错误码出现于SetAuxiliaryMapper接口，含义是辅助pattern类型错误,当前仅有辅助行与辅助列两类；请检查入参
#define ALGOPOINTIRREGULARMAPPER__ERROR_AUXILIARY_PATTERN_EMPTY             ERROR_CODE(ALGOMOD_POINT_IRREGULAR_MAPPER,104)	// 用于mapper的辅助灯点坐标不存在；该错误码出现于Compute接口CheckAuxiliaryPattern函数，可能是没有正确使用SetPatternPoints接口，请检查。
																															// 如果出现在MapperAuxiliaryPattern函数中，应该是辅助图像的finder结果有问题导致check后为空；
#define ALGOPOINTIRREGULARMAPPER__ERROR_PARTITION_SHAPE                     ERROR_CODE(ALGOMOD_POINT_IRREGULAR_MAPPER,105)	// 屏体形状错误；该错误码出现于SetPartitionShape接口，请检查入参是否是球身/球顶中的一种
#define ALGOPOINTIRREGULARMAPPER__ERROR_COMMEN_POINT_DEAD                   ERROR_CODE(ALGOMOD_POINT_IRREGULAR_MAPPER,106)	// 辅助行与辅助列交点为死灯且算法无法补全，超过算法边界，请微调分区位置规避或者手动补全死灯后重新排列；
#define ALGOPOINTIRREGULARMAPPER__ERROR_TOO_MANY_DEAD_POINT                 ERROR_CODE(ALGOMOD_POINT_IRREGULAR_MAPPER,107)	// 通常出现在折线排列场景，死灯过多或者位置较为特殊，超出算法边界，请微调分区位置规避或者手动补全死灯后重新排列；
#define ALGOPOINTIRREGULARMAPPER__ERROR_DEAD_POINT_ERROR                    ERROR_CODE(ALGOMOD_POINT_IRREGULAR_MAPPER,108)	// 死灯预测错误
#define ALGOPOINTIRREGULARMAPPER__ERROR_ROW_SEARCH_VECTOR                   ERROR_CODE(ALGOMOD_POINT_IRREGULAR_MAPPER,109)	// 该错误码出现在POINTROWMAPPER功能的逐行排列环节，行向灯点搜索向量计算错误，可能是最左列灯点距离过大/该行死灯情况特殊，超算法边界



#endif