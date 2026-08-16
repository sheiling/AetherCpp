#ifndef __LED_IRREGULAR_CONTOUR_SEARCHER_H__
#define __LED_IRREGULAR_CONTOUR_SEARCHER_H__

#include "ci_define.h"
#include "ci_declare.h"
#include "led_irregular_contour_searcher_data.h"
#include "led_irregular_contour_searcher_errors.h"
#include "meta_info_enum.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

    /************************************
    @brief  : 初始化句柄
    @note   :
    @param  : ir_contour_handle												灯点边缘句柄
    @return	: AlgoError
    @Author : GuoHao@2024/12/11
    ************************************/
    C_API AlgoError C_CALL LedIrContourCreateHandle(AlgoHandle* ir_contour_handle);
	 
	
	/************************************
    @brief  : 设置异形框类型（若不设置，默认类型为IrregularContourType::IrregularContourType_ROTATE_RECT）
    @note   : 
				算法会根据设置的异形框类型，自适应地选择对应搜索方法与角点数量
					类型定义与解释见led_irregular_contour_searcher_data.h中枚举变量IrregularContourType；
					球形屏球身校正：推荐使用类型为IrregularContourType::IrregularContourType_ROTATE_RECT；

    @param  : ir_contour_handle												灯点边缘句柄
    @param  : in_ir_contour_type											异形搜索框类型
    @return	: AlgoError
    @Author : GuoHao@2024/12/11
    ************************************/
    C_API AlgoError C_CALL LedIrContourSetIrregularType(AlgoHandle ir_contour_handle, IrregularContourType in_ir_contour_type);
	
	/************************************
	@brief  : 设置是否要基于shot图像重新定位灯点亚像素中心的开关（不设置时该开关默认为关）
	@note   : 
				true：开启基于shot计算亚像素中心的开关，此时必须在调用search的时候入参真实的、用于提取光通量的shot；
				false：关闭基于shot计算亚像素中心的开关

	@param  : ir_contour_handle												灯点边缘句柄
	@param  : in_compute_subpixel											重新计算亚像素中心的开关
	@return	: AlgoError
	@Author : GuoHao@2024/12/11
	************************************/
	C_API AlgoError C_CALL LedIrContourSetComSubPixelSwitch(AlgoHandle ir_contour_handle, bool in_compute_subpixel);

	/************************************
	@brief  : 计算异形灯点框
	@note   :
				in_point_type可以是以下几种：
					ALGOMOD_LED_POINT_LOCATOR，旧有的灯点定位模块的句柄，暂未开发
					ALGOMOD_POINT_MAPPER，新设计的常规灯点排列句柄，暂未开发
					ALGOMOD_POINT_IRREGULAR_MAPPER，新设计的异形屏灯点排列句柄，目前仅支持该类句柄，用于球形屏校正

	@param  : ir_contour_handle												灯点边缘句柄
	@param  : in_pure_shot													如果需要重新计算亚像素中心，该句柄必须为真实有效的用于提取光通量的shot
	@param  : in_ledpoint													有效的灯点排列结果
	@param  : in_point_type													灯点排列结果的类型
	@return	: AlgoError
	@Author : GuoHao@2024/12/11
	************************************/
	C_API AlgoError C_CALL LedIrContourSearch(AlgoHandle ir_contour_handle, const AlgoHandle in_pure_shot, const AlgoHandle in_ledpoint, int in_point_type = ALGOMOD_POINT_IRREGULAR_MAPPER);


	/************************************
	@brief  : 导出灯点框结果对应的图像
	@note   :
				in_point_type可以是以下几种：
					ALGOMOD_LED_POINT_LOCATOR，旧有的灯点定位模块的句柄，暂未开发
					ALGOMOD_POINT_MAPPER，新设计的常规灯点排列句柄，暂未开发
					ALGOMOD_POINT_IRREGULAR_MAPPER，新设计的异形屏灯点排列句柄，目前仅支持该类句柄，用于球形屏校正

	@param  : ir_contour_handle												灯点边缘句柄
	@param  : in_fullname													图像的保存路径与名称，推荐保存为png格式
	@param  : in_scale														缩放比例，推荐值为3或者5
	@param  : is_split														为true时取四角及中间区域拼接,减少图像大小，否则不做处理
	@param  : in_pure_shot													在shot上画框，默认null=在LedIrContourSearch输入的shot上画, 不为null=在此shot上画
	@return	: AlgoError
	@Author : GuoHao@2024/12/11
	************************************/
	C_API AlgoError C_CALL LedIrContourExportContourImage(AlgoHandle ir_contour_handle, const char* in_fullname, int in_scale = 5, 
		bool is_split = true, const AlgoHandle in_pure_shot = nullptr); 
	

    /************************************
    @brief  : 获取灯点边缘搜寻结果
    @note   : 
    @param  : ir_contour_handle												灯点边缘句柄
    @param  : out_led_ir_contour											导出的结构体,必须是已经初始化完毕的，尤其是其中的corners
    @param  : in_is_deep_copy												是否深拷贝, 默认浅拷贝  1=深拷贝;0=浅拷贝
    @return	: AlgoError
  	@Author : GuoHao@2024/12/11
    ************************************/
    C_API AlgoError C_CALL LedIrContourGet(AlgoHandle ir_contour_handle, AlgoLedIrContour* out_led_ir_contour, int in_is_deep_copy = 0);


    /************************************
    @brief  : 载入和保存灯点边缘数据
    @note   : 后缀必须是.ircontour
    @param  : ir_contour_handle    灯点边缘句柄
    @param  : in_fullname       文件名后缀必须是.ircontour
    @return	: AlgoError
	@Author : GuoHao@2024/12/11
    ************************************/
    C_API AlgoError C_CALL LedIrContourLoad(AlgoHandle ir_contour_handle, const char* in_fullname);
    C_API AlgoError C_CALL LedIrContourSave(AlgoHandle ir_contour_handle, const char* in_fullname);


    /************************************
    @brief  : 获取buffer大小
    @note   :
    @param  : vig_handle  当前有效句柄
    @param  : out_len     buffer大小
    @return	: AlgoError
	@Author : GuoHao@2024/12/11
    ************************************/
    C_API AlgoError C_CALL LedIrContourGetBufferSize(AlgoHandle ir_contour_handle, uint64_t* out_len);


    /************************************
    @brief  : 将contour里面的数据导出到buffer
    @note   :
    @param  : ir_contour_handle    有效的contour句柄
    @param  : in_out_buffer     已经申请好申请空间
    @param  : in_out_len        外部申请好的空间大小,会用真实大小修改
    @return	: AlgoError
	@Author : GuoHao@2024/12/11
    ************************************/
    C_API AlgoError C_CALL LedIrContourToBuffer(AlgoHandle ir_contour_handle, char* in_out_buffer, uint64_t* in_out_len);


    /************************************
    @brief  : 将buffer数据导入到contour
    @note   :
    @param  : ir_contour_handle   有效的contour句柄
    @param  : in_buffer
    @param  : in_len
    @return	: AlgoError
	@Author : GuoHao@2024/12/11
    ************************************/
    C_API AlgoError C_CALL LedIrContourFromBuffer(AlgoHandle ir_contour_handle, const char* in_buffer, uint64_t in_len);


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
    C_API AlgoError C_CALL LedIrContourRelease(AlgoHandle ir_contour_handle);
    C_API void C_CALL LedIrContourDestroyHandle(AlgoHandle ir_contour_handle);







#ifdef __cplusplus
}
#endif // __cplusplus
#endif