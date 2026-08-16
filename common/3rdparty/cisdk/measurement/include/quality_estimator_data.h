#ifndef __QUALITY_ESTIMATOR_DATA_H__
#define __QUALITY_ESTIMATOR_DATA_H__

#include "algo_mat_data.h"
#include "meta_info_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


	typedef struct
	{
		// 质量评估的各维度，格式都是 ALGO_32FC1 
		AlgoMat pt_temp_h_amat;
		AlgoMat pt_temp_w_amat;
		AlgoMat pt_fwhm_amat;
		AlgoMat pt_hw_dutyratio_amat;
		AlgoMat pt_avg_val_amat;
		AlgoMat pt_peak_val_amat;
		AlgoMat pt_sharpness_amat;
	}AlgoPointQualityMat;
	
	typedef struct
    {
        float area_ratio;// led_area/img_area
        float area_offset;// distance(led_area.center-img.center)
        float area_angle; // led_Region  rotate angle
        float area_deform; // led_area deform
        float area_brightness;//area_mean_brightness/CAM_MAX
        float area_sharpness; // avg blur or sharpness
    }AlgoAreaQuality;


    typedef struct
    {
        float pt_temp_h; // led_point template size >3
        float pt_temp_w; // led_point template size >3
        float pt_fwhm; // led_point mean of full width at half maxima
        float pt_hw_dutyratio; // led_point dutyratio of FWHM
        float pt_avg_val; // led_point avg brightness
        float pt_peak_val;// 灯点峰值，在QualityEstimatePoint中是所有灯点的峰值的均值，在QualityAssessmentPointFromBlock中是最亮的block的灯点峰值的均值；
        float pt_sharpness;// led_point avg blur or sharpness
    }AlgoPointQuality;


    typedef enum
    {
        QUALITY_CHECK_PASS = 0,

        FAILED_AREA_RATIO__TOO_LOW = 0x00000001,//相机视野内LED屏幕在相机视野中占比过低
        FAILED_AREA_RATIO__TOO_HIGH = 0x00000002,//相机视野内LED屏幕在相机视野中占比过高
        FAILED_AREA_OFFSET__TOO_LOW = 0x00000004,//相机视野内LED屏幕偏离相机视野中心过近
        FAILED_AREA_OFFSET__TOO_HIGH = 0x00000008,//相机视野内LED屏幕偏离相机视野中心过远
        FAILED_AREA_ANGLE__TOO_LOW = 0x00000010,//相机视野内LED屏幕旋转角度过小
        FAILED_AREA_ANGLE__TOO_HIGH = 0x00000020,//相机视野内LED屏幕旋转角度过大
        FAILED_AREA_DEFORM__TOO_LOW = 0x00000040,//相机视野内LED屏幕形变过小
        FAILED_AREA_DEFORM__TOO_HIGH = 0x00000080,//相机视野内LED屏幕形变过大
        FAILED_AREA_SHARPNESS__TOO_LOW = 0x00000100,//相机视野内LED屏幕过于模糊
        FAILED_AREA_SHARPNESS__TOO_HIGH = 0x00000200,//相机视野内LED屏幕过于清晰
        FAILED_AREA_BRIGHTNESS__TOO_LOW = 0x00000400,//相机视野内LED屏幕太暗
        FAILED_AREA_BRIGHTNESS__TOO_HIGH = 0x00000800,//相机视野内LED屏幕太亮

        FAILED_POINT_TEMPLATE__TOO_LOW = 0x00001000,//相机视野内LED灯点间隔过小
        FAILED_POINT_TEMPLATE__TOO_HIGH = 0x00002000,//相机视野内LED灯点间隔过大
        //FAILED_POINT_TEMPLATE_DIFF__TOO_LOW = 0x00004000,//请检查打屛灯点行列数
        //FAILED_POINT_TEMPLATE_DIFF__TOO_HIGH = 0x00008000,//请检查打屛灯点行列数
        FAILED_POINT_FWHM__TOO_LOW = 0x00010000,//相机视野内LED灯点半高宽过小
        FAILED_POINT_FWHM__TOO_HIGH = 0x00020000,//相机视野内LED灯点半高宽过大
        FAILED_POINT_HW_DUTYRATIO__TOO_LOW = 0x00040000,//相机视野内LED灯点占比过小
        FAILED_POINT_HW_DUTYRATIO__TOO_HIGH = 0x00080000,//相机视野内LED灯点占比过大
        FAILED_POINT_AVG_VAL__TOO_LOW = 0x00100000,//相机视野内LED灯点亮度均值过小
        FAILED_POINT_AVG_VAL__TOO_HIGH = 0x00200000,//相机视野内LED灯点亮度均值过大
        FAILED_POINT_PEAK_VAL__TOO_LOW = 0x00400000,//相机视野内LED灯点峰值过小
        FAILED_POINT_PEAK_VAL__TOO_HIGH = 0x00800000,//相机视野内LED灯点峰值过大
        FAILED_POINT_SHARPNESS__TOO_LOW = 0x01000000,//相机视野内LED灯点过于模糊
        FAILED_POINT_SHARPNESS__TOO_HIGH = 0x02000000,//相机视野内LED灯点过于尖锐

    }QualityResult;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // 
