#ifndef __LOWGRAY_DISPLAY_TUNING_H__
#define __LOWGRAY_DISPLAY_TUNING_H__

#include "algo_mat.h"
#include "ci_data.h"
#include "ci_declare.h"
#include "meta_info_data.h"
#include "lowgray_display_tuning_data.h"
#include "lowgray_display_tuning_error.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

	/*********************************************
	@brief   : 创建LowgrayDisplayTuning(低灰参数调节)句柄
	@note    :
	@param   : lowgrayTuning_handle
	@return  : AlgoError
	@Author  : chenpengfei@2025/12/11
	*********************************************/
	C_API AlgoError C_CALL LowgrayTuningCreateHandle(AlgoHandle* lowgrayTuning_handle);


	/*********************************************
	@brief   : 当前待调试模组ic参数调节参数配置
	@note    :
	@param   : lowgrayTuning_handle
	@param   : pureLamda_in：低灰纯色评价权重
	@param   : regVec_in：输入待调节寄存器配置
	@param   : boxInfo_in：输入待调节模组信息配置
	@param   : patternInfo_in：测试pattern信息配置
	@param	 : initParam：调节初始寄存器参数设置
	@param	 : regNum：待调节的寄存器数目
	@return  : AlgoError
	@Author  : chenpengfei@2025/12/11
	*********************************************/
	C_API AlgoError C_CALL LowgrayTuningParamSet(AlgoHandle lowgrayTuning_handle, 
		const double pureLamda_in, Register* regVec_in,
		IC_BoxInfo boxInfo_in, PatternInfo patternInfo_in, int* initParam,
		const int regNum);


	/*********************************************
	@brief   : 通过生成的param，采集meter图像，更新参数簇
	@note    :
	@param   : lowgrayTuning_handle
	@param   : regVec_in：输入当前寄存器向量
	@param   : pureMeter_in：输入低灰纯色光通量图像
	@param   : grayScaleMeter_in：输入灰阶过渡光通量图像
	@param   : regNum：待调节寄存器的数量
	@return  : AlgoError
	@Author  : chenpengfei@2025/12/11
	*********************************************/
	C_API AlgoError C_CALL LowgrayTuningParamRefresh(AlgoHandle lowgrayTuning_handle,
		int* regVec_in, int regNum, AlgoHandle pureMeter_in, AlgoHandle grayScaleMeter_in);


	/*********************************************
	@brief   : 利用LHS准则，生成初始化调节向量集合
	@note    :
	@param   : lowgrayTuning_handle
	@param   : lhsParamInit:	输出的LHS初始化的寄存器向量
	@return  : AlgoError
	@Author  : chenpengfei@2025/12/11
	*********************************************/
	C_API AlgoError C_CALL LowgrayTuningLhsParamInit(AlgoHandle lowgrayTuning_handle, 
		int lhsParamInit[][MAX_REG_NUM], int* lhsOutNum);


	/*********************************************
	@brief   : 利用Bayesian寻优策略，找寻下一个寻优点
	@note    :
	@param   : lowgrayTuning_handle
	@param   : bayesianParam_out:	输出的经过Bayesian寻优策略确定的下一个目标点
	@param	 : decisionVar_out:  当前寻优向量对应的决策树方差
	@return  : AlgoError
	@Author  : chenpengfei@2025/12/11
	*********************************************/
	C_API AlgoError C_CALL LowgrayTuningBayesianParamGenerate(AlgoHandle lowgrayTuning_handle,
		int* bayesianParam_out, double* decisionVar_out);


	/*********************************************
	@brief   : 利用当前的采集结果，对pure和grayScale双指标进行联合评价
	@note    :
	@param   : lowgrayTuning_handle
	@param   : outResult:	输出的最终评价结果，包含(参数Vec：评价误差)
	@return  : AlgoError
	@Author  : chenpengfei@2025/12/11
	*********************************************/
	C_API AlgoError C_CALL LowgrayTuningCalUnionOptScore(AlgoHandle lowgrayTuning_handle,
		ICResult* outResult);


	/*********************************************
	@brief   : 利用当前的采集结果，对pure和grayScale双指标进行联合评价
	@note    :
	@param   : lowgrayTuning_handle
	@param   : param_in:	输入的寄存器参数向量
	@return  : True：参数重复；False：参数不重复
	@Author  : chenpengfei@2025/12/11
	*********************************************/
	C_API AlgoError C_CALL LowgrayTuningRepetition(AlgoHandle lowgrayTuning_handle,
		int* param_in, int regNum, bool* repeatOut);

	/*********************************************
	@brief   : 利用当前的采集结果，对输入的单色图像进行均匀性的评价
	@note    :
	@param	 : meterHandle：经过曲面剔除后的光通量图像
	@param	 : boxInfo：待验证箱体的信息
	@param	 : patternInfo：打屏pattern的相关信息
	@return  : pureScore：基于均匀性图像的评价打分
	@Author  : chenpengfei@2025/12/11
	*********************************************/
	C_API AlgoError C_CALL LowgrayTuningPureScore(AlgoHandle lowgrayTuning_handle,
		AlgoHandle meterHandle, IC_BoxInfo boxInfo, PatternInfo patternInfo, double* pureScore);


	/*********************************************
	@brief   : 利用当前的采集结果，对灰阶过渡图像进行均匀性评价
	@note    :
	@param	 : meterHandle：经过曲面剔除后的光通量图像
	@param	 : boxInfo：待验证箱体的信息
	@param	 : patternInfo：打屏pattern的相关信息
	@return  : grayscaleScore：基于灰阶过渡图像进行均匀性打分
	@Author  : chenpengfei@2025/12/11
	*********************************************/
	C_API AlgoError C_CALL LowgrayTuningGrayScaleScore(AlgoHandle lowgrayTuning_handle,
		AlgoHandle meterHandle, IC_BoxInfo boxInfo, PatternInfo patternInfo, double* grayscaleScore);


	/*********************************************
	@brief   : 销毁句柄
	@note    :
	@param	 : lowgrayTuning_handle：待销毁的句柄
	@Author  : chenpengfei@2026/02/04
	*********************************************/
	C_API void C_CALL LowgrayTuningDestroyHandle(AlgoHandle lowgrayTuning_handle);

	//测试数据
	C_API AlgoError C_CALL LowgrayTuningItegrateName(AlgoHandle lowgrayTuning_handle,
		int* param, int regNum, std::string prefix, std::string pathDir, AlgoHandle meterHandle);


#ifdef __cplusplus
}
#endif
#endif // !__LOWGRAY_DISPLAY_TUNING_H__
