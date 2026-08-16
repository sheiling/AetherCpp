#ifndef __PRE_DEMURA_EVALUATOR_DATA_H__
#define __PRE_DEMURA_EVALUATOR_DATA_H__

#include "meta_info_data.h"
#include "algo_mat_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus




typedef struct
{
	double L;
	double x;
	double y;
} Ca_Lxy;

// 箱体移动方向
typedef enum
{
	MOVE_NONE = 0,       // 箱体不移动
	MOVE_LEFT = 1,       // 箱体左移
	MOVE_RIGHT = 2,      // 箱体右移
	MOVE_UP = 4,		 // 箱体上移
	MOVE_DOWN = 8        // 箱体下移

}PreDemuraCabinetMoveDirection;


// 箱体旋转方向
typedef enum
{
	ROTATE_NONE = 0,                // 箱体不旋转
	ROTATE_CLOCKWISE = 1,           // 箱体顺时针旋转
	ROTATE_COUNTERCLOCKWISE = 2,    // 箱体逆时针旋转
}PreDemuraCabinetRotation;


// 二轴水平轴旋转方向
typedef enum
{
	HORIZONTALAXIS_ROTATE_NONE = 0,            // 二轴水平轴不旋转
	HORIZONTALAXIS_ROTATE_LEFT = 1,            // 二轴水平轴向左旋转即左侧远离视野
	HORIZONTALAXIS_ROTATE_RIGHT = 2,           // 二轴水平轴向右旋转即右侧远离视野
}PreDemuraHorizontalAxisRotation;


// 二轴垂直轴旋转方向
typedef enum
{
	VERTICALAXIS_ROTATE_NONE = 0,            // 二轴垂直轴不旋转
	VERTICALAXIS_ROTATE_UP = 1,              // 二轴垂直轴向上旋转即上侧远离视野
	VERTICALAXIS_ROTATE_DOWN = 2,            // 二轴垂直轴向下旋转即下侧远离视野
}PreDemuraVerticalAxisRotation;

// 箱体操作
typedef struct
{
	int move_direction;
	PreDemuraCabinetRotation rotate_direction;
	PreDemuraHorizontalAxisRotation horizontal_axis_direction;
	PreDemuraVerticalAxisRotation vertical_axis_direction;
}PreDremuraCabinetPositionOperation;



#ifdef __cplusplus
}
#endif // __cplusplus
#endif