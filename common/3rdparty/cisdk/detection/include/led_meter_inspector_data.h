#ifndef __LED_METER_INSPECTOR_DATA_H__
#define  __LED_METER_INSPECTOR_DATA_H__

#include "algo_mat_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	typedef enum
	{
		PATH_S_HORIZONTAL_LEFT2RIGHT_FIRST = 0,    //s型路线，起点在左侧，先水平向右移动，再垂直向下移动
		PATH_S_HORIZONTAL_RIGHT2LEFT_FIRST = 1,    //s型路线，起点在右侧，先水平向左移动，再垂直向下移动
		PATH_I_HORIZONTAL_LEFT2RIGHT_FIRST = 2,    //直线型，起点在左侧，先水平向右移动，再垂直向下移动
		PATH_I_HORIZONTAL_RIGHT2LEFT_FIRST = 3,    //直线型，起点在右侧，先水平向左移动，再垂直向下移动
		PATH_S_VERTICAL_UP2DOWN_FIRST = 4,         //s型路线，起点在上，先垂直向下移动，再水平向右移动
		PATH_S_VERTICAL_DOWN2UP_FIRST = 5,         //s型路线，起点在下，先垂直向上移动，再水平向右移动
		PATH_I_VERTICAL_UP2DOWN_FIRST = 6,         //直线型，起点在上，先垂直向下移动，再水平向右移动
		PATH_I_VERTICAL_DOWN2UP_FIRST = 7,         //直线型，起点在下，先垂直向上移动，再水平向右移动
		/*
		* s型路线示例
		*  开始--------------->
		*					  |
		*  <----------------- ⬇
		* |
		* ⬇---------------->结束
		*
		* 直线型路线示例
		* 开始---------------->
		* <-------------------
		* |
		* |
		* ⬇------------------>
		* <------------------
		* |
		* |
		* ⬇------------------>
		* <----------------结束
		*
		*/
	}MotionPathType;



	typedef struct
	{
		//1、黑屏阈值
		float thr_black;//黑屏
		//2、常亮阈值
		float thr_black_lights;//黑屏亮灯：暂时不用
		//3、死灯阈值
		float thr_dead_R;//红色画面 
		float thr_dead_G;//绿色画面
		float thr_dead_B;//蓝色画面
		float thr_dead_W;//白色画面
		//4、暗灯阈值
		float thr_dimMin_R;//红色画面 //最小暗灯阈值
		float thr_dimMax_R;          //最大暗灯阈值
		float thr_dimMin_G;//绿色画面 
		float thr_dimMax_G;
		float thr_dimMin_B;//蓝色画面 
		float thr_dimMax_B;
		float thr_dimMin_W;//白色画面
		float thr_dimMax_W;
		//5、亮灯阈值
		float thr_bright_R;//红色画面
		float thr_bright_G;//绿色画面
		float thr_bright_B;//蓝色画面
		float thr_bright_W;//白色画面
		//6、串死串活阈值
		float thr_mapDead_R;//红色画面  //串死
		float thr_mapLive_R;         //串活
		float thr_mapDead_G;//绿色画面 
		float thr_mapLive_G;
		float thr_mapDead_B;//蓝色画面 
		float thr_mapLive_B;
		float thr_mapDead_W;//白色画面
		float thr_mapLive_W;
		//7、连续暗灯连续亮灯阈值，红绿蓝色画面共用1组阈值
		int thr_width_sec_num;//粗定位：宽分段数
		int thr_height_sec_num;//粗定位：高分段数
		float thr_coarseMin;//粗定位：连续暗灯行列定位阈值
		float thr_coarseMax;//粗定位：连续亮灯行列定位阈值
		float thr_continusMin;//精定位：连续暗灯灯点定位阈值
		float thr_continusMax;//精定位：连续亮灯灯点定位阈值
		//8、串色阈值
		float thr_Red_R_max;//红色画面：R、G、B通道阈值
		float thr_Red_G_min;
		float thr_Red_B_min;
		float thr_Green_G_max;//绿色画面：G、B、R通道阈值
		float thr_Green_B_min;
		float thr_Green_R_min;
		float thr_Blue_B_max;//蓝色画面：B、G、R通道阈值
		float thr_Blue_G_min;
		float thr_Blue_R_min;
		//9、色偏阈值
		float thr_colorcast_line;//色偏基准线
		//10、亮度异常判断阈值
		float thr_redLumMin;//红色画面
		float thr_redLumMax;
		float thr_greenLumMin;//绿色画面
		float thr_greenLumMax;
		float thr_blueLumMin;//蓝色画面
		float thr_blueLumMax;
		//11、算法调用
		float weight_val;//权重系数：0.0f-1.0f
		int temp_size;//检测模板：支持n*n大小的检测模板，n为大于1的奇数，且n*n为小于灯板短边灯点数
		//12、连续缺陷灯阈值：筛选连续串色、连续死灯、连续串活、连续串色缺陷使用
		float thr_continuous_num;//连续缺陷灯数目
		float thr_lineGap;//相邻线间距

	}MeterInspectThresh;//阈值结构体

	typedef struct {
		//混色检测阈值
		//1. 初筛阈值
		float thr_mixColor_preSelect_R;//红色画面 
		float thr_mixColor_preSelect_G;//绿色画面
		float thr_mixColor_preSelect_B;//蓝色画面
		float thr_mixColor_preSelect_W;//白色画面
		//2、▲E筛选阈值
		float thr_mixColor_deltaE_R;//红色画面 
		float thr_mixColor_deltaE_G;//绿色画面 
		float thr_mixColor_deltaE_B;//蓝色画面 
		float thr_mixColor_deltaE_W;//白色画面
		//3、滤波核大小
		int thr_preSelect_median_filter_kernel_size;//初筛中值滤波核大小，该值默认为推荐值，推荐值为5， 若需更改需找算法确认；
		int thr_preSelect_mean_filter_kernel_size;  //初筛均值滤波核大小，该值默认为推荐值，推荐值为13，若需更改需找算法确认；
		int thr_deltaE_block_size;//计算deltaE得分时参与计算的区域大小，该值默认为推荐值，推荐值为5， 若需更改需找算法确认；
	}MeterInspectMixColorThresh;
	

	typedef struct
	{
		//运动路径
		MotionPathType motion_path;  //设备运动路径
		int horizontal_shot_num;     //水平方向上的总计拍摄次数
		int vertical_shot_num;		 //垂直方向上的总计拍摄次数
		int horizontal_overlap_num;  //水平方向上重叠灯珠的数目
		int vertical_overlap_num;    //垂直方向上重叠灯珠的数目
		AlgoSize led_wh;             //整张led灯板的灯点数，w*h(列*高)
	}MeterInspectMergeInfo;


	typedef enum
	{
		//检测串灯：串死串活
		LEDMAP_HORIZONTICAL_SLASH_LINE = 0,//横线画面
		LEDMAP_VERTICAL_SLASH_LINE = 1,//竖线画面
		LEDMAP_LEFT_SLASH_LINE = 2,//左斜画面
		LEDMAP_RIGHT_SLASH_LINE = 3,//右斜画面
		LEDMAP_CHESS_BOARD_PATTERN = 4, //沃格背光棋盘格画面
		LEDMAP_HORIZONTICAL_LINE =5,//沃格背光水平扫描线（与类型0不同，类型0水平线固定线宽为1，该水平线宽可定制）
		LEDMAP_VERTICAL_LINE = 6,//沃格背光垂直扫描线（与类型1不同，类型1垂直线固定线宽为1，该垂直线宽可定制）
	}LedMapType;

	//缺陷标识
	typedef enum
	{
		LED_DEFECT_FLAG_UNINIT = 0x00000000,//正常灯：不存在缺陷灯
		LED_DEFECT_FLAG_MAP_LIVE = 0x00000001,//串亮：不该亮亮了
		LED_DEFECT_FLAG_MAP_DEAD = 0x00000002,//串死：该亮没亮
		LED_DEFECT_FLAG_DEAD = 0x00000004,//死灯：该亮没亮
		LED_DEFECT_FLAG_DIM = 0x00000008,//暗灯：亮度比正常灯暗
		LED_DEFECT_FLAG_BRIGHT = 0x00000010,//亮灯：亮度比正常灯亮
		LED_DEFECT_FLAG_WRONG_COLOR = 0x00000020,//错色示例：打红色时绿灯或蓝灯亮了，红灯没亮
		LED_DEFECT_FLAG_CONTINUS = 0x01000000,//连续缺陷：筛选连续串色、连续死灯、连续串活、连续串色缺陷使用
		LED_DEFECT_FLAG_CONTINUS_DIM = 0x00000040,//连续暗灯：1行或1列中有连续n颗暗灯
		LED_DEFECT_FLAG_CONTINUS_BRIGHT = 0x00000080,//连续亮灯：1行或1列中有连续n颗亮灯
		LED_DEFECT_FLAG_COLOR_CAST = 0x00000060,//色偏：颜色与基准颜色存在差异
		LED_DEFECT_FLAG_MIX_COLOR = 0x00000070,//混色：该亮的与不该亮的一起都亮了；如B画面，某个位置的BR灯一起亮或BG灯一起亮，或BGR一起亮
		
		////示例：组合缺陷标识符示例：既是暗灯又是串色
		//LED_DEFECT_STATE_DIM_WRONG_COLOR = LED_DEFECT_FLAG_DIM | LED_DEFECT_FLAG_WRONG_COLOR,
	}LedDefectFlag;

#ifdef __cplusplus
}
#endif // __cplusplus
#endif