#ifndef __EDGE_CALIB_H__
#define __EDGE_CALIB_H__

#include "ci_define.h"
#include "ci_declare.h"
#include "edge_calib_data.h"
#include "edge_calib_error.h"

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus


    /************************************
    @brief  : 创建edgecalib句柄
    @note   :
    @param  : out_edgecalib_handle   -- edgecalib句柄
    @return	: AlgoError
    @Author : chengjiayi@2021/06/24
    ************************************/
    C_API AlgoError C_CALL EdgeCalibCreateHandle(AlgoHandle *out_edgecalib_handle);


    /************************************
    @brief  : 计算异形箱体的边缘补偿系数
    @note   :
                对于state的要求：
                    每行活灯与死灯的数目之和与待校正区域该行真实灯点数一致；抽点/裁切掉的点都作为不存在灯点体现在state中；
                PARTITION_SHAPE_IRREGULAR_CUTTING，异形裁切屏体：
                    基于state的按照上下左右的顺序依次寻找边缘活灯，并将边缘归一化到指定宽度的光通量上
                    边界：目前针对内凹箱体，计算边缘修正系数的时候会丢失部分向内弯折的边缘的数据；对于狭窄区域，如果目标点是死灯，会跳过该点；
                PARTITION_SHAPE_SPHERICAL_CIRCLE，球形屏整块的球顶：
                    该情况仅有一个边缘；
                PARTITION_SHAPE_SPHERICAL_TRAPEZIUM，球形屏的球身与球顶局部区域，近似于梯形：
                    该情况有四个边缘（当上边缘仅有一颗灯点时，修正系数按1存储，左、右、下边缘依旧正常）
    @param  : edgecalib_handle				有效的edgecalib句柄
    @param  : in_xyz_meters_handle			同一个颜色的不同滤片meter数据
    @param  : in_meter_num					meter的数量，不能够超过3（对应X、Y、Z滤片）
    @param  : in_part_shape					meter对应的待校正区域形状；
    @return	: AlgoError
    @Author : GuoHao@2024/11/04
    ************************************/
    C_API AlgoError C_CALL
    EdgeCalibComputeIrregular(AlgoHandle edgecalib_handle, AlgoHandle *in_xyz_meters_handle, int in_meter_num,
                              PartitionShape in_part_shape = PartitionShape::PARTITION_SHAPE_SPHERICAL_TRAPEZIUM);


    /************************************
    @brief  : 修正异形箱体的边缘
    @note   :
                对于state的要求：
                    每行活灯与死灯的数目之和与待校正区域该行真实灯点数一致；抽点/裁切掉的点都作为不存在灯点体现在state中；
                PARTITION_SHAPE_IRREGULAR_CUTTING，异形裁切屏体：
                PARTITION_SHAPE_SPHERICAL_CIRCLE，球形屏整块的球顶：
                PARTITION_SHAPE_SPHERICAL_TRAPEZIUM，球形屏的球身与球顶局部区域，近似于梯形：
    @param  : edgecalib_handle				有效的edgecalib句柄
    @param  : in_meter_handle				有效的meter数据，必须包含state，否则会报错；
    @param  : in_part_shape					meter对应的待校正区域形状；
    @return	: AlgoError
    @Author : GuoHao@2024/11/04
    ************************************/
    C_API AlgoError C_CALL
    EdgeCalibRectifyIrregular(AlgoHandle edgecalib_handle, AlgoHandle in_meter_handle,
                              PartitionShape in_part_shape = PartitionShape::PARTITION_SHAPE_SPHERICAL_TRAPEZIUM);


    /************************************
    @brief  : 计算白色边缘系数===》利用RGB配比，通过RGB边缘系数合成白色边缘系数
    @note   :
    @param  : edgecalib_handle                有效的edgecalib句柄
    @param  : in_rgb_edge_handle
    输入的RGB的边缘系数，必须同时输入"RGB的X"或"RGB的Y"或"RGB的Z"(RZ可以为空，但不支持RZ为nullptr)，则输出的为W对应滤片的边缘系数，另外的滤片无系数,
                                              也可同时输入RGB的XYZ，则输出的为W_XYZ的边缘
                                              输入顺序必须依次为RGB
    @param  : in_rgb_edge_num                 in_rgb_edge_handle的长度，长度为3
    @param  : in_rgbw_xyz                     对应RGBW的XYZ，用于计算RGB配比，在合成w_edge时需要
    @param  : out_w_edge_handle               输出的w的边缘系数
    @return	: AlgoError
    @Author : zhangxuan@2024/07/03
    ************************************/
    C_API AlgoError C_CALL EdgeCalibComputeWhiteEdgeByRGBEdge(AlgoHandle edgecalib_handle,
                                                              AlgoHandle *in_rgb_edge_handle, int in_rgb_edge_num,
                                                              RGBWXYZ in_rgbw_xyz, AlgoHandle out_w_edge_handle);


    /************************************
    @brief  : 光通量数据边缘修正
    @note   : 归一化方式，将最边缘数据拉齐到内部数据
    @param  : edgecalib_handle                    有效的edgecalib句柄
    @param  : in_out_meter_handle                 需要修正的meter数据
    @param  : in_top/in_bottom/in_left/in_right   上下左右各自需要修正的宽度，大于0
    @return	: AlgoError
    @Author : ouxingtao@2023/1/10
    ************************************/
    C_API AlgoError C_CALL EdgeCalibFixEdgeNormal(AlgoHandle edgecalib_handle, AlgoHandle in_out_meter_handle,
                                                  const int in_top, const int in_bottom, const int in_left,
                                                  const int in_right);

    /************************************
    @brief  : 设置边缘宽度
    @note   :
    @param  : edgecalib_handle                有效的edgecalib句柄
    @param  : in_edge_height                  上下边缘宽度
    @param  : in_edge_width                   左右边缘宽度
    @return	: AlgoError
    @Author : liying@2021/12/31
    ************************************/
    C_API AlgoError C_CALL EdgeCalibSetEdgeWidthHeight(AlgoHandle edgecalib_handle, const int in_edge_height,
                                                       const int in_edge_width);


    /************************************
    @brief  : 计算边缘补偿系数
    @note   :
    @param  : edgecalib_handle                有效的edgecalib句柄
    @param  : in_meter_handle_1_with_edge     1. 有边缘的数据, xyz中任意一个,不能全为空 有效的meter_handle句柄
    详见algo_led_meter.h
                                              2. 当只有该meter输入，其他均为空时，为计算全屏边缘系数功能；
    @param  : in_meter_handle_1_without_edge  无边缘的数据,和in_meter_handle_1_with_edge匹配, xyz中任意一个,
    有效的meter_handle句柄 详见algo_led_meter.h
    @param  : in_meter_handle_2_with_edge     同上
    @param  : in_meter_handle_2_without_edge  同上
    @param  : in_meter_handle_3_with_edge     同上
    @param  : in_meter_handle_3_without_edge  同上
    @return	: AlgoError
    @Author : zhangtao@2023/09/08
    ************************************/
    C_API AlgoError C_CALL EdgeCalibCompute(AlgoHandle edgecalib_handle, AlgoHandle in_meter_handle_1_with_edge,
                                            AlgoHandle in_meter_handle_1_without_edge = nullptr,
                                            AlgoHandle in_meter_handle_2_with_edge = nullptr,
                                            AlgoHandle in_meter_handle_2_without_edge = nullptr,
                                            AlgoHandle in_meter_handle_3_with_edge = nullptr,
                                            AlgoHandle in_meter_handle_3_without_edge = nullptr);


    /************************************
    @brief  : 修改标定文件的内容：内缩边缘系数, 顺序是左右上下
    @note   :
    @param  : edgecalib_handle                有效的edgecalib句柄
    @param  : in_ratios_orient				  每个方向的修改比例
    @param  : in_nums_edge_orient			  每个方向希望修改的行数（从最边缘开始往内计算行列数）
    @param  : in_array_num			          数组的大小，必须为4
    @param  : in_cie_type			          想要修改的滤片类型
    @return	: AlgoError
    @Author : jpzheng@2025/09/23
    ************************************/
    C_API AlgoError C_CALL EdgeCalibSetEdgeCoeffRatio(AlgoHandle edgecalib_handle, const float *in_ratios_orient,
                                                      const int *in_nums_edge_orient, const int in_array_num,
                                                      const CIEType in_cie_type);

    /************************************
    @brief  : 设置二次边缘补偿值衰减系数
    @note   :
    @param  : edgecalib_handle                有效的edgecalib句柄
    @param  : in_ratio                        二次边缘补偿值衰减系数
    @return	: AlgoError
    @Author : liying@2021/12/31
    ************************************/
    C_API AlgoError C_CALL EdgeCalibSetRatioPara(AlgoHandle edgecalib_handle, const float in_ratio);


    /************************************
    @brief  : 计算二次边缘补偿系数
    @note   :
    @param  : edgecalib_handle                有效的edgecalib句柄
    @param  : in_block_height                 单模组高度，
    @param  : in_block_width                  单模组宽度，与上述拼接区域单个模组大小对应
    @param  : in_meter_handle_1_screen        校正后拼接区域的数据, xyz中任意一个,不能全为空 有效的meter_handle句柄
    详见algo_led_meter.h
    @param  : in_meter_handle_2_screen        同上
    @param  : in_meter_handle_3_screen        同上
    @param  : in_screen_startheight           拼接区域起始行，默认为0
    @param  : in_screen_startwidth            拼接区域起始列，默认为0
    @return	: AlgoError
    @Author : liying@2021/12/29
    ************************************/
    C_API AlgoError C_CALL EdgeCalibComputeOffset(AlgoHandle edgecalib_handle, const int in_block_height,
                                                  const int in_block_width, AlgoHandle in_meter_handle_1_screen,
                                                  AlgoHandle in_meter_handle_2_screen = nullptr,
                                                  AlgoHandle in_meter_handle_3_screen = nullptr,
                                                  const int in_screen_startheight = 0,
                                                  const int in_screen_startwidth = 0);


    /************************************
    @brief  : 应用边缘二次补偿系数，对原系数进行补偿
    @note   : 后缀只能是.edge_offset
    @param  : edgecalib_handle           有效的edgecalib句柄
    @param  : in_edgecalib_fullname      补偿系数文件,后缀只能是.edge_offset
    @return	: AlgoError
    @Author : liying@2022/01/07
    ************************************/
    C_API AlgoError C_CALL EdgeCalibApplyOffset(AlgoHandle edgecalib_handle, const char *in_compcalib_fullname);


    /************************************
    @brief  : 利用边缘系数进行修正
    @note   :
    @param  : edgecalib_handle     有效的edgecalib句柄
    @param  : in_meter_handle       有效的meter_handle句柄 详见algo_led_meter.h
    @return	: AlgoError
    @Author : chengjiayi@2021/06/25
    ************************************/
    C_API AlgoError C_CALL EdgeCalibRectify(AlgoHandle edgecalib_handle, AlgoHandle in_meter_handle);


    /************************************
     @brief  : 保存二次边缘补偿系数
     @note   : 后缀只能是.edge_offset
     @param  : edgecalib_handle        有效的edgecalib句柄
     @param  : in_edgecalib_fullname   标定文件,后缀只能是.edge_offset
     @return : AlgoError
     @Author : liying@2022/01/07
     ************************************/
    C_API AlgoError C_CALL EdgeCalibSaveOffset(AlgoHandle edgecalib_handle, const char *in_compcalib_fullname);


    /************************************
     @brief  : 读取保存边缘系数
     @note   : 后缀只能是.edge
     @param  : edgecalib_handle        有效的edgecalib句柄
     @param  : in_edgecalib_fullname   标定文件,后缀只能是.edge或者.edge.yml
     @return : AlgoError
     @Author : chengjiayi@2021/06/24
     ************************************/
    C_API AlgoError C_CALL EdgeCalibSave(AlgoHandle edgecalib_handle, const char *in_edgecalib_fullname);
    C_API AlgoError C_CALL EdgeCalibLoad(AlgoHandle edgecalib_handle, const char *in_edgecalib_fullname);


    /************************************
     @brief  : 生成公共边缘系数
     @note   : 用num组边缘系数生成公共边缘系数
     @param  : in_filename    输入num组文件地址
     @param  : out_filename   公共边缘系数保存地址
     @param  : num
     @return : AlgoError
     @Author : ouxingtao@2021/09/18
     ************************************/
    C_API AlgoError C_CALL EdgeCoeffMean(const char *in_filename[], const int in_length, const char *out_filename[],
                                         const int out_length, const int num);


    /************************************
     @brief  : 生成公共边缘系数
     @note   : 用num组边缘系数生成公共边缘系数
     @param  : in_edgecalib_handles		输入num组边缘系数
     @param  : in_edge_nums				边缘系数的数量
     @param  : out_edgecalib_avg_handle	输出的公共边缘系数
     @return : AlgoError
     @Author : 田博@2021/09/18
     ************************************/
    C_API AlgoError C_CALL EdgeCoeffAverage(AlgoHandle *in_edgecalib_handles, int in_edge_nums,
                                            AlgoHandle out_edgecalib_avg_handle);


    /************************************
     @brief  : 根据param来剔除异常边缘
     @note   :
     @param  : in_edgecalib_handles		输入num组边缘系数
     @param  : in_edge_nums				边缘系数的数量
     @param  : in_params				入参，长度为1，表示异常边缘的阈值。建议0.005。
                                        param=0.005,表示返回的有效边缘与边缘均值的最大差异不超过0.5%。
     @param  : in_params_num			入参长度，目前长度为1
     @param  : out_valid_edgecalib_handle	输出的有效边缘handles
     @param  : out_valid_edge_num			输出的有效边缘个数
     @return : AlgoError
     @Author : 田博@2024/01/04
     ************************************/
    C_API AlgoError C_CALL EdgeCoeffAbnomalEdgeDetec(AlgoHandle *in_edgecalib_handles, int in_edge_nums,
                                                     float *in_params, int in_params_num,
                                                     AlgoHandle *out_valid_edgecalib_handle, int *out_valid_edge_num);


    /************************************
    @brief  : 获取句柄内容
    @note   :
    @param  : edgecalib_handle    有效的edgecalib句柄
    @param  : out_para           导出的结构体,必须已经初始化完毕
    @param  : in_is_deep_copy    是否深拷贝, 默认浅拷贝  1=深拷贝;0=浅拷贝
    @return	: AlgoError
    @Author : chengjiayi@2021/06/24
    ************************************/
    C_API AlgoError C_CALL EdgeCalibGet(AlgoHandle edgecalib_handle, AlgoEdgeCalib *out_para, int in_is_deep_copy = 0);


    /************************************
    @brief  : 将edgecalib里面的数据导出到buffer
    @note   :
    @param  : edgecalib_handle   有效的edgecalib句柄
    @param  : out_buffer         已经申请好申请空间
    @param  : in_out_len         外部申请好的空间大小,会用真实大小修改
    @return	: AlgoError
    @Author : ouxingtao@2021/07/02
    ************************************/
    C_API AlgoError C_CALL EdgeCalibToBuffer(AlgoHandle edgecalib_handle, char *out_buffer, uint64_t *in_out_len);


    /************************************
    @brief  : 获取buffer大小
    @note   :
    @param  : edgecalib_handle   当前edgecalib句柄
    @param  : out_len            buffer大小
    @return	: AlgoError
    @Author : ouxingtao@2021/07/07
    ************************************/
    C_API AlgoError C_CALL EdgeCalibGetBufferSize(AlgoHandle edgecalib_handle, uint64_t *out_len);


    /************************************
    @brief  : 将buffer数据导入到edgecalib
    @note   :
    @param  : edgecalib_handle   有效的edgecalib句柄
    @param  : in_buffer
    @param  : in_len
    @return	: AlgoError
    @Author : ouxingtao@2021/07/02
    ************************************/
    C_API AlgoError C_CALL EdgeCalibFromBuffer(AlgoHandle edgecalib_handle, const char *in_buffer, uint64_t in_len);


    /**********************************************
    @Brief：检测输入的多个箱体边缘是否修好
    @Returns  :  AlgoError
    @Parameter  :  meter_handles： 输入 in_num 个meter数据
    @Parameter  :  in_num   --输入箱体meter数量
    @Parameter  :  *out_abEdgeBox：   ---数组，异常箱体边缘结构体信息
    @Parameter  :  int *out_num     输出，异常边缘箱体的数量
    @Parameter  :  edge_width       输入，边缘宽度，默认为8
    @Parameter  : int edge_ratio    输入，边缘异常阈值，默认为0.01
    @Author		:  Yaof@2024.3.12
    **********************************************/
    C_API AlgoError C_CALL EdgeCalibCheckAbnormalEdges(AlgoHandle *meter_handles, int in_num,
                                                       EdgeCalibEdgeInfo *out_abEdgeBox, int *out_num,
                                                       const int edge_width = 8, const float edge_ratio = 0.01f);


    /************************************
    Release  句柄指向的对象的内存空间释放, 句柄指向的对象还在
    Destroy  句柄指向的对象销毁, 此句柄已经无效, 请配合handle=nullptr;

    handle=new OBJ;
    OBJ.Set(a_lot_data);

    Release 相当于:
        OBJ.Release();
    Destroy 相当于:
        OBJ.release();
        delete handle;
    ************************************/
    C_API AlgoError C_CALL EdgeCalibRelease(AlgoHandle edgecalib_handle);
    C_API void C_CALL EdgeCalibDestroyHandle(AlgoHandle edgecalib_handle);


#ifdef __cplusplus
}
#endif // __cplusplus
#endif