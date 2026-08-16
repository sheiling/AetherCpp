#ifndef __MARKER_DETECTOR_DATA_H__
#define __MARKER_DETECTOR_DATA_H__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


	
	typedef enum
	{
		MARKER_CIRCLE = 1,				// 圆形模板
		MARKER_CROSS = 2,				// 十字模板
		MARKER_RECT = 3,				// 矩形模板
		MARKER_RING = 4,				// 圆环
		MARKER_FALSE_CROSS = 5,			// 虚十字
		MARKER_RECT_CORNER = 6,			// 矩形角点定位
		MARKER_LINE = 7,			    // 边缘线定位
    }MarkerType;//模板类型


	typedef enum
	{
		Method_CircleProjection = 1,	// 基于圆投影
		Method_MatchShape = 2,			// 基于轮廓形状识别
		Method_MatchGray = 3,			// 基于灰度匹配 ——目前推荐使用
		Method_MatchLocate = 4,			//基于模板匹配定位

	}MarkerDetectMethod;  // 识别和定位的方法


	typedef struct
	{
		int mark_wide;					// mark图像的宽，像素
		int mark_high;					// mark图像的高，像素
		int is_white_mark;				// 黑底白图  = 1， 白底黑图 = 0；

	}MarkerInfo; // 检测阈值

	typedef struct
	{
		float in_area_thresh;			// mark点的最小面积
		float in_low_thresh;			// 阈值下线
		float in_high_thresh;			// 阈值上限

		//marker点轮廓分割阈值，定位方法选择基于模板匹配（Method_MatchLocate）时需要设置
		int segment_thresh;				//marker标识轮廓分割阈值，内部默认为50，
		 
	}MarkerDetectThresh; // 检测阈值


	typedef struct
	{
		float k;						// 直线斜率 k
		float b;						// 直线偏执 b
	}MarkerLineKB; // 拟合出来的直线参数




#ifdef __cplusplus
}
#endif // __cplusplus



#endif