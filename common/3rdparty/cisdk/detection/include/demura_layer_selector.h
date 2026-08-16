#ifndef __DEMURA_LAYER_SELECTOR_H__
#define __DEMURA_LAYER_SELECTOR_H__

#include "ci_define.h"
#include "ci_declare.h"
#include "demura_layer_selector_data.h"
#include "demura_layer_selector_errors.h"
#include "ci_utils_algomat.h"

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

    /************************************
    @brief  : 创建DemuraLayerSelector句柄
    @note   :
    @param  : out_demura_layer_selector_handle   --
    DemuraLayerSelector句柄
    @return	: AlgoError
    @Author : lichaoping@2024/09/29
    ************************************/
    C_API AlgoError C_CALL DemuraLayerSelectorCreateHandle(
        AlgoHandle *out_demura_layer_selector_handle);

	/************************************
	@brief  : 设置箱体及拍图灰阶信息
	@note   :
	@param  : demura_layer_selector_handle		-- DemuraLayerSelector句柄
	@param  : in_start_gray								-- 拍摄灰阶的起始灰阶
    @param  : in_gray_step								-- 拍摄灰阶的灰阶间隔
    @param  : in_gray_num								-- 拍摄灰阶的灰阶数量
    @param  : in_lumin_loss_ratio						-- 亮度衰减比例
    @param  : in_gamma									-- 箱体gamma
	@return	: AlgoError
	@Author : lichaoping@2024/09/29
	************************************/
    C_API AlgoError C_CALL DemuraLayerSelectorSetInputInfo(
        AlgoHandle demura_layer_selector_handle,int in_start_gray,int in_gray_step,int in_gray_num,float in_lumin_loss_ratio,double in_gamma);


	/************************************
	@brief  : 设置各算法阈值
	@note   :
	@param  : demura_layer_selector_handle		-- DemuraLayerSelector句柄
	@param  : in_param_struct								-- 输入的阈值参数结构体
	@return	: AlgoError
	@Author : lichaoping@2024/09/29
	************************************/
    C_API AlgoError C_CALL DemuraLayerSelectorSetParam(
        AlgoHandle demura_layer_selector_handle,
        DemuraLayerSelectorParam in_param_struct);

	/************************************
	@brief  : 判断白色校正
	@note   :
	@param  : demura_layer_selector_handle   -- DemuraLayerSelector句柄
	@param  : in_r_meter_list							-- R通道每个灰阶下的meter
    @param  : in_g_meter_list							-- G通道每个灰阶下的meter
    @param  : in_b_meter_list							-- B通道每个灰阶下的meter
    @param  : in_w_meter_list							-- W通道每个灰阶下的meter
    @param  : in_gray_num								-- 每个通道对应meter数量
	@param  : out_need_W_correction_flag				-- 是否需要白色校正结果（1为需要，0为不需要）
	@return	: AlgoError
	@Author : lichaoping@2024/09/30
	************************************/
    C_API AlgoError C_CALL DemuraLayerSelectorJudgeWCorrection(
        AlgoHandle demura_layer_selector_handle,
        AlgoHandle in_r_meter_list[], AlgoHandle in_g_meter_list[],
        AlgoHandle in_b_meter_list[], AlgoHandle in_w_meter_list[],
        int in_gray_num,
        int *out_need_W_correction_flag);

	/************************************
	@brief  : 判断白色校正单灰阶or全灰阶
	@note   :
	@param  : demura_layer_selector_handle   -- DemuraLayerSelector句柄
	@param  : out_result_flag							-- 是否多层白色校正（1为全灰阶，0为单灰阶）
	@return	: AlgoError
	@Author : lichaoping@2024/09/30
	************************************/
    C_API AlgoError C_CALL DemuraLayerSelectorJudgeWMultiLevelCorrection(
        AlgoHandle demura_layer_selector_handle, int *out_result_flag);


	/************************************
	@brief  : 输出W校正灰阶金字塔
	@note   :
	@param  : demura_layer_selector_handle   -- DemuraLayerSelector句柄
	@param  : input_meter_list[]						-- W所有灰阶的meter图
    @param  : in_meter_num								-- meter数量
    @param  : out_w_correction_gray_level				-- 校正灰阶
	@return	: AlgoError
	@Author : lichaoping@2024/12/24
	************************************/
    C_API AlgoError C_CALL DemuraLayerSelectorExportWCorrectionGrayPyramid(
        AlgoHandle demura_layer_selector_handle,
        AlgoHandle input_meter_list[], int in_meter_num, AlgoMat *out_w_correction_gray_level);

    /************************************
	@brief  : 判断RGB单灰阶or全灰阶校正
	@note   :
	@param  : demura_layer_selector_handle   -- DemuraLayerSelector句柄
	@param  : in_r_meter_list							-- R通道每个灰阶下的meter
	@param  : in_g_meter_list							-- G通道每个灰阶下的meter
	@param  : in_b_meter_list							-- B通道每个灰阶下的meter
    @param  : in_meter_len								-- 各通道的meter数量
	@param  : out_need_rgb_mult_correction_flag				-- RGB是否全灰阶校正结果（1为需要，0为不需要）
	@return	: AlgoError
	@Author : lichaoping@2024/09/30
	************************************/
    C_API AlgoError C_CALL DemuraLayerSelectorJudgeRGBMultiLevelCorrection(
        AlgoHandle demura_layer_selector_handle,
        AlgoHandle in_r_meter_list[], AlgoHandle in_g_meter_list[],
        AlgoHandle in_b_meter_list[], int in_meter_len, int *out_need_rgb_mult_correction_flag);

	/************************************
	@brief  : 输出RGB校正灰阶金字塔
	@note   :
	@param  : demura_layer_selector_handle   -- DemuraLayerSelector句柄
	@param  : out_gray_pyramid							-- RGB校正的灰阶金字塔
	@return	: AlgoError
	@Author : lichaoping@2024/12/24
	************************************/
    C_API AlgoError C_CALL DemuraLayerSelectorExportRGBCorrectionGrayPyramid(
        AlgoHandle demura_layer_selector_handle, AlgoMat *out_gray_pyramid);


	/************************************
	@brief  : 计算亮度差异
	@note   :
	@param  : demura_layer_selector_handle   -- DemuraLayerSelector句柄
    @param  : in_mult_data								-- 多个Lxy数据
    @param  : in_data_len								-- Lxy数据长度
    @param  : out_lumin_uniformity						-- 亮度均匀性值,0-1数值，
	@param  : out_result_flag							-- 判断是否亮度差异大的结果标志
	@return	: AlgoError
	@Author : lichaoping@2024/09/30
	************************************/
    C_API AlgoError C_CALL
    DemuraLayerSelectorCalLuminanceDiff(
        AlgoHandle demura_layer_selector_handle,
        Ca_Lxy *in_mult_data, int in_data_len,
        double *out_lumin_uniformity, bool *out_result_flag);

	/************************************
	@brief  : 计算色度差异
	@note   :
	@param  : demura_layer_selector_handle   -- DemuraLayerSelector句柄
    @param  : in_mult_data								-- 多个Lxy数据
    @param  : in_data_len								-- Lxy数据长度
    @param  : out_x_deviation_value						-- 计算得到的x坐标偏差值
    @param  : out_y_deviation_value						-- 计算得到的y坐标偏差值
	@param  : out_result_flag							-- 判断是否色度差异大的结果标志,true为差异大
	@return	: AlgoError
	@Author : lichaoping@2024/09/30
	************************************/
    C_API AlgoError C_CALL
    DemuraLayerSelectorCalChromaDiff(
        AlgoHandle demura_layer_selector_handle,
        Ca_Lxy *in_mult_data, int in_data_len,
        double *out_x_deviation_value, double *out_y_deviation_value,
        bool *out_result_flag);
	
		/************************************
	@brief  : 计算箱体内亮度差异
	@note   :
	@param  : demura_layer_selector_handle   -- DemuraLayerSelector句柄
	@param  : in_mult_data								-- 多个Lxy数据
	@param  : in_data_len								-- Lxy数据长度
	@param  : out_lumin_uniformity						-- 亮度均匀性值
	@param  : out_result_flag							--判断是否亮度度差异大的结果标志,true为差异大
	@return	: AlgoError
	@Author : lichaoping@2024/12/04
	************************************/
    C_API AlgoError C_CALL DemuraLayerSelectorCalInnerLuminanceDiff(
        AlgoHandle demura_layer_selector_handle, Ca_Lxy *in_mult_data,
        int in_data_len, double *out_lumin_uniformity, bool *out_result_flag);

    /************************************
    @brief  : 计算箱体内色度差异
    @note   :
    @param  : demura_layer_selector_handle   -- DemuraLayerSelector句柄
    @param  : in_mult_data								-- 多个Lxy数据
    @param  : in_data_len								-- Lxy数据长度
    @param  : out_x_deviation_value						-- 计算得到的x坐标偏差值
    @param  : out_y_deviation_value						-- 计算得到的y坐标偏差值
    @param  : out_result_flag							-- 判断是否色度差异大的结果标志
    @return	: AlgoError
    @Author : lichaoping@2024/12/04
    ************************************/
    C_API AlgoError C_CALL DemuraLayerSelectorCalInnerChromaDiff(
        AlgoHandle demura_layer_selector_handle, Ca_Lxy *in_mult_data,
        int in_data_len, double *out_x_deviation_value,
        double *out_y_deviation_value, bool *out_result_flag);

	/************************************
	@brief  : 输出最终校正方案推荐
	@note   :
	@param  : demura_layer_selector_handle   -- DemuraLayerSelector句柄
	@param  : in_w_corretion_flag						-- 是否校正白色的结果标志，白色校正为true
    @param  : in_w_correction_gray						-- w校正灰阶数组；
    @param  : in_rgb_all_gray_collection_flag			-- rgb 是否全灰阶校正标志,全灰阶校正为true
    @param  : in_rgb_correction_gray					-- rgb校正灰阶
    @param  : in_inner_color_diff						-- 箱体内色度差异，差异大为true
    @param  : in_mult_cabinet							-- 单箱or多箱，多箱为true
    @param  : in_outer_color_diff						-- 箱体间亮色度差异，亮色度差异大为true
	@param  : out_rgb_shceme_index						-- rgb校正推荐的方案：  1为亮色度校正 2为高级亮色度校正  3为高精度校正  4 全灰阶校正
	@param  : out_rgb_correction_gray					-- rgb校正灰阶金字塔
    @param  : out_w_shceme_index						-- w校正推荐的方案：0为无需白色校正 5 为单灰阶校正  6 白色全灰阶校正
    @param  : out_w_correction_gray						-- w校正灰阶金字塔；
	@return	: AlgoError
	@Author : lichaoping@2024/09/30
	************************************/
    C_API AlgoError C_CALL DemuraLayerSelectorExportCorrectionScheme(
        AlgoHandle demura_layer_selector_handle,
        bool in_w_corretion_flag, bool in_w_mult_correction_flag, AlgoMat in_w_correction_gray, bool in_rgb_all_gray_collection_flag,
        AlgoMat in_rgb_correction_gray, bool in_inner_color_diff,
        bool in_mult_cabinet, bool in_outer_color_diff,
        int &out_rgb_shceme_index, AlgoMat* out_rgb_correction_gray,
        int &out_w_shceme_index, AlgoMat* out_w_correction_gray);


	/************************************
	@brief  : 对meter数据进行相机参数归一化
	@note   :
	@param  : demura_layer_selector_handle   -- DemuraLayerSelector句柄
	@param  : meter_hadndle								-- LedMeter句柄
	@return	: AlgoError
	@Author : lichaoping@2024/09/30
	************************************/
    C_API AlgoError C_CALL DemuraLayerSelectorNormalizeMeter(
        AlgoHandle demura_layer_selector_handle,
        AlgoHandle meter_handle);


	/************************************
	@brief  : 保存rgbw的mura图
	@note   :
	@param  : demura_layer_selector_handle   -- DemuraLayerSelector句柄
	@param  : in_save_path_name								-- LedMeter句柄
	@return	: AlgoError
	@Author : lichaoping@2024/09/30
	************************************/
    C_API AlgoError C_CALL DemuraLayerSelectorExportMuraImage(
        AlgoHandle demura_layer_selector_handle,
        const char *in_save_path_name);


	/************************************
	@brief  : 计算Lxy亮度均匀性和色度非均匀性
	@note   :
	@param  : demura_layer_selector_handle   -- DemuraLayerSelector句柄
	@param  : in_X_meter							-- Xmeter句柄
	@param  : in_Y_meter							-- Ymeter句柄
	@param  : out_l_uniformaty						-- 亮度均匀性值
	@param  : out_xy_uniformaty						-- xy非均匀性值
    @param  : out_lumin_mat						--Y亮度图
    @param  : out_xy_point_mat					--xy坐标
    @param  : in_Z_meter						--Zmeter句柄，默认为空，主要考虑为RZ很小
    @param  : in_point_num						--输出点的数量，默认为10
	@return	: AlgoError
	@Author : lichaoping@2024/12/05
	************************************/
    C_API AlgoError C_CALL DemuraLayerSelectorCalUniformatyLxy(
        AlgoHandle demura_layer_selector_handle, AlgoHandle in_X_meter,
        AlgoHandle in_Y_meter, double *out_l_uniformaty,
        double *out_xy_uniformaty, AlgoMat *out_lumin_mat,
        AlgoMat *out_xy_point_mat, AlgoHandle in_Z_meter = nullptr,
        int in_point_num = 10);

    /************************************
    @brief  : 计算Yuv亮度均匀性和色度非均匀性
    @note   :
    @param  : demura_layer_selector_handle   -- DemuraLayerSelector句柄
    @param  : in_X_meter							-- Xmeter句柄
    @param  : in_Y_meter							-- Ymeter句柄
    @param  : out_l_uniformaty						-- 亮度均匀性值
    @param  : out_uv_uniformaty						-- 色度非均匀性值
    @param  : out_lumin_mat						--Y亮度图
    @param  : out_upvp_point_mat				--uv坐标
    @param  : in_Z_meter						--Zmeter句柄，默认为空，主要考虑为RZ很小
    @param  : in_point_num						--输出点的数量，默认为10
    @return	: AlgoError
    @Author : lichaoping@2024/12/05
    ************************************/
    C_API AlgoError C_CALL DemuraLayerSelectorCalUniformatyYuv(
        AlgoHandle demura_layer_selector_handle, AlgoHandle in_X_meter,
        AlgoHandle in_Y_meter, double *out_l_uniformaty,
        double *out_upvp_uniformaty, AlgoMat *out_lumin_mat, AlgoMat *out_upvp_point_mat, AlgoHandle in_Z_meter = nullptr,int in_point_num = 10);


    /************************************
    @brief  : 获取buffer大小
    @note   :
    @param  : demura_layer_selector_handle  当前有效句柄
    @param  : out_len     buffer大小
    @return	: AlgoError
    @Author : lichaoping@2024/09/29
    ************************************/
    C_API AlgoError C_CALL DemuraLayerSelectorGetBufferSize(
        AlgoHandle demura_layer_selector_handle, uint64_t *out_len);


	/************************************
	@brief  : 将所有设定的阈值导出到buffer
	@note   :
	@param  : demura_layer_selector_handle 有效的DemuraLayerSelector句柄
	@param  : out_buffer        已经申请好申请空间
	@param  : in_out_len        *in_out_len外部申请好的空间大小,会用真实大小修改
	@return	: AlgoError
	@Author : lichaoping@2024/09/29
	************************************/
    AlgoError DemuraLayerSelectorToBuffer(
        AlgoHandle demura_layer_selector_handle, char *out_buffer,
        uint64_t *in_out_len);


	/************************************
	@brief  : 将buffer数据导入到DemuraLayerSelector
	@note   :
	@param  : demura_layer_selector_handle
	有效的DemuraLayerSelector句柄
	@param  : in_buffer
	@param  : in_len
	@return	: AlgoError
	@Author : lichaoping@2024/09/29
	************************************/
    C_API AlgoError C_CALL DemuraLayerSelectorFromBuffer(
        AlgoHandle demura_layer_selector_handle,
                                    const char *in_buffer, uint64_t in_len);
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
    C_API AlgoError C_CALL DemuraLayerSelectorRelease(
        AlgoHandle demura_layer_selector_handle);

    C_API void C_CALL DemuraLayerSelectorDestroyHandle(
        AlgoHandle demura_layer_selector_handle);

#ifdef __cplusplus
};
#endif // __cplusplus
#endif
