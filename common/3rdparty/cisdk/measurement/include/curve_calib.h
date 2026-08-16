#ifndef __CURVE_CALIB_H__
#define __CURVE_CALIB_H__

#include "ci_define.h"
#include "ci_declare.h"
#include "curve_calib_data.h"
#include "curve_calib_error.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


    /************************************
    @brief  : 创建curvecalib句柄
    @note   : 
    @param  : out_curvecalib_handle   -- curvecalib句柄
    @return	: AlgoError
    @Author : chengjiayi@2021/06/24
    ************************************/
    C_API AlgoError C_CALL CurveCalibCreateHandle(AlgoHandle* out_curvecalib_handle);

    /************************************
    @brief  : 设置标定曲面边缘padding方式
    @note   : 默认复制1=PADDING_SAME
    @param  : curvecalib_handle     -- curvecalib句柄
    @param  : in_method             -- 1=PADDING_SAME 外边缘复制，值相同
                                    -- 2=PADDING_INTERPOLATE 外边缘插值，针对边缘偏亮有改善
    @return	: AlgoError
    @Author : ouxingtao@2023/01/06
    ************************************/
    C_API AlgoError C_CALL CurveCalibSetMethod(AlgoHandle curvecalib_handle, const int in_method);

    /************************************
    @brief  : 曲面标定
    @note   : 利用410测量屏幕上显示的圆的光枪值进行曲面标定
    @param  : curvecalib_handle     有效的curvecalib句柄
    @param  : in_circle_size        屏幕上显示的圆的行列数
    @param  : in_truth_list         410测量值
    @param  : in_circle_list        410测量值时每个圆的中心及半径, xy=圆心,wh=半径
    @param  : in_num                in_truth_list in_circle_list 个数
    @param  : in_meter1_handle      xyz中任意一个,不能全为空 有效的meter_handle句柄 详见algo_led_meter.h
    @param  : in_meter2_handle      xyz中任意一个,不能全为空 有效的meter_handle句柄 详见algo_led_meter.h
    @param  : in_meter3_handle      xyz中任意一个,不能全为空 有效的meter_handle句柄 详见algo_led_meter.h
    @return	: AlgoError
    @Author : chengjiayi@2021/06/25
    ************************************/
    C_API AlgoError C_CALL CurveCalibCompute(AlgoHandle curvecalib_handle, const AlgoSize* in_circle_size,
        const TriXYZ* in_truth_list, const AlgoRect* in_circle_list, int in_num,
        AlgoHandle in_meter1_handle, AlgoHandle in_meter2_handle =nullptr, AlgoHandle in_meter3_handle = nullptr);


	/************************************
	@brief  : 设置曲面系数的ROI区域
	@note   : 使用曲面系数的ROI区域进行修正；
			  业务场景示例：曲面是完整的标准箱体学到的公共曲面，而meter是标准箱体裁切得到的异形箱体；用曲面系数的一部分去做修正
	@param  : curvecalib_handle     有效的curvecalib句柄
	@param  : in_curve_ROI			要使用的曲面系数的roi
	@return	: AlgoError
	@Author : GuoHao@2024/07/25
	************************************/
	C_API AlgoError C_CALL CurveCalibSetRoi(AlgoHandle curvecalib_handle, AlgoRect in_curve_ROI);

   
    /************************************
    @brief  : 利用标定的曲面进行修正
    @note   : 
    @param  : curvecalib_handle     有效的curvecalib句柄
    @param  : in_meter_handle       有效的meter_handle句柄 详见algo_led_meter.h
    @return	: AlgoError
    @Author : chengjiayi@2021/06/25
    ************************************/
    C_API AlgoError C_CALL CurveCalibRectify(AlgoHandle curvecalib_handle, AlgoHandle in_meter_handle);


    /************************************
     @brief  : 读取保存标定曲面文件
     @note   : 后缀只能是.curvec
     @param  : curvecalib_handle        有效的curvecalib句柄
     @param  : in_curvecalib_fullname   标定文件,后缀只能是.curvec
     @return : AlgoError
     @Author : chengjiayi@2021/06/24
     ************************************/
    C_API AlgoError C_CALL CurveCalibSave(AlgoHandle curvecalib_handle, const char* in_curvecalib_fullname);
    C_API AlgoError C_CALL CurveCalibLoad(AlgoHandle curvecalib_handle, const char* in_curvecalib_fullname);


    /************************************
    @brief  : 生成公共曲面系数
    @note   : 用num组曲面系数生成公共曲面系数
    @param  : in_filename    输入num组文件地址
    @param  : out_filename   公共曲面系数保存地址
    @param  : num
    @return : AlgoError
    @Author : ouxingtao@2021/09/18
    ************************************/
    C_API AlgoError C_CALL CurveCoeffMean(const char* in_filename[], const int in_length, const char* out_filename[], const int out_length, const int num);




	/************************************
	@brief  : 生成公共曲面系数
	@note   : 用in_nums组曲面系数生成公共曲面系数
	@param  : in_curvec_handles			输入曲面handles
	@param  : in_nums					输入曲面个数
	@param  : out_curvec_average_handle	输出average_handle
	@return : AlgoError
	@Author : tianbo@2023/09/06
	************************************/
	C_API AlgoError C_CALL CurveCoeffAverage(AlgoHandle* in_curvec_handles, const int in_nums, AlgoHandle out_curvec_average_handle);


   /************************************
   @brief  : 获取句柄内容
   @note   : 
   @param  : curvecalib_handle    有效的curvecalib句柄
   @param  : out_para           导出的结构体,必须已经初始化完毕
   @param  : in_is_deep_copy    是否深拷贝, 默认浅拷贝  1=深拷贝;0=浅拷贝
   @return	: AlgoError
   @Author : chengjiayi@2021/06/24
   ************************************/
   C_API AlgoError C_CALL CurveCalibGet(AlgoHandle curvecalib_handle, AlgoCurveCalib* out_para, int in_is_deep_copy=0);



   /************************************
   @brief  : 设置内容
   @note   : 
   @param  : curvecalib_handle  有效的curvecalib句柄
   @param  : in_para            要输入的数据
   @return	: AlgoError
   @Author : chengjiayi@2021/07/02
   ************************************/
   C_API AlgoError C_CALL CurveCalibSet(AlgoHandle curvecalib_handle,const AlgoCurveCalib* in_para);

   /************************************
	@brief  : 曲面系数基于原图roi区域进行归一化
	@note   :
	@param  : curvecalib_handle       有效的curvecalib句柄
	@param  : in_meters_xyz_handle	 有效的meter句柄数组，数据要求如下：
										 1、meter数组不能为空或超过3个
										 2、meter数组中的meter需要与当前模块的颜色相匹配
										 3、meter数组中的meter之间的滤片信息不能重复
										 4、每一个meter要求pos与type信息完整，故暂时不支持超分的meter（因为没有pos信息）
	@param  : in_meters_xyz_num		 meter句柄数组长度
	@param  : in_cmfcalib_roi	     光谱标定时的原图roi区域
	@param  : in_shot_roi			 shot的Rect roi区域,默认为{0,0,0,0},shot不裁剪
    @return : AlgoError
    @Author : zhongao@2026/03/16
   ************************************/
   C_API AlgoError C_CALL CurveCoeffRoiNorm(AlgoHandle curvecalib_handle, AlgoHandle* in_meters_xyz_handle, int in_meters_xyz_num,
	   const AlgoRect in_cmfcalib_roi, const AlgoRect in_shot_roi = { 0,0,0,0 });


   /************************************
   @brief  : 将系数矩阵导出成图片
   @note   : 
   @param  : curvecalib_handle       有效的curvecalib句柄
   @param  : in_img_fullname
   @param  : in_type                导出类型 CIE_X CIE_Y CIE_Z
   @return	: AlgoError
   @Author : chengjiayi@2021/06/24
   ************************************/
   C_API AlgoError C_CALL CurveCalibExportImage(AlgoHandle curvecalib_handle, const char* in_img_fullname, CIEType in_type=CIE_Y);


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
    C_API AlgoError C_CALL CurveCalibRelease(AlgoHandle curvecalib_handle);
    C_API void C_CALL CurveCalibDestroyHandle(AlgoHandle curvecalib_handle);


    /************************************
    @brief  : 获取在屏幕上画圆圈的相关信息
    @note   :
    @param  : in_part_size      分区大小
    @param  : in_circle_size    圆圈行列
    @param  : in_circle_radius  圆圈的半径
    @param  : in_num            圆圈个数,in_num==in_circle_size.width*in_circle_size.height
    @param  : out_circles_list  必须初始化大小,个数必须是in_num
    @return	: AlgoError
    @Author : chengjiayi@2021/06/25
    ************************************/
    C_API AlgoError C_CALL CurveCalibGetMeasureCircle(const AlgoSize* in_part_size, const AlgoSize* in_circle_size,
        const  int in_circle_radius, int in_num, AlgoRect* out_circles_list);


    /************************************
    @brief  : 获取buffer大小
    @note   :
    @param  : curvecalib_handle   当前curvecalib句柄
    @param  : out_len             buffer大小
    @return	: AlgoError
    @Author : ouxingtao@2021/07/07
    ************************************/
    C_API AlgoError C_CALL CurveCalibGetBufferSize(AlgoHandle curvecalib_handle, uint64_t* out_len);


    /************************************
    @brief  : 将curvecalib里面的数据导出到buffer
    @note   :
    @param  : curvecalib_handle  有效的curvecalib句柄
    @param  : out_buffer         已经申请好申请空间
    @param  : in_out_len         外部申请好的空间大小,会用真实大小修改
    @return	: AlgoError
    @Author : ouxingtao@2021/07/01
    ************************************/
    C_API AlgoError C_CALL CurveCalibToBuffer(AlgoHandle curvecalib_handle, char* out_buffer, uint64_t* in_out_len);


    /************************************
    @brief  : 将buffer数据导入到curvecalib
    @note   :
    @param  : curvecalib_handle   有效的curvecalib句柄
    @param  : in_buffer
    @param  : in_len
    @return	: AlgoError
    @Author : ouxingtao@2021/07/01
    ************************************/
    C_API AlgoError C_CALL CurveCalibFromBuffer(AlgoHandle curvecalib_handle, const char* in_buffer, uint64_t in_len);


	/************************************
	@brief  : 计算合成W曲面
	@note   :
	@param  : white_curve_optimizer_handle		有效的white_curve_optimizer句柄
	@param  : in_rgb_curve_handle				输入的RGB曲面
	@param  : out_w_curve_handle				输出的W曲面
	@param  : in_white_opti				        输入的白色耦合系数
	@param  : in_rgb_meter_handle				输入的RGB的meter
	@param  : in_w_meter_handle					输入的W的meter
	@param  : in_num					        输入的RGB的曲面数量
	@param  : out_num					        输出的w的曲面数量
	@return	: AlgoError
	@Author : zhangtao@2023/05/08
	************************************/
	//C_API AlgoError C_CALL CurveCalibComputeCurve(AlgoHandle curvecalib_handle,
	//	AlgoHandle in_rgb_curve_handle[], AlgoHandle out_w_curve_handle[], AlgoHandle in_white_opti[],
	//	AlgoHandle in_rgb_meter_handle[], AlgoHandle in_w_meter_handle[], const int in_num, const int out_num);


	/************************************
	@brief  : 通过RGB曲面合成W曲面,
	@note   : 目前适用于全屏COG校正
	@param  : curvecalib_handle					有效的curvecalib_handle句柄
	@param  : in_rgb_curvec_handle				输入的RGB曲面，其中必须包含xyz的曲面（Rcurve不含Z）
	                                            ==》3.0.2版本支持利用RGBW_XYZ计算白色配比从而计算白色曲面系数,
												可支持一次输入RGB的某个滤片的曲面系数并计算对应白色曲面，注意长度为3，支持RZ为空但不支持RZ为nullptr
												实现该功能时,入参in_rgb_xyz_meter_handle，in_w_xyz_meter_handle，in_expo_time均设置为nullptr;
												in_rgb_xyz_meter_num,in_w_xyz_meter_num,in_time_num均为0；
												顺序为R G B
	@param  : in_rgb_curvec_num					in_rgb_curvec_handle的长度，长度为3.
	@param  : in_rgb_xyz_meter_handle			输入的RGB的meter，其中RZ不能缺省。输入顺序必须为:RX,RY,RZ,GX,GY,GZ,BX,BY,BZ
	@param  : in_rgb_xyz_meter_num				in_rgb_xyz_meter_handle的长度，长度为9
	@param  : in_w_xyz_meter_handle				输入的w_xyz的meter
	@param  : in_w_xyz_meter_num				输入的in_w_xyz_meter_handle的长度，长度为3
	@param  : in_expo_time					    输入的rgbw曝光时间，依次为RX,RY,RZ,GX,GY,GZ,BX,BY,BZ,WX,WY,WZ的曝光时间
	@param  : in_time_num					    in_expo_time的长度，长度为12
	@param  : in_rgbw_xyz					    对应RGBW的XYZ，用于计算耦合系数，在合成w curve时需要
	                                            ==》3.0.2版本支持使用该RGBW_XYZ来计算白色中RGB的配比，在合成w curve时需要
	@param  : out_w_curvec_handle				输出的w的曲面
	@return	: AlgoError
	@Author : laihaokun@2023/12/07
	************************************/
	C_API AlgoError C_CALL CurveCalibComputeWhiteCurveByRGBCurve(AlgoHandle curvecalib_handle,
		AlgoHandle* in_rgb_curvec_handle, int in_rgb_curvec_num, AlgoHandle* in_rgb_xyz_meter_handle, int in_rgb_xyz_meter_num,
		AlgoHandle* in_w_xyz_meter_handle, int in_w_xyz_meter_num, const int *in_expo_time, int in_time_num, RGBWXYZ in_rgbw_xyz, AlgoHandle out_w_curvec_handle);

	/************************************
	@brief  : 设置曲面拟合的方式
	@note   : 如不设置，则默认滤波曲面拟合
	@param  : curvecalib_handle     -- curvecalib_handle句柄
	@param  : in_fiting_method		-- 5=23阶曲面拟合, 6=block亮度拉齐＋23阶曲面拟合，7滤波曲面
	                                8 均值拉齐后，3阶迭代拟合(全屏曲面新方案)
		                            9 均值拉齐后，4阶迭代拟合(全屏曲面新方案，8和9中推荐9)
	@return	: AlgoError
	@Author : tianbo@2023/07/07
	************************************/
	C_API AlgoError C_CALL CurveCalibSetFitingMethod(AlgoHandle curvecalib_handle, const CurveCalibFitingMethod in_fiting_method);

	/************************************
	@brief  : 设置滤波曲面中中值滤波参数
	@note   : 滤波曲面使用，默认参数5，一般不修改
	@param  : curvecalib_handle       -- curvecalib_handle句柄
	@param  : in_medianfilter_para  -- 中值滤波参数
	@return	: AlgoError
	@Author : liying@2025/11/17
	************************************/
	C_API AlgoError C_CALL CurveCalibSetMedianFilterPara(AlgoHandle curvecalib_handle, const int in_medianfilter_para);

	/************************************
	@brief  : 设置滤波曲面参数
	@note   : 滤波曲面使用, 默认参数为31,31,10,10
	@param  : curvecalib_handle      -- curvecalib_handle句柄
	@param  : in_filter_width		 -- 高斯滤波器宽度
	@param  : in_filter_height		 -- 高斯滤波器高度
	@param  : in_sigma_x			 -- 水平方向标准差
	@param  : in_sigma_y			 -- 垂直方向标准差
	@return	: AlgoError
	@Author : tianbo@2023/07/07
	************************************/
	C_API AlgoError C_CALL CurveCalibSetFilterPara(AlgoHandle curvecalib_handle, const int in_filter_width, const int in_filter_height, const float in_sigma_x, const float in_sigma_y);

	/************************************
	@brief  : 设置均值归一后，输出曲面的默认动态范围，用于截断异常数据
	@note   : 默认参数为 0.5-20
			  对CurveCalibComputeFiting以下曲面方法生效，异形屏及其它曲面标定接口不生效
				POLYFIT_2_3,  
				POLYFIT_BLOCK 
				POLYFIT_FILTER
				POLYFIT_AVERAGEPULL_Adaptive_3
				POLYFIT_AVERAGEPULL_Adaptive_4
	@param  : curvecalib_handle				-- curvecalib_handle句柄
	@param  : in_min_curve_thersh			-- curvec数据中最大阈值
	@param  : in_max_curve_thersh			-- curvec数据中最大阈值
	@return	: AlgoError
	@Author : lvyang@2025/06/25
	************************************/
	C_API AlgoError C_CALL CurveCalibSetDefaultDynamicRange(AlgoHandle curvecalib_handle, const float in_min_curve_thersh,const float in_max_curve_thersh);


	/************************************
	@brief  : 设置灯板数量
	@note   : 拉齐拟合曲面使用	bloxk模式下使用的参数
	@param  : curvecalib_handle      -- curvecalib_handle句柄
	@param  : block_num_row        -- 灯板行数
	@param  : block_num_col        -- 灯板列数
	@return	: AlgoError
	@Author : ouxingtao@2021/12/07
	************************************/
	C_API AlgoError C_CALL CurveCalibSetBlockNum(AlgoHandle curvecalib_handle, const int block_num_row, const int block_num_col);


	/************************************
	@brief  : 设置灯板归一化位置及宽度
	@note   : 拉齐拟合曲面使用  block模式下使用的参数
	@param  : curvecalib_handle      -- curvecalib_handle句柄
	@param  : refer_loc            -- 归一化位置，0代表第一行、最后一行、第一列、最后一列
	@param  : refer_width          -- 用来归一化的宽度
	@return	: AlgoError
	@Author : ouxingtao@2021/12/07
	************************************/
	C_API AlgoError C_CALL CurveCalibSetReferLocWidth(AlgoHandle curvecalib_handle, const int refer_loc, const int refer_width);


	/************************************
	@brief  : 设置均值拉齐拟合的参数
	@note   : 均值拉齐拟合使用， 设置 POLYFIT_AVERAGEPULL_Adaptive_3 和POLYFIT_AVERAGEPULL_Adaptive_4 模式下的参数
	@param  : curvecalib_handle      -- curvecalib_handle句柄
	@param  : neighborhood_size    --均值拉齐时使用的邻域大小  默认为8，  可选4或者8
	@param  : microalign_threshold   --细拉齐时的阈值         默认0.017f， 可设置的范围为 0.001f - 0.1f
	@param  : filter_threshold       --筛选灯板时的阈值       默认1.06f,  可设置的范围为 0 - 2.0f
	          注：只改变一个参数时，其他参数请传入默认值
	@return	: AlgoError
	@Author : yaofei@2024/12/17
	************************************/
	C_API AlgoError C_CALL CurveCalibSetDefaultAveragePullAdaptiveFitingPara(AlgoHandle curvecalib_handle, const int in_neighborhood_size, const float in_microalign_threshold, const float in_filter_threshold);


	/************************************
	@brief  : 使用拟合、滤波曲面进行曲面标定
	@note   : 输入的meter是公共meter，同一个handle可以重复使用，后续使用会覆盖先前的内容
	@param  : curvecalib_handle					有效的curvecalib_handle句柄
	@param  : in_xyz_meter_handle				输入同一个颜色的不同滤片的meter数据
	@param  : in_meter_num					    meter的个数，不能大于3
	@return	: AlgoError
	@Author : tianbo@2023/07/07
	************************************/
	C_API AlgoError C_CALL CurveCalibComputeFiting(AlgoHandle curvecalib_handle, AlgoHandle* in_xyz_meter_handle, const int in_meter_num);


	/************************************
	@brief  : 使用滤波曲面,对异形屏体待校正区域进行曲面标定
	@note   : 
				对于state的要求：
					每行活灯与死灯的数目之和与待校正区域该行真实灯点数一致；抽点/裁切掉的点都作为不存在灯点体现在state中；
				PARTITION_SHAPE_SPHERICAL_TRAPEZIUM，球形屏的球身与球顶局部区域，近似于梯形;
				PARTITION_SHAPE_SPHERICAL_CIRCLE，球形屏整块的球顶;
				PARTITION_SHAPE_IRREGULAR_CUTTING，异形裁切屏体：暂不支持；
	@param  : curvecalib_handle					有效的curvecalib_handle句柄
	@param  : in_xyz_meter_handle				输入同一个颜色的不同滤片的meter数据
	@param  : in_meter_num					    meter的个数，不能大于3，必须有state
	@param  : in_part_shape						meter对应的待校正区域形状
	@return	: AlgoError
	@Author : GuoHao@2024/11/04
	************************************/
	C_API AlgoError C_CALL CurveCalibComputeFitingIrregular(AlgoHandle curvecalib_handle, AlgoHandle* in_xyz_meter_handle, const int in_meter_num, PartitionShape in_part_shape = PartitionShape::PARTITION_SHAPE_SPHERICAL_TRAPEZIUM);


	/************************************
	@brief  : 利用标定的曲面进行修正
	@note   :
	@param  : curvecalib_handle					有效的curvecalib句柄
	@param  : in_meter_handle					有效的meter_handle句柄
	@param  : in_part_shape						meter对应的待校正区域形状
	@return	: AlgoError
	@Author : GuoHao@2024/11/04
	************************************/
	C_API AlgoError C_CALL CurveCalibRectifyIrregular(AlgoHandle curvecalib_handle, AlgoHandle in_meter_handle, PartitionShape in_part_shape = PartitionShape::PARTITION_SHAPE_SPHERICAL_TRAPEZIUM);



	/************************************
	@brief  : 根据箱体最小采集限制计算模组需要采集行列数
	@note   :
	@param  : in_box_size				箱体尺寸有几行几列模组
	@param  : in_min_410_m_rc			一个箱体最少采几行几列
	@param  : out_modle_measure_num		一个模组采几个点
	@return	: AlgoError
	@Author : chengjiayi@2021/06/25
	************************************/
	C_API AlgoError C_CALL CurveCalibGetModuleCircleSize(
		const AlgoSize* in_box_size,
		const AlgoSize* in_box_circle_size_min,
		AlgoSize *out_modle_circle_size);

	/************************************
	@brief  : 获取箱体410采集点位
	@note   :
	@param  : in_modle_size			模组size
	@param  : in_box_size			箱体尺寸有几行几列模组
	@param  : in_modle_circle_size	模组采集行列数
	@param  : in_circle_radius		光枪半径
	@param  : num					箱体一共采集点数 = in_box_size.w *in_box_size.h * in_modle_circle_size.w *in_modle_circle_size.h
	@param  : out_circles_list		箱体410采集点位,长度等于num
	@return	: AlgoError
	@Author : YangSQ @2024/05/13
	************************************/
	C_API AlgoError C_CALL CurveCalibGetMeasureCircleBOX(const AlgoSize* in_module_size,
		const AlgoSize* in_box_size,
		const AlgoSize* in_modle_circle_size,
		const  int in_circle_radius,
		int num,
		AlgoRect* out_circles_list);


	/**********************************************
		@brief: 检测输入的多个箱体的meter数据曲面是否修好, 返回曲面异常区域边界
		AlgoError：返回值
		@param	:	* meter_handle： 输入in_Num 个箱体数据
		@param	:	in_Num:       --输入箱体个数, 需要大于in_NccNum_thresh（默认为6），否则报错
		@param  :   *out_algoMat_border  --输出，曲面Mura边界点集合， 为1*N的矩阵，3通道， 通道0是点的X轴坐标，通道1是点的Y轴坐标，通道2是boerder序号，N是边界点数量
		@param  :   *out_algoMat_BoxID  --输出，曲面未修好的箱体ID的List
		@param  :   in_NccNum_thresh                  输入--规律化Mura阈值，可缺省，默认为6
		@param  :    in_Nccthreshold                  输入--相似箱体相似度阈值，可缺省，默认为1.3
		@Author		:  Yaof@2024.3.12
		**********************************************/
	C_API AlgoError C_CALL CurveCalibCheckAbnormalArea(AlgoHandle* meter_handles, int in_Num, AlgoMat *out_algoMat_border, int *out_algoMat_BoxID, \
		int *out_Num, const int in_NccNum_thresh = 6, const float in_Nccthreshold =1.3f);


	/**********************************************
	@brief: 手拉曲面边界
	AlgoError：返回值
	@param	:	in_width, int in_height           输入，数据宽高
	@param	:	*in_PosOri   输入参数， 原始点坐标
	@param	:	*in_PosNew： 输入参数， 手拉后点坐标
	@param	:	 *in_out_algoMat_border： 输入&输出， 输入手拉前的边界点集合，输出手拉后的边界点集合
	@Author		:  Yaof@2024.3.12
	**********************************************/
	C_API AlgoError C_CALL CurveCalibPullAbnormalAreaByPoint(const int in_width, const int in_height, AlgoPoint *in_PosOri, AlgoPoint *in_PosNew, AlgoMat *in_out_algoMat_border);


	/*************************************************
	@brief: 根据曲面Mura重新生成曲面参数
	AlgoError：返回值
	@param  :   curvecalib_handle：  曲面句柄，不能为空
	@param	:	*meter_handles： 输入in_Num个箱体meter数据,  只能选一个通道数据，例如如果meter为X通道则只能修X通道曲面
	@param	:	in_Num:           输入，箱体个数
	@param  :   in_algoMat_BoxID:  输入，曲面异常箱体编号list ， 通过CurveCalibCheckAbnormalArea 获取
	@param	:	*in_algoMat_border  输入，曲面Mura的border， 通过CurveCalibPullAbnormalAreaByPoint或者CurveCalibCheckAbnormalArea 获取
	@Author		:  Yaof@2024.3.12
	**********************************************/
	C_API AlgoError C_CALL CurveCalibModify(AlgoHandle curvefeature_handle, AlgoHandle* meter_handles, const int in_Num, int *in_algoMat_BoxID, const int in_boxNum, AlgoMat *in_algoMat_border);



#ifdef __cplusplus
}
#endif // __cplusplus
#endif