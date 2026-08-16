#ifndef ____LED_ILLUMINATION_DETECTOR_H__
#define ____LED_ILLUMINATION_DETECTOR_H__

#include "ci_declare.h"
#include "ci_define.h"
#include "ci_data.h"
#include "led_illumination_detector_data.h"
#include "led_illumination_detector_errors.h"
#include "led_meter_inspector.h"
#include "algo_mat.h"


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


	/************************************
	@brief  : 创建检测句柄
	@note   :
	@return : AlgoError
	@author : songshaojun@2024/04/25
	************************************/
	C_API AlgoHandle C_CALL LEDIlluminationCreateHandle();


	/************************************
	@brief  : 设置死暗亮灯检测阈值
	@note   : 
	@param  : detect_handle           有效的检测句柄
	@param  : in_thresh               输入量,阈值结构体,详见led_illumination_detector_data.h
	@return : AlgoError
	@author : songshaojun@2024/04/25
	************************************/
	C_API AlgoError C_CALL LEDIlluminationSetThresh(AlgoHandle detect_handle, const LEDIlluminationDetectThresh* in_thresh);


	/************************************
	@brief  : 获取死暗亮灯检测阈值
	@note   : 
	@param  : detect_handle           有效的检测句柄
	@param  : in_thresh               输出量,阈值结构体,详见led_illumination_detector_data.h
	@return : AlgoError
	@author : songshaojun@2024/04/25
	************************************/
	C_API AlgoError C_CALL LEDIlluminationGetThresh(AlgoHandle detect_handle, LEDIlluminationDetectThresh* out_thresh);

	/************************************
	@brief  : 设置检测方法
	@note   : 算法内部有默认值，默认值为1，即取背景时使用众数
	@param  : detect_handle           有效的检测句柄
	@param  : in_method               输入量,检测方法枚举,详见led_illumination_detector_data.h
	@return : AlgoError
	@author : songshaojun@2024/04/25
	************************************/
	C_API AlgoError C_CALL LEDIlluminationSetMethod(AlgoHandle detect_handle, const LEDIlluminationDetectMehtod* in_method);

	/************************************
	@brief  : 获取默认检测方法
	@note   : 
	@param  : detect_handle           有效的检测句柄
	@param  : out_method              输出量,方法枚举,详见led_illumination_detector_data.h
	@return : AlgoError
	@author : songshaojun@2024/04/25
	************************************/
	C_API AlgoError C_CALL LEDIlluminationGetMethod(AlgoHandle detect_handle, LEDIlluminationDetectMehtod* out_method);


	/************************************
    @brief  : 设置内参,构造背景窗口尺寸
    @note   : 内部有默认值为41,一般不需要调用
			  点亮机场景：推荐设置为5
    @param  : detect_handle           有效的检测句柄
    @param  : in_bg_kernel_size       输入量,构造背景窗口大小
    @return : AlgoError
    @author : songshaojun@2025/04/15
    ************************************/
	C_API AlgoError C_CALL LEDIlluminationSetBgKernelSize(AlgoHandle detect_handle, const int in_bg_kernel_size);


	/************************************
    @brief  :  获取内参,构造背景窗口尺寸
    @note   :  
    @param  : detect_handle           有效的检测句柄
    @param  : out_bg_kernel_size      输出量,构造背景窗口大小
    @return : AlgoError
    @author : songshaojun@2025/04/15
    ************************************/
	C_API AlgoError C_CALL LEDIlluminationGetBgKernelSize(AlgoHandle detect_handle,  int* out_bg_kernel_size);

	/************************************
    @brief  : 设置内参,滤波核大小
    @note   : 内部有默认值,一般不需要调用
    @param  : detect_handle           有效的检测句柄
    @param  : in_median_size          输入量,中值滤波核心大小
	@param  : in_mean_size            输入量,均值滤波核心大小
    @return : AlgoError
    @author : songshaojun@2024/05/07
    ************************************/
	C_API AlgoError C_CALL LEDIlluminationSetKernelSize(AlgoHandle detect_handle, const int in_median_size,const int in_mean_size);


	/************************************
    @brief  : 获取内参,滤波核大小
    @note   : 
    @param  : detect_handle           有效的检测句柄
    @param  : out_median_size         输出量,中值滤波核心大小
	@param  : out_mean_size           输出量,均值滤波核心大小
    @return : AlgoError
    @author : songshaojun@2024/05/07
    ************************************/
	C_API AlgoError C_CALL LEDIlluminationGetKernelSize(AlgoHandle detect_handle,  int* out_median_size, int* out_mean_size);


	/************************************
	@brief  : 设置内参,停止生长阈值
	@note   : 内部有默认值,一般不需要调用
	@param  : detect_handle           有效的检测句柄
	@param  : in_b_th                 输入量,b通道下停止生长阈值
	@param  : in_g_th                 输入量,g通道下停止生长阈值
	@param  : in_r_th                 输入量,r通道下停止生长阈值
	@return : AlgoError
	@author : songshaojun@2024/05/07
	************************************/
	C_API AlgoError C_CALL LEDIlluminationSetGrowthThresh(AlgoHandle detect_handle, const float in_b_th, const float in_g_th,const float in_r_th);


	/************************************
	@brief  : 获取内参,停止生长阈值
	@note   : 
	@param  : detect_handle           有效的检测句柄
	@param  : out_b_th                输入量,b通道下停止生长阈值
	@param  : out_g_th                输入量,g通道下停止生长阈值
	@param  : out_r_th                输入量,r通道下停止生长阈值
	@return : AlgoError
	@author : songshaojun@2024/05/07
	************************************/
	C_API AlgoError C_CALL LEDIlluminationGetGrowthThresh(AlgoHandle detect_handle,  float* out_b_th,  float* out_g_th, float* out_r_th);


	/************************************
	@brief  : 缺陷灯检测,检测项包含：死灯、暗灯、亮灯,兼容单颗形式,以及连续聚集形式
	@note   : 目前仅支持单色打屏画面,即蓝色、绿色、红色
	@param  : detect_handle           有效的检测句柄
	@param  : in_meter_handle         输入量,待检测的MeterHandle
	@return : AlgoError
	@author : songshaojun@2024/04/25
	************************************/
	C_API AlgoError C_CALL LEDIlluminationDetect(AlgoHandle detect_handle, const AlgoHandle in_meter_handle);


	/************************************
	@brief  : 获取LEDIlluminationDetect接口的检测结果,以灯点坐标List形式返回
	@note   : 根据标志位返回缺陷类型
	@param  : detect_handle                有效的检测句柄
	@param  : in_needed_defect_type        输入量,需要获取的缺陷灯类型,例如死灯 =LED_DEFECT_FLAG_DEAD, 详见led_meter_inspector_data.h;
	@param  : in_defect_num                输入量,Meter的灯点总数,最终输出会更改为实际缺陷灯个数
	@param  : out_defect_loc_list          输出量,缺陷灯点坐标的List
	@param  : in_no_need_defect_type       输入量,不需要获取的缺陷灯类型,例如暗灯 = LED_DEFECT_FLAG_DIM, 默认为正常灯, 详见led_meter_inspector_data.h;
	@return : AlgoError
	@author : songshaojun@2024/04/25
	************************************/
	C_API AlgoError C_CALL LEDIlluminationGetResultList(AlgoHandle detect_handle, 
		uint32_t in_needed_defect_type, int* in_defect_num,
		AlgoPoint* out_defect_loc_list, uint32_t in_no_need_defect_type = LED_DEFECT_FLAG_UNINIT);


	/************************************
	@brief  : 获取LEDIlluminationDetect接口的检测结果,以Mat形式返回
	@note   : 返回结果Mat
	@param  : detect_handle                有效的检测句柄
	@param  : in_meter_handle              输入量,待检测的MeterHandle
	@param  : out_result_mat               输出量,缺陷灯点的Mat结果
	@return : AlgoError
	@author : songshaojun@2024/04/25
	************************************/
	C_API AlgoError C_CALL LEDIlluminationGetResultMat(AlgoHandle detect_handle, AlgoMat* out_result_mat);


	///********************buffer相关的***************************
	/************************************
	@brief    ：	载入和保存
	@note     ：save,将handle内此时的阈值都保存下来,
                load,将文件内阈值读入handle内,	
				文件名后缀必须为.leddetect;
	@param    : detect_handle		               有效的检测句柄
	@param    : in_led_illumination_fullname	   输入量,文件路径,文件名后缀必须为.leddetect
	@return   : AlgoError
	@author   : songshaojun@2024/04/25
	************************************/
	C_API AlgoError C_CALL LEDIlluminationSave(AlgoHandle detect_handle, const char* in_led_illumination_fullname);
	C_API AlgoError C_CALL LEDIlluminationLoad(AlgoHandle detect_handle, const char* in_led_illumination_fullname);


	/************************************
	@brief    : 获取检测buffer大小
	@note     :
	@param    : detect_handle          有效的检测句柄
	@param    : out_len                输出量,buffer大小
	@return   : AlgoError
	@author   : songshaojun@2024/04/25
	************************************/
	C_API AlgoError C_CALL LEDIlluminationGetBufferSize(AlgoHandle detect_handle, uint64_t* out_len);


	/************************************
	@brief    : 将LEDIlluminationDetect里面的数据导出到buffer
	@note     :
	@param    : detect_handle          有效的检测句柄
	@param    : out_buffer             已经申请好申请空间
	@param    : in_out_len             外部申请好的空间大小,会用真实大小修改
	@return   : AlgoError
	@author   : songshaojun@2024/04/25
	************************************/
	C_API AlgoError C_CALL LEDIlluminationToBuffer(AlgoHandle detect_handle, char* out_buffer, uint64_t* in_out_len);


	/************************************
	@brief    : 将buffer数据导入到LEDIlluminationDetect
	@note     :
	@param    : detect_handle          有效的检测句柄
	@param    : in_buffer              输入量,buffer
	@param    : in_len                 输入量,buffer大小
	@return   : AlgoError
	@author   : songshaojun@2024/04/25
	************************************/
	C_API AlgoError C_CALL LEDIlluminationFromBuffer(AlgoHandle detect_handle, const char* in_buffer, uint64_t in_len);


	C_API AlgoError C_CALL LEDIlluminationRelease(AlgoHandle detect_handle);
	C_API void C_CALL LEDIlluminationDestroyHandle(AlgoHandle detect_handle);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif