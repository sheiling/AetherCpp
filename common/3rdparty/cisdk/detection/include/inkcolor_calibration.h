#ifndef __INKCOLOR_CALIBRATION_H__
#define __INKCOLOR_CALIBRATION_H__

#include "ci_define.h"
#include "ci_declare.h"
#include "inkcolor_calibration_error.h"
#include "inkcolor_calibration_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	/************************************
   @Brief		：	创建墨色标定句柄.
   @Note		：
   @Parameter	: 	inkcolor_calib_handle -- 墨色标定句柄.
   @Returns		:   AlgoError
   @Author		：	mumeng@2023/9/19
   ************************************/
	C_API AlgoError C_CALL InkColorCalibCreatHandle(AlgoHandle* inkcolor_calib_handle);
	

	/************************************
   @Brief		：	设置标定色块的数量.
   @Note		：
   @Parameter	: 	inkcolor_calib_handle -- 墨色标定句柄.
   @Parameter	: 	in_color_num          -- 色块数量
   @Returns		:   AlgoError
   @Author		：	mumeng@2023/9/19
   ************************************/
	C_API AlgoError C_CALL InkColorCalibSetColorNum(AlgoHandle inkcolor_calib_handle, const int in_color_num);


	/************************************
   @Brief		：	输入标定信息
   @Note		：
   @Parameter	: 	inkcolor_calib_handle -- 墨色标定句柄.
   @Parameter	: 	in_img                -- 色块图像
   @Parameter	: 	in_corner             -- 图像的四角点坐标
   @Parameter	: 	in_point_num          -- 角点数量
   @Parameter	: 	in_ground_xyz         -- 色块标准XYZ值
   @Parameter	: 	in_color_idx          -- 色块编号索引，从 0 开始
   @Returns		:   AlgoError
   @Author		：	mumeng@2023/9/19
   ************************************/
	C_API AlgoError C_CALL InkColorCalibInputCamCalibInfo(AlgoHandle inkcolor_calib_handle, AlgoMat* in_img,
		const AlgoPoint* in_corner, const int in_point_num, const TriXYZ* in_ground_xyz, const int in_color_idx);


	/************************************
   @Brief		：	计算相机色度标定矩阵
   @Note		：
   @Parameter	: 	inkcolor_calib_handle -- 墨色标定句柄.
   @Parameter	: 	out_cam_clib          -- 输出相机色度标定矩阵
   @Returns		:   AlgoError
   @Author		：	mumeng@2023/9/21
   ************************************/
	C_API AlgoError C_CALL InkColorCalibCamCalibCalc(AlgoHandle inkcolor_calib_handle, AlgoMat* out_cam_clib);


	/************************************
   @Brief		：	设置标定板上每个色块的roi区域.
   @Note		：
   @Parameter	: 	inkcolor_calib_handle -- 墨色标定句柄.
   @Parameter	: 	in_img_wh             -- 图像宽高
   @Parameter	: 	in_corner             -- roi区域角点
   @Parameter	: 	in_point_num          -- 角点个数
   @Parameter	: 	in_color_idx          -- 色块mask索引号，从 0 开始
   @Returns		:   AlgoError
   @Author		：	mumeng@2023/9/20
   ************************************/
	C_API AlgoError C_CALL InkColorCalibSetMask(AlgoHandle inkcolor_calib_handle, const AlgoSize* in_img_wh, 
		const AlgoPoint* in_corner, const int in_point_num, const int in_color_idx);


	/************************************
   @Brief		：	计算设备标定矩阵
   @Note		：
   @Parameter	: 	inkcolor_calib_handle -- 墨色标定句柄.
   @Parameter	: 	in_img                -- 采集的色块图像
   @Parameter	: 	out_facility_calib    -- 输出的偏差标定矩阵
   @Parameter	: 	is_dev_rest           -- 是否进行设备重置， 1 是 ，  0 否。  默认是 0
   @Returns		:   AlgoError
   @Author		：	mumeng@2023/9/21
   ************************************/
	C_API AlgoError C_CALL InkColorCalibFacilityCalc(AlgoHandle inkcolor_calib_handle, AlgoMat* in_img, AlgoMat* out_facility_calib, int is_dev_rest = 0);
	
	
   /************************************
   @Brief		：	直接从外部设置相机色度标定矩阵
   @Note		：
   @Parameter	: 	inkcolor_calib_handle -- 墨色标定句柄.
   @Parameter	: 	in_calib_mat          -- 标定矩阵内容
   @Returns		:   AlgoError
   @Author		：	mumeng@2023/9/21
   ************************************/
	C_API AlgoError C_CALL InkColorCalibSetCamCalibMat(AlgoHandle inkcolor_calib_handle, AlgoMat* in_calib_mat);


	/************************************
   @Brief		：	获取相机色度标定矩阵
   @Note		：
   @Parameter	: 	inkcolor_calib_handle -- 墨色标定句柄.
   @Parameter	: 	in_calib_mat          -- 标定矩阵内容
   @Returns		:   AlgoError
   @Author		：	mumeng@2023/9/21
   ************************************/
	C_API AlgoError C_CALL InkColorCalibGetCamCalibMat(AlgoHandle inkcolor_calib_handle, AlgoMat* out_calib_mat);


	/************************************
   @Brief		：	获取设备偏差标定矩阵
   @Note		：
   @Parameter	: 	inkcolor_calib_handle -- 墨色标定句柄.
   @Parameter	: 	in_calib_mat          -- 标定矩阵内容
   @Returns		:   AlgoError
   @Author		：	mumeng@2023/9/21
   ************************************/
	C_API AlgoError C_CALL InkColorCalibGetFacilityCalibMat(AlgoHandle inkcolor_calib_handle, AlgoMat* out_calib_mat);


   /************************************
   @Brief		：	保存墨色标定句柄内容，包含 该状态下标定矩阵、色块mask、初始标准值.
   @Note		：
   @Parameter	: 	inkcolor_calib_handle -- 墨色标定句柄.
   @Parameter	: 	in_file_name          -- 有效的文件路径。
   @Returns		:   AlgoError
   @Author		：	mumeng@2023/9/19
   ************************************/
	C_API AlgoError C_CALL InkColorCalibSave(AlgoHandle inkcolor_calib_handle, const char* in_file_name);	


	 /************************************
   @Brief		：	读取墨色标定句柄内容，包含 该状态下标定矩阵、色块mask、初始标准值.
   @Note		：
   @Parameter	: 	inkcolor_calib_handle -- 墨色标定句柄.
   @Parameter	: 	in_file_name          -- 有效的文件路径。
   @Returns		:   AlgoError
   @Author		：	mumeng@2023/9/19
   ************************************/
	C_API AlgoError C_CALL InkColorCalibLoad(AlgoHandle inkcolor_calib_handle, const char* in_file_name);


	/************************************
	@Brief		：	计算墨色标定句柄内容 的大小
	@Note		：
	@Parameter	: 	pcb_inkcolor_handle			-- 墨色标定句柄.
	@Parameter	: 	out_len					    -- buffer的大小
	@Returns	:   AlgoError
	@Author		：	mumeng@2023/9/18
	************************************/
	C_API AlgoError C_CALL InkColorCalibGetBufferSize(AlgoHandle inkcolor_calib_handle, uint64_t* out_len);


	/************************************
	@brief		:   将句柄里面的数据参数导出到buffer
	@note		:   
	@Parameter	:   pcb_inkcolor_handle		-- 墨色标定句柄.
	@Parameter	:   out_buffer			    -- 已经申请好的空间
	@Parameter	:   out_len				    -- 外部申请好的空间大小,会用真实大小修改
	@return		:   AlgoError
	@Author		:   mumeng@2023/9/18
	************************************/
	C_API AlgoError C_CALL InkColorCalibToBuffer(AlgoHandle inkcolor_calib_handle, char* out_buffer, uint64_t* out_len);

	/************************************
	@brief		:   将buffer里面的数据导入到句柄
	@note		:   
	@Parameter	:   pcb_inkcolor_handle		-- 墨色标定句柄.
	@Parameter	:   in_buffer				    -- 已经申请好申请空间
	@Parameter	:   in_len				    -- 外部申请好的空间大小,会用真实大小修改
	@return		:   AlgoError
	@Author		:   mumeng@2023/9/18
	************************************/
	C_API AlgoError C_CALL InkColorCalibFromBuffer(AlgoHandle inkcolor_calib_handle, const char* in_buffer, uint64_t in_len);



	C_API AlgoError C_CALL InkColorCalibRelease(AlgoHandle inkcolor_calib_handle);
	C_API void C_CALL InkColorCalibDestroyHandle(AlgoHandle inkcolor_calib_handle);


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


#ifdef __cplusplus
}
#endif // __cplusplus
#endif
