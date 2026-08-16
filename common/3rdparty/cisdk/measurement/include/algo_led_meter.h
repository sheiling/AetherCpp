#ifndef __ALGO_LED_METER_H__
#define __ALGO_LED_METER_H__


#include "ci_define.h"
#include "ci_declare.h"

#include "algo_led_meter_data.h"
#include "algo_led_meter_errors.h"
//#include "ci_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


	C_API AlgoError C_CALL LedMeterCreateHandle(AlgoHandle* out_meter_handle);

	/************************************
	@brief  : 设置LedMeterAdd方法，
	@note   : in_method = LEDADDMethod_ORI 表示 不同位置数据不需要归一到loc(0,0)位置的均值上 ，直接Add即可
			  in_method = LEDADDMethod_NORMALIZATION 表示 不同位置数据归一到loc(0,0)位置的均值上
	@param  : meter_handle          --有效的ledmeter句柄
	@param  : in_method             --add方法，分为LEDADDMethod_ORI和LEDADDMethod_NORMALIZATION，不设置时默认为LEDADDMethod_ORI

	@return	: AlgoError
	@Author : chengjiayi@2021/06/23
	************************************/
	C_API AlgoError C_CALL LedMeterSetAddMethod(AlgoHandle in_meter_handle, LedMeterAddMethod in_method);

	/************************************
	@brief  : 光通量数据内缩处理（针对水墨画效果优化）
	@note   : 对色度数据根据输入的in_per归一化到一个更小的区间
	@param  : meter_handle                --有效的ledmeter句柄
	@param  : in_out_xyz                  --输入：待处理颜色的光通量数据(生成色度系数的归一化后的数据，必须XYZ全部输入)，输出：处理后的数据
	@param  : in_meter_num                --meter数目，默认为3
	@param  : in_per                      --水墨画优化程度即内缩参数 （0<=in_per<=1，默认值0.92）
	@return	: AlgoError
	@Author : zhangxuan@2024/03/27
	************************************/
	C_API AlgoError C_CALL LedMeterShrink(AlgoHandle* meter_handles, AlgoHandle* in_out_xyz, const int in_meter_num = 3, float in_per = 0.92f);


	/************************************
	@brief  : 设置高斯滤波核参数
	@note   :
	@param  : meter_handle                --有效的ledmeter句柄
	@param  : in_filter_width             --滤波核宽，默认为51
	@param  : in_filter_height            --滤波核高，默认为51
	@param  : in_sigma_x,in_sigma_y       --滤波标准差,默认为3
	@return	: AlgoError
	@Author : zhangxuan@2023/07/18
	************************************/
	C_API AlgoError C_CALL LedMeterSetGauBlurPara(AlgoHandle meter_handle, const int in_filter_width, const int in_filter_height, const float in_sigma_x, const float in_sigma_y);

	/************************************
	@brief  : 对归一化后的XYZmeter中的单模组(灯板)进行高斯模糊处理
	@note   :
	@param  : meter_handle                --有效的ledmeter句柄
	@param  : light_W                     --灯板/模组的宽
	@param  : light_H                     --灯板/模组的高
	@return	: AlgoError
	@Author : zhangxuan@2023/07/18
	************************************/
	C_API AlgoError C_CALL LedMeterProcess(AlgoHandle meter_handle, const int light_W, const int light_H);


	/************************************
	@brief  : 将不同位置loc中的meter进行对应位置合并
	@note   :
	@param  : meter_handle          --有效的ledmeter句柄
	@param  : in_pattern            --位置信息
	@param  : in_single_meter          --单个屏meter
	@param  : in_led_add_size          --拼接屏分辨率
	@return	: AlgoError
	@Author : qianxiaotong@2023/06/19
	************************************/
	C_API AlgoError C_CALL LEDMeterAddArea(AlgoHandle meter_handle, const Pattern in_pattern, const AlgoHandle in_single_meter, const AlgoSize in_led_add_size);

	/************************************
	@brief  : 对meter中的chroma进行上采样
	@note   : 
	@param  : meter_handle          --有效的ledmeter句柄
	@param  : in_led_size              --需要resize的目标分辨率
	@return	: AlgoError
	@Author : qianxiaotong@2023/06/18
	************************************/
	C_API AlgoError C_CALL ResizeLEDMeter(AlgoHandle meter_handle, const AlgoSize in_led_size);

	/************************************
	@brief  : 对meter中的chroma进行gamma变换
	@note   :
	@param  : meter_handle          --有效的ledmeter句柄
	@param  : in_meter              --待做gamma变换的ledmeter
	@param  : in_gamma              --gamma值
	@return	: AlgoError
	@Author : qianxiaotong@2023/09/06
	************************************/
	C_API AlgoError C_CALL LEDMeterGammaTransForm(AlgoHandle meter_handle_out, const AlgoHandle in_meter, const float in_gamma);
	

	/************************************
	@brief  : 对meter中的chroma局部区域均值计算
	@note   :
	@param  : meter_handle          --有效的ledmeter句柄
	@param  : in_rect               --需要提取区域的Rect(数组长度4，分别代表（x,y,w,h），（x,y)区域左上角坐标，（w,h）区域宽高）
	@param  : out_mean              --输出区域均值
	@return	: AlgoError
	@Author : qianxiaotong@2023/08/31
	************************************/
	C_API AlgoError C_CALL CalLocalMean(AlgoHandle meter_handle, const int in_rect[], float *out_mean);
	
	/************************************
	@brief  : 对meter中的in_rect进行曝光的归一化
	@note   : 当in_rect为空时，对全局进行曝光归一化。 当曝光为0时，相当于对LedMeter求均值。
	@param  : meter_handle          --有效的ledmeter句柄
	@param  : in_rect               --需要提取区域的Rect
	@param  : in_exp_time           --当前meter采集时对应的曝光时间。 该曝光时间应换算为无增益下对应的曝光时间。
	@param  : out_meter_mean        --输出区域对曝光归一化后的均值
	@return	: AlgoError
	@Author : 田博@2024/06/21
	************************************/
	C_API AlgoError C_CALL CalLedMeterMean(AlgoHandle meter_handle, AlgoRect in_rect, int in_exp_time, float *out_meter_mean);

	/************************************
	@brief  : 全屏-分区差异效果处理
	@note   :
	@param  : out_meter_handle          --有效的ledmeter句柄
	@param  : in_meter1_handle          --有效的ledmeter句柄(相邻分区左or上区meter)
	@param  : in_meter2_handle          --有效的ledmeter句柄(相邻分区右or下区meter)
	@param  : in_hori_verti             --1代表横向拉齐，2代表纵向拉齐
	@param  : in_direction              --拉齐方向（1代表右对左or下对上，2代表左对右or上对下）
										  右对左即右边分区对齐到左边分区，out_meter_handle输出的为右边分区对齐后的Meter
	@return	: AlgoError
	@Author : qianxiaotong@2023/09/07
	************************************/
	C_API AlgoError C_CALL LEDMeterSubareaAlign(AlgoHandle out_meter_handle,const AlgoHandle in_meter1_handle, const AlgoHandle in_meter2_handle, const int in_hori_verti, const int in_direction);

	/************************************
	@brief  : 逐一添加数据,将不同隔点pattern组合完整
	@note   : point  和  lum 可以  至少有一个有效, 即不能同时为nullptr
	@param  : meter_handle          --有效的ledmeter句柄
	@param  : in_shot_handle        --有效的shot句柄,详见 algo_shot.h
	@param  : in_ledpoint_handle    --ledpoint句柄, nullptr=表示不添加    详见 led_point_locator.h或point_mapper.h
	@param  : in_lum_handle         --ledlum句柄,  nullptr=表示不添加     详见 led_lum_extractor.h
	@param  : out_num_remain        --还剩余多少个未添加,0表示已经添加完成;out_num_remain=nullptr,表示不检查剩余
	@param  : in_point_handle_type  --in_point_handle类型，默认值为ALGOMOD_LED_POINT_LOCATOR,当前支持ALGOMOD_LED_POINT_LOCATOR,ALGOMOD_POINT_MAPPER;
	@return	: AlgoError
	@Author : songshaojun@2024/11/13
	************************************/
	C_API AlgoError C_CALL LedMeterAdd(AlgoHandle meter_handle, AlgoHandle in_shot_handle,
		AlgoHandle in_ledpoint_handle, AlgoHandle in_lum_handle, int* out_num_remain = nullptr, int in_point_handle_type = ALGOMOD_LED_POINT_LOCATOR);


	/************************************
	@brief    : 逐一添加数据,将不同隔点pattern组合完整
	@note     : point  和  lum 可以  至少有一个有效, 即不能同时为nullptr
	@param    : meter_handle        --有效的ledmeter句柄
	@param    : in_part             --当前point/lum相关的分区信息
	@param    : in_pattern          --当前point/lum相关的打屏信息
	@param    : in_color            --当前point/lum相关的颜色信息
	@param    : in_type             --当前point/lum相关的cie类型
	@param  : in_ledpoint_handle    --ledpoint句柄, nullptr=表示不添加    详见 led_point_locator.h或point_mapper.h
	@param    : in_lum_handle       --ledlum句柄,  nullptr=表示不添加     详见 led_lum_extractor.h
	@param    : out_num_remain      --还剩余多少个未添加,0表示已经添加完成;out_num_remain=nullptr,表示不检查剩余
	@param  : in_point_handle_type  --in_point_handle类型，默认值为ALGOMOD_LED_POINT_LOCATOR,当前支持ALGOMOD_LED_POINT_LOCATOR,ALGOMOD_POINT_MAPPER;
	@return   : AlgoError
	@Modified : chengjiayi@2022/09/20
	************************************/
	C_API AlgoError C_CALL LedMeterAddNoShot(AlgoHandle meter_handle,
		const Partition* in_part, const Pattern* in_pattern, AlgoColor in_color, CIEType in_type,
		AlgoHandle in_ledpoint_handle, AlgoHandle in_lum_handle, int* out_num_remain = nullptr, int in_point_handle_type = ALGOMOD_LED_POINT_LOCATOR);

	/************************************
	@brief  : 利用blurlum构造ledmeter数据
	@param  : meter_handle          --有效的ledmeter句柄
	@param  : in_shot_handle        --有效的shot句柄,详见 algo_shot.h
	@param  : in_blur_lum_handle    --blurledlum句柄,  详见 blur_led_lum_extractor.h
	@param  : out_num_remain        --还剩余多少个未添加,0表示已经添加完成;out_num_remain=nullptr,表示不检查剩余
	@return	: AlgoError
	@Author : tianbo@2021/10/12
	************************************/
	C_API AlgoError C_CALL LedMeterAddForBlur(AlgoHandle meter_handle, AlgoHandle in_shot_handle,
		AlgoHandle in_blur_lum_handle, int* out_num_remain = nullptr);


	/************************************
	@brief  : 用于任意灯点排布的点定位算法，满足任意形状屏构造ledmeter数据
	@note   : in_ledpoint_handle和in_lum_handle可以有一个有效, 不能同时为nullptr
	@param  : meter_handle          --有效的ledmeter句柄
	@param    : in_part             --当前point/lum相关的分区信息
	@param    : in_pattern          --当前point/lum相关的打屏信息
	@param    : in_color            --当前point/lum相关的颜色信息
	@param    : in_type             --当前point/lum相关的cie类型
	@param  : in_ledpoint_handle    --ledpoint句柄, nullptr=表示不添加    详见 led_point_locator.h
	@param  : in_lum_handle         --ledlum句柄,  nullptr=表示不添加     详见 led_lum_extractor.h
	@param  : out_num_remain        --还剩余多少个未添加,0表示已经添加完成;out_num_remain=nullptr,表示不检查剩余
	@return	: AlgoError
	@Author : wangyueyao@2024/02/21
	************************************/
	C_API AlgoError C_CALL LedMeterAddForAnyArrangementNoShot(AlgoHandle meter_handle, const Partition* in_part, const Pattern* in_pattern,
		AlgoColor in_color, CIEType in_type,AlgoHandle in_ledpoint_handle, AlgoHandle in_lum_handle, int* out_num_remain = nullptr);


	/************************************
	@brief  : 用于任意灯点排布的点定位算法，满足任意形状屏构造ledmeter数据
	@note   : in_ledpoint_handle和in_lum_handle可以有一个有效, 不能同时为nullptr
	@param  : meter_handle          --有效的ledmeter句柄
	@param  : in_shot_handle        --有效的shot句柄,详见 algo_shot.h
	@param  : in_ledpoint_handle    --ledpoint句柄, nullptr=表示不添加    详见 led_point_locator.h
	@param  : in_lum_handle         --ledlum句柄,  nullptr=表示不添加     详见 led_lum_extractor.h
	@param  : out_num_remain        --还剩余多少个未添加,0表示已经添加完成;out_num_remain=nullptr,表示不检查剩余
	@return	: AlgoError
	@Author : wangyueyao@2024/02/21
	************************************/
	C_API AlgoError C_CALL LedMeterAddForAnyArrangement(AlgoHandle meter_handle, AlgoHandle in_shot_handle, AlgoHandle in_ledpoint_handle, AlgoHandle in_lum_handle, int* out_num_remain = nullptr);


	/************************************
	@brief    : 利用blurlum构造ledmeter数据
	@note     :
	@param    : meter_handle        --有效的ledmeter句柄
	@param    : in_part             --当前lum相关的分区信息
	@param    : in_color            --当前lum相关的颜色信息
	@param    : in_type             --当前lum相关的cie类型
	@param    : in_blur_lum_handle  --有效的blurlum句柄
	@param    : out_num_remain      --还剩余多少个未添加,0表示已经添加完成;out_num_remain=nullptr,表示不检查剩余
	@return   : AlgoError
	@Modified : chengjiayi@2022/09/20
	************************************/
	C_API AlgoError C_CALL LedMeterAddForBlurNoShot(AlgoHandle meter_handle, const Partition* in_part, AlgoColor in_color,
		CIEType in_type, AlgoHandle in_blur_lum_handle, int* out_num_remain = nullptr);


	/************************************
	@brief  : 将色度对齐到一个值上
	@note   : 当光枪采集位置或半径缺省时，计算chroma均值, 计算均值与目标的比,将所有数据根据比拉齐, 内部自动根据分量判断
			  当输入光枪采集位置和半径时，使用逐箱标定功能
	@param  : meter_handle  --有效的ledmeter句柄
	@param  : in_target     --目标值
	@param  : in_point      --光枪采集中心点
	@param  : in_radius     --光枪采集半斤
	@return	: AlgoError
	@Author : chengjiayi@2021/06/23
	************************************/
	C_API AlgoError C_CALL LedMeterAlignChroma(AlgoHandle meter_handle, const TriXYZ* in_target, const AlgoPoint* in_point = nullptr, const int* in_radius = nullptr);


	/************************************
	@brief  : 死灯处理
	@note   : 将死灯处的数据用全局的均值替代，该操作需要放置到曲面修正之后进行
	@param  : in_out_meter_handle  --待处理的meter_handle
	@return	: AlgoError
	@Author : tianbo@2023/06/07
	************************************/
	C_API AlgoError C_CALL LedMeterFixDeadPoint(AlgoHandle in_out_meter_handle);


	/************************************
	@brief  : 异常灯点处理-设置异常亮点和暗点阈值，0-1范围内，亮点默认0.25，暗点默认0.5
	@note   : 
	@param  : meter_handle  --有效的ledmeter句柄
	@param  : in_ratio_highpoint  --亮点阈值
	@param  : in_ratio_darkpoint  --暗点阈值
	@param  : in_near_size	   --邻域边长（矩形）,推荐5,其它尺寸效果不保证
	@return	: AlgoError
	@Author : zhangxuan@2023/09/07
	************************************/
	C_API AlgoError C_CALL  LedMeterSetAbnormalPointPara(AlgoHandle meter_handle, float in_ratio_highpoint, float in_ratio_darkpoint, int in_near_size=5);


	/************************************
	@brief  : 设置亮暗斑检测阈值
	@note   :
	@param  : simulator_handle                --有效的meter句柄
	@param  : in_ratio_spot_light                   输入--亮斑检测阈值，默认为0.05，
			  in_ratio_spot_dark                  输入--暗斑检测阈值，默认为0.05
			  in_spot_size                        输入，亮暗斑前景尺寸，整型，默认为5
	@return	: AlgoError
	@Author : yaof@2024/05/11
	************************************/
	C_API AlgoError C_CALL LedMeterSetAbnormalSpotPara(AlgoHandle meter_handle, const float in_ratio_spot_light, const float in_ratio_spot_dark, const int in_spot_size);


	/************************************
	@brief  : 异常灯点处理()
	@note   : 将光通量小于5*5邻域50%或大于25%的灯点用邻域均值替代（默认，阈值可设）
			  必须在 psf 边界修正之后进行
	@param  : in_out_meter_handle  --待处理的meter_handle
	@return	: AlgoError
	@Author : zhangxuan@2023/09/07
	************************************/
	C_API AlgoError C_CALL  LedMeterCheckAbnormalPoint(AlgoHandle in_out_meter_handle);



	/************************************
	@brief  : 获取句柄内容
	@note   :
	@param  : meter_handle      --有效的ledmeter句柄
	@param  : out_led_meter     --导出的结构体,必须已经初始化完毕
	@param  : in_is_deep_copy   --是否深拷贝, 默认浅拷贝  1=深拷贝;0=浅拷贝
	@return	: AlgoError
	@Author : chengjiayi@2021/06/23
	************************************/
	C_API AlgoError C_CALL LedMeterGet(AlgoHandle meter_handle, AlgoLedMeter* out_led_meter, int in_is_deep_copy = 0);


	/************************************
	 @brief  : 生成公共meter
	 @note   : 用num组光通亮数据生成公共meter
	 @param  : in_filename    输入num组文件地址
	 @param  : out_filename   保存地址
	 @param  : in_num
	 @return : AlgoError
	 @Author : ouxingtao@2021/12/07
	 ************************************/
	C_API AlgoError C_CALL LedMeterMean(const char* in_filename[], const int in_num, const char* out_fullname);


	/************************************
	 @brief  : 生成公共meter
	 @note   : 用num组光通亮数据生成公共meter
	 @param  : in_meter_handles		若干个ledmeter handle
	 @param  : in_num				in_meter_handles个数
	 @param  : out_meter_			handle	输出平均的ledmeter handle,不包含pos信息
	 @return : AlgoError
	 @Author : tianbo@2023/08/02
	 ************************************/
	C_API AlgoError C_CALL LedMeterAverage(AlgoHandle* in_meter_handles, const int in_num, AlgoHandle out_meter_handle);



	/************************************
	@brief  : ledmeter set 拍摄信息
	@note   :
	@param  : meter_handle --有效的ledmeter句柄
	@param  : in_capture   --拍摄信息
	@return : AlgoError
	@Author : chengjiayi@2022/08/01
	************************************/
	C_API AlgoError C_CALL LedMeterSetCapture(AlgoHandle meter_handle, const Capture* in_capture);



	/************************************
	@brief  : ledmeter set 拍摄信息
	@note   :
	@param  : meter_handle	--有效的ledmeter句柄
	@param  : in_type		--滤片信息
	@return : AlgoError
	@Author : GuoHao@2024/08/01
	************************************/
	C_API AlgoError C_CALL LedMeterSetCieType(AlgoHandle meter_handle, const CIEType in_type);




	/************************************
	@brief  : 设置分区信息
	@note   :
	@param  : meter_handle --有效的ledmeter句柄
	@param  : in_part      -- 分区信息
	@return : AlgoError
	@Author : chengjiayi@2022/08/01
	************************************/
	C_API AlgoError C_CALL LedMeterSetPartition(AlgoHandle meter_handle, const Partition* in_part);



	/************************************
	@brief  : set面板信息
	@note   :
	@param  : meter_handle  --有效的ledmeter句柄
	@param  : in_panel      --面板信息
	@return : AlgoError
	@Author : chengjiayi@2022/08/01
	************************************/
	C_API AlgoError C_CALL LedMeterSetLedPanel(AlgoHandle meter_handle, const LedPanel* in_panel);


	/************************************
	@brief  : 设置Layer信息
	@note   :
	@param  : meter_handle --有效的ledmeter句柄
	@param  : in_layer     --Layer信息
	@return : AlgoError
	@Author : chengjiayi@2022/08/01
	************************************/
	C_API AlgoError C_CALL LedMeterSetLayer(AlgoHandle meter_handle, const Layer* in_layer);

	/************************************
	@Brief		：	往operate_buffer中写入长度为in_info_len的业务操作记录数据
	@Note		：
	@Parameter	: 	meter_handle	--有效的ledmeter句柄
	@Parameter	: 	in_op_info		--输入业务操作记录数据
	@Parameter	: 	in_info_len		--输入业务操作记录数据的长度，不能超过LEDMETER_OPERATE_BUFFER_SIZE
	@Returns	:   AlgoError
	@Author		：	zhongao@2025/02/26
	************************************/
	C_API AlgoError C_CALL LedMeterSetOperateBuffer(AlgoHandle meter_handle, const char* in_op_info, size_t in_info_len);

	/************************************
	@Brief		：	从operate_buffer中读取全部的业务操作记录数据
	@Note		：  每次固定读取LEDMETER_OPERATE_BUFFER_SIZE大小的数据
	@Parameter	: 	meter_handle	--有效的ledmeter句柄
	@Parameter	: 	out_op_info		--业务操作记录数据读到外部内存段
	@Parameter	: 	in_info_len		--外部内存段长度，不能小于LEDMETER_OPERATE_BUFFER_SIZE，默认为LEDMETER_OPERATE_BUFFER_SIZE
	@Returns	:   AlgoError
	@Author		：	zhongao@2025/02/26
	************************************/
	C_API AlgoError C_CALL LedMeterGetOperateBuffer(AlgoHandle meter_handle, char* out_op_info, size_t in_info_len = LEDMETER_OPERATE_BUFFER_SIZE);

	/************************************
	@brief  : 设置色度信息
	@note   :
	@param  : meter_handle  --有效的ledmeter句柄
	@param  : in_color      --颜色分量, 只能是单分量
	@param  : in_type       --cie类型只能是单类型
	@param  : in_chroma     --色度矩阵,必须是ALGO_32FC1
	@return : AlgoError
	@Author : chengjiayi@2022/08/01
	************************************/
	C_API AlgoError C_CALL LedMeterSetChroma(AlgoHandle meter_handle, AlgoColor in_color, CIEType in_type, const AlgoMat* in_chroma);






	/************************************
	@brief  : 设置点定位信息
	@note   :
	@param  : meter_handle --有效的ledmeter句柄
	@param  : in_ledpoint  --点定位矩阵,必须是ALGO_32FC2
	@return : AlgoError
	@Author : chengjiayi@2022/08/01
	************************************/
	C_API AlgoError C_CALL LedMeterSetPos(AlgoHandle meter_handle, const AlgoMat* in_ledpoint);





	/************************************
	@brief  : 设置灯点状态
	@note   : 不调用次函数的话, 所有state为 LED_FLAG_LIVE
	@param  : meter_handle  --有效的ledmeter句柄
	@param  : in_ledstate   --灯点状态矩阵,必须是ALGO_8UC1
	@return : AlgoError
	@Author : chengjiayi@2022/08/01
	************************************/
	C_API AlgoError C_CALL LedMeterSetState(AlgoHandle meter_handle, const AlgoMat* in_ledstate);


	/************************************
	@brief  : 设置内容
	@note   :
	@param  : meter_handle  --有效的ledmeter句柄
	@param  : in_led_meter  --输入内容到句柄
	@return	: AlgoError
	@Author : chengjiayi@2021/06/29
	************************************/
	C_API AlgoError C_CALL LedMeterSet(AlgoHandle meter_handle, const AlgoLedMeter* in_led_meter);


	/************************************
	@brief  : 背光校正光通量子区域融合
	@note   :
	@param  : meter_handle  --有效的ledmeter句柄
	@param  : in_area_width/in_area_height  --输入的背光子区域宽高,例如3*2个灯由一组系数控制，则输入3,2
	@param  : fusion_mode                   --光通量融合的模式，0为求均值，1为求和，默认值为1：求和
	@return	: AlgoError
	@Author : 杜龙辉@2024/3/26
	************************************/
	C_API AlgoError C_CALL LedMeterPixelMerge(AlgoHandle meter_handle, const int in_area_width, const int in_area_height, const int fusion_mode = 1);



	/************************************
	@brief  : 输出meta信息
	@note   :
	@param  : meter_handle --有效的ledmeter句柄
	@return : AlgoError
	@Author : chengjiayi@2022/08/01
	************************************/
	C_API AlgoError C_CALL LedMeterInfo(AlgoHandle meter_handle);


	/************************************
	@brief  : 导出chroma为图像
	@note   :
	@param  : meter_handle      --有效的ledmeter句柄
	@param  : in_img_fullname   --图像文件名
	@param  : in_para           --图像放大系数
	@param  : in_mark_list      --标记点的坐标列表
	@param  : in_mark_num       --标记点的个数
	@param  : in_is_color       --是否为彩色图,当没有Color时候,输出是三通道内容相同的图像 1=彩色图;0=灰度图
	@return : AlgoError
	@Author : lipengpeng@2021/07/16
	************************************/
	C_API AlgoError C_CALL  LedMeterExportChromaImage(AlgoHandle meter_handle, const char* in_img_fullname, int in_is_color = 1, int in_para = 1,
		AlgoPoint* in_mark_list = nullptr, int in_mark_num = 0);

	/************************************
	@brief  : 保存载入
	@note   : 后缀只能是.meter
	@param  : meter_handle  --有效的ledmeter句柄
	@param  : in_fullname   --后缀只能是.meter
	@return	: AlgoError
	@Author : chengjiayi@2021/06/23
	************************************/
	C_API AlgoError C_CALL LedMeterLoad(AlgoHandle meter_handle, const char* in_fullname);
	C_API AlgoError C_CALL LedMeterSave(AlgoHandle meter_handle, const char* in_fullname);

	/************************************
	@brief  : 数据释放
	@note   :
	@param  : meter_handle--有效的ledmeter句柄
	@param  : in_type     --0=释放所有数据;1=释放pos数据;2=释放chroma数据;3=释放metainfo
	@return	: AlgoError
	@Author : chengjiayi@2021/06/23
	************************************/
	C_API AlgoError C_CALL LedMeterRelease(AlgoHandle meter_handle, int in_type = 0);

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
	C_API void C_CALL LedMeterDestroyHandle(AlgoHandle meter_handle);


	/************************************
	@brief  : 获取buffer大小
	@note   :
	@param  : meter_handle   当前meter句柄
	@param  : out_len        buffer大小
	@return	: AlgoError
	@Author : ouxingtao@2021/07/07
	************************************/
	C_API AlgoError C_CALL LedMeterGetBufferSize(AlgoHandle meter_handle, uint64_t* out_len);


	/************************************
	@brief  : 将meter里面的数据导出到buffer
	@note   :
	@param  : meter_handle   有效的meter句柄
	@param  : out_buffer     已经申请好申请空间
	@param  : in_out_len     外部申请好的空间大小,会用真实大小修改
	@return	: AlgoError
	@Author : ouxingtao@2021/07/01
	************************************/
	C_API AlgoError C_CALL LedMeterToBuffer(AlgoHandle meter_handle, char* out_buffer, uint64_t* in_out_len);


	/************************************
	@brief  : 将buffer数据导入到meter
	@note   :
	@param  : meter_handle   有效的meter句柄
	@param  : in_buffer
	@param  : in_len
	@return	: AlgoError
	@Author : ouxingtao@2021/07/01
	************************************/
	C_API AlgoError C_CALL LedMeterFromBuffer(AlgoHandle meter_handle, const char* in_buffer, uint64_t in_len);


	/************************************
	@brief  : 将本ledmeter内容拷贝到另一个另一个ledmeter
	@note   :
	@param  : meter_handle 有效的meter句柄
	@param  : out_meter    有效的meter句柄
	@return	: AlgoError
	@Author : chengjiayi@2022/03/01
	************************************/
	C_API AlgoError C_CALL LedMeterCopyTo(AlgoHandle meter_handle, AlgoHandle out_meter);


	/************************************
	@brief  : 将另一个的色度数据根据mask融合到this中
	@note   :
	@param  : meter_handle     有效的meter句柄,this ledmeter
	@param  : in_meter_handle  有效的meter句柄,另一个ledmeter
	@param  : in_mask          mask,非0部分生效
	@return	: AlgoError
	@Author : chengjiayi@2022/03/01
	************************************/
	C_API AlgoError C_CALL LedMeterMergeChroma(AlgoHandle meter_handle, const AlgoHandle in_meter_handle, const AlgoMat* in_mask);


	/************************************
	@brief  : 将本ledmeter拆分成4个ledmeter，针对三角排列灯点排列形式
	@note   :
	@param  : meter_handle 有效的meter句柄
	@param  : out_1     必须先通过Createhandle创建, nullptr=不输出, 以[0,0]位置灯点为起始做行列均隔一个灯点拆分
	@param  : out_2     必须先通过Createhandle创建, nullptr=不输出, 以[0,1]位置灯点为起始做行列均隔一个灯点拆分
	@param  : out_3     必须先通过Createhandle创建, nullptr=不输出, 以[1,0]位置灯点为起始做行列均隔一个灯点拆分
	@param  : out_4     必须先通过Createhandle创建, nullptr=不输出, 以[1,1]位置灯点为起始做行列均隔一个灯点拆分
	@return	: AlgoError
	@Author : lipengpeng@2023/02/24
	************************************/
	C_API AlgoError C_CALL LedMeterBreakup(AlgoHandle meter_handle, AlgoHandle out_1, AlgoHandle out_2, AlgoHandle out_3, AlgoHandle out_4);

	/************************************
	@brief  : 对ledmeter数据按灯板进行拉齐（主要针对曲面前数据处理）
	@note   :
	@param  : meter_handle       有效的meter句柄
	@param  : block_num_row      灯板行数
	@param  : block_num_col      灯板列数
	@param  : refer_loc          归一化位置，0代表第一行、最后一行、第一列、最后一列
	@param  : refer_width        用来归一化的宽度
	@return	: AlgoError
	@Author : liying@2023/07/18
	************************************/
	C_API AlgoError C_CALL LedMeterBlockUnity(AlgoHandle meter_handle, const int block_num_row, const int block_num_col, const int refer_loc, const int refer_width);


	/************************************
	@brief  : 设置Meter合并信息
	@note   :
	@param  : inspector_handle  有效的meter句柄,不能为空
	@param  : in_merge_info     输入的合并信息，详情请见AlgoLedMeterMergeInfo结构体
	@return : AlgoError
	@Author : songshaojun@2024/02/03
	************************************/
	C_API AlgoError C_CALL LedMeterSetMergeInfo(AlgoHandle meter_handle, const AlgoLedMeterMergeInfo*in_merge_info);

	/************************************
	@brief  : 合并Meter结果
	@note   : 用于将多张Meter合并在一起，并将所有数据结果对齐至第一张Meter
	@param  : meter_handle             有效的meter句柄,不能为空
	@param  : in_all_meter_handle      输入值，所有meter的handle
	@param  : in_result_num            输入值，共有几张meter
	@param  : out_merged_meter_handle  输出值，合并后的meter
	@return : AlgoError
	@Author : songshaojun@2024/02/03
	************************************/
	C_API AlgoError C_CALL LedMeterGetMergeResult(AlgoHandle meter_handle, const AlgoHandle* in_all_meter_handle,
		const int in_meter_num, AlgoHandle out_merged_meter_handle);


	

	/************************************
	@brief  : 检测meter中的亮暗点或亮暗斑的位置, 默认参数下边缘2行2列不检测
	@note   : 配合LedMeterSetAbnormalPointPara 和 LedMeterSetAbnormalSpotPara 设置阈值使用
	  
	@param  : meter_handle             有效的meter句柄,不能为空
	@param  : in_type                  输入值，选择检测亮暗点还是亮暗斑，  1——亮暗点   2——亮暗斑  //枚举
	@param  : out_algoMat_Pos          输出值，大小为 1*N， 3通道int型，  N是亮暗点总数 
	                                           0通道是异常点x坐标， 1通道是异常点y坐标,  2通道是亮暗点类型（1 为亮点，2 为暗点，枚举型）                                          
	@param  : out_algoMat_Lum          输出值，大小为 1*N， 1通道float型，  N是亮暗点总数, 0通道是异常点亮度，与out_algoMat_Pos的点一一对应
	@return : AlgoError
	@Author : yaofei@2024/05/11
	************************************/
	C_API AlgoError C_CALL LedMeterFindAbnormalPos(AlgoHandle meter_handle, LedMeterAbnormalModType in_type, AlgoMat *out_algoMat_Pos, AlgoMat *out_algoMat_Lum);
	                      

	/***********************************************************
	@brief: meter数据拼接
	@note   : 将多个meter的数据，按照输入的拓扑规则拼接成一个大的meter，meter排列的顺序为从左到右，从上到下
	          拼接后pos位置不生效
	@param  : in_meter_handles		（in_show_rows *in_show_cols）个ledmeter handle
	@param  : in_show_rows,in_show_cols    拼接拓扑规则，如3*3， 3*4， 4*5等
	@param  :  out_meter_handle            输出，拼接后的meter
	@return:   AlgoError
	@Author		:  Yaof@2024.3.12
	***********************************************************/
	C_API AlgoError C_CALL  LedMeterJoint(AlgoHandle* meter_handles, const int in_show_rows, const int in_show_cols, AlgoHandle out_meter_handle);
	


	/***********************************************************
	@brief: meter数据转成图片
	@note   :
	@param  : in_meter_handle         有效的meter句柄,不能为空
	@param  : in_type                 输入，选择输出图片为热力彩色图还是灰度图，  1--热力图   2--灰度图  //枚举
	@param  : in_outImageRoute        输入，图像保存路径
	@return :   AlgoError
	@Author	:  Yaof@2024.3.12
	***********************************************************/
	C_API AlgoError C_CALL  LedMeterExportImage(AlgoHandle meter_handle, const char *in_outImageRoute, const ExportImageType in_type = ExportImageType_HotPic, const float abnormalpoint_ratio = 0.002f);

	/**********************************************
	@brief:  找出一批箱体数据的偏色灯板
	@return:  AlgoError：返回
	@param	:	* meter_X_handle： 输入in_Num个X通道meter数据
	@param	:	* meter_Y_handle： 输入in_Num个Y通道meter数据
	@param	:	* meter_Z_handle： 输入in_Num个Z通道meter数据，RZ可缺省，自行构造0.002f的数据
	@param	:	in_Num, 输入meter个数
	@param  :   in_module_rows,  in_module_cols, 箱体的灯板拓扑规则
	@param	:	*out_algoMat_pos:     输出，偏色灯板信息， 大小为 1*N， 2通道int型， N是偏色灯板总数， 0通道是灯板所在箱体ID（base为0），
										1通道是灯板所在箱体的位置序号，  灯板的位置序号是从左到右，从上到下依次递增，base为0
	@param	:	*out_algoMat_dist:   输出， 偏色灯板信息， 大小为 1*N， 2通道float型， N是偏色灯板总数，0通道是该灯板Lx偏色程度， 1通道是该灯板Ly偏色程度
	@param  :   *chroma_abnormal:     输入，偏色阈值数组，默认为0.002f，目前使用数组的第一个元素作为阈值
	@Author		:  Yaof@2024.3.12
	**********************************************/
	C_API AlgoError C_CALL LedMeterDetectAbnormalModulePos(AlgoHandle* meter_X_handle, AlgoHandle* meter_Y_handle, AlgoHandle* meter_Z_handle, \
		int in_Num, const int in_module_rows, const int in_module_cols, AlgoMat *out_algoMat_pos, AlgoMat *out_algoMat_dist, float *chroma_abnormal_threshold =nullptr, int in_threshold_Num =0);


	/**********************************************
	@brief:  仿真meter数据的色度
	@return:  AlgoError：返回
	@param	:	* meter_handle：输入in_Num 个meter的数据, 输入的通道顺序为X、Y、Z， RZ可缺省(自动构造一个0.002的RZ)
	@param	:	*in_outImageRoute:     仿真图保存路径

	@Author		:  Yaof@2024.3.12
	**********************************************/
	C_API AlgoError C_CALL LedMeterExportImageRGB(AlgoHandle *meter_handle, const int in_num, const char *in_outImageRoute); 


	/************************************
    @brief  : 相似性检测
    @note   : 多个meter进行相似性检测，返回相似的meter，以及个数

    @param  : in_meter_handles         输入：in_meter_num个有效的meter句柄
    @param  : in_meter_num			   输入：in_meter_handles个数
    @param  : in_method			       输入：进行相似性检测的模式，现支持两种，模式1是基于聚类的方式进行相似性检测，模式2是基于SSIM的相似性检测
    @param  : in_params				   输入：不同的模式有不同的参数设置：
								   模式1的参数设置：
									  params[0]表示相对阈值，params[1]表示多少倍下采样。 建议params[0] = 0.15f， params[1] = 2. 暂时用于检测显示异常灯过于异常的数据
									  params[0]=0.15f时，表示当返回的有效meter的最大差异不超过均值的15%（形态上）。
									  params[1]=2,表示内部在计算时对原始meter做多少倍的下采样。

								   模式2的参数设置：
									  params[0]表示图像进行高斯滤波时滤波核的大小，建议图像边长的20%，必须是奇数
									  params[1]表示图像进行高斯滤波时标准差的大小，建议滤波核大小的1/3
									  params[2]表示计算SSIM时相似算子的卷积核大小占图像边长的百分比，建议图像边长的35%

    @param  : in_param_num			     输入：params的个数
    @param  : out_valid_meter_handles	 输出：有效的handle
    @param  : out_valid_meter_num	     输出：有效handle的长度
    @return : AlgoError
    @Author : DuLonghui@2024/06/12
    ************************************/
	C_API AlgoError C_CALL LedMeterSimilarityDetect(AlgoHandle* in_meter_handles, const int in_meter_num, float* in_params, const int in_param_num,
		const int in_method, AlgoHandle* out_valid_meter_handles, int* out_valid_meter_num);


#ifdef __cplusplus
}
#endif // __cplusplus
#endif