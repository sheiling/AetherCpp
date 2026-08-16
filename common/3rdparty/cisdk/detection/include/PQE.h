#ifndef ____PQE_H__
#define ____PQE_H__

#include "ci_declare.h"
#include "ci_define.h"
#include "ci_data.h"
#include "PQE_DATA.h"
#include "PQE_ERROR.h"
#include "meta_info_enum.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus



	C_API void C_CALL InitLinearity(S_Linearity &data);

	C_API void C_CALL FreeLinearity(S_Linearity &data);

	/***
	 * Function Name	: GetSreenBitWidth
	 * Description     	: 通过屏体的起灰亮度和最大亮度获取屏体位宽
	 *
	 * Parameters:
	 * min_lum:		IN,  屏体起灰亮度（光枪采集1灰阶亮度-0灰阶亮度）当1灰阶亮度小于光枪量程,则采集2灰阶亮度....(依次类推)
					进行 HDR True Black 认证时 min_lum需要大于 0.0005nit
	 * max_lum:		IN,  屏体最大亮度
	 * bit_width:   OUT, 屏体位宽
	 * Return:      ReturnValue
	 * Author    :  fengyang
	 * modify    :
	 */
	C_API ReturnValue GetSreenBitWidth(double min_lum, double max_lum, double &bit_width);

	/***
	* Function Name	: GetSreenLinearity
	* Description     	: 获取屏体线性度相关评价参数
	*
	* Parameters:
	* in_data:     IN,  灰阶亮色度信息
	* length:      IN,  in_data长度
	* part_num:    IN,  分段数                 [2,3,4,5,6]
	* gamma_table: IN,  gamma table 长度=length
	* part_weight: IN,  分段权重，长度= length + 2
	* out_data:   OUT,  线性相关参数(见S_Linearity注释)
	* return  :
	* Author  : fengyang
	*
	*/
	C_API int GetSreenLinearity(S_LXY *in_data, int length, int part_num, int* gamma_table, float* part_weight, S_Linearity &out_data);

	/***
	 * Function Name	: GetGammaByLogLog
	 * Description     	: 计算gamma值
	 *
	 * Parameters:
	 * in_data:						IN,   测量亮度值
	 * len:							IN,   测量长度值
	 * bit_depth:					IN,   源色深(8,10,12bit...)
	 * graylut:						IN,   采集表
	 * out_gamma:					OUT,  每个灰阶的gamma值
	 * return  :					gamma值
	 * Author  : fengyang
	 */
	C_API double GetGammaByLogLog(double *in_data, int len, int bit_depth, int* graylut, double *out_gamma);

	/***
	 * Function Name		: GetLumCurve
	 * Description     		: 获取理论亮度曲线以及偏差值
	 *
	 * Parameters:
	 * in_data:						IN,   测量亮度值
	 * len:							IN,   测量长度值
	 * gamma_table:					IN,   Gamma表
	 * part_weight:					IN,   每个灰阶的权重表默认均为1
	 * out_lum:						OUT,  每个灰阶的理想亮度值
	 * out_diff:				    OUT,  每个灰阶与理想亮度偏差值
	 * maxdiff:				        OUT,  最大偏差值
	 * mindiff:				        OUT,  最小偏差值
	 * avgdiff:				        OUT,  平均偏差值
	 * return  :					0：success   -2：gamma、亮度最大值小于等于0需检查是否其正确性
	 * data    ：
	 * Author  : fengyang
	 * modify  : fengyang,2023-05-29,add ExtendedGamma for 18bit Gamma
	 *
	 */
	C_API int GetLumCurve(double *in_data, int len, int* gamma_table, float* part_weight, double *out_lum, double *out_diff, double &maxdiff, double &mindiff, double &avgdiff);
	
	/***
	 * Function Name	: GetSwithFrameTime
	 * Description     	: 获取切换帧率响应时间
	 *
	 * Parameters:
	 * in_data:     IN,  亮度计测量数据
	 * length:      IN,  in_data长度
	 * sample:      IN， 采样数量
	 * return  :         切换帧率响应时间
	 * Author  : fengyang
	 *
	 */
	C_API double GetSwithFrameTime(double *in_data, int len, int sample);

	/***
	 * Function Name	: GetSwithFrameCCTVarRatio
	 * Description     	: 获取切换帧率色温最大偏差值
	 *
	 * Parameters:
	 * in_data:     IN,  色温值
	 * len:         IN,  in_data长度
	 * std_cctP:    IN,  标准色温值
	 * rate  :      OUT, 切换帧率色温最大偏差值
	 * return  :    ReturnValue
	 * Author  : fengyang
	 *
	 */
	C_API ReturnValue GetSwithFrameCCTVarRatio(double *in_data, int len, int std_cct, double &rate);
	
	/***
	 * Function Name	: GetSwithFrameTime
	 * Description     	: 获取切换帧率亮度最大偏差值
	 *
	 * Parameters:
	 * in_data:     IN,  亮度值
	 * len:         IN,  in_data长度
	 * std_lum:     IN,  标准亮度
	 * rate         OUT, 切换帧率亮度最大偏差值
	 * return  :    ReturnValue
	 * Author  : fengyang
	 *
	 */
	C_API ReturnValue GetSwithFrameLumVarRatio(double *in_data, int len, double std_lum, double &rate);

	/***
	* Function Name      : GetGrayScaleColorTemperatureDeviation
	* Description        : 计算每个灰阶色温与目标色温的偏差值
	*
	* Parameters :
	* in_data :          IN,  输入的色温数据数组
	* len :              IN,  偏差值数组的长度
	* Std_CCT :          IN,  标准色温
	* deviations :       OUT, 每个色温与目标色温的偏差值数组
	* return :           计算成功返回 1，输入不合法返回 0
	* Author :           zhangguommeng
	*
	*/
	C_API int GetGrayScaleColorTemperatureDeviation(double *in_data, int len, int Std_CCT, double *deviations);

	/***
	* Function Name      : GetGrayScaleStats
	* Description        : 计算指定灰阶范围的最大、最小和平均值
	*
	* Parameters :
	* Gray_Scales :       IN,  灰阶数组
	* in_data :           IN,  待处理的数据数组
	* Start_Gray_Scale :   IN,  起始灰阶
	* End_Gray_Scale :     IN,  终止灰阶
	* Max_Value :         OUT, 指定灰阶范围内的最大值
	* Min_Value :         OUT, 指定灰阶范围内的最小值
	* Avg_Value :         OUT, 指定灰阶范围内的平均值（绝对值计算）
	* return :            计算成功返回 1，输入不合法返回 0
	* Author :            zhangguomemng
	*
	*/
	C_API int GetGrayScaleStats(int* Gray_Scales, double* in_data, int len, int Start_Gray_Scale, int End_Gray_Scale, double* Max_Value, double* Min_Value, double* Avg_Value);

	/***
	 * Function Name	: GetBrightUniformity
	 * Description     	: 获取屏体亮度均匀性，计算方式参考method说明
	 *
	 * Parameters:
	 * S_LXY :            IN,  屏体亮色度信息
	 * len:               IN， in_data长度
	 * out_data:          OUT，各点计算值
	 * method  :          IN，默认为0，即原有Dmark计算方式1-max|Li-Lm|/Lm
							  为1时，新增均匀性计算方案 min/max
	 * return  :          整体亮度均匀度,-1时表示输入参数有误
	 * Author  : fengyang
	 *
	 */
	C_API double GetBrightUniformity(S_LXY *in_data, int len, double *out_data, int method = 0);
	
	/***
	 * Function Name	: GetBrightUniformity
	 * Description     	: 获取屏体色度均匀性
	 *
	 * Parameters:
	 * in_data:          IN,  屏体亮色度信息
	 * len:              IN， in_data长度
	 * out_data  :       OUT，各点计算值
	 * unifVaule  :      OUT，整体色度均匀度
	 * Author  : fengyang
	 *
	 */
	C_API void GetColorUniformity(S_LXY *in_data, int len, double *out_data, float &unifVaule);
	
	/***
	* Function Name      : GetBrightnessDifference
	* Description        : 计算屏体的亮度差异
	*
	* Parameters :
	* in_data :           IN,  屏体亮度信息数组
	* len :               IN,  in_data 数组的长度
	* return :                 亮度差异值
	* Author  : zhangguomeng
	*
	*/
	C_API  double GetBrightnessDifference(S_LXY* in_data, int len);

	/***
	* Function Name      : GetWaveLengthDifference
	* Description        : 计算屏体的波长差异
	*
	* Parameters :
	* in_data :            IN,  屏体波长信息数组
	* len :                IN,  in_data 数组的长度
	* return :                  波长差异值
	* Author  : zhangguomeng
	*
	*/
	C_API double GetWaveLengthDifference(S_LXY* in_data, int len);

	/***
	* Function Name      : GetMuraDifference
	* Description        : 计算灯板 A 和灯板 B 亮度值的 Mura 偏差
	*
	* Parameters :
	* Ya :               IN,  灯板 A 的各灰阶亮度值
	* Yb :               IN,  灯板 B 的各灰阶亮度值
	* len :              IN,  Ya 和 Yb 数组的长度
	* out_data :         OUT, 各灰阶的Mura值
	* return  :               Mura 偏差值
	* Author  : zhangguomeng
	*
	*/
	C_API double GetMuraDifference(double* Ya, double* Yb, int len, double* out_data);

	/***
	* Function Name      : GetRGBWCouplingDeviation
	* Description            : 计算多个灯板的RGBW耦合偏差
	*
	* Parameters :
	* light_panels :           IN,  灯板三刺激值信息数组
	* len :                   IN, light_panels数组的长度,最少要求是2
	* X_Deviation :            OUT, X耦合偏差
	* Y_Deviation :            OUT, Y耦合偏差
	* Z_Deviation :            OUT, Z耦合偏差
	* return :                计算成功返回 true，输入不合法返回 false
	* Author  : zhangguomeng
	*
	*/
	C_API bool GetRGBWCouplingDeviation(LightPanelXYZ* light_panels, int len, double* X_deviation, double* Y_deviation, double* Z_deviation);

	/***
	 * Function Name      : GetViewingAngleDifference
	 * Description            : 计算多个灯板的可视角差异
	 *
	 * Parameters :
	 * light_panels :           IN,  灯板左、中、右视角三刺激值信息数组
	 * len :                   IN, light_panels数组的长度，最少要求是2
	 * left_deviation :         OUT, 左视角光衰差异
	 * right_deviation :        OUT, 右视角光衰差异
	 * left_right_Deviation :    OUT, 左右视角光衰差异
	 * return :                计算成功返回 true，输入不合法返回 false
	 * Author  : zhangguomeng
	 *
	 */
	C_API bool GetViewingAngleDifference(LightPanelViewingAngle* light_panels, int len, double* left_deviation, double* right_deviation, double* left_right_deviation);

	/***
	 * Function Name	: RGBToLxy
	 * Description     	: 通过RGB转Lxy
	 *
	 * Parameters:
	 * org_rgb_value:           IN,  理论RGB值
	 * white_lxy:               IN,  白色色坐标
	 * org_colorgamut:          IN,  屏体测量色域
	 * gamatable     :          IN,  屏体使用Gamma表
	 * len     :                IN,  Gamma表长度
	 * invValuePoint :          OUT，理论亮色度值
	 * Author  : fengyang
	 *
	 */
	C_API void  RGBToLxy(S_RGB_VALUE org_rgb_value, S_LXY white_lxy, S_COLOR_GAMUT org_colorgamut, int *gamatable, int len, S_LXY& invValuePoint);

	/***
	* Function Name      : GetContouring
	* Description        : 计算每个灰阶的二阶导结果
	*
	* Parameters:
	* gray_scales:        IN,  灰阶数组
	* Yn :                IN,  亮度值数组
	* len :               IN,  N 和 Yn 数组的长度,长度最少要求是3
	* Yb :                IN,  全黑亮度，SDR时输入全黑实测亮度值，HDR时默认设置为0
	* out_result:         OUT, 存储每个灰阶的二阶导结果的数组
	* return :            计算成功返回1，输入不合法返回 0
	* Author  :           zhangguomeng
	*/
	C_API int GetContouring(int* gray_scales, double* Yn, int len, double Yb, double* out_result);

	/***
	* Function Name      : GetTransferFunction
	* Description        : 计算SDR/HDR电光转换特性的相对亮度偏差的结果
	*
	* Parameters:
	* Yn :                IN,  实测亮度值数组
	* Lt :                IN,  理论亮度值数组
	* len :               IN,  Yn 和 Lt 数组的长度
	* Yb :                IN,  全黑亮度，SDR时输入全黑实测亮度值，HDR时默认设置为0
	* out_result :        OUT, 存储亮度相对偏差结果的数组
	* return :            计算成功返回 1，输入不合法返回 0
	* Author  :           zhangguomeng
	*/
	C_API int GetTransferFunction(double* Yn, double* Lt, int len, double Yb, double* out_result);

	/***
	* Function Name      :      GetWhiteLightColorUniformity
	* Description        :      计算白点亮色度均匀性
	*
	* Parameters:
	* in_data :                 IN,  屏体亮度信息
	* len :                     IN,  in_data数组的长度
	* in_center_x :             IN,  中心点的色度坐标 x
	* in_center_y :             IN,  中心点的色度坐标 y
	* out_lum_nonuniformity :   OUT,  亮度非均匀性结果
	* out_chroma_nonUniformity: OUT,  色度非均匀性结果
	* return :                  计算成功返回 1，输入不合法返回 0
	* Author  :                 zhangguomeng
	*/
	C_API int GetWhiteLightColorUniformity(S_LXY* in_data, int len, double in_center_x, double in_center_y, double* out_lum_nonuniformity, double* out_chroma_nonuniformity);

	/***
	* Function Name      : GetVignetting
	* Description        : 计算屏体的亮度非均匀性
	*
	* Parameters:
	* in_data :           IN,  屏体亮度信息
	* len :               IN,  in_data数组的长度，len 最少是 2
	* out_vignetting :    OUT,  亮度非均匀性结果
	* return :            计算成功返回 1，输入不合法返回 0
	* Author  :           zhangguomeng
	*/
	C_API int GetVignetting(S_LXY* in_data, int len, double* out_vignetting);

	/***
	* Function Name      :   GetOffAxisUniformity
	* Description        :   计算离轴均匀性
	*
	* Parameters:
	* in_data :              IN,  亮色度数组
	* len :                  IN,  in_data 数组的长度
	* in_center_Lxy :        IN,  同轴中心的亮色度信息
	* out_lum_deviation :    OUT,  亮度变化结果数组
	* out_chroma_deviation : OUT,  色度变化结果数组
	* return :               计算成功返回 1，输入不合法返回 0
	* Author  :              zhangguomeng
	*/
	C_API int GetOffAxisUniformity(S_LXY* in_data, int len, S_LXY in_center_Lxy, double* out_lum_deviation, double* out_chroma_deviation);

	/***
	 * Function Name		: LxyToRGB
	 * Description     	: 亮色度转化为RGB
	 *
	 * Parameters:
	 * curr:                    IN,  屏幕实际测量亮色度信息
	 * scr_bit:                 IN,  图像位宽，例如0-255scr_bit=8,0-1023scr_bit=10
	 * gamma:                   IN,  屏幕每个灰阶的gamma值
	 * white_lv:                IN,  白色亮度值
	 * out_r:                   OUT,  R值   范围[0-2^scr_bit-1]
	 * out_g:                   OUT,  G值   范围[0-2^scr_bit-1]
	 * out_b：                  OUT,  B值   范围[0-2^scr_bit-1]
	 * Author  : fengyang
	 * modify  : fengyang
	 *
	 */
	C_API void LxyToRGB(S_LXY curr, int scr_bit, int &out_r, int &out_g, int &out_b, double *gamma, double white_lv);

	/***
	 * Function Name		: GetGammaCurve
	 * Description     	    : 获取Gamma曲线
	 *
	 * Parameters:
	 * curr:                    IN,  Gamma表
	 * in_len:                  IN,  Gamma表长度
	 * out_std_gamma:           OUT, 理论Gamma曲线
	 * Author  : fengyangA
	 * modify  : fengyang
	 *
	 */
	C_API int GetGammaCurve(int* in_gamma_table, int in_len, double *out_std_gamma);
	
	/***
	 * Function Name	: GetDeltaE76
	 * Description     	: 计算理论与实际Lxy的色差值（deE76色差公式）
	 *
	 * Parameters:
	 * orgPoint:                  IN,  理论Lxy值
	 * targetPoint:               IN,  目标Lxy值
	 * whitePoint:                IN,  白点Lxy值
	 * return    :                OUT, DeltaE76色差值
	 * Author  : fengyang
	 *
	 */
	C_API double GetDeltaE76(S_LXY orgPoint, S_LXY targetPoint, S_LXY whitePoint);
	
	/***
	 * Function Name	: GetDeltaE2000
	 * Description     	: 计算理论与实际Lxy的色差值（deE2000色差公式）
	 *
	 * Parameters:
	 * orgPoint:                  IN,  理论Lxy值
	 * targetPoint:               IN,  目标Lxy值
	 * whitePoint:                IN,  白点Lxy值
	 * return    :                     DeltaE2000色差值
	 * Author  : fengyang
	 *
	 */
	C_API double GetDeltaE2000(S_LXY orgPoint, S_LXY targetPoint, S_LXY whitePoint);
	
	/***
	 * Function Name	: LxyToITP
	 * Description     	: Lxy转换ITP值
	 *
	 * Parameters:
	 * S_LXY:                     IN, 当前Lxy值
	 * return    :                    ITP色差值
	 * Author  : fengyang
	 *
	 */
	C_API S_ITP LxyToITP(S_LXY currLxy);

	/***
	 * Function Name	: GetDeITP
	 * Description     	: 获取DeltaITP值
	 *
	 * Parameters:
	 * tarITP:                     IN, 目标ITP值
	 * curITP:					   IN, 当前ITP值
	 * return    :                     ITP色差值
	 * Author  : fengyang
	 *
	 */
	C_API double GetDeITP(S_ITP tarITP, S_ITP curITP);

	/***
	 * Function Name	: GetCCT
	 * Description     	: 通过色坐标xy计算色温
	 *
	 * Parameters:
	 * x_coordinate:                IN, x色坐标
	 * y_coordinate:			    IN, y色坐标
	 * return    :                      色温值
	 * Author  : fengyang
	 *
	 */
	C_API int GetCCT(float x_coordinate, float y_coordinate);

	/***
	 * Function Name	: GetContrast
	 * Description     	: 获取对比度值
	 *
	 * Parameters:
	 * maxLum:                IN, 最大亮度值
	 * minLum:			      IN, 最小起灰亮度
	 * contrast:			  OUT,对比度
	 * return  :              ReturnValue
	 * Author  : fengyang
	 *
	 */
	C_API ReturnValue GetContrast(float maxLum, float minLum, int &contrast);

	/***
	 * Function Name	: GetAveragePower
	 * Description     	: 计算平均功率
	 *
	 * Parameters:
	 * power_data:           IN, 功率计采集数据
	 * len:			         IN, power_data 长度
	 * return    :             平均功率
	 * Author  : fengyang
	 *
	 */
	C_API float GetAveragePower(double* power_data, int len);

	/***
	 * Function Name	: GetEnergyEfficiency
	 * Description     	: 计算亮度有效率
	 *
	 * Parameters:
	 * average_power:           IN, 平均功率
	 * lum:			            IN, 画面亮度
	 * return    :                  亮度有效率
	 * Author  : fengyang
	 *
	 */
	C_API float GetEnergyEfficiency(double average_power, double lum);//能源效率功率

	/***
	 * Function Name	: GetGamutCoverge
	 * Description     	: 计算色域覆盖率（两色域公共面积相对于各自色域比例）
	 *
	 * Parameters:
	 * OrgColorCoverge:            OUT, 源色域面积占比(利用率)
	 * TargetColorCoverge:		   OUT, 目标色域面积占比(覆盖率)
	 * AreaRatio                   OUT, 源色域与色域面积比(匹配率)
	 * OrgColorgamut:			   IN,  原始色域
	 * TargetColorgamut:		   IN,  目标色域
	 * return    :
	 * Author  : fengyang
	 */
	C_API void GetGamutCoverge(double &OrgColorCoverge, double &TargetColorCoverge, double &AreaRatio, S_COLOR_GAMUT OrgColorgamut, S_COLOR_GAMUT TargetColorgamut);

	/***
	 * Function Name	: ChromaticityTolerance
	 * Description     	: 计算色容差
	 *
	 * Parameters:
	 * org_x:						IN,   测量x坐标
	 * org_y:						IN,   测量y坐标
	 * trg_x:						OUT,  目标x坐标
	 * trg_y:						OUT,  目标y坐标
	 * return    :
	 * Author  : fengyang
	 */
	C_API double ChromaticityTolerance(double org_x, double org_y, double trg_x, double trg_y);

	/***
	 * Function Name	: CalculatePQ
	 * Description     	: 计算PQ理论曲线
	 *
	 * Parameters:
	 * in_data:						IN,   测量亮度值
	 * screen_peak_lum:				IN,   屏体峰值亮度
	 * bit_depth:				    IN,   源色深(10,12bit...)
	 * len:							IN,   数据长度
	 * gray_lut:				    IN,   灰阶采集表
	 * out_pq_lum:				    OUT,  目标PQ亮度曲线
	 * out_diff:					OUT,  每个灰阶与理想亮度偏差值 当out_diff=-1表示其理论亮度==0
	 * maxdiff:				        OUT,  最大偏差值
	 * mindiff:				        OUT,  最小偏差值
	 * avgdiff:				        OUT,  平均偏差值
	 * return    :
	 * Author  : fengyang
	 */
	C_API void CalculatePQ(double *in_data, double screen_peak_lum, int bit_depth, int len, int* gray_lut, double *out_pq_lum, double *out_diff, double &maxdiff, double &mindiff, double &avgdiff);

	/***
	 * Function Name	: CalculateHLG
	 * Description     	: 计算HLG理论曲线
	 *
	 * Parameters:
	 * in_data:						IN,   测量亮度值
	 * screen_peak_lum:				IN,   屏体峰值亮度
	 * bit_depth:				    IN,   源色深(10,12bit...)
	 * len:							IN,   数据长度
	 * gray_lut:				    IN,   采样表
	 * out_pq_lum:				    OUT,  目标HLG亮度曲线
	 * out_diff:					OUT,  每个灰阶与理想亮度偏差值 当out_diff=-1表示其理论亮度==0
	 * maxdiff:				        OUT,  最大偏差值
	 * mindiff:				        OUT,  最小偏差值
	 * avgdiff:				        OUT,  平均偏差值
	 * gamma:				        IN，  情况1：不输入，gamma默认为0，表示根据最大亮度计算gamma
										  情况2：推荐输入1到1.6，则为固定gamma值，可支持输入0.5到2.4
	 * return  :
	 * Author  : fengyang
	 */
	C_API void CalculateHLG(double *in_data, double lum_peak, int bit_depth, int len, int* gray_lut, double *out_hlg_lum, double *out_diff, double &maxdiff, double &mindiff, double &avgdiff, double gamma = 0);

	/***
	 * Function Name	: CalCoupingFactor
	 * Description     	: 计算RGB != W的耦合系数
	 *
	 * Parameters:
	 * r_data:						IN,   测量R亮度值
	 * g_data:				        IN,   测量G亮度值
	 * b_data:				        IN,   测量B亮度值
	 * w_data:				        IN,   测量W亮度值
	 * out_r:				    	OUT,  R 偏差值
	 * out_g:				        OUT,  G 偏差值
	 * out_b:				        OUT,  B 偏差值
	 * out_w:				        OUT,  W 偏差值
	 * return  :
	 * Author  : fengyang
	 */
	C_API void CalCoupingFactor(S_LXY *r_data, S_LXY *g_data, S_LXY *b_data, S_LXY *w_data, int len, double *out_r, double *out_g, double *out_b, double *out_w);

	/***
	 * Function Name	: LuminanceLossPrediction
	 * Description     	: 根据屏体均匀性数据，预估可被校正好的RGBW亮度衰减及校正均匀后的亮度
	 *
	 * Parameters:
	 * S_LXY :						IN,  屏体亮色度信息
	 * len:							IN， in_data长度
	 * in_diff_ratio:               IN， 基于经验值的相机与光枪采集差异补偿，有默认值（0.1），可修改
	 * out_R_loss_value:           OUT，R亮度衰减比例(数值为0-1)
	 * out_G_loss_value:           OUT，G亮度衰减比例(数值为0-1）
	 * out_B_loss_value:           OUT，B亮度衰减比例(数值为0-1）
	 * out_W_loss_value:           OUT，W亮度衰减比例(数值为0-1）
	 * out_R_predict_lumin:         OUT，预估校正后R亮度
	 * out_G_predict_lumin:         OUT，预估校正后G亮度
	 * out_B_predict_lumin:         OUT，预估校正后B亮度
	 * out_W_predict_lumin:         OUT，预估校正后W亮度
	 * return    :                      0正常，1异常
	 * Author  : lichaoping
	 *
	 */
	C_API int LuminanceLossPrediction(S_LXY *in_data, int len, double in_diff_ratio, double *out_R_loss_value, double *out_G_loss_value, double *out_B_loss_value, double*out_W_loss_value, double *out_R_predict_lumin, double *out_G_predict_lumin, double *out_B_predict_lumin, double *out_W_predict_lumin);

	/***
	 * Function Name	: CalWhitePointCCT
	 * Description     	: 根据W的xy色坐标计算色温 n=(x-0.3320)/(y-0.1858), CCT=-437*n^3 + 3601*n^2 - 6831*n + 5517。
	 *
	 * Parameters:
	 * x_coordinate:                IN, x色坐标
	 * y_coordinate:			    IN, y色坐标
	 * y_coordinate:			    OUT,色温值
	 * return    :                      0正常，1异常
	 * Author  : lichaoping
	 *
	 */
	C_API int CalWhitePointCCT(float x_coordinate, float y_coordinate, int *out_CCT);

	/***
	 * Function Name	: CalPredictionLuminAndLossRatio
	 * Description     	: 根据输入的RGB亮度衰减比例得到W的预估亮度、W衰减比例、RGB的预估亮度
	 *
	 * Parameters:
	  * S_LXY :							IN,  屏体亮色度信息
	 * len:                             IN， in_data长度
	 * in_R_loss_ratio:                IN, R亮度衰减比例(数值为0-1)
	 * in_G_loss_ratio:			    IN, G亮度衰减比例(数值为0-1)
	 * in_B_loss_ratio:			    IN, B亮度衰减比例(数值为0-1)
	 * out_W_predic_lumin:			    OUT, W预估亮度
	 * out_W_loss_ratio:			    OUT, W亮度衰减比例(数值为0-1)
	 * out_R_predic_lumin:			    OUT, R预估亮度
	 * out_G_predic_lumin:			    OUT, G预估亮度
	 * out_B_predic_lumin:			    OUT, B预估亮度
	 * return    :                      0正常，1异常
	 * Author  : lichaoping
	 *
	 */
	C_API int CalPredictionLuminAndLossRatio(S_LXY *in_data, int len, float in_R_loss_ratio, float in_G_loss_ratio, float in_B_loss_ratio, double *out_W_predic_lumin, double *out_W_loss_ratio, double *out_R_predic_lumin, double *out_G_predic_lumin, double *out_B_predic_lumin);

	/***
	 * Function Name	: CalPredictionxy
	 * Description     	: 预估校正后RGBW的色坐标
	 *
	 * Parameters:
	 * S_LXY :					IN,  屏体亮色度信息
	 * len:						IN， in_data长度
	 * out_R_x:					OUT，预估的R_x
	 * out_R_y:					OUT，预估的R_y
	 * out_G_x:					OUT，预估的G_x
	 * out_G_y:					OUT，预估的G_y
	 * out_B_x:					OUT，预估的B_x
	 * out_B_y:					OUT，预估的B_y
	 * out_W_x:					OUT，预估的W_x
	 * out_W_y:					OUT，预估的W_y
	 * return  :				0正常，1异常
	 * Author  : lichaoping
	 *
	 */
	C_API int CalPredictionxy(S_LXY *in_data, int len, double *out_R_predic_x, double *out_R_y, double *out_G_x, double *out_G_y, double *out_B_x, double *out_B_y, double *out_W_x, double *out_W_y);

	/***
	 * Function Name	: CalKAndBalanceT
	 * Description     	: 计算热平衡模型的k值以及预估的热平衡温度
	 *
	 * Parameters:
	 * in_temperature_data_array :			IN,  多点温度数据
	 * in_data_num:							IN， in_temperature_data_array长度
	 * k_num:								IN， 内部计算k值的数量（目前参考excel工具默认为1）
	 * out_k:								OUT，预估温度模型的k值
	 * out_balance_T:						OUT，预估的热平衡温度
	 * return  :							0正常，1异常
	 * Author  : lichaoping
	 *
	 */
	C_API int CalKAndBalanceT(double *in_temperature_data_array, int in_data_num, int k_num, double *out_k, double *out_balance_T);

	/***
	 * Function Name	: CalThermalEffectLuminLoss
	 * Description     	: 预估热力效果的亮度衰减比例值
	 *
	 * Parameters:
	 * in_cold_lumin_T_data :				IN,  冷屏后RGB的亮度以及温度数据
	 * in_hot_lumin_T_data :				IN,  热屏后RGB的亮度以及温度数据
	 * in_T :								IN,  热平衡温度
	 * out_R_lumin_loss:					OUT，R热力效果的亮度衰减比例值(数值为0-1)
	 * out_G_lumin_loss:					OUT，G热力效果的亮度衰减比例值(数值为0-1)
	 * out_B_lumin_loss:					OUT，B热力效果的亮度衰减比例值(数值为0-1)
	 * return  :							0正常，1异常
	 * Author  : lichaoping
	 *
	 */
	C_API int CalThermalEffectLuminLoss(S_Lumin_T *in_cold_lumin_T_data, S_Lumin_T *in_hot_lumin_T_data, double in_T, double *out_R_lumin_loss, double *out_G_lumin_loss, double *out_B_lumin_loss);

	/***
	 * Function Name	: SpecifyPercentileCal
	 * Description     	: 计算数据中具体百分位的数值
	 *
	 * Parameters:
	 * in_data_array :				IN, 输入的数据数组
	 * in_date_num:					IN，输入数据的长度
	 * in_percentile :				IN,  百分位，取值范围0-1
	 * out1:						OUT，从小到大，百分位对应的数值
	 * out2:						OUT，从大到小，百分位对应的数值
	 * return  :					true正常，false异常
	 * Author  : lichaoping
	 *
	 */
	C_API bool SpecifyPercentileCal(double *in_data_array, int in_date_num, double *in_percentile, double *out1, double *out2);

	/***
	 * Function Name	: GetGammaSpecifyGrayLevel
	 * Description     	: 计算指定灰阶范围的gamma值
	 *
	 * Parameters:
	 * in_data:						IN,   测量亮度值（所有灰阶的亮度）
	 * len:							IN,   测量长度值
	 * bit_depth:					IN,   源色深(8,10,12bit...)
	 * graylut:						IN,   采集表
	 * start_gray:					IN,   开始灰阶
	 * end_gray:					IN,   截止灰阶
	 * out_average_gamma:			OUT,  灰阶范围内的平均gamma值
	 * out_gamma:					OUT,  每个灰阶的gamma值
	 * return  :					0：success   -1：数据为空   -2:最大亮度或者最大灰阶为0  -3:输入灰阶范围异常
	 * Author  : lichaoping
	 */
	C_API int  GetGammaSpecifyGrayLevel(double *in_data, int len, int bit_depth, int* graylut, int start_gray, int end_gray, double *out_average_gamma, double *out_gamma);

	/***
	 * Function Name		: GetLumCurveSpecifyGrayLevel
	 * Description     		: 获取指定灰阶范围理论亮度曲线以及偏差值
	 *
	 * Parameters:
	 * in_data:						IN,   测量亮度值（所有灰阶的亮度）
	 * len:							IN,   测量长度值
	 * gamma_table:					IN,   Gamma表
	 * part_weight:					IN,   每个灰阶的权重表默认均为1
	 * start_gray:					IN,   开始灰阶
	 * end_gray:					IN,   截止灰阶
	 * out_lum:						OUT,  每个灰阶的理想亮度值
	 * out_diff:				    OUT,  每个灰阶与理想亮度偏差值
	 * maxdiff:				        OUT,  最大偏差值
	 * mindiff:				        OUT,  最小偏差值
	 * avgdiff:				        OUT,  平均偏差值
	 * return  :					0：success   -1：输入数据为空 -2：最大gamma为0    -3:输入灰阶范围异常
	 * data    ：
	 * Author  : lichaoping
	 *
	 */
	C_API int GetLumCurveSpecifyGrayLevel(double *in_data, int len, int* gamma_table, float* part_weight, int start_gray, int end_gray, double *out_lum, double *out_diff, double &maxdiff, double &mindiff, double &avgdiff);

	/***
	 * Function Name		: CalTemperatureInThermalImager
	 * Description     		: 计算热成像图具体位置的温度
	 *
	 * Parameters:
	 * in_data:						IN,   热成像图数据
	 * rows:						IN,   热成像图数据的rows
	 * cols:						IN,   热成像图数据的cols
	 * out_average_T:				OUT,  多个计算区域的平均温度值数组指针
	 * in_center_x:					IN,   多个计算区域的中心点x坐标数组指针，列。默认为空，此时为1个点，且为中心点的x
	 * in_center_y:					IN,   多个计算区域的中心点y坐标数组指针，行。默认为空，此时为1个点，且为中心点的y
	 * in_point_num					IN,   计算区域的数量，默认值为1；
	 * in_rect_width:				IN,   计算区域的宽，默认值为20；
	 * return  :					0：success   -1：输入数据为空 -2：区域参数设置异常  -3：区域大小异常
	 * Author  : lichaoping
	 *
	 */
	C_API int CalTemperatureInThermalImager(double* in_data, int rows, int cols, double *out_average_T, int* in_center_x = NULL, int* in_center_y = NULL, int in_point_num = 1, int in_rect_width = 20);

	/***
	 * Function Name		: JudgeThermalBalance
	 * Description     		: 判断热平衡
	 *
	 * Parameters:
	 * in_data:						IN,   所有点一段时间测量的温度值（两分钟持续时间的温度）,单位为摄氏度
	 * len:							IN,   输入温度数据的长度，
	 * in_point_num:				IN,   每个时刻测量点的数量，假设t时刻，取了5个点的数据，则此时in_point_num为5
	 * in_data_step:				IN,   计算平均温度时用的数据的数量 （温度探头为10，热成像仪为1）
	 * out_balance_flag:			OUT,  1为达到热平衡，0为未达到热平衡
	 * out_balance_T:				OUT,  热平衡温度，当未达到热平衡时，值为0
	 * in_t_th:						IN,   判断热平衡的温度变化阈值，默认为0.5
	 * return  :					0：success   -1：输入数据为空
	 * Author  : lichaoping
	 *
	 */
	C_API int JudgeThermalBalance(double *in_data, int len, int in_point_num, int in_data_step, int *out_balance_flag, double *out_balance_T, double in_t_th = 0.5);

	/***
	 * Function Name		: GetTemperatureRising
	 * Description     		: 计算温升
	 *
	 * Parameters:
	 * in_data:						IN,   测量温度值（两分钟持续时间的温度）,单位为开尔文
	 * len:							IN,   测量数据长度
	 * out_rising_T_value:			OUT,  温升值
	 * return  :					0：success   -1：输入数据为空，或者长度小于等于1
	 * Author  : lichaoping
	 *
	 */
	C_API int GetTemperatureRising(double *in_data, int len, double* out_rising_T_value);

	/***
	 * Function Name		: GetTemperatureCharacteristicsLuminLoss
	 * Description     		: 计算温度特性中亮度衰减
	 *
	 * Parameters:
	 * in_data:						IN,   测量的点亮色度信息
	 * len:							IN,   测量长度值
	 * out_loss_ratio:				OUT,  亮度衰减比例，结果为百分比
	 * return  :					0：success   -1：输入数据为空 -2:设定的阈值为负数 -3:亮度初始值为0
	 * Author  : lichaoping
	 *
	 */
	C_API int GetTemperatureCharacteristicsLuminLoss(S_LXY *in_data, int len, double *out_loss_ratio);

	/***
	 * Function Name		: GetColorTemperatureDeviation
	 * Description     		: 计算色温偏差
	 *
	 * Parameters:
	 * in_data:						IN,   测量的色温值
	 * len:							IN,   测量长度值
	 * out_deviation_value:			OUT,  色温偏差绝对值
	 * return  :					0：success   -1：输入数据为空 -2:设定的阈值为负数
	 * Author  : lichaoping
	 *
	 */
	C_API int GetColorTemperatureDeviation(double *in_data, int len, double *out_deviation_value);

	/***
	 * Function Name		: GetColorCoordinatesDeviation
	 * Description     		: 计算温度特性中色坐标偏差
	 *
	 * Parameters:
	 * in_data:						IN,   测量的点亮色度信息
	 * len:							IN,   测量长度值
	 * out_x_deviation_value:		OUT,  x坐标偏差绝对值
	 * out_x_deviation_value:		OUT,  y坐标偏差绝对值
	 * return  :					0：success   -1：输入数据为空 -2:设定的阈值异常
	 * Author  : lichaoping
	 *
	 */
	C_API int GetColorCoordinatesDeviation(S_LXY *in_data, int len, double *out_x_deviation_value, double *out_y_deviation_value);

	/***
	 * Function Name	: GetBrightViewAngle
	 * Description     	: 计算亮度视角
	 *
	 * Parameters:
	 * mesuData:				IN,  测量亮度数据
	 * startAngle:				IN,  起始角度
	 * stepAngle:				IN,  间隔角度
	 * angle1:					OUT，左最大视角
	 * angle2:                  OUT, 右最大视角
	 * out_one_side_ratio:		OUT，其中一边亮度最大变化量
	 * out_other_side_ratio:	OUT，另外一边亮度最大变化量
	 * threshold:				IN,  亮度衰减阈值（>0&&<1 推荐0.5）
	 * return  :				0：success   -1：输入数据为空  -2：数据异常
	 * Author    :      lichaoping
	 * modify    :
	 */
	C_API int GetBrightViewAngle(S_LXY *mesuData, int len, int startAngle, int stepAngle, int &angle1, int &angle2, double &out_one_side_ratio, double &out_other_side_ratio, float threshold = 0.5);

	/***
	 * Function Name	: GetColorViewAngle
	 * Description     	: 计算色度视角
	 *
	 * Parameters:
	 * mesuData:				IN,  测量亮色度数据
	 * startAngle:				IN,  起始角度
	 * stepAngle:				IN,  间隔角度
	 * angle1,angele2:		    OUT，左、右最大视角
	 * out_one_side_ratio:		OUT，其中一边色度最大变化量
	 * out_other_side_ratio:	OUT，另外一边色度最大变化量
	 * threshold:				IN,  色度衰减阈值（推荐0.005）
	 * return  :				0：success   -1：输入数据为空  -2：数据异常
	 * Author    :      lichaoping
	 * modify    :  左右遍历，将左右角度相加
	 */
	C_API int GetColorViewAngle(S_LXY *mesuData, int len, int startAngle, int stepAngle, int &angle1, int &angle2, double &out_one_side_ratio, double &out_other_side_ratio, float threshold = 0.005);

	/***
	 * Function Name		: GetEnergyEfficiencyByArea
	 * Description     		: 计算能源效率
	 *
	 * Parameters:
	 * in_lumin:					IN,   测量的点亮度
	 * in_pitch_size:				IN,   点间距
	 * in_v_pixel_num:				IN,   垂直像素数
	 * in_h_pixel_num:				IN,   水平像素数
	 * in_power:					IN,   测量的功率值
	 * out_energy_efficiency:		OUT,  能源效率
	 * return  :					0：success   -1：输入参数异常
	 * Author  : lichaoping
	 *
	 */
	C_API int GetEnergyEfficiencyByArea(double in_lumin, double in_pitch_size, int in_v_pixel_num, int in_h_pixel_num, double in_power, double *out_energy_efficiency);

	/***
	 * Function Name		: GetPower
	 * Description     		: 计算功率
	 *
	 * Parameters:
	 * in_W:						IN,   功耗
	 * in_t:						IN,   时间
	 * out_power:					OUT,  out_power
	 * return  :					0：success   -1：输入参数异常
	 * Author  : lichaoping
	 *
	 */
	C_API int GetPower(double in_W, double in_t, double *out_power);

	/***
	 * Function Name		: CalWaveLength
	 * Description     		: 计算主波长
	 *
	 * Parameters:
	 * in_data:								IN,   - 测量的点Lv xy数据
	 * length:								IN,   - 测量数据的数量
	 * in_table1_array:						IN,   - 本地载入的主波长查找表数据
	 * in_table2_array:						IN,   - 本地载入的主波长查找表数据
	 * in_d65:								OUT,  - 白点坐标，定义即可
	 * out_wavelen:							OUT,  - 主波长结果
	 * table1_length:						IN,   - table1查找表长度
	 * table2_length:						IN,   - table2查找表长度
	 * return  :							0：success   -1：输入参数异常
	 * Author  : lichaoping
	 *
	 */
	C_API int CalWaveLength(S_LXY *in_data, int length, Wave_Table *in_table1_array, Wave_Table *in_table2_array, Std_Light in_d65, int *out_wavelen, int table1_length = 173, int table2_length = 152);

	/***
	 * Function Name		                : 检测跳灰和灰阶合并
	 * Description     		                : 检测输入的亮度信息是否存在跳灰及灰阶合并以及具体的灰阶
	 *
	 * Parameters:
	 * in_grayscale:					    IN,   - 要检测的gamma值
	 * in_lum:								IN,   - 测量得到屏体各个灰阶的亮度值
	 * in_length:						    IN,   - 输入数据个数
	 * out_result:						    OUT,  - 输出的检测结果：数据见上方定义结果包括了(是否存在跳灰，是否存在灰阶合并，跳灰的灰阶个数，灰阶合并段数，跳灰的具体灰阶，灰阶合并的具体灰阶段)
											注意：这里被判定为灰阶合并会有两种情况：
											1. 输入的Gamma值连续，并存在连续超过合并的最小灰阶长度个灰阶发生了合并
											2. 输入的Gamma值不连续，并相邻输入gamma值的亮度发生了合并，且起始的gamma值和结束的gamma值的差超过了被判定为合并的最小灰阶长度
	 * in_min_merge_len:                    IN,   - 会被判定为合并的最小灰阶长度(默认参数，默认值为8)
	 * return  :							0：success   -1：输入参数异常
	 * Author  : shimai
	 *
	 */
	 // todo:把灰阶合并数判定放在参数表里
	C_API int DetectGrayScaleInversionAndMerge(int* in_gamma, double* in_lum, int in_length, GrayScale_Lum_Order_Condition* out_result, int in_min_merge_len = 8);

	/***
	* Function Name		: GetMinLum
	* Description     	: 计算原始起灰亮度
	*
	* Parameters        :
	* in_l0_lum         : IN，0灰阶亮度
	* in_l1_lum         : IN，1灰阶亮度
	* in_l2_lum         : IN，2灰阶亮度
	* out_min_lum       : OUT，原始起灰亮度
	* return            : 0:success   -1:输入参数异常
	* Author            : ouxingtao
	*/
	C_API int GetMinLum(double in_l0_lum, double in_l1_lum, double in_l2_lum, double* out_min_lum);

	/***
	* Function Name		: GetMuraUniformity
	* Description     	: 计算Mura一致性，输出两块灯板的最大mura偏差
	*
	* Parameters        :
	* in_panel1_lum     : IN，灯板1不同灰阶的亮度
	* in_panel2_lum     : IN，灯板2不同灰阶的亮度
	* in_len            : IN，两块灯板采集的数据组数
	* out_mura_diff     : OUT，两块灯板最大mura偏差
	* return            : 0:success   -1:输入参数异常
	* Author            : ouxingtao
	*/
	C_API int GetMuraUniformity(double* in_panel1_lum, double* in_panel2_lum, int in_len, double* out_mura_diff);

	/***
	 * Function Name	: MeasuredRefreshRatePosition
	 * Description     	: 计算测量刷新率pattern亮起区域的位置，需要注意：
	 *					  1.以箱体左上角为起始点，第一行/列表示为1
	 *					  2.输出为箱体需要亮起的行/列,起始/结束的行/列灯点需要亮起
	 * Parameters:
	 * cabinet_size：         IN,箱体宽度、高度灯点数量
	 * panel_size：           IN,模组/灯板宽度、高度灯点数量
	 * scan_count:            IN,扫描数
	 * detect_scan_direction: IN,判断箱体扫描方式是行扫还是列扫，"true"是行扫,"false"是列扫
	 * illuminated_area:      OUT,测量刷新率亮起灯点的行列范围
	 * Return:      ReturnValue
	 * Author    :  zhaopengtao
	 * modify    :
	 *s
	 */

	C_API ReturnValue MeasuredRefreshRatePosition(const Pixel_Count &cabinet_size, const Pixel_Count &panel_size, const int scan_count, const bool detect_scan_direction, Illuminated_Area &illuminated_area);

	/***
	 * Function Name	   : GetRefresh
	 * Description     	   : 计算视觉刷新率
	 *					    1.支持刷新率采集设备Admesy Asteria、Admesy Promethesu LF
	 *					    2.支持TBS A1664芯片刷新率计算，适配RGB错峰、子场错峰策略
	 * Parameters:
	 * in_fps：				IN,帧率
	 * in_gray_data：       IN,高速光度计采集数据
	 * in_gray_data_len:    IN,in_gray_data长度
	 * in_sample_frequency: IN,高速光度计采样频率
	 * in_color：           IN,in_gray_data对应pattern颜色
	 * out_refresh：        Out,视觉刷新率
	 * Return:              AlgoError
	 * Author    :			zhaopengtao
	 * modify    :
	 *s
	 */
	C_API AlgoError GetRefresh(const int in_fps, const float *in_gray_data, const int in_gray_data_len, const RefreshSampleFrequency in_sample_frequency, const AlgoColor in_color, int &out_refresh);
#ifdef __cplusplus
}
#endif // __cplusplus
#endif