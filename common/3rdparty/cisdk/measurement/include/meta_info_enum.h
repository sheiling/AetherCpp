#ifndef __META_INFO_ENUM_H__
#define __META_INFO_ENUM_H__

// 版本：20251117——钟傲往CameraType添加CAM_SC6100       

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

    typedef enum
    {
        CAM_UNDEFINED = 0,
        CAM_16_BIT_DEPTH = 16,//16bit位深
        CAM_14_BIT_DEPTH = 14,//14
        CAM_12_BIT_DEPTH = 12,//12
        CAM_10_BIT_DEPTH = 10,//10
        CAM_8_BIT_DEPTH = 8,//8


        CAM_C1200 = 120016,//1200
        CAM_C3200 = 320012,//3200
		CAM_TPM6100 = 610016,//TP表示品牌，M表示黑白相机
		CAM_SC6100 = 610116,//SC60
        CAM_TP2600= 260016,
        CAM_TP2000= 200012,
        CAM_MOBILE_MI11 = 77771110,//77=Mobile 77=Mi 11= 11
        CAM_MER_2000_5GC = 68200012,//D=68
        CAM_MER_503_20GM = 68050310,
        CAM_MER_503_20GM_DEPTH8 = 68050308,

    } CameraType;//相机类型


    typedef enum
    {
        LENS_UNDEFINED = 0,
        LENS_FIXED_35MM = 35,//定焦35mm
        LENS_FIXED_50MM = 50,
        LENS_FIXED_80MM = 80,
        LENS_FIXED_100MM = 100,
		LENS_FIXED_135MM = 135,
        LENS_FIXED_8MM = 8,

    } LensType;//镜头类型


    typedef enum
    {
        FILTER_UNDEFINED = 0b00000000,//0
        FILTER_X = 0b00000001,//1
        FILTER_Y = 0b00000010,//2
        FILTER_Z = 0b00000100,//4
        FILTER_DN = 0b00001000,//8

        FILTER_XYZ = 0b00000111,//7
    }CamFilter;//相机滤片


    typedef enum {
        COLOR_BLACK = 0b00000000,//0
        COLOR_RED =   0b00000100,//4
        COLOR_GREEN = 0b00000010,//2
        COLOR_BLUE =  0b00000001,//1
        COLOR_WHITE = 0b00000111,//7
        //COLOR_WHITE = 0b00001000,//8
        //COLOR_RGB = 0b00000111,//7
        //COLOR_RGBW = 0b00001111,//15
    }AlgoColor;//颜色类型


    typedef enum
    {
        CIE_UNDEFINED = 0x00000000,//0
        CIE_X = 0x00000001,//1  ==FILTER_X
        CIE_Y = 0x00000002,//2  ==FILTER_Y
        CIE_Z = 0x00000004,//4  ==FILTER_Z
        CIE_XYZ = 0x00000007,//7  ==FILTER_XYZ

        CAM_R = 0x00000010,//16
        CAM_G = 0x00000020,//32
        CAM_B = 0x00000040,//64
        CAM_RGB = 0x00000070,//112
    }CIEType;//色度分量




    typedef enum
    {
        PATTERN_UNDEFINED = 0,
        PATTERN_GRID = 1,//普通隔点
    } PatternType;//隔点类型


    typedef enum
    {
        LED_PANEL_UNDEFINED = 0,
        LED_PANEL_COB = 1,
        LED_PANEL_SMD = 2,
        LED_PANEL_COG = 3,

    } LedPanelType;//灯板类型



    typedef enum
    {
        LED_LAYOUT_UNDEFINED = 0b0000,
        LED_LAYOUT_RECTANGLE = 0b0100,//4  灯点排布 矩形

        // 三角形/菱形灯点排布, 索引时候,行数最大
        LED_LAYOUT_TRIANGLE_ROW = 0b0001,//1

        // 三角形/菱形灯点排布, 索引时候,列数最大
        LED_LAYOUT_TRIANGLE_COL = 0b0010,//2

        // 三角形/菱形灯点排布, 索引时候,行数列数都最大
        LED_LAYOUT_TRIANGLE = 0b0011,//3
    } LedLayout;//灯点排布

	typedef enum
	{
		PARTITION_SHAPE_NORMAL = 0,						// 正常矩形屏体，默认值，不需要额外设置；
		PARTITION_SHAPE_IRREGULAR_CUTTING = 1,			// 不规则裁切屏体，灯点行列点间距一致，对齐排列；
		PARTITION_SHAPE_SPHERICAL_TRAPEZIUM = 2,		// 球形屏球身或球顶局部近似梯形：每行有抽点，但没有抽行；
		PARTITION_SHAPE_SPHERICAL_CIRCLE = 3,			// 球形屏整块球顶（一个圆或者多边形）；
	} PartitionShape;//待校正区域形状


    typedef enum
    {
        LED_SUBPIXEL_UNDEFINED = 0,
        LED_SUBPIXEL_SINGLE = 1,//一个灯点只是一个颜色构成 R或者G或者B
        LED_SUBPIXEL_RGB_STD = 123,//一个灯点由三个颜色(小灯)构成, 依次RGB

        //LED_SUBPIXEL_RGGB_DELTA = 1223,//一个灯点由4个颜色(小灯)构成, 依次RGGB周冬雨排列-Pentile 
        //LED_SUBPIXEL_RGBG_DIAMAND = 1232,//钻石排列
        //LED_SUBPIXEL_BRBG =3132,
    } LedSubpixelType;//灯点亚像素类型

    typedef enum
    {
        COLOR_BayerNone = 0,
        COLOR_BayerBG = 46,//bayer类型 cv::COLOR_BayerRG2RGB
        COLOR_BayerGB = 47,//bayer类型 cv::COLOR_BayerGR2RGB
        COLOR_BayerRG = 48, //cv::COLOR_BayerBG2RGB
        COLOR_BayerGR = 49,//cv::COLOR_BayerGB2RGB
    }BayerType;//Bayer类型



#ifdef __cplusplus
}
#endif // __cplusplus

#endif // 