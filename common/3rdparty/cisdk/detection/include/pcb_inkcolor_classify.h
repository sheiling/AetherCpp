#ifndef __PCB_INKCOLOR_CLASSIFY_H__
#define __PCB_INKCOLOR_CLASSIFY_H__

#include "ci_define.h"
#include "ci_declare.h"
#include "pcb_inkColor_classify_error.h"
#include "pcb_inkColor_classify_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


	/************************************
   @Brief		：	创建墨色句柄.
   @Note		：
   @Parameter	: 	pcb_inkcolor_handle -- 墨色特征句柄
   @Returns		:   AlgoError
   @Author		：	mumeng@2023/6/16
   ************************************/
	C_API AlgoError C_CALL PCBInkColorCreateHandle(AlgoHandle* pcb_inkcolor_handle);


	/************************************
   @Brief		：	设置相机数量和图像位深
   @Note		：	
   @Parameter	: 	pcb_inkcolor_handle   -- 有效的墨色特征句柄
   @Parameter	: 	in_cam_num			  -- 相机数量
   @Parameter	: 	in_img_depth	      -- 采集图像位深
   @Returns	    :   AlgoError
   @Author		：	mumeng@2023/9/19
   ************************************/
	C_API AlgoError C_CALL PCBInkColorSetCamNumAndDepth(AlgoHandle pcb_inkcolor_handle, const int in_cam_num, const int in_img_depth);


	/************************************
   @Brief		：	设置设备标定参数
   @Note		：
   @Parameter	: 	pcb_inkcolor_handle   -- 有效的墨色特征句柄
   @Parameter	: 	dev_calib_handle	  -- 设备标定句柄
   @Returns	    :   AlgoError
   @Author		：	mumeng@2023/9/19
   ************************************/
	C_API AlgoError C_CALL PCBInkColorSetCalibFactor(AlgoHandle pcb_inkcolor_handle, AlgoHandle dev_calib_handle);



	/************************************
	@Brief		：	框选角点区域，计算灯板的mask。
	@Note		：
	@Parameter	: 	pcb_inkcolor_handle -- 有效的墨色特征句柄
	@Parameter	: 	in_panel_corner		-- 框选的4个角点坐标
	@Parameter	: 	in_point_num		-- 角点数量
	@Parameter	: 	in_cam_index		-- 相机索引,从 0 开始
	@Returns	:   AlgoError
	@Author		：	mumeng@2023/9/20
	************************************/
	C_API AlgoError C_CALL PCBInkColorSetPanelROI(AlgoHandle pcb_inkcolor_handle, AlgoSize* in_img_wh,
		const AlgoPoint* in_panel_corner, const int in_point_num, const int in_cam_index);


	/************************************
	@Brief		：	计算并设置焊盘的面积、宽、高
	@Note		：
	@Parameter	: 	pcb_inkcolor_handle     -- 有效的墨色特征句柄
	@Parameter	: 	in_panel_mat            -- 灯板图像
	@Parameter	: 	in_pad_corner		    -- 框选的4个灯珠区域角点、16个焊盘区域
	@Parameter	: 	in_point_num			-- 角点数量
	@Parameter	: 	in_cam_index	        -- 相机索引,从 0 开始
	@Returns	:   AlgoError
	@Author		：	mumeng@2023/9/19
	************************************/
	C_API AlgoError C_CALL PCBInkColorSetPadSizeInfo(AlgoHandle pcb_inkcolor_handle, const AlgoMat* in_panel_mat, 
		const AlgoPoint* in_pad_corner, const int in_point_num, const int in_cam_index);


	/************************************
   @Brief		：	设置相机相机权重、测试区域权重
   @Note		：	
   @Parameter	: 	pcb_inkcolor_handle   -- 有效的墨色特征句柄
   @Parameter	: 	in_cam_weight		  -- 相机权重列表
   @Parameter	: 	in_cam_weight_len	  -- 相机权重列表的长度
   @Parameter	: 	in_region_weigth	  -- 测量区域权重列表
   @Parameter	: 	in_region_weigth_len  -- 区域权重列表的长度
   @Returns	    :   AlgoError
   @Author		：	mumeng@2023/9/19
   ************************************/
	C_API AlgoError C_CALL PCBInkColorSetCamAndReionWeight(AlgoHandle pcb_inkcolor_handle, 
		const float* in_cam_weight, const int in_cam_weight_len,
		const float* in_region_weigth, const int in_region_weigth_len);


	/************************************
   @Brief		：	设置特征颜色空间
   @Note		：	
   @Parameter	: 	pcb_inkcolor_handle   -- 有效的墨色特征句柄
   @Parameter	: 	in_color_sapce	      -- 指定的特征颜色空间，默认LCH颜色空间
   @Returns	    :   AlgoError
   @Author		：	mumeng@2023/9/18
   ************************************/
	C_API AlgoError C_CALL PCBInkColorSetFeatureColorSpace(AlgoHandle pcb_inkcolor_handle, ColorSpace in_color_sapce);



   /************************************
   @Brief		：	设置焊盘扣除面积阈值
   @Note		：	
   @Parameter	: 	pcb_inkcolor_handle   -- 有效的墨色特征句柄
   @Parameter	: 	in_pad_area_th_list	  -- 筛选焊盘的面积参数列表，一个相机一个值，默认值都是0.8。
   @Parameter	: 	in_pad_area_th_len	  -- 列表长度
   @Returns	    :   AlgoError
   @Author		：	mumeng@2023/9/19
   ************************************/
	C_API AlgoError C_CALL PCBInkColorSetPadAreaThresh(AlgoHandle pcb_inkcolor_handle, const float* in_pad_area_th_list, int in_pad_area_th_len);



   /************************************
   @Brief		：	设置判定灯板是否均匀的阈值
   @Note		：	
   @Parameter	: 	pcb_inkcolor_handle   -- 有效的墨色特征句柄
   @Parameter	: 	in_pad_area_th	      -- 筛选焊盘的面积参数，
                                             默认 Brightness：0.8，color1:0.1， color2:0.02， 
											 panel_blance: 0.0， compresse_chroma：0.6
   @Returns	    :   AlgoError
   @Author		：	mumeng@2023/9/19
   ************************************/
	C_API AlgoError C_CALL PCBInkColorSetPanelUniformThresh(AlgoHandle pcb_inkcolor_handle, UniformThresh in_area_th);


	/************************************
	@Brief		：	计算PCB缝隙墨色信息
	@Note		：
	@Parameter	: 	pcb_inkcolor_handle			-- 有效的墨色特征句柄
	@Parameter	: 	in_img						-- 待检测图像
	@Parameter	: 	in_cam_idx				    -- 相机索引号， 从 0 开始
	@Returns	:   AlgoError
	@Author		：	mumeng@2023/9/18
	************************************/
	C_API AlgoError C_CALL PCBInkColorCalcPCBGapInfo(AlgoHandle pcb_inkcolor_handle, const AlgoMat* in_img, 
		const int in_cam_idx);



	/************************************
	@Brief		：	获取灯板的camRGB 、XYZ 、Lch 
	@Note		：   
	@Parameter	: 	pcb_inkcolor_handle			-- 有效的墨色特征句柄
	@Parameter	: 	out_info					-- 输出的图像信息，按照列分别是 R G B X Y Z L C H 
	@Returns	:   AlgoError
	@Author		：	mumeng@2023/8/30
	************************************/
	C_API AlgoError C_CALL PCBInkColorGetGapInfo(AlgoHandle pcb_inkcolor_handle, AlgoMat* out_info);


	/************************************
	@Brief		：	计算灯板墨色特征
	@Note		：
	@Parameter	: 	pcb_inkcolor_handle			-- 有效的墨色特征句柄
	@Parameter	: 	out_inkfeature				-- 输出的墨色特征
	@Parameter	: 	in_region_index				-- 第几个区域的索引号，从 0 开始， 默认为0
	@Returns	:   AlgoError
	@Author		：	mumeng@2023/9/18
	************************************/
	C_API AlgoError C_CALL PCBInkColorCalcFeature(AlgoHandle pcb_inkcolor_handle, const int in_region_index = 0);


	/************************************
	@Brief		：	输出指定角度相机的墨色特征信息
	@Note		：
	@Parameter	: 	pcb_inkcolor_handle			-- 有效的墨色特征句柄
	@Parameter	: 	out_cam_inkfeature			-- 输出指定角度相机的墨色特征
	@Parameter	: 	in_cam_index				-- 相机索引编号， 从0开始
	@Returns	:   AlgoError
	@Author		：	mumeng@2023/9/19
	************************************/
	C_API AlgoError C_CALL PCBInkColorCalcCamFeature(AlgoHandle pcb_inkcolor_handle, InkColorFeature* out_cam_inkfeature, int in_cam_index);


	/************************************
	@Brief		：	判断灯板不同区域墨色是否均匀
	@Note		：
	@Parameter	: 	pcb_inkcolor_handle			-- 有效的墨色特征句柄
	@Parameter	: 	in_method_flag				-- 计算哪个量的极差
	@Parameter	: 	out_uniform_ret             -- 判断结果， 1代表灯板均匀； -1代表灯板不均匀，NG
	@Returns	:   AlgoError
	@Author		：	mumeng@2023/9/18
	************************************/
	C_API AlgoError C_CALL PCBInkColorJudgePanelUniform(AlgoHandle pcb_inkcolor_handle, ClassifyMethod in_method_flag, int* out_uniform_ret);
	


	/************************************
	@Brief		：	计算多个区域灯板的加权均匀墨色值
	@Note		：
	@Parameter	: 	pcb_inkcolor_handle			-- 有效的墨色特征句柄
	@Parameter	: 	out_inkfeature			    -- 输出的墨色特征
	@Returns	:   AlgoError
	@Author		：	mumeng@2023/9/18
	************************************/
	C_API AlgoError C_CALL PCBInkColorCalcMultRegionFeature(AlgoHandle inkcolor_handle_pcb, InkColorFeature* out_inkfeature);


	/************************************
	@Brief		：	获取最终的墨色特征值
	@Note		：
	@Parameter	: 	pcb_inkcolor_handle			-- 有效的墨色特征句柄
	@Parameter	: 	out_inkfeature	            -- 输出的墨色特征
	@Returns	:   AlgoError
	@Author		：	mumeng@2023/9/18
	************************************/
	C_API AlgoError C_CALL PCBInkColorGetPanelFeature(AlgoHandle inkcolor_handle_pcb, InkColorFeature* out_inkfeature);



	/************************************
	@Brief		：	获取多个区域墨色值列表
	@Note		：
	@Parameter	: 	pcb_inkcolor_handle			  -- 有效的墨色特征句柄
	@Parameter	: 	out_region_inkfeature_list	  -- 输出的墨色特征
	@Returns	:   AlgoError
	@Author		：	mumeng@2023/9/18
	************************************/
	C_API AlgoError C_CALL PCBInkColorGetRegionFeatureList(AlgoHandle inkcolor_handle_pcb, InkColorFeature* out_region_inkfeature_list);



	/************************************
	@Brief		：	生成指定特征值的分类基类List 和 分类编号List
	@Note		：	
	@Parameter	: 	min_val					-- 分类区间的最小值
	@Parameter	: 	max_val					-- 分类区间的最大值
	@Parameter	: 	class_num				-- 分类数量
	@Parameter	: 	in_mothod				-- 生成哪个量的List
											 (1--Brightness、 2--Color1、 3--Color2、 
											  4--PanelBlance（各向异性）、5--Color1*Color2)
	@Parameter	: 	out_feature_val_list	-- 输出的特征值分类List
	@Parameter	: 	out_label_list			-- 输出的分类编号List
	@Returns	:   AlgoError
	@Author		：	mumeng@2023/6/16
	************************************/
	C_API AlgoError C_CALL PCBInkColorMakeClassifyList(float in_min_val, float in_max_val, int in_class_num,
		 float* out_feature_val_list, int* out_label_list);


	/************************************
	@Brief		：	对墨色表征量进行分类 
	@Note		：	调用一次，分类一个特征， 多维分类，多次调用。
	@Parameter	: 	in_feature				-- 存放墨色特征的结构体
	@Parameter	: 	in_mothod				-- 需要分类第几个量，对应关系如下
											（1--Brightness、 2--Color1、 3--Color2、 
											  4--PanelBlance（各向异性）、5--color1*color2）
	@Parameter	: 	in_feature_val_list		-- 所有灯板在该分量的分类区间列表
	@Parameter	: 	in_feature_val_list_len	-- 分类区间list的长度
	@Parameter	: 	in_label_list			-- 预设的分类编号列表
	@Parameter	: 	in_label_list_len		-- 分类编号list的长度
	@Parameter	: 	out_label				-- 输出的分类结果
	@Returns	:   AlgoError
	@Author		：	mumeng@2023/6/16
	************************************/
	C_API AlgoError C_CALL PCBInkColorClassify(const InkColorFeature* in_feature, ClassifyMethod in_method_flag,
		const float* in_feature_val_list, int in_feature_val_list_len,
		int* in_label_list, int in_label_list_len, 
		int* out_label);




	/************************************
	@Brief		：	保存
	@Note		：
	@Parameter	: 	pcb_inkcolor_handle			-- 有效的墨色特征句柄
	@Parameter	: 	in_cam_file_path			-- 文件路径
	@Returns	:   AlgoError
	@Author		：	mumeng@2023/6/16
	************************************/
	C_API AlgoError C_CALL PCBInkColorInkFeatureSave(AlgoHandle pcb_inkcolor_handle, const char* in_cam_file_path);
	
	C_API AlgoError C_CALL PCBInkColorInkFeatureLoad(AlgoHandle pcb_inkcolor_handle, const char* in_cam_file_path);



	/************************************
	@Brief		：	获取墨色分类句柄内容 InkFeatureClassify 的大小
	@Note		：
	@Parameter	: 	pcb_inkcolor_handle			-- 有效的墨色特征句柄
	@Parameter	: 	out_len					-- buffer的大小
	@Returns	:   AlgoError
	@Author		：	mumeng@2023/6/16
	************************************/
	C_API AlgoError C_CALL PCBInkColorGetInkFeatureBufferSize(AlgoHandle pcb_inkcolor_handle, uint64_t* out_len);


	/************************************
	@brief		: 将InkFeatureClassify里面的数据导出到buffer
	@note		:
	@Parameter	: pcb_inkcolor_handle		-- 有效的墨色特征句柄
	@Parameter	: out_buffer			-- 已经申请好的空间
	@Parameter	: out_len				-- 外部申请好的空间大小,会用真实大小修改
	@return		: AlgoError
	@Author		: mumeng@2023/6/16
	************************************/
	C_API AlgoError C_CALL PCBInkColorInkFeatureToBuffer(AlgoHandle pcb_inkcolor_handle, char* out_buffer, uint64_t* out_len);



	/************************************
	@brief		: 将buffer里面的数据导入到InkFeatureClassify
	@note		:
	@Parameter	: pcb_inkcolor_handle		-- 有效的墨色特征句柄
	@Parameter	: in_buffer				-- 已经申请好申请空间
	@Parameter	: in_len				-- 外部申请好的空间大小,会用真实大小修改
	@return		: AlgoError
	@Author		: mumeng@2023/6/16
	************************************/
	C_API AlgoError C_CALL PCBInkColorInkFeatureFromBuffer(AlgoHandle pcb_inkcolor_handle, const char* in_buffer, uint64_t in_len);


	/************************************
	@Brief		：	灯板拼屏
	@Note		：	in_panel_feature_list		-- 灯板特征信息
	@Parameter	: 	in_panel_cols				-- 目标屏幕灯板列数
	@Parameter	: 	in_panel_rows				-- 目标屏幕灯板行数
	@Parameter	: 	out_panel_pos				-- 输出拼屏位置结果
	@Returns	:   AlgoError
	@Author		：	mumeng@2023/6/16
	************************************/
	C_API AlgoError C_CALL PCBInkColorSpliceScreen(InkColorFeature* in_panel_feature_list, int in_panel_cols, int in_panel_rows, AlgoPoint* out_panel_pos);


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
	C_API AlgoError C_CALL PCBInkColorRelease(AlgoHandle pcb_inkcolor_handle);

	C_API void C_CALL PCBInkColorDestroyHandle(AlgoHandle pcb_inkcolor_handle);


#ifdef __cplusplus
}
#endif // __cplusplus
#endif
