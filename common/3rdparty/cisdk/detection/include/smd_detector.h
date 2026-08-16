#ifndef __SMD_DETECTOR_H__
#define __SMD_DETECTOR_H__

#include <stdint.h>
#include "ci_declare.h"
#include "ci_define.h"
#include "smd_detector_data.h"
#include "smd_detector_errors.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	
	/************************************
	@brief    : 创建句柄
	@note     :
	@return   :
	@Modified : haopanxia@2023/01/09
	************************************/
	C_API AlgoHandle C_CALL SMDCreateHandle();



	/************************************
	@Brief		：	设置smd检测方法信息，可选方式：模板匹配，传统方式
	@Note		：
	@Parameter	: 	smd_handle       -- SMD检测句柄
	@Parameter	: 	in_flag	     	 -- 输入选择的检测方法
	                                   MODEL_METHOD     //基于模板检测方法
		                               TRADITION_METHOD //基于传统检测方法
	@Returns	:   AlgoError
	@Author		：	haopanxia@2023/12/03
	************************************/
	C_API AlgoError C_CALL SMDSetDetectMethodFlag(AlgoHandle smd_handle, DetectMethodFlag in_flag);



	/************************************
	@brief    : 设置模板
	@note     :
	@param    : smd_handle          有效的smd句柄
	@param    : model_handle        model句柄
	@param    : in_template_name    模板名字
	@return   : AlgoError
	@Modified : haopanxia@2023/12/03
	************************************/
	C_API AlgoError C_CALL SMDSetTemplate(AlgoHandle smd_handle, AlgoHandle model_handle, const char* in_template_name);



	/************************************
	@Brief		：	设置smd检测类型信息
	@Note		：
	@Parameter	: 	smd_handle       -- SMD检测句柄
	@Parameter	: 	in_param		 -- 输入的相关参数信息
	@Returns	:   AlgoError
	@Author		：	haopanxia@2023/01/09
	************************************/
	C_API AlgoError C_CALL SMDSetPramaInfo(AlgoHandle smd_handle, SMDParam* in_param);


	/************************************
	@Brief		：	设置smd检测阈值信息
	@Note		：
	@Parameter	: 	smd_handle       -- SMD检测句柄
	@Parameter	: 	in_th		     -- 输入的检测阈值
	@Returns	:   AlgoError
	@Author		：	haopanxia@2023/01/09
	************************************/
	C_API AlgoError C_CALL SMDSetThresh(AlgoHandle smd_handle, SMDThresh* in_th);


	/************************************
	@Brief		：	获取smd检测阈值信息
	@Note		：
	@Parameter	: 	smd_handle       -- SMD检测句柄
	@Parameter	: 	out_th		     -- 检测阈值信息
	@Returns	:   AlgoError
	@Author		：	haopanxia@2023/01/09
	************************************/
	C_API AlgoError C_CALL SMDGetThresh(AlgoHandle smd_handle, SMDThresh* out_th);




	/************************************
	@Brief		：	设置确认灯点/焊盘定位的中心获取方式，是单焊盘还是芯片组，该接口只适用于裸板COB检测
	@Note		：
	@Parameter	: 	smd_handle       -- SMD检测句柄
	@Parameter	: 	in_mode		     -- 检测阈值信息，当in_mode设置为0时，则是每个焊盘中心都检出，当in_mode设置未1时，代表将一个RGB焊盘组中心检出
	@Returns	:   AlgoError
	@Author		：	haopanxia@2024/08/29
	************************************/
	C_API AlgoError C_CALL SMDSetPadCenterLocateMode(AlgoHandle smd_handle, int* in_mode);







	/************************************
	@Brief		：	SMD灯珠定位
	@Note		：
	@Parameter	: 	smd_handle            -- SMD检测句柄
	@Parameter	: 	in_img		          -- 输入的待检测图像
	@Parameter	: 	out_led_num  		  -- 输出灯点个数
	@Parameter	: 	out_led_center		  -- 输出的每个灯点在图像中的中心坐标,out_led_center = nullptr时，不输出灯点坐标信息
	@Returns	:   AlgoError
	@Author		：	haopanxia@2023/01/09
	************************************/
	C_API AlgoError C_CALL SMDLocate(AlgoHandle smd_handle, const AlgoMat* in_img, int* out_led_num, AlgoMat* out_led_center = nullptr);


	/************************************
	@Brief		：	灯珠角度检测
	@Note		：
	@Parameter	: 	smd_handle                -- SMD检测句柄
	@Parameter	: 	out_angle_id_row		  -- 行方向，超出偏移范围的灯点编号，（第几列，第几行）
	@Parameter	: 	out_angle_row		      -- 行方向，灯点的实际偏移角度
	@Parameter	: 	out_num_row		          -- 行方向，超出检测阈值的灯点个数
	@Returns	:   AlgoError
	@Author		：	haopanxia@2023/09/11
	************************************/
	C_API AlgoError C_CALL SMDAngleDetect(AlgoHandle smd_handle,
		 AlgoPoint* out_angle_id_row, float* out_angle_row, int* out_num_row);


	/************************************
	@Brief		：	直线度偏移量检测
	@Note		：
	@Parameter	: 	smd_handle                -- SMD检测句柄
	@Parameter	: 	out_offest_id_row		  -- 行方向，超出偏移范围的灯点编号，（第几列，第几行）
	@Parameter	: 	out_offest_row		      -- 行方向，灯点的实际直线度偏移量
	@Parameter	: 	out_num_row		          -- 行方向，超出检测阈值的灯点个数
	@Parameter	: 	out_offest_id_col		  -- 列方向，超出偏移范围的灯点编号，（第几列，第几行）
	@Parameter	: 	out_offest_row		      -- 行方向，灯点的实际直线度偏移量
	@Parameter	: 	out_num_col		          -- 列方向，超出检测阈值的灯点个数
	@Returns	:   AlgoError
	@Author		：	haopanxia@2023/01/09
	************************************/
	C_API AlgoError C_CALL SMDLineOffestDetect(AlgoHandle smd_handle,
		 AlgoPoint* out_offset_id_row, float* out_offset_row, int* out_num_row,
		 AlgoPoint* out_offset_id_col, float* out_offset_col, int* out_num_col);



	/************************************
	@Brief		：	灯点间隔偏移量检测
	@Note		：
	@Parameter	: 	smd_handle                -- SMD检测句柄
	@Parameter	: 	out_gap_id_row		      -- 行方向，超出偏移范围的灯点编号，（第几列，第几行）
	@Parameter	: 	out_gap_value_row		  -- 行方向，灯点实际的间隔距离
	@Parameter	: 	out_num_row		          -- 行方向，超出检测阈值的灯点个数
	@Parameter	: 	out_gap_id_col		      -- 列方向，超出偏移范围的灯点编号，（第几列，第几行）
	@Parameter	: 	out_gap_value_col	      -- 列方向，灯点实际的间隔距离
	@Parameter	: 	out_num_col		          -- 列方向，超出检测阈值的灯点个数
	@Returns	:   AlgoError
	@Author		：	haopanxia@2023/01/09
	************************************/
	C_API AlgoError C_CALL SMDGapDetect(AlgoHandle smd_handle,
		 AlgoPoint* out_gap_id_row, float* out_gap_value_row, int* out_num_row,
		 AlgoPoint* out_gap_id_col, float* out_gap_value_col, int* out_num_col);

	/************************************
	@brief    : 输出直线度检测结果图
	@note     : SMDLineOffestDetect之后，可调用此接口看结果
	@param    : smd_handle               -- 有效的SMD句柄
	@param    : out_result_filename      -- 检测图像保存路径
	@return   : AlgoError
	@Modified : haopanxia@2023/01/09
	************************************/
	C_API AlgoError C_CALL SMDExportLineResultPng(AlgoHandle smd_handle, const char *out_result_filename);

	/************************************
	@brief    : 输出直线度+角度检测结果图
	@note     : SMDAngleDetect 和 SMDLineOffestDetect之后，可调用此接口看结果
	@param    : smd_handle               -- 有效的SMD句柄
	@param    : out_result_filename      -- 检测图像保存路径
	@return   : AlgoError
	@Modified : haopanxia@2023/01/09
	************************************/
	C_API AlgoError C_CALL SMDExportLineAngleResultPng(AlgoHandle smd_handle, const char *out_result_filename);






	/************************************
	@brief    : 输出灯珠间隔检测结果图
	@note     : SMDGapDetect之后，可调用此接口看结果
	@param    : smd_handle               -- 有效的SMD句柄
	@param    : out_result_filename      -- 检测图像保存路径
	@return   : AlgoError
	@Modified : haopanxia@2023/01/09
	************************************/
	C_API AlgoError C_CALL SMDExportGapResultPng(AlgoHandle smd_handle, const char *out_result_filename);



    /************************************
    @brief  : 获取buffer大小
    @note   :
    @param  : smd_handle      当前有效句柄
    @param  : out_len         buffer大小
    @return	: AlgoError
    @Author : haopanxia@2023/01/09
    ************************************/
    C_API AlgoError C_CALL SMDGetBufferSize(AlgoHandle smd_handle, uint64_t* out_len);

    /************************************
    @brief  : 将smd里面的数据导出到buffer
    @note   :
    @param  : smd_handle      有效的smd句柄
    @param  : out_buffer      已经申请好申请空间
    @param  : in_out_len      外部申请好的空间大小,会用真实大小修改
    @return	: AlgoError
    @Author : haopanxia@2023/01/09
    ************************************/
    C_API AlgoError C_CALL SMDToBuffer(AlgoHandle smd_handle, char* out_buffer, uint64_t* in_out_len);


    /************************************
    @brief  : 将buffer数据导入到smd
    @note   :
    @param  : smd_handle   有效的smd句柄
    @param  : in_buffer
    @param  : in_len
    @return	: AlgoError
    @Author : haopanxia@2023/01/09
    ************************************/
    C_API AlgoError C_CALL SMDFromBuffer(AlgoHandle smd_handle, const char* in_buffer, uint64_t in_len);


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
    C_API AlgoError C_CALL SMDRelease(AlgoHandle smd_handle);
    C_API void C_CALL SMDDestroyHandle(AlgoHandle smd_handle);



#ifdef __cplusplus
}
#endif // __cplusplus
#endif