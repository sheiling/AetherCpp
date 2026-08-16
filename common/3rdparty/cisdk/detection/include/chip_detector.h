#ifndef ____CHIP_DETECTOR_H__
#define ____CHIP_DETECTOR_H__

#include "ci_declare.h"
#include "ci_define.h"
#include "ci_data.h"
#include "chip_detector_errors.h"
#include "chip_detector_data.h"
#include "algo_mat.h"


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

    //Chip的检测对象主要有三类
    //1.灯板返修前的chip定位——返回chip中心坐标
    //2.点锡后，放置好chip，固晶前的chip检测——返回chip中心坐标+偏移角度
    //3.返修完成，固晶结束后的chip检测——返回chip中心坐标+偏移角度
    //共用一个检测方法


    /************************************
    @brief    : 创建句柄
    @note     :
    @return   : 
    @Modified : haopanxia@2022/09/30
    ************************************/
    //C_API AlgoError C_CALL ChipCreateHandle(AlgoHandle* out_chip_handle);
    C_API AlgoHandle C_CALL ChipCreateHandle();

    /************************************
    @brief    : 设置Chip类型信息
    @note     : 
    @param    : chip_handle           有效的chip句柄
    @param    : in_chip_type          chip类型 ,R/G/B 
	@param    : in_chip_align         chip整组排列类型
    @param    : in_chip_size          chip的像素尺寸
    @return   : AlgoError
    @Modified : haopanxia@2022/09/30
    ************************************/
    C_API AlgoError C_CALL ChipSetTypeInfo(AlgoHandle chip_handle, ChipDetectRGBType in_chip_type, ChipDetectRGBAlignType in_chip_align, const AlgoSize2f* in_chip_size);

    /************************************
    @brief    : 设置焊锡检测相关阈值参数
    @note     :
    @param    : chip_handle              有效的chip句柄
    @param    : in_thresh                灯板上芯片检测相关阈值参数
    @return   : AlgoError
    @Modified : haopanxia@2022/09/30
    ************************************/
    C_API AlgoError C_CALL ChipSetThresh(AlgoHandle chip_handle, const ChipDetectThresh* in_thresh);

	
	/************************************
	@brief    : 设置焊锡检测相关阈值参数
	@note     :
	@param    : chip_handle              有效的chip句柄
	@param    : in_roi_size              灯板上截取的ROI区域的大小
	@return   : AlgoError
	@Modified : haopanxia@2022/04/12
	************************************/
	C_API AlgoError C_CALL ChipSetROI(AlgoHandle chip_handle, const AlgoSize* in_roi_size);
	
	
	/************************************
	@brief    : 获取焊锡检测相关阈值参数
	@note     :
	@param    : chip_handle              有效的chip句柄
	@param    : out_thresh                灯板上芯片检测相关阈值参数
	@return   : AlgoError
	@Modified : haopanxia@2023/01/03
	************************************/
	C_API AlgoError C_CALL ChipGetThresh(AlgoHandle chip_handle, ChipDetectThresh* out_thresh);


    /************************************
    @brief    : 设置模板
    @note     : 
    @param    : chip_handle         有效的chip句柄
    @param    : model_handle        model句柄
    @param    : in_template_name    模板名字
    @return   : AlgoError
    @Modified : chengjiayi@2022/11/08
    ************************************/
    C_API AlgoError C_CALL ChipSetTemplate(AlgoHandle chip_handle, AlgoHandle model_handle, const char* in_template_name);
    
	/************************************
	@brief    : 设置双模板，第一个模板是整体RGB图像模板，第二个模板是R/G/B其中之一的模板
	@note     :
	@param    : chip_handle                有效的chip句柄
	@param    : model_handle               model句柄
	@param    : in_first_template_name     主模板名字
	@param    : in_second_template_name    副模板名字
	@return   : AlgoError
	@Modified : haopanxia@2023/09/19
	************************************/
	C_API AlgoError C_CALL ChipSetDoubleTemplate(AlgoHandle chip_handle, AlgoHandle model_handle, const char* in_first_template_name, const char* in_second_template_name);

    /************************************
    @brief    : 载入chip模板文件
    @note     :
    @param    : chip_handle               有效的chip句柄
    @param    : in_chip_template_files    chip模板文件
    @param    : in_template_name          chip模板名字
    @return   : AlgoError
    @Modified : haopanxia@2022/09/30
    ************************************/
    C_API AlgoError C_CALL ChipLoadTemplate(AlgoHandle chip_handle, const char* in_chip_template_files, const char* in_template_name);

    /************************************
    @brief    : 确定chip中心及偏移角度，主要用在返修机中进行精定位操作，一代二代通用
    @note     : 
    @param    : chip_handle              有效的chip句柄
    @param    : in_image                 需要定位的图像
    @param    : out_chip_center          chip中心相对于图像中心的偏移量
	@param    : out_solder_center        solder中心相对于图像中心的偏移量
    param     : out_chip_angle           chip的偏移角度，相对于X轴（后续可调整）
    @return   : AlgoError
    @Modified : haopanxia@2022/09/30
    ************************************/
    C_API AlgoError C_CALL ChipDetectTarget(AlgoHandle chip_handle, const AlgoMat* in_image,
        AlgoPoint2f* out_chip_center, AlgoPoint2f* out_solder_center, float* out_chip_angle);



	/************************************
	@brief    : 确定chip中心及偏移角度
	@note     :
	@param    : chip_handle              有效的chip句柄
	@param    : in_image                 需要定位的图像
	@param    : out_chip_center          chip中心相对于图像中心的偏移量
	@param    : out_solder_center        solder中心相对于图像中心的偏移量
	param     : out_chip_angle           chip的偏移角度，相对于X轴（后续可调整）
	@return   : AlgoError
	@Modified : haopanxia@2022/12/09
	************************************/
	C_API AlgoError C_CALL ChipSingleDetectTarget(AlgoHandle chip_handle, const AlgoMat* in_image,
		AlgoPoint2f* out_chip_center, AlgoPoint2f* out_solder_center, float* out_chip_angle);


	/************************************
	@brief    : 确定开窗hole中心、宽度、高度(像素值)
	@note     :
	@param    : chip_handle              有效的chip句柄
	@param    : in_image                 需要定位的图像
	@param    : out_hole_center          hole中心相对于图像中心的偏移量
	@param    : out_hole_width           开窗hole的像素宽度，-1的时候说明没有找到挖胶区域
	param     : out_hole_height          开窗hole的像素高度，-1的时候说明没有找到挖胶区域
	@return   : AlgoError
	@Modified : haopanxia@2025/12/01
	************************************/
	C_API AlgoError C_CALL ChipGlueHoleDetectTarget(AlgoHandle chip_handle, const AlgoMat* in_image,
		AlgoPoint2f* out_hole_center, int* out_hole_width, int* out_hole_height);


	/************************************
	@brief    : 获取chip的轮廓点集点数量
	@note     :
	@param    : chip_handle          有效的chip句柄
	@param    : out_len              chip的轮廓点集的点数量
	@return   : AlgoError
	@Modified : haopanxia@2023/01/03
	************************************/
	C_API AlgoError C_CALL ChipDetectGetBorderSize(AlgoHandle chip_handle, int* out_len);


    /************************************
    @brief    : 获取chip的轮廓点集
    @note     :
    @param    : chip_handle          有效的chip句柄
	@param    : in_len               chip的轮廓点集的点数量
    @param    : out_chip_border      chip的轮廓点集
    @return   : AlgoError
    @Modified : haopanxia@2022/09/30
    ************************************/
    C_API AlgoError C_CALL ChipDetectGetBorder(AlgoHandle chip_handle, int* in_len, AlgoPoint* out_chip_border);



    /************************************
    @brief    : 输出检测的chip的轮廓的结果图
    @note     :
    @param    : chip_handle          有效的chip句柄
    @param    : out_png_filename     图像保存路径
    @return   : AlgoError
    @Modified : haopanxia@2022/09/30
    ************************************/
    C_API AlgoError C_CALL ChipDetectExportResultPng(AlgoHandle chip_handle, const char *out_png_filename);


	/************************************
	@brief    : 以mat形式输出检测的chip的结果图
	@note     :
	@param    : chip_handle           有效的solder句柄
	@param    : out_result_mat         mat图像
	@return   : AlgoError
	@Modified : haopanxia@2024/11/14
	************************************/
	C_API AlgoError C_CALL ChipDetectGetResultMat(AlgoHandle chip_handle, AlgoMat* out_result_mat);




    /************************************
    @brief    : 获取buffer大小
    @note     :
    @param    : chip_handle          有效的chip句柄
    @param    : out_len              buffer大小
    @return   : AlgoError
    @Modified : haopanxia@2022/09/30
    ************************************/
    C_API AlgoError C_CALL ChipDetectGetBufferSize(AlgoHandle chip_handle, uint64_t* out_len);




    /************************************
    @brief    : 将chipDetect里面的数据导出到buffer
    @note     :
    @param    : chip_handle         有效的chip句柄
    @param    : out_buffer          已经申请好申请空间
    @param    : in_out_len          外部申请好的空间大小,会用真实大小修改
    @return   : AlgoError
    @Modified : haopanxia@2022/09/30
    ************************************/
    C_API AlgoError C_CALL ChipDetectToBuffer(AlgoHandle chip_handle, char* out_buffer, uint64_t* in_out_len);


    /************************************
    @brief    : 将buffer数据导入到ChipDetect
    @note     :
    @param    : chip_handle          有效的chip句柄
    @param    : in_buffer
    @param    : in_len
    @return   : AlgoError
    @Modified : haopanxia@2022/09/30
    ************************************/
    C_API AlgoError C_CALL ChipDetectFromBuffer(AlgoHandle chip_handle, const char* in_buffer, uint64_t in_len);



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
    C_API AlgoError C_CALL ChipRelease(AlgoHandle chip_handle);
    C_API void C_CALL ChipDestroyHandle(AlgoHandle chip_handle);












#ifdef __cplusplus
}
#endif // __cplusplus
#endif