#ifndef __ALGO_MAT_DATA_H__
#define __ALGO_MAT_DATA_H__
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

    typedef enum
    {
        ALGO_8U = 0,//8位 unsigned
        ALGO_8S = 1,//8位
        ALGO_16U = 2,//16位 unsigned
        ALGO_16S = 3,//16位
        ALGO_32S = 4,//32位
        ALGO_32F = 5,//32位 float
        ALGO_64F = 6,//64位 float
    } AlgoMatDepth;//位深,按照opencv的,方便转换


    typedef enum
    {
        ALGO_8UC1 = 0,//8位1通道 unsigned
        ALGO_8SC1 = 1,//8位1通道
        ALGO_16UC1 = 2,//16位1通道 unsigned
        ALGO_16SC1 = 3,//16位1通道
        ALGO_32SC1 = 4,//32位1通道
        ALGO_32FC1 = 5,//32位1通道 float
        ALGO_64FC1 = 6,//64位1通道 float

        ALGO_8UC2 = 8,//8位2通道
        ALGO_8SC2 = 9,//8位2通道
        ALGO_16UC2 = 10,//16位2通道
        ALGO_16SC2 = 11,//16位2通道
        ALGO_32SC2 = 12,//32位2通道
        ALGO_32FC2 = 13,//32位2通道
        ALGO_64FC2 = 14,//64位2通道

        ALGO_8UC3 = 16,//8位3通道
        ALGO_8SC3 = 17,//8位3通道
        ALGO_16UC3 = 18,//16位3通道
        ALGO_16SC3 = 19,//16位3通道
        ALGO_32SC3 = 20,//32位3通道
        ALGO_32FC3 = 21,//32位3通道
        ALGO_64FC3 = 22,//64位3通道

        ALGO_8UC4 = 24,//8位4通道
        ALGO_8SC4 = 25,//8位4通道
        ALGO_16UC4 = 26,//16位4通道
        ALGO_16SC4 = 27,//16位4通道
        ALGO_32SC4 = 28,//32位4通道
        ALGO_32FC4 = 29,//32位4通道
        ALGO_64FC4 = 30,//64位4通道
    } AlgoMatType;//矩阵类型



    //举例: 3通道8位图像 type=ALGO_8UC3  channels=3,depth_in_byte=3*8/8=3
    //举例: 1通道16位图像 type=ALGO_16UC1 channels=1,depth_in_byte=1*16/8=2
    typedef struct
    {
        int rows;//矩阵行数
        int cols;//矩阵列数
        AlgoMatType type;//矩阵类型
        unsigned int len;// == depth_in_byte * rows * cols
        unsigned char* data;//矩阵元素指针
    } AlgoMat;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // 
