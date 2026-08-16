#ifndef __DISTORTION_CORRECTION_H__
#define __DISTORTION_CORRECTION_H__

#include <stdint.h>
#include "ci_declare.h"
#include "ci_define.h"
#include "distortion_correction_data.h"
#include "distortion_correction_errors.h"
#include "algo_mat.h"
#include <vector>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	/************************************
	 @brief  : 初始化句柄
	 @note   :
	 @param  :
	 @return : AlgoHandle
	 @Author : zhoujun@2025/04/08
	 ************************************/
	C_API AlgoError C_CALL DistortionCreateHandle(AlgoHandle *correct_handle);


	/************************************
	 @brief  : set棋盘格的信息
	 @note   :
	 @param  : in_chessboard_coenerpoint_num_rows             --棋盘格角点横的数量
	 @param  : in_chessboard_coenerpoint_num_cols		   	  --棋盘格角点竖的数量
	 @param  : in_square_size				                  --棋盘格的物理间隔大小
	 @return : AlgoError
	 @Author : zhoujun@2025/04/08
	 ************************************/
	C_API AlgoError C_CALL DistortionSetChessBoardInfo(AlgoHandle correct_handle, const int in_chessboard_coenerpoint_num_rows, const int in_chessboard_coenerpoint_num_cols, const double in_square_size);


	/************************************
	 @brief  : set棋盘格的信息
	 @note   :
	 @param  : out_chessboard_coenerpoint_num_rows             --棋盘格角点横的数量
	 @param  : out_chessboard_coenerpoint_num_cols		   	   --棋盘格角点竖的数量
	 @param  : out_square_size				                   --棋盘格的物理间隔大小
	 @return : AlgoError
	 @Author : zhoujun@2025/04/08
	 ************************************/
	C_API AlgoError C_CALL DistortionGetChessBoardInfo(AlgoHandle correct_handle, int* out_chessboard_coenerpoint_num_rows, int* out_chessboard_coenerpoint_num_cols, double* out_square_size);
	
	
	/************************************
	 @brief  : 输入图像，并对图像进行寻找角点
	 @note   :
	 @param  : correct_handle
	 @param  : in_img						 --输入的图像指针
	 @param  : in_num						 --图像个数
	 @return : AlgoError
	 @Author : zhoujun@2025/04/08
	 ************************************/
	C_API AlgoError C_CALL DistortionSetInputFromMat(AlgoHandle correct_handle, AlgoMat* in_img, int in_num);


	/************************************
	 @brief  : 输入图像，并对图像进行寻找角点
	 @note   :
	 @param  : correct_handle
	 @param  : in_shot						 --输入的shot指针
	 @param  : in_num						 --图像个数
	 @return : AlgoError
	 @Author : zhoujun@2025/04/08
	 ************************************/
	C_API AlgoError C_CALL DistortionSetInput(AlgoHandle correct_handle, AlgoHandle *in_shot, int in_num);


	/************************************
	 @brief  : 计算矫正系数
	 @note   :
	 @param  : correct_handle
	 @return : AlgoError
	 @Author : zhoujun@2025/04/08
	 ************************************/
	C_API AlgoError C_CALL DistortionCompute(AlgoHandle correct_handle);


	/************************************
	 @brief  : 畸变矫正
	 @note   :
	 @param  : correct_handle
	 @param  : in_out_shot			 --待矫正的shot
	 @return : AlgoError
	 @Author : zhoujun@2025/04/08
	 ************************************/
	C_API AlgoError C_CALL DistortionRectify(AlgoHandle correct_handle, AlgoHandle in_shot);


	/************************************
	 @brief  : 设置内参、矫正系数
	 @note   :
	 @param  : correct_handle
	 @param  : in_mtx       --输入的矫正信息
	 @return : AlgoError
	 @Author : zhoujun@2025/04/08
	 ************************************/
	C_API AlgoError C_CALL DistortionSet(AlgoHandle correct_handle, AlgoDistortion *in_dist);


	/************************************
	 @brief  : 获取内参、矫正系数
	 @note   :
	 @param  : correct_handle
	 @param  : out_dist       --输出的矫正信息
	 @return : AlgoError
	 @Author : zhoujun@2025/04/08
	 ************************************/
	C_API AlgoError C_CALL  DistortionGet(AlgoHandle correct_handle, AlgoDistortion *out_dist);


	/************************************
	 @brief  : 保存内参、矫正系数
	 @note   :
	 @param  : correct_handle
	 @param  : out_chromacalib_fullname		--标定文件地址
	 @return : AlgoError
	 @Author : zhoujun@2025/04/08
	 ************************************/
	C_API AlgoError C_CALL  DistortionSave(AlgoHandle correct_handle, const char* out_chromacalib_fullname);

	//文件后缀.distortion

	/************************************
	 @brief  : 读取内参、矫正系数
	 @note   :
	 @param  : correct_handle
	 @param  : out_chromacalib_fullname		--标定文件地址
	 @return : AlgoError
	 @Author : zhoujun@2025/04/08
	 ************************************/
	C_API AlgoError C_CALL  DistortionLoad(AlgoHandle correct_handle, const char* in_chromacalib_fullname);




	/************************************
	 @brief  : 获取buffer大小
	 @note   :
	 @param  : correct_handle			--当前有效句柄
	 @param  : out_len					--buffer大小
	 @return : AlgoError
	 @Author : zhoujun@2025/04/08
	 ************************************/
	C_API AlgoError C_CALL DistortionGetBufferSize(AlgoHandle correct_handle, uint64_t* out_len);


	/************************************
	@brief  : 将数据导出到buffer
	@note   :
	@param  : correct_handle				--句柄
	@param  : out_buffer				--已经申请好申请空间
	@param  : in_out_len				--外部申请好的空间大小,会用真实大小修改
	@return	: AlgoError
	@Author : zhoujun@2025/04/08
	************************************/
	C_API AlgoError C_CALL DistortionToBuffer(AlgoHandle correct_handle, char* out_buffer, uint64_t* in_out_len);


	/************************************
	@brief  : 将buffer数据导入
	@note   :
	@param  : correct_handle   
	@param  : in_buffer
	@param  : in_len
	@return	: AlgoError
	@Author : zhoujun@2025/04/08
	************************************/
	C_API AlgoError C_CALL DistortionFromBuffer(AlgoHandle correct_handle, const char* in_buffer, uint64_t in_len);


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
	C_API AlgoError C_CALL DistortionRelease(AlgoHandle correct_handle);
	C_API void C_CALL DistortionDestroyHandle(AlgoHandle correct_handle);


#ifdef __cplusplus
}
#endif // __cplusplus
#endif