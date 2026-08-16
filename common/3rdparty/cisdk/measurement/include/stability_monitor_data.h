#ifndef __STABILITY_MONITOR_DATA_H__
#define __STABILITY_MONITOR_DATA_H__

#include "ci_data.h"
#include "meta_info_data.h"
#include "algo_mat_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	//稳定性监控状态
	enum StabilityMonitorState {
		STABILITYMONITORSTATE_OK = 0x00,
		STABILITYMONITORSTATE_NONE = 0x01,
		STABILITYMONITORSTATE_SHORT_TREM_ABONORMAL = 0x02,		//短期异常
		STABILITYMONITORSTATE_LONG_TREM_ABONORMAL = 0x04,		//长期异常

	};

	// 计算重复性阈值方法
	enum MonitorThresCalcMethod {
		REPETITION_THRESH_CALC_METHOD_AVERAGE_BY_RATIO = 0,
		REPETITION_THRESH_CALC_METHOD_AVERAGE_BY_OFFSET = 1,
		REPETITION_THRESH_CALC_METHOD_MIN_MAX_BY_RATIO = 2,
		REPETITION_THRESH_CALC_METHOD_MIN_MAX_BY_OFFSET = 3,
	};
	

#ifdef __cplusplus
}
#endif // __cplusplus
#endif