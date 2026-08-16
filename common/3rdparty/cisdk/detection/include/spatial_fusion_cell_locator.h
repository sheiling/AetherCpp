#ifndef ____SPATIAL_FUSION_CELL_LOCATOR_H__
#define ____SPATIAL_FUSION_CELL_LOCATOR_H__

#include "ci_declare.h"
#include "ci_define.h"
#include "ci_data.h"
#include "spatial_fusion_cell_locator_errors.h"
#include "spatial_fusion_cell_locator_data.h"
#include "led_meter_inspector.h"
#include "algo_mat.h"


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


	/************************************
	@brief  : 创建数据融合句柄
	@note   :
	@return : AlgoError
	@Author : songshaojun@2023/12/13
	************************************/
	C_API AlgoHandle C_CALL SpatialFusionCreateHandle();

	/************************************
	@brief  : 设置wafer尺寸信息
	@note   : 该部分信息，用来合并多张线扫样本的数据，定位出那几个pad点属于一个cell
	@param  : data_fusion_handle           有效的数据融合句柄
	@param  : in_cell_info                 输入量，cell的信息，详见wafer_cell_pad_locator_data.h
	@return : AlgoError
	@Author : songshaojun@2023/12/13
	************************************/
	C_API AlgoError C_CALL SpatialFusionSetWaferInfo(AlgoHandle data_fusion_handle,WaferCellInfo *in_wafer_cell_info);

	/************************************
	@brief  : 设置相机信息
	@note   : 该部分信息，用来进行坐标偏移，坐标转换
	@param  : data_fusion_handle           有效的数据融合句柄
	@param  : in_cam_type                  输入量，相机类型，是面阵还是线阵
	@param  : in_cam_info                  输入量，相机信息，详见data_fusion_converter_data.h中，CamInfo结构体
	@return : AlgoError
	@Author : songshaojun@2023/12/13
	************************************/
	C_API AlgoError C_CALL SpatialFusionSetCamInfo(AlgoHandle data_fusion_handle, const CamType *in_cam_type,const CamCoordInfo *in_cam_info);

	/************************************
	@brief  : 设置pad点和cell顶点检测结果合并的阈值
	@note   : 用于cell合并的阈值
	@param  : data_fusion_handle   有效的数据融合句柄
	@param  : in_merge_th           输入量，合并阈值
	@param  : in_mech_acc           输入量，机械移动精度
	@return : AlgoError
	@Author : songshaojun@2023/12/13
	************************************/
	C_API AlgoError C_CALL SpatialFusionSetCellMergeThreshold(AlgoHandle data_fusion_handle, float *in_merge_th, float *in_mech_acc);

	/************************************
	@brief  : 给线扫相机的像素坐标加偏移
	@note   : 目前该接口仅支持点对点转换，即输入偏移前坐标，吐出偏移后坐标
	@param  : data_fusion_handle           有效的数据融合句柄
	@param  : in_overlap_num               输入量，上下两张样本重叠的像素数
	@param  : in_data_idx                  输入量，送进来的数据是该列线扫样本的第几张
	@param  : in_point                     输入量，需要加偏移的点
	@param  : out_point_offset             输出量，吐出去的加过偏移的点
	@return : AlgoError
	@Author : songshaojun@2023/12/13
	************************************/
	C_API AlgoError C_CALL SpatialFusionAddOffSet(AlgoHandle data_fusion_handle,const int *in_overlap_num, const int *in_data_idx,
		const AlgoPoint *in_point, AlgoPoint *out_point_offset);

	///************************************
	//@brief  : 给cellpad定位的结果加偏移，加完偏移后，仍旧是像素坐标
	//@note   : 用于wafer_cell_pad_locator模块吐出的结果
	//@param  : data_fusion_handle           有效的数据融合句柄
	//@param  : in_off_set_info              输入量，偏移信息，详见data_fusion_converter_data.h
	//@param  : in_pad_result                输入量，pad结果
	//@param  : in_vertex_result             输入量，vertex顶点结果
	//@param  : out_pad_result_Offset        输出量，加过偏移的pad结果
	//@param  : out_vertex_result_offset     输出量，加过偏移的vertex顶点结果
	//@return : AlgoError
	//@Author : songshaojun@2023/12/13
	//************************************/
	//C_API AlgoError C_CALL SpatialFusionAddOffSetForLocate(AlgoHandle data_fusion_handle,OffSetInfo *in_off_set_info, const AlgoMat *in_pad_result,const AlgoMat *in_vertex_result,
	//	AlgoMat *out_pad_result_Offset, AlgoMat *out_vertex_result_offset);

	/************************************
	@brief  : 给cellpad定位的结果加偏移，加完偏移后，仍旧是像素坐标
	@note   : 用于wafer_cell_pad_locator模块吐出的结果
	@param  : data_fusion_handle           有效的数据融合句柄
	@param  : in_overlap_num               输入量，前后两张样本重叠张数
	@param  : in_data_idx                  输入量，当前是第几张样本
	@param  : in_pad_result                输入量，pad结果
	@param  : in_vertex_result             输入量，vertex顶点结果
	@param  : out_pad_result_Offset        输出量，加过偏移的pad结果
	@param  : out_vertex_result_offset     输出量，加过偏移的vertex顶点结果
	@return : AlgoError
	@Author : songshaojun@2023/12/13
	************************************/
	C_API AlgoError C_CALL SpatialFusionAddOffSetForLocate(AlgoHandle data_fusion_handle, const int *in_overlap_num, const int *in_data_idx, const AlgoMat *in_pad_result,const AlgoMat *in_vertex_result,
		AlgoMat *out_pad_result_Offset, AlgoMat *out_vertex_result_offset);

	///************************************
	//@brief  : 给缺陷检测的结果加偏移，加完偏移后，仍旧是像素坐标
	//@note   : 用于wafer_defect_detector模块吐出的结果
	//@param  : data_fusion_handle              有效的数据融合句柄
	//@param  : in_off_set_info                 输入量，偏移信息，详见data_fusion_converter_data.h
	//@param  : in_defect_feature               输入量，缺陷特征结果
	//@param  : in_defect_contour               输入量，缺陷轮廓结果
	//@param  : in_defect_num                   输入量，缺陷数量
	//@param  : out_defect_feature_offset       输出量，加过偏移的缺陷特征结果
	//@param  : out_defect_contour_offset		  输出量，加过偏移的缺陷轮廓结果
	//@return : AlgoError
	//@Author : songshaojun@2023/12/13
	//************************************/
	//C_API AlgoError C_CALL SpatialFusionAddOffSetForDefectDetection(AlgoHandle data_fusion_handle,OffSetInfo *in_off_set_info, const WaferDefectFeature *in_defect_feature,
	//	const AlgoMat *in_defect_contour,const int in_defect_num,  WaferDefectFeature *out_defect_feature_offset, AlgoMat *out_defect_contour_offset);

	/************************************
	@brief  : 给缺陷检测的结果加偏移，加完偏移后，仍旧是像素坐标
	@note   : 用于wafer_defect_detector模块吐出的结果
	@param  : data_fusion_handle              有效的数据融合句柄
	@param  : in_overlap_num                  输入量，上下两张样本重叠数量
	@param  : in_data_idx                     输入量，当前是第几张样本
	@param  : in_defect_feature               输入量，缺陷特征结果
	@param  : in_defect_contour               输入量，缺陷轮廓结果
	@param  : in_defect_num                   输入量，缺陷数量
	@param  : out_defect_feature_offset       输出量，加过偏移的缺陷特征结果
	@param  : out_defect_contour_offset		  输出量，加过偏移的缺陷轮廓结果
	@return : AlgoError
	@Author : songshaojun@2023/12/13
	************************************/
	C_API AlgoError C_CALL SpatialFusionAddOffSetForDefectDetection(AlgoHandle data_fusion_handle, const int *in_overlap_num, const int *in_data_idx, const WaferDefectFeature *in_defect_feature,
		const AlgoMat *in_defect_contour,const int in_defect_num,  WaferDefectFeature *out_defect_feature_offset, AlgoMat *out_defect_contour_offset);

	/************************************
	@brief  : 将相机像素坐标转换至载台坐标
	@note   : 目前该接口仅支持点对点转换，即输入像素坐标，吐出该点在载台坐标系中的坐标
			  使用该接口前，请使用SpatialFusionSetCamInfo()接口设置相机信息，否则参数为系统内部默认线扫相机！！！！！！！！
			  使用该接口前，请使用SpatialFusionSetCamInfo()接口设置相机信息，否则参数为系统内部默认线扫相机！！！！！！！！
			  使用该接口前，请使用SpatialFusionSetCamInfo()接口设置相机信息，否则参数为系统内部默认线扫相机！！！！！！！！
	@param  : data_fusion_handle                    有效的数据融合句柄
	@param  : in_cam_plat_coordinate                输入量，拍摄该点时相机在载台（机械）坐标系中的位置；【备注】：如是线扫相机，该点坐标为线扫相机执行该列扫描时的起始位置
	@param  : in_point_pixel                        输入量，送进来的数据需要转换的像素坐标点
	@param  : out_point_mech                        输出量，吐出去的转换至载台（机械）坐标系的点
	@return : AlgoError
	@Author : songshaojun@2023/12/13
	************************************/
	C_API AlgoError C_CALL SpatialFusionCoordinateTrans(AlgoHandle data_fusion_handle, const AlgoPoint2f *in_plat_coordinate,const AlgoPoint2f *in_point_pixel,
		AlgoPoint2f *out_point_mech);

	///************************************
	//@brief  : 将cellpad定位的结果转换至载台坐标系，转换后，为载台坐标（机械）
	//@note   : 用于wafer_cell_pad_locator模块吐出的结果
	//@param  : data_fusion_handle               有效的数据融合句柄
	//@param  : in_trans_mat1                    输入量，转换矩阵1
	//@param  : in_trans_mat2                    输入量，转换矩阵2（若为面阵相机，该矩阵可设置为空）
	//@param  : in_cam_type                      输入量，相机类型，面阵相机或线阵相机
	//@param  : in_cam_plat_coordinate           输入量，相机在载台中的位置坐标
	//@param  : in_cam_size                      输入量，相机画幅大小（若为线阵相机，height可设置为0）        
	//@param  : in_plat_coordinate               输入量，拍摄当前画面时，载台位置坐标
	//@param  : in_pad_result                    输入量，pad结果
	//@param  : in_vertex_result                 输入量，vertex顶点检测结果
	//@param  : out_pad_result_trans             输出量，经过坐标转换的pad结果
	//@param  : out_vertex_result_trans          输出量，经过坐标转换的vertex顶点检测结果
	//@return : AlgoError
	//@Author : songshaojun@2023/12/13
	//************************************/
	//C_API AlgoError C_CALL SpatialFusionCoordinateTransForLocate(AlgoHandle data_fusion_handle,const AlgoMat *in_trans_mat1, const AlgoMat *in_trans_mat2,const CamType *in_cam_type,
	//	const AlgoPoint2f *in_cam_plat_coordinate,const AlgoSize *in_cam_size,
	//	const AlgoPoint2f *in_plat_coordinate, const AlgoMat *in_pad_result,const AlgoMat *in_vertex_result,
	//	AlgoMat *out_pad_result_trans,AlgoMat *out_vertex_result_trans);

	/************************************
	@brief  : 将cellpad定位的结果转换至载台坐标系，转换后，为载台坐标（机械）
	@note   : 用于wafer_cell_pad_locator模块吐出的结果
	@param  : data_fusion_handle               有效的数据融合句柄   
	@param  : in_plat_coordinate               输入量，拍摄当前画面时，载台位置坐标
	@param  : in_pad_result                    输入量，pad结果
	@param  : in_vertex_result                 输入量，vertex顶点检测结果
	@param  : out_pad_result_trans             输出量，经过坐标转换的pad结果
	@param  : out_vertex_result_trans          输出量，经过坐标转换的vertex顶点检测结果
	@return : AlgoError
	@Author : songshaojun@2023/12/13
	************************************/
	C_API AlgoError C_CALL SpatialFusionCoordinateTransForLocate(AlgoHandle data_fusion_handle,const AlgoPoint2f *in_plat_coordinate,
		const AlgoMat *in_pad_result,const AlgoMat *in_vertex_result,
		AlgoMat *out_pad_result_trans,AlgoMat *out_vertex_result_trans);

	///************************************
	//@brief  : 将缺陷检测的结果转换至载台坐标系，转换后，为载台坐标（机械）
	//@note   : 用于wafer_defect_detector模块吐出的结果
	//@param  : data_fusion_handle               有效的数据融合句柄
	//@param  : in_trans_mat1                    输入量，转换矩阵1
	//@param  : in_trans_mat2                    输入量，转换矩阵2（若为面阵相机，该矩阵可设置为空）
	//@param  : in_cam_type                      输入量，相机类型，面阵相机或线阵相机
	//@param  : in_cam_plat_coordinate           输入量，相机在载台中的位置坐标
	//@param  : in_cam_size                      输入量，相机画幅大小（若为线阵相机，height可设置为0）        
	//@param  : in_plat_coordinate               输入量，拍摄当前画面时，载台位置坐标
	//@param  : in_defect_feature                输入量，缺陷特征结果
	//@param  : in_defect_contour                输入量，缺陷轮廓结果
	//@param  : defect_num                       输入量，缺陷的数量
	//@param  : out_defect_feature_trans         输出量，经过坐标转换的缺陷特征结果
	//@param  : out_defect_contour_trans         输出量，经过坐标转换的缺陷轮廓结果
	//@return : AlgoError
	//@Author : songshaojun@2023/12/13
	//************************************/
	//C_API AlgoError C_CALL SpatialFusionCoordinateTransForDefectDetection(AlgoHandle data_fusion_handle,const AlgoMat *in_trans_mat1, const AlgoMat *in_trans_mat2,const CamType *in_cam_type,
	//	const AlgoPoint2f *in_cam_plat_coordinate,const AlgoSize *in_cam_size,const AlgoPoint2f *in_plat_coordinate, 
	//	const WaferDefectFeature *in_defect_feature, const AlgoMat *in_defect_contour,int defect_num,
	//	WaferDefectFeature *out_defect_feature_trans, AlgoMat *out_defect_contour_trans);

	/************************************
	@brief  : 将缺陷检测的结果转换至载台坐标系，转换后，为载台坐标（机械）
	@note   : 用于wafer_defect_detector模块吐出的结果
	@param  : data_fusion_handle               有效的数据融合句柄   
	@param  : in_plat_coordinate               输入量，拍摄当前画面时，载台位置坐标
	@param  : in_defect_feature                输入量，缺陷特征结果
	@param  : in_defect_contour                输入量，缺陷轮廓结果
	@param  : defect_num                       输入量，缺陷的数量
	@param  : out_defect_feature_trans         输出量，经过坐标转换的缺陷特征结果
	@param  : out_defect_contour_trans         输出量，经过坐标转换的缺陷轮廓结果
	@return : AlgoError
	@Author : songshaojun@2023/12/13
	************************************/
	C_API AlgoError C_CALL SpatialFusionCoordinateTransForDefectDetection(AlgoHandle data_fusion_handle,const AlgoPoint2f *in_plat_coordinate,
		const WaferDefectFeature *in_defect_feature, const AlgoMat *in_defect_contour,int defect_num,
		WaferDefectFeature *out_defect_feature_trans, AlgoMat *out_defect_contour_trans);

	/************************************
	@brief  : 合并所有pad点和cell顶点结果，得到共有几个cell
	@note   : 
	@param  : data_fusion_handle          有效的数据融合句柄
	@param  : in_all_pad_result           输入量，所有pad结果
	@param  : in_all_vertex_result        输入量，所有veretex顶带你结果
	@param  : in_result_num               输入量，输入结果的数量，即共有几张图像的检测结果送进来
	@param  : out_cell_num                输出量，共检测到了几个cell
	@return : AlgoError 
	@Author : songshaojun@2023/12/13
	************************************/
	C_API AlgoError C_CALL SpatialFusionMergeLocateResult(AlgoHandle data_fusion_handle, const AlgoMat *in_all_pad_result,const AlgoMat *in_all_vertex_result,
		const int in_result_num, int *out_cell_num);

	/************************************
	@brief    : 获取所有cell结果的个数，即获取共检测到了几个cell
	@note     :
	@param    : data_fusion_handle          有效的data_fusion_handle句柄
	@param    : out_num                     输出量，返回cell_list内有几个cell
	@return   : AlgoError
	@Modified : songshaojun@2024/06/13
	************************************/
	C_API AlgoError C_CALL SpatialFusionGetCellLocateResultNum(AlgoHandle data_fusion_handle, int* out_num);

	/************************************
	@brief  : 获取所有cell的结果
	@note   : 
	@param  : data_fusion_handle          有效的数据融合句柄
	@param  : out_one_cell_list           输出量，检测到的所有cell
	@return : AlgoError 
	@Author : songshaojun@2023/12/13
	************************************/
	C_API AlgoError C_CALL SpatialFusionGetCellLocateResult(AlgoHandle data_fusion_handle, WaferOneCellInfo *out_one_cell_list);

	/************************************
	@brief  : 获取指定索引的cell结果
	@note   : 需在调用DataFusionMergeLocateResult（）接口后使用
	@param  : data_fusion_handle          有效的数据融合句柄
	@param  : out_one_cell_result         输出量，指定cell的CellPad定位结果
	@return : AlgoError
	@Author : songshaojun@2023/12/13
	************************************/
	C_API AlgoError C_CALL SpatialFusionGetOneCellLocateResult(AlgoHandle data_fusion_handle,const int cell_idx, WaferOneCellInfo *out_one_cell_list);

	/************************************
	@brief  : 合并缺陷结果到每个cell上，即给出每cell上对应的缺陷
	@note   : 
	@param  : data_fusion_handle               有效的数据融合句柄
	@param  : in_result_num                    输入量，总共多少张样本的缺陷检测结果要合并到每个cell上，即检测了多少张样本
	@param  : in_all_defect_feature            输入量，所有缺陷的特征信息
	@param  : in_all_defect_contour            输入量，所有缺陷的轮廓信息
	@param  : in_all_defect_num                输入量，每张样本里面检测到的缺陷的数量
	@param  : in_all_one_cell_list             输入量，所有的cell结果
	@param  : out_one_cell_defect_num          输出量，每个cell上缺点的数量
	@return : AlgoError 
	@Author : songshaojun@2023/12/13
	************************************/
	C_API AlgoError C_CALL SpatialFusionMergeDefectResult(AlgoHandle data_fusion_handle, const int in_result_num, WaferDefectFeature **in_all_defect_feature,
	        AlgoMat **in_all_defect_contour,const int* in_all_defect_num,const WaferOneCellInfo *in_all_one_cell_list,const int in_cell_num, int *out_one_cell_defect_num);

	/************************************
	@brief  : 获取每个cell上的缺陷结果
	@note   : 
	@param  : data_fusion_handle            有效的数据融合句柄
	@param  : in_cell_num                   输入量，cell的数量
	@param  : in_one_cell_defect_num        输入量，每个cell上缺陷的数量
	@param  : out_one_cell_defect_list      输出量，每个cell上缺陷的结果
	@Author : songshaojun@2023/12/13
	************************************/
	C_API AlgoError C_CALL SpatialFusionGetCellDefectResult(AlgoHandle data_fusion_handle, const int in_cell_num, int *in_one_cell_defect_num, WaferOneCellDefectInfo **out_one_cell_defect_list);

	/************************************
	@brief  : 获取指定cell上的缺陷的数量
	@note   : 与接口DataFusionGetOneCellDefectResult（）搭配使用
	@param  : data_fusion_handle            有效的数据融合句柄
	@param  : in_cell_idx                   输入量，cell的索引
	@param  : out_one_cell_defect_num       输出量，该cell上缺陷的数量
	@Author : songshaojun@2023/12/13
	************************************/
	C_API AlgoError C_CALL SpatialFusionGetOneCellDefectNum(AlgoHandle data_fusion_handle, const int in_cell_idx, int *out_one_cell_defect_num);

	/************************************
	@brief  : 获取指定cell上的缺陷结果
	@note   : 与接口DataFusionGetOneCellDefectNum（）搭配使用，先调用DataFusionGetOneCellDefectNum，后调用本接口
	@param  : data_fusion_handle               有效的数据融合句柄
	@param  : in_cell_idx                      输入量，cell的索引
	@param  : out_one_cell_defect_result       输出量，该cell上缺陷的结果
	@Author : songshaojun@2023/12/13
	************************************/
	C_API AlgoError C_CALL SpatialFusionGetOneCellDefectResult(AlgoHandle data_fusion_handle, const int in_cell_idx, WaferOneCellDefectInfo *out_one_cell_defect_result);

	/************************************
	@brief  : 将点亮检测结果存储
	@note   : 
	@param  : data_fusion_handle                    有效的数据融合句柄
	@param  : in_led_defect_flag                    输入量，缺陷类型标志
	@param  : in_defect_num                         输入量，缺陷数量
	@param  : out_one_cell_info                     输入量，输入cell信息
	@param  : in_out_cell_illumination_info         输出量，该cell上的点亮检测结果
	@return : AlgoError 
	@Author : songshaojun@2023/12/13
	************************************/
	C_API AlgoError C_CALL SpatialFusionMergeIlluminationResult(AlgoHandle data_fusion_handle, const LedDefectFlag *in_led_defect_flag,const int in_defect_num, AlgoPoint *in_defect_loc,
		WaferOneCellInfo *in_one_cell_info, WaferOneCellIlluminationInfo *in_out_cell_illumination_info);

	///************************************
	//@brief  : 将cell定位结果和缺陷检测结果绘制出来
	//@note   : 
	//@param  : data_fusion_handle              有效的数据融合句柄
	//@param  : in_all_one_cell_list            输入量，所有cell的结果
	//@param  : in_cell_num                     输入量，cell的数量
	//@param  : in_one_cell_defect_list         输入量，每个cell上缺陷的结果
	//@param  : in_one_cell_defect_num          输入量，每个cell上缺陷的数量
	//@param  : in_draw_flag                    输入量，绘制标记
	//@param  : out_draw_png_dir                输入量，导出png地址
	//@Author : songshaojun@2023/12/13
	//************************************/
	//C_API AlgoError C_CALL SpatialFusionExportVisualDetectResultPng(AlgoHandle data_fusion_handle, const WaferOneCellInfo *in_all_one_cell_list, const int in_cell_num, WaferOneCellDefectInfo **in_one_cell_defect_list,
	//	const int *in_one_cell_defect_num,int in_draw_flag,const char* out_draw_png_dir);

	/************************************
	@brief  : 将cell定位结果和缺陷检测结果绘制出来
	@note   : 绘制出来的为虚拟图
	@param  : data_fusion_handle              有效的数据融合句柄
	@param  : in_draw_flag                    输入量，绘制标记
	@param  : out_draw_png_dir                输入量，导出png地址
	@Author : songshaojun@2023/12/13
	************************************/
	C_API AlgoError C_CALL SpatialFusionExportVisualDetectResultPng(AlgoHandle data_fusion_handle, int in_draw_flag,const char* out_draw_png_dir);

	/************************************
	@brief  : 将指定cell的缺陷检测结果绘制出来
	@note   : 绘制出来的为虚拟图
	@param  : data_fusion_handle              有效的数据融合句柄
	@param  : cell_idx                        输入量，需要绘制的cell索引
	@param  : in_draw_flag                    输入量，绘制标记
	@param  : out_draw_png_dir                输入量，导出png地址
	@Author : songshaojun@2023/12/13
	************************************/
	C_API AlgoError C_CALL SpatialFusionExportOneCellVisualDetectResultPng(AlgoHandle data_fusion_handle, const int cell_idx ,int in_draw_flag,const char* out_draw_png_dir);

	/************************************
	@brief  : 将wafer_img_stitch模块吐出的拼接大图进行一次修正
	@note   : 该接口在完成所有列的扫描及wafer_img_stitch模块拼接后才可以调用
	@param  : data_fusion_handle              有效的数据融合句柄
	@param  : in_stitch_img                   输入量，wafer_img_stitch模块吐出的大图
	@param  : out_rectify_img                 输出量，修正后的大图
	@Author : songshaojun@2024/04/02
	************************************/
	C_API AlgoError C_CALL SpatialFusionRectifyStitchImage(AlgoHandle data_fusion_handle, const AlgoMat in_stitch_img,
		 AlgoMat *out_rectify_img);

	/************************************
	@brief  : 将计算得到的所有cell轮廓框，都绘制在wafer_image_stitch模块吐出的大图上(实物图)
	@note   : 该接口在完成所有列的扫描以及计算后才可调用
	@param  : data_fusion_handle              有效的数据融合句柄
	@param  : in_source_img                   输入量，wafer_img_stitch模块吐出的大图，所有结果都将绘制在这张大图上
	@param  : scale_ratio                     输入量，wafer_img_stitch模块吐出大图的缩放比例，可通过wafer_img_stitch得到
	@param  : in_base0                        输入量，线扫相机开始扫描第一张样本时的载台坐标
	@param  : out_draw_result                 输出量，结果图
	@Author : songshaojun@2024/04/02
	************************************/
	C_API AlgoError C_CALL SpatialFusionDrawAllCellContourInSourceImageMat(AlgoHandle data_fusion_handle, const AlgoMat in_source_img,
		const int scale_ratio, const AlgoPoint2f *in_base0, AlgoMat *out_draw_result);

	/************************************
	@brief  : 获取指定cell的全部检测结果（cell轮廓+cell缺陷）(实物图)
	@note   :该接口在完成所有列的扫描以及计算后才可调用
	@param  : data_fusion_handle              有效的数据融合句柄
	@param  : in_source_img                   输入量，wafer_img_stitch模块吐出的大图，所有结果都将绘制在这张大图上
	@param  : scale_ratio                     输入量，wafer_img_stitch模块吐出大图的缩放比例，可通过wafer_img_stitch得到
	@param  : in_base0                        输入量，线扫相机开始扫描第一张样本时的载台坐标
	@param  : cell_idx                        输入量，想要获取的指定cell的索引号
	@param  : out_draw_result                 输出量，结果图
	@Author : songshaojun@2024/04/02
	************************************/
	C_API AlgoError C_CALL SpatialFusionDrawOneCellResultInSourceImageMat(AlgoHandle data_fusion_handle, const AlgoMat in_source_img,
		const int scale_ratio, const AlgoPoint2f *in_base0, const int cell_idx, AlgoMat *out_draw_result);

	//save,load,buffer相关
	/************************************
	@Brief		：	载入和保存结果
	@Note		：	文件名后缀必须为.datafusion,载入和保存的结果中存有缺陷检测结果
	@Parameter	: 	locator_handle		    有效的数据融合句柄
	@Parameter	: 	in_locator_fullname		输入量，导出的.datafusion文件路径
	@Returns	:   AlgoError
	@Author		：	songshaojun@2023/12/13
	************************************/
	C_API AlgoError C_CALL SpatialFusionLoad(AlgoHandle data_fusion_handle, const char* in_data_fusion_fullname);
	C_API AlgoError C_CALL SpatialFusionSave(AlgoHandle data_fusion_handle, const char* in_data_fusion_fullname);


	/************************************
	@brief    : 获取检测buffer大小
	@note     :
	@param    : data_fusion_handle          有效的数据融合句柄
	@param    : out_len                     输出量，buffer大小
	@return   : AlgoError
	@Modified : songshaojun@2023/12/13
	************************************/
	C_API AlgoError C_CALL SpatialFusionGetBufferSize(AlgoHandle data_fusion_handle, uint64_t* out_len);

	/************************************
	@brief    : 将datafusion里面的数据导出到buffer
	@note     :
	@param    : data_fusion_handle        有效的data_fusion_handle句柄
	@param    : out_buffer                已经申请好申请空间
	@param    : in_out_len                外部申请好的空间大小,会用真实大小修改
	@return   : AlgoError
	@Modified : songshaojun@2023/12/13
	************************************/
	C_API AlgoError C_CALL SpatialFusionToBuffer(AlgoHandle data_fusion_handle, char* out_buffer, uint64_t* in_out_len);

	/************************************
	@brief    : 将buffer数据导入到datafusion
	@note     :
	@param    : data_fusion_handle          有效的data_fusion_handle句柄
	@param    : in_buffer
	@param    : in_len
	@return   : AlgoError
	@Modified : songshaojun@2023/12/13
	************************************/
	C_API AlgoError C_CALL SpatialFusionFromBuffer(AlgoHandle data_fusion_handle, const char* in_buffer, uint64_t in_len);


	C_API AlgoError C_CALL SpatialFusionnRelease(AlgoHandle data_fusion_handle);
	C_API void C_CALL SpatialFusionDestroyHandle(AlgoHandle data_fusion_handle);

	//以下这两个接口专为调试使用，用于保存送进来的所有pad点和vertex顶点结果
	C_API AlgoError C_CALL SpatialFusionLoadInput(AlgoHandle data_fusion_handle, const char* in_data_fusion_fullname);
	C_API AlgoError C_CALL SpatialFusionSaveInput(AlgoHandle data_fusion_handle, const char* in_data_fusion_fullname);

	//临时测试接口
	//C_API AlgoError C_CALL SpatialFusionGetTemp(AlgoHandle data_fusion_handle);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif