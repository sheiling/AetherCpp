#ifndef _WAFER_ANGLE_DETECTOR_DATA_H_
#define _WAFER_ANGLE_DETECTOR_DATA_H_

#include "ci_data.h"
#include "algo_mat.h"

#define USE_CUDA 0

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
	
	// 与灯板尺寸参数
	typedef struct
	{
		AlgoSize solder_size;      // 焊盘尺寸（包括焊盘的宽高）
		AlgoSize chip_size;        // 芯片尺寸（包括芯片的宽高）
		AlgoSize led_gap;          // led点间距(水平方向上的点间距与垂直方向上的点间距)
	}DieBondingParameter;

	// 灯板芯片排列类型
	typedef enum
	{
		CHIP_RGB_H = 0,      // 芯片以RGB的顺序 水平放置在焊盘上
		CHIP_BGR_H = 1,      // 芯片以BGR的顺序 水平放置在焊盘上
		CHIP_RGB_V = 2,		 // 芯片以RGB的顺序 垂直放置在焊盘上
		CHIP_BGR_V = 3,      // 芯片以BGR的顺序 垂直放置在焊盘上
	}DieBondingChipAlignType;

	// 芯片组检测阈值
	typedef struct
	{
		float chip_group_match_threshold;    // 芯片组的匹配阈值
		float line_distance_threshold;		 // 线性度检测的距离阈值
	}DieBondingGroupThreshold;


	// 芯片相关的阈值
	typedef struct
	{
		float chip_match_threshold;          // 芯片的匹配阈值
		int chip_segment_threshold;          // 芯片的分割阈值
		float polar_match_threshold;         // 极性的匹配阈值
	}DieBondingChipThreshold;


	// 芯片类型RGB
	typedef enum
	{
		CHIP_R = 0,        // R芯片
		CHIP_G = 1,		   // G芯片
		CHIP_B = 2,        // B芯片
		CHIP_N = 3,        // 无芯片
	}DieBondingChipType;

	// 芯片缺陷类型
	typedef enum
	{
		CHIP_NORMAL = 0x00000000,            // 芯片正常
		CHIP_MISSING = 0x00000001,           // 芯片缺失
		CHIP_DIRTY = 0x00000002,             // 芯片脏污  
		CHIP_BROKEN = 0x00000004,            // 芯片破损
		SOLDER_FOREIGN_OBJECT = 0x00000008,  // 焊盘上有异物
		SOLDER_ABNORMAL = 0x00000010,        // 焊盘异常(检测到的焊盘的尺寸和预设的不一致，一般情况下，此异常不对外展示)
		CHIP_BONDING_ERROR = 0x00000020,     // 芯片固错，把RGB的位置固错
	}DieBondingDefectType;

	// 极性类型 
	typedef enum
	{
		// todo： 把极性类型定为 正常 方向 未检出极性
		POLARITY_FORWARD = 0,					// 0：极性正向（即极性与模版的方向一致）
		POLARITY_REVERSE = 1,					// 1: 极性反向 (即极性与模版的方向相反)
		POLARITY_UNDETECTABLE = 2,              // 2: 不能检出极性
	}DieBondingPolarType;

	// 模版类型
	typedef enum
	{
		CHIP_GROUP_TEMPLATE = 0,             //CHIP_GROUP_TEMPLATE 芯片组模版
		CHIP_TEMPLATE = 1,					 //CHIP_TEMPLATE 芯片模版
		POLAR_TEMPLATE = 2,					 //POLAR_TEMPLATE 极性模版
	}DieBondingTemplateType;

	// 线性度检测flag
	typedef enum
	{
		LINE_DETECT_ROW = 1,		// 只在行方向上进行线性度检测
		LINE_DETECT_COL = 2,		// 只在列方向上进行线性度检测
		LINE_DETECT_ROW_AND_COL = 4,// 在行列方向上都进行线性度检测
	}DieBondingLineFlag;


	//============================================
	// 芯片信息
	typedef struct DieBondingChipInfoTag
	{
		DieBondingChipType chip_type = DieBondingChipType::CHIP_N;                       // chip_type: 芯片类型
		DieBondingChipType chip_actual_type = DieBondingChipType::CHIP_N;                // chip_actual_type：芯片实际类型 只有当芯片检测出固错时才会用到，一般情况下看chip_type即可
		DieBondingPolarType polar_type = DieBondingPolarType::POLARITY_UNDETECTABLE;     // polar_type: 芯片极性类型
		AlgoPoint2f solder_coordinate = {0.0f, 0.0f};									 // solder_coordinate: 焊盘中心坐标
		AlgoPoint2f chip_coordinate = { 0.0f, 0.0f };									 // chip_coordinate: 芯片中心坐标
		AlgoPoint2f solder_chip_deflection = { 0.0f, 0.0f };						     // solder_chip_deflection 芯片中心相对于焊盘中心的偏移
		float chip_angle = 0.0f;														 // chip_angle：芯片角度
		float chip_area = 0.0f;														     // chip_area: 芯片面积
		float chip_width = 0.0f;														 // chip_width：芯片宽度
		float chip_height = 0.0f;														 // chip_height: 芯片高度
		float solder_width = 0.0f;                                                       // solder_width：焊盘宽度
		float solder_height = 0.0f;                                                      // solder_height：焊盘高度
		int chip_defect = 0x00000000;													 // chip_defect: 芯片缺陷：通过正常芯片与DefectType中芯片进行或运算得到当前芯片的的脏污情况
																						 // 最终第几位为1（即与对应缺陷进行与运算的结果不为0）就是存在哪种缺陷
																						 // 如一个芯片存在破损与极性反向,则其存在的缺陷为DefectType::NORMAL_CHIP | DefectType::POLARITY_REVERSED | DefectType::CHIP_BROKEN
																						 // 结果为0x00000030 = 0b110000
																						 // 故此处chip_defect的类型为int
		bool chip_offset_row = false;													 // chip_offset_row: 芯片是否偏移了它所属的行（所属行直线度存在问题）
		bool chip_offset_col = false;													 // chip_offset_col: 芯片是否便宜了它所属的列（所属列直线度存在问题）
		float chip_offset_value_row = 0.0f;												 // chip_offset_value_row：芯片对于所在行的偏移
		float chip_offset_value_col = 0.0f;												 // chip_offset_value_col：芯片对于所在列的偏移
		AlgoRect chip_image_rect;														 // chip_image_roi: 芯片图像在原图上的ROI，用于后续查看图片进行人工复判


		float solder_paste_area_1 = 0.0f;                                                //上锡膏或左锡膏的像素面积
		float solder_paste_area_2 = 0.0f;                                                //下锡膏或右锡膏的像素面积

		AlgoPoint2f solder_paste_center_1 = { 0.0f, 0.0f };                              //上锡膏或左锡膏的轮廓中心
		AlgoPoint2f solder_paste_center_2 = { 0.0f, 0.0f };                              //下锡膏或右锡膏的轮廓中心
		AlgoPoint2f solder_paste_center = { 0.0f, 0.0f };                                //上下或左右锡膏整体轮廓中心

		AlgoPoint2f channel_coordinate = { 0.0f, 0.0f };                                 // 河道中心坐标
		float channel_width = 0.0f;                                                      // 河道宽度
		float channel_height = 0.0f;                                                     // 河道高度


	}DieBondingChipInfo;

	// 芯片组信息
	typedef struct
	{
		int row;								 // 芯片组所在行数
		int col;								 // 芯片组所在列数
		AlgoRect chip_group_rect;				 // 芯片组所在图像上的roi
		AlgoPoint2f coordinate;                  // 当前芯片组中心在图像上的坐标
		DieBondingChipInfo chip_rgb_info[3];     // rgb三个芯片信息
		char image_index[256];					 // 图像索引信息，不同图像的索引信息必须保持唯一，用来确定芯片属于哪一张图像
	}DieBondingChipGroupInfo;
	


#ifdef __cplusplus
}
#endif // __cplusplus

#endif


