#ifndef __LED_CONTOUR_SEARCHER_H__
#define __LED_CONTOUR_SEARCHER_H__

#include "ci_define.h"
#include "ci_declare.h"
#include "led_contour_searcher_data.h"
#include "led_contour_searcher_errors.h"
#include "meta_info_enum.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

    /************************************
    @brief  :  初始化句柄
    @note   :
    @param  : out_contour_handle  灯点边缘句柄
    @return	: AlgoError
    @Author : chengjiayi@2021/11/10
    ************************************/
    C_API AlgoError C_CALL LedContourCreateHandle(AlgoHandle* out_contour_handle);


    /************************************
    @brief  : 初始化句柄
    @note   : 不调用此函数, 模块默认 = CPU线程数-1
    @param  : contour_handle 有效的句柄
    @param  : in_omp_num    并行运算线程个数
    @return	: AlgoError
    @Author : chengjiayi@2021/11/10
    ************************************/
    C_API AlgoError C_CALL LedContourSetThreadsNum(AlgoHandle contour_handle, int in_omp_num);

    /************************************
    @brief  :  设置模块方法
    @note   : 不调用此函数, 模块默认方法0, 目前仅支持RECT_FIXED,不要随意更改
              in_method =0, RECT_FIXED  // Glabal方法 搜索框大小一致
              in_method = 1, RECT_FLOAT // Local方法  搜索框大小不一致
              in_method = 2,CIRCLE_FIXED
              in_method= 3, CIRCLE_FLOAT
    @param  : contour_handle 有效的句柄
    @param  : in_method           计算方法
    @return	: AlgoError
    @Author : chengjiayi@2021/11/10
    ************************************/
    C_API AlgoError C_CALL LedContourSetMethod(AlgoHandle contour_handle, int in_method);


    /************************************
    @brief  : 设置灯点框的缩放系数
    @note   : 
    @param  : contour_handle        有效的句柄
    @param  : in_shrink_para        0<in_shrink_para<=1
    @return	: AlgoError
    @Author : chengjiayi@2022/03/14
    ************************************/
    /************************************
    @brief  : 设置灯点框的缩放系数
    @note   : 
    @param  : contour_handle   有效的句柄
    @param  : in_shrink_w      宽度方向缩放,>0
    @param  : in_shrink_h      高度方向缩放,>0
    @return	: AlgoError
    @Author : chengjiayi@2022/04/29
    ************************************/
    C_API AlgoError C_CALL LedContourSetBorderShrink(AlgoHandle contour_handle, float in_shrink_w, float in_shrink_h);


    /************************************
    @brief  : 使用自适应矩形框,默认方形框
    @note   : 
    @param  : contour_handle 有效的句柄
    @return	: AlgoError
    @Author : chengjiayi@2022/04/29
    ************************************/
    C_API AlgoError C_CALL LedContourSetUsingRect(AlgoHandle contour_handle);


	/************************************
	@brief  : 基于任意灯点排布的点定位算法的搜寻灯点边缘，暂不支持灯点物理位置映射
	@note   : 通过点定位计算的灯点半径中值确定搜索框大小
	@param  : contour_handle        灯点边缘句柄
	@param  : in_shot_no_metainfo   有效的shot句柄, metainfo可以缺失;如果shot包含metainfo,将会被忽略
	@param  : in_ledpoint           有效的ledpoint句柄    详见led_point_locator.h中定义
	@return : AlgoError
	@Author : wangyueyao@2024/2/1
	************************************/
	C_API AlgoError C_CALL LedContourSearchForAnyArrangement(AlgoHandle contour_handle, const AlgoHandle in_shot_no_metainfo, const AlgoHandle in_ledpoint);


    /************************************
    @brief  : 搜寻灯点边缘
    @note   :
    @param  : contour_handle        灯点边缘句柄
    @param  : in_shot_no_metainfo   有效的shot句柄, metainfo可以缺失;如果shot包含metainfo,将会被忽略
    @param  : in_ledpoint           有效的ledpoint句柄    详见led_point_locator.h中定义
    @param  : panel_layput          灯点排布类型
    @return : AlgoError
    @Author : chengjiayi@2022/09/20
    ************************************/
    C_API AlgoError C_CALL LedContourSearchNoMetainfo(AlgoHandle contour_handle, const AlgoHandle in_shot_no_metainfo,
        const AlgoHandle in_ledpoint, LedLayout in_panel_layput);

    /************************************
    @brief  : 搜寻灯点边缘
    @note   : 兼容LedContourSearchIr 和 LedContourSearchWithPointMap。
    @param  : contour_handle    灯点边缘句柄
    @param  : in_pure_shot      有效的shot句柄    详见algo_shot.h中定义
    @param  : in_ledpoint       有效的ledpoint句柄    详见led_point_locator.h中定义
    @return : AlgoError
    @Author : chengjiayi@2022/09/20
    ************************************/
    C_API AlgoError C_CALL LedContourSearch(AlgoHandle contour_handle, const AlgoHandle in_pure_shot, const AlgoHandle in_ledpoint, int in_point_type = ALGOMOD_LED_POINT_LOCATOR);
    
	/************************************
	@brief  : 搜寻灯点边缘
	@note   :
	@param  : contour_handle    灯点边缘句柄
	@param  : in_pure_shot      有效的shot句柄			详见algo_shot.h中定义
	@param  : ir_mapper_handle  有效的PointIrMap句柄     
	@return : AlgoError
	@Author : GuoHao@2024/07/31
	************************************/
	//C_API AlgoError C_CALL LedContourSearchIr(AlgoHandle contour_handle, const AlgoHandle in_pure_shot, const AlgoHandle ir_mapper_handle);

	/************************************
	@brief  : 搜寻PointMap灯点边缘
	@note   :
	@param  : contour_handle    灯点边缘句柄
	@param  : in_pure_shot      有效的shot句柄,必须包含panel,image	详见algo_shot.h中定义
	@param  : in_pointmap       有效的PointMap句柄      详见point_mapper.h定义
	@return : AlgoError
	@Author : huangziming@2024/09/29
	************************************/
	//C_API AlgoError C_CALL LedContourSearchWithPointMap(AlgoHandle contour_handle, const AlgoHandle in_pure_shot, const AlgoHandle in_pointmap);

    /************************************
    @brief  : 获取灯点边缘搜寻结果
    @note   : 
    @param  : contour_handle    灯点边缘句柄
    @param  : out_ledcontour    导出的结构体,必须已经初始化完毕
    @param  : in_is_deep_copy   是否深拷贝, 默认浅拷贝  1=深拷贝;0=浅拷贝
    @return	: AlgoError
    @Author : chengjiayi@2021/06/18
    ************************************/
    C_API AlgoError C_CALL LedContourGet(AlgoHandle contour_handle, AlgoLedContour* out_ledcontour, int in_is_deep_copy = 0);


    /************************************
    @brief  : 载入和保存灯点边缘数据
    @note   : 后缀必须是.contour
    @param  : contour_handle    灯点边缘句柄
    @param  : in_fullname       文件名后缀必须是.contour
    @return	: AlgoError
    @Author : chengjiayi@2021/06/18
    ************************************/
    C_API AlgoError C_CALL LedContourLoad(AlgoHandle contour_handle, const char* in_fullname);
    C_API AlgoError C_CALL LedContourSave(AlgoHandle contour_handle, const char* in_fullname);


    /************************************
    @brief  : 导出灯点边缘搜寻结果图
    @note   : 为了将灯点边缘画清楚, 进行了放大
    @param  : contour_handle     灯点边缘句柄
    @param  : in_img_fullname    导出图片完整名字
    @param  : in_scale           放大倍数
    @param  : is_split           1=将放大的图像,取四角及中间区域拼接,减少图像大小; 0=显示完整放大图像
    @param  : in_shot            在shot上画框, 默认null=在LedContourSearch输入的shot上画, 不为null=在此shot上画
    @return	: AlgoError
    @Author : chengjiayi@2022/02/22
    ************************************/
    C_API AlgoError C_CALL LedContourExportImage(AlgoHandle contour_handle, 
        const char* in_img_fullname, 
        int in_scale = 5,
        int is_split = 1, 
        const AlgoHandle in_shot=nullptr);



    /************************************
    @brief    : 导出灯点状态图
    @note     : 红色=死灯,黑色=物理不存在;绿色=活灯;
    @param    : contour_handle  --有效灯点边缘句柄
    @param    : in_img_fullname 
    @param    : in_scale        --放大倍数建议3或5,默认=3放大
    @return   : AlgoError
    @Modified : chengjiayi@2022/10/27
    ************************************/
    C_API AlgoError C_CALL LedContourExportStateImage(AlgoHandle contour_handle, const char* in_img_fullname, int in_scale=3);
    
    
    /************************************
    @brief  : 获取buffer大小
    @note   :
    @param  : vig_handle  当前有效句柄
    @param  : out_len     buffer大小
    @return	: AlgoError
    @Author : chengjiayi@2021/07/06
    ************************************/
    C_API AlgoError C_CALL LedContourGetBufferSize(AlgoHandle contour_handle, uint64_t* out_len);


    /************************************
    @brief  : 将contour里面的数据导出到buffer
    @note   :
    @param  : contour_handle    有效的contour句柄
    @param  : in_out_buffer     已经申请好申请空间
    @param  : in_out_len        外部申请好的空间大小,会用真实大小修改
    @return	: AlgoError
    @Author : ouxingtao@2021/07/01
    ************************************/
    C_API AlgoError C_CALL LedContourToBuffer(AlgoHandle contour_handle, char* in_out_buffer, uint64_t* in_out_len);


    /************************************
    @brief  : 将buffer数据导入到contour
    @note   :
    @param  : contour_handle   有效的contour句柄
    @param  : in_buffer
    @param  : in_len
    @return	: AlgoError
    @Author : ouxingtao@2021/07/01
    ************************************/
    C_API AlgoError C_CALL LedContourFromBuffer(AlgoHandle contour_handle, const char* in_buffer, uint64_t in_len);



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
    C_API AlgoError C_CALL LedContourRelease(AlgoHandle contour_handle);
    C_API void C_CALL LedContourDestroyHandle(AlgoHandle contour_handle);







#ifdef __cplusplus
}
#endif // __cplusplus
#endif