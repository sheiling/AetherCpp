#ifndef ____SHARPNESS_AUTOFOCUS_H__
#define ____SHARPNESS_AUTOFOCUS_H__

#include "ci_declare.h"
#include "ci_define.h"
#include "ci_data.h"

#include "sharpness_autofocus_data.h"
#include "sharpness_autofocus_errors.h"
#include "algo_mat.h"


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


	/************************************
	@brief  : 创建检测句柄
	@note   : 
	@param  : out_handle,      输出量, 返回的有效句柄
	@return : AlgoError
	@author : songshaojun@2025/09/05
	************************************/
	C_API AlgoError C_CALL SharpnessCreateHandle(AlgoHandle* out_handle);


	/************************************
    @brief  : 初始化线程数句柄
    @note   : 不调用此函数, 模块默认 = CPU线程数-1
    @param  : sharpness_handle,            输入量, 有效的清晰度评价句柄
    @param  : in_omp_num,                  输入量, 并行运算线程个数
    @return	: AlgoError
	@author : songshaojun@2025/09/05
    ************************************/
    C_API AlgoError C_CALL SharpnessSetDefaultThreadsNum(AlgoHandle sharpness_handle, int in_omp_num);


	/************************************
    @brief  : 设置清晰度评价方法
    @note   : 当不调用该接口时，默认使用方法=LAPLACIAN
	@param  : sharpness_handle,            输入量, 有效的清晰度评价句柄
	@param  : in_method,                   输入量, 清晰度评价函数方法, 详情见 meta_info_enum.h
    @return	: AlgoError
	@author : songshaojun@2025/09/05
	************************************/
    C_API AlgoError C_CALL SharpnessSetMethods(AlgoHandle sharpness_handle, const SharpnessMethod &in_method);
	C_API AlgoError C_CALL SharpnessGetMethods(AlgoHandle sharpness_handle,  SharpnessMethod *out_method);


	/************************************
    @brief  : 设置自动对焦时测量样本的采集位置
    @note   : 若无需推荐位置，该接口可不调用
	@param  : sharpness_handle,            输入量, 有效的清晰度评价句柄
	@param  : in_position_list,            输入量, 输入的样本采集位置,Z轴
	@param  : in_loc_num,                  输入量, 输入的采集样本位置数量
    @return	: AlgoError
	@author : songshaojun@2025/09/05
	************************************/
    C_API AlgoError C_CALL SharpnessSetInputPosition(AlgoHandle sharpness_handle, const float* in_position_list, const int in_loc_num);


	/************************************
    @brief  : 自动对焦计算，获取输入样本中最清晰的样本索引
    @note   : 输出的最清晰样本索引号所对应的机械坐标即为当前输入样本list中对焦最清晰位置
	@param  : sharpness_handle,            输入量, 有效的清晰度评价句柄
	@param  : in_img_list,                 输入量, 输入的样本list，该list为清晰对焦点附近采集的样本
    @param  : in_img_num,                  输入量, 输入的样本list数量
	@param  : out_sharpness_score_list,    输出量, 输出的样本list清晰度得分
	@param  : out_sharpnessest_idx,        输出量, 输出的最清晰样本的索引号
	@return	: AlgoError
	@author : songshaojun@2025/09/05
	************************************/
	C_API AlgoError C_CALL SharpnessAutoFocus(AlgoHandle sharpness_handle, const AlgoMat *in_img_list, const int in_img_num,
		float *out_sharpness_score_list, int *out_sharpnessest_idx, const AlgoRect in_rect = { 0,0,0,0 });


	/************************************
    @brief  : 设置样本清晰度得分
    @note   : 
	@param  : sharpness_handle,            输入量, 有效的清晰度评价句柄
	@param  : in_sharpness_score_list,     输入量, 输入的清晰度得分列表
	@param  : int_score_num,               输入量, 输入的清晰度得分数量
    @return	: AlgoError
	@author : songshaojun@2025/09/05
	************************************/
    C_API AlgoError C_CALL SharpnessSetInputScore(AlgoHandle sharpness_handle, const float *in_sharpness_score_list, const int int_score_num);



	/************************************
    @brief  : 设置样本清晰度得分
    @note   : 
	@param  : sharpness_handle,            输入量, 有效的清晰度评价句柄
	@param  : in_sharpness_score_list,     输入量, 输入的清晰度得分列表
	@param  : int_score_num,               输入量, 输入的清晰度得分数量
    @return	: AlgoError
	@author : songshaojun@2025/09/05
	************************************/
    C_API AlgoError C_CALL SharpnessGetRecommendedFocusPosition(AlgoHandle sharpness_handle, float *out_recommend_position);

	
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
    C_API AlgoError C_CALL SharpnessRelease(AlgoHandle sharpness_handle);
    C_API void C_CALL SharpnessDestroyHandle(AlgoHandle sharpness_handle);


#ifdef __cplusplus
}
#endif // __cplusplus
#endif