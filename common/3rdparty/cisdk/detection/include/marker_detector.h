#ifndef ___MARKER_DETECTOR_H__
#define ___MARKER_DETECTOR_H__

#include "ci_declare.h"
#include "ci_define.h"
#include "ci_data.h"
#include "marker_detector_errors.h"
#include "marker_detector_data.h"
#include "algo_mat.h"


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


	// todo    内存泄露    release峰值  


    /************************************
    @brief    : 创建句柄
    @note     :
    @return   : AlgoHandle
    @Modified : chengjiayi@2022/09/29
    ************************************/
	C_API AlgoHandle C_CALL MarkerCreateHandle();


    /************************************
    @brief    : 设置Marker类型信息
    @note     : 
    @param    : marker_handle   有效的marker句柄
    @param    : in_marker_type  marker类型    
    @param    : in_mark_info    包含：图像的宽，高，  是否黑底白图（是为1，否为0）
    @return   : AlgoError
    @Modified : mumeng@2023/2/17
    ************************************/
    C_API AlgoError C_CALL MarkerSetTypeInfo(AlgoHandle marker_handle, MarkerType* in_marker_type, MarkerInfo* in_mark_info);

	/************************************
	@brief    : 获取Marker类型信息
	@note     :
	@param    : marker_handle   有效的marker句柄
	@param    : out_marker_type  marker类型
	@param    : out_mark_info    包含：图像的宽，高，  是否黑底白图（是为1，否为0）
	@return   : AlgoError
	@Modified : wushaoshuang@2024/4/9
	************************************/
	C_API AlgoError C_CALL MarkerGetTypeInfo(AlgoHandle marker_handle, MarkerType* out_marker_type, MarkerInfo* out_mark_info);

	/************************************
	@brief    : 设置mark点检测方法和检测阈值
	@note     :
	@param    : marker_handle            有效的marker句柄
	@param    : detect_method            mark点检测方法  有三种：基于圆投影    基于轮廓形状识别   基于灰度匹配
	@param    : in_thresh                mark点检测阈值  有三个：mark点面积    检测阈值上限      检测阈值下限
	@return   : AlgoError
	@Modified : mumeng@2022/12/28
	************************************/
	C_API AlgoError C_CALL MarkerSetMethodAndThresh(AlgoHandle marker_handle,const MarkerDetectMethod* in_detect_method, const MarkerDetectThresh* in_thresh);
	
	/************************************
	@brief    : 获取mark点检测方法和检测阈值
	@note     :
	@param    : marker_handle            有效的marker句柄
	@param    : detect_method            mark点检测方法  有三种：基于圆投影    基于轮廓形状识别   基于灰度匹配， 推荐使用基于灰度匹配
	@param    : in_thresh                mark点检测阈值  有三个：mark点面积    检测阈值上限      检测阈值下限
	@return   : AlgoError
	@Modified : mumeng@2022/12/28
	************************************/
	C_API AlgoError C_CALL MarkerGetMethodAndThresh(AlgoHandle marker_handle, MarkerDetectMethod* out_detect_method, MarkerDetectThresh* out_thresh);


	/************************************
	@brief    : 从model模块中载入模板
	@note     :
	@param    : marker_handle         有效的marker句柄
	@param    : in_model_handle       model句柄
	@param    : in_template_name      模板名字,生成model文件时有设置
	@return   : AlgoError
	@Modified : haopanxia@2022/11/09
	************************************/
	C_API AlgoError C_CALL MarkerSetTemplate(AlgoHandle marker_handle, AlgoHandle in_model_handle, const char* in_template_name);


	/************************************
	@brief    : 当检测mark类型为MARKER_RECT_LINE时，设置吐出的点类型
	@note     : 1、当不调用该接口时，in_type默认值为0，取值范围为0，1，2，推荐设置为方法0；
				2、in_type=0代表使用检测出的轮廓点拟合出的直线点；
				3、in_type=1代表直接使用检测出的轮廓点；
				4、in_type=2代表使用距离拟合线非常近的点；
	@param    : marker_handle         输入量，有效的marker句柄
	@param    : in_type               输入量，吐出点的类型
	@return   : AlgoError
	@Modified : songshaojun@2025/07/18
	************************************/
	C_API AlgoError C_CALL MarkerSetLineContourPointsType(AlgoHandle marker_handle, const int in_type = 0);


	/************************************
	@brief    : 检测直角边的线
	@note     : 1.吐出的为检测到的直角边的线距离图像中心的偏差;
				2.吐出数据存储在AlgoMat中，该AlgoMat为N行1列双通道，其中通道0存储delta_x，通道1存储delta_y;
	@param    : marker_handle         输入量，有效的marker句柄
	@param    : in_type               输入量，吐出点的类型
	@return   : AlgoError
	@Modified : songshaojun@2025/07/18
	************************************/
	C_API AlgoError C_CALL MarkerDetectRectLine(AlgoHandle marker_handle, const AlgoMat* in_image, AlgoMat* out_line_points);


	/************************************
	@brief    : 对送入的点进行直线拟合
	@note     : 工具接口,送入点以AlgoMat形式存储，该AlgoMat为N行1列双通道，其中通道0存储x坐标，通道1存储y坐标;
	@param    : in_points_list         输入量，送入的点集
	@param    : out_line_para          输出量，吐出的点集曲线参数，类型为flaot，共计4个参数，按顺序分别为：vx向量，vy向量，任意一点的x坐标，任意一点的y坐标
	@return   : AlgoError
	@Modified : songshaojun@2025/07/18
	************************************/
	C_API AlgoError C_CALL MarkerFitLine(const AlgoMat* in_points_list, float* out_line_para);


	/************************************
	@brief    : 计算任意两条直线的交点
	@note     : 工具接口,送入直线参数以float*存储
	@param    : in_line_para1            输入量，第一条曲线参数
	@param    : in_line_para2            输入量，第二条曲线参数
	@param    : out_cross_point			 输出量，两条直线交叉点
	@return   : songshaojun
	@Modified : 宋绍军@2025/07/18
	************************************/
	C_API AlgoError C_CALL MarkerCalcCrossPoint(const float* in_line_para1, const float* in_line_para2, AlgoPoint2f* out_cross_point);



	/************************************
	@brief    : 从本地图像文件中载入模板
	@note     :
	@param    : marker_handle               有效的marker句柄
	@param    : in_marker_template_files    marker模板文件
	@param    : in_template_name            marker模板名字
	@return   : AlgoError
	@Modified : haopanxia@2022/11/09
	************************************/
	C_API AlgoError C_CALL MarkerLoadTemplate(AlgoHandle marker_handle, const char* in_marker_template_files, const char* in_template_name);

    /************************************
    @brief    : 计算mark中心与图像尺寸中心的偏差
    @note     : 
    @param    : marker_handle       有效的marker句柄
    @param    : in_image            待检测图像
    @param    : out_result          检测结果，结果是mark点中心到图像中心的像素偏差delta_x  delta_y
    @return   : AlgoError
    @Modified : mumeng@2022/12/28
    ************************************/
    C_API AlgoError C_CALL MarkerDetector(AlgoHandle marker_handle, const AlgoMat* in_image, AlgoPoint2f* out_result);

	/************************************
	@brief    : 获取mark点定位的置信度（目标匹配程度）
	@note     : 仅支持定位方法为Method_MatchLocate时调用，输出值表示当前mark点定位的可靠度，最高为1，最低为0，外部可根据该值判断定位是否准确可靠，建议设置0.9以上为可靠，可以以0.1或0.05为步长适当向下调整，一般不低于0.7
	@param    : marker_handle       有效的marker句柄
	@param    : out_mathch_value    输出mark点定位的匹配度(定位结果的置信度)
	@return   : AlgoError
	@Modified : wushaoshuang@2024/04/10
	************************************/
	C_API AlgoError C_CALL MarkerGetMatchValue(AlgoHandle marker_handle, float& out_mathch_value);


	/************************************
	@brief    : 如果是 MARKER_RECT_CORNER 类型mark点，输出直线参数
	@note     : 仅支持定位方法为Method_MatchLocate时调用，输出值表示当前mark点定位的可靠度，最高为1，最低为0，外部可根据该值判断定位是否准确可靠，建议设置0.9以上为可靠，可以以0.1或0.05为步长适当向下调整，一般不低于0.7
	@param    : marker_handle       有效的marker句柄
	@param    : out_line_kb         输出的直线kb值，MarkerLineKB数组，目前是两组 k、b.
	@return   : AlgoError
	@Modified : mumeng@2025/1/17
	************************************/
	C_API AlgoError C_CALL MarkerDetectGetLineKB(AlgoHandle marker_handle, MarkerLineKB* out_line_kb, int kb_len);


	/************************************
	@brief    : 获取marker的轮廓点集大小
	@note     :
	@param    : marker_handle          有效的marker句柄
	@param    : out_len                点数量
	@return   : AlgoError
	@Modified : mumeng@2022/12/27
	************************************/
	C_API AlgoError C_CALL MarkerDetectGetBorderSize(AlgoHandle marker_handle, int* out_len);

	/************************************
	@brief    : 获取marker的轮廓点集                    // 加大小参数
	@note     :
	@param    : marker_handle          有效的marker句柄
	@param    : out_marker_border      marker的轮廓点集
	@return   : AlgoError
	@Modified : haopanxia@2022/09/30
	************************************/
	C_API AlgoError C_CALL MarkerDetectGetBorder(AlgoHandle marker_handle, int* in_len, AlgoPoint* out_marker_border);


	/************************************
	@brief    : 画出mark点轮廓并输出png格式图像
	@note     :
	@param    : marker_handle          有效的marker句柄
	@param    : in_png_filename        图像保存路径
	@return   : AlgoError
	@Modified : haopanxia@2022/09/30
	************************************/
	C_API AlgoError C_CALL MarkerDetectExportResultPng(AlgoHandle marker_handle, const char *in_png_filename);




	/************************************
	@brief    : 画出mark点轮廓并输出png格式图像
	@note     :
	@param    : marker_handle          有效的marker句柄
	@param    : out_result_mat          mat图像
	@return   : AlgoError
	@Modified : haopanxia@2024/11/20
	************************************/
	C_API AlgoError C_CALL MarkerDetectGetResultMat(AlgoHandle marker_handle, AlgoMat* out_result_mat);




	/************************************
	@brief    : 获取buffer大小
	@note     :
	@param    : marker_handle          有效的marker句柄
	@param    : out_len                buffer大小
	@return   : AlgoError
	@Modified : haopanxia@2022/09/30
	************************************/
	C_API AlgoError C_CALL MarkerDetectGetBufferSize(AlgoHandle marker_handle, uint64_t* out_len);

	/************************************
	@brief    : 将MarkerDetect里面的数据导出到buffer
	@note     :
	@param    : marker_handle       有效的marker句柄
	@param    : out_buffer          已经申请好申请空间
	@param    : in_out_len          外部申请好的空间大小,会用真实大小修改
	@return   : AlgoError
	@Modified : haopanxia@2022/09/30
	************************************/
	C_API AlgoError C_CALL MarkerDetectToBuffer(AlgoHandle marker_handle, char* out_buffer, uint64_t* in_out_len);

	/************************************
	@brief    : 将buffer数据导入到MarkerDetect
	@note     :
	@param    : marker_handle          有效的marker句柄
	@param    : in_buffer
	@param    : in_len
	@return   : AlgoError
	@Modified : haopanxia@2022/09/30
	************************************/
	C_API AlgoError C_CALL MarkerDetectFromBuffer(AlgoHandle marker_handle, const char* in_buffer, uint64_t in_len);

	/*********************************************
	@brief   : 保存MarkerDetect文件
	@note    :
	@param   : marker_handle           有效的marker句柄
	@param   : in_save_path            输入的文件路径，后缀必须为.marker
	@return  : AlgoError
	@Author  : wushaoshuang@2024/04/09
	*********************************************/
	C_API AlgoError C_CALL MarkerDetectSave(AlgoHandle marker_handle, const char* in_save_path);

	/*********************************************
	@brief   : 载入MarkerDetect文件
	@note    :
	@param   : marker_handle           有效的marker句柄
	@param   : in_load_path            输入的文件路径，后缀必须为.marker
	@return  : AlgoError
	@Author  : wushaoshuang@2024/04/09
	*********************************************/
	C_API AlgoError C_CALL MarkerDetectLoad(AlgoHandle marker_handle, const char* in_load_path);


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
    C_API AlgoError C_CALL MarkerRelease(AlgoHandle marker_handle);
	C_API void C_CALL MarkerDestroyHandle(AlgoHandle marker_handle);



#ifdef __cplusplus
}
#endif // __cplusplus
#endif