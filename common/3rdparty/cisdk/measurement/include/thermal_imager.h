
#ifndef __THERMAL_IMAGER_H__
#define __THERMAL_IMAGER_H__

#include "ci_define.h"
#include "ci_declare.h"
#include "ci_measurement_enum.h"
#include "thermal_imager_data.h"
#include "thermal_imager_errors.h"


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	//初始化
    C_API AlgoError C_CALL ThermalImagerCreateHandle(AlgoHandle* out_thermalimager_handle);

	/************************************
	@Brief		：	设置thermalimager中的temperatue数据
	@Note		：  
	@Parameter	: 	thermalimager_handle		--thermalimager句柄
	@Parameter	: 	in_temperature		        --set的数据，需要软件端自己构造AlgoMat
	@Returns	:   AlgoError
	@Author		：	zhangxuan@2025/06/20
	************************************/
	C_API AlgoError C_CALL ThermalImagerSetImage(AlgoHandle thermalimager_handle, const AlgoMat* in_temperature);

	/************************************
	@Brief		：	直接从内存构建thermalimager
	@Note		：  可按照in_roi_rect大小读取数据，
	                当in_roi_rect={0，0，0，0}时读取的是热成像仪分辨率大小（该情况下图像大小与热成像仪分辨率一致）
					其余正常rect下，读取的是rect大小（该情况下图像大小与rect一致）
	@Parameter	: 	thermalimager_handle		--thermalimager句柄
	@Parameter	: 	in_p_mem      		        --图像指针
	@param      :   in_camera_resolotion        --相机分辨率
	@param      :   in_type                     --图像类型
	@param      :   in_roi_rect                 --Rect信息
	@Returns	:   AlgoError
	@Author		：	zhangxuan@2025/06/20
	************************************/
	C_API AlgoError C_CALL ThermalImagerCreateFromMem(AlgoHandle thermalimager_handle, const char* in_p_mem, const AlgoSize in_camera_resolotion, 
		AlgoMatType in_type, const AlgoRect in_roi_rect = {0,0,0,0});

	/************************************
	@Brief		：	读取raw数据到thermalimager
	@Note		：  可按照in_roi_rect大小读取数据，
	                当in_roi_rect={0，0，0，0}时读取的是热成像仪分辨率大小（该情况下图像大小与热成像仪分辨率一致）
					其余正常rect下，读取的是rect大小（该情况下图像大小与rect一致）
	@Parameter	: 	in_thermalimager_handle		--thermalimager句柄
	@Parameter	: 	in_raw_fname      		    --raw数据文件
	@param      :   in_camera_resolotion        --热成像仪分辨率
	@param      :   in_roi_rect                 --Rect信息
	@Returns	:   AlgoError
	@Author		：	zhangxuan@2025/06/20
	************************************/
	C_API AlgoError C_CALL ThermalImagerReadRaw(AlgoHandle thermalimager_handle, const char* in_raw_fname, const AlgoSize in_camera_resolotion, 
		const AlgoRect in_roi_rect = {0,0,0,0});
	
	/************************************
	@Brief		：	读取csv数据到thermalimager
	@Note		：  可按照in_roi_rect大小提取目标区域的数据
					当in_roi_rect={0，0，0，0}时读取的是热成像仪分辨率大小
					其余正常rect下，读取的是rect大小（该情况下图像大小与rect一致）
	@Parameter	: 	thermalimager_handle		--thermalimager句柄
	@Parameter	: 	in_csv_fname      		    --csv数据文件
	@param      :   in_camera_resolotion        --热成像仪分辨率
	@param      :   in_roi_rect                 --Rect信息
	@Returns	:   AlgoError
	@Author		：	zhangxuan@2025/06/20
	************************************/
	C_API AlgoError C_CALL ThermalImagerReadCsv(AlgoHandle thermalimager_handle, const char* in_csv_fname, const AlgoSize in_camera_resolotion,
		const AlgoRect in_roi_rect = { 0,0,0,0 });
	
	/************************************
	@Brief		：	设置温度数据的roi_rect信息
	                
	@Note		：  当设置了roi_rect，后续set的img或之前set的img的wh必须和roi_rect的wh一致。
	@Parameter	: 	thermalimager_handle		--thermalimager句柄
	@Parameter	: 	in_roi_rect      		    --roi_rect区域
	@Returns	:   AlgoError
	@Author		：	zhangxuan@2025/06/20
	************************************/
	C_API AlgoError C_CALL  ThermalImagerSetRoiRect(AlgoHandle thermalimager_handle, const AlgoRect in_roi_rect);

	/************************************
    @Brief		：	计算矩形区域均值
    @Note		：  roi_rect=0时，计算全局均值
    @Parameter	: 	thermalimager_handle		--thermalimager句柄
    @Parameter	: 	in_roi_rect      		    --矩形区域信息
    @param      :   out_mean                    --输出的矩形区域均值
    @Returns	:   AlgoError
    @Author		：	zhangxuan@2025/06/20
    ************************************/
	C_API AlgoError C_CALL ThermalImagerGetRoiAverage(AlgoHandle thermalimager_handle, float* out_mean, const AlgoRect in_roi_rect = {0,0,0,0});
	


	/************************************
	@Brief		：	计算圆形区域均值
	@Note		：  
	@Parameter	: 	thermalimager_handle		--thermalimager句柄
	@Parameter	: 	in_center      		        --圆心坐标
	@Parameter	: 	in_radius      		        --半径
	@param      :   out_mean                    --输出的圆形区域均值
	@Returns	:   AlgoError
	@Author		：	zhangxuan@2025/06/20
    ************************************/
	C_API AlgoError C_CALL ThermalImagerGetCircleRoiAverage(AlgoHandle thermalimager_handle, const AlgoPoint in_center, const int in_radius, float* out_mean);


	/************************************
	@Brief		：	设置角点坐标
	@Note		：  该角点是图像中目标对象所在区域的四个原始角点
	@Parameter	: 	thermalimager_handle		--thermalimager句柄
	@Parameter	: 	in_corner4      		    --变换前的原始角点
	@Parameter	: 	in_num      		        --角点个数，必须=4
	@Returns	:   AlgoError
	@Author		：	zhangxuan@2025/06/20
	************************************/
	C_API AlgoError C_CALL ThermalImagerSetCorner(AlgoHandle thermalimager_handle, const AlgoPoint* in_corner4, int in_num);

	/************************************
	@Brief		：	设置透视变换矩阵
	@Note		：  
	@Parameter	: 	thermalimager_handle		--thermalimager句柄
	@Parameter	: 	in_transform_handle 		--有效的transform句柄
	@Returns	:   AlgoError
	@Author		：	zhangxuan@2025/06/20
	************************************/
	C_API AlgoError C_CALL ThermalImagerSetTransform(AlgoHandle thermalimager_handle, const AlgoHandle in_transform_handle);
	
	/************************************
	@Brief		：	温度数据处理
	@Note		：  最终提取出的数据与当前屏体分区的分辨率一致(必须设置part)
	                最终改变handle中的temperatue内容，大小以及ThermalImagerDataType=ThermalImagerDataType_FEATURE
	@Parameter	: 	thermalimager_handle		--thermalimager句柄
	@Returns	:   AlgoError
	@Author		：	zhangxuan@2025/06/20
	************************************/
	C_API AlgoError C_CALL ThermalImagerCompute(AlgoHandle thermalimager_handle);


	/************************************
	@Brief		：	设置温度采集设备分辨率
	@Note		：  即所采集的图像的分辨率
	@Parameter	: 	thermalimager_handle		--thermalimager句柄
	@Parameter	: 	in_resolotion      		    --分辨率
	@Returns	:   AlgoError
	@Author		：	zhangxuan@2025/06/20
	************************************/
	C_API AlgoError C_CALL ThermalImagerSetCameraResolotion(AlgoHandle thermalimager_handle, const AlgoSize in_resolotion);

	/************************************
	@Brief		：	设置温度数据的颜色信息
	@Note		：  
	@Parameter	: 	thermalimager_handle		--thermalimager句柄
	@Parameter	: 	in_color      		        --颜色
	@Returns	:   AlgoError
	@Author		：	zhangxuan@2025/06/20
	************************************/
	C_API AlgoError C_CALL  ThermalImagerSetColor(AlgoHandle thermalimager_handle, const AlgoColor in_color);

	/************************************
	@Brief		：	设置温度数据的part信息
	@Note		：  调用compute时必须设置该信息，内部上采样时需要part.xywh值
	@Parameter	: 	thermalimager_handle		--thermalimager句柄
	@Parameter	: 	in_part      		        --part信息
	@Returns	:   AlgoError
	@Author		：	zhangxuan@2025/06/20
	************************************/
	C_API AlgoError C_CALL  ThermalImagerSetPartition(AlgoHandle thermalimager_handle, const Partition* in_part);

	/************************************
	@Brief		：	设置温度数据的layer信息
	@Note		：  
	@Parameter	: 	thermalimager_handle		--thermalimager句柄
	@Parameter	: 	in_layer      		        --layer信息
	@Returns	:   AlgoError
	@Author		：	zhangxuan@2025/06/20
	************************************/
	C_API AlgoError C_CALL   ThermalImagerSetLayer(AlgoHandle thermalimager_handle, const Layer* in_layer);

	/************************************
	@Brief		：	设置温度数据的device信息
	@Note		：
	@Parameter	: 	thermalimager_handle		--thermalimager句柄
	@Parameter	: 	in_device      		        --Device信息
	@Returns	:   AlgoError
	@Author		：	zhangxuan@2025/06/20
	************************************/
	C_API AlgoError C_CALL   ThermalImagerSetDevice(AlgoHandle thermalimager_handle,	const Device* in_device);
	
	/************************************
	@Brief		：	设置温度数据的data_type信息
	@Note		：
	@Parameter	: 	thermalimager_handle		--thermalimager句柄
	@Parameter	: 	in_data_type      		    --in_data_type信息
	@Returns	:   AlgoError
	@Author		：	zhangxuan@2025/06/20
	************************************/
	C_API AlgoError C_CALL   ThermalImagerSetDataType(AlgoHandle thermalimager_handle, const ThermalImagerDataType in_data_type);

	/************************************
	@Brief		：	设置温度数据的cie_type信息
	@Note		：
	@Parameter	: 	thermalimager_handle		--thermalimager句柄
	@Parameter	: 	in_cie_type      		    --cie_type信息
	@Returns	:   AlgoError
	@Author		：	zhangxuan@2025/06/20
	************************************/
	C_API AlgoError C_CALL   ThermalImagerSetCIEType(AlgoHandle thermalimager_handle, const CIEType in_cie_type);

	/************************************
	@Brief		：	载入thermalimager文件
	@Note		：  后缀为：“.tempmap”
	@Parameter	: 	thermalimager_handle		--thermalimager句柄
	@Parameter	: 	in_fullname      		    --载入文件路径
	@Returns	:   AlgoError
	@Author		：	zhangxuan@2025/06/20
	************************************/
	C_API AlgoError C_CALL ThermalImagerLoad(AlgoHandle thermalimager_handle, const char* in_fullname);
	
	/************************************
	@Brief		：	保存thermalimager文件
	@Note		：  后缀为：“.tempmap”
	@Parameter	: 	thermalimager_handle		--thermalimager句柄
	@Parameter	: 	in_fullname      		    --保存文件路径
	@Returns	:   AlgoError
	@Author		：	zhangxuan@2025/06/20
	************************************/
	C_API AlgoError C_CALL ThermalImagerSave(AlgoHandle thermalimager_handle, const char* in_fullname);

	/************************************
    @brief      : 获取ThermalImage内容
    @note       : 
    @param      : thermalimager_handle 		--thermalimager句柄
    @param      : out_thermalimager             --导出的结构体,必须已经初始化完毕
    @param      : is_deep_copy                  --是否深拷贝, 默认浅拷贝  1=深拷贝;0=浅拷贝
    @return  	: AlgoError
    @Author     : zhangxuan@2025/06/20
    ************************************/
	C_API AlgoError C_CALL ThermalImagerGet(AlgoHandle thermalimager_handle, AlgoThermalImager* out_thermalimager, int in_is_deep_copy = 0);

	/************************************
	@brief      : 获取ThermalImage内容
	@note       :
	@param      : thermalimager_handle 		--thermalimager句柄
	@param      : in_thermalimager          --待set的结构体
	@param      : is_deep_copy              --是否深拷贝, 默认浅拷贝  1=深拷贝;0=浅拷贝
	@return  	: AlgoError
	@Author     : zhangxuan@2025/06/20
	************************************/
	C_API AlgoError C_CALL ThermalImagerSet(AlgoHandle thermalimager_handle,const AlgoThermalImager* in_thermalimager);

	/************************************
    @brief  : 将温度数据导出成图片
    @note   :
    @param  : thermalimager_handle       --有效的thermalimager句柄
    @param  : in_img_fullname            --保存路径，支持png，jpg，bmp格式
	@param  : in_exportimagetype         --保存图像是热图还是灰度图
    @return	: AlgoError
    @Author : zhangxuan@2025/11/13
    ************************************/
	C_API AlgoError C_CALL ThermalImagerExportImage(AlgoHandle thermalimager_handle, const char* in_img_fullname,const ExportImageType in_exportimagetype= ExportImageType::ExportImageType_HotPic);


	/************************************
	@brief      : 将ThermalImager中的数据到处到buffer
	@note       :
	@param      : thermalimager_handle 		 --thermalimager句柄
	@param      : out_buffer                     --已经申请好申请空间
	@param      : in_out_len                     --*in_out_len外部申请好的空间大小,会用真实大小修改
	@return	    : AlgoError
	@Author     : zhangxuan@2025/06/20
	************************************/
	C_API AlgoError C_CALL ThermalImagerToBuffer(AlgoHandle thermalimager_handle, char* out_buffer, uint64_t* in_out_len);


	/************************************
	@brief      : 将buffer数据导入到ThermalImager
	@note       :
	@param      : thermalimager_handle 		 --thermalimager句柄
	@param      : in_buffer                     
	@param      : in_len                      
	@return	    : AlgoError
	@Author     : zhangxuan@2025/06/20
	************************************/
	C_API AlgoError C_CALL ThermalImagerFromBuffer(AlgoHandle thermalimager_handle, char* in_buffer, uint64_t in_len);


	/************************************
	@brief      : 获取buffer大小
	@note       :
	@param      : thermalimager_handle 		 --thermalimager句柄
	@param      : out_len                        --buffer大小
	@return	    : AlgoError
	@Author     : zhangxuan@2025/06/20
	************************************/
	C_API AlgoError C_CALL ThermalImagerGetBufferSize(AlgoHandle thermalimager_handle, uint64_t* out_len);

	/************************************
	@Brief		：	往operate_buffer中写入长度为in_info_len的业务操作记录数据
	@Note		：
	@Parameter	: 	thermalimager_handle		--thermalimager句柄
	@Parameter	: 	in_op_info		            --输入业务操作记录数据
	@Parameter	: 	in_info_len		            --输入业务操作记录数据的长度，不能超过ThermalImager_OPERATE_BUFFER_SIZE
	@Returns	:   AlgoError
	@Author		：	zhangxuan@2025/06/20
	************************************/
	C_API AlgoError C_CALL ThermalImagerSetOperateBuffer(AlgoHandle thermalimager_handle, const char* in_op_info, size_t in_info_len);

	/************************************
	@Brief		：	从operate_buffer中读取全部的业务操作记录数据
	@Note		：  每次固定读取ThermalImager_OPERATE_BUFFER_SIZE大小的数据
	@Parameter	: 	thermalimager_handle		--thermalimager句柄
	@Parameter	: 	out_op_info		            --业务操作记录数据读到外部内存段
	@Parameter	: 	in_info_len		            --外部内存段长度，不能小于ThermalImager_OPERATE_BUFFER_SIZE，默认为ThermalImager_OPERATE_BUFFER_SIZE
	@Returns	:   AlgoError
	@Author		：	zhangxuan@2025/06/20
	************************************/
	C_API AlgoError C_CALL ThermalImagerGetOperateBuffer(AlgoHandle thermalimager_handle, char* out_op_info, size_t in_info_len = ThermalImager_OPERATE_BUFFER_SIZE);




	/************************************
    @Brief		：	将thermalimager数据释放
    @Note		：
    @Parameter	: 	thermalimager_handle		--thermalimager句柄
    @Returns	:   AlgoError
    @Author		：	zhangxuan@2025/06/20
    ************************************/
	C_API AlgoError C_CALL ThermalImagerRelease(AlgoHandle thermalimager_handle);

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
	C_API void C_CALL ThermalImagerDestroyHandle(AlgoHandle thermalimager_handle);

#ifdef __cplusplus
};
#endif // __cplusplus
#endif
