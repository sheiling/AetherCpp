#ifndef __LED_LUM_EXTRACTOR_H__
#define __LED_LUM_EXTRACTOR_H__

#include "ci_define.h"
#include "ci_declare.h"
#include "led_lum_extractor_data.h"
#include "led_lum_extractor_errors.h"
#include "geometry_transformer.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

    C_API AlgoError C_CALL LedLumCreateHandle(AlgoHandle* out_lum_handle);

	/************************************
	@brief  : 设置灯点光通量提取方法
	@note   : 不调用此函数，模块默认使用原点定位方法, 不要随意更改方法
			  in_extract_method = LedLumMethod_SUM_METHOD,原点定位采用方法，输出搜索框光通量求和结果
			  in_extract_method = LedLumMethod_MEAN_METHOD,点阵超分采用方法，输出搜索框光通量均值结果

	@param  : lum_handle 有效的光通量提取句柄
	@param  : in_extract_method    --光通量提取方法
	@return	: AlgoError
	@Author : huangziming@2024/09/29
	************************************/
	C_API AlgoError C_CALL LedLumSetDefaultMethod(AlgoHandle lum_handle, const LedLumMethod in_extract_method);

	/************************************
	@brief  : 设置Transform,及在提取光通量时，是基于transform后的shot进行的
	@note   : 该逻辑目前仅适用于超分2.0方案中
	@param  : lum_handle		 有效的光通量提取句柄
	@param  : in_transforms    --有效的transform模块
	@return	: AlgoError
	@Author : 田博@2025/12/26
	************************************/
	C_API AlgoError C_CALL LedLumSetTransform(AlgoHandle lum_handle, AlgoHandle in_transforms);

    /************************************s
    @brief  : 光通量提取
    @note   : 
    @param  : lum_handle    有效的光通量句柄
    @param  : in_pure_shot  有效的shot句柄 详见algo_shot.h中定义,可以不包含metainfo
    @param  : in_ledcoutour 有效的LedContour句柄详见led_contour_searcher.h中定义
    @return	: AlgoError
    @Author : chengjiayi@2021/06/21
    ************************************/
    C_API AlgoError C_CALL LedLumExtract(AlgoHandle lum_handle, const AlgoHandle in_pure_shot, const AlgoHandle in_ledcoutour, int in_contour_type = ALGOMOD_LED_COUTOUR_SEARCHER);

	/************************************
    @brief  : 设置异形搜索框光通量提取时框放大倍数
    @note   : 该值越大，亚像素误差越小，但耗时越长，默认值为10,输入范围应该是1~12；
    @param  : lum_handle    有效的光通量句柄
    @param  : in_scale		缩放倍数
    @return	: AlgoError
    @Author : GuoHao@2024/12/13
    ************************************/
	C_API AlgoError C_CALL LedLumSetPixelScale(AlgoHandle lum_handle, const int in_pixel_scale);
	
	/************************************
	@brief  : 设置光通量提取时检索灯点框的方法
	@note   : 框检索方法，指的是提取光通量的时候用什么方法确认某个像素是否在搜索框内，目前仅针对异形搜索框ir_contour
			    针对不规则搜索框可以使用mask求均值的方式实现，如果搜索框是四边形可以基于几何学逐像素求和；
				目前测试下来，mask的速度会快很多（in_pixel_scale = 10的时候，针对Demo_ExtracSphericalScreen测试数据，几何法3.1s，mask0.8s）精度上从数据来看趋势一致,偏差在0.5%以内，暂推荐mask方法；
	@param  : lum_handle    有效的光通量句柄
	@param  : in_pure_shot  有效的shot句柄 详见algo_shot.h中定义,可以不包含metainfo
	@param  : in_ledcoutour 有效的LedContour句柄详见led_contour_searcher.h中定义
	@return	: AlgoError
	@Author : GuoHao@2024/12/13
	************************************/
	C_API AlgoError C_CALL LedLumSetContourRetrieveMethod(AlgoHandle lum_handle, const ContourRetrieveMethod in_method);


    /************************************
    @brief  : 光通量提取
    @note   : 去除dc值后的光通量
    @param  : lum_handle    有效的光通量句柄
    @param  : in_pure_shot  有效的shot句柄 详见algo_shot.h中定义,可以不包含metainfo
    @param  : in_ledcoutour 有效的LedContour句柄详见led_contour_searcher.h中定义
    @param  : in_ledroi     有效的ROICreateHandle句柄详见roi_detector.h中定义
    @param  : dc_start      计算dc值所用区域的起始位置，为0则从roi区域开始，为1则对roi外扩1个灯点宽度
    @param  : dc_width      计算dc值所用区域的宽度，为1则从起始区域外扩1个灯点宽度
    @return	: AlgoError
    @Author : ouxingtao@2023/03/14
    ************************************/
    C_API AlgoError C_CALL LedLumExtractRemoveDC(AlgoHandle lum_handle, const AlgoHandle in_pure_shot, const AlgoHandle in_ledcoutour,
        const AlgoHandle in_ledroi, const int dc_start, const int dc_width);

	/************************************
	@brief  : 点亮机侧面光通量提取
	@note   : 仅用于点亮机侧面光通量提取，其余业务场景不可调用
	@param  : lum_handle     有效的光通量句柄
	@param  : in_pure_shot   有效的shot句柄 详见algo_shot.h中定义,可以不包含metainfo
	@param  : in_ledcoutour  有效的LedContour句柄详见led_contour_searcher.h中定义
	@param  : in_roi_before_corner4 roi定位前设置的4个侧面角点
	@param  : in_roi_after_corner4 roi定位后获取的4个角点  
	@Param  : in_corner_num	 角点数量，默认为4
	@return	: AlgoError
	@Author : wushaoshuang@2025/04/14
	************************************/
	C_API AlgoError C_CALL LedLumExtractFromSideLEDPanel(AlgoHandle lum_handle, const AlgoHandle in_pure_shot, const AlgoHandle in_ledcoutour,
		const AlgoPoint* in_roi_before_corner4, const AlgoPoint* in_roi_after_corner4, int in_corner_num = 4);


    /************************************
    @brief  : 获取光通量
    @note   : 
    @param  : lum_handle    有效的光通量句柄
    @param  : out_lum       导出的结构体,必须已经初始化完毕
    @param  : in_is_deep_copy   是否深拷贝, 默认浅拷贝  1=深拷贝;0=浅拷贝
    @return	: AlgoError
    @Author : chengjiayi@2021/06/21
    ************************************/
    C_API AlgoError C_CALL LedLumGet(AlgoHandle lum_handle, AlgoLedLum* out_lum, int in_is_deep_copy=0);


    /************************************
    @brief  : 载入和保存灯点光通量数据
    @note   : 文件名后缀必须是.lum
    @param  : lum_handle    有效的光通量句柄
    @param  : in_fullname   文件名后缀必须是.lum
    @return	: AlgoError
    @Author : chengjiayi@2021/06/21
    ************************************/
    C_API AlgoError C_CALL LedLumLoad(AlgoHandle lum_handle, const char* in_fullname);
    C_API AlgoError C_CALL LedLumSave(AlgoHandle lum_handle, const char* in_fullname);


    /************************************
    @brief  : 导出灯点光通量结果图
    @note   : 
    @param  : lum_handle        有效的光通量句柄
    @param  : in_img_fullname   文件名
    @param  : in_is_color       是否为彩色  1=彩色,0=灰度
    @param  : in_multiple       图像缩放倍数
    @return	: AlgoError
    @Author : chengjiayi@2021/11/11
    ************************************/
    C_API AlgoError C_CALL LedLumExportImage(AlgoHandle lum_handle, const char* in_img_fullname,int in_is_color=0, int in_multiple = 1);


    /************************************
    @brief    : 导出灯点状态图
    @note     : 红色=死灯,黑色=物理不存在;绿色=活灯;
    @param    : lum_handle      --有效的光通量句柄
    @param    : in_img_fullname 
    @param    : in_scale        --放大倍数建议3或5,默认=3放大
    @return   : AlgoError
    @Modified : chengjiayi@2022/10/27
    ************************************/
    C_API AlgoError C_CALL LedLumExportStateImage(AlgoHandle lum_handle, const char* in_img_fullname, int in_scale=3);

    /************************************
    @brief  : 获取buffer大小
    @note   :
    @param  : lum_handle  当前有效句柄
    @param  : out_len     buffer大小
    @return	: AlgoError
    @Author : chengjiayi@2021/07/06
    ************************************/
    C_API AlgoError C_CALL LedLumGetBufferSize(AlgoHandle lum_handle, uint64_t* out_len);

    /************************************
    @brief  : 将lum里面的数据导出到buffer
    @note   :
    @param  : lum_handle     有效的lum句柄
    @param  : out_buffer     已经申请好申请空间
    @param  : in_out_len     外部申请好的空间大小,会用真实大小修改
    @return	: AlgoError
    @Author : ouxingtao@2021/07/01
    ************************************/
    C_API AlgoError C_CALL LedLumToBuffer(AlgoHandle lum_handle, char* out_buffer, uint64_t* in_out_len);


    /************************************
    @brief  : 将buffer数据导入到lum
    @note   :
    @param  : lum_handle   有效的lum句柄
    @param  : in_buffer
    @param  : in_len
    @return	: AlgoError
    @Author : ouxingtao@2021/07/01
    ************************************/
    C_API AlgoError C_CALL LedLumFromBuffer(AlgoHandle lum_handle, const char* in_buffer, uint64_t in_len);




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
    C_API AlgoError C_CALL LedLumRelease(AlgoHandle lum_handle);
    C_API void C_CALL LedLumDestroyHandle(AlgoHandle lum_handle);







#ifdef __cplusplus
}
#endif // __cplusplus
#endif