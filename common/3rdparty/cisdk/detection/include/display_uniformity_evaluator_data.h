#ifndef __DISPLAY_UNIFORMITY_EVALUATOR_DATA_H__
#define __DISPLAY_UNIFORMITY_EVALUATOR_DATA_H__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
#include "algo_mat_data.h"

	typedef struct 
	{
		float light_dark_block_score;	       //亮暗块得分（板间差异-全局均值），即待检测屏幕上，以模组为单位的亮暗块均匀性得分，得分范围0~100，越高代表亮暗块越少，越均匀		
		float light_dark_junction_block_score; //亮暗块得分（板间差异-交界处均值），即待检测屏幕上，以模组为单位的亮暗块均匀性得分，得分范围0~100，越高代表亮暗块越少，越均匀		
		float module_curve_strength_score;	   //模组曲面强度得分（板内过渡），即全屏所有模组的公共曲面强度，得分范围0~100，得分越高代表越均匀；
		float banding_score;                   //贯穿条纹得分，即待检测屏上的贯穿条纹得分，得分范围0~100，越高代表贯穿条纹越少，越均匀；
		float periodicity_score;               //周期性得分，即待检测屏幕上的周期性均匀性得分，得分范围0~100，越高代表周期性越弱，越均匀；
		float mura_score;				       //mura得分，即待检测屏幕上的mura性均匀性得分，得分范围0~100，越高代表mura越少，越均匀；
		float entropy_score;		           //麻点得分，即待检测屏幕上的麻点性均匀性得分，得分范围0~100，越高代表麻点越少，越均匀；
	}UniformFeatureScore;//均匀性特征得分

	typedef struct
	{
		float light_dark_block_score_weight;    //亮暗块得分权重(板间差异-全局均值)
		float light_dark_junction_block_weight; //亮暗块得分权重(板间差异-交界处均值)
		float module_curve_strength_weight;     //模组曲面强度得分（板内过渡）
		float banding_score_weight;				//贯穿条纹得分权重
		float periodicity_score_weight;			//周期性得分权重
		float mura_score_weight;				//mura得分权重
		float entropy_score_weight;				//麻点得分权重
	}UniformFeatureScoreWeight;//均匀性特征得分权重

	typedef struct
	{
		bool is_detect_light_dark_block;         //是否进行亮暗块特征检测(板间差异-全局均值)
		bool is_detect_light_dark_junction_block;//是否进行亮暗块特征检测(板间差异-交界处均值)
		bool is_detect_module_curve_strength;    //是否进行曲面强度特征检测（板内过渡）
		bool is_detect_banding;					 //是否进行贯穿条纹特征检测
		bool is_detect_periodicity;				 //是否进行周期性特征检测
		bool is_detect_mura;					 //是否进行mura特征检测
		bool is_detect_entropy;					 //是否进行麻点特征检测
	} UniformFeatureDetectConfig;//均匀性检测参与计算的特征配置


	typedef struct
	{
		AlgoMat light_dark_block_mat;
		AlgoMat light_dark_junction_block_mat;
		AlgoMat module_curve_strength_matx;
		AlgoMat module_curve_strength_maty;
		AlgoMat module_curve_strength_matz;		
	} UniformFeatureExportResult;//均匀性检测导出检测特征可视化结果



#ifdef __cplusplus
}
#endif // __cplusplus



#endif