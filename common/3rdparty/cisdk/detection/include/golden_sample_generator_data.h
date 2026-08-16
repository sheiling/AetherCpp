#ifndef __GOLDEN_SAMPLE_GENERATOR_DATA_H__
#define __GOLDEN_SAMPLE_GENERATOR_DATA_H__


#ifdef __cplusplus
extern "C" 
{
#endif // __cplusplus

	typedef enum 
	{
		//待测样本的类型（目标在样本图像中的分布类型）
		MULIT_TARGET_SAMPLE = 0,   //多目标样本，即一张样本图像中有多个目标，算法在这张样本图像中的多个目标里生成一个最优的作为模板，如：wafer图像样本（一张wanfer图像上有多个芯片），内部默认为该类型。
		SINGLE_TARGET_SAMPLE = 1,  //单目标样本，即一张样本图像中有一个目标，算法需要在多张样本中生成一个最优的作为模板，如：mark点样本（一张mark图像中仅有一个mark点）
	}SampleTargetType; //样本目标类型


	typedef struct 
	{
		//匹配目标阈值
		float target_match_thresh;  //目标的匹配阈值，即匹配值为多少以上时认为目标匹配正确，内部默认为0.7，该值推荐与后续算法的匹配阈值保持一致。
	}GoldenSampleThresh;  //最优模板阈值结构体

#ifdef __cplusplus
}

#endif //  __cplusplus

#endif
