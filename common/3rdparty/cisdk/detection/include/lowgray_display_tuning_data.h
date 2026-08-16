#ifndef __LOWGRAY_DISPLAY_TUNING_DATA_H__
#define __LOWGRAY_DISPLAY_TUNING_DATA_H__

#include "ci_data.h"
#include "meta_info_data.h"

#define MAX_REG_NUM  20

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

    typedef struct {

        int key[MAX_REG_NUM];
        double err;

    }ICResult;

    // 测试pattern信息
    typedef struct {
        int pLocLen;			//点定位框的像素边长
        int gapLen;				//灰阶过渡gap像素宽度
        bool vertical;			//垂直灰阶判定
        double pureNorm;        //灰阶纯色均匀性准出Norm阈值
        double grayNorm;        //灰阶过渡均匀性准出Norm阈值

    }PatternInfo;

    // 寄存器参数信息
    typedef struct {

        char* name;             //寄存器名称
        int low_bound;          //寄存器参数下界
        int up_bound;           //寄存器参数上界
        int step;               //调试步进

    }Register;

    //单灯板IC的行列数
    typedef struct {

        int rows;               //行数
        int cols;               //列数

    }IC_Area;

    typedef struct {

        AlgoSize screenSize;	//模组大小
        LedPanel panel;			//灯板大小
        Pattern pattern;		//点亮灯点区域大小
        IC_Area icArea;			//单灯板ic的行列分布

    }IC_BoxInfo;

    struct IC_TREE_Node {
        int feat = -1;          // 分裂特征
        double split = 0.0;     // 分裂值
        double mean = 0.0;      // 叶节点均值
        struct IC_TREE_Node* left = NULL;
        struct IC_TREE_Node* right = NULL;
        //~Node() { delete left; delete right; }
    };

#ifdef __cplusplus
}
#endif __cplusplus
#endif // !__LOWGRAY_DISPLAY_TUNING_DATA_H__
