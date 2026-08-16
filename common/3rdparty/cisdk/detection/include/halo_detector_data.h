#ifndef __HALO_DETECTOR_DATA_H__
#define __HALO_DETECTOR_DATA_H__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	typedef enum
	{
		LIGHT_NORMAL = 1,		 // 灯珠正常
		LIGHT_HIGH = 2,			 // 浮高
		LIGHT_DEVIATE = 3,		 // 偏位
		LIGHT_SCRATH = 4,		 // 刮花
		LIGHT_BLURRED = 5,		 // 雾化
		LIGHT_DIE = 6,			 // 死灯暗灯
		LIGHT_DIRTY = 7,         // 脏污
		LIGHT_OTHER = 8,		 // 其他
		LIGHT_DETECT_FAIL = 9, 	 // 检测失败
		LIGHT_LAMP_LINE = 10,	 // 工艺线
		LIGHT_JAGGY = 11         // 翘曲
	}DefectType;//缺陷类型

	typedef enum
	{
		REFLECT = 1,  // 反射式灯珠， 凹的灯珠
		REFRACT = 2,  // 折射式灯珠， 凸的灯珠
		OTHER = 3     // 其他备用类型
	}LightType;//灯珠类型


	typedef enum
	{
		PARAL_VERTICAL = 1,    // 灯珠并列排列，点亮的灯珠上下有亮点
		PARAL_HORZCAT = 2,     // 灯珠并列排列，点亮的灯珠左右有亮点
		CROSS = 3			   // 灯珠交叉排列
	}LightArray;//灯条排列方式


	typedef enum
	{
		LOCATE_TEMPLATE = 1,						// 定位模板

		PARAL_HALO_TEMPLATE_MID = 2,				// 并排光型模板--mid
		PARAL_HALO_TEMPLATE_BIG = 3,				// 并排光型模板--big

		CROSS_HALO_TEMPLATE_MID_LEFT = 4,			// 错位光型模板--mid--干扰点：左边
		CROSS_HALO_TEMPLATE_BIG_LEFT = 5,			// 错位光型模板--big--干扰点：左边
		CROSS_HALO_TEMPLATE_MID_RIGHT = 6,			// 错位光型模板--mid--干扰点：右边
		CROSS_HALO_TEMPLATE_BIG_RIGHT = 7,			// 错位光型模板--big--干扰点：右边
	}TemplateName;//灯条排列方式



	typedef enum
	{
		BLUE = 1,		// 偏蓝
		YELLOW = 2,     // 偏黄
		RED = 3,        // 偏红
		GREEN = 4,      // 偏绿
	}ChromaDirect;  // 判断色偏的方向。

	typedef struct
	{
		LightType light_type; // 反射式 == 1，折射式 == 2；
		LightArray light_array;
	}LightInfo;// 灯珠信息


	typedef struct
	{
		float die_light_th;                        //死灯检测阈值，推荐0.2f
		float bright_light_th;                     //爆亮检测阈值，推荐0.95f
		float round_th;		                       //中心内偏位，推荐0.013f
		float chroma_th;		                   //中心色偏阈值，推荐0.008f

		float outer_moon_sym_contrast_th;          //外大光圈、大月牙：对称方向对比度阈值，推荐0.13f  
		float outer_moon_foreback_contrast_th;     //外大光圈、大月牙：前景背景阈值，推荐0.08f  
		float outer_moon_wh_ratio_th;              //外月牙/内光圈：长宽比，推荐0.5f  

		float inner_aperture_sym_th;               //内光圈：对称方向对比度阈值，推荐0.3f  
		float inner_aperture_foreback_th;          //内光圈：与背景比较对比度阈值，推荐0.1f  

		float inner_moon_contrast_th;              //内月牙：对比度阈值，推荐>0.04f
		float inner_moon_curve_th;                 //内月牙：弯曲程度阈值，推荐>0.4f
		float inner_moon_wh_ratio_th;              //内月牙：长宽比阈值，推荐>2.0f
		float center_spot_scale_th;                //内月牙：中心亮斑等比例缩放因子，值变大，则亮斑放大，推荐2.2f 

		float scrath_contrast_th;		           //刮花：推荐0.2f
		float blurred_x_th;	                       //色差：色度x，推荐0.005f
		float blurred_y_th;	                       //色差：色度y，推荐0.005f

		float jaggy_foreback_contrast_th;          //翘曲：前景背景对比度，推荐-0.05f
		float jaggy_sym_constast_th;		       //翘曲：对称方向对比度，推荐-0.1f

		int dirty_area_th;						   //脏污：面积阈值 
		float dirty_contrast_th;	               //脏污：对比度阈值  

		float edge_process_row_th;                 //处理行边缘：处理因膜材不平整导致行边缘灯点过暗，推荐0.3f
		float edge_process_col_th;                 //处理列边缘：因膜材不平整导致列边缘灯点部分过暗，推荐0.7f
		float edge_remove_scale_th;                //处理异常边缘：缩放因子，值越大，异常边缘裁剪的越多，推荐2.0f 
		int   rectify_label;                       //处理亮暗差异较大的光型：0代表光型亮度不用管，1代表光型亮度和检测模板对齐  


	}DetectThresh;// 检测阈值


	typedef enum
	{
		MERGE_HORZCAT = 1,  // 水平串联拼接 
		MERGE_VERTCAT = 2,  // 垂直串联拼接 
	}MergeMethod; // 拼接方法



	/********************************模型检测数据**************************************************/

	typedef struct
	{
		float locate_th;			// 定位置信度阈值
		float two_class_th;			// 若是YOLO模型，二分类阈值； 若是anomalib，则是缺陷面积阈值。
		float two_class_segm_th;    // 二分类置信度分割阈值，只有anomalib模型用
		float multi_class_th;		// 多分类置信度阈值
		float svm_th;		// 多分类置信度阈值
	}DetectConfidenceThresh;		//模型检测阈值

	// 模型的类别
	typedef enum
	{
		LOACTION_MODEL = 1,		  // 定位模型
		TWO_CALSS_MODEL = 2,	  // 二分类模型
		MULTI_CLASS_MODEL = 3,    // 多分类模型
	}ModelType;

	// 用的模型是什么
	typedef enum
	{
		YOLO = 1,
		ANOMALIB = 2,
	}ModelName;



#ifdef __cplusplus
}
#endif // __cplusplus



#endif