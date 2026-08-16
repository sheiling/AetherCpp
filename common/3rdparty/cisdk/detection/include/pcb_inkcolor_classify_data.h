#ifndef __PCB_INKCOLOR_CLASSIFY_DATA_H__
#define __PCB_INKCOLOR_CLASSIFY_DATA_H__

#include "algo_mat_data.h"
#include "meta_info_data.h"
#include <opencv2/imgproc/types_c.h>
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	typedef enum
	{
		CS_CAM_RGB = 0,
		CS_CIE_Lch = 1,
		CS_CIE_Lab = 2,
		CS_CIE_XYZ = 3,
	}ColorSpace; //需要计算的颜色类型

		
	typedef enum
	{
		OFFCalibrate = 0,
		ONCalibrate = 1,
	}ColorCalibrate; //是否进行光源补偿


	 //变量缩合之后的值
	typedef struct
	{
		float brightness;  //  L / XYZ空间的 Y  /R   
		float color1;  // c / a  / X  /G
		float color2; // h / b  /  z  /B
		float panel_blance;

	}InkColorFeature;

	typedef struct
	{
		float brightness_th;  
		float color1_th;  
		float color2_th; 
		float panel_blance_th;
		float compresse_chroma_th; // C * H 得到的色度
	}UniformThresh;


	typedef enum
	{
		Brightness = 0,
		Color1 = 1,
		Color2 = 2,
		PanelBlance = 3, 
		CompresseChroma = 4, // C * H 得到的色度
	}ClassifyMethod;


	typedef struct
	{
		int cam_id;
		cv::Mat panel_mask;
		std::vector<cv::Point> panel_corner = std::vector<cv::Point>(4, {0,0});
	}PanelRoiInfo;


	typedef struct
	{
		int cam_id;
		float pad_area_thresh = 0.8f;
		float pad_area;
		int pad_width;
		int pad_height;
	}PadAreaInfo;



#ifdef __cplusplus
}
#endif // __cplusplus
#endif