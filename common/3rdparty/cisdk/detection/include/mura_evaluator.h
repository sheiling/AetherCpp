#ifndef __MURA_EVALUATOR_H__
#define __MURA_EVALUATOR_H__

#include "ci_data.h"
#include "ci_declare.h"
#include "mura_data.h"
#include "mura_error.h"
#include "algo_mat.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

	/****************************************
	@brief   : 创建MuraEvaluate句柄
	@note    :
	@param   : mura_handle
	@return  : AlgoError
	@Author  : wushaoshuang@2023/06/01
	*****************************************/
	C_API AlgoError C_CALL MuraEvaluateCreateHandle(AlgoHandle* mura_handle);

	/***************************************
	@brief   : 设置Mura测评阈值，根据实际检测场景和需求设置
	@note    : mura测评的一些阈值
	@param   : mura_handle       有效的mura测评句柄
	@param   : in_thresh         mura测评相关阈值
	@return  : AlgoError
	@Author  : wushaoshuang@2023/06/01
	****************************************/
	C_API AlgoError C_CALL MuraEvaluateSetThresh(AlgoHandle mura_handle, MuraThreshold* in_thresh);

	/***************************************
	@brief   : 设置Mura测评参数
	@note    : 内部检测出mura时需要的参数,根据实际检测场景和需求设置
	@param   : mura_handle           有效的mura测评句柄
	@param   : in_detect_param       mura测评中检测mura的参数
	@return  : AlgoError
	@Author  : wushaoshuang@2023/06/01
	****************************************/
	C_API AlgoError C_CALL MuraEvaluateSetDetectParam(AlgoHandle mura_handle, MuraDetectParam* in_detect_param);

	/***************************************
	@brief   : 设置Mura拍摄参数
	@note    : 相机在拍摄mura时的参数，包括拍摄距离及屏体信息等
	@param   : mura_handle           有效的mura测评句柄
	@param   : in_capture_param      相机拍摄时的mura参数
	@return  : AlgoError
	@Author  : wushaoshuang@2023/06/01
	****************************************/
	C_API AlgoError C_CALL MuraEvaluateSetCaptureParam(AlgoHandle mura_handle, MuraCaptureParam* in_capture_param);

	/***************************************
	@brief   : 设置Mura评价权重
	@note    : 评价mura时所需的权重，如不设置，内部默认仅使用score参数来评价
	@param   : mura_handle            有效的mura测评句柄
	@param   : in_evaluate_weight     评价权重，决定评价参数选择，评价参数 = k1*score+k2*contrast+k3*semu+k4*dsemu+k5*area+k6*location+k7*shape
	@return  : AlgoError
	@Author  : wushaoshuang@2023/06/01
	****************************************/
	C_API AlgoError C_CALL MuraEvaluateSetWeight(AlgoHandle mura_handle, MuraEvaluWeight* in_evaluate_weight);

	/***************************************
	@brief   : 设置待检测数据
	@note    : 传入算法内部测评的数据、数据类型及角点坐标，raw数据或meter数据，内部默认为raw数据，除meter数据外，其余数据均可设置为raw数据
	@param   : mura_handle           有效的mura测评句柄
	@param   : in_image              输入的待测评数据，为AlgoMat类型
	@param   : in_data_type          输入的数据类型枚举，详见mura_data.h中的枚举类型，raw数据或meter数据，内部默认为raw数据
	@param   : in_corner_point[4]    输入的四个角点信息
	@param   : in_method             输入的测评类型枚举, 内部默认为单通道灰度mura测评，详见mura_data.h中的MuraDetectMethod枚举类型
	@return  : AlgoError
	@Author  : wushaoshuang@2023/06/05
	****************************************/
	C_API AlgoError C_CALL MuraEvaluateSetInputData(AlgoHandle mura_handle, AlgoMat* in_image, MuraDataType* in_data_type, AlgoPoint in_corner_point[4], MuraDetectMethod* in_method);


	/***************************************
	@brief   : 设置绘制测评结果的图像颜色———可选调用（仅供测试使用）
	@note    : 设置绘制测评结果的图像颜色，内部将检测的mura绘制在该颜色的图像上，便于外部显化观测mura，仅供观测测试
	@param   : mura_handle           有效的mura测评句柄
	@param   : in_color              输入的颜色枚举,详见mura_data.h中的ExportImgColor枚举类型
	@return  : AlgoError
	@Author  : wushaoshuang@2023/06/01
	****************************************/
	C_API AlgoError C_CALL MuraEvaluateSetDrawColor(AlgoHandle mura_handle, ExportImgColor* in_color);

	/***************************************
	@brief   : 设置色差计算所需的参照值———可选调用
	@note    : 内部计算色差时需要一组参照值，外部可选择调用，如不调用，内部有默认的参照值（全局均值）
	@param   : mura_handle           有效的mura测评句柄
	@param   : in_DE_refer           输入的参考值结构体
	@return  : AlgoError
	@Author  : wushaoshuang@2023/06/01
	****************************************/
	C_API AlgoError C_CALL MuraEvaluateSetDeltaERefer(AlgoHandle mura_handle, DeltaERefer* in_DE_refer);

	/***************************************
	@brief   : 加载配置参数，从本地yml文件中载入———可选调用
	@note    : 该函数未调用时，算法内部采用默认值
	@param   : mura_handle           有效的mura测评句柄
	@param   : in_yml_path           yml文件完整文件名，含路径
	@return  : AlgoError
	@Author  : wushaoshuang@2023/06/04
	****************************************/
	C_API AlgoError C_CALL MuraEvaluateLoadYML(AlgoHandle mura_handle,const char* in_yml_path);

	/***************************************
	@brief   : 计算mura特征———可选调用
	@note    : 计算mura特征接口，仅供外部需要计算mura特征时使用，获取计算结果时，调用获取mura特征接口获取结果即可
	@param   : mura_handle           有效的mura测评句柄
	@return  : AlgoError
	@Author  : wushaoshuang@2023/06/01
	****************************************/
	C_API AlgoError C_CALL MuraEvaluateCalcFeature(AlgoHandle mura_handle);

	/***************************************
	@brief   : mura测评主接口
	@note    : mura测评主接口，获取测评结果时通过相应的获取接口获得
	@param   : mura_handle           有效的mura测评句柄
	@return  : AlgoError
	@Author  : wushaoshuang@2023/06/01
	****************************************/
	C_API AlgoError C_CALL MuraEvaluate(AlgoHandle mura_handle);

	/***************************************
	@brief   : 获取mura全屏综合评价值
	@note    : 获取mura全局的评价值
	@param   : mura_handle           有效的mura测评句柄
	@param   : out_mura_grade        输出的全局mura评价结果
	@return  : AlgoError
	@Author  : wushaoshuang@2023/06/01
	****************************************/
	C_API AlgoError C_CALL MuraEvaluateGetGrade(AlgoHandle mura_handle, MuraEvaluation* out_mura_grade);

	/***************************************
	@brief   : 获取mura的特征———可选调用 
	@note    : 获取mura的特征，可根据需要调用
	@param   : mura_handle           有效的mura测评句柄
	@param   : out_mura_feature      输出的mura特征
	@param   : in_num                new的内存空间大小（mura数量）
	@return  : AlgoError
	@Author  : wushaoshuang@2023/06/01
	****************************************/
	C_API AlgoError C_CALL MuraEvaluateGetFeature(AlgoHandle mura_handle, MuraFeature* out_mura_feature, int in_num);

	/***************************************
	@brief   : 获取mura测评的评价值———可选调用
	@note    : 获取每个mura的评价值，该值与mura评价权重有关，可根据需要调用
	@param   : mura_handle           有效的mura测评句柄
	@param   : out_result            输出每个mura的评价结果，需要在外部new分配内存
	@param   : in_num                new的内存空间大小（mura数量）
	@return  : AlgoError
	@Author  : wushaoshuang@2023/06/01
	****************************************/
	C_API AlgoError C_CALL MuraEvaluateGetResultData(AlgoHandle mura_handle, float* out_result, int in_num);

	/***************************************
	@brief   : 获取mura描述因子———可选调用
	@note    : 该数据主要用于绘图，例如：质心坐标、mura外接矩形、外接圆、边缘点集合等，详见mura_data.h，可根据需要调用
	@param   : mura_handle           有效的mura测评句柄
	@param   : out_description       需要在外部new分配内存
	@param   : in_num                new的内存空间大小（mura数量）
	@return  : AlgoError
	@Author  : wushaoshuang@2023/06/01
	****************************************/
	C_API AlgoError C_CALL MuraEvaluateGetDescription(AlgoHandle mura_handle, MuraDescription* out_description, int in_num);

	/***************************************
	@brief   : 获取mura轮廓———可选调用
	@note    : out_contours中单独存储mura边缘点集合，二维数组，第一行存储x，第二行存储y，与MuraDescription中point_list一致，二选一即可可根据需要调用
	@param   : mura_handle           有效的mura测评句柄
	@param   : out_contours          需要在外部new分配内存
	@param   : in_num                new的内存空间大小（mura数量）
	@return  : AlgoError
	@Author  : wushaoshuang@2023/06/01
	****************************************/
	C_API AlgoError C_CALL MuraEvaluateGetContours(AlgoHandle mura_handle, AlgoMat* out_contours, int in_num);

	/***************************************
	@brief   : 获取mura数量———可选调用
	@note    : 获取mura的数量，可根据数量开辟空间存储mura特征等，可根据需要调用
	@param   : mura_handle           有效的mura测评句柄
	@param   : out_mura_num          输出的mura数量
	@return  : AlgoError
	@Author  : wushaoshuang@2023/06/01
	****************************************/
	C_API AlgoError C_CALL MuraEvaluateGetMuraNum(AlgoHandle mura_handle, int* out_mura_num);

	/***************************************
	@brief   : 获取mura测评权重————可选调用
	@note    : 获取mura测评权重参数
	@param   : mura_handle           有效的mura测评句柄
	@param   : out_mura_weight       输出mura测评权重结构体
	@return  : AlgoError
	@Author  : wushaoshuang@2023/06/01
	****************************************/
	C_API AlgoError C_CALL MuraEvaluateGetWeight(AlgoHandle mura_handle, MuraEvaluWeight* out_mura_weight);

	/***************************************
	@brief   : 获取mura测评阈值————可选调用
	@note    : 获取mura测评阈值，可根据需要调用
	@param   : mura_handle           有效的mura测评句柄
	@param   : out_mura_thresh       输出mura测评阈值结构体
	@return  : AlgoError
	@Author  : wushaoshuang@2023/06/01
	****************************************/
	C_API AlgoError C_CALL MuraEvaluateGetThresh(AlgoHandle mura_handle, MuraThreshold* out_mura_thresh);

	/***************************************
	@brief   : 获取mura测评参数————可选调用
	@note    : 获取检测出mura的相关参数，可根据需要调用
	@param   : mura_handle           有效的mura测评句柄
	@param   : out_detect_param      输出mura检测参数结构体
	@return  : AlgoError
	@Author  : wushaoshuang@2023/06/01
	****************************************/
	C_API AlgoError C_CALL MuraEvaluateGetDetectParam(AlgoHandle mura_handle, MuraDetectParam* out_detect_param);

	/***************************************
	@brief   : 获取mura拍摄参数————可选调用
	@note    : 获取mura拍摄的相关参数，可根据需要调用
	@param   : mura_handle           有效的mura测评句柄
	@param   : out_capture_param     输出mura拍摄参数结构体
	@return  : AlgoError
	@Author  : wushaoshuang@2023/06/01
	****************************************/
	C_API AlgoError C_CALL MuraEvaluateGetCaptureParam(AlgoHandle mura_handle, MuraCaptureParam* out_capture_param);

	/***************************************
	@brief   : 获取mura测评的数据类型————可选调用
	@note    : 获取mura测评的数据类型，可根据需要调用
	@param   : mura_handle           有效的mura测评句柄
	@param   : out_date_type         输出mura测评的数据类型枚举
	@return  : AlgoError
	@Author  : wushaoshuang@2023/06/01
	****************************************/
	C_API AlgoError C_CALL MuraEvaluateGetDataType(AlgoHandle mura_handle, MuraDataType* out_date_type);

	/***************************************
	@brief   : 获取mura测评类型————可选调用
	@note    : 获取mura测评的类型，可根据需要调用
	@param   : mura_handle           有效的mura测评句柄
	@param   : out_method            输出mura测评模式枚举
	@return  : AlgoError
	@Author  : wushaoshuang@2023/06/01
	****************************************/
	C_API AlgoError C_CALL MuraEvaluateGetMethod(AlgoHandle mura_handle, MuraDetectMethod* out_method);

	/***************************************
	@brief   : 获取色差计算的参照值————可选调用
	@note    : 获取色差计算的参照值，可根据需要调用
	@param   : mura_handle           有效的mura测评句柄
	@param   : out_refer             输出色差计算的参照值结构体
	@return  : AlgoError
	@Author  : wushaoshuang@2023/06/01
	****************************************/
	C_API AlgoError C_CALL MuraEvaluateGetDeltaERefer(AlgoHandle mura_handle, DeltaERefer* out_refer);

	/***************************************
	@brief   : 导出mura测评结果图像————可选调用
	@note    : 导出mura测评结果图像，可根据需要调用，仅供测试使用
	@param   : mura_handle           有效的mura测评句柄
	@param   : in_img_path           输入导出图像的路径
	@param   : in_img_filename       输入导出图像的文件名（不包含后缀，内部设定后缀为bmp）
	@param   :in_threshold           输出结果控制阈值，将检测结果中score>in_threshold的mura绘制在图像中,默认为0
	@return  : AlgoError
	@Author  : wushaoshuang@2023/06/01
	****************************************/
	C_API AlgoError C_CALL MuraEvaluateExportImage(AlgoHandle mura_handle, const char* in_img_path, const char* in_img_filename, float in_threshold = 0);

	/***************************************
	@brief   : 导出mura测评结果数据至csv文件————可选调用
	@note    : 导出mura测评结果数据至csv文件，可根据需要调用，仅供测试使用
	@param   : mura_handle           有效的mura测评句柄
	@param   : in_csv_path           输入导出表格的路径
	@param   : in_filename       输入导出表格的文件名（不包含后缀名）
	@param   : in_threshold          输出结果控制阈值，将检测结果中score>in_threshold的mura导出,默认为0
	@return  : AlgoError
	@Author  : wushaoshuang@2023/06/01
	****************************************/
	C_API AlgoError C_CALL MuraEvaluateExportToCSV(AlgoHandle mura_handle, const char* in_csv_path, const char* in_filename, float in_threshold = 0);

	/***************************************
	@brief   : 均匀性检测
	@note    : 背光检测中的均匀性检测,输入数据必须为XYZ（曲面修正后的数据，内部不进行曲面修正），内部包括全局均匀性检测、分区均匀性检测、多区域均匀性检测、九点均匀性检测及异常点检测，可根据业务情况选择调用
	@param   : mura_handle           有效的mura测评句柄
	@param   : in_xyz_data           输入XYZ数据,为一个三通道的AlgoMat数据(可以配合MuraEvaluateMergeData生成),曲面修正请直接调用曲面修正模块完成
	@param   : in_corner_point       输入XYZ数据的四个角点信息
	@param   : in_partition_info     输入分区检测信息PartitionInfo结构体
	@param   : in_region_info        输入多区域检测信息，多个AlgoRect类型
	@param   : in_region_num         输入多区域检测的区域数量，即AlgoRect类型的数量
	@param   : in_point_position     输入的九点检测信息PointPositionInfo结构体
	@param   : dim_thresh            输入的异常点检测暗点阈值，可根据需要设置，内部默认为0，外部若不输入则表示不需要异常点检测，一般光通量图检测时需要
	@param   : bright_thresh         输入的异常点检测亮点阈值，可根据需要设置，内部默认为0，外部若不输入则表示不需要异常点检测，一般光通量图检测时需要
	@return  : AlgoError
	@Author  : wushaoshuang@2023/06/06
	****************************************/
	C_API AlgoError C_CALL MuraEvaluateDetectUniformity(AlgoHandle mura_handle, AlgoMat* in_XYZ_data, AlgoPoint in_corner_point[4], PartitionInfo* in_partition_info, CircularInfo* in_region_info, int in_region_num, PointPositionInfo* in_point_position, float* dim_thresh = 0, float* bright_thresh = 0);

	/***************************************
	@brief   : 获取均匀性检测结果
	@note    : 获取MuraEvaluateDetectUniformity检测的结果，内部包括全局均匀性检测结果、分区均匀性检测结果、多区域均匀性检测结果、九点均匀性检测及异常点检测结果，仅与MuraEvaluateDetectUniformity配合调用
	@param   : mura_handle              有效的mura测评句柄
	@param   : out_global_result        输出的全局均匀性检测结果
	@param   : out_partition_result     输出的分区均匀性检测结果
	@param   : out_multiRegion_result   输出的任意区均匀性检测结果
	@param   : out_ninepoint_result     输出的九点均匀性检测结果
	@param   : out_abnormal_result      输出的异常点检测结果，若调用MuraEvaluateDetectUniformity接口时未设置亮暗阈值（即不进行异常点检测），该参数仍然要输出，外部不使用即可
	@return  : AlgoError
	@Author  : wushaoshuang@2023/06/06
	****************************************/
	C_API AlgoError C_CALL MuraEvaluateGetUniformityResult(AlgoHandle mura_handle, GlobalResult* out_global_result, PartitionResult* out_partition_result, MultiRegionResult* out_multiRegion_result, NinePointResult* out_ninepoint_result, AbnormalPointResult* out_abnormal_result);

	/***************************************
	@brief   : 计算全屏均匀性
	@note    : 背光检测中的全屏均匀性检测,输入数据必须为XYZ（曲面修正后的数据，内部不进行曲面修正，外部可调用相关模块做曲面修正）
	@param   : mura_handle           有效的mura测评句柄
	@param   : in_xyz_data           输入XYZ数据,类型为三通道的AlgoMat类型(可配合MuraEvaluateMergeData生成)，曲面修正请直接调用曲面修正模块完成
	@param   : in_corner_point       输入XYZ数据的四个角点信息
	@param   : out_result            输出的全屏均匀性检测结果结构体
	@param   : in_downsamp_ratio     输入的图像数据下采样比例，默认为5
	@return  : AlgoError
	@Author  : wushaoshuang@2023/06/01
	****************************************/
	C_API AlgoError C_CALL MuraEvaluateCalcUniformity(AlgoHandle mura_handle, AlgoMat* in_xyz_data, AlgoPoint in_corner_point[4], GlobalResult* out_result, int in_downsamp_ratio = 5);

	/***************************************
	@brief   : 分区计算均匀性
	@note    : 背光检测中的分区均匀性检测,输入数据必须为XYZ（曲面修正后的数据，内部不进行曲面修正，外部可调用相关模块做曲面修正）
	@param   : mura_handle           有效的mura测评句柄
	@param   : in_xyz_data           输入XYZ数据,类型为三通道的AlgoMat类型(可配合MuraEvaluateMergeData生成)，曲面修正请直接调用曲面修正模块完成
	@param   : in_corner_point       输入XYZ数据的四个角点信息
	@param   : in_block_info         输入分区信息结构体
	@param   : out_result            输出的分区均匀性检测结果结构体
	@return  : AlgoError
	@Author  : wushaoshuang@2023/06/01
	****************************************/
	C_API AlgoError C_CALL MuraEvaluateCalcBlockUniformity(AlgoHandle mura_handle, AlgoMat* in_xyz_data, AlgoPoint in_corner_point[4], PartitionInfo* in_block_info, PartitionResult* out_result);

	/***************************************
	@brief   : 任意区域计算均匀性
	@note    : 背光检测中的任意区域均匀性检测,输入数据必须为XYZ（曲面修正后的数据，内部不进行曲面修正，外部可调用相关模块做曲面修正）
	@param   : mura_handle           有效的mura测评句柄
	@param   : in_xyz_data           输入XYZ数据,类型为三通道的AlgoMat类型(可配合MuraEvaluateMergeData生成)，曲面修正请直接调用曲面修正模块完成
	@param   : in_corner_point       输入XYZ数据的四个角点信息
	@param   : in_region_info        输入任意区域信息(圆形区域信息，对应于实际光枪测量场景)
	@param   : out_result            输出的任意区域均匀性检测结果结构体
	@return  : AlgoError
	@Author  : wushaoshuang@2023/06/01
	****************************************/
	C_API AlgoError C_CALL MuraEvaluateCalcMultiRegionUniformity(AlgoHandle mura_handle, AlgoMat* in_xyz_data, AlgoPoint in_corner_point[4], CircularInfo* in_region_info, int in_rect_num, MultiRegionResult* out_result);

	/***************************************
	@brief   : 计算九点位置均匀性
	@note    : 背光检测中的九点位置均匀性检测,输入数据必须为XYZ（曲面修正后的数据，内部不进行曲面修正，外部可调用相关模块做曲面修正）
	@param   : mura_handle           有效的mura测评句柄
	@param   : in_xyz_data           输入XYZ数据,类型为三通道的AlgoMat类型(可配合MuraEvaluateMergeData生成)，曲面修正请直接调用曲面修正模块完成
	@param   : in_corner_point       输入XYZ数据的四个角点信息
	@param   : in_nine_info          输入九点信息结构体
	@param   : out_result            输出的任意区域均匀性检测结果结构体
	@return  : AlgoError
	@Author  : wushaoshuang@2023/06/01
	****************************************/
	C_API AlgoError C_CALL MuraEvaluateCalcNinepointUniformity(AlgoHandle mura_handle, AlgoMat* in_xyz_data, AlgoPoint in_corner_point[4], PointPositionInfo* in_nine_info, NinePointResult* out_result);

	/***************************************
	@brief   : 异常点检测————可选调用
	@note    : 背光检测中的异常点检测,用于筛选一些异常点，外部根据需要调用
	@param   : mura_handle           有效的mura测评句柄
	@param   : in_data               输入待检测数据，曲面修正请直接调用曲面修正模块完成
	@param   : in_corner_point       输入XYZ数据的四个角点信息
	@param   : in_dim_thresh         输入暗异常点阈值
	@param   : in_bright_thresh      输入亮异常点阈值
	@param   : out_result            输出的异常点检测结果结构体
	@return  : AlgoError
	@Author  : wushaoshuang@2023/06/01
	****************************************/
	C_API AlgoError C_CALL MuraEvaluateDetectAbnormalPoint(AlgoHandle mura_handle, AlgoMat* in_data, AlgoPoint in_corner_point[4], const float* in_dim_thresh, const float* in_bright_thresh, AbnormalPointResult* out_result);

	/***************************************
	@brief   : 计算主波长
	@note    : 背光检测中的计算主波长,原来与均匀性检测混为一体，现在分离开，外部根据需要调用即可，输入数据必须为XYZ（曲面修正后的数据，内部不进行曲面修正，外部可调用相关模块做曲面修正）
	@param   : mura_handle                    有效的mura测评句柄
	@param   : in_xyz_data                    输入XYZ数据,类型为三通道的AlgoMat类型(可配合MuraEvaluateMergeData生成)，曲面修正请直接调用曲面修正模块完成
	@param   : in_corner_point                输入XYZ数据的四个角点信息
	@param   : out_dominant_waveLength        输出的主波长数据
	@param   : out_target_k                   输出的斜率
	@return  : AlgoError
	@Author  : wushaoshuang@2023/06/09
	****************************************/
	C_API AlgoError C_CALL MuraEvaluateCalcDWaveLength(AlgoHandle mura_handle, AlgoMat* in_xyz_data, AlgoPoint in_corner_point[4], AlgoMat* out_dominant_waveLength, AlgoMat* out_target_k);

	/***************************************
	@brief   : 统计数据信息
	@note    : 背光检测中的统计数据信息,即计算数据信息的最大值、最小值、均值，原来与均匀性检测混为一体，现在分离开，外部根据需要调用即可
	@param   : mura_handle                    有效的mura测评句柄
	@param   : in_data                        输入待统计的数据
	@param   : in_corner_point                输入数据的四个角点信息
	@param   : out_data_info                  输出的数据信息，为一个结构体（DataInfoResult），详见mura_data.h
	@return  : AlgoError
	@Author  : wushaoshuang@2023/06/09
	****************************************/
	C_API AlgoError C_CALL MuraEvaluateCalcDataInfo(AlgoHandle mura_handle, AlgoMat* in_data, AlgoPoint in_corner_point[4], DataInfoResult* out_data_info);

	/*********************************************
	@brief   : 获取buffer大小
	@note    :
	@param   : mura_handle           有效的mura测评句柄
	@param   : out_len              buffer大小
	@return  : AlgoError
	@Author  : wushaoshuang@2023/06/01
	**********************************************/
	C_API AlgoError C_CALL MuraEvaluateGetBufferSize(AlgoHandle mura_handle, uint64_t* out_len);

	/******************************************
	@brief   : 将mura测评数据导入buffer里
	@note    :
	@param   : mura_handle         有效的mura测评句柄
	@param   : out_buffer          输出检测参数的buffer空间
	@param   : in_out_len          buffer空间大小
	@return  : AlgoError
	@Author  : wushaoshuang@2023/06/01
	*******************************************/
	C_API AlgoError C_CALL MuraEvaluateToBuffer(AlgoHandle mura_handle, char* out_buffer, uint64_t* in_out_len);

	/************************************
	@brief    : 将buffer数据导入到模块中
	@note     :
	@param    : mura_handle         有效的mura测评句柄
	@param    : in_buffer               buffer
	@param    : in_len                  buffer大小
	@return   : AlgoError
	@Author  : wushaoshuang@2023/06/01
	************************************/
	C_API AlgoError C_CALL MuraEvaluateFromBuffer(AlgoHandle mura_handle, const char* in_buffer, uint64_t in_len);

	/*******************************************
	@brief   : 空间内存释放
	@note    : 句柄指向的对象销毁，此句柄已经无效，请配合handle=nullptr
	@param   : mura_handle
	@return  :
	@Author  : wushaoshuang@2023/06/01
	********************************************/
	C_API void C_CALL MuraEvaluateDestroyHandle(AlgoHandle mura_handle);

	/***********************************************
	@brief   : 句柄指向的对象的内存空间释放，句柄指向的对象还在
	@note    : 内存释放
	@param   : mura_handle    需要释放的句柄
	@return  : AlgoError
	@Author  : wushaoshuang@2023/06/01
	************************************************/
	C_API AlgoError C_CALL MuraEvaluateRelease(AlgoHandle mura_handle);


#ifdef __cplusplus
}
#endif
#endif