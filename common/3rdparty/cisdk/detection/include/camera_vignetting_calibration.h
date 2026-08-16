#pragma once
#ifndef __CAMERA_VIGNETTING_CALIBRATION_H__
#define __CAMERA_VIGNETTING_CALIBRATION_H__
#include "ci_define.h"
#include "ci_declare.h"
#include "algo_mat.h"
#include "meta_info_data.h"
#include "camera_vignetting_calibration_data.h"
#include "camera_vignetting_calibration_error.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	/************************************
	@brief  : 创建设备相机原位渐晕标定句柄
	@note   :
	@param  : out_vigcalib_handle                            -- 有效的设备原位相机渐晕标定句柄
	@param  : in_ratio                                       -- 下采样倍率,默认16
	@return	: AlgoError
	@Author : lipengpeng@2023/02/10
	************************************/
	C_API AlgoError C_CALL CamVigCalibCreateHandle(AlgoHandle* out_vigcalib_handle, int in_ratio = 16);


	/************************************
	@brief  : 计算设备相机原位渐晕标定系数矩阵
	@note   : 用于相机渐晕校正
	@param  : vigcalib_handle                                -- 有效的设备原位相机渐晕标定句柄
	@param  : in_vigcalib_img                                -- 输入标定板图像
	@param  : in_roi                                         -- 输入roi区域，此区域为标定区域
	@param  : in_vigcalib_bg                                 -- 输入背景图，无背景时候置为nullptr
	@return	: AlgoError
	@Author : lipengpeng@2023/02/10
	************************************/
	C_API AlgoError C_CALL CamVigCalibCompute(AlgoHandle vigcalib_handle, const AlgoMat* in_vigcalib_img,
		AlgoRect in_roi = { -1, -1, -1, -1 }, const AlgoMat* in_vigcalib_bg = nullptr);


	/************************************
	@brief  : 相机渐晕校正
	@note   : 通过渐晕校正系数完成校正
	@param  : vigcalib_handle                                -- 有效的设备原位相机渐晕标定句柄
	@param  : in_out_img                                     -- 输入待修正的Mat，图像通道当前只支持1和3通道
	@return	: AlgoError
	@Author : lipengpeng@2023/02/10
	************************************/
	C_API AlgoError C_CALL CamVigCalibRectify(AlgoHandle vigcalib_handle, AlgoMat* in_out_img);


	/************************************
	@brief  : 导出相机渐晕校正系数矩阵
	@note   : 后缀为.xml，用于软件端调用
	@param  : vigcalib_handle                                -- 有效的设备原位相机渐晕标定句柄
	@param  : in_vigcalib_fullname                           -- 渐晕校正文件,后缀是.xml
	@return	: AlgoError
	@Author : lipengpeng@2023/02/10
	************************************/
	C_API AlgoError C_CALL CamVigCalibExportXml(AlgoHandle vigcalib_handle, const char* in_vigcalib_fullname);


	/************************************
	@brief  : 导入相机渐晕校正系数矩阵：此接口暂未开放
	@note   : 后缀为.xml，用于软件端调用
	@param  : vigcalib_handle                                -- 有效的设备原位相机渐晕标定句柄
	@param  : in_vigcalib_fullname                           -- 渐晕标定文件,后缀是.xml
	@return	: AlgoError
	@Author : lipengpeng@2023/02/10
	************************************/
	C_API AlgoError C_CALL CamVigCalibImportXml(AlgoHandle vigcalib_handle, const char* in_vigcalib_fullname);


	/************************************
	@brief  : 读取保存的渐晕校正文件
	@note   : 后缀只能是.vigcalib
	@param  : vigcalib_handle                                -- 有效的设备原位相机渐晕标定句柄
	@param  : in_vigcalib_fullname                           -- 标定文件,后缀只能是.vigcalib
	@return	: AlgoError
	@Author : lipengpeng@2023/02/10
	************************************/
	C_API AlgoError C_CALL CamVigCalibSave(AlgoHandle vigcalib_handle, const char* in_vigcalib_fullname);
	C_API AlgoError C_CALL CamVigCalibLoad(AlgoHandle vigcalib_handle, const char* in_vigcalib_fullname);


	/************************************
	@brief  : set设备信息
	@note   :
	@param  : vigcalib_handle                                -- 有效的设备原位相机渐晕标定句柄
	@param  : in_device                                      -- 设备信息
	@return	: AlgoError
	@Author : lipengpeng@2023/02/10
	************************************/
	C_API AlgoError C_CALL CamVigCalibSetDevice(AlgoHandle vigcalib_handle, const Device* in_device);


	/************************************
	@brief  : get设备信息
	@note   :
	@param  : vigcalib_handle                                -- 有效的设备原位相机渐晕标定句柄
	@param  : out_device                                     -- 导出设备信息
	@return	: AlgoError
	@Author : lipengpeng@2023/02/10
	************************************/
	C_API AlgoError C_CALL CamVigCalibGetDevice(AlgoHandle vigcalib_handle, Device* out_device);


	/************************************
	@brief  : 获取畸变标定数据
	@note   : 注意浅拷贝
	@param  : vigcalib_handle                                -- 有效的设备原位相机渐晕标定句柄
	@param  : out_vigcalib                                   -- 导出的结构体,必须已经初始化完毕
	@param  : is_deep_copy                                   -- 是否深拷贝, 默认浅拷贝  1=深拷贝;0=浅拷贝
	@return	: AlgoError
	@Author : lipengpeng@2023/02/10
	************************************/
	C_API AlgoError C_CALL CamVigCalibGet(AlgoHandle vigcalib_handle, AlgoCamVigCalib* out_vigcalib, int in_is_deep_copy = 0);


	/************************************
	@brief  : 设置畸变标定模块数据
	@note   :
	@param  : vigcalib_handle                                -- 有效的设备原位相机渐晕标定句柄
	@param  : in_vigcalib                                    -- 数据结构体
	@return	: AlgoError
	@Author : lipengpeng@2023/02/10
	************************************/
	C_API AlgoError C_CALL CamVigCalibSet(AlgoHandle vigcalib_handle, const AlgoCamVigCalib* in_vigcalib);


	/************************************
	@brief  : 获取buffer大小
	@note   :
	@param  : vigcalib_handle                                -- 有效的设备原位相机渐晕标定句柄
	@param  : out_len                                        -- buffer大小
	@return	: AlgoError
	@Author : lipengpeng@2023/02/10
	************************************/
	C_API AlgoError C_CALL CamVigCalibGetBufferSize(AlgoHandle vigcalib_handle, uint64_t* out_len);


	/************************************
	@brief  : 将vigcalib里面的数据导出到buffer
	@note   :
	@param  : vigcalib_handle                                -- 有效的设备原位相机渐晕标定句柄
	@param  : out_buffer                                     -- 已经申请好申请空间
	@param  : in_out_len                                     -- *in_out_len外部申请好的空间大小,会用真实大小修改
	@return	: AlgoError
	@Author : lipengpeng@2023/02/10
	************************************/
	C_API AlgoError C_CALL CamVigCalibToBuffer(AlgoHandle vigcalib_handle, char* out_buffer, uint64_t* in_out_len);


	/************************************
	@brief  : 将buffer数据导入到vigcalib
	@note   :
	@param  : vigcalib_handle                                -- 有效的设备原位相机渐晕标定句柄
	@param  : in_buffer
	@param  : in_len
	@return	: AlgoError
	@Author : lipengpeng@2023/02/10
	************************************/
	C_API AlgoError C_CALL CamVigCalibFromBuffer(AlgoHandle vigcalib_handle, const char* in_buffer, uint64_t in_len);


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
	C_API AlgoError C_CALL CamVigCalibRelease(AlgoHandle vigcalib_handle);
	C_API void C_CALL CamVigCalibDestroyHandle(AlgoHandle vigcalib_handle);



#ifdef __cplusplus
}
#endif // __cplusplus
#endif