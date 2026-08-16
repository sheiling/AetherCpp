#ifndef __MOBILE_CAMERA_CONTROLLER_HPP__
#define __MOBILE_CAMERA_CONTROLLER_HPP__

#include "ci_define.h"
#include "ci_declare.h"
#include "mobile_camera_controller_data.h"
#include "mobile_camera_controller_error.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	
	/************************************
	@brief  : 创建手机相机参数调节句柄
	@note   : 
	@param  : out_mobilecamerpar_handle      手机相机参数调节句柄
	@return	: AlgoError
	@Author : Zhangxuan@2023/2/7
	************************************/

	C_API AlgoError C_CALL MobileCamConCreateHandle(AlgoHandle* out_mobilecamerpar_handle);

	/************************************
	@brief  : 配置当前手机相机参数
	@note   :
	@param  : mobilecamerpar_handle      手机相机参数调节句柄；
	@param  : in_step                    当前迭代次数，初始为1；
	@param  : in_cameraWidth             手机jpg图像分辨率宽
	@param  : in_cameraHeight            手机jpg图像分辨率高             
	@param  : in_isomin                  手机ISO可调最小范围
	@param  : in_isomax                  手机ISO可调最大范围
	@param  : in_expomin                 手机曝光时间可调最小范围
	@param  : in_expomax                 手机曝光时间可调最大范围
	@param  : in_focmin                  手机微焦可调最小范围
	@param  : in_focmax                  手机微焦可调最大范围
	@param  : in_focstep                 手机微焦可调刻度  
	@param  : in_expotime                当前拍摄的JPG图像的曝光时间
	@param  : in_iso                     当前拍摄的JPG图像的ISO值
	@param  : in_foucs                   当前拍摄的JPG图像的微焦
	@param  : Grayscale                  当前打屏亮度(灰阶)
	@param  : focbefore                  上次拍摄的JPG图像的微焦(初次为0)
	@param  : Sbefore                    上次成像质量：灯点峰值(初次为0；0-1之间)
	@param  : Qbefore                    上次成像质量：灯点占比(初次为0；0-1之间)
	
	@return	: AlgoError
	@Author : Zhangxuan@2023/2/7
	************************************/

	C_API AlgoError C_CALL MobileCamConSetPara(AlgoHandle mobilecamerpar_handle, const int in_step, 
		const int in_cameraWidth, const int in_cameraHeight, const int in_isomin, const int in_isomax, const int in_expomin, const int in_expomax, 
		const float in_focmin, const float in_focmax, const float in_focstep, const int in_expotime, const int in_iso, const float in_foucs, 
		const int Grayscale, const float focbefore, const float Sbefore, const float Qbefore);


	/************************************
	@brief  : 暗环境下：设置JPG图像信息（用来生成JPG图像G通道的shot文件和区域定位文件）
	@note   :
	@param  : mobilecamerpar_handle      手机相机参数调节句柄；
	@param  : in_JPGfile                 JPG图像路径，注意格式为三通道的JPG图像；
	@param  : in_partWidth               当前分区的宽(注意是指隔点前的分区宽，而不是隔点之后的)
	@param  : in_partHeight              当前分区的高(注意是指隔点前的分区宽，而不是隔点之后的)
	@param  : shotJPG_out                输出的JPG图像shot文件(G通道)
	@param  : areaJPG_out                输出的JPG图像area文件(G通道)
	@param  : in_color                   当前打屏颜色，默认为绿色COLOR_GREEN，可不输入
	@param  : pattern_wh                 当前隔点大小，默认为3*3，可不输入
	@return	: AlgoError
	@Author : Zhangxuan@2023/2/7
	************************************/

	C_API AlgoError C_CALL MobileCamConSetImgDark(AlgoHandle mobilecamerpar_handle, const char* in_JPGfile, const int in_partWidth, const int in_partHeight, 
		const AlgoHandle shotJPG_out, const AlgoHandle areaJPG_out, const int in_color = 2, const AlgoSize in_pattern_wh = {5,5});
	/************************************
	@brief  : 亮环境下：设置JPG图像信息（用来生成JPG图像G通道的shot文件和区域定位文件）
	@note   :
	@param  : mobilecamerpar_handle      手机相机参数调节句柄；
	@param  : in_JPGfile                 JPG图像路径，注意格式为三通道的JPG图像；
	@param  : in_partWidth               当前分区的宽(注意是指隔点前的分区宽，而不是隔点之后的)
	@param  : in_partHeight              当前分区的高(注意是指隔点前的分区宽，而不是隔点之后的)
	@param  : shotJPG_out                输出的JPG图像shot文件(G通道)
	@param  : areaJPG_out                输出的JPG图像area文件(G通道)
	@param  : in_color                   当前打屏颜色，默认为绿色COLOR_GREEN，可不输入
	@param  : pattern_wh                 当前隔点大小，默认为3*3，可不输入
	@return	: AlgoError
	@Author : Zhangxuan@2023/2/7
	************************************/

	C_API AlgoError C_CALL MobileCamConSetImgBright(AlgoHandle mobilecamerpar_handle, const char* in_JPGfile, const int in_partWidth, const int in_partHeight,
		const AlgoHandle shotJPG_out, const AlgoHandle areaJPG_out, const int in_color = 2, const AlgoSize in_pattern_wh = { 5,5 });

	/************************************
	@brief  : 决策当前图像调节
	@note   :
	@param  : mobilecamerpar_handle      手机相机参数调节句柄；
	@param  : in_shotJPG                 JPG图像的shot文件(G通道)；通过SetImg可以得到；
	@param  : in_areaJPG                 in_shotJPG的area文件(G通道)；通过SetImg可以得到；
	@param  : S_out                      当前图像质量：灯点峰值；
	@param  : Q_out                      当前图像质量：灯点占比；
	@param  : step_out                   下次迭代次数
	@param  : camstate_out               当前成像状态 0：未成功，继续拍摄；1：成功，停止拍摄；2：屏体过亮，停止拍摄；3：屏体过暗，停止拍摄
	@param  : expotime_out               下次拍摄推荐曝光时间
	@param  : iso_out                    下次拍摄推荐ISO
	@param  : fou_out                    下次拍摄推荐微焦
	@param  : gray_out                   下次推荐打屏亮度（如果camState=2,3会改变，0，1时不改变）
	@return	: AlgoError
	@Author : Zhangxuan@2023/2/7
	************************************/
	C_API AlgoError C_CALL MobileCamConAnalyze(AlgoHandle mobilecamerpar_handle, const AlgoHandle in_shotJPG, const AlgoHandle in_areaJPG, float &S_out, float &Q_out, int &step_out, int &camstate_out, int &expotime_out,
		int &iso_out, float &fou_out, int &gray_out);

	/************************************
	@Brief		：	设置成像质量区间
	@Note		：
	@Parameter	: 	S_max_in                --灯点峰值上限
	@Parameter	: 	S_min_in                --灯点峰值下限
	@Parameter	: 	Q_max_in                --灯点大小上限
	@Parameter	: 	Q_min_in                --灯点大小上限
	@Returns	:   AlgoError
	@Author		：	Zhangxuan@2023/2/7
	************************************/
	C_API AlgoError C_CALL MobileCamConSetQualityRange(AlgoHandle mobilecamerpar_handle, const float in_S_max, const float in_S_min, const float in_Q_max, const float in_Q_min);
	





//*****************************************************************************************************************************************************
// 手机亮度修缝相关函数见下：

	/************************************
	@Brief		：	手机亮度修缝-设置推荐的成像质量区间
	@Note		：
	@Parameter	:   mobilecamerpar_handle	   手机相机参数调节句柄；
	@Parameter	: 	max_sharpness_in		   灯点锐度上限
	@Parameter	: 	min_sharpness_in           灯点锐度下限
	@Parameter	: 	max_expo_val_in            曝光峰值上限
	@Parameter	: 	min_expo_val_in            曝光峰值下限
	@Returns	:   AlgoError
	@Author		：	GuoHao@2023/12/11
	************************************/
	C_API AlgoError C_CALL MobileCamConSetQualityRangeForGrid(AlgoHandle mobilecamerpar_handle, const float max_sharpness_in, const float min_sharpness_in, const float max_expo_val_in, const float min_expo_val_in);
	

	/************************************
	@brief		:	配置当前手机相机参数
	@note		:
	@param		:	mobilecamerpar_handle      手机相机参数调节句柄；
	@param		:	in_step                    当前迭代次数，初始为1；
	@param		:	in_cameraWidth             手机jpg图像分辨率宽
	@param		:	in_cameraHeight            手机jpg图像分辨率高
	@param		:	in_isomin                  手机ISO可调最小值
	@param		:	in_isomax                  手机ISO可调最大值
	@param		:	in_expomin                 手机曝光时间可调最小值
	@param		:	in_expomax                 手机曝光时间可调最大值
	@param		:	in_focmin                  手机微焦可调最小值
	@param		:	in_focmax                  手机微焦可调最大值
	@param		:	in_focstep                 手机微焦可调步长
	@param		:	in_expotime                当前拍摄的JPG图像的曝光时间
	@param		:	in_iso                     当前拍摄的JPG图像的ISO值
	@param		:	in_foucs                   当前拍摄的JPG图像的微焦
	@param		:	in_Grayscale               当前打屏亮度(灰阶)
	@param		:	focbefore                  上次拍摄的JPG图像的微焦(初次为0)
	@param		:	expoval_before             上次成像质量：曝光峰值(初次为0；0-1之间)
	@param		:	sharpness_before           上次成像质量：屏体锐度(初次为1；0-1之间)

	@return		:	AlgoError
	@Author		:	GuoHao@2023/12/11
	************************************/
	C_API AlgoError C_CALL MobileCamConSetParaForGrid(AlgoHandle mobilecamerpar_handle, const int in_step,
		const int in_cameraWidth, const int in_cameraHeight, const int in_isomin, const int in_isomax, const int in_expomin, const int in_expomax,
		const float in_focmin, const float in_focmax, const float in_focstep, const int in_expotime, const int in_iso, const float in_foucs,
		const int in_Grayscale, const float focbefore, const float expoval_before, const float sharpness_before);


	/************************************
	@brief		:   手机亮度修缝-决策当前图像调节
	@note		:
	@param		:	mobilecamerpar_handle      手机相机参数调节句柄；
	@param		:	pt_sharpness_in            屏体锐度；
	@param		:	pt_expo_val_in             屏体曝光度；
	@param		:	step_out                   下次迭代次数
	@param		:	camstate_out               当前成像状态 0：未成功，继续拍摄；1：成功，停止拍摄；2：屏体过亮，停止拍摄；3：屏体过暗，停止拍摄；4：停止拍摄，请调节相机；
	@param		:	expotime_out               下次拍摄推荐曝光时间
	@param		:	iso_out                    下次拍摄推荐ISO
	@param		:	fou_out                    下次拍摄推荐微焦
	@param		:	gray_out                   下次推荐打屏亮度（如果camState=2,3会改变，0，1时不改变）
	@return		:	AlgoError
	@Author		:	GuoHao@2023/12/11
	************************************/
	C_API AlgoError C_CALL MobileCamConAnalyzeGrid(AlgoHandle mobilecamerpar_handle, const float pt_sharpness_in, const float pt_expo_val_in,
		int &step_out, int &camstate_out, int &expotime_out, int &iso_out, float &fou_out, int &gray_out);

// 手机亮度修缝相关函数见上：
//*****************************************************************************************************************************************************


//*****************************************************************************************************************************************************
// 手机亮度修缝3.0相关J接口见下（该接口APP实现，不直接调用）：

	/************************************
	@brief		:   手机亮度修缝3.0-AE
	@note		:
	@param		:	mobilecamerpar_handle      手机相机参数调节句柄；
	@param		:	in_cur_expo_val            当前曝光值；
	@param		:	in_aim_expo_val            目标曝光度；
	@param		:	in_iso					   当前ISO
	@param		:	in_gray					   当前屏体灰阶；
	@param		:	in_expo_time               当前曝光时间
	@param		:	out_iso                    下次拍摄推荐ISO
	@param		:	out_gray                   下次拍摄推荐打屏灰阶
	@param		:	out_expo_time              下次拍摄推荐曝光时间
	@return		:	AlgoError
	@Author		:	GuoHao@2024/06/04
	************************************/
	C_API AlgoError C_CALL MobileCamConCameraAE(AlgoHandle mobilecamerpar_handle, const float in_cur_expo_val, const float in_aim_expo_val, const int in_iso, const int in_gray, 
		const float in_expo_time, int* out_iso, int* out_gray, float* out_expo_time);


	/************************************
	@brief		:   手机亮度修缝3.0-AF
	@note		:
	@param		:	mobilecamerpar_handle      手机相机参数调节句柄
	@param		:	in_camera_type             相机类型，ANDROID/IOS, 安卓或苹果
	@param		:	in_foc_history             微焦历史信息
	@param		:	in_sharp_history		   锐度历史信息
	@param		:	in_history_lens            焦距、锐度历史数组元素数量
	@param		:	in_sharp_range_max		   目标锐度区间上限
	@param		:	in_sharp_range_min         目标锐度区间下限
	@param		:	in_foc_step                Foc搜索步长
	@param		:	out_foc                    下次拍摄推荐微焦
	@param		:	out_state				   微焦搜索状态，SUCCESS/TOOSHARPLY/BINARYSEARCH, 成功/仍单调搜索/二分搜索
	@return		:	AlgoError
	@Author		:	GuoHao@2024/06/04
	************************************/
	C_API AlgoError C_CALL MobileCamConCameraAF(AlgoHandle mobilecamerpar_handle, MobileCameraType in_camera_type, float* in_foc_history, float* in_sharp_history, int in_history_lens,
		float in_sharp_range_max, float in_sharp_range_min, float in_foc_step, float* out_foc, AFState* out_state);




//*****************************************************************************************************************************************************




	/************************************
	@Brief		：	设置相机参数调节区间
	@Note		：
	@Parameter	: 	in_iso_min                --ISO上限
	@Parameter	: 	in_iso_max                --ISO下限
	@Parameter	: 	in_expotime_min           --曝光时间上限
	@Parameter	: 	in_expotime_max           --曝光时间下限
	@Parameter	: 	in_foc_min                --微焦上限
	@Parameter	: 	in_foc_max                --微焦下限
	@Returns	:   AlgoError
	@Author		：	Zhangxuan@2023/2/7
	************************************/
	C_API AlgoError C_CALL MobileCamConSetCameraParaRange(AlgoHandle mobilecamerpar_handle, const int in_iso_min, const int in_iso_max, const int in_expotime_min,
		const int in_expotime_max, const float in_foc_min, const float in_foc_max);
	
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
	C_API void C_CALL  MobileCamConDestroyHandle(AlgoHandle mobilecamerpar_handle);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif