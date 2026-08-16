#ifndef __ALGO_SHOT_DATA_H__
#define __ALGO_SHOT_DATA_H__


#include "meta_info_data.h"
#include "algo_mat_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	typedef struct {
		Partition* part;//分区信息
		LedPanel* panel;//面板信息
		Layer* layer;//层信息
		Pattern* pattern;//打点信息
		AlgoColor* color;//打屛颜色
		Device* device;//设备信息
		Capture* capture;//拍摄信息

		//CamPos* cam_pos;//相机位置
		AlgoRect rect;//image的位置信息
		CIEType type;//色度分量类型
		AlgoMat image; //ALGO_16UC1 
	}  AlgoShot;


	enum ShotDetectType
	{
		ShotDetectType_Bg = 0,  //对背景图进行检测，查看是否有异常亮灯

	};

	enum ShotDetectResult
	{
		ShotDetectResult_OK = 0,  //结果ok
		ShotDetectResult_ABNORMAL = 1,  //结果异常
	};

#define SHOT_OPERATE_BUFFER_SIZE 1048576 // 1MB=1024*1024

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // 
