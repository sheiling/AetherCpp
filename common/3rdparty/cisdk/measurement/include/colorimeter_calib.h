#ifndef __COLORIMETER_CALIB_H__
#define __COLORIMETER_CALIB_H__

#include "ci_define.h"
#include "ci_declare.h"
#include "colorimeter_calib_data.h"
#include "colorimeter_calib_error.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

    /************************************
    @brief  : 创建色度计句柄
    @note   :
    @param  : out_colorimeter_handle    色度计句柄
    @return	: AlgoError
    @Author : ouxingtao@2021/07/29
    ************************************/
    C_API AlgoError C_CALL ColorimeterCreateHandle(AlgoHandle* out_colorimeter_handle);


	/************************************
	@brief  : 输出是否校正的标志位
	@note   : 将色度计采集的目标和原始的亮度及色温差异进行检查判断是否继续校正
	@param  : in_Tcp                  色温数组，[0]代表目标色温，[1]代表原始色温，[2]代表色温相对差异的限制
	@param  : in_Lv                   亮度数组，[0]代表目标亮度，[1]代表原始亮度，[2]代表亮度相对差异的限制
	@param  : out_flag                 输出标志位0代表继续校正，1代表停止校正
	@return	: AlgoError
	@Author : qianxiaotong@2023/09/20
	************************************/
	C_API AlgoError C_CALL ColorimeterCmpLTcp(const float* in_Tcp, const float* in_Lv, int* out_flag);

    
	/************************************
	@brief  : 设置剔除异常值的范围
	@note   : n_sigma表示取 means±n_sigma*sigma的数据为有效数据。默认为1. 在标定前设置
	@param  : n_sigma        表示有效数据的范围，±n_sigma*sigma。 默认为1
	@Author : tianbo@2025/06/12
	************************************/
	C_API AlgoError C_CALL ColorimeterSetAbnormalTh(AlgoHandle colorimeter_handle, const float in_n_sigma);


	/************************************
	@brief  : 设备标定类型
	@note   :
	@param  : in_type   COLORIMETERCALIB_MEASURE = 0,		//色度计标定：标定测量端， 即将两个色度计标定到一致
						COLORIMETERCALIB_TARGET = 1,		//色度计标定：标定最终结果，即通过修改最终的目标而保证两个色度计起到一样效果的作用。
						COLORIMETERCALIB_MULTICAMERA_AFTERCALIBRATION = 2,  //相机多光道标定，校正后标定，系数上屏后采集数据进行的标定
		                COLORIMETERCALIB_MULTICAMERA_ORIGINALDATA = 3      //相机多光道标定，标定原始值，利用校正过程中的数据进行标定（无需额外的采集操作）
	@Author : tianbo@2025/06/12
	************************************/
	C_API AlgoError C_CALL ColorimeterSetCalibType(AlgoHandle colorimeter_handle, const ColorimeterCalibType in_type);


	/************************************
	@brief  : 设置色度计标定入参数据（设置全局数据，如ca410采集的数据）
	@note   : 目的是解决因色度计不一致（ca410或成像式或...）
			支持两种模式，
			模式1： 修正两个色度计采集之间的差异
					这时输入的in_original_xyz为待标定色度计采集同一目标的值，in_target_xyz为标准色度计采集同一目标的值。
					此时的标定后，可得到两个色度计之间的差异，后续可通过Rectify将待标定设备采集值标定到标准设备采集值上。
			模式2： 修正待标定色度计设备的目标值，使得待标定设备和标准设备校正后的目标值一致（效果一致）。
					这时输入的in_original_xyz为使用待标定色度计进行校正后的模组在“固定某一设备”上采集的校正后的数据。
							  in_target_xyz为使用标准色度计进行校正后的模组在“固定某一设备”上采集的校正后的数据。
					此时标定后，可得到因两个色度计差异导致的目标值之间的差异，后续可通过Rectify将待标定设备的目标值标定到与标准设备一致（数值不一致，但效果一致）。
	@param  : colorimeter_handle       有效的色度计句柄
	@param  : in_original_xyz          待标定色度计数据,或待标定色度计校正后采集的数据，输入同一种颜色的XYZ数据,若只需标定某一滤片，则其他滤片置负数，如-1.0f
	@param  : in_target_xyz            参考色度计数据，或参考色度计校正后采集的数据，输入同一种颜色的XYZ数据，若只需标定某一滤片，则其他滤片置负数，如-1.0f
	@param  : in_len                   TriXYZ组数，即标定数据组数
	@param  : in_color                 待标定的色度计数据颜色 COLOR_RED / COLOR_GREEN / COLOR_BLUE / COLOR_WHITE
	@return	: AlgoError
	@Author : chengjuanru@2025/09/29
	************************************/
	C_API AlgoError C_CALL ColorimeterSetCalibTriXYZ(AlgoHandle colorimeter_handle, const TriXYZ* in_original_xyz, const TriXYZ* in_target_xyz, const int in_len, const AlgoColor in_color);
	

	/************************************
	@brief  : 设置逐点标定入参数据（设置用于标定计算的逐点数据，如meter数据）
	@note   : 目的是解决多光道校正问题,
		支持同时输入三个滤片数据，但实际是顺序读入，若长度为3，第一个元素和第三个元素有值，第二个元素为空时，算法内部会做是否为空的判断，为空会报错
			
	@param  : colorimeter_handle       有效的色度计句柄
	@param  : in_original_meter        待标定光道相机数据,输入同一种颜色的XYZ数据
	@param  : in_target_meter          目标光道相机数据，输入同一种颜色的XYZ数据
	@param  : in_meter_num             输入meter的个数，不能大于3
	@return	: AlgoError
	@Author : chengjuanru@2025/09/29
	************************************/
	C_API AlgoError C_CALL ColorimeterSetCalibMeter(AlgoHandle colorimeter_handle, AlgoHandle* in_original_meter, AlgoHandle* in_target_meter, const int in_meter_num);
	

	/************************************
	@brief  : 标定接口
	@note   : 在调用该接口前，必须先设置标定类型和设置标定入参数据

	@param  : colorimeter_handle       有效的色度计句柄
	@return	: AlgoError
	@Author : chengjuanru@2025/09/29
	************************************/
	C_API AlgoError C_CALL ColorimeterCalib(AlgoHandle colorimeter_handle);


	/************************************
	@brief  : 色度计修正（修正全局数据，如ca410采集的数据）
	@note   : 
	@param  : colorimeter_handle       有效的色度计句柄
	@param  : in_out_xyz               待修正色度计数据，输入同一种颜色的XYZ数据,若只需修正某一滤片，则其他滤片置负数，如-1.0f
	@param  : in_color                 待修正的色度计数据颜色 COLOR_RED / COLOR_GREEN / COLOR_BLUE / COLOR_WHITE
	@return	: AlgoError
	@Author : ouxingtao@2021/07/29
	************************************/
	C_API AlgoError C_CALL ColorimeterRectifyTriXYZ(AlgoHandle colorimeter_handle, TriXYZ* in_out_xyz, const AlgoColor in_color);


	/************************************
	@brief  : 多光道修正（修正逐点数据，如meter数据）
	@note   : 用于多光道修正
	@param  : colorimeter_handle       有效的色度计句柄
	@param  : in_out_meter             待修正光道数据
	@return	: AlgoError
	@Author : chengjuanru@2025/09/29
	************************************/
	C_API AlgoError C_CALL ColorimeterRectifyMeter(AlgoHandle colorimeter_handle, AlgoHandle in_out_meter);


	/************************************
	@brief  : 多个色度计标定文件取均值
	@note   : 
	@param  : in_colorimeter_handle    输入多个色度计句柄
	@param  : in_num                   输入的色度计句柄数量
	@param  : out_colorimeter_handle   输出的色度计句柄
	@return	: AlgoError
	@Author : chengjuanru@2026/03/05
	************************************/
	C_API AlgoError C_CALL ColorimeterAverage(AlgoHandle *in_colorimeter_handle, int in_num, AlgoHandle out_colorimeter_handle);



	/************************************
	@brief  : 多个色度计标定文件取中位数
	@note   :
	@param  : in_colorimeter_handle    输入多个色度计句柄
	@param  : in_num                   输入的色度计句柄数量
	@param  : out_colorimeter_handle   输出的色度计句柄
	@return	: AlgoError
	@Author : chengjuanru@2026/03/05
	************************************/
	C_API AlgoError C_CALL ColorimeterMedian(AlgoHandle *in_colorimeter_handle, int in_num, AlgoHandle out_colorimeter_handle);



    /************************************
    @brief  : 读取保存色度计标定文件
    @note   : 后缀只能是.colorimeter
    @param  : colorimeter_handle        有效的色度计句柄
    @param  : in_colorimeter_fullname   标定文件,后缀只能是.colorimeter
    @return	: AlgoError
    @Author : chengjuanru@2025/09/29
    ************************************/
    C_API AlgoError C_CALL ColorimeterSave(AlgoHandle colorimeter_handle, const char* in_colorimeter_fullname);
    C_API AlgoError C_CALL ColorimeterLoad(AlgoHandle colorimeter_handle, const char* in_colorimeter_fullname);


    /************************************
    @brief  : 获取buffer大小
    @note   :
    @param  : colorimeter_handle  当前有效句柄
    @param  : out_len             buffer大小
    @return	: AlgoError
    @Author : chengjuanru@2025/09/29
    ************************************/
    C_API AlgoError C_CALL ColorimeterGetBufferSize(AlgoHandle colorimeter_handle, uint64_t* out_len);


    /************************************
    @brief  : 将colorimeter里面的数据导出到buffer
    @note   :
    @param  : colorimeter_handle  当前有效句柄
    @param  : out_buffer          已经申请好申请空间
    @param  : in_out_len          外部申请好的空间大小,会用真实大小修改
    @return	: AlgoError
    @Author : chengjuanru@2025/09/29
    ************************************/
    C_API AlgoError C_CALL ColorimeterToBuffer(AlgoHandle colorimeter_handle, char* out_buffer, uint64_t* in_out_len);


    /************************************
    @brief  : 将buffer数据导入到colorimeter
    @note   :
    @param  : colorimeter_handle   有效的colorimeter句柄
    @param  : in_buffer
    @param  : in_len
    @return	: AlgoError
    @Author : chengjuanru@2025/09/29
    ************************************/
    C_API AlgoError C_CALL ColorimeterFromBuffer(AlgoHandle colorimeter_handle, const char* in_buffer, uint64_t in_len);


    /************************************
    @brief  : 获取句柄内容
    @note   : 注意浅拷贝
    @param  : colorimeter_handle    有效的colorimeter句柄
    @param  : out_colorimeter       导出的结构体,必须已经初始化完毕
    @param  : is_deep_copy          是否深拷贝, 默认浅拷贝  1=深拷贝;0=浅拷贝
    @return	: AlgoError
    @Author : chengjuanru@2025/09/29
    ************************************/
    C_API AlgoError C_CALL ColorimeterGet(AlgoHandle colorimeter_handle, AlgoColorimeterCalib* out_colorimeter, int in_is_deep_copy = 0);


	/************************************
	@brief  : 设置内容
	@note   :
	@param  : colorimeter_handle    有效的colorimeter句柄
	@param  : in_colorimeter        要输入的数据
	@return	: AlgoError
	@Author : chengjuanru@2025/09/29
	************************************/
	C_API AlgoError C_CALL ColorimeterSet(AlgoHandle colorimeter_handle, const AlgoColorimeterCalib* in_colorimeter);


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
    C_API AlgoError C_CALL ColorimeterRelease(AlgoHandle colorimeter_handle);
    C_API void C_CALL ColorimeterDestroyHandle(AlgoHandle colorimeter_handle);


#ifdef __cplusplus
}
#endif // __cplusplus
#endif
