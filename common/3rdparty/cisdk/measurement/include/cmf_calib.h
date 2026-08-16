#ifndef __CMF_CALIB_H__
#define __CMF_CALIB_H__

#include "ci_define.h"
#include "ci_declare.h"
#include "cmf_calib_data.h"
#include "cmf_calib_error.h"

#ifdef __cplusplus
extern "C" 
{
#endif // __cplusplus  

	/************************************
	@brief  : 创建相机标定句柄
	@note   :
	@param  : out_cmfcalib_handle		有效的CMF句柄
	@return	: AlgoError
	@Author : yuxingchao@2025/03/20
	************************************/
	C_API AlgoError C_CALL CmfCalibCreateHandle(AlgoHandle *out_cmfcalib_handle);

	/************************************
	@brief  : 设置CMF标定的波段范围与数据采集的步长         
	@note   : 不调用该接口时，按照内部默认参数进行标定与修正
	@param  : cmfcalib_handle			有效的CMF句柄
	@param  : in_wave_min				标定波段左区间，要求 360.f < in_wave_min < 380.f	    
	@param  : in_wave_max				标定波段右区间，要求 720.f < in_wave_min < 780.f	 
	@param  : in_step					数据采集波长步长, 要求 1.f < in_step < 60.f
	@return : AlgoError
	@Author : zhongao@2025/04/24
	************************************/
	C_API AlgoError C_CALL CmfCalibSetDefaultWavelength(AlgoHandle cmfcalib_handle, const float in_wave_min, const float in_wave_max, const float in_step);

	/************************************
	@brief  : 设置生成标定数据库参数
	@note   : 不调用该接口时，则按照内部默认参数进行标定与修正
	@param  : cmfcalib_handle			有效的CMF标定句柄
	@param  : in_spec_params			标定数据库参数    
	@return : AlgoError
	@Author : zhongao@2025/04/24
	************************************/
	C_API AlgoError C_CALL CmfCalibSetDefaultSpecGenParam(AlgoHandle cmfcalib_handle, const CamCmfGenParam& in_spec_params);

	/************************************
	@brief  : 设置标定的自定义曲线
	@note   : 参数含义详见camera_cmf_calib_data.h
	@param  : cmfcalib_handle			有效的CMF标定句柄
	@param  : in_std_mat				输入的std标准曲线，格式要求如下：
											1、尺寸需要严格按照(n,4)，且每一行按照（波长，X刺激值，Y刺激值，Z刺激值）的顺序排列
											2、需要按照第一列数据（波长数据）递增顺序进行排列
											3、整个in_std_mat不能存在负值
	@return : AlgoError
	@Author : zhongao@2025/04/27
	************************************/
	C_API AlgoError C_CALL CmfCalibSetDefaultCMFCurve(AlgoHandle cmfcalib_handle, AlgoMat* in_std_mat);

	/************************************
	@brief  : 设置标定的标准CMF曲线
	@note   : 参数含义详见camera_cmf_calib_data.h
	@param  : cmfcalib_handle			有效的CMF标定句柄
	@param  : in_std_method				选择内置的CMF标定曲线，目前支持CMF1931、1964、2006_2、2006_10，标定曲线标准需要与CS3000拍摄标准同步
	@param  : in_std_method_crt		    选择内置的CMF校正曲线，目前支持CMF1931、1964、2006_2、2006_10、2012_2 - 10等
	@return : AlgoError
	@Author : zhongao@2025/04/28 Modified by yuxingchao@2025/06/30
	************************************/
	C_API AlgoError C_CALL CmfCalibSetDefaultCMFStdMethod(AlgoHandle cmfcalib_handle, const CMFStandard& in_std_method, const CMFStandard& in_std_method_crt);

	/************************************
	@brief  : 设置标定时计算shot均值的roi区域
	@note   : 
	@param  : cmfcalib_handle			有效的CMF标定句柄
	@param  : in_roi					标定时计算shot均值的roi区域
	@return : AlgoError
	@Author : zhongao@2025/04/28
	************************************/
	C_API AlgoError C_CALL CmfCalibSetRoi(AlgoHandle cmfcalib_handle, const AlgoRect& in_roi);

	/************************************
	@brief  : 按照采集的波长顺序设置对应的shot与光谱数据
	@note   : 
	@param  : cmfcalib_handle			有效的CMF标定句柄
	@param  : in_shot					当前波长对应的shot句柄，数据要求如下：
											1、in_shot为当前波长下重复采集8张shot，减去首尾数据（即6张shot），各自去除背景后，求均值得到的shot（软件端处理）
											2、in_shot的颜色、滤片、曝光时间、相机增益、光圈信息完整
	@param  : in_spectra				当前波长对应的光谱仪采集的XYZ数据，数据要求如下：
											1、in_spectra.X为光谱仪X数据，in_spectra.Y为光谱仪Y数据，in_spectra.Z为光谱仪Z数据
	@param  : in_wave_id				in_shot对应的波长索引，数据解释如下：
											1、如标定波段为[380nm,720nm],步长为5nm，那么380nm对应in_shot的in_wave_id=0，那么385nm对应in_shot的in_wave_id=1，以此类推
	@param  : in_rect					计算in_shot均值的roi区域
	@Author : zhongao@2025/04/27
	************************************/
	C_API AlgoError C_CALL CmfCalibSetInput(AlgoHandle cmfcalib_handle, AlgoHandle in_shot, const TriXYZ& in_spectra, int in_wave_id);
	
	/************************************
	@brief  : 生成相机标定主接口（基于标定的相机CMF生成标定文件）
	@note   : 
	@param  : cmfcalib_handle			有效的CMF句柄
	@return : AlgoError
	@Author : zhongao@2025/04/27
	************************************/
	C_API AlgoError C_CALL CmfCalibCompute(AlgoHandle cmfcalib_handle);


	/************************************
	@brief  : 生成用于光枪测量等一次测量的标定主接口（基于标准CMF生成标定文件）
	@note   :
	@param  : cmfcalib_handle			有效的CMF句柄
	@return : AlgoError
	@Author : yuxingchao@2025/07/03
	************************************/
	C_API AlgoError C_CALL CmfCalibComputeForStd(AlgoHandle cmfcalib_handle);


	/************************************
	@brief  : 修正主接口
	@note   : 待修正的xyz meter的颜色、chroma大小需要保持一致，且需要完整的pos信息
	@param  : cmfcalib_handle			有效的CMF句柄
	@param  : in_out_x_meter_handle		修正前的in_x_meter_handle句柄
	@param  : in_out_y_meter_handle		修正前的in_y_meter_handle句柄
	@param  : in_out_z_meter_handle		修正前的in_z_meter_handle句柄        
	@param  : in_rgb_xyz_meters_handle	仅白色修正时需要设置，为RxRyRzGxGyGzBxByBz meter句柄数组，白色修正时数据要求如下：
										1、必须按照RxRyRzGxGyGzBxByBz的顺序存放
										2、RxRyRzGxGyGzBxByBz的chroma大小一致且与待修正的白色xyz meter一致
										3、RxRyRzGxGyGzBxByBz必须是单色修正前的meter
	@param  : in_rgb_xyz_meters_num		白色修正需要的meter句柄数组长度，白色修正时必须为9
	@return : AlgoError
	@Author : zhongao@2025/04/27
	************************************/	
	C_API AlgoError C_CALL CmfCalibRectify(AlgoHandle cmfcalib_handle, AlgoHandle in_out_x_meter_handle, AlgoHandle in_out_y_meter_handle, AlgoHandle in_out_z_meter_handle,
		AlgoHandle* in_rgb_xyz_meters_handle = nullptr, int in_rgb_xyz_meters_num = 0);

	/************************************
	@brief  : 对任意单色LED屏幕指定任意选区进行平均光谱的重建还原
	@note   : 函数运行后meter不会被更新
			  需要保证in_x_meter_handle、in_y_meter_handle、in_z_meter_handle的颜色、相机增益一致
	@param  : cmfcalib_handle			有效的CMF句柄
	@param  : roi_rect					屏幕指定选区的ROIrect
	@param  : in_x_meter_handle			用于计算重建光谱的in_x_meter_handle句柄
	@param  : in_y_meter_handle			用于计算重建光谱的in_y_meter_handle句柄
	@param  : in_z_meter_handle			用于计算重建光谱的in_z_meter_handle句柄
	@param  : out_spectra				重建的均值光谱图像，尺寸为(2,341)，第一列为光谱数据y，第二列为波长x，外部画图按照x,y的顺序画图即可
	@return : AlgoError
	@Author : yuxingchao@2025/04/09
	************************************/
	C_API AlgoError C_CALL CmfCalibSpectraRebuild(AlgoHandle cmfcalib_handle, AlgoHandle in_x_meter_handle, AlgoHandle in_y_meter_handle, AlgoHandle in_z_meter_handle,
		AlgoRect roi_rect, AlgoMat* out_spectra);


	/************************************
	@brief  : 对任意单色LED屏幕用光枪测量得到的XYZ的光谱的重建还原（）
	@note   : 需要保证XYZ的颜色一致
	@param  : cmfcalib_handle			有效的CMF句柄
	@param  : in_XYZ			        用于计算重建光谱的单词测量XYZ值
	@param  : out_spectra				重建的均值光谱图像，尺寸为(2,341)，第一列为光谱数据y，第二列为波长x，外部画图按照x,y的顺序画图即可
	@param  : in_color                  只能红绿蓝单色的color type：如COLOR_RED
	@return : AlgoError
	@Author : yuxingchao@2025/07/03
	************************************/
	C_API AlgoError C_CALL CmfCalibSpectraRebuildForTri(AlgoHandle cmfcalib_handle, TriXYZ in_XYZ, AlgoColor in_color, 
		AlgoMat* out_spectra);

	/************************************
	@brief  : 获取均匀场的device信息
	@note   : 
	@param  : cmfcalib_handle			有效的CMF句柄
	@param  : out_device			    导出的device结构体
	@return : AlgoError
	@Author : zhongao@2025/11/14
	************************************/
	C_API AlgoError C_CALL CmfCalibGetDevice(AlgoHandle cmfcalib_handle, Device* out_device);

	/************************************
	@brief  : 获取均匀场的calib_roi信息
	@note   :
	@param  : cmfcalib_handle			有效的CMF句柄
	@param  : out_rect					导出的标定时的ROI
	@return : AlgoError
	@Author : zhongao@2026/03/16
	************************************/
	C_API AlgoError C_CALL CmfCalibGetRoi(AlgoHandle cmfcalib_handle, AlgoRect* out_rect);

	/************************************
	@brief  : 读取/保存标定相机标定文件
	@note   : 后缀只能是.camcalib
	@param  : cmfcalib_handle			有效的CMF句柄
	@param  : in_cmfcalib_fullname		标定文件名,后缀只能是.cmfcalib
	@return : AlgoError
	@Author : yuxingchao@2025/04/09
	************************************/
	C_API AlgoError C_CALL CmfCalibSave(AlgoHandle cmfcalib_handle, const char* in_cmfcalib_fullname);
	C_API AlgoError C_CALL CmfCalibLoad(AlgoHandle cmfcalib_handle, const char* in_cmfcalib_fullname);

	/************************************
	@brief  : 获取buffer大小
	@note   :
	@param  : cmfcalib_handle			有效的CMF句柄
	@param  : out_len					buffer大小
	@return	: AlgoError
	@Author : zhongao@2025/04/24
	************************************/
	C_API AlgoError C_CALL CmfCalibGetBufferSize(AlgoHandle cmfcalib_handle, uint64_t* out_len);

	/************************************
	@brief  : 将uniform里面的数据导出到buffer
	@note   :
	@param  : cmfcalib_handle			有效的CMF句柄
	@param  : out_buffer				外部已经申请好的内存空间
	@param  : in_out_len				外部空间大小
	@return	: AlgoError
	@Author : zhongao@2025/04/24
	************************************/
	C_API AlgoError C_CALL CmfCalibToBuffer(AlgoHandle cmfcalib_handle, char* out_buffer, uint64_t* in_out_len);

	/************************************
	@brief  : 将buffer数据导入到uniform
	@note   :
	@param  : cmfcalib_handle			有效的CMF句柄
	@param  : in_buffer					外部已经申请好的内存空间
	@param  : in_len					外部空间大小
	@return	: AlgoError
	@Author : zhongao@2025/04/24
	************************************/
	C_API AlgoError C_CALL CmfCalibFromBuffer(AlgoHandle cmfcalib_handle, const char* in_buffer, uint64_t in_len);

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
	C_API AlgoError C_CALL CmfCalibRelease(AlgoHandle cmfcalib_handle);
	C_API void C_CALL  CmfCalibDestroyHandle(AlgoHandle cmfcalib_handle);


#ifdef __cplusplus
}
#endif // __cplusplus
#endif


