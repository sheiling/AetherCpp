#ifndef __CAMERA_LENS_CONTROLLER_DATA_H__
#define __CAMERA_LENS_CONTROLLER_DATA_H__


#include "meta_info_data.h"
#include "algo_mat_data.h"

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

    typedef struct
    {

        int eps_m_T = 20;       // 曝光时间
        signed int eps_m_A = 3; //固定为3
        int eps_m_S = 0;        //增益

        int eps_M_T = 5000;     // 曝光时间
        signed int eps_M_A = 3; //固定为3
        int eps_M_S = 27;       //增益

        int H = 218;        // 合理曝光值上界
        int h = 60;         // 合理曝光值下界
        int h_target = 162; // 曝光目标值
        int h_over = 250;   // 完全过曝阈值
        int h_under = 5;    // 完全欠曝阈值
        int iter_max = 5;   // 最大迭代次数

        int in_rowRegionNum = 10; //每行用作计算曝光值的区域数量
        int in_colRegionNum = 10; //每列用作计算曝光值的区域数量
        int in_center_roi_width = 500;  //中心区域的宽
        int in_center_roi_height = 500; //中心区域的高

    } CameraLensControllerParam;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //
