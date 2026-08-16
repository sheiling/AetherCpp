#ifndef __CAMERA_CHROMA_CALIB_H__
#define __CAMERA_CHROMA_CALIB_H__

#include "ci_define.h"
#include "ci_declare.h"
#include "camera_chroma_calib_data.h"
#include "camera_chroma_calib_error.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

    /************************************
    @brief  : 创建相机标定句柄
    @note   :
    @param  : out_camcalib_handle    相机标定句柄
    @return	: AlgoError
    @Author : ouxingtao@2022/04/02
    ************************************/
    C_API AlgoError C_CALL CamCalibCreateHandle(AlgoHandle* out_camcalib_handle);

    /************************************
    @brief  : 设置中心区域半径，默认半径20
    @note   : 实际映射未 2r*2r的矩形区域
    @param  : camcalib_handle    有效的相机标定句柄
    @param  : in_radius          输入半径
    @return	: AlgoError
    @Author : ouxingtao@2022/04/02
    ************************************/
    C_API AlgoError C_CALL CamCalibSetRadius(AlgoHandle camcalib_handle, const int in_radius);

    /************************************
    @brief  : 计算标定矩阵
    @note   : 
    @param  : camcalib_handle           有效的相机标定句柄
    @param  : in_x_filenames            包含num组x_meter路径
    @param  : in_y_filenames            包含num组y_meter路径
    @param  : in_z_filenames            包含num组z_meter路径
    @param  : in_colorimeter_xyz        包含num组TriXYZ路径
    @param  : in_num                    输入标定数据组数
    @param  : in_color                  输入颜色信息
    @return	: AlgoError
    @Author : ouxingtao@2022/04/02
    ************************************/
    C_API AlgoError C_CALL CamCalibCompute(AlgoHandle camcalib_handle, const char* in_x_filenames[], const char* in_y_filenames[],
        const char* in_z_filenames[], TriXYZ* in_colorimeter_xyz, const int in_num, AlgoColor in_color);

    /************************************
    @brief  : 校准相机数据
    @note   :
    @param  : camcalib_handle           有效的相机标定句柄
    @param  : in_r_out_x_meter_handle   输入有效的r_meter句柄，输出属性变为CIEType = X
    @param  : in_g_out_y_meter_handle   输入有效的g_meter句柄，输出属性变为CIEType = Y
    @param  : in_b_out_z_meter_handle   输入有效的z_meter句柄，输出属性变为CIEType = Z
    @param  : in_color                  输入颜色信息
    @return	: AlgoError
    @Author : ouxingtao@2022/04/02
    ************************************/
    C_API AlgoError C_CALL CamCalibRectify(AlgoHandle camcalib_handle, AlgoHandle in_r_out_x_meter_handle,
        AlgoHandle in_g_out_y_meter_handle, AlgoHandle in_b_out_z_meter_handle, AlgoColor in_color);

    /************************************
    @brief  : 读取保存标定相机标定文件
    @note   : 后缀只能是.camcalib
    @param  : curvecalib_handle        有效的curvecalib句柄
    @param  : in_camcalib_fullname     标定文件,后缀只能是.camcalib或.camcalib.yml
    @return	: AlgoError
    @Author : ouxingtao@2022/04/02
    ************************************/
    C_API AlgoError C_CALL CamCalibSave(AlgoHandle camcalib_handle, const char* in_camcalib_fullname);
    C_API AlgoError C_CALL CamCalibLoad(AlgoHandle camcalib_handle, const char* in_camcalib_fullname);


	/************************************
   @brief  : 设置计算方法
   @note   : 共三种，1-对应原始转换矩阵方法；2-对应仅计算K方法；3-对应计算K和b方式 4-对应计算相机机差模式
   @param  : camcalib_handle    有效的相机标定句柄
   @param  : method             对应方法
   @return : AlgoError
   @Author : liying@2023/09/20    dulonghui@2023/11/13，补充
   ************************************/
	C_API AlgoError C_CALL CamCalibSetMethod(AlgoHandle camcalib_handle, const int method);

	/************************************
	@brief  : 计算标定K/Kb值
	@note   :
	@param  : camcalib_handle           有效的相机标定句柄
	@param  : in_x_filenames            包含in_num组相机采集X_meter
	@param  : in_y_filenames            包含in_num组相机采集Y_meter
	@param  : in_z_filenames            包含in_num组相机采集Z_meter
	@param  : in_x_colorimeterlist      包含in_num组光枪采集X数据
	@param  : in_x_colorimeterlist      包含in_num组光枪采集Y数据
	@param  : in_x_colorimeterlist      包含in_num组光枪采集Z数据
	@param  : in_num                    输入标定数据组数
	@param  : in_color                  输入颜色信息
	@param  : in_point                  色度计采集区域对应中心点坐标：默认为meter对应数据中心点坐标
    @param  : in_radius                 色度计半径：默认为15
	@return	: AlgoError
	@Author : liying@2023/09/20
	************************************/
	C_API AlgoError C_CALL CamCalibComputeKb(AlgoHandle camcalib_handle, 
		                                     AlgoHandle* in_x_cammeterlist, const float* in_x_colorimeterlist,
		                                     AlgoHandle* in_y_cammeterlist, const float* in_y_colorimeterlist,
		                                     AlgoHandle* in_z_cammeterlist, const float* in_z_colorimeterlist, 
		                                     const int in_num, AlgoColor in_color, 
		                                     AlgoPoint* in_point = nullptr, int* in_radius = nullptr);


	/************************************
	@brief  : 相机标定系数K或者Kb值应用，K和Kb应用为同一个接口，内部会判断
	@note   :
	@param  : camcalib_handle           有效的相机标定句柄
	@param  : in_out_meter_handle       输入有效的in_out_meter_handle句柄，输入meter应用色度标定系数K值或Kb值后输出
	@return : AlgoError
	@Author : liying@2023/09/20
	************************************/
	C_API AlgoError C_CALL  CamCalibRectifyKb(AlgoHandle camcalib_handle, AlgoHandle in_out_meter_handle);


    /************************************
    @brief  : 获取句柄内容
    @note   : 
    @param  : camcalib_handle          有效的camcalib句柄
    @param  : out_para                 导出的结构体,必须已经初始化完毕
    @param  : in_is_deep_copy          是否深拷贝, 默认浅拷贝  1=深拷贝;0=浅拷贝
    @return	: AlgoError
    @Author : ouxingtao@2022/04/02
    ************************************/
    C_API AlgoError C_CALL CamCalibGet(AlgoHandle camcalib_handle, AlgoCamCalib* out_camcalib);

    /************************************
    @brief  : 设置内容
    @note   :
    @param  : camcalib_handle          有效的camcalib_handle句柄
    @param  : in_para                  要输入的数据
    @return	: AlgoError
    @Author : ouxingtao@2022/04/02
    ************************************/
    C_API AlgoError C_CALL CamCalibSet(AlgoHandle camcalib_handle, const AlgoCamCalib* in_camcalib);

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
    C_API AlgoError C_CALL CamCalibRelease(AlgoHandle camcalib_handle);
    C_API void C_CALL CamCalibDestroyHandle(AlgoHandle camcalib_handle);

    /************************************
    @brief  : 获取buffer大小
    @note   :
    @param  : camcalib_handle     当前camcalib句柄
    @param  : out_len             buffer大小
    @return	: AlgoError
    @Author : ouxingtao@2022/04/02
    ************************************/
    C_API AlgoError C_CALL CamCalibGetBufferSize(AlgoHandle camcalib_handle, uint64_t* out_len);


    /************************************
    @brief  : 将camcalib里面的数据导出到buffer
    @note   :
    @param  : camcalib_handle     当前camcalib句柄
    @param  : out_buffer          已经申请好申请空间
    @param  : in_out_len          外部申请好的空间大小,会用真实大小修改
    @return	: AlgoError
    @Author : ouxingtao@2022/04/02
    ************************************/
    C_API AlgoError C_CALL CamCalibToBuffer(AlgoHandle camcalib_handle, char* out_buffer, uint64_t* in_out_len);


    /************************************
    @brief  : 将buffer数据导入到camcalib
    @note   :
    @param  : camcalib_handle     有效的camcalib句柄
    @param  : in_buffer
    @param  : in_len
    @return	: AlgoError
    @Author : ouxingtao@2022/04/02
    ************************************/
    C_API AlgoError C_CALL CamCalibFromBuffer(AlgoHandle camcalib_handle, const char* in_buffer, uint64_t in_len);
	
	/************************************
	@brief  : 设置机差滤波参数,机差是指不同相机之间的差异（同一模组经不同设备边缘曲面修正后形态不一致）
	@note   : 机差滤波使用, 默认参数为0.1,6.0,6.0
	@param  : camcalib_handle        -- camcalib_handle句柄
	@param  : ratio		             -- 滤波核的大小占led尺寸的百分比，建议10%
	@param  : in_sigma_x			 -- 水平方向标准差
	@param  : in_sigma_y			 -- 垂直方向标准差
	@Author : dulonghui@2023/11/06
	************************************/
	C_API AlgoError C_CALL CamCalibSetFilterPara(AlgoHandle camcalib_handle, const float ratio, const float sigma_x, const float sigma_y);
	/************************************
	@brief  : 机差系数计算，机差是指不同相机之间的差异（同一模组经不同设备边缘曲面修正后形态不一致）
	@note   : 利用标准设备计算机差系数，计算出的不同颜色的机差系数按XYZ顺序排列
	@param  : camcalib_handle               -- camcalib_handle句柄
	@param  : in_standard_xyzmeter_handle	-- 标准设备：输入同一个颜色的边缘曲面修正后的meter数据，可以输入单个meter，
	                                           一般输入多个meter（同一颜色不同滤片），如果输入两个，只能输入X，Y数据，
	
	@param  : in_xyzmeter_handle		    -- 待修正设备：输入同一个颜色的边缘曲面修正后的meter数据，可以输入单个meter，
	                                           一般输入多个meter（同一颜色不同滤片），如果输入两个，只能输入X，Y数据，
	@param  : in_meter_num                  -- 输入meter数量
	@return	: AlgoError
	@Author : dulonghui@2023/11/06
	************************************/
	C_API AlgoError C_CALL CamCalibCameraDiffCompute(AlgoHandle camcalib_handle, AlgoHandle* in_standard_xyzmeter_handle, 
		AlgoHandle* in_xyz_meter_handle, const int in_meter_num);
	/************************************
	@brief  : 公共机差系数计算，机差是指不同相机之间的差异（同一模组经不同设备边缘曲面修正后形态不一致）
	@note   : 用num组机差系数生成公共机差系数
	@param  : in_camcalib_handles     -- 若干个camcalib_handle句柄
	@param  : in_num		         -- camcalib_handle句柄个数
	@param  : out_camcalib_handle    -- 输出剔除异常系数后的平均camcalib_handle
	@return	: AlgoError
	@Author : dulonghui@2023/11/06
	************************************/
	C_API AlgoError C_CALL CamCalibCameraDiffCoeffAverage(AlgoHandle* in_camcalib_handles,  const int in_num, AlgoHandle out_camcalib_handle);
	/************************************
	@brief  : 利用机差系数修正meter,机差是指不同相机之间的差异（同一模组经不同设备边缘曲面修正后形态不一致）
	@note   : 用机差系数修正meter
	@param  : camcalib_handle        -- camcalib_handle句柄
	@param  : in_out_ledmeter		 -- 输入待修正meter
	@return	: AlgoError
	@Author : dulonghui@2023/11/06
	************************************/
	C_API AlgoError C_CALL CamCalibCameraDiffRectify(AlgoHandle camcalib_handle,  AlgoHandle in_out_ledmeter);
	
	/************************************
    @brief  : 利用机差系数修正曲面系数,机差是指不同相机之间的差异（同一模组经不同设备边缘曲面修正后形态不一致）
    @note   : 用机差系数修正curve,输入不同颜色的曲面系数与对应颜色的机差系数
    @param  : camcalib_handle        -- camcalib_handle句柄
    @param  : in_out_curve		     -- 输入待修正curve
    @return	: AlgoError
    @Author : dulonghui@2023/11/06
    ************************************/
	//C_API AlgoError C_CALL CamCalibCameraDiffRectifyCurve(AlgoHandle camcalib_handle, AlgoHandle in_out_curve);


#ifdef __cplusplus
}
#endif // __cplusplus
#endif

