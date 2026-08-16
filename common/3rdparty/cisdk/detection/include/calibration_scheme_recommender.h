#ifndef __CALIBRATION_SCHEME_RECOMMENDER_H__
#define __CALIBRATION_SCHEME_RECOMMENDER_H__

#include "ci_declare.h"
#include "ci_define.h"
#include "ci_data.h"
#include "calibration_scheme_recommender_data.h"
#include "calibration_scheme_recommender_errors.h"
#include "algo_mat.h"


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


    /************************************
    @brief    : 创建句柄
    @note     :
	@param    : out_recommend_handle    校正方案推荐句柄
    @return   : AlgoError
    @Modified : zhangxuan@2025/11/04
    ************************************/
    C_API AlgoError C_CALL CalibrationSchemeRecommendCreateHandle(AlgoHandle* out_recommend_handle);


	/************************************
	@brief    : set校正方案推荐 需要采集的灰阶列表（灰阶必须从小到大），和灰阶数量
	@note     : 
	@param    : recommend_handle        校正方案推荐句柄
	@param    : in_gray_list            输入需要采集的灰阶列表（灰阶从小到大）
	@param    : in_gray_num             灰阶数量
	@return	  : AlgoError
	@Modified : chengjuanru@2025/12/11
	************************************/
	C_API AlgoError C_CALL CalibrationSchemeRecommendSetGrayList(AlgoHandle recommend_handle, const int* in_gray_list, const int in_gray_num);


	/************************************
	@brief    : set校正方案推荐需要的meter数据（这里的meter是经过边缘-异常点-曲面-归一化处理后的）
	@note     : （注意：meter一定要有灯板的宽高信息，即LedPanel.wh一定要是正确的！！！！！！
						meter一定要有灰阶信息，即Layer.value一定要是正确的！！！！！！)
				这里的meter数量需要和CalibrationSchemeRecommendSetGrayList接口的灰阶数一致，
				调用一次该接口就会set某一颜色滤片的数据，且对应灰阶必须从低到高。
			    仅支持：1）GBW需要同时输入XYZ滤片数据 2）对于红色R必须同时输入XY滤片的数据（Z输入nullptr）3）其他情况都会报错
			
	@param    : recommend_handle        校正方案推荐句柄
	@param    : in_meter_x              输入的X滤片的meters
	@param    : in_meter_y              输入的Y滤片的meters
	@param    : in_meter_z              输入的Z滤片的meters
	@param    : in_meter_num            输入的meters的数量，这里必须和CalibrationSchemeRecommendSetGrayList接口set的灰阶数量一致
										若meters的数量和灰阶数量不一致，会报错
	@return	  : AlgoError
	@Modified : chengjuanru@2025/12/11
	************************************/
	C_API AlgoError C_CALL CalibrationSchemeRecommendSetMeter(AlgoHandle recommend_handle,
			                                        AlgoHandle* in_meter_x, AlgoHandle* in_meter_y, AlgoHandle* in_meter_z, const int in_meter_num);
	

	/************************************
	@brief    : 设置亮度目标值
	@note     :
	@param    : recommend_handle        校正方案推荐句柄
	@param    : in_target               亮度目标值 RY GY BY
	@return	  : AlgoError
	@Modified : chengjuanru@2025/12/11
	************************************/
	C_API AlgoError C_CALL CalibrationSchemeRecommendSetLumTargetValue(AlgoHandle recommend_handle, const RGBLxy* in_target);


	/************************************
	@brief    : 设置色度目标值
	@note     :
	@param    : recommend_handle        校正方案推荐句柄
	@param    : in_target               色度目标值
	@return	  : AlgoError
	@Modified : chengjuanru@2025/12/11
	************************************/
	C_API AlgoError C_CALL CalibrationSchemeRecommendSetChromaTargetValue(AlgoHandle recommend_handle, const RGBWLxy* in_target);


	/************************************
	@brief    : 设置资源情况
	@note     : 若接受卡支持4层校正，那么亮度校正和白色校正(若有)加在一起一共四层
				如果设置了资源，此时调用CalibrationSchemeRecommendCompute接口，会给出给定资源下的方案
	            也可以不设置，若不设置，此时调用CalibrationSchemeRecommendCompute接口，会给出不限制资源下方案
	@param    : recommend_handle        校正方案推荐句柄
	@param    : in_num                  接受卡支持的校正层数
	@param    : in_white_flag           接受卡是否支持白色校正
	@return	  : AlgoError
	@Modified : chengjuanru@2025/12/11
	************************************/
	C_API AlgoError C_CALL CalibrationSchemeRecommendSetResource(AlgoHandle recommend_handle, const int in_num, const bool in_white_flag);


	/************************************
	@brief    : 设置gamma表
	@note     : 
	@param    : recommend_handle        校正方案推荐句柄
	@param    : in_gamma_rgb            gamma表 尺寸为256x3(wh)
	@return	  : AlgoError
	@Modified : chengjuanru@2025/12/11
	************************************/
	C_API AlgoError C_CALL CalibrationSchemeRecommendSetGammaTable(AlgoHandle recommend_handle, const AlgoMat* in_gamma_rgb);



	/************************************
	@brief    : 校正层数and灰阶推荐
	@note     : 如果输入了接收卡资源(即调用CalibrationSchemeRecommendSetResource)，该接口会计算给出给定资源限制下的方案；
				如果没有输入接收卡资源，则该接口会计算给出不限制资源下的方案。
	@param    : recommend_handle        校正方案推荐句柄
	@return	  : AlgoError
	@Modified : chengjuanru@2025/12/11
	************************************/
	C_API AlgoError C_CALL CalibrationSchemeRecommendCompute(AlgoHandle recommend_handle);


	/************************************
	@brief    : 亮度校正迭代判断
	@note     : 若不需要迭代，则迭代次数为0，只需采集一次数据并生成系数
				若迭代次数为1，则需要执行：采集数据-生成系数-上传系数-采集数据-生成系数-上传系数，
				
				（输入的meter一定要是修正了曲面的，且meter中一定要有灰阶信息，即Layer.value一定要是正确的！！！！！！）
				接口一次只能输入一个灰阶的数据
	
	@param    : recommend_handle        校正方案推荐句柄
	@param    : in_ry_meter             RY meter（一定要修正过曲面）
	@param    : in_gy_meter             GY meter（一定要修正过曲面）
	@param    : in_by_meter             BY meter（一定要修正过曲面）
	@param    : out_flag                是否继续迭代，=false表示不继续迭代  =true表示需要继续迭代
	@return	  : AlgoError
	@Modified : chengjuanru@2025/12/11
	************************************/
	C_API AlgoError C_CALL CalibrationSchemeRecommendJudgeLumIteration(AlgoHandle recommend_handle,
		                                                        AlgoHandle in_ry_meter, AlgoHandle in_gy_meter, AlgoHandle in_by_meter, bool* out_flag);


	/************************************
	@brief    : 白色校正迭代判断
	@note     : 若不需要迭代，则迭代次数为0，只需采集一次数据并生成系数
				若迭代次数为1，则需要执行：采集数据-生成系数-上传系数-采集数据-生成系数-上传系数，
				
				（输入的meter一定要是修正了曲面的，且meter中一定要有灰阶信息，即Layer.value一定要是正确的！！！！！！）
				接口一次只能输入一个灰阶的数据
	
	@param    : recommend_handle        校正方案推荐句柄
	@param    : in_wx_meter             WX meter（一定要修正过曲面）
	@param    : in_wy_meter             WY meter（一定要修正过曲面）
	@param    : in_wz_meter             WZ meter（一定要修正过曲面）
	@param    : out_flag                是否继续迭代，=false表示不继续迭代  =true表示需要继续迭代
	@return	  : AlgoError
	@Modified : chengjuanru@2025/12/11
	************************************/
	C_API AlgoError C_CALL CalibrationSchemeRecommendJudgeWhiteIteration(AlgoHandle recommend_handle, 
		                                                         AlgoHandle in_wx_meter, AlgoHandle in_wy_meter, AlgoHandle in_wz_meter, bool* out_flag);


	/************************************
	@brief  : 读取保存校正方案推荐文件
	@note   : 后缀只能是.scheme
	@param  : recommend_handle          有效的校正方案推荐句柄
	@param  : in_fullname               校正方案推荐文件,后缀只能是.scheme
	@return	: AlgoError
	@Author : chengjuanru@2025/12/11
	************************************/
	C_API AlgoError C_CALL CalibrationSchemeRecommendSave(AlgoHandle recommend_handle, const char* in_fullname);
	C_API AlgoError C_CALL CalibrationSchemeRecommendLoad(AlgoHandle recommend_handle, const char* in_fullname);



	/************************************
	@brief  : 获取buffer大小
	@note   :
	@param  : recommend_handle  当前有效句柄
	@param  : out_len             buffer大小
	@return	: AlgoError
	@Author : chengjuanru@2025/12/11
	************************************/
	C_API AlgoError C_CALL CalibrationSchemeRecommendGetBufferSize(AlgoHandle recommend_handle, uint64_t* out_len);


	/************************************
	@brief  : 将recommend_handle里面的数据导出到buffer
	@note   :
	@param  : recommend_handle    当前有效句柄
	@param  : out_buffer          已经申请好申请空间
	@param  : in_out_len          外部申请好的空间大小,会用真实大小修改
	@return	: AlgoError
	@Author : chengjuanru@2025/12/11
	************************************/
	C_API AlgoError C_CALL CalibrationSchemeRecommendToBuffer(AlgoHandle recommend_handle, char* out_buffer, uint64_t* in_out_len);


	/************************************
	@brief  : 将buffer数据导入到recommend_handle
	@note   :
	@param  : recommend_handle   有效句柄
	@param  : in_buffer
	@param  : in_len
	@return	: AlgoError
	@Author : chengjuanru@2025/12/11
	************************************/
	C_API AlgoError C_CALL CalibrationSchemeRecommendFromBuffer(AlgoHandle recommend_handle, const char* in_buffer, uint64_t in_len);


	/************************************
	@brief  : 获取句柄内容
	@note   : 注意浅拷贝
	@param  : recommend_handle           有效的recommend_handle句柄
	@param  : out_scheme_recommend       导出的结构体,必须已经初始化完毕
	@param  : is_deep_copy               是否深拷贝, 默认浅拷贝  1=深拷贝;0=浅拷贝
	@return	: AlgoError
	@Author : chengjuanru@2025/12/11
	************************************/
	C_API AlgoError C_CALL CalibrationSchemeRecommendGet(AlgoHandle recommend_handle, AlgoCalibrationSchemeRecommend* out_scheme_recommend, int in_is_deep_copy = 0);



	/************************************
	@brief  : 设置内容
	@note   :
	@param  : recommend_handle      有效的recommend_handle句柄
	@param  : in_scheme_recommend   要输入的数据
	@return	: AlgoError
	@Author : chengjuanru@2025/12/11
	************************************/
	C_API AlgoError C_CALL CalibrationSchemeRecommendSet(AlgoHandle recommend_handle, const AlgoCalibrationSchemeRecommend* in_scheme_recommend);


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
	C_API AlgoError C_CALL CalibrationSchemeRecommendRelease(AlgoHandle recommend_handle);
	C_API void C_CALL CalibrationSchemeRecommendDestroyHandle(AlgoHandle recommend_handle);



#ifdef __cplusplus
}
#endif // __cplusplus
#endif