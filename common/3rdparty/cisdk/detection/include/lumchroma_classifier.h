#pragma once
#ifndef __LUMCHROMA_CLASSIFIER_H__
#define __LUMCHROMA_CLASSIFIER_H__
#include "ci_define.h"
#include "ci_declare.h"
#include "lumchroma_classifier_error.h"
#include "lumchroma_classifier_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	/************************************
	@Brief		：	创建侧面亮色度分选句柄.
	@Note		：
	@Parameter	: 	lum_chroma_handle           -- 有效的亮色度特征句柄
	@Returns		:   AlgoError
	@Author		：	lipengpeng@2022/06/30
	************************************/
	C_API AlgoError C_CALL LumChromaCreateHandle(AlgoHandle* lum_chroma_handle);


	/************************************
	@Brief		：	设置阈值.
	@Note		：
	@Parameter	: 	lum_chroma_handle           -- 有效的亮色度特征句柄
	@Parameter	: 	in_para			            -- 侧面亮色度分选阈值输入
	@Returns		:   AlgoError
	@Author		：	lipengpeng@2022/06/30
	************************************/
	C_API AlgoError C_CALL LumChromaSetClassifyPara(AlgoHandle lum_chroma_handle, LumChromaClassifyThreshold* in_para);


	/************************************
	@Brief		：	阈值输出.
	@Note		：
	@Parameter	: 	lum_chroma_handle           -- 有效的亮色度特征句柄
	@Parameter	: 	out_para			        -- 侧面亮色度分选阈值输出
	@Returns		:   AlgoError
	@Author		：	lipengpeng@2022/06/30
	************************************/
	C_API AlgoError C_CALL LumChromaGetClassifyPara(AlgoHandle lum_chroma_handle, LumChromaClassifyThreshold* out_para);


	/************************************
	@Brief		：	读取相机属性文件 .cam
	@Note		：
	@Parameter	: 	lum_chroma_handle           -- 有效的亮色度特征句柄
	@Parameter	: 	in_cam_index			    -- 相机索引号
	@Parameter	: 	in_cam_filename			    -- 文件读取路径
	@Returns	:   AlgoError
	@Author		：	lipengpeng@2022/06/30
	************************************/
	C_API AlgoError C_CALL LumChromaLoadCaptureProperty(AlgoHandle lum_chroma_handle, int in_cam_index, const char* in_cam_filename);


	/************************************
	@Brief		：	计算到XYZ
	@Note		：
	@Parameter	: 	lum_chroma_handle			-- 有效的亮色度特征句柄
	@Parameter	: 	in_shot					    -- shot句柄
	@Parameter	: 	in_cam_index			    -- 相机索引号
	@Returns	:   AlgoError
	@Author		：	lipengpeng@2022/06/30
	************************************/
	C_API AlgoError C_CALL LumChromaCalc(AlgoHandle lum_chroma_handle, AlgoHandle in_shot, int in_cam_index);


	/************************************
	@Brief		：	灯板的camRGB和XYZ获取
	@Note		：
	@Parameter	: 	lum_chroma_handle			-- 有效的亮色度特征句柄
	@Parameter	: 	out_info			        -- 输出相机camRGB和XYZ分量
	@Returns	:   AlgoError
	@Author		：	lipengpeng@2022/06/30
	************************************/
	C_API AlgoError C_CALL LumChromaGetCalcInfo(AlgoHandle lum_chroma_handle, AlgoMat* out_info);


	/************************************
	@Brief		：	XYZ到指定的亮色度分选表征量
	@Note		：
	@Parameter	: 	lum_chroma_handle			-- 有效的亮色度特征句柄
	@Parameter	: 	out_colorcast_feature  	    -- 输出亮色度分选表征量
	@Returns	:   AlgoError
	@Author		：	lipengpeng@2022/06/30
	************************************/
	C_API AlgoError C_CALL LumChromaCalcColorcast(AlgoHandle lum_chroma_handle, Colorcast* out_colorcast_feature);


	/************************************
	@Brief		：	主色偏度分选，次亮度分选
	@Note		：
	@Parameter	: 	lum_chroma_handle			-- 有效的亮色度特征句柄
	@Parameter	: 	in_colorcast_feature	    -- 输入亮色度分选表征量
	@Parameter	: 	out_class_id	            -- 输出分类结果坐标
	@Returns	:   AlgoError
	@Author		：	lipengpeng@2022/06/30
	************************************/
	C_API AlgoError C_CALL LumChromaClassifyColorcast(AlgoHandle lum_chroma_handle, Colorcast* in_colorcast_feature, AlgoPoint* out_class_id);


	/************************************
	@Brief		：	主亮度分选，次色偏度分选
	@Note		：
	@Parameter	: 	lum_chroma_handle			-- 有效的亮色度特征句柄
	@Parameter	: 	in_colorcast_feature	    -- 输入亮色度分选表征量
	@Parameter	: 	out_class_id	            -- 输出分类结果坐标
	@Returns	:   AlgoError
	@Author		：	lipengpeng@2022/09/23
	************************************/
	C_API AlgoError C_CALL LumChromaClassifyLuminance(AlgoHandle lum_chroma_handle, Colorcast* in_colorcast_feature, AlgoPoint* out_class_id);


	/************************************
	@Brief		：	亮色度屏体异常筛选
	@Note		：
	@Parameter	: 	lum_chroma_handle			-- 有效的亮色度特征句柄
	@Parameter	: 	in_colorcast_feature	    -- 输入亮色度分选表征量
	@Parameter	: 	cam_num             	    -- 检测使用相机数量：2代表2个侧面相机，3代表1个正面2个侧面相机
	@Parameter	: 	in_method             	    -- 检测方法：0：代表比值方法，1：代表差值方法
	@Parameter	: 	out_result  	            -- 输出检测结果：0代表ok，1代表ng
	@Returns	:   AlgoError
	@Author		：	lipengpeng@2023/10/31
	************************************/
	C_API AlgoError C_CALL LumChromaCheckAbnormal(AlgoHandle lum_chroma_handle, Colorcast* in_colorcast_feature, int cam_num, int in_method, int* out_result);


	/************************************
	@Brief		：	校正前后亮度或色偏度分选一致性
	@Note		：
	@Parameter	: 	lum_chroma_handle			-- 有效的亮色度特征句柄
	@Parameter	: 	in_delta_1	                -- 输入亮度或色偏度特征值（主相机与侧面相机1关联后的特征值）
	@Parameter	: 	in_delta_2	                -- 输入亮度或色偏度特征值（主相机与侧面相机2关联后的特征值）
	@Parameter	: 	offset	                    -- 偏移量（亮度或）,offset>0，主要是想坐标亮度（或色偏度）x、y不为负值，这个值建议开放到软件端
	@Parameter	: 	out_class_id	            -- 输出分类结果坐标
	@Returns	:   AlgoError
	@Author		：	lipengpeng@2023/06/09
	************************************/
	C_API AlgoError C_CALL LumChromaClassifyConsistency(AlgoHandle lum_chroma_handle, float in_delta_1, float in_delta_2, float offset, AlgoPoint* out_class_id);


	/************************************
	@Brief		：	获取亮色度分选LumChromaClassify的大小
	@Note		：
	@Parameter	: 	lum_chroma_handle			-- 有效的亮色度特征句柄
	@Parameter	: 	out_len				        -- buffer的大小
	@Returns	:   AlgoError
	@Author		：	lipengpeng@2022/06/30
	************************************/
	C_API AlgoError C_CALL LumChromaGetColorcastBufferSize(AlgoHandle lum_chroma_handle, uint64_t* out_len);


	/************************************
	@brief		: 将LumChromaClassify里面的数据导出到buffer
	@note		:
	@Parameter	: 	lum_chroma_handle			-- 有效的亮色度特征句柄
	@Parameter	:   out_buffer			        -- 已经申请好的空间
	@Parameter	:   out_len				        -- 外部申请好的空间大小,会用真实大小修改
	@return		:   AlgoError
	@Author		：	lipengpeng@2022/06/30
	************************************/
	C_API AlgoError C_CALL LumChromaColorcastToBuffer(AlgoHandle lum_chroma_handle, char* out_buffer, uint64_t* out_len);


	/************************************
	@brief		: 将buffer里面的数据导入到LumChromaClassify
	@note		:
	@Parameter	: 	lum_chroma_handle			-- 有效的亮色度特征句柄
	@Parameter	:   in_buffer				    -- 已经申请好申请空间
	@Parameter	:   in_len				        -- 外部申请好的空间大小,会用真实大小修改
	@return		:   AlgoError
	@Author		：	lipengpeng@2022/06/30
	************************************/
	C_API AlgoError C_CALL LumChromaColorcastFromBuffer(AlgoHandle lum_chroma_handle, const char* in_buffer, uint64_t in_len);


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
	C_API AlgoError C_CALL LumChromaRelease(AlgoHandle lum_chroma_handle);

	C_API void C_CALL LumChromaDestroyHandle(AlgoHandle lum_chroma_handle);


#ifdef __cplusplus
}
#endif // __cplusplus
#endif