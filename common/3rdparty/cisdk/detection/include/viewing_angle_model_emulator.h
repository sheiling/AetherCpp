#ifndef __VIEWING_ANGLE_MODEL_EMULATOR_H__
#define __VIEWING_ANGLE_MODEL_EMULATOR_H__

#include "algo_mat.h"
#include "ci_data.h"
#include "ci_declare.h"
#include "meta_info_data.h"
#include "viewing_angle_model_emulator_data.h"
#include "viewing_angle_model_emulator_error.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


    /************************************
    @brief  : 创建viewingAngle_handle句柄
    @note   :
    @param  : viewingAngle_handle     句柄
    @param  : in_methods
    @return	: AlgoError
    @Author : chengjiayi@2021/07/06
    ************************************/
    C_API AlgoError C_CALL ViewingAngleCreateHandle(AlgoHandle* viewingAngle_handle);

    /************************************
    @brief  : 读取保存配光曲线文件
    @note   : 后缀只能是.lightdistrib
    @param  : viewingAngle_handle         有效的句柄
    @param  : in_fullname                 配光曲线文件,后缀只能是.lightdistrib
    @return	: AlgoError
    @Author : ouxingtao@2021/10/11
    ************************************/
    C_API AlgoError C_CALL ViewingAngleSave(AlgoHandle viewingAngle_handle, const char* in_fullname);
    C_API AlgoError C_CALL ViewingAngleLoad(AlgoHandle viewingAngle_handle, const char* in_fullname);


    /************************************
    @brief  : 获取buffer大小
    @note   :
    @param  : viewingAngle_handle  当前有效句柄
    @param  : out_len     buffer大小
    @return	: AlgoError
    @Author : ouxingtao@2021/10/11
    ************************************/
    C_API AlgoError C_CALL ViewingAngleGetBufferSize(AlgoHandle viewingAngle_handle, uint64_t* out_len);


    /************************************
    @brief  : 将viewingAngle里面的数据导出到buffer
    @note   :
    @param  : viewingAngle_handle     有效的vig句柄
    @param  : out_buffer        已经申请好申请空间
    @param  : in_out_len        *in_out_len外部申请好的空间大小,会用真实大小修改
    @return	: AlgoError
    @Author : ouxingtao@2021/10/11
    ************************************/
    C_API AlgoError C_CALL ViewingAngleToBuffer(AlgoHandle viewingAngle_handle, char* out_buffer, uint64_t* in_out_len);


    /************************************
    @brief  : 将buffer数据导入到target_handle
    @note   :
    @param  : target_handle   有效的target_handle句柄
    @param  : in_buffer
    @param  : in_len
    @return	: AlgoError
    @Author : ouxingtao@2021/10/11
    ************************************/
    C_API AlgoError C_CALL ViewingAngleFromBuffer(AlgoHandle viewingAngle_handle, const char* in_buffer, uint64_t in_len);




	/***
	* Function Name		: ViewingAngleSetAngles
	* Description       : 输入采集数据的角度数组
	*
	* Parameters:
	@param  ： viewingAngle_handle                       --输入，viewing_angle_model_emulator模块句柄
	@param  : in_angle_array                             --输入，角度数组, 角度必须是-90 ~90°的合理角度
	@param  : in_angle_numbers                           --输入，角度数组长度,要和ViewingAngleModelCompute 接口的meter数量一致， 完全一一对应
	* return  :					0：success
	*                           未调用本接口的情况下，模块会用以下内容默认初始化：
	                            { -65, -63, -61, -59, -57, -55, -53, -51, -49, -47, -45, -43, -41, -39, -37, -35, -33, -31,
								  -29, -27, -25, -23, -21, -19, -17, -15, -13, -11, -9, -7, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 7, 9, 11, 13, 15, 17, 19, 
									21, 23, 25, 27, 29, 31, 33, 35, 37, 39, 41, 43, 45, 47, 49, 51, 53, 55, 57, 59, 61, 63, 65};
	*
	* Author  :  yaof 20250225
	*
	*/
	C_API AlgoError C_CALL ViewingAngleSetAngles(AlgoHandle viewingAngle_handle, float *in_angle_array, int in_angle_numbers);



	/*** 
 * Function Name		: ViewingAngleModelCompute
 * Description     		: 获取箱体有灯珠的配光曲线
 *
 * Parameters:
 @param: viewingAngle_handle		--输入，viewing_angle_model_emulator模块句柄
 @param: meter_handles：		   --输入的各角度的meter， 要求为同一颜色和滤片, meter的点间距信息需正确
 @param :angle_table			 --输入，各meter拍摄时的角度数组， 各角度与输入的meter一一对应
 @param  : len					 -- 输入meter的个数，输入角度数量，都是此值
 @param :in_direction_flag		 --输入，是水平方向，还是垂直方向， 水平方向是1，垂直方向为0
 @param :  in_camera_distance	  --输入，相机拍摄距离，单位毫米， 默认4米， 则输入4000.0f，  最多4位小数
 * return  :					0：success   
 *
 * Author  :  yaof 20250225
 *
 */
	C_API AlgoError C_CALL ViewingAngleModelCompute(AlgoHandle viewingAngle_handle,
		AlgoHandle *in_meter_handles,  int in_len,  int in_direction_flag,  float in_camera_distance = 4000);



	/***
 * Function Name		: ViewingAngleModelScore
 * Description     		: 计算各通道下所有灯珠的视角一致性评分和可视角度评分
 *
 * Parameters:
 @param  ： viewingAngle_handle                       --输入，viewing_angle_model_emulator模块句柄
 @param  : *out_scoure                                --输出，各灯珠视角特性一致性评分、可视觉度数组，
                                                               一致性评分取值范围0~1， 分数越低，一致性越好
															   可视角度取值范围为0~180 °
													  数组的长度为11， 数据内容依次 为：
													  总体评分，
													  X水平评分，X垂直评分，
													  Y水平评分，Y垂直评分，
													  Z水平评分，Z垂直评分，(当是红色时缺省)
													  
													  Y水平可视角，2个，左视角和右视角
													  Y垂直可视角，2个，上视角和下视角
													  
													  当某个通道的配光曲线数据缺失时，对应的输出置为-1，不参与总体评分计算。

 * return  :					0：success
 *
 * Author  :  yaof 20250225
 *
 */
	C_API AlgoError C_CALL ViewingAngleModelScore(AlgoHandle viewingAngle_handle, float* out_scores);




/***
* Function Name		:    ViewingAngleModelPrediction
* Description     		: 给定箱体的原始数据，预测指定距离和视角下观测的效果数据
*
* Parameters:
@param: viewingAngle_handle		         --输入，viewing_angle_model_emulator模块句柄
@param: inout_meter_handle：		      --输入输出，原始数据meter， 及观看视角变化后的meter
                                                      输入meter的点间距信息需正确
@param  : in_cemera_distance			 --输入，原始数据的拍摄距离，单位毫米
@param  :in_box_Xbias, in_box_Ybias		 --输入，箱体中心相对于全屏中心的横向和纵向偏移距离，单位毫米
@param :in_watch_pos,                    --输入, 观察者的距离和角度          

* return  :					0：success
* Author  :  yaof 20250225
*
*/
C_API AlgoError C_CALL ViewingAnglePrediction(AlgoHandle viewingAngle_handle,
		AlgoHandle in_out_meter_handle, float in_camera_distance, float in_box_xbias, float in_box_ybias, ViewPos in_watch_pos);



/***
* Function Name		:    ViewingAngleMultiplePrediction
* Description     		: 给定多个或一个箱体的原始数据， 预测指定距离和视角下观看全屏的效果数据
*
* Parameters:
@param: viewingAngle_handle		         --输入，viewing_angle_model_emulator模块句柄
@param :screen_size:                    --输入， 全屏的尺寸， 宽高必须是箱体宽高的整数倍
                                                 箱体的列数 = 全屏的宽度/箱体的宽度
												 箱体的行数 = 全屏的高度/箱体的高度
@param: in_meter_handles：		        --输入，多个箱体的原始数据meter句柄
                                                箱体在全屏中的顺序为：从左到右，从上到下
@param :in_Num                          --输入，in_meter_handles的数量
                                                如果in_Num 大于1， 则必须等于（箱体的列数*箱体的行数）;
												in_Num也可等于1，则所有箱体共用一个原始meter数据
										  输入meter的点间距信息需正确
@param  : in_camera_pos			        --输入，原始数据的拍摄位置， 包含距离和角度，单位毫米
@param : in_watch_pos,                    --输入, 观察者的距离和角度， 单位毫米，角度°
@param :out_meter_handle,               --输出， 大屏的仿真效果meter句柄 

* return  :					0：success
* Author  :  yaof 20250225
*/
C_API AlgoError C_CALL ViewingAngleMultiplePrediction(AlgoHandle viewingAngle_handle,
	 AlgoHandle *in_meter_handles, int in_Num, AlgoSize *screen_size, ViewPos in_camera_pos,  ViewPos in_watch_pos, AlgoHandle out_meter_handle);


/******************************
@brief  :  根据数据采集场景、观看场景，生成数据在观看场景下的形态分布
@note   :  采集场景：全屏、箱体、模组、采集距离、角度。观看场景：距离、角度
@param  :  viewingangle_handle          --有效的viewingangle句柄
@param  :  in_meter_handle              --有效的ledmeter句柄，输入的屏体数据
@param  :  in_camera_pos                --采集距离和角度，全屏或每个单元的采集信息
@param  :  in_capture_size                      --采集数据单元大小，全屏场景则和in_meter大小相同，箱体场景则输入箱体大小，模组则模组大小
@param  :  in_watch_pos                 --观看距离和角度，输入屏体数据的整体观看信息
@param  :  out_meter_handle             --在观看场景下的数据形态
@return :  AlgoError
@Author :
******************************/
C_API AlgoError C_CALL ViewingAngleUniformityPrediction(AlgoHandle viewingangle_handle, AlgoHandle in_meter_handle,
	ViewPos in_camera_pos, AlgoSize *in_capture_size, ViewPos in_watch_pos, AlgoHandle out_meter_handle);



/***
* Function Name		: ViewingAngleLoadLightdistrib
* Description       : 载入配光曲线文件
*
* Parameters:
@param  ： viewingAngle_handle                       --输入，viewing_angle_model_emulator模块句柄
@param  : *in_LightpatternCurve_file                 --输入，配光曲线文件名， 后缀为 .lightdistrib

* return  :					0：success
*
* Author  :  yaof 20250225
*
*/
C_API AlgoError C_CALL ViewingAngleLoad(AlgoHandle viewingAngle_handle, const char *in_lightpatternCurve_file);




/***
* Function Name		: ViewingAngleGet
* Description       : 获取配光曲线数据
*
* Parameters:
@param  ： viewingAngle_handle                       --输入，viewing_angle_model_emulator模块句柄
@param  : out_lightdistrib_data                         --输出， 配光曲线文件数据         

* return  :					0：success                   
*
* Author  :  yaof 20250225
*
*/
C_API AlgoError C_CALL ViewingAngleGet(AlgoHandle viewingAngle_handle,  LightDistrib *out_lightdistrib_data);





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
    C_API AlgoError C_CALL  ViewingAngleRelease(AlgoHandle target_handle);
    C_API void C_CALL ViewingAngleDestroyHandle(AlgoHandle target_handle);



	//2025.08.28 modify by songshaojun 

	/************************************
	@brief  : 创建句柄
	@note   :
	@param  : screen_viewingAngle_handle
	@return	: AlgoError
	@Author : songshoajun@2025.08.28
	************************************/
	C_API AlgoError C_CALL ScreenViewingAngleCreateHandle(AlgoHandle *screen_viewingAngle_handle);

	/************************************
	@brief  : 获取计算全屏视角特性曲线时用到的ROI区域
	@note   : 需要输入水平,垂直两个视角下的最大角度shot，根据4个shot自动计算得到ROI
	@param  : screen_viewingAngle_handle  ---输入：有效的屏幕视角特性handle
	@param  : in_shot_list                ---输入：水平或垂直方向上观测时，在最大视角状态下的shot样本（如，80度与-80度）
	@param  : in_shot_num                 ---输入：输入的shot样本数量
	@return	: AlgoError
	@Author : songshoajun@2025.08.28
	************************************/
	C_API AlgoError C_CALL ScreenViewingAngleGetScreenPatternROI(AlgoHandle screen_viewingAngle_handle, const AlgoHandle *in_shot_list, const int in_shot_num = 2);


	/************************************
	@brief  : 获取handle内的ROI区域
	@note   : 该roi区域用来提取屏幕视角特性
	@param  : screen_viewingAngle_handle  ---输入：有效的屏幕视角特性handle
	@param  : out_rect                    ---输出：返回的handle内部的roi
	@return	: AlgoError
	@Author : songshoajun@2025.08.28
	************************************/
	C_API AlgoError C_CALL ScreenViewingAngleGet(AlgoHandle screen_viewingAngle_handle, AlgoRect *out_rect);


	/************************************
	@brief  : 设置handle内的ROI区域
	@note   : 该roi区域用来提取屏幕视角特性
	@param  : screen_viewingAngle_handle  ---输入：有效的屏幕视角特性handle
	@param  : out_rect                    ---输出：返回的handle内部的roi
	@return	: AlgoError
	@Author : songshoajun@2025.08.28
	************************************/
	C_API AlgoError C_CALL ScreenViewingAngleSet(AlgoHandle screen_viewingAngle_handle, const AlgoRect *in_rect);

	/************************************
	@brief  : 计算指定视角下屏幕XYZ数据
	@note   :
	@param  : screen_viewingAngle_handle  ---输入：有效的屏幕视角特性handle
	@param  : in_x_shot					  ---输入：当前视角下的X数据
	@param  : in_y_shot				      ---输入：当前视角下的Y数据
	@param  : in_z_shot                   ---输入：当前视角下的Z数据
	@param  : in_x_shot_expoure_time      ---输入：拍摄X数据时的曝光时间，单位：ms
	@param  : in_y_shot_expoure_time      ---输入：拍摄Y数据时的曝光时间，单位：ms
	@param  : in_z_shot_expoure_time      ---输入：拍摄Z数据时的曝光时间，单位：ms
	@param  : in_roi                      ---输入：参与计算的roi区域，由ScreenViewingAngleGetScreenPatternROI接口获取；
	@param  : out_XYZ                     ---输出：输出的当前视角下的屏幕XYZ数据
	@return	: AlgoError
	@Author : songshoajun@2025.08.28
	************************************/
	C_API AlgoError C_CALL ScreenViewingAngleSingleCompute(AlgoHandle screen_viewingAngle_handle, const AlgoHandle in_x_shot, const AlgoHandle in_y_shot, const AlgoHandle in_z_shot, TriXYZ *out_XYZ);


	/************************************
	@brief  : 计算所有视角下的屏幕视角特性曲线数据
	@note   : 输入所有视角下采集计算得到的XYZ数据及标准光枪数据，得到视角特性曲线
	@param  : screen_viewingAngle_handle      ---输入：有效的屏幕视角特性handle
	@param  : in_XYZ_list					  ---输入：当前屏幕所有视角下的XYZ数据
	@param  : in_angle_list				      ---输入：in_XYZ_list数据中所对应的采集角度
	@param  : in_data_num					  ---输入：输入样本数量
	@param  : in_stand_XYZ_0_data             ---输入：标准光枪数据，此数据必须为光枪在屏幕为0度时采集的数据！！！
	@param  : in_calc_data_mode               ---输入：吐出数据类型，默认模式为=2，in_calc_mode = 0吐出XYZ数据，in_calc_mode = 1吐出Lxy数据，in_calc_mode = 2吐出Lu'v'数据
	@param  : out_screen_lightpattern_curve   ---输入：吐出的屏幕视角特性数据
	【注意】 : 此处吐出的AlgoMat中，按照如下方式存储数据
			  in_calc_mode==0		             in_calc_mode==1			        in_calc_mode==2
		Angle    X      Y      Z	       Angle    L      x      y			  Angle    L      u'   v'
		 -80   99.11  88.11  77.11	   	    -80   99.11  0.11  0.11			   -80   99.11  0.11  0.11
		 -75   99.11  88.11  77.11 	   	    -75   99.11  0.11  0.11			   -75   99.11  0.11  0.11
		 ···						   	    ···								   ···
		 ···						   	    ···								   ···
		 ···						   	    ···								   ···
		  80   99.11  88.11  77.11	   	     80   99.11  0.11  0.11			    80   99.11  0.11  0.11
		  75   99.11  88.11  77.11	   	     75   99.11  0.11  0.11			    75   99.11  0.11  0.11
	@return	: AlgoError
	@Author : songshoajun@2025.08.28
	************************************/
	C_API AlgoError C_CALL ScreenViewingAngleCompute(AlgoHandle screen_viewingAngle_handle, const TriXYZ *in_XYZ_list, const int *in_angle_list, const int in_data_num,
		const TriXYZ in_stand_XYZ_0_data, AlgoMat *out_screen_lightpattern_curve, const int in_calc_data_mode = 2);



	/************************************
	@brief  : 获取对应屏体、颜色下的可视角
	@note   : 输入吐出的屏幕视角特性数据，输出亮度和u'v'色度可视角
	@param  : screen_viewingAngle_handle      ---输入：有效的屏幕视角特性handle
	@param  : in_luv_mat					  ---输入：当前屏体的颜色下算法得到的luv数据特性
	@param  : out_left_lum_angle			  ---输出：左侧亮度(L)可视角
	@param  : out_right_lum_angle			  ---输出：右侧亮度(L)可视角
	@param  : out_left_uv_angle				  ---输出：左侧色度(u'v')可视角
	@param  : out_right_uv_angle			  ---输出：右侧色度(u'v')可视角
	@param  : int_lum_ratio			          ---输入：亮度可视角判断比例(即亮度为0°视角下亮度的多少时被判定为可视角边界)
	@param  : in_delta_uv			          ---输入：色度可视角变化距离判断(deltau'v' 超出多少时被判定为可视角边界)
	@return	: AlgoError
	@Author : shimai@2025.12.3
	************************************/
	C_API AlgoError C_CALL ScreenViewingAngleGetAngle(AlgoHandle screen_viewingAngle_handle, AlgoMat* in_luv_mat, float* out_left_lum_angle, float* out_right_lum_angle, float* out_left_uv_angle, float* out_right_uv_angle, float in_lum_ratio = 0.5f, float in_delta_uv = 0.005f);


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
	C_API AlgoError C_CALL  ScreenViewingAngleRelease(AlgoHandle target_handle);
	C_API void C_CALL ScreenViewingAngleDestroyHandle(AlgoHandle target_handle);




#ifdef __cplusplus
}
#endif // __cplusplus
#endif