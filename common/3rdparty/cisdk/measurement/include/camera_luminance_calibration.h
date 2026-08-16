#ifndef __CAMERA_LUMINANCE_CALIBRATION_HPP__
#define __CAMERA_LUMINANCE_CALIBRATION_HPP__

#include "ci_define.h"
#include "ci_declare.h"
#include "camera_luminance_calibration_data.h"
#include "camera_luminance_calibration_error.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/************************************
@brief  : 创建相机亮度标定句柄
@note   :
@return	: AlgoError
@Author : Zhangxuan@2022/11/16
************************************/
	C_API AlgoError C_CALL CameraLuminanceCreateHandle(AlgoHandle* out_cameraluminance_handle);

	

	/************************************
	@brief  : 读取raw图    --本地端（系数标定）
	@note   : 
	@param  : cameraluminance_handle      --有效句柄
	@param  : in_filename_img             --标定所用raw图的完整路径
	@param  : in_filename_imgbg           --标定所用背景raw图的完整路径
	@param  : in_cam_type                 --相机型号
	@param  : in_cie_type                 --相机滤片
	@return	: AlgoError
	@Author : Zhangxuan@2022/12/2
	************************************/
	C_API AlgoError C_CALL CameraLuminanceReadCoeffImgRaw(AlgoHandle cameraluminance_handle, const char* in_filename_img,
		const char* in_filename_imgbg, CameraType in_cam_type, const CIEType in_cie_type);


	/************************************
	@brief  : 参数读取及设置    --本地端（系数标定）
	@note   : 
	@param  : in_col                     --对应的颜色：例如：green-y       
	@param  : in_exptime                 --对应的曝光时间
	@param  : in_brightness              --对应的亮度数据
	@param  : in_aperture                --光圈值
	@param  : in_microfocus              --微焦
	@param  : in_origarea_w=300          --初始选取区域的宽，默认为300
	@param  : in_origarea_h=300          --初始选取区域的高，默认为300
	@return	: AlgoError               
	@Author : Zhangxuan@2022/11/19
	************************************/
	C_API AlgoError C_CALL CameraLuminanceSetParaCoeffLocal(AlgoHandle cameraluminance_handle, const char* in_col,
		const float in_exptime, const float in_brightness, const float in_aperture, const float in_microfocus,
		int in_origarea_w=300, int in_origarea_h=300);
	


	/************************************
	@brief  : 参数读取及设置    --软件端（系数标定）
	@note   :
	@param  : in_img_handle              --原始raw图像句柄
	@param  : in_imgbg_handle            --原始背景图像句柄
	@param  : in_cam_type                 --相机型号
	@param  : in_cie_type                 --相机滤片
	@param  : in_col                     --对应的颜色：例如：green-y       
	@param  : in_exptime                 --对应的曝光时间
	@param  : in_brightness              --对应的亮度数据
	@param  : in_aperture                --光圈值
	@param  : in_microfocus              --微焦
	@param  : in_origarea_w=300          --初始选取区域的宽，默认为300
	@param  : in_origarea_h=300          --初始选取区域的高，默认为300
	@return	: AlgoError
	@Author : Zhangxuan@2022/12/2
	************************************/
	C_API AlgoError C_CALL CameraLuminanceSetParaCoeffSoft(AlgoHandle cameraluminance_handle, AlgoHandle in_img_handle, AlgoHandle in_imgbg_handle,
		const CameraType in_cam_type, const CIEType in_cie_type, const char* in_col, const float in_exptime, const float in_brightness,
		const float in_aperture, const float in_microfocus, int in_origarea_w=300, int in_origarea_h=300);


	/************************************
	@brief  : 成像质量评估     --本地/软件端（系数标定/亮度计算）
	@note   : 
	@param  : in_img_handle   --需进行质量评估的图像
	@param  : th              --成像矩形度判断的阈值，默认为0.95
	@return	: AlgoError
	@Author : Zhangxuan@2022/11/19
	************************************/
	C_API AlgoError C_CALL CameraLuminanceQualEvaluate(AlgoHandle cameraluminance_handle, 
		AlgoHandle in_img_handle,const float th=0.95f);

	/************************************
	@brief  : 模型系数计算    --本地/软件端（系数标定）
	@note   : 
	@param  : 
	@return	: AlgoError
	@Author : Zhangxuan@2022/11/19
	************************************/
	C_API AlgoError C_CALL CameraLuminanceCoeffCompute(AlgoHandle cameraluminance_handle);

	/************************************
	@brief  : 标定相机下不同微焦对应成像面积比例关系的计算         --本地/软件端（系数标定）
	@note   : 
	@param  : in_img_handle       --不同微焦下的图像
	@param  : in_num              --微焦数
	@return	: AlgoError
	@Author : Zhangxuan@2022/11/19
	************************************/
	C_API AlgoError C_CALL CameraLuminanceCompuAreaRatios(AlgoHandle cameraluminance_handle, AlgoHandle* in_img_handle, const int in_num);

	/************************************
	@brief  : 标定数据的保存       --本地/软件端（系数标定）
	@note   : 
	@param  : in_savefile           --保存路径(按列保存为标定系数，光圈，微焦，颜色，滤片，不同微焦下的面积比)
	@return	: AlgoError
	@Author : Zhangxuan@2022/11/19
	************************************/
	C_API AlgoError C_CALL CameraLuminanceSaveCalibdata(AlgoHandle cameraluminance_handle, const char* in_savefile);

/*******************************************************************************************************************************************************/
/*******************************************************************************************************************************************************/
/*******************************************************************************************************************************************************/
	//实际亮度计算阶段
    
    /************************************
    @brief  : 标定文件读取：标定数据读入输出变量out_calibdata中   --本地/软件端（亮度计算）
    @note   : 
    @param  : in_calibdatafile       --标定数据的保存路径
	@param  : out_algocalibdata      --读取的标定数据  
    @return	: AlgoError
    @Author : Zhangxuan@2022/11/19
    ************************************/
	C_API AlgoError C_CALL CameraLuminanceReadCalibdata(AlgoHandle cameraluminance_handle, const char* in_calibdatafile,
		AlgoCameraLuminanceCalibData* out_algocalibdata);


	/************************************
	@brief  : 测试raw图像的读取     --本地端（亮度计算）      
	@note   : 
	@param  : in_filename_img             --标定所用raw图的完整路径
	@param  : in_filename_imgbg           --标定所用背景raw图的完整路径
	@param  : in_cam_type                 --相机型号
	@param  : in_cie_type                 --相机滤片
	@return	: AlgoError
	@Author : Zhangxuan@2022/11/19
	************************************/
	C_API AlgoError C_CALL CameraLuminanReadLumImgRaw(AlgoHandle cameraluminance_handle, const char* in_filename_img, const char* in_filename_imgbg,
		CameraType in_cam_type, const CIEType in_cie_type);
	/************************************
	@brief  : 参数设置             --本地端（亮度计算）
	@note   : 
	@param  : in_algocalibdata           --对应的标定数据
	@param  : in_col_str                 --对应的颜色：例如：green-y（请以此格式输入）       
	@param  : in_exptime                 --对应的曝光时间
	@param  : in_aperture                --光圈值
	@param  : in_microfocus              --微焦
	@param  : in_origarea_w=300          --初始选取区域的宽，默认为300
	@param  : in_origarea_h=300          --初始选取区域的高，默认为300
	@return	: AlgoError
	@Author : Zhangxuan@2022/11/21
	************************************/
	C_API AlgoError C_CALL CameraLuminanceSetParaLumLocal(AlgoHandle cameraluminance_handle, AlgoCameraLuminanceCalibData* in_algocalibdata, 
		const char* in_col_str,		const float in_exptime, const float in_aperture, const float in_microfocus, 
		int in_origarea_w=300, int in_origarea_h=300);
	

	/************************************
	@brief  : 参数读取及设置          --软件端（亮度计算）
	@note   :
	@param  : cameraluminance_handle
	@param  : in_img_handle              --原始raw图像句柄
	@param  : in_imgbg_handle            --原始背景图像句柄
	@param  : in_cam_type                --相机型号
	@param  : in_cie_type                --相机滤片
	@param  : in_algocalibdata           --对应的标定数据
	@param  : in_col                     --对应的颜色：例如：green-y
	@param  : in_exptime                 --对应的曝光时间
	@param  : in_aperture                --光圈值
	@param  : in_microfocus              --微焦
	@param  : in_origarea_w=300          --初始选取区域的宽，默认为300
	@param  : in_origarea_h=300          --初始选取区域的高，默认为300

	@return	: AlgoError
	@Author : Zhangxuan@2022/11/19
	************************************/
	C_API AlgoError C_CALL CameraLuminanceSetParaLumSoft(AlgoHandle cameraluminance_handle, AlgoHandle in_img_handle, 
		AlgoHandle in_imgbg_handle,	const CameraType in_cam_type, const CIEType in_cie_type, 
		AlgoCameraLuminanceCalibData* in_algocalibdata,	const char* in_col, const float in_exptime, 
		const float in_aperture, const float in_microfocus, int in_origarea_w = 300, int in_origarea_h = 300);


	/************************************
	@brief  : 计算亮度        --本地/软件端（亮度计算）
	@note   : 
	@param  : 
	@return	: AlgoError
	@Author : Zhangxuan@2022/11/19
	************************************/
	C_API AlgoError C_CALL CameraLuminanceLumCompute(AlgoHandle cameraluminance_handle);
	
	/************************************
	@brief  : 标定文件读取：标定数据读入输出变量out_algocalibdata中   
	@note   :
	@param  : in_calibdatafile       --标定数据的保存路径
	@param  : out_algocalibdata      --读取的标定数据
	@return	: AlgoError
	@Author : Zhangtao@2023/8/4
	************************************/
	C_API AlgoError C_CALL CameraLuminanceReadchromaCalibdata(AlgoHandle cameraluminance_handle, 
		const char* in_calibdatafile[], AlgoCameraLuminanceCalibchromaData* out_algocalibdata);


	/************************************
	@brief  : 计算亮色度       
	@note   :
	@param  : in_img_handle    -- XYZ_shot
	@return	: AlgoError
	@Author : Zhangtao@2023/8/4
	************************************/
	C_API AlgoError C_CALL CameraLuminanceChromaCompute(AlgoHandle cameraluminance_handle, 
		AlgoHandle in_img_handle[]);


	/************************************
	@brief  : 参数设置             --本地端（亮度计算）
	@note   :
	@param  : in_algocalibdata           --对应的标定数据
	@param  : in_col_str                 --对应的颜色：例如：green-x（请以此格式输入）
	@param  : in_exptime                 --对应的曝光时间
	@param  : in_aperture                --光圈值
	@param  : in_microfocus              --微焦
	@param  : in_cie_type                --CIE_TYPE
	@return	: AlgoError
	@Author : Zhangtao@2023/8/4
	************************************/
	C_API AlgoError C_CALL CameraLuminanceSetParaLumLocalChroma(AlgoHandle cameraluminance_handle, 
		AlgoCameraLuminanceCalibchromaData* in_algocalibdata, const char* in_color_str[],
		const float in_exptime[3], const float in_aperture[3], const float in_microfocus[3], const CIEType in_cie_type[3]);

	
	
	/************************************
	@brief  : 获取亮色度数据
	@note   :
	@param  : cameraluminance_handle    --有效的相机亮度标定句柄
	@param  : size                      --像素比例（逐点、2*2、4*4）
	@param  : size_method               --输出类型：0: xyz   1：lxy
	@param  : out_data                  --输出数据
	@return	: AlgoError
	@Author : Zhangtao@2023/8/4
	************************************/
	C_API AlgoError C_CALL CameraLuminanceGetCalOptiChroma(AlgoHandle cameraluminance_handle, 
		int size, int save_method, AlgoMat* out_data, int in_is_deep_copy = 0);



	/************************************
	@brief  : 获取指定区域亮色度数据
	@note   :
	@param  : cameraluminance_handle    --有效的相机亮度标定句柄
	@param  : r                         --指定位置行索引
	@param  : c                         --指定位置列索引
	@param  : length                    --指定位置半径
	@param  : out_point_data            --输出数据
	@return	: AlgoError
	@Author : Zhangtao@2023/8/4
	************************************/
	C_API AlgoError C_CALL CameraLuminanceGetPointChroma(AlgoHandle cameraluminance_handle, 
		int r, int c, int length, AlgoMat* out_point_data, int in_is_deep_copy = 0);
	/******************************************************************************************************/
	/***************************************************其他***********************************************/
	/******************************************************************************************************/
	
	/************************************
	@brief  : 获取相机亮度标定数据
	@note   :
	@param  : cameraluminance_handle    --有效的相机亮度标定句柄
	@param  : out_cameraluminance
	@return	: AlgoError
	@Author : Zhangxuan@2022/11/19
	************************************/
	C_API AlgoError C_CALL CameraLuminanceGet(AlgoHandle cameraluminance_handle, AlgoCameraLuminanceCalibration* out_cameraluminance);

	C_API AlgoError C_CALL CameraLuminanceSave(AlgoHandle cameraluminance_handle, const char* in_fullname);
	C_API AlgoError C_CALL CameraLuminanceLoad(AlgoHandle cameraluminance_handle, const char* in_fullname);

	/************************************
	@brief  : 将cameraluminance_handle里面的数据导出到buffer
	@note   :
	@param  : cameraluminance_handle    --有效的相机亮度标定句柄
	@param  : out_buffer                --已经申请好申请空间
	@param  : in_out_len                --*in_out_len外部申请好的空间大小,会用真实大小修改
	@return	: AlgoError
	@Author : Zhangxuan@2022/11/19
	************************************/
	C_API AlgoError C_CALL CameraLuminanceToBuffer(AlgoHandle cameraluminance_handle, char* out_buffer, uint64_t* in_out_len);
	

	/************************************
	@brief  : 将buffer数据导入到cameraluminance_handle中
	@note   :
	@param  : cameraluminance_handle   有效的ChromaCoeff句柄
	@param  : in_buffer
	@param  : in_len
	@return	: AlgoError
	@Author : Zhangxuan@2022/11/19
	************************************/
	C_API AlgoError C_CALL CameraLuminanceFromBuffer(AlgoHandle cameraluminance_handle, const char* in_buffer, uint64_t in_len);
	
	/************************************
	@brief  : 获取buffer大小
	@note   :
	@param  : cameraluminance_handle    --当前有效句柄
	@param  : out_len                   --buffer大小
	@return	: AlgoError
	@Author : Zhangxuan@2022/11/19
	************************************/
	C_API AlgoError C_CALL CameraLuminanceGetBufferSize(AlgoHandle cameraluminance_handle, uint64_t* out_len);
	
	C_API void C_CALL CameraLuminanceDestroyHandle(AlgoHandle cameraluminance_handle);
	C_API AlgoError C_CALL CameraLuminanceRelease(AlgoHandle cameraluminance_handle);



	C_API AlgoError C_CALL test_shot(AlgoHandle cameraluminance_handle);
	C_API AlgoError C_CALL test_read_shot(AlgoHandle shot_handle,
		const char* in_raw_file,
		CIEType in_cie_type,
		int in_cam_width, int in_cam_height,
		BayerType in_bayer_type,
		int in_cam_bit_depth = 16);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif