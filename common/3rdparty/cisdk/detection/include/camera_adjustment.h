#pragma once
#ifndef __CAMERA_ADJUSTMENT_H__
#define __CAMERA_ADJUSTMENT_H__
#include "ci_define.h"
#include "ci_declare.h"
#include "algo_mat.h"
#include "meta_info_data.h"
#include "camera_adjustment_data.h"
#include "camera_adjustment_error.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	/************************************
	@brief  : 设备相机位置姿态框计算
	@note   :
	@param  : in_target                              -- 测试目标实际宽高,单位mm
	@param  : in_cam                                 -- 相机信息
	@param  : in_lens                                -- 镜头信息
	@param  : in_pos                                 -- 相机姿态
	@param  : in_capture_distance                    -- 相机和测量目标距离,单位mm
	@param  : out_pose_box_corners                   -- 输出姿态框四个角点,像素
	@param  : in_corner_num                          -- 角点个数
	@return	: AlgoError
	@Author : lipengpeng@2022/12/22
	************************************/
	C_API AlgoError C_CALL CamAdjComputePose(const AlgoSize* in_target,
		const CameraInfo* in_cam,
		const LensInfo* in_lens,
		const CameraPos* in_pos,
		float in_capture_distance,
		AlgoPoint* out_pose_box_corners,
		int in_corner_num = 4);

	/************************************
	@brief  : 设备相机清晰度表征值计算
	@note   :
	@param  : out_sharpness                          -- 输出清晰度表征值
	@param  : in_img                                 -- 输入Mat图像
	@param  : in_color                               -- 输入颜色类型：COLOR_RED,COLOR_GREEN,COLOR_BLUE,COLOR_WHITE
	@param  : in_corner4                             -- 输入角点坐标
	@Param	: in_corner_num		                     -- 角点个数=4
	@return	: AlgoError
	@Author : lipengpeng@2022/12/22
	************************************/
	C_API AlgoError C_CALL CamAdjComputeSharpness(float* out_sharpness, const AlgoMat* in_img,
		AlgoColor in_color, const AlgoPoint* in_corner4, int in_corner_num = 4);

	/************************************
	@brief  : 设备相机亮度表征值计算
	@note   :
	@param  : out_brightness                         -- 输出亮度表征值
	@param  : in_img                                 -- 输入Mat图像
	@param  : in_color                               -- 输入颜色类型：COLOR_RED,COLOR_GREEN,COLOR_BLUE,COLOR_WHITE
	@param  : in_corner4                             -- 输入角点坐标
	@Param	: in_corner_num		                     -- 角点个数=4
	@param  : in_cam_bit_depth                       -- 相机类型-位深：8,10,12,14,16；默认=-1为图像默认位深
	@return	: AlgoError
	@Author : lipengpeng@2022/12/22
	************************************/
	C_API AlgoError C_CALL CamAdjComputeBrightness(float* out_brightness, const AlgoMat* in_img,
		AlgoColor in_color, const AlgoPoint* in_corner4, int in_corner_num = 4,
		int  in_cam_bit_depth = -1);

	/************************************
	@brief  : 设备相机清晰度调节测试卡生成
	@note   :
	@param  : out_sharpness_card                     -- 输出清晰度测试卡图像,打印后使用
	@param  : in_target                              -- 清晰度测试卡实际宽高,单位mm
	@param  : in_cam                                 -- 相机信息
	@param  : in_lens                                -- 镜头信息
	@param  : in_capture_distance                    -- 相机和测量目标距离,单位mm
	@return	: AlgoError
	@Author : lipengpeng@2022/12/22
	************************************/
	C_API AlgoError C_CALL CamAdjWriteSharpnessCard(const char * out_sharpness_card,
		const AlgoSize* in_target,
		const CameraInfo* in_cam,
		const LensInfo* in_lens,
		float in_capture_distance);


#ifdef __cplusplus
}
#endif // __cplusplus
#endif