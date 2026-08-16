#ifndef __CHIP_DETECTOR_DATA_H__
#define __CHIP_DETECTOR_DATA_H__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


    //我们定义
    typedef enum
    {
        PANEL_CHIP_R    = 1,
		PANEL_CHIP_G    = 2,
		PANEL_CHIP_B    = 3,
		PANEL_CHIP_MIP  = 4,

    }ChipDetectRGBType;//芯片类型



	typedef struct
	{
		float in_match_thresh;
		float in_low_thresh;
		float in_high_thresh;
		float in_area_thresh;

	}ChipDetectThresh;


	typedef enum
	{
		PANEL_CHIP_RGB_H  = 1,
		PANEL_CHIP_BGR_H  = 2,
		PANEL_CHIP_RGB_V  = 3, 
		PANEL_CHIP_BGR_V  = 4,
		PANEL_CHIP_SINGLE = 5,
		PANEL_CHIP_COG    = 6,

	}ChipDetectRGBAlignType;//芯片排列类型



#ifdef __cplusplus
}
#endif // __cplusplus



#endif