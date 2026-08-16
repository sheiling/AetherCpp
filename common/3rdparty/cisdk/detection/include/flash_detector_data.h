#ifndef __FLASH_DETECTOR_DATA_H__
#define __FLASH_DETECTOR_DATA_H__


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	typedef struct
	{
		float screen_th;    // 闪屏检测阈值，前后两帧图像均值差异，  默认0.01，即灰度差10% 
		float strip_th;     // 闪线阈值，闪烁区域与周围区域的像素差异，默认0.1，即灰度差10% 
		float point_gray_coefficient_th; // 闪点检测，闪烁区域与周围的像素差异，默认0.3, 即灰度差30%  
		int point_num_th;   // 闪点的像素数量， 默认50
	}DetectThresh;



	// 灯板信息
	typedef struct
	{
		int panel_num;			// 灯板数量
		int panel_rows;			// 灯板行数
		int panel_cols;			// 灯板列数
		int led_point_rows;		// LED灯珠的行数
		int led_point_cols;     // LED灯珠的列数
		int point_gap_x;		// 灯板x方向点间距
		int point_gap_y;		// 灯板Y方向点间距
		float back_lum;			// 灯板背景亮度 
		float led_lum;			// 灯板灯板前景亮度
	}PanelInfo;




#ifdef __cplusplus
}
#endif // __cplusplus

#endif