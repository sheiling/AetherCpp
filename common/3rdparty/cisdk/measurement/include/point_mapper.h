#ifndef ____POINT_MAPPER_H__
#define ____POINT_MAPPER_H__

#include "ci_declare.h"
#include "ci_define.h"
#include "ci_data.h"
#include "point_mapper_data.h"
#include "point_mapper_errors.h"
#include "algo_mat.h"
#include "meta_info_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


	/************************************
	@brief  : 创建检测句柄
	@note   : 
	@param  : out_mapper_handle, 输出量, 返回的mapper句柄
	@return : AlgoError
	@author : songshaojun@2024/12/17
	************************************/
	C_API AlgoError C_CALL PointMapCreateHandle(AlgoHandle* out_mapper_handle);


	/************************************
    @brief  : 初始化线程数句柄
    @note   : 不调用此函数, 模块默认 = CPU线程数-1
    @param  : pointmap_handle, 输入量, 有效的排列句柄
    @param  : in_omp_num, 输入量, 并行运算线程个数
    @return	: AlgoError
	@author : songshaojun@2024/09/06
    ************************************/
    C_API AlgoError C_CALL PointMapSetDefaultThreadsNum(AlgoHandle pointmap_handle, int in_omp_num);


	/************************************
	@brief  : 设置灯点排列方法
	@note   : 现阶段有两种方法，区别与边界如下：
				
	@param  : pointmap_handle						--输入量, 有效的排列句柄
	@param  : in_method								--输入量, 排列方法，具体边界详见doc对应交付文档
	@return	: AlgoError
	@author : GuoHao@2025/09/04    
	************************************/
	C_API AlgoError C_CALL PointMapSetMapMethod(AlgoHandle pointmap_handle, const PointMapMethod in_method);
	

	/************************************
    @brief  : 设置点的排列形式及排布行列数，目前支持三角排列，矩形排列
    @note   : 
    @param  : pointmap_handle						--输入量, 有效的排列句柄
    @param  : in_type								--输入量, 排列类型, 详情见 meta_info_enum.h
	@param  : in_array_size							--输入量, 点集排列行列数, 输入形式为：宽度*高度
    @return	: AlgoError
	@author : songshaojun@2024/09/06
    ************************************/
    C_API AlgoError C_CALL PointMapSetMapInfo(AlgoHandle pointmap_handle, const LedLayout in_type, const AlgoSize in_array_size);


	/************************************
    @brief  : 设置输入点集
    @note   : 输入为AlgoMat形式
    @param  : pointmap_handle, 
	@param  : in_point_finder_handle, 输入量, 点集,n行1列2通道的一个AlgoMat,其中第一个通道为x坐标,第二个通道为y坐标,AlgoMat的类型必须为ALGO_32FC2;
    @return	: AlgoError
	@author : songshaojun@2024/09/06
    ************************************/
    C_API AlgoError C_CALL PointMapSetInputFromMat(AlgoHandle pointmap_handle, const AlgoMat in_point_finder_handle);


	/************************************
    @brief  : 设置输入点集
    @note   : 输入为AlgoHandle形式
    @param  : pointmap_handle, 输入量, 有效的排列句柄
	@param  : in_points,输入量, 有效point_finder模块句柄
    @return	: AlgoError
	@author : songshaojun@2024/09/06
    ************************************/
    C_API AlgoError C_CALL PointMapSetInputFromHandle(AlgoHandle pointmap_handle, const AlgoHandle in_points);


	///************************************
    //@brief  : 设置参数
    //@note   : 默认不要调用，当灯点行与行，列与列之间的间隔不同时，再调用该接口
    //@param  : pointmap_handle, 输入量, 有效的排列句柄
	//@param  : in_points_on_curve_th, 输入量, 判断点是否位于曲线上的阈值
	//@param  : in_row_mutation_th, 输入量, 判断是否缺失整行的阈值
	//@param  : in_col_mutaion_th, 输入量, 判断是否缺失整列的阈值
    //@return	: AlgoError
	//@author : songshaojun@2024/09/06
    //************************************/
	//C_API AlgoError C_CALL PointMapSetPara(AlgoHandle pointmap_handle, const float in_points_on_curve_th, const float in_row_mutation_th, const float in_col_mutaion_th);


	/************************************
    @brief  : 对输入点集进行排列
    @note   : 
    @param  : pointmap_handle, 输入量, 有效的排列句柄
    @return	: AlgoError
	@author : songshaojun@2024/09/06
    ************************************/
    C_API AlgoError C_CALL PointMapMapping(AlgoHandle pointmap_handle);


	/************************************
    @Brief	：载入和保存排列
    @Note	：文件名后缀必须是.map
    @param	: pointmap_handle, 输入量, 排列句柄
    @param	: in_map_fullname, 输入量, 文件名后缀必须是.map
    @return : AlgoError
	@author	: songshaojun@2024/09/06
    ************************************/
    C_API AlgoError C_CALL PointMapLoad(AlgoHandle pointmap_handle, const char* in_map_fullname);
    C_API AlgoError C_CALL PointMapSave(AlgoHandle pointmap_handle, const char* in_map_fullname);


	/************************************
    @brief  : 获取排列结果
    @note   :
    @param  : pointmap_handle, 输入量, 有效的排列句柄
    @param  : out_pointmap_result, 输出量, 导出的结构体,必须已经初始化完毕,数据类型详见 point_mapper_data.h
    @param  : is_deep_copy, 输入量, 是否深拷贝, 默认浅拷贝  1=深拷贝;0=浅拷贝
    @return	: AlgoError
	@author	: songshaojun@2024/09/06
    ************************************/
    C_API AlgoError C_CALL PointMapGet(AlgoHandle pointmap_handle, PointMapResult* out_pointmap_result, int in_is_deep_copy = 0);

	/************************************
    @brief  : 设置排列结果
    @note   :
    @param  : pointmap_handle, 输入量,有效的排列句柄
    @param  : in_pointmap_result, 输入量, 结果数据
    @return	: AlgoError
	@author	: songshaojun@2024/09/06
    ************************************/
    C_API AlgoError C_CALL PointMapSet(AlgoHandle pointmap_handle, const PointMapResult* in_pointmap_result);


	/************************************
    @brief  : 导出排列结果图像
    @note   : 
    @param  : pointmap_handle,	输入量，有效的排列句柄
	@param  : in_fullname,		输入量，导出路径及名称
	@param  : in_drawon,		输入量，将点画在此图像上
	@param  : in_is_show,		输入量，0=不显示排列外其他信息,1=显示其他信息
    @param  : int in_scale,		输入量，放大倍数
    @return	: AlgoError
	@author	: songshaojun@2024/09/06
    ************************************/
    C_API AlgoError C_CALL PointMapExportMappingImage(AlgoHandle pointmap_handle, const char* in_fullname, AlgoMat* in_drawon = nullptr, const int in_is_show = 1, int in_scale = 1);


	/************************************
    @brief    : 导出排列状态图
    @note     : 红色=目标可能没找到,黑色=物理不存在;绿色=目标存在;
    @param    : pointmap_handle, 输入量，有效的排列句柄
    @param    : in_fullname, 输入量，导出路径及名称
    @return   : AlgoError
	@author	  : songshaojun@2024/09/06
    ************************************/
    C_API AlgoError C_CALL PointMapExportMappingState(AlgoHandle pointmap_handle, const char* in_fullname);


	/************************************
	@Brief		：	检查Maphandle内的state属性是否都是LIVE（正常灯）
	@Note		：
	@Parameter	: 	pointmap_handle	mapper位句柄
	@Returns	:   true表示全是正常灯， false表示handle输入不正常或内部存在非正常灯（eg：死灯）
	@Author		：	songshaojun@2025/05/29
	************************************/
	C_API bool C_CALL PointMapCheckAllStateLive(AlgoHandle pointmap_handle);


	/************************************
    @brief  : 获取buffer大小
    @note   :
    @param  : pointmap_handle, 输入量 有效的排列句柄
    @param  : out_len, 输出量, buffer大小
    @return	: AlgoError
	@author	: songshaojun@2024/09/06
    ************************************/
    C_API AlgoError C_CALL PointMapGetBufferSize(AlgoHandle pointmap_handle, uint64_t* out_len);


	/************************************
	@brief  : 将ledpoint里面的数据导出到buffer
	@note   :
	@param  : ledpoint_handle   有效的pointmap_handle句柄
	@param  : out_buffer        已经申请好申请空间
	@param  : in_out_len        外部申请好的空间大小,会用真实大小修改
	@return	: AlgoError
	@author	: songshaojun@2024/09/06
	************************************/
	C_API AlgoError C_CALL PointMapToBuffer(AlgoHandle pointmap_handle, char* out_buffer, uint64_t* in_out_len);


	/************************************
	@brief  : 将buffer数据导入到ledpoint
	@note   :
	@param  : ledpoint_handle   有效的pointmap_handle句柄
	@param  : in_buffer
	@param  : in_len
	@return	: AlgoError
	@author	: songshaojun@2024/09/06
	************************************/
	C_API AlgoError C_CALL PointMapFromBuffer(AlgoHandle pointmap_handle, const char* in_buffer, uint64_t in_len);



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
    C_API AlgoError C_CALL PointMapRelease(AlgoHandle pointmap_handle);
    C_API void C_CALL PointMapDestroyHandle(AlgoHandle pointmap_handle);


#ifdef __cplusplus
}
#endif // __cplusplus
#endif