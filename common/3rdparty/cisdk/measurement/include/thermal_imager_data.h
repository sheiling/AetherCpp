#ifndef __THERMAL_IMAGER_DATA_H__
#define __THERMAL_IMAGER_DATA_H__


#include "meta_info_data.h"
#include "algo_mat_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
	typedef enum 
	{
		ThermalImagerDataType_RAW = 0x00000000,//检查nullptr 
		ThermalImagerDataType_FEATURE = 0x00000001,//检查nullptr 
	}ThermalImagerDataType;

	

	typedef struct {
		AlgoMat temperatue; // 数据信息
		AlgoRect rect;//temperatue的位置信息
		AlgoSize camera_resolotion;//分辨率信息
		AlgoColor* color;//打屛颜色
		CIEType cie_type;//色度分量类型
		Partition* part;//分区信息
		Layer* layer;//灰阶层信息
		Device* device;
		ThermalImagerDataType data_type;
	}AlgoThermalImager;

	
#define ThermalImager_OPERATE_BUFFER_SIZE 1048576 // 1MB=1024*1024


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // 
