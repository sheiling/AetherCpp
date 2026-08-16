#pragma once
#ifndef __CAMERA_CHROMA_CALIBRATION_H__
#define __CAMERA_CHROMA_CALIBRATION_H__
#include "ci_define.h"
#include "ci_declare.h"
#include "algo_mat.h"
#include "meta_info_data.h"
#include "camera_chroma_calibration_data.h"
#include "camera_chroma_calibration_error.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	/************************************
	@brief  : 创建设备相机原位色度标定句柄
	@note   :
	@param  : out_chromacalib_handle                         -- 有效的设备相机原位色度标定句柄
	@return	: AlgoError
	@Author : lipengpeng@2023/02/10
	************************************/
	C_API AlgoError C_CALL CamChromaCalibCreateHandle(AlgoHandle* out_chromacalib_handle);


	/************************************
	@brief  : 计算相机色度标定矩阵
	@note   : 用于相机色度标定颜色转换矩阵计算
	@param  : chromacalib_handle                             -- 有效的设备相机原位色度标定句柄
	@param  : in_rgb_img                                     -- 包含in_num组色度标定需要的RGB三通道图像
	@param  : in_mask                                        -- mask区域：用于提取色度标定区域图像信息
	@param  : in_colorimeter_xyz                             -- 包含in_num组TriXYZ色度计XYZ数据
	@param  : in_img_num                                     -- 图像数量：色度标定采集的图像数量
	@param  : in_bg                                          -- 输入背景图，无背景时候置为nullptr
	@return	: AlgoError
	@Author : lipengpeng@2023/02/10
	************************************/
	C_API AlgoError C_CALL CamChromaCalibCompute(AlgoHandle chromacalib_handle, const AlgoMat* in_rgb_img,
		const AlgoMat* in_mask, TriXYZ* in_colorimeter_xyz, const int in_num, const AlgoMat* in_bg = nullptr);


	/************************************
	@brief  : 打屏Pattern图像生成
	@note   : 打屏Pattern图像用于点亮屏体，色度标定时使用
	@param  : chromacalib_handle                             -- 有效的设备相机原位色度标定句柄
	@param  : in_led_wh                                      -- 输入实际灯点分辨率
	@param  : in_r_gray                                      -- 输入打屏灰阶值：R通道灰阶值范围0-255
	@param  : in_g_gray                                      -- 输入打屏灰阶值：G通道灰阶值范围0-255
	@param  : in_b_gray                                      -- 输入打屏灰阶值：B通道灰阶值范围0-255
	@param  : out_pattern_img                                -- 输出生成的打屏Pattern图像，用于灯板点亮时打在屏体上
	@return	: AlgoError
	@Author : lipengpeng@2023/02/10
	************************************/
	C_API AlgoError C_CALL CamChromaCalibPattern(AlgoHandle chromacalib_handle, const AlgoSize* in_led_wh,
		const unsigned char in_r_gray, const unsigned char in_g_gray, const unsigned char  in_b_gray,
		const char * out_pattern_img);


	/************************************
	@brief  : 设备相机色度原位标定
	@note   : 通过设备相机原位色度标定矩阵完成相机色度标定
	@param  : chromacalib_handle                             -- 有效的设备相机原位色度标定句柄
	@param  : in_rgb_out_xyz_img                             -- 输入rgb三通道图像，输出xyz三通道融合后的图像
	@param  : in_bg                                          -- 输入背景图，无背景时候置为nullptr
	@return	: AlgoError
	@Author : lipengpeng@2023/02/10
	************************************/
	C_API AlgoError C_CALL CamChromaCalibRectify(AlgoHandle chromacalib_handle, AlgoMat* in_rgb_out_xyz_img,
		const AlgoMat* in_bg = nullptr);


	/************************************
	@brief  : 导出相机色度标定矩阵,用于后续的相机色度标定
	@note   : 后缀为.xml，用于软件端调用
	@param  : chromacalib_handle                             -- 有效的设备相机原位色度标定句柄
	@param  : in_chromacalib_fullname                        -- 色度标定文件,后缀是.xml
	@return	: AlgoError
	@Author : lipengpeng@2023/02/10
	************************************/
	C_API AlgoError C_CALL CamChromaCalibExportXml(AlgoHandle chromacalib_handle, const char* in_chromacalib_fullname);


	/************************************
	@brief  : 导入相机色度标定矩阵,用于相机色度标定：此接口暂未开放
	@note   : 后缀为.xml
	@param  : chromacalib_handle                             -- 有效的设备相机原位色度标定句柄
	@param  : in_chromacalib_fullname                        -- 色度标定文件,后缀是.xml
	@return	: AlgoError
	@Author : lipengpeng@2023/02/10
	************************************/
	C_API AlgoError C_CALL CamChromaCalibImportXml(AlgoHandle chromacalib_handle, const char* in_chromacalib_fullname);


	/************************************
	@brief  : 读取保存的色度标定文件
	@note   : 后缀只能是.chromacalib
	@param  : chromacalib_handle                             -- 有效的设备相机原位色度标定句柄
	@param  : in_chromacalib_fullname                        -- 色度标定文件,后缀只能是.chromacalib
	@return	: AlgoError
	@Author : lipengpeng@2023/02/10
	************************************/
	C_API AlgoError C_CALL CamChromaCalibSave(AlgoHandle chromacalib_handle, const char* in_chromacalib_fullname);
	C_API AlgoError C_CALL CamChromaCalibLoad(AlgoHandle chromacalib_handle, const char* in_chromacalib_fullname);


	/************************************
	@brief  : set设备信息
	@note   :
	@param  : chromacalib_handle                             -- 有效的设备相机原位色度标定句柄
	@param  : in_device                                      -- 设备信息
	@return	: AlgoError
	@Author : lipengpeng@2023/02/10
	************************************/
	C_API AlgoError C_CALL CamChromaCalibSetDevice(AlgoHandle chromacalib_handle, const Device* in_device);


	/************************************
	@brief  : get设备信息
	@note   :
	@param  : chromacalib_handle                             -- 有效的设备相机原位色度标定句柄
	@param  : out_device                                     -- 导出设备信息
	@return	: AlgoError
	@Author : lipengpeng@2023/02/10
	************************************/
	C_API AlgoError C_CALL CamChromaCalibGetDevice(AlgoHandle chromacalib_handle, Device* out_device);


	/************************************
	@brief  : 获取色度标定数据
	@note   : 注意浅拷贝
	@param  : chromacalib_handle                             -- 有效的设备相机原位色度标定句柄
	@param  : out_chromacalib                                -- 导出的结构体,必须已经初始化完毕
	@param  : is_deep_copy                                   -- 是否深拷贝, 默认浅拷贝  1=深拷贝;0=浅拷贝
	@return	: AlgoError
	@Author : lipengpeng@2023/02/10
	************************************/
	C_API AlgoError C_CALL CamChromaCalibGet(AlgoHandle chromacalib_handle, AlgoCamChromaCalib* out_chromacalib, int in_is_deep_copy = 0);


	/************************************
	@brief  : 设置色度标定模块数据
	@note   :
	@param  : chromacalib_handle                             -- 有效的设备相机原位色度标定句柄
	@param  : in_chromacalib                                 -- 数据结构体
	@return	: AlgoError
	@Author : lipengpeng@2023/02/10
	************************************/
	C_API AlgoError C_CALL CamChromaCalibSet(AlgoHandle chromacalib_handle, const AlgoCamChromaCalib* in_chromacalib);


	/************************************
	@brief  : 获取buffer大小
	@note   :
	@param  : chromacalib_handle                             -- 有效的设备相机原位色度标定句柄
	@param  : out_len                                        -- buffer大小
	@return	: AlgoError
	@Author : lipengpeng@2023/02/10
	************************************/
	C_API AlgoError C_CALL CamChromaCalibGetBufferSize(AlgoHandle chromacalib_handle, uint64_t* out_len);


	/************************************
	@brief  : 将chromacalib里面的数据导出到buffer
	@note   :
	@param  : chromacalib_handle                             -- 有效的设备相机原位色度标定句柄
	@param  : out_buffer                                     -- 已经申请好申请空间
	@param  : in_out_len                                     -- *in_out_len外部申请好的空间大小,会用真实大小修改
	@return	: AlgoError
	@Author : lipengpeng@2023/02/10
	************************************/
	C_API AlgoError C_CALL CamChromaCalibToBuffer(AlgoHandle chromacalib_handle, char* out_buffer, uint64_t* in_out_len);


	/************************************
	@brief  : 将buffer数据导入到chromacalib
	@note   :
	@param  : chromacalib_handle                             -- 有效的设备相机原位色度标定句柄
	@param  : in_buffer
	@param  : in_len
	@return	: AlgoError
	@Author : lipengpeng@2023/02/10
	************************************/
	C_API AlgoError C_CALL CamChromaCalibFromBuffer(AlgoHandle chromacalib_handle, const char* in_buffer, uint64_t in_len);


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
	C_API AlgoError C_CALL CamChromaCalibRelease(AlgoHandle chromacalib_handle);
	C_API void C_CALL CamChromaCalibDestroyHandle(AlgoHandle chromacalib_handle);



#ifdef __cplusplus
}
#endif // __cplusplus
#endif