#ifndef __INKCOLOR_CLASSIFY_H__
#define __INKCOLOR_CLASSIFY_H__

#include "ci_define.h"
#include "ci_declare.h"
#include "inkcolor_classify_error.h"
#include "inkcolor_classify_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


	/************************************
   @Brief		：	创建墨色句柄.
   @Note		：
   @Parameter	: 	inkcolor_handle -- 墨色特征句柄
   @Returns		:   AlgoError
   @Author		：	mumeng@2021/11/30
   ************************************/
	C_API AlgoError C_CALL InkColorCreateHandle(AlgoHandle* inkcolor_handle);


	/************************************
   @Brief		：	设置相机数量
   @Note		：	
   @Parameter	: 	inkcolor_handle -- 有效的墨色特征句柄
   @Parameter	: 	in_cam_num      -- 相机数量
   @Returns	    :   AlgoError
   @Author		：	mumeng@2021/12/7
   ************************************/
	C_API AlgoError C_CALL InkColorSetCamNumAndDepth(AlgoHandle inkcolor_handle, const int in_cam_num, const int in_cam_depth);




    /************************************
   @Brief		：	设置相机数量
   @Note		：	
   @Parameter	: 	inkcolor_handle -- 有效的墨色特征句柄
   @Parameter	: 	in_cam_num      -- 相机数量
   @Returns	    :   AlgoError
   @Author		：	mumeng@2025/12/31
   ************************************/
	C_API AlgoError C_CALL InkColorSetMonitorThresh(AlgoHandle inkcolor_handle, const float in_math_th, const float in_dis_th, MonitPositionFlag in_monit_flag);



	 /************************************
	@Brief		：	计算LED灯板图像的Mask,手动区域定位
	@Note		：
	@Parameter	: 	inkcolor_handle -- 有效的墨色特征句柄
	@Parameter	: 	in_roi		    -- ROI句柄,灯板墨色的计算区域
	@Parameter	: 	in_cam_index	-- 相机索引号
	@Returns	:   AlgoError
	@Author		：	mumeng@2021/11/30
	************************************/
	C_API AlgoError C_CALL InkColorSetROI(AlgoHandle inkcolor_handle, AlgoHandle in_roi, const int in_cam_index);



	 /************************************
	@Brief		：	设置用于相机位置标记的角点
	@Note		：
	@Parameter	: 	inkcolor_handle -- 有效的墨色特征句柄
	@Parameter	: 	in_roi		    -- ROI句柄,相机位置标记的区域
	@Parameter	: 	in_cam_index	-- 相机索引号
	@Returns	:   AlgoError
	@Author		：	mumeng@2025/12/29
	************************************/
	C_API AlgoError C_CALL InkColorSetCamPosMarkCorners(AlgoHandle inkcolor_handle, AlgoHandle in_roi, const int in_cam_index);




	/************************************
	@Brief		：	计算标定色块的Mask, 手动区域定位，
	@Note		：
	@Parameter	: 	inkcolor_handle -- 有效的墨色特征句柄
	@Parameter	: 	in_roi		    -- ROI句柄
	@Parameter	: 	in_cam_index	-- 相机索引号，从1开始，后面相同
	@Parameter	: 	in_cs_idex	    -- 色块索引号，从1开始，后面相同
	@Returns	:   AlgoError
	@Author		：	mumeng@2021/11/30
	************************************/
	C_API AlgoError C_CALL InkColorSetColorSquareCorners(AlgoHandle inkcolor_handle, AlgoHandle in_roi, const int in_cam_index, const int in_cs_idex);


	/************************************
	@Brief		：	设置灯板位置标记的roi区域
	@Note		：
	@Parameter	: 	inkcolor_handle -- 有效的墨色特征句柄
	@Parameter	: 	in_roi		    -- ROI句柄，色卡的位置标记mark区域。
	@Parameter	: 	in_cam_index	-- 相机索引号，从1开始，后面相同
	@Parameter	: 	in_card_index   -- 色卡索引号，左边位1，右边位2， 从1开始
	@Returns	:   AlgoError
	@Author		：	mumeng@2025/12/29
	************************************/
	C_API AlgoError C_CALL InkColorSetColorSquareMarkCorners(AlgoHandle inkcolor_handle, AlgoHandle in_roi, 
		const int in_cam_index, const int in_card_index);



	/************************************
	@Brief		：	计算标定色块的模板图像
	@Note		：	
	@Parameter	: 	inkcolor_handle -- 有效的墨色特征句柄
	@Parameter	: 	in_img_amat		-- 输入相机当前拍摄的图像
	@Parameter	: 	out_mark_img	-- 输出用于定位的模板图像
	@Returns	:   AlgoError
	@Author		：	mumeng@2025/12/29
	************************************/
	C_API AlgoError C_CALL InkColorCalcColorSquareMarkImg(AlgoHandle inkcolor_handle, AlgoMat* in_img_amat, AlgoMat* out_mark_img);



	/************************************
	@Brief		：	计算标定色块的Mask, 手动区域定位，
	@Note		：	
	@Parameter	: 	inkcolor_handle -- 有效的墨色特征句柄
	@Parameter	: 	in_img_amat		-- 输入用于定位的模板图像
	@Parameter	: 	in_cam_indes	-- 输入相机索引编号， 从1开始
	@Parameter	: 	out_mark_img	-- 输出用于定位的模板图像
	@Returns	:   AlgoError
	@Author		：	mumeng@2025/12/29
	************************************/
	C_API AlgoError C_CALL InkColorCalcCamPosMarkImg(AlgoHandle inkcolor_handle, AlgoMat* in_img_amat, const int in_cam_indes, AlgoMat* out_mark_img);


	/************************************
	@Brief		：	设置图像颜色空间转换矩阵
	@Note		：
	@Parameter	: 	inkcolor_handle		-- 有效的墨色特征句柄
	@Parameter	: 	in_cam_index		-- 相机索引号
	@Parameter	: 	in_camrgb2xyz_mat   -- 相机RGB至CIEXYZ颜色转化矩阵
	@Returns	:   AlgoError
	@Author		：	mumeng@2021/12/08
	************************************/
	C_API AlgoError C_CALL InkColorSetTransMat(AlgoHandle inkcolor_handle, const int in_cam_index, const AlgoMat* in_camrgb2xyz_mat);

	/************************************
	@Brief		：	保存设置好的相机属性，包括相机编号、ROI、转换矩阵， 文件格式为 .cam
	@Note		：	
	@Parameter	: 	inkcolor_handle			-- 有效的墨色特征句柄
	@Parameter	: 	in_cam_index			-- 相机索引号
	@Parameter	: 	in_cam_filename			-- 文件保存路径
	@Returns	:   AlgoError
	@Author		：	mumeng@2022/5/12
	************************************/
	C_API AlgoError C_CALL InkColorSaveCamProperty(AlgoHandle inkcolor_handle, const int in_cam_index, const char* in_cam_filename);


	/************************************
	@Brief		：	读取相机属性文件 .cam
	@Note		：
	@Parameter	: 	inkcolor_handle			-- 有效的墨色特征句柄
	@Parameter	: 	in_cam_index			-- 相机索引号
	@Parameter	: 	in_cam_filename			-- 文件读取路径
	
	@Returns	:   AlgoError
	@Author		：	mumeng@2022/5/12
	************************************/
	C_API AlgoError C_CALL InkColorLoadCamProperty(AlgoHandle inkcolor_handle, const int in_cam_index, const char* in_cam_filename);


	/************************************
	@Brief		：	计算图像特征计算，有相机权重、墨色特征的颜色空间、设备标定方式
	@Note		：
	@Parameter	: 	inkcolor_handle			-- 有效的墨色特征句柄
	@Parameter	: 	in_weight				-- 各个相机权重
	@Parameter	: 	in_weight_len			-- 长度
	@Parameter	: 	color_sapce			    -- 指定的有颜色空间， 默认为LCH颜色空间
	@Parameter	: 	in_dev_calib_method		-- 设备偏差标定方法，默认为比例标定

	@Returns	:   AlgoError
	@Author		：	mumeng@2024/4/22
	************************************/
	C_API AlgoError C_CALL InkColorSetParameter(AlgoHandle inkcolor_handle, float* in_weight, int in_weight_len, ColorSpace in_color_sapce = CS_CIE_Lch, DevCalibrateMethod in_dev_calib_method = RATIO_CALIB);


	/************************************
	@Brief		：	计算PCB缝隙墨色信息，2023.11.1后。已经弃用
	@Note		：
	@Parameter	: 	inkcolor_handle			-- 有效的墨色特征句柄
	@Parameter	: 	in_img				    -- 相机拍摄的图像
	@Parameter	: 	in_calibrate_flag		-- 是否进行光源标定，ON_CALIBRARE 打开光源补偿；OFF_CALIBRARE 关闭光源补偿
	@Parameter	: 	in_cam_index			-- 相机索引号
	@Returns	:   AlgoError
	@Author		：	mumeng@2023/4/22
	************************************/
	C_API AlgoError C_CALL InkColorCalcPCBGapInfo(AlgoHandle inkcolor_handle, const AlgoMat* in_img, LightCalibrate in_calibrate_flag, const int in_cam_index);


	/************************************
	@Brief		：	计算灯板的camRGB、XYZ（补偿后的）、Lab、Lch 
	@Note		：	
	@Parameter	: 	inkcolor_handle			-- 有效的墨色特征句柄
	@Parameter	: 	in_img					-- 相机拍摄的图像
	@Parameter	:   in_calibrate_flag       -- 是否开启光源补偿flag，ON_CALIBRARE 打开光源补偿；OFF_CALIBRARE 关闭光源补偿
	@Parameter	: 	in_cam_index			-- 相机索引号，从1开始
	@Returns	:   AlgoError
	@Author		：	mumeng@2023/12/26
	************************************/
	C_API AlgoError C_CALL InkColorCalcImgInfo(AlgoHandle inkcolor_handle, const AlgoMat* in_img, LightCalibrate in_calibrate_flag, const int in_cam_index);


	/************************************
	@Brief		：	获取灯板的camRGB和XYZ，矩阵大小为 （相机数量行 * 6列），三列RGB，后三列XYZ， 矩阵大小为 （相机数量行 * 6列）
	@Note		：   
	@Parameter	: 	inkcolor_handle			-- 有效的墨色特征句柄
	@Parameter	: 	out_info			    -- 输出的墨色计算信息。
	@Returns	:   AlgoError
	@Author		：	mumeng@2022/12/26
	************************************/
	C_API AlgoError C_CALL InkColorGetImgInfo(AlgoHandle inkcolor_handle, AlgoMat* out_info);


	/************************************
	@Brief		：	保存初始状态标准色块颜色值
	@Parameter	: 	inkcolor_handle			-- 有效的墨色特征句柄
	@Parameter	: 	in_cam_filename			-- 文件保存路径
	@Returns	:   AlgoError
	@Author		：	mumeng@2022/6/1
	************************************/
	C_API AlgoError C_CALL InkColorSaveStdColor(AlgoHandle inkcolor_handle, const char* in_inkc_filename);
	

	/************************************
	@Brief		：  从本地读取初始状态标准色块颜色值
	@Parameter	: 	inkcolor_handle			-- 有效的墨色特征句柄
	@Parameter	: 	in_cam_filename			-- 文件保存路径
	@Returns	:   AlgoError
	@Author		：	mumeng@2022/6/1
	************************************/
	C_API AlgoError C_CALL InkColorLoadStdColor(AlgoHandle inkcolor_handle, const char* in_inkc_filename);


	/************************************
	@Brief		：  获取初始状态标定色块的颜色信息， 色块的XYZ信息，大小为（相机数量行， 18列） 18列包含6个色块的XYZ值 = 6*3。
	@Parameter	: 	inkcolor_handle			-- 有效的墨色特征句柄
	@Parameter	: 	out_CS_std_info			-- 输出的颜色信息矩阵
	@Returns	:   AlgoError
	@Author		：	mumeng@2022/6/1
	************************************/
	C_API AlgoError C_CALL InkColorGetStdInfo(AlgoHandle inkcolor_handle, AlgoMat* out_CS_std_info);


	/************************************
	@Brief		：	计算光源补偿标定色块的XYZ
	@Note		：
	@Parameter	: 	inkcolor_handle			-- 有效的墨色特征句柄
	@Parameter	: 	in_img					-- 相机拍摄的图像
	@Parameter	: 	in_cam_index			-- 相机索引号
	@Returns	:   AlgoError
	@Author		：	mumeng@2022/5/16
	************************************/
	C_API AlgoError C_CALL InkColorSetStdColorInfo(AlgoHandle inkcolor_handle, const AlgoMat* in_img, const int in_cam_index);


	/************************************
	@Brief		：	设置客户标准灯板数量
	@Note		：
	@Parameter	: 	inkcolor_handle			-- 有效的墨色特征句柄
	@Parameter	: 	in_std_panel_num		-- 标准灯板的数量，要求大于等于3个
	@Returns	:   AlgoError
	@Author		：	mumeng@2023/12/26
	************************************/
	C_API AlgoError C_CALL InkColorSetStdPanelNum(AlgoHandle inkcolor_handle, const int in_std_panel_num);


	/************************************
	@Brief		：	计算客户标准色板的标定矩阵，一次计算5个
	@Note		：
	@Parameter	: 	inkcolor_handle		-- 有效的墨色特征句柄
	@Parameter	: 	in_std_panel_xyz	-- 标定灯板的xyz值， 一个灯板有 5个相机*3（XYZ） = 15个值， 矩阵大小为（相机数量行，15列）
	@Returns	:   AlgoError
	@Author		：	mumeng@2023/12/26
	************************************/
	C_API AlgoError C_CALL InkColorCalcPanelCalib(AlgoHandle inkcolor_handle, AlgoMat* in_std_panel_xyz); // 计算标定矩阵


	/************************************
	@Brief		：	设备标定灯板发生偏差，进行数据修复。
	@Note		：
	@Parameter	: 	inkcolor_handle		-- 有效的墨色特征句柄
	@Parameter	: 	in_std_panel_xyz	-- 标定灯板的xyz值， 一个灯板有 5个相机*3（XYZ） = 15个值， 矩阵大小为（相机数量行，15列）
	@Returns	:   AlgoError
	@Author		：	mumeng@2025/4/10
	************************************/
	C_API AlgoError C_CALL InkColorCorrectPanelStd(AlgoHandle inkcolor_handle, AlgoMat* in_std_panel_xyz);// 计算标定矩阵


	/************************************
	@Brief		：	获取灯板标板的标定矩阵，一个相机一个
	@Note		：
	@Parameter	: 	inkcolor_handle			-- 有效的墨色特征句柄
	@Parameter	: 	out_panel_calib	        -- 输出的标定矩阵，得到数据后，软件需维护
	@Parameter	: 	in_cam_index	        -- 相机索引，从1开始
	@Returns	:   AlgoError
	@Author		：	mumeng@2023/12/26
	************************************/
	C_API AlgoError C_CALL InkColorGetPanelCalib(AlgoHandle inkcolor_handle, AlgoMat* out_panel_calib, const int in_cam_index);


	/************************************
	@Brief		：	设置客户标板的标定矩阵
	@Note		：
	@Parameter	: 	inkcolor_handle			-- 有效的墨色特征句柄
	@Parameter	: 	in_panel_calib	        -- 输出的标定矩阵，得到数据后，软件需维护
	@Parameter	: 	in_cam_index            -- 相机索引，从1开始
	@Returns	:   AlgoError
	@Author		：	mumeng@2023/12/25
	************************************/
	C_API AlgoError C_CALL InkColorSetPanelCalib(AlgoHandle inkcolor_handle, AlgoMat* in_panel_calib, const int in_cam_index); // 设置标定矩阵



	/************************************
	@Brief		：	设置客户标准色板的颜色值
	@Note		：
	@Parameter	: 	inkcolor_handle	     -- 有效的墨色特征句柄
	@Parameter	: 	in_std_panel_xyz	 -- 标定灯板的xyz值，  一个灯板有 5个相机*3（XYZ） = 15个值， 矩阵大小为（相机数量行，15列）
	@Returns	:   AlgoError
	@Author		：	mumeng@2023/12/26
	************************************/
	C_API AlgoError C_CALL InkColorSetStdPanelInfo(AlgoHandle inkcolor_handle, AlgoMat* in_std_panel_xyz);


	/************************************
	@Brief		：	保存初始的客户标准灯板颜色值至文件
	@Note		：
	@Parameter	: 	inkcolor_handle		-- 有效的墨色特征句柄
	@Parameter	: 	in_std_info_file	-- 输出的文件路径
	@Returns	:   AlgoError
	@Author		：	mumeng@2022/12/6
	************************************/
	C_API AlgoError C_CALL InkColorSaveStdPanelInfo(AlgoHandle inkcolor_handle, const char* in_std_info_file);
	
	/************************************
	@Brief		：	读取初始的客户标准灯板颜色值
	@Note		：
	@Parameter	: 	inkcolor_handle		-- 有效的墨色特征句柄
	@Parameter	: 	in_std_info_file	-- 存放颜色值文件路径
	@Returns	:   AlgoError
	@Author		：	mumeng@2022/12/6
	************************************/
	C_API AlgoError C_CALL InkColorLoadStdPanelInfo(AlgoHandle inkcolor_handle, const char* in_std_info_file);




	/************************************
	@Brief		：	设置相机位置标识模板
	@Note		：
	@Parameter	: 	inkcolor_handle		-- 有效的墨色特征句柄
	@Parameter	: 	in_cam_pos_mark_img	-- 色卡位置mark模板图像。
	@Parameter	: 	in_cam_idx			-- 相机索引。
	@Returns	:   AlgoError
	@Author		：	mumeng@2025/12/30   
	************************************/
	C_API AlgoError C_CALL InkColorSetCamPosModel(AlgoHandle inkcolor_handle, AlgoMat* in_cam_pos_mark_img, const int in_cam_idx);


	/************************************
	@Brief		：	设置色卡位置标识模板
	@Note		：
	@Parameter	: 	inkcolor_handle		-- 有效的墨色特征句柄
	@Parameter	: 	in_std_mark_img		-- 色卡位置mark模板图像。
	@Returns	:   AlgoError
	@Author		：	mumeng@2025/12/30   
	************************************/
	C_API AlgoError C_CALL InkColorSetStdColorModel(AlgoHandle inkcolor_handle, AlgoMat* in_std_mark_img);






	/************************************
	@Brief		：	获取墨色分类 InkFeatureClassify 的大小
	@Note		：
	@Parameter	: 	inkcolor_handle			-- 有效的墨色特征句柄
	@Parameter	: 	out_len					-- buffer的大小
	@Returns	:   AlgoError
	@Author		：	mumeng@2022/5/7
	************************************/
	C_API AlgoError C_CALL InkColorGetInkFeatureBufferSize(AlgoHandle inkcolor_handle, uint64_t* out_len);


	/************************************
    @brief		: 将InkFeatureClassify里面的数据导出到buffer
    @note		:
    @Parameter	: inkcolor_handle		-- 有效的墨色特征句柄
    @Parameter	: out_buffer			-- 已经申请好的空间
    @Parameter	: out_len				-- 外部申请好的空间大小,会用真实大小修改
    @return		: AlgoError
    @Author		: mumeng@2022/5/7
    ************************************/
	C_API AlgoError C_CALL InkColorInkFeatureToBuffer(AlgoHandle inkcolor_handle, char* out_buffer, uint64_t* out_len);


	/************************************
    @brief		: 将buffer里面的数据导入到InkFeatureClassify
    @note		:
    @Parameter	: inkcolor_handle		-- 有效的墨色特征句柄
    @Parameter	: in_buffer				-- 已经申请好申请空间
    @Parameter	: in_len				-- 外部申请好的空间大小,会用真实大小修改
    @return		: AlgoError
    @Author		: mumeng@2022/5/7
    ************************************/
	C_API AlgoError C_CALL InkColorInkFeatureFromBuffer(AlgoHandle inkcolor_handle, const char* in_buffer, uint64_t in_len);


	/************************************
	@Brief		：	计算灯板墨色特征
	@Note		：
	@Parameter	: 	inkcolor_handle			-- 有效的墨色特征句柄
	@Parameter	: 	out_inkfeature			-- 输出的墨色特征
	@Returns	:   AlgoError
	@Author		：	mumeng@2022/5/5
	************************************/
	C_API AlgoError C_CALL InkColorCalcFeature(AlgoHandle inkcolor_handle, InkColorFeature* out_inkfeature);



	/************************************
	@Brief		：	输出指定角度相机的墨色特征信息
	@Note		：
	@Parameter	: 	inkcolor_handle			-- 有效的墨色特征句柄
	@Parameter	: 	out_inkfeature			-- 输出指定角度相机的墨色特征
	@Parameter	: 	in_cam_index            -- 相机索引编号
	@Returns	:   AlgoError
	@Author		：	mumeng@2023/4/12
	************************************/
	C_API AlgoError C_CALL InkColorCalcCamFeature(AlgoHandle inkcolor_handle, InkColorFeature* out_cam_inkfeature, const int in_cam_index);


	/************************************
	@Brief		：	输出6个标定色块当前采集的XYZ值
	@Note		：
	@Parameter	: 	inkcolor_handle			-- 有效的墨色特征句柄
	@Parameter	: 	out_info			    -- 输出的色块颜色信息，3行6列，一列一个色块，每列分别为 X Y Z
	@Returns	:   AlgoError
	@Author		：	mumeng@2023/4/12
	************************************/
	C_API AlgoError C_CALL InkColorGetColorSquareInfo(AlgoHandle inkcolor_handle, AlgoMat* out_info);


	/************************************
	@Brief		：	生成指定特征值的分类基类List 和 分类编号List
	@Note		：	
	@Parameter	: 	min_val					-- 分类区间的最小值
	@Parameter	: 	max_val					-- 分类区间的最大值
	@Parameter	: 	class_num				-- 分类数量
	@Parameter	: 	in_mothod				-- 生成哪个量的List
											 (1--Brightness、 2--Color1、 3--Color2、 
											  4--PanelBlance（各向异性）、5. Color1*Color2)
	@Parameter	: 	out_feature_val_list	-- 输出的特征值分类List
	@Parameter	: 	out_label_list			-- 输出的分类编号List
	@Returns	:   AlgoError
	@Author		：	mumeng@2021/12/29
	************************************/
	C_API AlgoError C_CALL InkColorMakeClassifyList(float in_min_val, float in_max_val, int in_class_num,
		int in_mothod, float* out_feature_val_list, int* out_label_list);


	/************************************
	@Brief		：	设置基类List，分类编号List
	@Note		：	
	@Parameter	: 	class_num				-- 分类数量
	@Parameter	: 	out_label_list			-- 输出的分类编号List
	@Returns	:   AlgoError
	@Author		：	mumeng@2025/10/16
	************************************/
	C_API AlgoError C_CALL  InkColorMakeUnequarClassifyList(int in_class_num, int* out_label_list);



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
	@Author		：	mumeng@2021/12/30
	************************************/
	C_API AlgoError C_CALL InkColorClassify(const InkColorFeature* in_feature,int in_mothod, 
		const float* in_feature_val_list, int in_feature_val_list_len,
		int* in_label_list, int in_label_list_len, 
		int* out_label);


	/************************************
	@Brief		：	灯板拼屏
	@Note		：	in_panel_feature_list		-- 灯板特征信息
	@Parameter	: 	in_panel_cols				-- 目标屏幕灯板列数
	@Parameter	: 	in_panel_rows				-- 目标屏幕灯板行数
	@Parameter	: 	out_panel_pos				-- 输出拼屏位置结果
	@Returns	:   AlgoError
	@Author		：	mumeng@2021/12/15
	************************************/
	C_API AlgoError C_CALL InkColorSpliceScreen(InkColorFeature* in_panel_feature_list, int in_panel_cols, int in_panel_rows, AlgoPoint* out_panel_pos);


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
	C_API AlgoError C_CALL InkColorRelease(AlgoHandle inkcolor_handle);

	C_API void C_CALL InkColorDestroyHandle(AlgoHandle inkcolor_handle);


#ifdef __cplusplus
}
#endif // __cplusplus
#endif
