#ifndef ____WAFER_DEFECT_DETECTOR_H__
#define ____WAFER_DEFECT_DETECTOR_H__

#include "ci_declare.h"
#include "ci_define.h"
#include "ci_data.h"
#include "wafer_defect_detector_errors.h"
#include "wafer_defect_detector_data.h"
#include "algo_mat.h"


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


    /************************************
    @brief    : 创建句柄
    @note     :
    @return   : 
    @Modified : haopanxia@2023/08/04
    ************************************/
    C_API AlgoHandle C_CALL WaferDefectCreateHandle();

    /************************************
    @brief    : 设置wafer类型信息,根据产品信息自行定义，与产品类型强相关，该部分不可以省略
    @note     : 
    @param    : micro_handle           有效的wafer句柄
    @param    : in_micro_size          晶圆尺寸信息，像素个数
	@param    : in_gap_size            晶圆间距信息，像素个数
    @return   : AlgoError
    @Modified : haopanxia@2023/08/04
    ************************************/
    C_API AlgoError C_CALL WaferDefectSetSizeInfo(AlgoHandle wafer_handle, const AlgoSize2f* in_micro_size, const AlgoSize2f* in_gap_size);

    /************************************
    @brief    : 设置wafer缺陷检测相关阈值参数,如果不调用的话
    @note     : 内部写有默认值，实际测试时建议开放，参数配置稳定后可选择不开放
    @param    : wafer_handle              有效的wafer句柄
    @param    : in_thresh                 micro缺陷检测相关阈值参数
    @return   : AlgoError
    @Modified : haopanxia@2023/08/04
    ************************************/
    C_API AlgoError C_CALL WaferDefectSetThresh(AlgoHandle wafer_handle, const WaferDefectDetectThresh* in_thresh);

	
	/************************************
	@brief    : 获取micro缺陷检测相关阈值参数
	@note     :
	@param    : wafer_handle              有效的micro句柄
	@param    : out_thresh                灯板上芯片检测相关阈值参数
	@return   : AlgoError
	@Modified : haopanxia@2023/08/04
	************************************/
	C_API AlgoError C_CALL WaferDefectGetThresh(AlgoHandle wafer_handle, WaferDefectDetectThresh* out_thresh);


	/************************************
	@brief    : 设置缺陷评价相关参数及评价方式，设置的参数只作用于WaferDefectEvaluate，如果不需要缺陷评估，可以不用设置该部分
	@note     :
	@param    : wafer_handle              有效的wafer句柄
	@param    : in_param                  灯板上芯片检测相关阈值参数
	@param    : in_eval_method                  灯板上芯片检测相关阈值参数
	@return   : AlgoError
	@Modified : haopanxia@2023/08/04
	************************************/
	C_API AlgoError C_CALL WaferDefectSetEvaluateParam(AlgoHandle wafer_handle, const WaferDefectEvaluateParam* in_param,
		WaferDefectMethod in_eval_method);



	

    /************************************
    @brief    : wafer缺陷检测，主要将缺陷进行检出，输出缺陷个数，用于后面进行缺陷评估或border获取时，创建空间
    @note     : 
    @param    : wafer_handle              有效的wafer句柄
    @param    : in_img                    需要检测的图像
	@param    : out_contour_img           缺陷轮廓组成的图像
    @param    : out_defect_num            检测得到的缺陷数量
    @return   : AlgoError
    @Modified : haopanxia@2023/08/04
    ************************************/
    C_API AlgoError C_CALL WaferDefectDetect(AlgoHandle wafer_handle, const AlgoMat* in_img, int* out_defect_num);


	/************************************
	@brief    : wafer缺陷评价，要跟在WaferDefectDetect后面使用，不可单独使用
	@note     : out_defect_feature的空间需要WaferDefectDetect吐出的out_defect_num进行空间大小定义
	@param    : wafer_handle              有效的wafer句柄
	@param    : in_img                    需要检测的图像
	@param    : out_defect_feature        每个缺陷的详细信息，详见micro_detector_data.h文件释义
	@return   : AlgoError
	@Modified : haopanxia@2023/08/04
	************************************/
	C_API AlgoError C_CALL WaferDefectEvaluate(AlgoHandle wafer_handle, const AlgoMat* in_img,
		WaferDefectFeature* out_defect_feature);


	/************************************
	@brief    : 获取每个缺陷区域的轮廓数据，每一个AlgoMat代表一个缺陷轮廓
	@note     :
	@param    : wafer_handle              有效的wafer句柄
	@param    : in_len                    wafer defect的轮廓点集的点数量
	@param    : out_defect_contours       wafer上每个缺陷的轮廓点，以Mat图展示
	@return   : AlgoError
	@Modified : haopanxia@2023/08/04
	************************************/
	C_API AlgoError C_CALL WaferDefectGetBorder(AlgoHandle wafer_handle, int* in_len, AlgoMat* out_defect_contours);



	//根据不同的缺陷导出不同的缺陷图像
    /************************************
    @brief    : 输出检测的defect的轮廓的结果图
    @note     :
    @param    : wafer_handle              有效的wafer句柄
    @param    : out_png_filename          图像保存路径
	@param    : in_defect_type            用于根据不同缺陷类型将缺陷图像输出，不设置时，默认为ALL_DEFECT，全部输出
	@param    : in_defect_severity        用于根据不同缺陷程度进行结果输出，不设置时，默认为-1，表示全部输出
	          in_defect_type和in_defect_severity只有一个有效，因此使用其中之一时，需要将另外一个值输入为默认值
    @return   : AlgoError
    @Modified : haopanxia@2023/08/04
    ************************************/
    C_API AlgoError C_CALL WaferDefectExportResultPng(AlgoHandle wafer_handle, const char *out_png_filename, WaferDefectType in_defect_type = ALL_DEFECT, float in_defect_severity = -1);




    /************************************
    @brief    : 获取buffer大小
    @note     :
    @param    : wafer_handle              有效的wafer句柄
    @param    : out_len                   buffer大小
    @return   : AlgoError
    @Modified : haopanxia@2023/08/04
    ************************************/
    C_API AlgoError C_CALL WaferDefectGetBufferSize(AlgoHandle wafer_handle, uint64_t* out_len);




    /************************************
    @brief    : 将wafer_handle里面的数据导出到buffer，把详细的东西写清楚
    @note     :
    @param    : wafer_handle        有效的wafer句柄
    @param    : out_buffer          已经申请好申请空间
    @param    : in_out_len          外部申请好的空间大小,会用真实大小修改
    @return   : AlgoError
    @Modified : haopanxia@2023/08/04
    ************************************/
    C_API AlgoError C_CALL WaferDefectToBuffer(AlgoHandle wafer_handle, char* out_buffer, uint64_t* in_out_len);


    /************************************
    @brief    : 将buffer数据导入到WaferDetect
    @note     :
    @param    : wafer_handle              有效的wafer句柄
    @param    : in_buffer
    @param    : in_len
    @return   : AlgoError
    @Modified : haopanxia@2023/08/04
    ************************************/
    C_API AlgoError C_CALL WaferDefectFromBuffer(AlgoHandle wafer_handle, const char* in_buffer, uint64_t in_len);




	/************************************
    @Brief	：载入和保存检测结果
    @Note	：文件名后缀必须是.aoidefect
    @param	: wafer_handle, 输入量, 有效的wafer handle
    @param	: in_fullname,          输入量, 文件名后缀必须是.aoidefect
    @return : AlgoError
	@Author : songshaojun@2024/12/04
    ************************************/
    C_API AlgoError C_CALL WaferDefectLoad(AlgoHandle wafer_handle, const char* in_fullname);
    C_API AlgoError C_CALL WaferDefectSave(AlgoHandle wafer_handle, const char* in_fullname);

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
    C_API AlgoError C_CALL WaferDefectRelease(AlgoHandle wafer_handle);
    C_API void C_CALL WaferDefectDestroyHandle(AlgoHandle wafer_handle);



#ifdef __cplusplus
}
#endif // __cplusplus
#endif