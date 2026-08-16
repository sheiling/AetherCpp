#ifndef __META_INFO_DATA_H__
#define __META_INFO_DATA_H__

#include "ci_data.h"
#include "ci_define.h"
#include "meta_info_enum.h"



#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


	typedef struct
	{
		CameraType type;

		int width;//分辨率
		int height;

		float sensor_w;//mm 像素尺寸 
		float sensor_h;//mm

		int depth;//相机位深
	} CameraInfo;

	typedef struct
	{
		LensType type;
		float focal;//焦距 mm
		//float fov;//视场角 角度
	} LensInfo;



	typedef struct
	{
		//float x;//相机的三个位置
		//float y;
		//float z;

		//相机方向为z,固定相机方向为y,右手坐标系
		//定义为int 方便使用, 单位为角度°
		int pitch;//俯仰角,相机沿着x方向旋转
		int yaw;//偏转角,相机沿着y方向旋转

		//测量目标正方向为0°  右手坐标系
		//roll = 0,相机视野正方向0°
		//roll = 90,//相机视野正方向90°
		//roll = 180,//相机视野正方向180°
		//roll = 270,//相机视野正方向270°
		int roll;//翻滚角,相机沿着z方向旋转
	}CameraPos;



	typedef struct {
		char id[ALGO_DEVICE_STR_LEN];//设备id
		CameraType cam_type;
		char lens_id[ALGO_DEVICE_STR_LEN];//镜头编号
		LensType lens_type;//镜头类型
	} Device;


	typedef struct {
		int exposure;//ms
		int gain;//相机增益
		int aperture;//光圈
		int microfocus;//微焦，单位mm
		CamFilter filter;//XYZ哪个滤片, 数码相机为 FILTER_XYZ
	} Capture;

	typedef struct {
		int index;//layer 索引
		float linear_norms;//多层归一化的线性亮度百分比，范围0 - 1.f, pos
		//float reduction;//亮度衰减 0-1.f  
		int value;//layer打屛亮度
	} Layer;


	typedef struct {
		LedPanelType type;//LED面板类型
		LedLayout layout;//LED灯点排列类型
		LedSubpixelType subpixel_type;//LED灯点RGB亚像素排列类型
		AlgoSize wh;//灯板大小
		float pixel_pitch;//灯点间距mm
	} LedPanel;


	typedef struct {
		PatternType type;//隔点类型
		AlgoSize interval;//隔点宽高
		AlgoPoint loc;//loc= xy, 点亮pattern中的哪个点;例如4x4隔点,点亮第4行第3列,loc=2,3
		AlgoSize lighted_led_wh;//点亮灯点的行数和列数
	} Pattern;

	typedef struct {
		AlgoRect xywh;//xywh=分区rect
		AlgoSize screen_wh;//屏幕大小
	}  Partition;


	
#ifdef __cplusplus
}
#endif // __cplusplus

#endif // 
