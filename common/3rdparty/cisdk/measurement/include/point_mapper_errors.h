#ifndef __POINT_MAPPER_ERRORS_H__
#define __POINT_MAPPER_ERRORS_H__

//#include "ci_errors.h"
#include "ci_basic_error.h"
#include "ci_frontend_modname.h"


#define ALGOPOINTMAP__ERROR_WRONG_THREAD_NUMS         ERROR_CODE(ALGOMOD_POINT_MAPPER,100)  //输入的CPU并行线程数错误，请检查是否输入的值小于1或设置的并行线程数超过了CPU最大线程数
#define ALGOPOINTMAP__ERROR_INPUT_TYPE_MISMATCH       ERROR_CODE(ALGOMOD_POINT_MAPPER,101)  //输入的点集type不符合要求，1)调用PointMapSetInputFromMat接口设置输入时，要求AlgoMat类型必须为ALGO_32FC2；2)调用PointMapSet接口时，要求输入参数PointMapResult.state的类型必须为ALGO_8UC1，PointMapResult.loc的类型必须为ALGO_32FC2;
#define ALGOPOINTMAP__ERROR_NO_SEED_POINT             ERROR_CODE(ALGOMOD_POINT_MAPPER,102)  //初始化种子点失败,存在如下可能的原因：1)输入了错误的灯点排布形式（如本应该为矩形排列，输入为了三角排列），请见检查灯点排布形式是否与输入保持一致；2)输入了错误的灯点数，请检查灯点数是否输入正确，或检查输入的灯点数与灯点排布形式是否相互对应；3)输入的点定灯坐标（point_finder输入）混乱（不成规律分布或干扰点过多），请检查输入的灯点坐标（point_finder输入）是否正确；
#define ALGOPOINTMAP__ERROR_NO_BASE_ROW               ERROR_CODE(ALGOMOD_POINT_MAPPER,103)  //寻找初始行失败（基本不会触发该错误），存在如下可能的原因： //1)寻找到的初始行的灯点数量，小于规定点数；																					
#define ALGOPOINTMAP__ERROR_TARGET_WH                 ERROR_CODE(ALGOMOD_POINT_MAPPER,104)  //输入的灯点行列数错误,存在如下可能的原因：1)请检查，是否输入的行列数为负数；2)请检查，是否在LedLayout==LED_LAYOUT_TRIANGLE_ROW时，输入的灯点列数为奇数;3)请检查，是否在LedLayout==LED_LAYOUT_TRIANGLE_COL时，输入的灯点行数为奇数;4)请检查，是否灯板存在较大程度的错行/错列现象（灯点错行/错列位置大于等于点间距一半）;
#define ALGOPOINTMAP__ERROR_MAP_SIZE_NOT_MATCH        ERROR_CODE(ALGOMOD_POINT_MAPPER,105)  //找到的行列数与输入行列数不匹配，存在如下可能的原因：1)输入的行列数错误，请检查输入灯点行列数是否正确且与排布形式相互对应；2)灯板/箱体/全屏显示异常，请检查，是否存在异常亮起的行或列，或首行/首列、末尾行/末尾列是否全部为死灯；
#define ALGOPOINTMAP__ERROR_MISSING_TOO_MATCH_POINTS  ERROR_CODE(ALGOMOD_POINT_MAPPER,106)  //灯点存在大面积缺失，存在如下可能原因：//1)灯板/箱体/全屏显示异常，请检查灯板/箱体/全屏是否出现了大面积连续死灯； //2)上游点定位point_finder模块送入的点存在异常，请检查point_finder模块的输出结果；																					 																																				
#define ALGOPOINTMAP__ERROR_NO_CROSS_POINTS           ERROR_CODE(ALGOMOD_POINT_MAPPER,107)  //补全死灯时无法根据曲线计算得到死灯点坐标（基本不会触发该错误），存在如下可能原因：1）该死灯点所在的行列所对应的曲线方程，不存在交叉点；
#define ALGOPOINTMAP__ERROR_EMPTY_INPUT_IMG           ERROR_CODE(ALGOMOD_POINT_MAPPER,108)  //输入的图像为空
#define ALGOPOINTMAP__ERROR_WRITE_IMG                 ERROR_CODE(ALGOMOD_POINT_MAPPER,109)  //写入图像失败，存在如下可能的原因：1）输入的存储路径下，无内存空间写入该图像；2)输入存储路径错误或为空，请检查输入路径是否正确；3)输入的原始图像为空，请检查输入图像是否正确；	
#define ALGOPOINTMAP__ERROR_EMPTY_MAP_RESULT          ERROR_CODE(ALGOMOD_POINT_MAPPER,110)  //本handle内的map信息为空，请检查输入的map信息，可能存在如下两种情况：1)map的handle的指针为空；2)map内的结果为空(point_loc、point_state)；																			
#define ALGOPOINTMAP__ERROR_EMPTY_4CORNER_POINTS      ERROR_CODE(ALGOMOD_POINT_MAPPER,111)  //使用标准灯点定位结果生成一个新的定位结果时，输入的4个角点附近的灯点为空																	
#define ALGOPOINTMAP__ERROR_CORNER_NO_VALID_POINT     ERROR_CODE(ALGOMOD_POINT_MAPPER,112)  //四角点匹配求loc时，有效四角点的角点数量小于1。角点死灯过多或灯板偏移过大 
#define ALGOPOINTMAP__ERROR_MAP_METHOD_MISMATCH       ERROR_CODE(ALGOMOD_POINT_MAPPER,113)  //排列方法不匹配 
#define ALGOPOINTMAP__ERROR_DEAD_POINT_OVERSIZE       ERROR_CODE(ALGOMOD_POINT_MAPPER,114)	//死灯预测错误，越界
#define ALGOPOINTMAP__ERROR_INPUT_POINTS_SIZE         ERROR_CODE(ALGOMOD_POINT_MAPPER,115)	//输入的灯点数量大于给定灯点宽*高的25%或低于给定灯点宽*高的25%。请检查输入灯点数是否正确或PointFind结果是否正确


#endif