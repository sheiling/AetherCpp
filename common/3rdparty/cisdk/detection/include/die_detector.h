#ifndef ____DIE_DETECTOR_H__
#define ____DIE_DETECTOR_H__

#include "ci_declare.h"
#include "ci_define.h"
#include "ci_data.h"
#include "die_detector_errors.h"
#include "die_detector_data.h"
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
    C_API AlgoHandle C_CALL DieCreateHandle();

	
	/************************************
	@brief    : 设置die类型信息
	@note     :
	@param    : die_handle        入参，有效的die句柄
	@param    : in_die_type       入参，die类型(R/G/B/MIP)
	@param    : in_die_size       入参，die的像素尺寸(像素间距),该入参可置为{-1.f,-1.f},当置为{-1.f,-1.f}时算法内部将自行计算die的像素尺寸
	@param    : in_die_pitch_gap  入参，die的水平与竖直间距（像素间距）【返修机不需要设置该参数】
	@return   : AlgoError
	@Modified : songshaojun@2023/03/06
	************************************/
	C_API AlgoError C_CALL DieSetTypeInfo(AlgoHandle die_handle, WaferDieRGBType in_die_type, const AlgoSize2f* in_die_size,const AlgoSize2f* in_die_pitch_gap);


	/************************************
	@brief    : 设置wafer上die检测相关阈值参数
	@note     :
	@param    : die_handle      入参，有效的die句柄
	@param    : in_thresh       入参，wafer上die检测相关阈值参数；
								in_match_thresh：float类型，代表匹配阈值，值越大，代表与模板越匹配；
								in_segment_thresh：int类型，
								1）当DieSetTypeInfo接口中入参in_die_type设置为WAFER_DIE_MIP时，该阈值代表芯片检测角度范围阈值，即只检测多少角度范围内的芯片，
								   该入参可设置为-1;当设置为-1时，算法内部默认为只检测±60度范围内的芯片；
								2）当DieSetTypeInfo接口中入参in_die_type设置不为WAFER_DIE_MIP时，该阈值代表芯片分割阈值；该入参可设置为-1,当设置为-1时，
								   算法内部将自动计算分割阈值；
								in_die_area_low_thresh：float类型，代表面积筛选的低阈值，分割面积小于该阈值乘以die像素尺寸时，该晶粒不予计算；该入参可设置为-1.f;当设置为-1.f时,算法内部将自动计算；
								in_die_area_high_thresh：float类型，代表面积筛选的高阈值，分割面积大于该阈值乘以die像素尺寸时，该晶粒不予计算；该入参可设置为-1.f;当设置为-1.f时,算法内部将自动计算
	@return   : AlgoError
	@Modified : songshaojun@2023/03/06
	************************************/
	C_API AlgoError C_CALL DieSetThresh(AlgoHandle die_handle, const DieDetectThresh* in_thresh);

		
	/************************************
	@brief    : 设置模板
	@note     :
	@param    : die_handle          有效的Die句柄
	@param    : model_handle        model句柄
	@param    : in_template_name    模板名字
	@return   : AlgoError
	@Modified : songshaojun@2023/03/06
	************************************/
	C_API AlgoError C_CALL DieSetTemplate(AlgoHandle die_handle, AlgoHandle model_handle, const char* in_template_name);


    /************************************
    @brief    : 载入die模板文件
    @note     : 
    @param    : die_handle               有效的die句柄
    @param    : in_die_template_files    die模板文件
	@param    : in_template_name         die模板名字
    @return   : AlgoError
    @Modified : songshaojun@2023/03/06
    ************************************/
    C_API AlgoError C_CALL DieLoadTemplate(AlgoHandle die_handle, const char* in_die_template_files, const char* in_template_name);


	/************************************
	@brief    : 设置检测的ROI，如果设置ROI，则只对ROI区域进行检测，没有设置ROI或ROI设置有误则对整张输入图像进行检测
	@note     :
	@param    : die_handle          入参，有效的die句柄
	@param    : in_roi_rect         入参，输入的roi
	@return   : AlgoError
	@Modified : songshaojun@2024/05/06
	************************************/
	C_API AlgoError C_CALL DieSetROI(AlgoHandle die_handle, const AlgoRect* in_roi_rect);  



	/************************************
	@brief    : 寻找wafer中die的中心（备注，至少有2个die，其中一个die的坐标保存至缓存中，指导运动）
	@note     : 当输入图像为单通道时，会使用返修机一代的wafer位置算法进行die中心定位，
				当输入图像为三通道时，会使用返修机二代的wafer位置算法进行die中心定位
	@param    : die_handle          有效的die句柄
	@param    : in_image            需要定位的图像
	@param    : out_die_center      视野中所有die中心与图像中心的像素偏移量（x=列，y=行）
	@return   : AlgoError
	@Modified : songshaojun@2023/03/06
	************************************/
	C_API AlgoError C_CALL DieLocate(AlgoHandle die_handle, const AlgoMat* in_image,
		AlgoMat* out_die_center);  



	/************************************
	@brief    : 中转位处die精细定位（只计算一个die） 
	@note     :
	@param    : die_handle          有效的die句柄
	@param    : in_image            需要定位的图像
	@param    : out_die_center      中转位die中心与图像中心的像素偏移量（x=列，y=行）
	@param    : out_die_angle       芯片的偏移角度，单位/°
	@return   : AlgoError
	@Modified : songshaojun@2023/03/06
	************************************/
	C_API AlgoError C_CALL DieInspect(AlgoHandle die_handle, const AlgoMat* in_image,
		 AlgoPoint2f* out_die_center, float* out_die_angle);   


	/************************************
    @brief    : 获取die的轮廓点集数量
    @note     :
    @param    : chip_handle          有效的die句柄
    @param    : out_len              die的轮廓点集的点数量
    @return   : AlgoError
    @Modified : songshaojun@2023/03/06
    ************************************/
	C_API AlgoError C_CALL DieGetBorderSize(AlgoHandle die_handle, int* out_len);  



	/************************************
	@brief    : 获取die的轮廓点集外接矩形
	@note     :
	@param    : die_handle           有效的die句柄
	@param    : in_len               die的轮廓点集的点数量
	@param    : out_die_border       die的轮廓点集，rect形式
	@return   : AlgoError
	@Modified : songshaojun@2023/03/06
	************************************/
	C_API AlgoError C_CALL DieGetBorderRect(AlgoHandle die_handle, int* in_len, AlgoRect* out_die_border); 


	/************************************
	@brief    : 输出检测的die的结果图,结果图上绘制有中心点和die的外接矩形
	@note     :
	@param    : die_handle          有效的die句柄
	@param    : out_png_filename    图像保存路径
	@return   : AlgoError
	@Modified : songshaojun@2023/03/06
	************************************/
	C_API AlgoError C_CALL DieExportResultPng(AlgoHandle die_handle, const char *out_png_filename); 


	/************************************
	@brief    : 以mat形式输出检测的die的结果图,结果图上绘制有中心点和die的外接矩形
	@note     :
	@param    : die_handle          有效的die句柄
	@param    : out_result_mat      mat图像
	@return   : AlgoError
	@Modified : haopanxia@2024/11/14
	************************************/
	C_API AlgoError C_CALL DieGetResultMat(AlgoHandle die_handle, AlgoMat* out_result_mat);


	/************************************
	@brief    : 获取buffer大小
	@note     :
	@param    : die_handle          有效的die句柄
	@param    : out_len             buffer大小
	@return   : AlgoError
	@Modified : songshaojun@2023/03/06
	************************************/
	C_API AlgoError C_CALL DieGetBufferSize(AlgoHandle die_handle, uint64_t* out_len);   


	/************************************
	@brief    : 将DieDetect里面的数据导出到buffer
	@note     :
	@param    : die_handle          有效的die句柄
	@param    : out_buffer          已经申请好申请空间
	@param    : in_out_len          外部申请好的空间大小,会用真实大小修改
	@return   : AlgoError
	@Modified : songshaojun@2023/03/06
	************************************/
	C_API AlgoError C_CALL DieToBuffer(AlgoHandle die_handle, char* out_buffer, uint64_t* in_out_len); 


	/************************************
	@brief    : 将buffer数据导入到DieDetect
	@note     :
	@param    : die_handle          有效的die句柄
	@param    : in_buffer
	@param    : in_len
	@return   : AlgoError
	@Modified : haopanxia@2022/09/30
	************************************/
	C_API AlgoError C_CALL DieFromBuffer(AlgoHandle die_handle, const char* in_buffer, uint64_t in_len); 


	//点测机相关----------------------------------------------------------------------------------
	/************************************
	@brief    : 设置wafer上die检测相关阈值参数
	@note     :
	@param    : die_handle      有效的die句柄
	@param    : in_thresh       wafer上die点测相关阈值参数；
								match_select_thold：float类型，代表模板匹配偏置值（用于扫直）；
								die_defect_score_thold：float类型，代表晶粒外观得分值，小于该值将被认为是破损晶粒；
								die_match_thold：float类型，代表匹配阈值，值越大，代表与模板越匹配；
								die_segment_thold：int类型，代表分割阈值的偏置值；
								die_area_low_thold：float类型，代表面积筛选的低阈值，分割面积小于该阈值乘以die像素尺寸时，该晶粒不予计算；
								die_area_high_thold：float类型，代表面积筛选的高阈值，分割面积大于该阈值乘以die像素尺寸时，该晶粒不予计算；
								object_segment_thold;    int类型，找到最近晶粒或目标点的背景分割阈值
								mark_die_match_thold;  float类型，对位点的筛选阈值
								ptn_w;  int类型，将对位点图像在水平方向上分为几份
								ptn_h;  int类型，将对位点图像在垂直方向上分为几份
	@return   : AlgoError
	@Modified : songshaojun@2023/05/22
	************************************/
	C_API AlgoError C_CALL DieProbeSetThresh(AlgoHandle die_handle, const DieProbeThresh* in_thresh);


	/************************************
	@brief    : 获取样本图像中，距离中心最近的晶粒或目标
	@note     :
	@param    : die_handle          有效的die句柄
	@param    : in_image            输入的样本图像  
	@param    : out_object_center   返回距离图像中心最近的晶粒或目标的中心，返回的x坐标、y坐标分别是相对于图像中心点的偏移量
	@return   : AlgoError
	@Modified : songshaojun@2023/05/22
	************************************/
	C_API AlgoError C_CALL DieProbeGetNearestObject(AlgoHandle die_handle, const AlgoMat* in_image,
		AlgoPoint2f* out_object_center);

	/************************************
	@brief    : 获取对位点（缺角晶粒或Mark晶粒）的中心点
	@note     :
	@param    : die_handle            有效的die句柄
	@param    : in_image              输入的样本图像
	@param    : out_mark_die_center   对位点的中心，返回的x坐标、y坐标分别是相对于图像中心点的偏移量          
	@return   : AlgoError
	@Modified : songshaojun@2023/05/22
	************************************/
	C_API AlgoError C_CALL DieProbeGetMarkCenter(AlgoHandle die_handle, const AlgoMat* in_image,
		AlgoPoint2f* out_mark_die_center);

	/************************************
	@brief    : 寻找wafer中die的中心，并进行初步的缺陷检测
	@note     :
	@param    : die_handle          有效的die句柄
	@param    : in_image            需要定位的图像
	@param    : out_die_center      输出为一个三通道mat，channel[0]、channel[1]中的数据为视野中所有die中心与图像中心的像素偏移量（x=列，y=行）
									channel[2]为die的好坏标签，1代表完好晶粒，-1代表有破损,0代表无晶粒；
	@return   : AlgoError
	@Modified : songshaojun@2023/04/26
	************************************/
	C_API AlgoError C_CALL DieProbeLocate(AlgoHandle die_handle, const AlgoMat* in_image,
		AlgoMat* out_die_center);

	/************************************
	@brief    : 计算wafer的角度
	@note     :
	@param    : die_handle          有效的die句柄
	@param    : in_image            输入的wafer图像；建议：晶粒尽量铺满图像，当晶粒较少时，计算得到的角度精度下降
	@param    : out_wafer_angle     输出为一个float类型的值，代表当前的wafer角度。
									其中：逆时针为负，顺时针为正
	@return   : AlgoError
	@Modified : songshaojun@2023/04/26
	************************************/
	C_API AlgoError C_CALL DieGetWaferAngle(AlgoHandle die_handle, const AlgoMat* in_image,
		float* out_wafer_angle);

	/************************************
	@brief    : 获取当前拍摄画面中，晶粒的排布,只是展示出更直观的检测结果，无法直接使用该接口进行map拼接
	@note     :
	@param    : die_handle          有效的die句柄
	@param    : in_image            输入的wafer图像；
	@param    : out_wafer_angle     输出AlgoMat
	@return   : AlgoError
	@Modified : songshaojun@2023/04/26
	************************************/
	C_API AlgoError C_CALL DieGetWaferMap(AlgoHandle die_handle, AlgoMat* out_wafer_map);


	/************************************
	@brief    : 获取buffer大小
	@note     :
	@param    : die_handle          有效的die句柄
	@param    : out_len             buffer大小
	@return   : AlgoError
	@Modified : songshaojun@2023/04/28
	************************************/
	C_API AlgoError C_CALL DieProbeGetBufferSize(AlgoHandle die_handle, uint64_t* out_len);


	/************************************
	@brief    : 将DieDetect里面的数据导出到buffer
	@note     :
	@param    : die_handle          有效的die句柄
	@param    : out_buffer          已经申请好申请空间
	@param    : in_out_len          外部申请好的空间大小,会用真实大小修改
	@return   : AlgoError
	@Modified : songshaojun@2023/04/28
	************************************/
	C_API AlgoError C_CALL DieProbeToBuffer(AlgoHandle die_handle, char* out_buffer, uint64_t* in_out_len);


	/************************************
	@brief    : 将buffer数据导入到DieDetect
	@note     :
	@param    : die_handle          有效的die句柄
	@param    : in_buffer
	@param    : in_len
	@return   : AlgoError
	@Modified : songshaojun@2023/04/28
	************************************/
	C_API AlgoError C_CALL DieProbeFromBuffer(AlgoHandle die_handle, const char* in_buffer, uint64_t in_len);



/************************************分选机（放置位）相关接口*****************/
	/************************************
	@brief    : 设置分割阈值
	@note     : 分选机放置位芯片检测分割阈值设置，外部根据情况调用，算法内部本身会自动生成阈值，若自动生成检测有误，需要外部调用该接口手动设置
	@param    : die_handle          有效的die句柄
	@param    : in_segment_thresh   输入的分割阈值
	@return   : AlgoError
	@Modified : wushaoshuang@2023/08/01
	************************************/
	C_API AlgoError C_CALL DieSortSetSegmentThresh(AlgoHandle die_handle, const int* in_segment_thresh);

	/************************************
	@brief    : 设置面积阈值
	@note     : 分选机放置位芯片检测面积阈值设置，该面积指芯片在图像中的像素面积大小
	@param    : die_handle          有效的die句柄
	@param    : in_min_area_thresh  输入的最小面积阈值，表示目标面积最小不低于该值
	@param    : in_max_area_thresh  输入的最大面积阈值，表示目标面积最大不超过该值
	@return   : AlgoError
	@Modified : wushaoshuang@2023/08/01
	************************************/
	C_API AlgoError C_CALL DieSortSetAreaThresh(AlgoHandle die_handle, const int* in_min_area_thresh, const int* in_max_area_thresh);

	/************************************
	@brief    : 获取分割阈值（根据需求调用）
	@note     : 获取算法内部的分割阈值，外部根据情况调用
	@param    : die_handle          有效的die句柄
	@param    : out_segment_thresh  输出的分割阈值
	@return   : AlgoError
	@Modified : wushaoshuang@2023/08/01
	************************************/
	C_API AlgoError C_CALL DieSortGetSegmentThresh(AlgoHandle die_handle, int* out_segment_thresh);

	/************************************
	@brief    : 获取面积阈值（根据需求调用）
	@note     : 获取算法内部的面积阈值，外部根据情况调用
	@param    : die_handle          有效的die句柄
	@param    : out_min_area_thresh 输出的最小面积阈值
	@param    : out_max_area_thresh 输出的最小面积阈值
	@return   : AlgoError
	@Modified : wushaoshuang@2023/08/01
	************************************/
	C_API AlgoError C_CALL DieSortGetAreaThresh(AlgoHandle die_handle, int* out_min_area_thresh, int* out_max_area_thresh);

	/************************************
	@brief    : 芯片检测主接口
	@note     : 芯片检测主接口,检测结果以一个三通道的1行n列的mat保存，n表示有n个芯片被检测到，mat的三个通道数据依次保存芯片的x坐标、y坐标、旋转角度。（原点在图像中心）
	@param    : die_handle          有效的die句柄
	@param    : in_image            输入的待检测图像
	@param    : in_roi              输入的待检测ROI
	@param    : out_result          输出的检测结果，以一个三通道的1行n列的mat保存，n表示有n个芯片被检测到，mat的三个通道数据依次保存芯片的x坐标、y坐标、旋转角度。
	@return   : AlgoError
	@Modified : wushaoshuang@2023/08/01
	************************************/
	C_API AlgoError C_CALL DieSortDetect(AlgoHandle die_handle, AlgoMat* in_image, AlgoRect* in_roi, AlgoMat* out_result);

	/************************************
	@brief    : 导出检测结果图像（根据需求调用）
	@note     : 导出检测结果图像，外部根据需求调用，图像仅供人眼观测参考
	@param    : die_handle          有效的die句柄
	@param    : in_png_path         输入的图像存储路径
	@return   : AlgoError
	@Modified : wushaoshuang@2023/08/01
	************************************/
	C_API AlgoError C_CALL DieSortExportPng(AlgoHandle die_handle, const char* in_png_path);

	/************************************
	@brief    : 获取定位轮廓大小（根据需求调用）
	@note     : 获取定位轮廓大小（根据需求调用），轮廓仅供参考
	@param    : die_handle          有效的die句柄
	@param    : out_len             输出的轮廓大小
	@return   : AlgoError
	@Modified : wushaoshuang@2023/08/01
	************************************/
	C_API AlgoError C_CALL DieSortGetBorderSize(AlgoHandle die_handle, int* out_len);

	/************************************
	@brief    : 获取定位轮廓（根据需求调用）
	@note     : 获取定位轮廓（根据需求调用），轮廓仅供参考,该轮廓以4个点为一组属于一个芯片的轮廓，请以4的倍数来获取同一芯片的轮廓
	@param    : die_handle          有效的die句柄
	@param    : out_border          输出的轮廓,需要外部根据BorderSize来new内存空间
	@return   : AlgoError
	@Modified : wushaoshuang@2023/08/01
	************************************/
	C_API AlgoError C_CALL DieSortGetBorder(AlgoHandle die_handle, AlgoPoint2f* out_border);

	/*********************************************
	@brief   : 获取buffer大小
	@note    : 
	@param   : die_handle              有效的die句柄
	@param   : out_len                 buffer大小
	@return  : AlgoError
	@Author  : wushaoshuang@2024/08/01
	*********************************************/
	C_API AlgoError C_CALL DieSortGetBufferSize(AlgoHandle die_handle, uint64_t* out_len);

	/*********************************************
	@brief   : 将数据导入buffer
	@note    :
	@param   : die_handle              有效的die句柄
	@param   : out_buffer              输出检测参数的buffer空间
	@param   : in_out_len              buffer空间大小
	@return  : AlgoError
	@Author  : wushaoshuang@2024/08/01
	*********************************************/
	C_API AlgoError C_CALL DieSortToBuffer(AlgoHandle die_handle, char* out_buffer, uint64_t* in_out_len);

	/*********************************************
	@brief   : 将buffer数据读取到DieSort
	@note    :
	@param   : die_handle              有效的die句柄
	@param   : in_buffer               buffer
	@param   : in_len                  buffer大小
	@return  : AlgoError
	@Author  : wushaoshuang@2024/08/01
	*********************************************/
	C_API AlgoError C_CALL DieSortFromBuffer(AlgoHandle die_handle, const char* in_buffer, uint64_t in_len);

	/*********************************************
	@brief   : 载入DieSort文件
	@note    :
	@param   : die_handle              有效的die句柄
	@param   : in_load_path            输入的文件路径，后缀必须为.diesort
	@return  : AlgoError
	@Author  : wushaoshuang@2024/08/01
	*********************************************/
	C_API AlgoError C_CALL DieSortLoad(AlgoHandle die_handle, const char* in_load_path);

	/*********************************************
	@brief   : 保存CameraSpeckle文件
	@note    :
	@param   : die_handle              有效的die句柄
	@param   : in_save_path            输入要保存的文件路径，后缀必须为.diesort
	@return  : AlgoError
	@Author  : wushaoshuang@2024/08/01
	*********************************************/
	C_API AlgoError C_CALL DieSortSave(AlgoHandle die_handle, const char* in_save_path);




/************************************极性检测相关接口*****************/
	/*********************************************
	@brief   : 设置极性匹配度阈值
	@note    : 设置极性检测匹配度阈值，即匹配程度的大小，认为匹配度大于阈值则为目标，小于则不是。默认阈值为0.9
	@param   : die_handle              有效的die句柄
	@param   : in_match_thresh         输入的匹配度阈值
	@return  : AlgoError
	@Author  : wushaoshuang@2024/08/05
	*********************************************/
	C_API AlgoError C_CALL DiePolarSetMatchThresh(AlgoHandle die_handle, const float* in_match_thresh);

	/*********************************************
	@brief   : 设置芯片宽高
	@note    : 设置芯片宽高，可以是上游定位输出的宽高，也可以是外部人工手动输入的宽高，注：宽高一律为像素宽高
	@param   : die_handle          有效的die句柄
	@param   : in_die_size         输入的芯片宽高
	@return  : AlgoError
	@Author  : wushaoshuang@2024/08/05
	*********************************************/
	C_API AlgoError C_CALL DiePolarSetDieSize(AlgoHandle die_handle, const AlgoSize* in_die_size);

	/*********************************************
	@brief   : 设置正常极性芯片模板,从model文件设置模板
	@note    : 设置正常极性芯片模板，即需要输入认为极性正常的芯片（极性标识清晰，极性顺序正确）,以model形式载入
	@param   : die_handle          有效的die句柄
	@param   : model_handle        输入的模板handle
	@param   : in_template_name    输入的模板名称
	@param   : in_polar_roi        输入的芯片极性roi，即需要输入芯片上两个极性的ROI，以一个AlgoRect的数组输入。ROI数量必须为2个
	@param   : in_polar_lay_type   输入的芯片极性排布方式，详见die_detector_data.h中的DiePolarLayType结构体
	@return  : AlgoError
	@Author  : wushaoshuang@2024/08/05
	*********************************************/
	C_API AlgoError C_CALL DiePolarSetDieTemplateFromModel(AlgoHandle die_handle, AlgoHandle model_handle, const char* in_template_name, const AlgoRect* in_polar_roi, const DiePolarLayType* in_polar_lay_type);

	/*********************************************
	@brief   : 设置正常芯片模板,从图像设置模板
	@note    : 设置正常芯片模板，即需要输入认为极性正常的芯片（极性标识清晰，极性顺序正确）,以AlgoMat形式载入
	@param   : die_handle          有效的die句柄
	@param   : in_die_template     输入的模板
	@param   : in_polar_roi        输入的芯片极性roi，即需要输入芯片上两个极性的ROI，以一个AlgoRect的数组输入。ROI数量必须为2个
	@param   : in_polar_lay_type   输入的芯片极性排布方式，详见die_detector_data.h中的DiePolarLayType结构体
	@return  : AlgoError
	@Author  : wushaoshuang@2024/08/05
	*********************************************/
	C_API AlgoError C_CALL DiePolarSetDieTemplateFromMat(AlgoHandle die_handle, AlgoMat* in_die_template, const AlgoRect* in_polar_roi, const DiePolarLayType* in_polar_lay_type);

	/*********************************************
	@brief   : 设置极性模板,从model文件设置模板
	@note    : 设置极性模板，即需要输入芯片的两个极性模板（极性标识清晰，极性顺序正确）,以model形式载入
	@param   : die_handle          有效的die句柄
	@param   : model_handle        输入的模板handle
	@param   : in_polar_1_name     输入的极性1模板名称
	@param   : in_polar_2_name     输入的极性2模板名称
	@param   : in_polar_lay_type   输入的芯片极性排布方式，详见die_detector_data.h中的DiePolarLayType结构体
	@return  : AlgoError
	@Author  : wushaoshuang@2024/08/21
	*********************************************/
	C_API AlgoError C_CALL DiePolarSetPolarTemplateFromModel(AlgoHandle die_handle, AlgoHandle model_handle, const char* in_polar_1_name, const char* in_polar_2_name, const DiePolarLayType* in_polar_lay_type);

	/*********************************************
	@brief   : 设置极性模板,从Mat图像中设置模板
	@note    : 设置极性模板，即需要输入芯片的两个极性模板图像（极性标识清晰），输入极性图像时按照极性顺序输入，极性1为左极性，极性2为右极性
	@param   : die_handle          有效的die句柄
	@param   : in_polar1_template  输入的极性1图像
	@param   : in_polar2_template  输入的极性2图像
	@param   : in_polar_lay_type   输入的芯片极性排布方式，详见die_detector_data.h中的DiePolarLayType结构体
	@return  : AlgoError
	@Author  : wushaoshuang@2024/08/21
	*********************************************/
	C_API AlgoError C_CALL DiePolarSetPolarTemplateFromMat(AlgoHandle die_handle, AlgoMat* in_polar1_template, AlgoMat* in_polar2_template, const DiePolarLayType* in_polar_lay_type);

	/*********************************************
	@brief   : 芯片极性检测
	@note    : 芯片极性检测主接口，检测结果以一个1行n列的单通道mat存储，n表示有n个芯片被检测，mat的值与DiePolarDetectResult结构体枚举值对应
	@param   : die_handle          有效的die句柄
	@param   : in_image            输入的待检测图像
	@param   : in_locate_data      输入的芯片位置数据,数据类型见结构体DiePolarInputData，详情见die_detector_data.h，需要外部new空间，以DiePolarInputData类型的数组传入
	@param   : in_locate_data_len  输入的待检测芯片的数量
	@param   : out_detect_result   输出的检测结果
	@return  : AlgoError
	@Author  : wushaoshuang@2024/08/05
	*********************************************/
	C_API AlgoError C_CALL DiePolarDetect(AlgoHandle die_handle, AlgoMat* in_image, DiePolarInputData* in_locate_data, const int in_locate_data_len, AlgoMat* out_detect_result);

	/*********************************************
	@brief   : 导出检测结果图像
	@note    : 导出检测结果图像，外部根据需求调用，图像仅供参考
	@param   : die_handle          有效的die句柄
	@param   : in_png_path         输入的图像存储路径
	@return  : AlgoError
	@Author  : wushaoshuang@2024/08/05
	*********************************************/
	C_API AlgoError C_CALL DiePolarExportPng(AlgoHandle die_handle, const char* in_png_path);

	/*********************************************
	@brief   : 获取极性匹配度阈值
	@note    : 获取极性检测匹配度阈值，外部根据需求调用
	@param   : die_handle              有效的die句柄
	@param   : out_match_thresh         输出的匹配度阈值
	@return  : AlgoError
	@Author  : wushaoshuang@2024/08/05
	*********************************************/
	C_API AlgoError C_CALL DiePolarGetMatchThresh(AlgoHandle die_handle, float* out_match_thresh);

	/*********************************************
	@brief   : 获取芯片宽高
	@note    : 获取芯片宽高，外部根据需求调用
	@param   : die_handle              有效的die句柄
	@param   : out_die_size            输出的芯片宽高
	@return  : AlgoError
	@Author  : wushaoshuang@2024/08/05
	*********************************************/
	C_API AlgoError C_CALL DiePolarGetDieSize(AlgoHandle die_handle, AlgoSize* out_die_size);

	/*********************************************
	@brief   : 获取正常极性芯片模板
	@note    : 获取正常极性芯片模板，以AlgoMat类型输出，与DiePolarSetDieTemplateFromModel或DiePolarSetDieTemplateFromMat接口配合使用
	@param   : die_handle          有效的die句柄
	@param   : out_template        输出的模板图像
	@param   : out_polar_roi       输出的极性roi，以AlgoRect的数组输出，需要外部new 2个AlgoRect的空间
	@param   : out_polar_lay_type  输出的芯片极性排布方式，详见die_detector_data.h中的DiePolarLayType结构体
	@return  : AlgoError
	@Author  : wushaoshuang@2024/08/05
	*********************************************/
	C_API AlgoError C_CALL DiePolarGetDieTemplate(AlgoHandle die_handle, AlgoMat* out_template, AlgoRect* out_polar_roi, DiePolarLayType* out_polar_lay_type);

	/*********************************************
	@brief   : 获取极性模板图像
	@note    : 获取极性模板图像，以两个AlgoMat类型输出，与DiePolarSetPolarTemplateFromModel或DiePolarSetPolarTemplateFromMat接口配合使用
	@param   : die_handle          有效的die句柄
	@param   : out_polar1_template        输出的模板图像
	@param   : out_polar2_template        输出的模板图像
	@param   : out_polar_lay_type  输出的芯片极性排布方式，详见die_detector_data.h中的DiePolarLayType结构体
	@return  : AlgoError
	@Author  : wushaoshuang@2024/08/21
	*********************************************/
	C_API AlgoError C_CALL DiePolarGetPolarTemplate(AlgoHandle die_handle, AlgoMat* out_polar1_template, AlgoMat* out_polar2_template, DiePolarLayType* out_polar_lay_type);

	/*********************************************
	@brief   : 获取buffer大小
	@note    :
	@param   : die_handle              有效的die句柄
	@param   : out_len                 buffer大小
	@return  : AlgoError
	@Author  : wushaoshuang@2024/08/05
	*********************************************/
	C_API AlgoError C_CALL DiePolarGetBufferSize(AlgoHandle die_handle, uint64_t* out_len);

	/*********************************************
	@brief   : 将数据导入buffer
	@note    :
	@param   : die_handle              有效的die句柄
	@param   : out_buffer              输出检测参数的buffer空间
	@param   : in_out_len              buffer空间大小
	@return  : AlgoError
	@Author  : wushaoshuang@2024/08/05
	*********************************************/
	C_API AlgoError C_CALL DiePolarToBuffer(AlgoHandle die_handle, char* out_buffer, uint64_t* in_out_len);

	/*********************************************
	@brief   : 将buffer数据读取到DieSort
	@note    :
	@param   : die_handle              有效的die句柄
	@param   : in_buffer               buffer
	@param   : in_len                  buffer大小
	@return  : AlgoError
	@Author  : wushaoshuang@2024/08/05
	*********************************************/
	C_API AlgoError C_CALL DiePolarFromBuffer(AlgoHandle die_handle, const char* in_buffer, uint64_t in_len);

	/*********************************************
	@brief   : 载入DieSort文件
	@note    :
	@param   : die_handle              有效的die句柄
	@param   : in_load_path            输入的文件路径，后缀必须为.polar
	@return  : AlgoError
	@Author  : wushaoshuang@2024/08/05
	*********************************************/
	C_API AlgoError C_CALL DiePolarLoad(AlgoHandle die_handle, const char* in_load_path);

	/*********************************************
	@brief   : 保存CameraSpeckle文件
	@note    :
	@param   : die_handle              有效的die句柄
	@param   : in_save_path            输入要保存的文件路径，后缀必须为.polar
	@return  : AlgoError
	@Author  : wushaoshuang@2024/08/05
	*********************************************/
	C_API AlgoError C_CALL DiePolarSave(AlgoHandle die_handle, const char* in_save_path);





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
    C_API AlgoError C_CALL DieRelease(AlgoHandle die_handle);
	C_API void C_CALL DieDestroyHandle(AlgoHandle die_handle);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif