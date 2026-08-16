#ifndef ____SOLDER_DETECTOR_H__
#define ____SOLDER_DETECTOR_H__

#include "ci_declare.h"
#include "ci_define.h"
#include "ci_data.h"
#include "solder_detector_errors.h"
#include "solder_detector_data.h"
#include "algo_mat.h"


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


    /************************************
    @brief    : 创建句柄
    @note     :
    @return   : AlgoHandle
    @Modified : haopanxia@2022/09/30
    ************************************/
    //C_API AlgoError C_CALL SolderCreateHandle(AlgoHandle* out_solder_handle);
	C_API AlgoHandle C_CALL SolderCreateHandle();

	/************************************
	@brief    : 设置焊盘相关参数
	@note     :
	@param    : solder_handle            有效的solder句柄
	@param    : in_solder_size           焊盘尺寸信息
	@return   : AlgoError
	@Modified : haopanxia@2022/11/04
	************************************/
	C_API AlgoError C_CALL SolderSetTypeInfo(AlgoHandle solder_handle, const AlgoSize2f* in_solder_size, SolderDirection in_solder_direction);



	/************************************
	@brief    : 设置焊锡检测相关阈值参数
	@note     :
	@param    : solder_handle            有效的solder句柄
	@param    : in_thresh                焊锡检测相关阈值参数 
	@return   : AlgoError
	@Modified : haopanxia@2022/09/30
	************************************/
	C_API AlgoError C_CALL SolderSetThresh(AlgoHandle solder_handle, const SolderDetectThresh* in_thresh);


	/************************************
	@brief    : 设置焊锡检测相关阈值参数
	@note     :
	@param    : solder_handle            有效的solder句柄
	@param    : in_thresh                焊锡检测相关阈值参数
	@return   : AlgoError
	@Modified : haopanxia@2022/09/30
	************************************/
	C_API AlgoError C_CALL SolderSetROI(AlgoHandle solder_handle, const AlgoSize* in_roi_size);



	/************************************
	@brief    : 设置模板
	@note     :
	@param    : solder_handle         有效的solder句柄
	@param    : model_handle          model句柄
	@param    : in_template_name      模板名字
	@return   : AlgoError
	@Modified : haopanxia@2022/11/08
	************************************/
	C_API AlgoError C_CALL SolderSetTemplate(AlgoHandle solder_handle, AlgoHandle model_handle, const char* in_template_name);




	/************************************
	@brief    : 载入solder模板文件
	@note     :
	@param    : solder_handle               有效的solder句柄
	@param    : in_solder_template_files    solder模板文件
	@param    : in_template_name            solder模板名字
	@return   : AlgoError
	@Modified : haopanxia@2022/09/30
	************************************/
	C_API AlgoError C_CALL SolderLoadTemplate(AlgoHandle solder_handle, const char* in_solder_template_files, const char* in_template_name);





	/************************************
	@brief    : 焊盘整体及左右两个位置定位（共输出三个位置信息）
	@note     :
	@param    : solder_handle            有效的solder句柄
	@param    : in_image                 需要定位的图像
	@param    : out_pad_center           检测得到的pad中心像素坐标
	@param    : out_pad_angle            检测得到的pad的偏移角度
	@param    : out_left_pad_center      检测到的左侧焊盘中心像素坐标
	@param    : out_right_pad_center     检测到的右侧焊盘中心像素坐标
	@return   : AlgoError
	@Modified : haopanxia@2022/09/30
	************************************/
	C_API AlgoError C_CALL SolderPadLocator(AlgoHandle solder_handle, const AlgoMat* in_image,
		AlgoPoint2f* out_pad_center, float* out_pad_angle, AlgoPoint2f* out_left_pad_center, AlgoPoint2f* out_right_pad_center);





    /************************************
    @brief    : 焊锡残留检测
    @note     : 
    @param    : solder_handle            有效的solder句柄
	@param    : in_image                 需要定位的图像
	@param    : out_left_soledr_residue  左侧焊锡残留程度，0-1,0代表无残留，1代表残留程度最高
	@param    : out_right_soledr_residue 右侧焊锡残留程度，0-1,0代表无残留，1代表残留程度最高
    @return   : AlgoError
    @Modified : haopanxia@2022/09/30
    ************************************/
    C_API AlgoError C_CALL SolderResidueDetector(AlgoHandle solder_handle, const AlgoMat* in_image,
		float *out_left_solder_residue, float *out_right_solder_residue);



	/************************************
	@brief    : 点锡量检测
	@note     :
	@param    : solder_handle             有效的solder句柄
	@param    : in_image                  需要定位的图像
	@param    : out_left_solder_area      左侧焊盘点锡量的面积
	@param    : out_right_solder_area     右侧焊盘点锡量的面积
	@return   : AlgoError
	@Modified : haopanxia@2022/09/30
    ************************************/
    C_API AlgoError C_CALL SolderQuantityDetector(AlgoHandle solder_handle, const AlgoMat* in_image,
		float *out_left_solder_area, float *out_right_solder_area);
	// to do 是否需要输出焊锡量的程度，辅助判断，个人认为需要添加





	/************************************
	@brief    : 点锡量检测-new-20230706
	@note     :
	@param    : solder_handle                   有效的solder句柄
	@param    : in_front_image                  点锡前的图像
	@param    : in_after_image                  点锡后的图像
	@param    : out_left_solder_area            左侧焊盘点锡量的面积
	@param    : out_left_solder_proportion      左侧焊盘点锡量相对于焊盘的占比
	@param    : out_right_solder_area           右侧焊盘点锡量的面积
	@param    : out_right_solder_proportion     右侧焊盘点锡量相对于焊盘的占比
	@return   : AlgoError
	@Modified : haopanxia@2023/07/06
	************************************/
	C_API AlgoError C_CALL SolderMassDetector(AlgoHandle solder_handle, const AlgoMat* in_front_image, const AlgoMat* in_after_image,
		float *out_left_solder_area, float *out_left_solder_proportion, 
		float *out_right_solder_area, float *out_right_solder_proportion);


	/************************************
	@brief    : 焊盘缺陷检测，缺陷类别见solder_detector_data.h中的SolderDefectType示意
	@note     :
	@param    : solder_handle            有效的solder句柄
	@param    : in_image                 需要定位的图像
	@param    : out_defect_type          检测到的缺陷类型分类
	@return   : AlgoError
	@Modified : haopanxia@2024/06/27
	************************************/
	C_API AlgoError C_CALL SolderDefectDetector(AlgoHandle solder_handle, const AlgoMat* in_image, SolderDefectType* out_defect_type);


	/************************************
	@brief    : 获取solder的轮廓点集
	@note     :
	@param    : solder_handle                有效的solder句柄
	@param    : out_left_solder_border       左侧solder的轮廓点集
	@param    : out_right_solder_border      右侧solder的轮廓点集
	@param    : out_num                      点数量
	@return   : AlgoError
	@Modified : haopanxia@2022/09/30
	************************************/
	C_API AlgoError C_CALL SolderDetectGetBorder(AlgoHandle die_handle, AlgoPoint* out_left_solder_border, AlgoPoint* out_right_solder_border, int *out_num);



	/************************************
	@brief    : 输出检测的solder的轮廓的结果图
	@note     :
	@param    : solder_handle          有效的solder句柄
	@param    : out_png_filename       图像保存路径
	@return   : AlgoError
	@Modified : haopanxia@2022/09/30
	************************************/
	C_API AlgoError C_CALL SolderDetectExportResultPng(AlgoHandle solder_handle, const char *out_png_filename);




	/************************************
	@brief    : 以mat形式输出检测的solder的结果图
	@note     :
	@param    : solder_handle          有效的solder句柄
	@param    : out_result_mat         mat图像
	@return   : AlgoError
	@Modified : haopanxia@2024/11/14
	************************************/
	C_API AlgoError C_CALL SolderDetectGetResultMat(AlgoHandle solder_handle, AlgoMat* out_result_mat);


	/************************************
	@brief    : 获取buffer大小
	@note     :
	@param    : solder_handle          有效的solder句柄
	@param    : out_len                buffer大小
	@return   : AlgoError
	@Modified : haopanxia@2022/09/30
	************************************/
	C_API AlgoError C_CALL SolderDetectGetBufferSize(AlgoHandle solder_handle, uint64_t* out_len);




	/************************************
	@brief    : 将SolderDetect里面的数据导出到buffer
	@note     :
	@param    : solder_handle       有效的solder句柄
	@param    : out_buffer          已经申请好申请空间
	@param    : in_out_len          外部申请好的空间大小,会用真实大小修改
	@return   : AlgoError
	@Modified : haopanxia@2022/09/30
	************************************/
	C_API AlgoError C_CALL SolderDetectToBuffer(AlgoHandle solder_handle, char* out_buffer, uint64_t* in_out_len);


	/************************************
	@brief    : 将buffer数据导入到SolderDetect
	@note     :
	@param    : solder_handle          有效的solder句柄
	@param    : in_buffer
	@param    : in_len
	@return   : AlgoError
	@Modified : haopanxia@2022/09/30
	************************************/
	C_API AlgoError C_CALL SolderDetectFromBuffer(AlgoHandle solder_handle, const char* in_buffer, uint64_t in_len);



















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
    C_API AlgoError C_CALL SolderRelease(AlgoHandle solder_handle);
	C_API void C_CALL SolderDestroyHandle(AlgoHandle solder_handle);












#ifdef __cplusplus
}
#endif // __cplusplus
#endif