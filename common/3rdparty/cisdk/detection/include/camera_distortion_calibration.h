#pragma once
#ifndef __CAMERA_DISTORTION_CALIBRATION_H__
#define __CAMERA_DISTORTION_CALIBRATION_H__
#include "ci_define.h"
#include "ci_declare.h"
#include "algo_mat.h"
#include "meta_info_data.h"
#include "camera_distortion_calibration_data.h"
#include "camera_distortion_calibration_error.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	/************************************
	@brief  : 创建设备相机原位畸变标定句柄
	@note   :
	@param  : out_distcalib_handle                           -- 有效的设备相机原位畸变标定句柄
	@return	: AlgoError
	@Author : lipengpeng@2023/02/10
	************************************/
	C_API AlgoError C_CALL CamDistCalibCreateHandle(AlgoHandle* out_distcalib_handle);


	/************************************
	@brief  : 计算相机内参外参畸变标定矩阵系数
	@note   :
	@param  : distcalib_handle                               -- 有效的设备相机原位畸变标定句柄
	@param  : in_cell_size                                   -- 棋盘格每个单元格实际尺寸，单位mm
	@param  : in_corner_num                                  -- 棋盘格内角点数：行和列角点数
	@param  : in_checkerboard_img                            -- 包含in_img_num组棋盘格标定板图像路径
	@param  : in_img_num                                     -- 图像数量：棋盘格标定板图像数量
	@return	: AlgoError
	@Author : lipengpeng@2023/02/10
	************************************/
	C_API AlgoError C_CALL CamDistCalibCompute(AlgoHandle distcalib_handle, const AlgoSize2f* in_cell_size,
		const AlgoSize* in_corner_num, const AlgoMat*  in_checkerboard_img, const int in_img_num);


	/************************************
	@brief  : 棋盘格标定板图像生成
	@note   : 生成的棋盘格的每个单元格为正方形
	@param  : distcalib_handle                               -- 有效的设备相机原位畸变标定句柄
	@param  : in_checkerboard_rows                           -- 棋盘格内角点：行的数量
	@param  : in_checkerboard_cols                           -- 棋盘格内角点：列的数量
	@param  : in_cell_pixel_num                              -- 棋盘格每个单元格子所占行或列的像素数
	@param  : out_checkerboard                               -- 输出生成的棋盘格图像，打印后用于畸变校正内参外参计算
	@return	: AlgoError
	@Author : lipengpeng@2023/02/10
	************************************/
	C_API AlgoError C_CALL CamDistCalibBoard(AlgoHandle distcalib_handle, const int in_checkerboard_rows,
		const int in_checkerboard_cols, const int in_cell_pixel, const char* out_checkerboard);


	/************************************
	@brief  : 相机畸变矫正
	@note   : 通过内参外参矩阵完成相机畸变矫正
	@param  : distcalib_handle                               -- 有效的设备相机原位畸变标定句柄
	@param  : in_out_img                                     -- 输入待矫正的图像，输入输出图像通道一致
	@return	: AlgoError
	@Author : lipengpeng@2023/02/10
	************************************/
	C_API AlgoError C_CALL CamDistCalibRectify(AlgoHandle distcalib_handle, AlgoMat* in_out_img);


	/************************************
	@brief  : 导出相机内参外参矩阵,用于后续的相机畸变矫正
	@note   : 后缀为.xml，用于软件端调用
	@param  : distcalib_handle                               -- 有效的设备相机原位畸变标定句柄
	@param  : in_distcalib_fullname                          -- 畸变标定文件,后缀是.xml
	@return	: AlgoError
	@Author : lipengpeng@2023/02/10
	************************************/
	C_API AlgoError C_CALL CamDistCalibExportXml(AlgoHandle distcalib_handle, const char* in_distcalib_fullname);


	/************************************
	@brief  : 导入相机内参外参矩阵,用于后续的相机畸变矫正：此接口暂未开放
	@note   : 后缀为.xml，用于软件端调用
	@param  : distcalib_handle                               -- 有效的设备相机原位畸变标定句柄
	@param  : in_distcalib_fullname                          -- 畸变标定文件,后缀是.xml
	@return	: AlgoError
	@Author : lipengpeng@2023/02/10
	************************************/
	C_API AlgoError C_CALL CamDistCalibImportXml(AlgoHandle distcalib_handle, const char* in_distcalib_fullname);


	/************************************
	@brief  : 读取保存的畸变矫正文件
	@note   : 后缀只能是.distcalib
	@param  : distcalib_handle                               -- 有效的设备相机原位畸变标定句柄
	@param  : in_distcalib_fullname                          -- 畸变标定文件,后缀只能是.distcalib
	@return	: AlgoError
	@Author : lipengpeng@2023/02/10
	************************************/
	C_API AlgoError C_CALL CamDistCalibSave(AlgoHandle distcalib_handle, const char* in_distcalib_fullname);
	C_API AlgoError C_CALL CamDistCalibLoad(AlgoHandle distcalib_handle, const char* in_distcalib_fullname);


	/************************************
	@brief  : set设备信息
	@note   :
	@param  : distcalib_handle                               -- 有效的设备相机原位畸变标定句柄
	@param  : in_device                                      -- 设备信息
	@return	: AlgoError
	@Author : lipengpeng@2023/02/10
	************************************/
	C_API AlgoError C_CALL CamDistCalibSetDevice(AlgoHandle distcalib_handle, const Device* in_device);


	/************************************
	@brief  : get设备信息
	@note   :
	@param  : distcalib_handle                               -- 有效的设备相机原位畸变标定句柄
	@param  : out_device                                     -- 导出设备信息
	@return	: AlgoError
	@Author : lipengpeng@2023/02/10
	************************************/
	C_API AlgoError C_CALL CamDistCalibGetDevice(AlgoHandle distcalib_handle, Device* out_device);


	/************************************
	@brief  : 获取畸变标定数据
	@note   : 注意浅拷贝
	@param  : distcalib_handle                               -- 有效的设备相机原位畸变标定句柄
	@param  : out_distcalib                                  -- 导出的结构体,必须已经初始化完毕
	@param  : is_deep_copy                                   -- 是否深拷贝, 默认浅拷贝  1=深拷贝;0=浅拷贝
	@return	: AlgoError
	@Author : lipengpeng@2023/02/10
	************************************/
	C_API AlgoError C_CALL CamDistCalibGet(AlgoHandle distcalib_handle, AlgoCamDistCalib* out_distcalib, int in_is_deep_copy = 0);


	/************************************
	@brief  : 设置畸变标定模块数据
	@note   :
	@param  : distcalib_handle                               -- 有效的设备相机原位畸变标定句柄
	@param  : in_distcalib                                   -- 数据结构提：标定参数
	@return	: AlgoError
	@Author : lipengpeng@2023/02/10
	************************************/
	C_API AlgoError C_CALL CamDistCalibSet(AlgoHandle distcalib_handle, const AlgoCamDistCalib* in_distcalib);


	/************************************
	@brief  : 获取buffer大小
	@note   :
	@param  : distcalib_handle                               -- 有效的设备相机原位畸变标定句柄
	@param  : out_len                                        -- buffer大小
	@return	: AlgoError
	@Author : lipengpeng@2023/02/10
	************************************/
	C_API AlgoError C_CALL CamDistCalibGetBufferSize(AlgoHandle distcalib_handle, uint64_t* out_len);


	/************************************
	@brief  : 将distcalib里面的数据导出到buffer
	@note   :
	@param  : distcalib_handle                               -- 有效的设备相机原位畸变标定句柄
	@param  : out_buffer                                     -- 已经申请好申请空间
	@param  : in_out_len                                     -- *in_out_len外部申请好的空间大小,会用真实大小修改
	@return	: AlgoError
	@Author : lipengpeng@2023/02/10
	************************************/
	C_API AlgoError C_CALL CamDistCalibToBuffer(AlgoHandle distcalib_handle, char* out_buffer, uint64_t* in_out_len);


	/************************************
	@brief  : 将buffer数据导入到camdist
	@note   :
	@param  : distcalib_handle                               -- 有效的设备相机原位畸变标定句柄
	@param  : in_buffer
	@param  : in_len
	@return	: AlgoError
	@Author : lipengpeng@2023/02/10
	************************************/
	C_API AlgoError C_CALL CamDistCalibFromBuffer(AlgoHandle distcalib_handle, const char* in_buffer, uint64_t in_len);


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
	C_API AlgoError C_CALL CamDistCalibRelease(AlgoHandle distcalib_handle);
	C_API void C_CALL CamDistCalibDestroyHandle(AlgoHandle distcalib_handle);



#ifdef __cplusplus
}
#endif // __cplusplus
#endif