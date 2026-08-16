#ifndef __SURFACE_TEMPERATURE_CALIB_H__
#define __SURFACE_TEMPERATURE_CALIB_H__


#include "ci_define.h"
#include "ci_declare.h"
#include "surface_temperature_calib_data.h"
#include "surface_temperature_calib_error.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

    /************************************
    @brief  : 创建surface_temp_calib句柄
    @note   :
    @param  : out_surface_temp_calib_handle   -- surface_temp_calib句柄
    @return	: AlgoError
    @Author : ouxingtao@2021/12/22
    ************************************/
    C_API AlgoError C_CALL SurfaceTemperatureCalibCreateHandle(AlgoHandle* out_surface_temp_calib_handle);
	
	/************************************
    @brief  : 设置温度补偿标定模式
    @note   :
    @param  : surface_temp_calib_handle  --有效的surface_temp_calib句柄
    @param  : in_calib_type              --标定模式
	                                         --SurfaceTempCalibType_Point = 0,  //逐点标定系数(不调用该接口时，默认该模式)
		                                     --SurfaceTempCalibType_Global = 1,  //Global标定系数
		                                     --SurfaceTempCalibType_Cabinet = 2,  //单箱复用标定系数(全屏校正推荐模式)
		                                     --SurfaceTempCalibType_Module = 3,  //单模组复用标定系数
    @return	: AlgoError
    @Author : zhangxuan@2025/06/20
    ************************************/
	C_API AlgoError C_CALL SurfaceTemperatureCalibSetCalibType(AlgoHandle surface_temp_calib_handle, SurfaceTempCalibType in_calib_type);

	/************************************
	@brief  : 设置校正基准温度
	@note   : 校正基准温度应与校正模式对应
	@param  : surface_temp_calib_handle  --有效的surface_temp_calib句柄
	@param  : in_tar_temperature         --校正基准温度(thermal_imager句柄)
	@return	: AlgoError
	@Author : zhangxuan@2025/06/20
	************************************/
	C_API AlgoError C_CALL SurfaceTemperatureCalibSetTemperature(AlgoHandle surface_temp_calib_handle,const AlgoHandle in_tar_thermal_imager);
	
	/************************************
	@brief  : 设置校正基准温度
	@note   : 重构前接口为SurfaceTemperatureCalibSetTemperataure
	@param  : surface_temp_calib_handle  有效的surface_temp_calib句柄
	@param  : in_calib_temperature       校正基准温度--float
	@return	: AlgoError
	@Author : zhangxuan@2025/06/20
	************************************/
	C_API AlgoError C_CALL SurfaceTemperatureCalibSetGlobalTemperature(AlgoHandle surface_temp_calib_handle, const float in_calib_temperature);

	/************************************
	@brief  : 设置单箱分辨率
	@note   : SurfaceTempCalibType==SurfaceTempCalibType_Cabinet时使用
	@param  : surface_temp_calib_handle  --有效的surface_temp_calib句柄
	@param  : in_tar_temperature         --校正基准温度(thermal_imager句柄)
	@return	: AlgoError
	@Author : zhangxuan@2025/06/20
	************************************/
	C_API AlgoError C_CALL SurfaceTemperatureCalibSetCabinetSize(AlgoHandle surface_temp_calib_handle, const AlgoSize in_cabinet_size);

	/************************************
    @brief  : 温度补偿系数计算
    @note   : 替换重构前版本的SurfaceTemperatureCalibCompute接口
    @param  : surface_temp_calib_handle  --有效的surface_temp_calib句柄
    @param  : in_color                   --颜色信息
    @return	: in_xyz                     --三刺激值XYZ信息, TriXYZ型指针数组
    @return	: in_temperature             --温度信息，float型指针数组，其长度需与in_xyz相同
    @return	: in_len                     --用来运算的数据长度，其小于等于in_xyz和in_temperature的长度
    @Author :  zhangxuan@2025/06/20
    ************************************/
	C_API AlgoError C_CALL SurfaceTemperatureCalibComputeTriXYZ(AlgoHandle surface_temp_calib_handle, const AlgoColor in_color, const TriXYZ* in_xyz,
		const float* in_temperature, const int in_len);
	
	/************************************
	@brief  : 温度补偿系数计算
	@note   : 温度数据与XYZ数据均为逐点矩阵数据,单个颜色进行计算，同时输出单色的XYZ系数
	          注意温度数据in_temperature需要set正确的color,part信息
			  meter需要设置正确的color,part,panel信息，与温度数据的信息保持一致
			  RZ的值可以为空，但必须creat同长度的handle
	@param  : surface_temp_calib_handle  --有效的surface_temp_calib句柄
	@param  : in_meter_x                 --X滤片meter
	@param  : in_temperature_x           --X滤片对应温度（通常情况下X,Y,Z滤片的温度一致）
	@param  : in_meter_y                 --Y滤片meter
	@param  : in_temperature_y           --Y滤片对应温度（通常情况下X,Y,Z滤片的温度一致）
	@param  : in_meter_z                 --Z滤片meter
	@param  : in_temperature_z           --Z滤片对应温度（通常情况下X,Y,Z滤片的温度一致）
	@return	: in_len                     --用来运算的数据长度，其小于等于in_xyz和in_temperature的长度
	@Author : zhangxuan@2025/06/20
	************************************/
	C_API AlgoError C_CALL SurfaceTemperatureCalibCompute(AlgoHandle surface_temp_calib_handle, const AlgoHandle* in_meter_x, const AlgoHandle* in_temperature_x,
		const AlgoHandle* in_meter_y, const AlgoHandle* in_temperature_y, const AlgoHandle* in_meter_z, const AlgoHandle* in_temperature_z, const int in_z_len);

	/************************************
	@brief  : 设置修正方式
	@note   : 默认使用色度不变修正，及认为温度变化对色坐标无影响（适应于环境温度变化不大的情况）。
	@param  : surface_temp_calib_handle   --有效的surface_temp_calib句柄
	@param  : rectify_type_in             --修正方式，可选色度不变修正或色度改变修正。
	@return	: AlgoError
	@Author : tianbo@2024/05/13
	************************************/
	C_API AlgoError C_CALL SurfaceTemperatureCalibSetRectifyType(AlgoHandle surface_temp_calib_handle, SurfaceTempRectifyType in_rectify_type);


	/************************************
    @brief  : 温度补偿应用--rgb
    @note   : 替换重构前版本的SurfaceTemperatureCalibRectify接口
    @param  : surface_temp_calib_handle   --有效的surface_temp_calib句柄
    @param  : in_color                    --颜色信息
    @param  : in_act_temperature          --实际温度，float型指针
    @param  : in_out_xyz                  --待修正的三刺激值XYZ, TriXYZ型指针
    @return	: AlgoError
    @Author : zhangxuan@2025/06/20
    ************************************/
	C_API AlgoError C_CALL SurfaceTemperatureCalibRectifyTriXYZ(AlgoHandle surface_temp_calib_handle, const AlgoColor in_color,const float* in_act_temperature, 
		TriXYZ* in_out_xyz);


	/************************************
	@brief  : 温度补偿应用--rgb
	@note   : 可以修正同一颜色任意个数任意滤片的数据
	@param  : surface_temp_calib_handle   --有效的surface_temp_calib句柄
	@param  : in_act_temperature          --实际温度()
	@param  : in_temperature_num          --in_act_temperature的长度；
	                                        注意：当in_temperature_num=1时，in_meter_num可以!=1,所有meter的实际温度相同，
											当in_temperature_num!=1时，必须使in_temperature_num==in_meter_num,meter的实际温度使用对应索引的温度数据
	@param  : in_out_meter                --待修正的meter（颜色相同，滤片属于XYZ即可）
	@param  : in_meter_num                --待修正的meter的长度,>=1
	@return	: AlgoError
	@Author : zhangxuan@2025/06/20
	************************************/
	C_API AlgoError C_CALL SurfaceTemperatureCalibRectify(AlgoHandle surface_temp_calib_handle,const AlgoHandle* in_act_temperature, 
		const int in_temperature_num,AlgoHandle* in_out_meter,const int in_meter_num);

	/************************************
	@brief  : 温度补偿应用--W
	@note   : 每次只能处理一组W_XYZmeter
	@param  : surface_temp_calib_handle   --有效的surface_temp_calib句柄
	@param  : in_act_temperature          --白色的实际温度(XYZ使用同一温度)
	@param  : in_w_tar_temperature        --白色的目标校正温度，输入为空时，默认使用红色的温度
    @param  : in_rgb_xyz_target_meter     --目标校正温度下的RGB_XYZ_meter(可以通过补偿后的RGB数据得到，与w_meter分辨率一致)
	@param  : in_num_rgb                  --in_rgb_xyz_target_meter的长度=9
	@param  : in_rgb_surf_temp            --rgb温度补偿系数
	@param  : in_out_w_xyz_meter          --w待修正数据
	@param  : in_num_w                    --in_out_w_xyz_meter的长度==3
	@param  : in_rectify_type             --修正方式
	@return	: AlgoError
	@Author : zhangxuan@2025/06/20
	************************************/
	//RectifyWhite(ThermalImage in_act_temperature, std::vector<LedMeter*> in_rgb_xyz_target_meter, std::vector<SurfaceTemperature*> in_rgb_surf_temp,
	//SurfaceTempRectifyType in_rectify_type, std::vector<LedMeter*> in_out_w_xyz_meter, ThermalImage in_w_tar_temperature);
	C_API AlgoError C_CALL SurfaceTemperatureCalibRectifyWhite(const AlgoHandle in_act_temperature, const AlgoHandle in_w_tar_temperature, 
		const AlgoHandle* in_rgb_xyz_target_meter, const int in_num_rgb,
		const AlgoHandle* in_rgb_surf_temp, AlgoHandle* in_out_w_xyz_meter,const int in_num_w, 
		const SurfaceTempRectifyType in_rectify_type = SurfaceTempRectifyType_ChangedChroma);


   /////////////////////////////////////////////////////////////

   

    /************************************
    @brief  : 读取保存模组表面温度补偿系数
    @note   : 后缀只能是.surf_temp
    @param  : surface_temp_calib_handle        有效的surface_temp_calib句柄
    @param  : in_surface_temp_calib_fullname   标定文件,后缀只能是.surf_temp
    @return : AlgoError
    @Author : ouxingtao@2021/12/22
    ************************************/
    C_API AlgoError C_CALL SurfaceTemperatureCalibSave(AlgoHandle surface_temp_calib_handle, const char* in_surface_temp_calib_fullname);
    C_API AlgoError C_CALL SurfaceTemperatureCalibLoad(AlgoHandle surface_temp_calib_handle, const char* in_surface_temp_calib_fullname);


    /************************************
    @brief  : 获取句柄内容
    @note   :
    @param  : surface_temp_calib_handle  有效的surface_temp_calib句柄
    @param  : out_para                   导出的结构体,必须已经初始化完毕
    @param  : in_is_deep_copy            是否深拷贝, 默认浅拷贝  1=深拷贝;0=浅拷贝
    @return : AlgoError
    @Author : ouxingtao@2021/12/22
    ************************************/
    C_API AlgoError C_CALL SurfaceTemperatureCalibGet(AlgoHandle surface_temp_calib_handle, AlgoSurfTempCalib* out_para, int in_is_deep_copy = 0);


    /************************************
    @brief  : 将surface_temp_calib里面的数据导出到buffer
    @note   :
    @param  : surface_temp_calib_handle  有效的surface_temp_calib句柄
    @param  : out_buffer                 已经申请好申请空间
    @param  : in_out_len                 外部申请好的空间大小,会用真实大小修改
    @return	: AlgoError
    @Author : ouxingtao@2021/12/22
    ************************************/
    C_API AlgoError C_CALL SurfaceTemperatureCalibToBuffer(AlgoHandle surface_temp_calib_handle, char* out_buffer, uint64_t* in_out_len);


    /************************************
    @brief  : 获取buffer大小
    @note   :
    @param  : surface_temp_calib_handle  当前surface_temp_calib句柄
    @param  : out_len                    buffer大小
    @return	: AlgoError
    @Author : ouxingtao@2021/12/22
    ************************************/
    C_API AlgoError C_CALL SurfaceTemperatureCalibGetBufferSize(AlgoHandle surface_temp_calib_handle, uint64_t* out_len);


    /************************************
    @brief  : 将buffer数据导入到surface_temp_calib
    @note   :
    @param  : surface_temp_calib_handle  有效的surface_temp_calib句柄
    @param  : in_buffer
    @param  : in_len
    @return	: AlgoError
    @Author : ouxingtao@2021/12/22
    ************************************/
    C_API AlgoError C_CALL SurfaceTemperatureCalibFromBuffer(AlgoHandle surface_temp_calib_handle, const char* in_buffer, uint64_t in_len);


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
    C_API AlgoError C_CALL  SurfaceTemperatureCalibRelease(AlgoHandle surface_temp_calib_handle);
    C_API void C_CALL SurfaceTemperatureCalibDestroyHandle(AlgoHandle surface_temp_calib_handle);


#ifdef __cplusplus
}
#endif // __cplusplus
#endif