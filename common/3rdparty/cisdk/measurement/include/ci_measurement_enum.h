#ifndef __CI_MEASUREMENT_ENUM_H__
#define __CI_MEASUREMENT_ENUM_H__


// 该头文件中是跨模块使用的基础的枚举变量


typedef enum
{
    ROTATEANGLE_ClookSize_90 = 90,   // 顺时针旋转90°
    ROTATEANGLE_ClookSize_180 = 180, // 顺时针旋转180°
    ROTATEANGLE_ClookSize_270 = 270, // 顺时针旋转270°
} RotateAngle; // 旋转方向，以角度为单位，顺时针为标准，用于旋转系数、大表等


typedef enum
{
    SPHERICALTYPE_Table_Line_Alignment = 0,   // 西瓜皮类球形屏抽点设计，大表行对齐
    SPHERICALTYPE_Table_Column_Alignment = 1, // 西瓜皮类球形屏抽点设计，大表列对齐
} SphericalType; // 旋转方向，以角度为单位，顺时针为标准，用于旋转系数、大表等


typedef enum
{
    COMPUTE_POINT_TEMPLATE_WH_HEIGHT_METHOD =
        0, // 高度法，根据左上到左下、右上到右下角点距离以及灯点行数计算模板尺寸，适用于不抽行的屏体，例如球形屏球身
    COMPUTE_POINT_TEMPLATE_WH_RADIUS_METHOD =
        1, // 半径法，将大表行数作为半径灯点个数，roi的宽高均值作为直径计算灯点间距，适用于球形屏球顶
    COMPUTE_POINT_TEMPLATE_WH_WIDTH_HEIGHT_METHOD = 2, // 宽高法，ROI的宽高除以点亮灯点行列数，适用于常规屏、裁切屏体
    COMPUTE_POINT_TEMPLATE_WH_AREA_METHOD =
        3, // 面积法，ROI的mask的面积除以灯点个数，得到单灯面积，开方得到灯点宽高，适用于抽点屏体、矩形屏体，不适用于裁切屏体(业务上拿不到灯点个数)
} ComputePointTemplateWhMethod; // 粗略计算单颗灯点大小的方法，不同屏体类型适用不同方法


enum ExportImageType
{
    ExportImageType_HotPic = 0,  //热力图
    ExportImageType_GrayPic = 1, //灰度图
};                               //图类型


enum CiInterpolationMethod
{
    INTER_NEAREST = 0, //同opencv
    INTER_LINEAR = 1,
    INTER_CUBIC = 2,
    INTER_AREA = 3,
};

// 校验等级
typedef enum VerificationLevel
{
    VERIFICATIONLEVEL_HIGH = 0, // 强校验模式，尽可能保证效果，对能预料到的异常情况都会报错。
    VERIFICATIONLEVEL_LOW = 1,  // 弱校验模式，不保证效果，对不影响流程正常运行的情况不报错。
} VerificationLevel;


#endif