#ifndef __CAMERA_CMF_CALIB_H__
#define __CAMERA_CMF_CALIB_H__

#include "ci_define.h"
#include "ci_declare.h"
#include "camera_cmf_calib_data.h"
#include "camera_cmf_calib_error.h"

#ifdef __cplusplus
extern "C" 
{
#endif // __cplusplus  

	/************************************
	@brief  : 创建相机标定句柄
	@note   :
	@param  : out_cmfcalib_handle       CMF标定句柄
	@return	: AlgoError
	@Author : yuxingchao@2025/03/20
	************************************/
	C_API AlgoError C_CALL CmfCalibCreateHandle(AlgoHandle *out_cmfcalib_handle);


	/************************************
	Destroy  句柄指向的对象销毁

	handle=new OBJ;
	OBJ.Set(a_lot_data);

	Destroy 相当于:
		OBJ.release();
		delete handle;
	************************************/
	C_API void C_CALL CmfCalibDestroyHandle(AlgoHandle cmfcalib_handle);

	
	///************************************
	//@brief  : 设置CMF标定拍摄所使用的单色仪波长序列
	//@note   : 不设置时的默认序列：380nm:5nm步进:720nm
	//@param  : cmfcalib_handle         有效的CMF标定句柄
	//@param  : shot_wavelength         CMF标定拍摄所使用的单色仪波长序列
	//@param  : wave_num                CMF标定拍摄所使用的单色仪波长序列的波长个数
	//@return : AlgoError
	//@Author : yuxingchao@2025/04/09
	//************************************/
	C_API AlgoError C_CALL CmfCalibSetShotWavelength(AlgoHandle cmfcalib_handle, float* shot_wavelength, int wave_num);


	///************************************
	//@brief  : 设定CMF标定拍摄raw图所需求得的中央均值的ROI区域大小
	//@note   : 不设置时的默认Size：128 x 128 (1040*706)
	//@param  : cmfcalib_handle         有效的CMF标定句柄
	//@param  : roi_size			    标定拍摄raw图所需求得的中央均值的ROI区域大小：width，height
	//@return : AlgoError
	//@Author : yuxingchao@2025/04/09
	//************************************/
	C_API AlgoError C_CALL CmfCalibSetCalibRoiSize(AlgoHandle cmfcalib_handle, const AlgoSize roi_size);


	///************************************
	//@brief  : 传递每一帧标定拍摄得到的有效shot
	//@note   : 传入的每帧shot务必已经完成黑帧扣除，且在同一标定波长下已经完成多帧（排除波长切换时拍摄的无效帧）的均值计算，需保留正确的cie_type信息
	//@param  : cmfcalib_handle         有效的CMF标定句柄
	//@param  : dark_removed_mean_shot  黑帧扣除后的、在同一标定波长下多次拍摄得到的有效shot数据的shot均值   
	//@param  : wave_idx                标定拍摄所处的波长序号（按短波端起始波长从0开始计）
	//@param  : round_idx		        标定拍摄所处的拍摄轮次（一共两轮：0/1）
	//@return : AlgoError
	//@Author : yuxingchao@2025/04/09
	//************************************/
	C_API AlgoError C_CALL CmfCalibSetCalibShotData(AlgoHandle cmfcalib_handle, AlgoHandle dark_removed_mean_shot, const int wave_idx, const int round_idx);


	///************************************
	//@brief  : 设置CMF标定结果所使用的输出波长序列         
	//@note   : 不设置时的默认序列：380nm:1nm步进:720nm
	//@param  : cmfcalib_handle         有效的CMF标定句柄
	//@param  : target_wavelength       CMF标定结果所使用的输出波长序列    
	//@param  : wave_num                CMF标定结果所使用的输出波长序列的波长个数
	//@return : AlgoError
	//@Author : yuxingchao@2025/04/09
	//************************************/
	C_API AlgoError C_CALL CmfCalibSetTargetWavelength(AlgoHandle cmfcalib_handle, float* target_wavelength, int wave_num);


	///************************************
	//@brief  : 设置标定文件的参数结构体 // 需要调整 c data 单独定义
	//@note   : 参数结构体包含了（每项参数可以单独设置，不设置将采用默认值）：
	// wid_range_r（红色半波宽度）默认值：3.0f:1.0f步进:30.0f;
	// cur_range_r（红色曲率系数）默认值：50.0f;
	// wav_range_r（红色波长范围）默认值：610.0f:1.0f步进:640.0f;
	// wid_range_g（绿色半波宽度）默认值：15.0f:1.0f步进:34.0f;
	// cur_range_g（绿色曲率系数）默认值：3.0f:2.0f步进:9.0f;
	// wav_range_g（绿色波长范围）默认值：500.0f:1.0f步进:560.0f;
	// wid_range_b（蓝色半波宽度）默认值：8.0f:1.0f步进:27.0f;
	// cur_range_b（蓝色曲率系数）默认值：3.0f:2.0f步进:9.0f;
	// wav_range_b（蓝色波长范围）默认值：430.0f:1.0f步进:490.0f;
	//@param  : cmfcalib_handle        有效的CMF标定句柄
	//@param  : update_spec_params     有效的标定文件的参数结构体句柄    
	//@return : AlgoError
	//@Author : yuxingchao@2025/04/09
	//************************************/
	C_API AlgoError C_CALL CmfCalibSetSpecGenParam(AlgoHandle cmfcalib_handle, CmfCalibGenParam update_spec_params);


	///************************************
	//@brief  : 读取CS3000光谱仪输出的csv文件并自动赋值给成员变量
	//@note   : 
	//@param  : cmfcalib_handle        有效的CMF标定句柄
	//@param  : in_cmfcalib_fullname   CS3000光谱仪输出的csv文件路径   
	//@param  : round_idx		       CS3000光谱仪输出的csv文件在标定拍摄所处的拍摄轮次（一共两轮：0/1）
	//@param  : in_cie_type			   CS3000光谱仪输出的csv文件在标定拍摄时对应的相机拍摄所用的滤片类型 
	//@return : AlgoError
	//@Author : yuxingchao@2025/04/09
	//************************************/
	C_API AlgoError C_CALL CmfCalibReadCS3000ref(AlgoHandle cmfcalib_handle, const char* in_cmfcalib_fullname, const int round_idx, const CIEType in_cie_type);


	///************************************
	//@brief  : 完成所有参数和数据的set指令后，执行标定文件的生成和输出函数
	//@note   : 版本1可以设置的曲线标准仅为默认值CMF1931(0)，其余标准后续版本再开放
	//@param  : cmfcalib_handle        有效的CMF标定句柄
	//@param  : CmfMethod			   所采用的CMF曲线标准，如“CMF1931”，对应int为0; 其余选项：CMF1964(1), CMF2006_2(2), CMF2006_10(3), +new(4)
	//@return : AlgoError
	//@Author : yuxingchao@2025/04/09
	//************************************/
	C_API AlgoError C_CALL CmfCalibXYZCalib(AlgoHandle cmfcalib_handle, const int CmfMethod);


	///************************************
	//@brief  : 读取/保存标定相机标定文件
	//@note   : 后缀只能是.camcalib
	//@param  : cmfcalib_handle        有效的CMF标定句柄
	//@param  : in_cmfcalib_fullname   标定文件名,后缀只能是.cmfcalib
	//@return : AlgoError
	//@Author : yuxingchao@2025/04/09
	//************************************/
	C_API AlgoError C_CALL CmfCalibSave(AlgoHandle cmfcalib_handle, const char* in_cmfcalib_fullname);
	C_API AlgoError C_CALL CmfCalibLoad(AlgoHandle cmfcalib_handle, const char* in_cmfcalib_fullname);


	///************************************
	//@brief  : 将RGB-XYZ的Meter数据赋值给成员变量，用于白色的XYZ校正
	//@note   : 进行白色校正前务必先用该函数set，且需要传递的是RGB校正之前的原始meter，一共需要应用三次：R、G、B各一次，共九个meter;
	//@param  : cmfcalib_handle       有效的CMF标定句柄
	//@param  : in_x_meter_handle     输入有效的in_x_meter_handle句柄，输入的meter需要保证其AlgoColor、Capture的gain和exposure值正确  
	//@param  : in_y_meter_handle     输入有效的in_y_meter_handle句柄，输入的meter需要保证其AlgoColor、Capture的gain和exposure值正确
	//@param  : in_z_meter_handle     输入有效的in_z_meter_handle句柄，输入的meter需要保证其AlgoColor、Capture的gain和exposure值正确
	//@return : AlgoError
	//@Author : yuxingchao@2025/04/09
	//************************************/	
	C_API AlgoError C_CALL CmfCalibSetRGBMeters(AlgoHandle cmfcalib_handle, AlgoHandle in_x_meter_handle, AlgoHandle in_y_meter_handle, AlgoHandle in_z_meter_handle);


	///************************************
	//@brief  : XYZ校正，根据输入meter自带的AlgoColor判断进行哪个颜色的校正
	//@note   : 函数运行后meter的chroma值会被更新。
	//@note   : 进行白色校正前务必先用SetRGBMeters函数set，且需要传递的是RGB校正之前的原始meter，一共需要应用三次：R、G、B各一次，共九个meter
	//@param  : cmfcalib_handle       有效的CMF标定句柄
	//@param  : in_x_meter_handle     输入有效的in_x_meter_handle句柄，输入的meter需要保证其AlgoColor、Capture的gain和exposure值正确  
	//@param  : in_y_meter_handle     输入有效的in_y_meter_handle句柄，输入的meter需要保证其AlgoColor、Capture的gain和exposure值正确
	//@param  : in_z_meter_handle     输入有效的in_z_meter_handle句柄，输入的meter需要保证其AlgoColor、Capture的gain和exposure值正确            
	//@return : AlgoError
	//@Author : yuxingchao@2025/04/09
	//************************************/	
	C_API AlgoError C_CALL CmfCalibXYZRectify(AlgoHandle cmfcalib_handle, AlgoHandle in_out_x_meter_handle, AlgoHandle in_out_y_meter_handle, AlgoHandle in_out_z_meter_handle);


	///************************************
	//@brief  : 对任意单色LED屏幕指定任意选区进行平均光谱的重建还原
	//@note   : 函数运行后meter不会被更新，需要用GetSpectra来获取光谱结果。
	//@param  : cmfcalib_handle       有效的CMF标定句柄
	//@param  : roi_rect              屏幕指定选区的ROI rect
	//@param  : in_x_meter_handle     输入有效的in_x_meter_handle句柄，输入的meter需要保证其AlgoColor、Capture的gain和exposure值正确  
	//@param  : in_y_meter_handle     输入有效的in_y_meter_handle句柄，输入的meter需要保证其AlgoColor、Capture的gain和exposure值正确
	//@param  : in_z_meter_handle     输入有效的in_z_meter_handle句柄，输入的meter需要保证其AlgoColor、Capture的gain和exposure值正确  
	//@param  : out_spectra			  重建的均值光谱
	//@return : AlgoError
	//@Author : yuxingchao@2025/04/09
	//************************************/
	C_API AlgoError C_CALL CmfCalibSpectraRebuild(AlgoHandle cmfcalib_handle, AlgoHandle in_x_meter_handle, AlgoHandle in_y_meter_handle, AlgoHandle in_z_meter_handle, AlgoRect roi_rect, AlgoMat* out_spectra);



#ifdef __cplusplus
}
#endif // __cplusplus
#endif


