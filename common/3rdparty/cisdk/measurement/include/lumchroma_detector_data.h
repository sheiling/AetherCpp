#ifndef __LUMCHROMA_DETECTOR_DATA_H__
#define __LUMCHROMA_DETECTOR_DATA_H__

#include "algo_mat_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	typedef enum
	{
		LUM_CHROMA_DETECT_CAM_RGB = 0,                        //检测空间：板间差异时使用，当前要求使用：LUM_CHROMA_DETECT_CIE_LXY
		LUM_CHROMA_DETECT_CIE_LXY = 1,
		LUM_CHROMA_DETECT_CIE_LAB = 2,
		LUM_CHROMA_DETECT_CIE_LCH = 3,
		LUM_CHROMA_DETECT_CIE_HSV = 4,
	}LumChromaDetectColorSpace;//颜色空间

	typedef enum
	{
		LUM_CHROMA_DETECT_FLAG_UNINIT = 0x00000000,           //正常：不存在缺陷---板内  
		LUM_CHROMA_DETECT_FLAG_DEAD_POINT = 0x00000001,       //死点：灯点不亮---板内  
		LUM_CHROMA_DETECT_FLAG_DARK_POINT = 0x00000002,       //暗点：亮度比正常灯暗---板内  
		LUM_CHROMA_DETECT_FLAG_BRIGHT_POINT = 0x00000004,     //亮点：亮度比正常灯亮---板内  
		LUM_CHROMA_DETECT_FLAG_DARK_BLOCK = 0x00000008,       //暗块：亮度比正常区域暗---板内  
		LUM_CHROMA_DETECT_FLAG_BRIGHT_BLOCK = 0x00000010,     //亮块：亮度比正常区域亮---板内  
	}LumChromaDetectFlag;//缺陷标识

	typedef enum
	{
		LUM_CHROMA_DETECT_FIVE_AREAS = 0,                     //板内分区检测评价亮色度差异：0：代表均分五区，1：代表均分九区
		LUM_CHROMA_DETECT_NINE_AREAS = 1,
	}LumChromaDetectAreas;//分区情况

	typedef struct
	{
		float thr_deadPoint_Rr;                               //红色：死点含义：阈值范围(0,1.0f),若设置为0.3f,则代表比正常灯亮度暗70%以上判为死点
		float thr_deadPoint_Rg;
		float thr_deadPoint_Rb;
		float thr_darkPoint_Rr;                               //红色：暗点含义：阈值范围(0,1.0f),若设置为0.8f,则代表比正常灯亮度暗20%以上判为暗点
		float thr_darkPoint_Rg;
		float thr_darkPoint_Rb;
		float thr_brightPoint_Rr;                             //红色：亮点含义：阈值范围(1.0f,＋∞),若设置为1.2f,则代表比正常灯亮度亮20%以上判为亮点
		float thr_brightPoint_Rg;
		float thr_brightPoint_Rb;

		float thr_deadPoint_Gr;                               //绿色：死点含义：阈值范围(0,1.0f),若设置为0.3f,则代表比正常灯亮度暗70%以上判为死点
		float thr_deadPoint_Gg;
		float thr_deadPoint_Gb;
		float thr_darkPoint_Gr;                               //绿色：暗点含义：阈值范围(0,1.0f),若设置为0.8f,则代表比正常灯亮度暗20%以上判为暗点
		float thr_darkPoint_Gg;
		float thr_darkPoint_Gb;
		float thr_brightPoint_Gr;							  //绿色：亮点含义：阈值范围(1.0f,＋∞),若设置为1.2f,则代表比正常灯亮度亮20%以上判为亮点
		float thr_brightPoint_Gg;
		float thr_brightPoint_Gb;

		float thr_deadPoint_Br;                               //蓝色：死点含义：阈值范围(0,1.0f),若设置为0.3f,则代表比正常灯亮度暗70%以上判为死点
		float thr_deadPoint_Bg;
		float thr_deadPoint_Bb;
		float thr_darkPoint_Br;                               //蓝色：暗点含义：阈值范围(0,1.0f),若设置为0.8f,则代表比正常灯亮度暗20%以上判为暗点
		float thr_darkPoint_Bg;
		float thr_darkPoint_Bb;
		float thr_brightPoint_Br;							  //蓝色：亮点含义：阈值范围(1.0f,＋∞),若设置为1.2f,则代表比正常灯亮度亮20%以上判为亮点
		float thr_brightPoint_Bg;
		float thr_brightPoint_Bb;

		float thr_deadPoint_Wr;                               //白色：死点含义：阈值范围(0,1.0f),若设置为0.3f,则代表比正常灯亮度暗70%以上判为死点
		float thr_deadPoint_Wg;
		float thr_deadPoint_Wb;
		float thr_darkPoint_Wr;                               //白色：暗点含义：阈值范围(0,1.0f),若设置为0.8f,则代表比正常灯亮度暗20%以上判为暗点
		float thr_darkPoint_Wg;
		float thr_darkPoint_Wb;
		float thr_brightPoint_Wr;							  //白色：亮点含义：阈值范围(1.0f,＋∞),若设置为1.2f,则代表比正常灯亮度亮20%以上判为亮点
		float thr_brightPoint_Wg;
		float thr_brightPoint_Wb;

	}LumChromaDetectPointThresh;//板内-亮暗点死点阈值：红绿蓝白-RGBW

	typedef struct
	{
		float thr_darkBlock_Rr;                               //红色：暗块含义：阈值范围(0,1.0f),若设置为0.8f,则代表比正常灯亮度暗20%以上判为暗块
		float thr_darkBlock_Rg;
		float thr_darkBlock_Rb;
		float thr_brightBlock_Rr;                             //红色：亮块含义：阈值范围(1.0f,＋∞),若设置为1.2f,则代表比正常灯亮度亮20%以上判为亮块
		float thr_brightBlock_Rg;
		float thr_brightBlock_Rb;

		float thr_darkBlock_Gr;                               //绿色：暗块含义：阈值范围(0,1.0f),若设置为0.8f,则代表比正常灯亮度暗20%以上判为暗块
		float thr_darkBlock_Gg;
		float thr_darkBlock_Gb;
		float thr_brightBlock_Gr;                             //绿色：亮块含义：阈值范围(1.0f,＋∞),若设置为1.2f,则代表比正常灯亮度亮20%以上判为亮块
		float thr_brightBlock_Gg;
		float thr_brightBlock_Gb;

		float thr_darkBlock_Br;                               //蓝色：暗块含义：阈值范围(0,1.0f),若设置为0.8f,则代表比正常灯亮度暗20%以上判为暗块
		float thr_darkBlock_Bg;
		float thr_darkBlock_Bb;
		float thr_brightBlock_Br;                             //蓝色：亮块含义：阈值范围(1.0f,＋∞),若设置为1.2f,则代表比正常灯亮度亮20%以上判为亮块
		float thr_brightBlock_Bg;
		float thr_brightBlock_Bb;

		float thr_darkBlock_Wr;                               //白色：暗块含义：阈值范围(0,1.0f),若设置为0.8f,则代表比正常灯亮度暗20%以上判为暗块
		float thr_darkBlock_Wg;
		float thr_darkBlock_Wb;
		float thr_brightBlock_Wr;                             //白色：亮块含义：阈值范围(1.0f,＋∞),若设置为1.2f,则代表比正常灯亮度亮20%以上判为亮块
		float thr_brightBlock_Wg;
		float thr_brightBlock_Wb;

		float thr_seed_ratio;                                 //亮暗块检测初始对比度值：越小检出数量越多，越大检出数量越少，默认值0.03f  
		float thr_seed_area;                                  //亮暗块检测粗筛面积值：越小检出数量越多，越大检出数量越少，默认值100.f  
		float thr_defect_area;                                //亮暗块检测精筛面积值：越小检出数量越多，越大检出数量越少，默认值100.f  
	}LumChromaDetectBlockThresh;//板内-亮暗块阈值：红绿蓝白-RGBW

	typedef struct
	{
		float thr_lum_R;                                      //红色：亮度色度：分为①②③④挡
		float thr_chroma1_R;
		float thr_chroma2_R;

		float thr_lum_G;                                      //绿色：亮度色度：分为①②③④挡
		float thr_chroma1_G;
		float thr_chroma2_G;

		float thr_lum_B;                                      //蓝色：亮度色度：分为①②③④挡
		float thr_chroma1_B;
		float thr_chroma2_B;

		float thr_lum_W;                                      //白色：亮度色度：分为①②③④挡
		float thr_chroma1_W;
		float thr_chroma2_W;
	}LumChromaDetectConsisThresh;//板间/板内-亮色度一致性阈值：红绿蓝白-RGBW

	typedef struct
	{
		float Lum;                                            //亮色度表征
		float Chroma1;
		float Chroma2;
	}LumChromaDetectFeature;//板间-表征---输入输出	

	typedef struct
	{
		float Lum_diff;                                       //亮色度与基准偏差
		float Chroma1_diff;
		float Chroma2_diff;
	}LumChromaDetectDiff;//板间/板内-与基准差异---输出


#ifdef __cplusplus
}
#endif // __cplusplus
#endif