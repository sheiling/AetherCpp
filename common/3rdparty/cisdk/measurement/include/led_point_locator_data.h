#ifndef __LED_POINT_LOCATOR_DATA_H__
#define __LED_POINT_LOCATOR_DATA_H__

#include "ci_data.h"
#include "algo_mat_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

    typedef enum
    {
        LED_FLAG_NONEXIST = 0b00000000,//物理不存在
        LED_FLAG_DEAD = 0b00000001,//死灯标志  
        LED_FLAG_LIVE = 0b00000010,//非死灯标志
        LED_FLAG_ISOLATED = 0b00000110,//孤立标志
    } LedFlag;//灯点状态

#define LED_FLAG_CHECK_EXIST(flag) ((bool)flag)
#define LED_FLAG_CHECK_DEAD(flag) (flag & LED_FLAG_DEAD)
#define LED_FLAG_CHECK_LIVE(flag) (flag & LED_FLAG_LIVE)
#define LED_FLAG_CHECK_ISOLATED(flag) (flag == LED_FLAG_ISOLATED)


    typedef struct
    {
        AlgoMat state;//灯点状态  ALGO_8UC1
        AlgoMat img_loc;//灯点亚像素位置 ALGO_32FC2
    }AlgoLedPoint;


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // 
