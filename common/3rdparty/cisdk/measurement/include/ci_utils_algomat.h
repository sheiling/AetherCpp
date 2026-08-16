#ifndef __CI_UTILS_ALGOMAT_H__
#define __CI_UTILS_ALGOMAT_H__

#include "ci_measurement_enum.h"
#include "ci_declare.h"
#include "ci_define.h"
#include "ci_utils_errors.h"
#include "algo_mat.h"
#include "ci_data.h"
#include "meta_info_data.h"


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	/************************************
	@brief    : 载入.amat或者.mbin文件到AlgoMat
	@note     : AlgoMat 需要提前create或者new, 详见algo_mat.h
	@param    : out_algomat     需要提前create或者new, 详见algo_mat.h
	@param    : in_amat_file    .amat或者.mbin文件
	@return   : AlgoError
	@Modified : chengjiayi@2022/11/04
	************************************/
	C_API AlgoError C_CALL AlgoMatLoad(AlgoMat * out_algomat, const char * in_amat_file);

	/************************************
	@brief    : 保存AlgoMat到.amat文件
	@note     : 后缀只能是.amat
	@param    : in_algomat      有效的algomat
	@param    : in_amat_file    后缀只能是.amat
	@return   : AlgoError
	@Modified : chengjiayi@2022/11/04
	************************************/
	C_API AlgoError C_CALL AlgoMatSave(AlgoMat * in_algomat, const char * in_amat_file);


	/************************************
	@brief  : 获取buffer大小
	@note   :
	@param  : in_algomat	获取buffer size的对象
	@param  : out_len       buffer大小
	@return	: AlgoError
	@Author : tianbo@2025/04/01
************************************/
	C_API AlgoError C_CALL AlgoMatGetBufferSize(AlgoMat * in_algomat, uint64_t* out_len);

	/************************************
	@brief  : 将buffer数据导入到meter
	@note   :
	@param  : in_algomat	待转换为buffer的对象
	@param  : in_buffer     获取的buffer对象
	@param  : in_len        
	@return	: AlgoError
	@Author : tianbo@2025/04/01
************************************/
	C_API AlgoError C_CALL AlgoMatToBuffer(AlgoMat * in_algomat, char* out_buffer, uint64_t* in_out_len);


	/************************************
	@brief  : 将buffer数据导入到AlgoMat
	@note   : 深拷贝
	@param  : in_algomat	有效的algomat结构
	@param  : in_buffer
	@param  : in_len
	@return	: AlgoError
	@Author : tianbo@2025/04/01
	************************************/
	C_API AlgoError C_CALL AlgoMatFromBuffer(AlgoMat * in_algomat, const char* in_buffer, uint64_t in_len);


	/************************************
	@brief    : 读取图像到AlgoMat
	@note     : 1. 支持的图像bmp,dib,jpg,jpeg,jpg,jpe,png,pbm,pgm,ppm,tiff,tif
				2. 通道顺序同opencv, bgr
	@param    : out_algomat     需要提前create或者new, 详见algo_mat.h
	@param    : in_cvimg_file   文件名
	@param    : flags           同opencv:
								IMREAD_UNCHANGED    = -1,//If set, return the loaded image as is (with alpha channel, otherwise it gets cropped). Ignore EXIF orientation.
								IMREAD_GRAYSCALE    = 0,//If set, always convert image to the single channel grayscale image (codec internal conversion).
								IMREAD_COLOR        = 1,//If set, always convert image to the 3 channel BGR color image.
								IMREAD_ANYDEPTH     = 2,//If set, return 16-bit/32-bit image when the input has the corresponding depth, otherwise convert it to 8-bit.
								IMREAD_ANYCOLOR     = 4,//If set, the image is read in any possible color format.
								其他见opencv imgcodes.hpp中imread说明
	@return   : AlgoError
	@Modified : chengjiayi@2022/11/04
	************************************/
	C_API AlgoError C_CALL AlgoMatReadCvImg(AlgoMat * out_algomat, const char * in_cvimg_file, int flags = 1);

	/************************************
	@brief    : 导出AlgoMat到png图像
	@note     : 主要是给人看, 会转成8位图, 有损失
	@param    : in_algomat  有效的algomat
	@param    : in_png_file 文件名
	@param    : is_bgr   传入图像数据顺序是否为bgr,是为1，否为0（即为rgb顺序），默认为0
	@return   : AlgoError
	@Modified : wushaoshuang@2023/07/27
	************************************/
	C_API AlgoError C_CALL AlgoMatExportPNG(const AlgoMat * in_algomat, const char * in_png_file, int is_bgr = 0);

	/************************************
	@brief	  ：	读取dng格式,只支持16U
	@note	  ：
	@param    : out_algomat         --需要提前create或者new, 详见algo_mat.h
	@Param    : in_dng_file		    --输入的dng文件全名
	@Param    : in_cie_type	        --颜色分量类型 dng debayer后,保留选择的通道
	@param    : in_is_debayer       --是否进行debayer操作，0=不进行debayer；1=默认，进行debayer
	@return   : AlgoError
	@Modified ：	lipengpeng@2022/12/22
	************************************/
	C_API AlgoError C_CALL AlgoMatReadDng(AlgoMat * out_algomat, const char* in_dng_file, CIEType in_cie_type,
		int in_is_debayer = 1);

	/************************************
	@brief    : 对AlgoMat图像翻转
	@note     :
	@param    : out_algomat         --需要提前create或者new, 详见algo_mat.h
	@param    : in_flip_code        --in_flip_code>0,沿y轴翻转;in_flip_code==0,沿x轴翻转;in_flip_code<0,同时翻转
	@param    : in_algomat          --有效的algomat
	@return	  : AlgoError
	@Modified :	lipengpeng@2022/12/26
	************************************/
	C_API AlgoError C_CALL AlgoMatFlip(AlgoMat * out_algomat, int in_flip_code, const AlgoMat* in_algomat);


	/************************************
	@brief    : 对AlgoMat图像进行旋转
	@note     :	
	@param    : out_algomat         --需要提前create或者new, 详见algo_mat.h
	@param    : in_flip_code        --in_flip_code>0,沿y轴翻转;in_flip_code==0,沿x轴翻转;in_flip_code<0,同时翻转
	@param    : in_algomat          --有效的algomat
	@return	  : AlgoError
	@Modified :	GuoHao@2025/11/17
	************************************/
	C_API AlgoError C_CALL AlgoMatRotate(AlgoMat * out_algomat, RotateAngle in_rotation_angle, const AlgoMat* in_algomat);


	/************************************
	@brief    : 获取指定RECT区域的图像
	@note     :
	@param    : in_algomat          --输入的图像
	@param    : in_rect             --目标rect， 包含起点坐标 x y 和区域宽高 with height
	@param    : out_algomat_roi     --输出的ROI区域图像
	@return	  : AlgoError
	@Modified :	mumeng@2022/12/30
	************************************/
	C_API AlgoError C_CALL AlgoMatGetRect(AlgoMat * in_algomat, AlgoRect* in_rect, AlgoMat * out_algomat_roi);



	/************************************
	@brief    : 获取AlgoMat的均值
	@note     : 可选获取rect处的均值
	@param    : in_algomat          --输入的图像
	@param    : out_mean            --输出的均值
	@param    : in_rect				--ROI区域，可选
	@return	  : AlgoError
	@Modified :	tianbo@2024/11/15
	************************************/
	C_API AlgoError C_CALL AlgoMatGetMean(AlgoMat* in_algomat, double* out_mean, AlgoRect* in_rect = nullptr);

	/************************************
	@brief    : 获取AlgoMat的和
	@note     : 可选获取rect处的和
	@param    : in_algomat          --输入的图像
	@param    : out_sum            --输出的均值
	@param    : in_rect				--ROI区域，可选
	@return	  : AlgoError
	@Modified :	GuoHao@2025/11/17
	************************************/
	C_API AlgoError C_CALL AlgoMatGetSum(AlgoMat* in_algomat, double* out_sum, AlgoRect* in_rect = nullptr);


	/************************************
	@brief	  ：	读取Raw数据，debyer时使用
	@note	  ：
	@param    : out_algomat         --输出debyer后的algomat
	@param    : in_algomat          --输入有效debyer前的algomat
	@param    : in_bayer_type       --bayer类型
	@param    : in_cie_type         --颜色分量类型，支持CAM_RGB、CAM_R、CAM_G、CAM_B，默认CAM_RGB
	@return   : AlgoError
	@Modified ：	lipengpeng@2023/01/10
	************************************/
	C_API AlgoError C_CALL AlgoMatImageDebyer(AlgoMat * out_algomat, const AlgoMat * in_algomat,
		BayerType in_bayer_type, CIEType in_cie_type = CAM_RGB);


	/************************************
	@brief	  ：	图像数据编码成数据流
	@note	  ：
	@param    : out_stream         --数据流输出: out_stream是外面申请的，以in_img的size大小申请
	@param    : out_real_size      --数据流真实的size大小
	@param    : in_img             --输入图像
	@param    : in_img_type        --图像类型：bmp,dib,jpg,jpeg,jpg,jpe,png,pbm,pgm,ppm,tiff,tif
	@return   : AlgoError
	@Modified ：	lipengpeng@2023/02/23
	************************************/
	C_API AlgoError C_CALL AlgoMatEncode(char* out_stream, size_t* out_real_size, const AlgoMat* in_img, const char* in_img_type);


	/************************************
	@brief	  ：	解码内存数据流，变成图像
	@note	  ：
	@param    : out_img            --输出AlgoMat图像
	@param    : in_stream          --数据流载入
	@param    : in_real_size       --外部申请的，以img图像size大小申请
	@return   : AlgoError
	@Modified ：	lipengpeng@2023/02/23
	************************************/
	C_API AlgoError C_CALL  AlgoMatDecode(AlgoMat * out_img, char * in_stream, size_t * in_real_size);

	/************************************
	@brief	  ：	单通道AlgoMat合并为三通道的AlgoMat
	@note	  ：输入必须为三个单通道的AlgoMat,输入若为多通道AlgoMat，内部会强制转换为单通道
	@param    : in_algomat_1       --第一个单通道AlgoMat
	@param    : in_algomat_2       --第二个单通道AlgoMat
	@param    : in_algomat_3       --第三个单通道AlgoMat
	@param    : out_algomat        --输出三通道的AlgoMat
	@return   : AlgoError
	@Modified ：	wushaoshuang@2023/06/12
	************************************/
	C_API AlgoError C_CALL AlgoMatMerge(AlgoMat * in_algomat_1, AlgoMat * in_algomat_2, AlgoMat * in_algomat_3, AlgoMat* out_algomat);

	/************************************
	@brief	  ：三通道的AlgoMat拆分为三个单通道的AlgoMat
	@note	  ：输入必须为一个三通道的AlgoMat,输出为三个单通道的AlgoMat
	@param    : in_algomat         --输入的AlgoMat
	@param    : out_algomat_1      --输出的第一个通道AlgoMat
	@param    : out_algomat_2      --输出的第二个通道AlgoMat
	@param    : out_algomat_3      --输出的第三个通道AlgoMat
	@return   : AlgoError
	@Modified ：	songshaojun@2023/09/20
	************************************/
	C_API AlgoError C_CALL AlgoMatSplit(AlgoMat * in_algomat, AlgoMat * out_algomat_1, AlgoMat * out_algomat_2, AlgoMat* out_algomat_3);



	/************************************
	@brief  : algo_mat的堆叠。堆叠方向的尺寸必须一致
	@note   : 注意：这里的out_mat内部new，必须自己手动释放。
	@param  : in_src	输入的algomat数组
	@param  : in_num	algomat数组长度
	@return	: out_mat	输出的mat
	@return	: derection 堆叠的方向，0为横向堆叠，1为纵向堆叠
	@Author : tianbo@2025/06/22
	************************************/
	C_API AlgoError C_CALL AlgoMatContact(const AlgoMat* in_src, int in_num, AlgoMat* out_mat, int derection = 0);


	///************************************
	//@brief    : 读取raw图到AlgoMat
	//@note     : raw数据必须是
	//			1 纯像素数据,不包含其他数据
	//			2 rowmajor
	//			3 每个像素位深uint16 或者 uint8
	//@param    : out_algomat         --需要提前create或者new, 详见algo_mat.h
	//@Param    : in_raw_file		    --输入的raw文件全名
	//@param    : in_cie_type         --颜色分量类型, 详见ShotSetImage
	//@param    : in_img_wh           --相机图像分辨率
	//@param    : in_bayer_type       --bayer类型, 详见ShotSetImage
	//@param    : in_cam_bit_depth    --图像位深，可选择读取16位图像 和 8位图像，默认为16
	//@return   : AlgoError
	//@Modified : lipengpeng@2022/12/22
	//************************************/
	//C_API AlgoError C_CALL AlgoMatReadRaw(AlgoMat * out_algomat, const char * in_raw_file, CIEType in_cie_type,
	//	AlgoSize in_img_wh, BayerType in_bayer_type, int in_cam_bit_depth = 16);



#ifdef __cplusplus
}

#endif // __cplusplus
#endif