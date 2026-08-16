#ifndef __STABILITY_MONITOR_H__
#define __STABILITY_MONITOR_H__

#include "ci_define.h"
#include "ci_declare.h"
#include "stability_monitor_data.h"
#include "stability_monitor_error.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus





	/************************************
	@brief  : 添加监测数据
	@note   : float*
	@param  : monitor_handle		-- stability_monitor句柄
	@param  : in_data				-- 待添加数据。温漂检测时输入当前探头与热像仪采集数据差异 。
	@param  : in_num				-- num
	@return	: AlgoError
	@Author : lvyang@2026/02/26
	************************************/
	C_API AlgoError C_CALL StabilityMonitorAdd(AlgoHandle monitor_handle, const float *in_data, const int in_num);


	/************************************
	@brief  : 添加监测数据
	@note   : float*
	@param  : monitor_handle		-- stability_monitor句柄
	@param  : in_data				-- 待添加数据。输入TriXYZ 。
	@param  : in_num				-- num
	@return	: AlgoError
	@Author : lvyang@2026/02/26
	************************************/
	C_API AlgoError C_CALL StabilityMonitorAddTriXYZ(AlgoHandle monitor_handle, const TriXYZ *in_data, const int in_num);



	/************************************
	@brief  : 添加监测数据
	@note   : float*
	@param  : monitor_handle		-- stability_monitor句柄
	@param  : in_batch_size			-- batch的size，默认一个batch为1000 。
	@return	: AlgoError
	@Author : tianbo@2026/02/26
	************************************/
	C_API AlgoError C_CALL StabilityMonitorSetBatchSize(AlgoHandle monitor_handle, int in_batch_size);

	/************************************
	@brief  : 添加监测数据
	@note   : float*
	@param  : monitor_handle		-- stability_monitor句柄
	@param  : in_max_data_length	-- 最大数据记录长度，默认为100K 。
	@return	: AlgoError
	@Author : tianbo@2026/02/26
	************************************/
	C_API AlgoError C_CALL StabilityMonitorSetMaxDataLength(AlgoHandle monitor_handle, int in_max_data_length);

	/************************************
	@brief  : 获取当前监控状态
	@note   :
	@param  : monitor_handle		-- stability_monitor句柄
	@param  : out_state				-- 输出监控状态，STABILITYMONITORSTATE_OK 为正常. STABILITYMONITORSTATE_NONE表示目前还无法进行结果返回
	@return	: AlgoError
	@Author : lvyang@2026/02/26
	************************************/
	C_API AlgoError C_CALL StabilityMonitorGetState(AlgoHandle monitor_handle, int *out_state);

	/************************************
	@brief  : 基于数据计算阈值
	@note   : 可用于计算温度探头与热像仪数据的机差，该接口
	@param  : monitor_handle		-- stability_monitor句柄
	@param  : in_data				-- 输入温度标定时，探头与热像仪采集数据差异list
	@param  : in_num				-- num，num >= 10
	@param  : in_ratio				-- 温漂阈值ratio. 在输入数据差异的基础上*ratio.结果小于1时取1
	@param  : in_method				-- 计算阈值的方法，默认取最大
	@return	: AlgoError
	@Author : lvyang@2026/02/26
	************************************/
	//C_API AlgoError C_CALL StabilityMonitorCalcThreshByRepeatability(AlgoHandle monitor_handle, const float *in_data, const int in_num,
	//	const float in_ratio, const RepetitionThresCalcMethod in_method = RepetitionThresCalcMethod::REPETITION_THRESH_CALC_METHOD_MAX);

	/************************************
	@brief  : 基于数据计算阈值
	@note   : 用于计算阈值的数据，可直接计算得到阈值。后续每一次add均可进行
	@param  : monitor_handle		-- stability_monitor句柄
	@param  : in_data				-- 输入的数据，每一列为一个样本，每个样本可以是多维度的。
	@param  : in_num				-- num，num >= 10
	@param  : in_ratio				-- 温漂阈值ratio. 在输入数据差异的基础上*ratio.结果小于1时取1
	@param  : in_method				-- 计算阈值的方法，默认取最大
	@return	: AlgoError
	@Author : lvyang@2026/02/26
	************************************/
	C_API AlgoError C_CALL StabilityMonitorCalcThresh(AlgoHandle monitor_handle, AlgoMat in_data);

	/************************************
	@brief  : 基于数据计算阈值
	@note   : 阈值设定
	@param  : monitor_handle		-- stability_monitor句柄
	@param  : in_short_th_param		-- 对每一个数据校验的阈值，通常内部计算为某个基准±in_short_ratio。
	@param  : in_batch_th_param		-- 对每一个batch数据校验的阈值，通常内部计算为某个基准±in_batch_ratio。一般情况下batch的ratio会更小
	@param  : in_ratio				-- 温漂阈值ratio. 在输入数据差异的基础上*ratio.结果小于1时取1
	@param  : in_method				-- 计算阈值的方法，默认取最大
	@return	: AlgoError
	@Author : lvyang@2026/02/26
	************************************/
	C_API AlgoError C_CALL StabilityMonitorSetThreshParam(AlgoHandle monitor_handle, float in_short_th_param, float in_batch_th_param);


	/************************************
	@brief  : 基于数据计算阈值
	@note   : 阈值设定
	@param  : monitor_handle		-- stability_monitor句柄
	@param  : in_method				-- 计算阈值基准的方法。
	@return	: AlgoError
	@Author : lvyang@2026/02/26
	************************************/
	C_API AlgoError C_CALL StabilityMonitorSetThreCalMethold(AlgoHandle monitor_handle, const MonitorThresCalcMethod in_method);


	/*
		1.不同数据维度下的阈值。    float    float[3]
		2.不知道重复性的时候， 可能要基于重复性，以及ratio给个阈值。   类似你这个接口  fanwei    MAX   MAX
		3.基于一个base的偏差，给一个范围。      mean*(1-ratio,1+ratio)
	
	*/


	/**************************************************相关工具*********************************/
	/************************************
	@brief  : 创建monitor_handle句柄
	@note   :
	@param  : out_monitor_handle     -- stability_monitor句柄
	@return	: AlgoError
	@Author : lvyang@2026/02/26
	************************************/
	C_API AlgoError C_CALL StabilityMonitorCreateHandle(AlgoHandle* out_monitor_handle);

	/************************************
	Release  句柄指向的对象的内存空间释放, 句柄指向的对象还在
	Destroy  句柄指向的对象销毁, 此句柄已经无效, 请配合handle=nullptr;

	handle=new OBJ;
	OBJ.Set(a_lot_data);

	Release 相当于:
		OBJ.Release();
	Destroy 相当于:
		OBJ.release();
		delete handle;
	************************************/
	C_API AlgoError C_CALL StabilityMonitorRelease(AlgoHandle monitor_handle);
	C_API void C_CALL StabilityMonitorDestroyHandle(AlgoHandle monitor_handle);


	/************************************
	@brief  : 获取buffer大小
	@note   :
	@param  : monitor_handle       -- stability_monitor句柄
	@param  : out_len                buffer大小
	@return	: AlgoError
	@Author : lvyang@2026/02/26
	************************************/
	C_API AlgoError C_CALL StabilityMonitorGetBufferSize(AlgoHandle monitor_handle, uint64_t* out_len);


	/************************************
	@brief  : 将monitor_handle数据导出到buffer
	@note   :
	@param  : monitor_handle       -- stability_monitor句柄
	@param  : out_buffer          已经申请好申请空间
	@param  : in_out_len          外部申请好的空间大小,会用真实大小修改
	@return	: AlgoError
	@Author : lvyang@2026/02/26
	************************************/
	C_API AlgoError C_CALL StabilityMonitorToBuffer(AlgoHandle monitor_handle, char* out_buffer, uint64_t* in_out_len);


	/************************************
	@brief  : 将buffer数据导入到monitor_handle
	@note   :
	@param  : monitor_handle       -- stability_monitor句柄
	@param  : in_buffer
	@param  : in_len
	@return	: AlgoError
	@Author : lvyang@2026/02/26
	************************************/
	C_API AlgoError C_CALL StabilityMonitorFromBuffer(AlgoHandle monitor_handle, const char* in_buffer, uint64_t in_len);

	/************************************
	@brief  : 读取保存 文件
	@note   : 后缀只能是.
	@param  : monitor_handle		  -- stability_monitor句柄
	@param  : in_monitor_fullname     -- 标定文件,后缀只能是.
	@return	: AlgoError
	@Author : lvyang@2026/02/26
	************************************/
	C_API AlgoError C_CALL StabilityMonitorSave(AlgoHandle monitor_handle, const char* in_monitor_fullname);
	C_API AlgoError C_CALL StabilityMonitorLoad(AlgoHandle monitor_handle, const char* in_monitor_fullname);


#ifdef __cplusplus
}
#endif // __cplusplus
#endif