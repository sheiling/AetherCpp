#ifndef __SUBPIXEL_DISPLAY_EVALUATOR_H__
#define __SUBPIXEL_DISPLAY_EVALUATOR_H__

#include "ci_declare.h"
#include "ci_define.h"
#include "ci_data.h"
#include "algo_mat.h"

#include "subpixel_display_evaluator_data.h"
#include "subpixel_display_evaluator_error.h"


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	/************************************
	@brief    : 创建句柄
	@note     :
	@param    : evaluator_handle
	@return   : AlgoError
	@Author   : wushaoshuang@2025/01/08
	************************************/
	C_API AlgoError C_CALL SubpixelDisplayCreateHandle(AlgoHandle* evaluator_handle);
	
	/************************************
	@brief    : 计算模糊程度
	@note     : 用于计算相机的对焦程度，输入一张图像，评价一个结果，输入图像必须为白色画面下的Y数据
	@param    : evaluator_handle    有效的测评句柄
	@param    : in_camera_data      输入的相机待测数据
	@param    : in_screen_bulid_size 输入的屏体搭建时宽高方向上所用模组的数量，如该屏体在宽方向上由3个模组（灯板）拼成，在高方向上由2个模组（灯板）拼成，则输入为AalgoSize（3，2）
	@param    : out_judge_result    输出的模糊程度判断结果，以BlurEvaluateResult枚举类型表示，详情见subpixel_display_evaluator_data.h
	@return   : AlgoError
	@Author   : wushaoshuang@2025/01/08
	************************************/
	C_API AlgoError C_CALL SubpixelDisplayCalcBlurLevel(AlgoHandle evaluator_handle, const AlgoMat* in_camera_data, const AlgoSize* in_screen_bulid_size, BlurEvaluateResult* out_judge_result);

	/************************************
	@brief    : 设置标准数据
	@note     : 设置标准数据，即将相机原始数据对齐到光枪标准值上，进一步提高测评的准确性，外部根据实际情况选择性调用即可，该接口若不调用，内部直接用相机的原始数据进行测评
	@param    : evaluator_handle       有效的测评句柄
	@param    : in_camera_data         输入的相机采集的XYZ原始数据，要求采集的屏幕为白色画，以AlgoMatXYZData结构体传入，详情见subpixel_display_evaluator_data.h
	@param    : in_standard_data       输入的标准XYZ数据，一般为光枪采集的屏幕数据，以StandardXYZ结构体传入，详情见subpixel_display_evaluator_data.h
	@return   : AlgoError
	@Author   : wushaoshuang@2025/01/08
	************************************/
	C_API AlgoError C_CALL SubpixelDisplaySetStandardXYZ(AlgoHandle evaluator_handle, const AlgoMatXYZData* in_camera_data, const StandardXYZ* in_standard_data);

	/************************************
	@brief    : 设置清晰度评价数据
	@note     : 设置清晰度评价数据，数据为多个pattern下的Y数据
	@param    : evaluator_handle       有效的测评句柄
	@param    : in_sharpness_data_list 输入的清晰度评价数据，以AlgoMat的数组传入
	@param    : in_len                 输入的清晰度评价数据个数，即AlgoMat数组的大小
	@param    : in_org_mode_data_list  输入的原始模式下清晰度评价数据，以AlgoMat的数组传入，其大小与in_sharpness_data_list相同，外部可根据实际情况选择性输入，默认为不输入，即空指针，若输入后其内部会以此为基准对后续的数据进行归一化处理，可提高数据的重复性
	@return   : AlgoError
	@Author   : wushaoshuang@2025/01/08
	************************************/
	C_API AlgoError C_CALL SubpixelDisplaySetSharpnessData(AlgoHandle evaluator_handle, const AlgoMat* in_sharpness_data_list, const int* in_len, AlgoMat* in_org_mode_data_list = nullptr);

	/************************************
	@brief    : 设置边缘颜色错误评价数据
	@note     : 设置边缘颜色错误评价数据，数据为多个pattern下的XYZ数据
	@param    : evaluator_handle         有效的测评句柄
	@param    : in_color_error_data_list 输入的颜色错误评价数据，以AlgoMatXYZData结构体的数组传入，详情见subpixel_display_evaluator_data.h
	@param    : in_len                   输入的颜色错误评价数据个数，即AlgoMatXYZData结构体数组的大小
	@param    : in_org_mode_data_list    输入的原始模式下颜色错误评价数据，以AlgoMatXYZData结构体的数组传入，其大小与in_color_error_data_list相同，外部可根据实际情况选择性输入，默认为不输入，即空指针，若输入后其内部会以此为基准对后续的数据进行归一化处理，可提高数据的重复性
	@return   : AlgoError
	@Author   : wushaoshuang@2025/01/08
	************************************/
	C_API AlgoError C_CALL SubpixelDisplaySetColorErrorData(AlgoHandle evaluator_handle, const AlgoMatXYZData* in_color_error_data_list, const int* in_len, AlgoMatXYZData* in_org_mode_data_list = nullptr);

	/************************************
	@brief    : 设置锯齿评价数据
	@note     : 设置锯齿评价数据，数据为多个pattern下的Y数据，如果测评类型为清晰度、颜色错误、锯齿综合测评时，该接口可以不调用，内部默认数据与颜色错误数据共用，具体可根据实际情况设置
	@param    : evaluator_handle         有效的测评句柄
	@param    : in_sawtooth_data_list    输入的锯齿评价数据，以AlgoMat的数组传入
	@param    : in_len                   输入的锯齿评价数据个数，即AlgoMat数组的大小
	@param    : in_org_mode_data_list    输入的原始模式下锯齿评价数据，以AlgoMat的数组传入，其大小与in_sawtooth_data_list相同，外部可根据实际情况选择性输入，默认为不输入，即空指针，若输入后其内部会以此为基准对后续的数据进行归一化处理，可提高数据的重复性
	@return   : AlgoError
	@Author   : wushaoshuang@2025/01/08
	************************************/
	C_API AlgoError C_CALL SubpixelDisplaySetSawtoothData(AlgoHandle evaluator_handle, const AlgoMat* in_sawtooth_data_list, const int* in_len, AlgoMat* in_org_mode_data_list = nullptr);

	/************************************
	@brief    : 设置屏体ROI
	@note     : 设置屏体ROI四个角点，角点顺序必须为左上、右上、右下、左下，即按照顺时针方向输入对应角点
	@param    : evaluator_handle         有效的测评句柄
	@param    : in_corner4               输入的四个角点数据，以AlgoPoint的数组传入
	@param    : in_num                   输入的四个角点数据个数，即默认为4个角点
	@return   : AlgoError
	@Author   : wushaoshuang@2025/01/08
	************************************/
	C_API AlgoError C_CALL SubpixelDisplaySetROI(AlgoHandle evaluator_handle, const AlgoPoint* in_corner4, int in_num = 4);

	/************************************
	@brief    : 获取屏体ROI
	@note     : 获取屏体ROI四个角点，角点顺序为左上、右上、右下、左下，即按照顺时针方向输出对应角点，外部根据需求调用
	@param    : evaluator_handle         有效的测评句柄
	@param    : out_corner4              输出的四个角点数据，以AlgoPoint的数组输出，需要外部new4个角点的空间
	@return   : AlgoError
	@Author   : wushaoshuang@2025/01/08
	************************************/
	C_API AlgoError C_CALL SubpixelDisplayGetROI(AlgoHandle evaluator_handle, AlgoPoint* out_corner4);

	/************************************
	@brief    : 设置评价权重
	@note     : 设置评价权重，在对清晰度、颜色错误、锯齿三个维度综合测评时设置有效，内部默认权重为80，7，4。外部根据实际情况选择性的设置
	@param    : evaluator_handle         有效的测评句柄
	@param    : in_evaluate_weight       输入的评价权重，以EvaluateWeight的结构体输入，详情见subpixel_display_evaluator_data.h
	@return   : AlgoError
	@Author   : wushaoshuang@2025/01/08
	************************************/
	C_API AlgoError C_CALL SubpixelDisplaySetWeight(AlgoHandle evaluator_handle, const EvaluateWeight* in_evaluate_weight);

	/************************************
	@brief    : 获取评价权重
	@note     : 获取评价权重，外部根据需求调用
	@param    : evaluator_handle         有效的测评句柄
	@param    : out_evaluate_weight      输出的评价权重，以EvaluateWeight的结构体输出，详情见subpixel_display_evaluator_data.h
	@return   : AlgoError
	@Author   : wushaoshuang@2025/01/08
	************************************/
	C_API AlgoError C_CALL SubpixelDisplayGetWeight(AlgoHandle evaluator_handle, EvaluateWeight* out_evaluate_weight);

	/************************************
	@brief    : 设置屏体分辨率基准
	@note     : 设置屏体分辨率基准，外部根据需求调用，内部默认为2k，即1920*1080
	@param    : evaluator_handle         有效的测评句柄
	@param    : in_physic_resolution     输入的分辨率基准，以AlgoSize类型输入，其中width为宽方向的分辨率基准，height为高方向的分辨率基准
	@return   : AlgoError
	@Author   : wushaoshuang@2025/01/08
	************************************/
	C_API AlgoError C_CALL SubpixelDisplaySetResolution(AlgoHandle evaluator_handle, const AlgoSize* in_physic_resolution);

	/************************************
	@brief    : 获取屏体分辨率基准
	@note     : 获取屏体分辨率基准，外部根据需求调用
	@param    : evaluator_handle         有效的测评句柄
	@param    : out_physic_resolution    输出的分辨率基准，以AlgoSize类型输出，其中width为宽方向的分辨率，height为高方向的分辨率
	@return   : AlgoError
	@Author   : wushaoshuang@2025/01/08
	************************************/
	C_API AlgoError C_CALL SubpixelDisplayGetResolution(AlgoHandle evaluator_handle, AlgoSize* out_physic_resolution);

	/************************************
	@brief    : 测评主接口
	@note     : 测评主接口，属于综合测评接口，内部对清晰度、颜色错误、锯齿三个维度综合测评，接口返回三个维度的综合测评结果
	@param    : evaluator_handle         有效的测评句柄
	@param    : out_evaluate_value       输出的综合测评值
	@return   : AlgoError
	@Author   : wushaoshuang@2025/01/08
	************************************/
	C_API AlgoError C_CALL SubpixelDisplayEvaluate(AlgoHandle evaluator_handle, float* out_evaluate_value);

	/************************************
	@brief    : 获取测评结果
	@note     : 获取测评结果，获取内部对清晰度、颜色错误、锯齿三个维度各维度下的测评结果，根据需求调用
	@param    : evaluator_handle         有效的测评句柄
	@param    : out_result               输出的各维度下的测评结果，以EvaluateResult结果体类型输出，详情见subpixel_display_evaluator_data.h
	@return   : AlgoError
	@Author   : wushaoshuang@2025/01/08
	************************************/
	C_API AlgoError C_CALL SubpixelDisplayGetResult(AlgoHandle evaluator_handle, EvaluateResult* out_result);

	/************************************
	@brief    : 清晰度测评接口
	@note     : 清晰度测评接口，该接口仅对清晰度这一维度单独测评，与SubpixelDisplaySetSharpnessData接口配对使用
	@param    : evaluator_handle         有效的测评句柄
	@param    : out_result               输出的清晰度测评结果，以SharpnessResult结果体类型输出，详情见subpixel_display_evaluator_data.h
	@return   : AlgoError
	@Author   : wushaoshuang@2025/01/08
	************************************/
	C_API AlgoError C_CALL SubpixelDisplayEvaluateSharpness(AlgoHandle evaluator_handle, SharpnessResult* out_result);

	/************************************
	@brief    : 颜色错误测评接口
	@note     : 线条边缘颜色错误测评接口，该接口仅对颜色错误这一维度单独测评，与SubpixelDisplaySetColorErrorData接口配对使用
	@param    : evaluator_handle         有效的测评句柄
	@param    : out_result               输出的颜色错误测评结果
	@return   : AlgoError
	@Author   : wushaoshuang@2025/01/08
	************************************/
	C_API AlgoError C_CALL SubpixelDisplayEvaluateColorError(AlgoHandle evaluator_handle, float* out_result);

	/************************************
	@brief    : 锯齿测评接口
	@note     : 锯齿测评接口，该接口仅对锯齿这一维度单独测评，与SubpixelDisplaySetSawtoothData接口配对使用
	@param    : evaluator_handle         有效的测评句柄
	@param    : out_result               输出的锯齿测评结果
	@return   : AlgoError
	@Author   : wushaoshuang@2025/01/08
	************************************/
	C_API AlgoError C_CALL SubpixelDisplayEvaluateSawtooth(AlgoHandle evaluator_handle, float* out_result);

	/*********************************************
	@brief   : 获取buffer大小
	@note    :
	@param   : evaluator_handle    有效的测评句柄
	@param   : out_len             buffer大小
	@return  : AlgoError
	@Author  : wushaoshuang@2025/01/08
	*********************************************/
	C_API AlgoError C_CALL SubpixelDisplayGetBufferSize(AlgoHandle evaluator_handle, uint64_t* out_len);

	/*********************************************
	@brief   : 将数据导入buffer
	@note    :
	@param   : evaluator_handle    有效的测评句柄
	@param   : out_buffer          输出检测参数的buffer空间
	@param   : in_out_len          buffer空间大小
	@return  : AlgoError
	@Author  : wushaoshuang@2025/01/08
	*********************************************/
	C_API AlgoError C_CALL SubpixelDisplayToBuffer(AlgoHandle evaluator_handle, char* out_buffer, uint64_t* in_out_len);

	/*********************************************
	@brief   : 将buffer数据读取到算法内
	@note    :
	@param   : evaluator_handle    有效的测评句柄
	@param   : in_buffer               buffer
	@param   : in_len                  buffer大小
	@return  : AlgoError
	@Author  : wushaoshuang@2025/01/08
	*********************************************/
	C_API AlgoError C_CALL SubpixelDisplayFromBuffer(AlgoHandle evaluator_handle, const char* in_buffer, uint64_t in_len);

	/*********************************************
	@brief   : 载入subpixel文件
	@note    :
	@param   : evaluator_handle    有效的测评句柄
	@param   : in_load_path        输入的文件路径，后缀必须为.subpixel
	@return  : AlgoError
	@Author  : wushaoshuang@2025/01/08
	*********************************************/
	C_API AlgoError C_CALL SubpixelDisplayLoad(AlgoHandle evaluator_handle, const char* in_load_path);

	/*********************************************
	@brief   : 保存subpixel文件
	@note    :
	@param   : evaluator_handle    有效的测评句柄
	@param   : in_save_path        输入要保存的文件路径，后缀必须为.subpixel
	@return  : AlgoError
	@Author  : wushaoshuang@2025/01/08
	*********************************************/
	C_API AlgoError C_CALL SubpixelDisplaySave(AlgoHandle evaluator_handle, const char* in_save_path);

	/*********************************************
	@brief   : 保存默认config文件
	@note    : 保存测评所需的config文件，该文件中保存的是算发内部默认的数据，即生成的congfig为默认config，该config文件中将三灯和四灯的默认参数均进行了保存，在使用时只需正确输入对应的SubpixelLayoutType即可
	@param   : evaluator_handle    有效的测评句柄
	@param   : in_save_path        输入要保存的文件路径，后缀必须为.spinit
	@return  : AlgoError
	@Author  : wushaoshuang@2025/02/28
	*********************************************/
	C_API AlgoError C_CALL SubpixelDisplaySaveConfig(AlgoHandle evaluator_handle, const char* in_save_path);

	/*********************************************
	@brief   : 载入config文件
	@note    : 载入测评所需的config文件，该文件可以为算法默认保存的config，也可为经过setconfig重新生成后的config，注意：若在实际测评时，若测评时不输入原始模式数据，则该接口无需调用
	@param   : evaluator_handle    有效的测评句柄
	@param   : in_load_path        输入要载入的文件路径，后缀必须为.spinit
	@param   : in_subpixel_type    输入的子像素屏体类型，以SubpixelLayoutType枚举类型输入
	@return  : AlgoError
	@Author  : wushaoshuang@2025/02/28
	*********************************************/
	C_API AlgoError C_CALL SubpixelDisplayLoadConfig(AlgoHandle evaluator_handle, const char* in_load_path, const SubpixelLayoutType* in_subpixel_type);

	/*********************************************
	@brief   : 设置config文件
	@note    : 重新设置config文件，即使用默认config文件进行检测时效果不理想时，请使用该接口重新设置以下congfig文件，以使其参数尽可能接近当前测评环境，
	           推荐在环境变动较大时，重新设置一次config文件。不同config文件下的检测结果不可相互对比，同一config文件下以及环境基本相同下的检测结果才具有可比性。
			   该接口需要配合SubpixelDisplaySetSharpnessData接口、或SubpixelDisplaySetColorErrorData接口、或SubpixelDisplaySetSawtoothData接口使用
			   即在调用该接口前，必须设置对应的基准数据后，算法内部才可修改对应的config文件，否则算法报错。详情使用情况请参考对应demo
			   该接口调用完后会自动生成一个新的config文件，使用该新的文件进行测评即可。
	@param   : evaluator_handle    有效的测评句柄
	@param   : in_save_path        输入要设置config文件后要保存的路径，后缀必须为.spinit
	@param   : in_subpixel_type    输入的子像素屏体类型，以SubpixelLayoutType枚举类型输入，注：设置了该枚举后，该接口生成的config文件，后续只能用于相同排布类型的屏体测评
	@return  : AlgoError
	@Author  : wushaoshuang@2025/02/28
	*********************************************/
	C_API AlgoError C_CALL SubpixelDisplaySetConfig(AlgoHandle evaluator_handle, const char* in_save_path, const SubpixelLayoutType* in_subpixel_type);

	/*********************************************
	@brief   : 空间内存释放
	@note    : 句柄指向的对象销毁，此句柄已经无效，请配合handle=nullptr
	@param   : evaluator_handle    有效的测评句柄
	@return  :
	@Author  : wushaoshuang@2025/01/08
	*********************************************/
	C_API void C_CALL SubpixelDisplayDestroyHandle(AlgoHandle evaluator_handle);

	/*********************************************
	@brief   : 句柄指向的对象的内存空间释放，句柄指向的对象还在
	@note    : 内存释放
	@param   : evaluator_handle    有效的测评句柄
	@return  : AlgoError
	@Author  : wushaoshuang@2025/01/08
	*********************************************/
	C_API AlgoError C_CALL SubpixelDisplayRelease(AlgoHandle evaluator_handle);



#ifdef __cplusplus
}
#endif // __cplusplus
#endif