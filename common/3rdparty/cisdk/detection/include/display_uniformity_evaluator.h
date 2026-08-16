#ifndef ____DISPLAY_UNIFORMITY_EVALUATOR_H__
#define ____DISPLAY_UNIFORMITY_EVALUATOR_H__

#include "ci_declare.h"
#include "ci_define.h"
#include "ci_data.h"
#include "display_uniformity_evaluator_data.h"
#include "display_uniformity_evaluator_errors.h"
#include "algo_mat.h"


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


    /************************************
    @brief    : 创建句柄
    @note     :
    @return   : AlgoError
    @Modified : songshaojun@2025/06/25
    ************************************/
    C_API AlgoError C_CALL UnifomityEvaluatorCreateHandle(AlgoHandle* out_evaluate_handle);


	/************************************
    @brief  : 设置屏体信息
	@note   : 当前屏幕中，每个箱体的灯点数，每个模组的灯点数，已点亮的灯点数
	          1.不支持点亮等点数存在偏移的情况，如点亮的第一行或第一列不是当前检测屏幕的第一行或第一例
			  2.最小点亮单元为一个模组
    @param  : evaluate_handle,        输入量, 有效的评价句柄
	@param  : in_cabinet_size,        输入量, 当前屏幕中的箱体灯点数，每个箱体的灯点数是多少；
	@param  : in_panel_size,          输入量, 当前屏幕中的模组的灯点数，每个模组的灯点数是多少；
	@param  : in_lighted_wh,          输入量, 当前屏幕点亮的灯珠是多少个;
    @return	: AlgoError
	@author : songshaojun@2025/06/25
    ************************************/
	C_API AlgoError C_CALL UnifomityEvaluatorSetScreenInfo(AlgoHandle evaluate_handle, AlgoSize in_cabinet_size, AlgoSize in_panel_size, AlgoSize in_lighted_wh);


	/************************************
	@brief  : 设置待检测样本，输入样本为Shot类型
	@note   : 1) 输入Y数据或XYZ数据;
			  2) 输入的数据已完成光路修正（Vig模块）;
			  3) 若输入的为XYZ数据，则必须按照顺序X,Y,Z排列;
			  4) 若输入的为XYZ数据，且需要执行色度均匀性评价，则XYZ数据需已完成归一化操作,若没有完成归一化操作，可调用接口UnifomityEvaluatorNormalizeDetectImage对输入数据执行归一化操作;
	@param  : evaluate_handle,            输入量, 有效的评价句柄
	@param  : in_detect_sample_list,      输入量, 输入的待检测样本，送入handle必须为Shot
	@param  : in_detect_sampel_num,       输入量, 输入的待检测样本数量，等于1或3
	@param  : in_corner4,                 输入量, 当前待检测样本的四个ROI角点
	@param  : in_detect_flag,             输入量, 检测标志，=0执行亮度均匀性检测，=2执行亮色度均匀性检测，默认值为2，推荐2；
	@param  : in_do_curve_rectify,        输入量, 当前送入样本是否需要执行曲面修正(需要执行=true，不需要执行=false)，默认需要执行曲面修正
	@param  : in_corner_num,              输入量, 角点个数，默认为4;不需要变动
	@return	: AlgoError
	@author : songshaojun@2025/06/25
	************************************/
	C_API AlgoError C_CALL UnifomityEvaluatorSetDetectImageFromShot(AlgoHandle evaluate_handle, const AlgoHandle *in_detect_sample_list, const int in_detect_sampel_num, 
		const AlgoPoint* in_corner4, const int in_detect_flag = 2, const bool in_do_curve_rectify = true, int in_corner_num = 4);

	/************************************
	@brief  : 设置待检测样本，输入样本为meter类型
	@note   : 1) 输入Y数据或XYZ数据;
			  2) 输入的数据已完成光路修正（Vig模块）;
			  3) 若输入的为XYZ数据，则必须按照顺序X,Y,Z排列;
			  4) 若输入的为XYZ数据，且需要执行色度均匀性评价，则XYZ数据需已完成归一化操作,若没有完成归一化操作，可调用接口UnifomityEvaluatorNormalizeDetectImage对输入数据执行归一化操作;
	@param  : evaluate_handle,            输入量, 有效的评价句柄
	@param  : in_detect_sample_list,      输入量, 输入的待检测样本，送入handle必须为Meter
	@param  : in_detect_sampel_num,       输入量, 输入的待检测样本数量，等于1或3
	@param  : in_detect_flag,             输入量, 检测标志，=0执行亮度均匀性检测，=2执行亮色度均匀性检测，默认值为2，推荐2；
	@param  : in_do_curve_rectify,        输入量, 当前送入样本是否需要执行曲面修正(需要执行=true，不需要执行=false)，默认需要执行曲面修正
	@return	: AlgoError
	@author : songshaojun@2025/06/25
	************************************/
	C_API AlgoError C_CALL UnifomityEvaluatorSetDetectImageFromMeter(AlgoHandle evaluate_handle, const AlgoHandle *in_detect_sample_list, const int in_detect_sampel_num, 
		const int detect_flag = 2, const bool in_do_curve_rectify = true);


	/************************************
	@brief  : 对输入的XYZ数据执行归一化操作(曝光采集时间)
	@note   : 若调用UnifomityEvaluatorSetDetectImageFromShot或UnifomityEvaluatorSetDetectImageFromMeter接口
	          送入的数据是在同一曝光时间下采集的，或已执行过归一化操作，则无需调用该接口；
			  若送入的XYZ数据不是在同一曝光时间下采集的，但只进行亮度评价，则无需调用该接口
	@param  : evaluate_handle,            输入量, 有效的评价句柄
	@param  : in_normalize_para,          输入量, 输入的归一化参数，分别为XYZ数据的采集曝光时长，单位为ms
	@param  : in_normalize_para_num,      输入量, 归一化参数个数，默认为3,
	@return	: AlgoError
	@author : songshaojun@2025/06/25
	************************************/
	C_API AlgoError C_CALL UnifomityEvaluatorNormalizeDetectImage(AlgoHandle evaluate_handle, const float *in_normalize_para, const int in_normalize_para_num = 3);


	/************************************
	@brief  : 将输入的XYZ数据归一化到标准XYZ数据上
	@note   : 若无标准XYZ数据(如光枪采集)，可不调用
	@param  : evaluate_handle,            输入量, 有效的评价句柄
	@param  : in_normalize_para,          输入量, 输入的标准XYZ值
	@param  : in_xyz_num ,                输入量, 标准XYZ个数
	@return	: AlgoError
	@author : songshaojun@2025/06/25
	************************************/
	C_API AlgoError C_CALL UnifomityEvaluatorSetStandardXYZ(AlgoHandle evaluate_handle, const float *in_stand_xyz, const int in_xyz_num = 3);


	/************************************
	@brief  : 设置检测项，设置哪些特征，哪些特征参与得分计算
	@note   : 算法内部含有默认值，当不调用该接口时，默认检测特flag如下：
	          { true,true,true,false,false }
	@param  : evaluate_handle,            输入量, 有效的评价句柄
	@param  : in_detect_feature,          输入量, 输入的待检测特征种类，UniformFeatureDetectConfig参见display_uniformity_evaluator_data.h
	@return	: AlgoError
	@author : songshaojun@2025/06/25
	************************************/
	C_API AlgoError C_CALL UnifomityEvaluatorSetDetectFeature(AlgoHandle evaluate_handle, const UniformFeatureDetectConfig in_detect_feature);


	/************************************
	@brief  : 设置计算最终得分时，每个特征的权重
	@note   : 算法内部含有默认值，当不调用该接口时，默认检测权重如下：
	          {1.0f,1.1f,1.3f,0.3f,0.1f }
	@param  : evaluate_handle,            输入量, 有效的评价句柄
	@param  : in_detect_feature_wight,    输入量, 待检测特征权重，UniformFeatureScoreWeight参见display_uniformity_evaluator_data.h
	@return	: AlgoError
	@author : songshaojun@2025/06/25
	************************************/
	C_API AlgoError C_CALL UnifomityEvaluatorSetFeatureWeight(AlgoHandle evaluate_handle, const UniformFeatureScoreWeight in_detect_feature_wight);


	/************************************
	@brief  : 设置评估亮暗块特征时，参与计算的最小单元
	@note   : 期望检测的是以模组为单位的亮暗块，还是以箱体为单位的亮暗块
			  算法内部含有默认值，当不调用该接口时，默认标志为=0，代表最小计算单元为模组
	@param  : evaluate_handle,            输入量, 有效的评价句柄
	@param  : in_flag,                    输入量, 检测特征模式，=0代表以模组为最小计算单元，=1代表以箱体为最小计算单元
										  例如：当检测全屏时，可设置flag=0或=1。当检测某个单独的箱体时，该flag需设置为=0，=1时算法不生效；
	@return	: AlgoError
	@author : songshaojun@2025/06/25
	************************************/
	C_API AlgoError C_CALL UnifomityEvaluatorSetFeatureLightDarkDetectMode(AlgoHandle evaluate_handle, const int in_flag);


	/************************************
	@brief  : 计算输入样本均匀性得分
	@note   : 得分越高，越均匀
	@param  : in_evaluate_handle,            输入量, 有效的评价句柄
	@param  : out_feature_score,             输出量, 不同维度特征的检测得分，单个得分范围【0~100】值越大越均匀
	@param  : out_final_score,               输出量, 均匀性得分的总分
	@return	: AlgoError
	@author : songshaojun@2025/06/25
	************************************/
	C_API AlgoError C_CALL UnifomityEvaluatorCompute(AlgoHandle in_evaluate_handle, UniformFeatureScore *out_feature_score, float *out_final_score);


	/************************************
	@brief  : 输出计算过程中部分可视化结果
	@note   : 检测图，以及周期等特征中的曲线数据，以及各个功能模块计算的中间过程数据，导出在csv中
	@param  : in_evaluate_handle,            输入量, 有效的评价句柄
	@param  : out_export_root,               输入量, 可视化结果存放路径,不要加文件类型后缀
	@return	: AlgoError
	@author : songshaojun@2025/06/25
	************************************/
	C_API AlgoError C_CALL UnifomityEvaluatorExportResultPng(AlgoHandle in_evaluate_handle, const char *out_export_root);
	

	/************************************
	@brief  : 输出计算过程中部分可视化结果
	@note   : 检测图，以及周期等特征中的曲线数据，以及各个功能模块计算的中间过程数据，以mat形式导出在buffer中
	@param  : in_evaluate_handle,            输入量, 有效的评价句柄
	@param  : out_export_root,               输入量, 可视化结果存放路径,不要加文件类型后缀
	@return	: AlgoError
	@author : songshaojun@2025/06/25
	************************************/
	C_API AlgoError C_CALL UnifomityEvaluatorExportResultMat(AlgoHandle in_evaluate_handle, UniformFeatureExportResult *out_export_mat);




	/************************************
    @Brief	：载入和保存
    @Note	：文件名后缀必须是.UniformityEvaluator
			  存储内容包括:屏幕信息，
    @param	: in_evaluate_handle, 输入量, 有效的评价句柄
    @param	: in_fullname,        输入量, 文件名后缀必须是.UniformityEvaluator
    @return : AlgoError
	@author : songshaojun@2025/06/25
	************************************/
    C_API AlgoError C_CALL UnifomityEvaluatorLoad(AlgoHandle in_evaluate_handle, const char* in_fullname);
    C_API AlgoError C_CALL UnifomityEvaluatorSave(AlgoHandle in_evaluate_handle, const char* in_fullname);


	/************************************
	@brief  : 获取检测结果
	@note   : 得分越高，越均匀
	@param  : in_evaluate_handle,            输入量, 有效的评价句柄
	@param  : out_feature_score,             输出量, 不同维度特征的检测得分，单个得分范围【0~100】值越大越均匀
	@param  : out_final_score,               输出量, 均匀性得分的总分
	@return	: AlgoError
	@author : songshaojun@2025/06/25
	************************************/
	C_API AlgoError C_CALL UnifomityEvaluatorGetResult(AlgoHandle in_evaluate_handle, UniformFeatureScore *out_feature_score, float *out_final_score);


	/************************************
	@brief  : 获取buffer大小
	@note   :
	@param	: in_evaluate_handle, 输入量, 有效的评价句柄
	@param  : out_len, 输出量, buffer大小
	@return	: AlgoError
	@author : songshaojun@2025/06/25
	************************************/
	C_API AlgoError C_CALL UnifomityEvaluatorGetBufferSize(AlgoHandle in_evaluate_handle, uint64_t* out_len);


	/************************************
	@brief  : 将handle里面的数据导出到buffer
	@note   :
	@param	: in_evaluate_handle, 输入量, 有效的评价句柄
	@param  : out_buffer        已经申请好申请空间
	@param  : in_out_len        外部申请好的空间大小,会用真实大小修改
	@return	: AlgoError
	@author : songshaojun@2025/06/25
	************************************/
	C_API AlgoError C_CALL UnifomityEvaluatorToBuffer(AlgoHandle in_evaluate_handle, char* out_buffer, uint64_t* in_out_len);


	/************************************
	@brief  : 将buffer数据导入到handle
	@note   :
	@param	: in_evaluate_handle, 输入量, 有效的评价句柄
	@param  : in_buffer
	@param  : in_len
	@return	: AlgoError
	@author : songshaojun@2025/06/25
	************************************/
	C_API AlgoError C_CALL UnifomityEvaluatorFromBuffer(AlgoHandle in_evaluate_handle, const char* in_buffer, uint64_t in_len);


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
    C_API AlgoError C_CALL UnifomityEvaluatorRelease(AlgoHandle in_evaluate_handle);
    C_API void C_CALL UnifomityEvaluatorDestroyHandle(AlgoHandle in_evaluate_handle);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif