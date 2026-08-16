#ifndef __DIE_DEFECT_DETECTOR_H__
#define __DIE_DEFECT_DETECTOR_H__

#include "ci_declare.h"
#include "ci_define.h"
#include "ci_data.h"
#include "algo_mat.h"

#include "die_defect_detector_data.h"
#include "die_defect_detector_error.h"


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	/************************************
	@brief    : 创建句柄
	@note     :
	@param    : detector_handle
	@return   : AlgoError
	@Author   : wushaoshuang@2024/09/24
	************************************/
	C_API AlgoError C_CALL DieDefectCreateHandle(AlgoHandle* detector_handle);

	/************************************
	@brief    : 设置芯片模板，从model设置模板
	@note     :
	@param    : detector_handle     有效的检测句柄
	@param    : model_handle        model句柄
	@param    : in_template_name    模板名字
	@return   : AlgoError
	@Author   : wushaoshuang@2024/09/24
	************************************/
	C_API AlgoError C_CALL DieDefectSetTemplateFromModel(AlgoHandle detector_handle, AlgoHandle model_handle, const char* in_template_name);

	/************************************
	@brief    : 设置芯片模板，从图像设置模板
	@note     :
	@param    : detector_handle     有效的检测句柄
	@param    : in_die_template     输入的模板
	@return   : AlgoError
	@Author   : wushaoshuang@2024/09/24
	************************************/
	C_API AlgoError C_CALL DieDefectSetTemplateFromMat(AlgoHandle detector_handle, const AlgoMat* in_die_template);

	/************************************
	@brief    : 获取芯片模板（根据需求调用）
	@note     :
	@param    : detector_handle     有效的检测句柄
	@param    : out_die_template    输出的模板图像
	@return   : AlgoError
	@Author   : wushaoshuang@2024/09/24
	************************************/
	C_API AlgoError C_CALL DieDefectGetTemplate(AlgoHandle detector_handle, AlgoMat* out_die_template);

	/************************************
	@brief    : 设置背景类型
	@note     : 设置背景类型，即黑底白目标或白底黑目标，表示目标前景与背景的亮度对比情况，背景亮度比前景亮度大时，表示白底黑目标，反之为黑底白目标
	@param    : detector_handle     有效的检测句柄
	@param    : in_back_type        输入的背景类型，以DieBackgroundType枚举类型输入，详见die_defect_detector_data.h
	@return   : AlgoError
	@Author   : wushaoshuang@2024/09/24
	************************************/
	C_API AlgoError C_CALL DieDefectSetBackgroundType(AlgoHandle detector_handle, const DieBackgroundType* in_back_type);

	/************************************
	@brief    : 获取背景类型（根据需求调用）
	@note     : 获取背景类型
	@param    : detector_handle     有效的检测句柄
	@param    : out_back_type       输出的背景类型，以DieBackgroundType枚举类型输入，详见die_defect_detector_data.h
	@return   : AlgoError
	@Author   : wushaoshuang@2024/09/24
	************************************/
	C_API AlgoError C_CALL DieDefectGetBackgroundType(AlgoHandle detector_handle, DieBackgroundType* out_back_type);

	/************************************
	@brief    : 设置芯片宽高
	@note     : 设置芯片宽高，该宽高为图像像素宽高
	@param    : detector_handle     有效的检测句柄
	@param    : in_die_size         输入的芯片宽高，以AlgoSize类型输入
	@return   : AlgoError
	@Author   : wushaoshuang@2024/10/11
	************************************/
	C_API AlgoError C_CALL DieDefectSetDieSize(AlgoHandle detector_handle, const AlgoSize* in_die_size);

	/************************************
	@brief    : 获取芯片宽高（根据需求调用）
	@note     : 获取芯片宽高，该宽高为图像像素宽高
	@param    : detector_handle     有效的检测句柄
	@param    : out_die_size        输出的芯片宽高，以AlgoSize类型输出
	@return   : AlgoError
	@Author   : wushaoshuang@2024/10/11
	************************************/
	C_API AlgoError C_CALL DieDefectGetDieSize(AlgoHandle detector_handle, AlgoSize* out_die_size);

	/************************************
	@brief    : 设置多胞检测ROI
	@note     : 设置多胞检测ROI，表示以该ROI为界限，在其内部进行多胞缺陷检测,即以该roi来截取单个芯片图像，多胞检测时必须设置。
	@param    : detector_handle     有效的检测句柄
	@param    : in_twin_roi         输入的ROI
	@return   : AlgoError
	@Author   : wushaoshuang@2024/09/24
	************************************/
	C_API AlgoError C_CALL DieDefectSetTwinROI(AlgoHandle detector_handle, const AlgoRect* in_twin_roi);

	/************************************
	@brief    : 获取多胞检测ROI（根据需求调用）
	@note     : 获取多胞检测ROI
	@param    : detector_handle     有效的检测句柄
	@param    : out_twin_roi        输出的ROI
	@return   : AlgoError
	@Author   : wushaoshuang@2024/09/24
	************************************/
	C_API AlgoError C_CALL DieDefectGetTwinROI(AlgoHandle detector_handle, AlgoRect* out_twin_roi);

	/************************************
	@brief    : 设置多胞检测方法
	@note     : 设置多胞检测方法，分为面积占比检测方法、边缘超出检测方法以及两种组合方法共三种方法，详情见die_defect_detector_data.h中DieDefectDetectMethod中的方法说明。
	@param    : detector_handle      有效的检测句柄
	@param    : in_twin_detect_flag  输入的检测方法，类型为枚举DieDefectDetectMethod中的枚举值
	@return   : AlgoError
	@Author   : wushaoshuang@2024/09/24
	************************************/
	C_API AlgoError C_CALL DieDefectSetTwinDetectMethod(AlgoHandle detector_handle, const DieDefectDetectMethod* in_twin_detect_flag);

	/************************************
	@brief    : 获取多胞检测方法（根据需求调用）
	@note     : 获取多胞检测方法，详情见die_defect_detector_data.h中DieDefectDetectMethod中的方法说明。
	@param    : detector_handle       有效的检测句柄
	@param    : out_twin_detect_flag  输出的检测方法，类型为枚举DieDefectDetectMethod中的枚举值
	@return   : AlgoError
	@Author   : wushaoshuang@2024/09/24
	************************************/
	C_API AlgoError C_CALL DieDefectGetTwinDetectMethod(AlgoHandle detector_handle, DieDefectDetectMethod* out_twin_detect_flag);

	/************************************
	@brief    : 设置破损检测ROI
	@note     : 设置破损检测ROI，表示以该ROI为界限，在其外部进行破损缺陷检测，同时内部默认在该roi内部进行脏污检测，外部根据需求情况设置，破损检测时必须设置。
	@param    : detector_handle     有效的检测句柄
	@param    : in_damage_roi       输入的ROI
	@return   : AlgoError
	@Author   : wushaoshuang@2024/09/24
	************************************/
	C_API AlgoError C_CALL DieDefectSetDamageROI(AlgoHandle detector_handle, const AlgoRect* in_damage_roi);

	/************************************
	@brief    : 获取破损检测ROI（根据需求调用）
	@note     : 获取破损检测ROI
	@param    : detector_handle     有效的检测句柄
	@param    : out_damage_roi      输出的ROI
	@return   : AlgoError
	@Author   : wushaoshuang@2024/09/24
	************************************/
	C_API AlgoError C_CALL DieDefectGetDamageROI(AlgoHandle detector_handle, AlgoRect* out_damage_roi);

	/************************************
	@brief    : 设置破损检测方法
	@note     : 设置破损检测方法，分为面积占比检测方法、边缘超出检测方法以及两种组合方法共三种方法，详情见die_defect_detector_data.h中DieDefectDetectMethod中的方法说明。
	@param    : detector_handle        有效的检测句柄
	@param    : in_damage_detect_flag  输入的检测方法，类型为枚举DieDefectDetectMethod中的枚举值
	@return   : AlgoError
	@Author   : wushaoshuang@2024/09/24
	************************************/
	C_API AlgoError C_CALL DieDefectSetDamageDetectFlag(AlgoHandle detector_handle, const DieDefectDetectMethod* in_damage_detect_flag);

	/************************************
	@brief    : 获取破损检测方法（根据需求调用）
	@note     : 获取破损检测方法，详情见die_defect_detector_data.h中DieDefectDetectMethod中的方法说明。
	@param    : detector_handle         有效的检测句柄
	@param    : out_damage_detect_flag  输出的检测方法，类型为枚举DieDefectDetectMethod中的枚举值
	@return   : AlgoError
	@Author   : wushaoshuang@2024/09/24
	************************************/
	C_API AlgoError C_CALL DieDefectGetDamageDetectFlag(AlgoHandle detector_handle, DieDefectDetectMethod* out_damage_detect_flag);

	/************************************
	@brief    : 设置脏污检测ROI（根据需求调用）
	@note     : 设置脏污检测ROI，根据需要设置，表示以该ROI为界限，在其内部进行脏污检测，若不设置，内部默认以破损ROI界限为脏污界限，在破损ROI内部进行脏污检测，若破损ROI也未设置则会报错。注意：若需要设置该ROI，则该roi必须为模板图像上的roi数据，否则内部无法识别到该roi。
	@param    : detector_handle     有效的检测句柄
	@param    : in_dirty_roi        输入的ROI
	@return   : AlgoError
	@Author   : wushaoshuang@2024/10/11
	************************************/
	C_API AlgoError C_CALL DieDefectSetDirtyROI(AlgoHandle detector_handle, const AlgoRect* in_dirty_roi);

	/************************************
	@brief    : 获取脏污检测ROI（根据需求调用）
	@note     : 获取脏污检测ROI
	@param    : detector_handle     有效的检测句柄
	@param    : out_dirty_roi       输出的ROI
	@return   : AlgoError
	@Author   : wushaoshuang@2024/09/24
	************************************/
	C_API AlgoError C_CALL DieDefectGetDirtyROI(AlgoHandle detector_handle, AlgoRect* out_dirty_roi);

	/************************************
	@brief    : 设置分割阈值
	@note     : 设置芯片分割阈值，即表示可以将芯片背景与芯片前景区分开的灰度值，外部可以实际情况设置
	@param    : detector_handle        有效的检测句柄
	@param    : in_segment_thresh      输入的分割阈值，该值不能小于0
	@return   : AlgoError
	@Author   : wushaoshuang@2024/09/24
	************************************/
	C_API AlgoError C_CALL DieDefectSetSegmentThresh(AlgoHandle detector_handle, const int* in_segment_thresh);

	/************************************
	@brief    : 获取分割阈值（根据需求调用）
	@note     : 获取芯片分割阈值（根据需求调用）
	@param    : detector_handle        有效的检测句柄
	@param    : out_segment_thresh     输出的分割阈值
	@return   : AlgoError
	@Author   : wushaoshuang@2024/09/24
	************************************/
	C_API AlgoError C_CALL DieDefectGetSegmentThresh(AlgoHandle detector_handle, int* out_segment_thresh);

	/************************************
	@brief    : 设置多胞阈值
	@note     : 设置多胞缺陷阈值，在多胞检测方法设置为AREA_DETECT或ALL_DETECT时需要设置该阈值，表示目标分割面积与正常芯片分割面积的比值超过多少时则认为是多胞缺陷
	@param    : detector_handle        有效的检测句柄
	@param    : in_twin_thresh         输入的多胞阈值，该值不能小于1
	@return   : AlgoError
	@Author   : wushaoshuang@2024/09/24
	************************************/
	C_API AlgoError C_CALL DieDefectSetTwinThresh(AlgoHandle detector_handle, const float* in_twin_thresh);

	/************************************
	@brief    : 获取多胞阈值（根据需求调用）
	@note     : 获取多胞缺陷阈值（根据需求调用）
	@param    : detector_handle        有效的检测句柄
	@param    : out_twin_thresh        输出的多胞阈值
	@return   : AlgoError
	@Author   : wushaoshuang@2024/09/24
	************************************/
	C_API AlgoError C_CALL DieDefectGetTwinThresh(AlgoHandle detector_handle, float* out_twin_thresh);

	/************************************
	@brief    : 设置破损阈值
	@note     : 设置破损缺陷阈值，在破损检测方法设置为AREA_DETECT或ALL_DETECT时需要设置该阈值，表示目标分割面积与正常芯片分割面积的比值超过多少时则认为是破损缺陷
	@param    : detector_handle        有效的检测句柄
	@param    : in_damage_thresh       输入的破损阈值，该值不能小于0，不能大于1
	@return   : AlgoError
	@Author   : wushaoshuang@2024/09/24
	************************************/
	C_API AlgoError C_CALL DieDefectSetDamageThresh(AlgoHandle detector_handle, const float* in_damage_thresh);

	/************************************
	@brief    : 获取破损阈值（根据需求调用）
	@note     : 获取破损缺陷阈值（根据需求调用）
	@param    : detector_handle        有效的检测句柄
	@param    : out_damage_thresh        输出的多胞阈值，该值不能小于1
	@return   : AlgoError
	@Author   : wushaoshuang@2024/09/24
	************************************/
	C_API AlgoError C_CALL DieDefectGetDamageThresh(AlgoHandle detector_handle, float* out_damage_thresh);

	/************************************
	@brief    : 设置面积阈值
	@note     : 设置脏污缺陷面积阈值，表示检出脏污的轮廓面积大于该值时则认为是脏污缺陷，小于认为不是，内部默认为100
	@param    : detector_handle        有效的检测句柄
	@param    : in_area_thresh         输入的脏污面积阈值，该值不能小于0
	@return   : AlgoError
	@Author   : wushaoshuang@2024/09/24
	************************************/
	C_API AlgoError C_CALL DieDefectSetDirtyAreaThresh(AlgoHandle detector_handle, const int* in_area_thresh);

	/************************************
	@brief    : 获取面积阈值（外部根据需求调用）
	@note     : 获取脏污缺陷面积阈值
	@param    : detector_handle        有效的检测句柄
	@param    : out_area_thresh        输出的脏污面积阈值
	@return   : AlgoError
	@Author   : wushaoshuang@2024/09/24
	************************************/
	C_API AlgoError C_CALL DieDefectGetDirtyAreaThresh(AlgoHandle detector_handle, int* out_area_thresh);

	/************************************
	@brief    : 设置差分阈值
	@note     : 设置脏污缺陷差分阈值，表示检出脏污区域的灰度值与正常模板对应区域的灰度值的差异值大于该值时认为是脏污缺陷（即对脏污的亮度进行判断），小于认为不是，内部默认为60
	@param    : detector_handle        有效的检测句柄
	@param    : in_diff_thresh         输入的脏污差分阈值，该值不能小于0
	@return   : AlgoError
	@Author   : wushaoshuang@2024/09/24
	************************************/
	C_API AlgoError C_CALL DieDefectSetDirtyDiffThresh(AlgoHandle detector_handle, const int* in_diff_thresh);

	/************************************
	@brief    : 获取差分阈值（根据需求设置）
	@note     : 获取脏污缺陷差分阈值，
	@param    : detector_handle        有效的检测句柄
	@param    : out_diff_thresh        输出的脏污差分阈值，该值不能小于0
	@return   : AlgoError
	@Author   : wushaoshuang@2024/09/24
	************************************/
	C_API AlgoError C_CALL DieDefectGetDirtyDiffThresh(AlgoHandle detector_handle, int* out_diff_thresh);

	/************************************
	@brief    : 缺陷检测主接口
	@note     : 缺陷检测主接口，检测结果以一个1行n列的2通道mat存储，n表示有n个芯片被检测，mat中第一个通道的值与DieDefectDetectResult结构体枚举值对应，为检测的芯片的缺陷结果，第二个通道的值为当前缺陷所对应的缺陷面积。当缺陷检测方法为仅边缘检测时，算法不会计算缺陷面积，即默认为0。当缺陷检测结果为未知时，对应的缺陷面积为-1。检测前必须先保证芯片定位的准确性，要不然检测结果不准确。
	@param    : detector_handle     有效的检测句柄
	@param    : in_image            输入的待检测图像
	@param    : in_locate_info      输入的芯片位置数据,数据类型见结构体DieLocateInfo，详情见die_defect_detector_data.h，需要外部new空间，以DieLocateInfo类型的数组传入
	@param    : in_locate_data_len  输入的待检测芯片的数量,即数组的大小
	@param    : out_detect_result   输出的检测结果
	@param    : in_detect_item      输入的缺陷检测内容，数据结构及说明见DieDefectDetectList，默认为三种缺陷类型都检测，外部无需额外输入，若有变动则以外部输入为准。
	@return   : AlgoError
	@Author   : wushaoshuang@2024/09/24
	************************************/
	C_API AlgoError C_CALL DieDefectDetect(AlgoHandle detector_handle, AlgoMat* in_image, DieLocateInfo* in_locate_info, const int in_locate_info_len, AlgoMat* out_detect_result, const DieDefectDetectList* in_detect_item = nullptr);

	/*********************************************
	@brief   : 导出检测结果图像
	@note    : 导出检测结果图像，外部根据需求调用，图像仅供参考
	@param   : detector_handle     有效的检测句柄
	@param   : in_png_path         输入的图像存储路径
	@return  : AlgoError
	@Author  : wushaoshuang@2024/09/24
	*********************************************/
	C_API AlgoError C_CALL DieDefectExportPng(AlgoHandle detector_handle, const char* in_png_path);

	/*********************************************
	@brief   : 获取buffer大小
	@note    :
	@param   : detector_handle     有效的检测句柄
	@param   : out_len             buffer大小
	@return  : AlgoError
	@Author  : wushaoshuang@2024/09/24
	*********************************************/
	C_API AlgoError C_CALL DieDefectGetBufferSize(AlgoHandle detector_handle, uint64_t* out_len);

	/*********************************************
	@brief   : 将数据导入buffer
	@note    :
	@param   : detector_handle     有效的检测句柄
	@param   : out_buffer          输出检测参数的buffer空间
	@param   : in_out_len          buffer空间大小
	@return  : AlgoError
	@Author  : wushaoshuang@2024/09/24
	*********************************************/
	C_API AlgoError C_CALL DieDefectToBuffer(AlgoHandle detector_handle, char* out_buffer, uint64_t* in_out_len);

	/*********************************************
	@brief   : 将buffer数据读取到算法内
	@note    :
	@param   : detector_handle     有效的检测句柄
	@param   : in_buffer               buffer
	@param   : in_len                  buffer大小
	@return  : AlgoError
	@Author  : wushaoshuang@2024/09/24
	*********************************************/
	C_API AlgoError C_CALL DieDefectFromBuffer(AlgoHandle detector_handle, const char* in_buffer, uint64_t in_len);

	/*********************************************
	@brief   : 载入defect文件
	@note    :
	@param   : detector_handle     有效的检测句柄
	@param   : in_load_path            输入的文件路径，后缀必须为.defect
	@return  : AlgoError
	@Author  : wushaoshuang@2024/09/24
	*********************************************/
	C_API AlgoError C_CALL DieDefectLoad(AlgoHandle detector_handle, const char* in_load_path);

	/*********************************************
	@brief   : 保存CameraSpeckle文件
	@note    :
	@param   : detector_handle     有效的检测句柄
	@param   : in_save_path            输入要保存的文件路径，后缀必须为.defect
	@return  : AlgoError
	@Author  : wushaoshuang@2024/09/24
	*********************************************/
	C_API AlgoError C_CALL DieDefectSave(AlgoHandle detector_handle, const char* in_save_path);

	/*********************************************
	@brief   : 空间内存释放
	@note    : 句柄指向的对象销毁，此句柄已经无效，请配合handle=nullptr
	@param   : detector_handle         有效的检测句柄
	@return  :
	@Author  : wushaoshuang@2024/09/24
	*********************************************/
	C_API void C_CALL DieDefectDestroyHandle(AlgoHandle detector_handle);

	/*********************************************
	@brief   : 句柄指向的对象的内存空间释放，句柄指向的对象还在
	@note    : 内存释放
	@param   : detector_handle         有效的检测句柄
	@return  : AlgoError
	@Author  : wushaoshuang@2024/09/24
	*********************************************/
	C_API AlgoError C_CALL DieDefectRelease(AlgoHandle detector_handle);


#ifdef __cplusplus
}
#endif // __cplusplus
#endif