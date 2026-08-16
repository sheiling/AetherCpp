#ifndef __point_irregular_mapper_H__
#define __point_irregular_mapper_H__


#include "ci_define.h"
#include "ci_declare.h"
#include "point_irregular_mapper_errors.h"
#include "point_irregular_mapper_data.h"
#include "meta_info_enum.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


	/************************************
	@brief  : 初始化句柄
	@note   :
	@param  : in_out_point_irregualr_map_handle
	@return	: AlgoError
	@Author : GuoHao@2024/07/29
	************************************/
	C_API AlgoError C_CALL PointIrMapCreateHandle(AlgoHandle* in_out_point_irregualr_map_handle);

	/************************************
	@brief  : 设置异形屏类型
	@note   : 必须调用，说明类型见point_irregular_mapper_data.h；
	@param  : point_irregular_map_handle			有效的灯点排布句柄
	@param  : in_ir_type							异形屏体类型
														IRREGULAR_TYPE_POINT_RECT	= 0b00000000,	// 灯点是规则的按照矩形排布，即灯点都同行同列，如经裁切后的常规矩形屏
														IRREGULAR_TYPE_POINT_ROW	= 0b00000001,	// 灯点是规则的按照行排布，即灯点都同行不同列
	@return	: AlgoError
	@Author : HuangZiming@2024/11/22
	************************************/
	C_API AlgoError C_CALL PointIrMapSetIrregularType(AlgoHandle point_irregular_map_handle, IrregularType in_ir_type);

	/************************************
	@brief  : 设置大表信息
	@note   : 当不设置大表时，针对常规屏体的裁切，生成的点位置是有效灯点的最小外接矩形。
			  设置了大表后，会将提取有效灯点的最小外接矩形放置到大表的有效区域处（大表最小外接矩形的xy处），无严格判断。
	@param  : point_irregular_map_handle			有效的灯点排布句柄
	@param  : in_tab								大表信息，类型为ALGO_8UC1。
	@return	: AlgoError
	@Author : Tianbo@2024/07/29
	************************************/
	C_API AlgoError C_CALL PointIrMapSetTab(AlgoHandle point_irregular_map_handle, AlgoMat* in_tab);

	/************************************
	@brief  : 设置折线场景下的折线边数
	@note   : 当球形屏的球顶是正多边形且边数小于16的时候，需要调用该接口设置当前分区的折线边数，内部排列时就会使用折线排列方式
	@param  : point_irregular_map_handle			有效的灯点排布句柄
	@param  : in_sides_num							边数
	@return	: AlgoError
	@Author : Tianbo@2024/07/29
	************************************/
	C_API AlgoError C_CALL PointIrMapSetSideNum(AlgoHandle point_irregular_map_handle, int in_sides_num);

	/************************************
	@brief  : 设置球型屏排列先验信息，获取打屏点定位结果并按行列进行拆分作为先验信息
	@note   : 球型屏灯点排列时必须调用，球身球顶的辅助图像都是两类，分别是辅助行、辅助列
	@param  : point_irregular_map_handle			有效的灯点排布句柄
	@param  : point_find_handle			            有效的灯点搜索句柄
	@param  : in_pattern_type						打屏pattern类型  todo 暂未找到合适的枚举名称，后续增加pattern类型后再集成为枚举类型
														AUXILIARY_ROW = 0，辅助行
														AUXILIARY_COL = 1，辅助列
	@return	: AlgoError
	@Author : GuoHao@2025/08/08
	************************************/
	C_API AlgoError C_CALL PointIrMapSetAuxiliaryMapper(AlgoHandle point_irregular_map_handle, AlgoHandle point_find_handle, const AuxiliaryMapperType in_pattern_type);

	/************************************
	@brief  : 设置球形屏排列需要提供的分区形状信息
	@note   : 球型屏灯点排列时必须调用，目前主要有两种类型
				当行灯点轨迹为不封闭曲线时，入参PartitionShape::PARTITION_SHAPE_SPHERICAL_TRAPEZIUM（可兼容球身、常规屏体、弧形屏等）
				当行灯点轨迹为封闭曲线时，入参PartitionShape::PARTITION_SHAPE_SPHERICAL_CIRCLE（可兼容球顶、圆环、圆饼屏体等）
	@param  : point_irregular_map_handle			有效的灯点排布句柄
	@param  : in_part_shape							屏体形状，该参数有以下两种情况：
	@return	: AlgoError
	@Author : GuoHao@2025/08/08
	************************************/
	C_API AlgoError C_CALL PointIrMapSetPartitionShape(AlgoHandle point_irregular_map_handle, const PartitionShape in_part_shape);


	/************************************
	@brief  : 针对异形屏体进行灯点排布计算
	@note   : 当前支持常规矩形屏裁切得到的异形屏体（即灯点矩形排布、屏体外轮廓形状不规则）和行方向弧线排布的球型屏体
	@param  : point_irregular_map_handle			有效的灯点排布句柄
	@param  : in_point_find_handle					有效的灯点搜索句柄
	@param  : in_part_w_h							灯点行列数量：即裁切后灯点最小外接矩形的行列数
	@return	: AlgoError
	@Author : GuoHao@2024/07/29
	************************************/
	C_API AlgoError C_CALL PointIrMapCompute(AlgoHandle point_irregular_map_handle, AlgoHandle in_point_find_handle, AlgoSize in_part_w_h);

	/************************************
	@Brief		：	载入和保存定位数据
	@Note		：	文件名后缀必须是.irmap
	@Parameter	: 	point_irregular_map_handle		有效的灯点排布句柄
	@Parameter	: 	in_pts_fullname		--文件名后缀必须是.irmap
	@Returns	:   AlgoError
	@Author		：	GuoHao@2024/07/29
	************************************/
	C_API AlgoError C_CALL PointIrMapLoad(AlgoHandle point_irregular_map_handle, const char* in_pts_fullname);
	C_API AlgoError C_CALL PointIrMapSave(AlgoHandle point_irregular_map_handle, const char* in_pts_fullname);

	/************************************
	@brief  : 导出排布的结果图像
	@note   : 黄色灯点是死灯
	@param  : point_irregular_map_handle	--未释放的点定位句柄
	@param  : in_png_fullname				--导出路径与文件名
	@param  : in_scale						--放大倍数建议3或5,默认=1不放大
	@param  : in_shot_handle         		--有效的shot句柄，用于将irmap排列结果绘制在shot图像上，shot.image不能为空
	@return	: AlgoError
	@Author : GuoHao@2024/07/30
	************************************/
	C_API AlgoError C_CALL PointIrMapExportMapImage(AlgoHandle point_irregular_map_handle, const char* in_png_fullname, float in_scale = 1.0f, const AlgoHandle in_shot_handle = nullptr);

	/************************************
	@brief  : 导出灯点状态图
	@note   : 红色=死灯,黑色=物理不存在;绿色=活灯;
	@param  : point_irregular_map_handle	--有效点定位句柄
	@param  : in_png_fullname				--导出路径与文件名
	@param  : in_scale						--放大倍数建议3或5,默认=3放大
	@return : AlgoError
	@Author : GuoHao@2024/07/30
	************************************/
	C_API AlgoError C_CALL PointIrMapExportStateImage(AlgoHandle point_irregular_map_handle, const char* in_png_fullname, int in_scale = 3);

	/************************************
	@brief  : 获取buffer大小
	@note   :
	@param  : handle  当前有效句柄
	@param  : out_len     buffer大小
	@return	: AlgoError
	@Author :	GuoHao@2024/07/29
	************************************/
	C_API AlgoError C_CALL PointIrMapGetBufferSize(AlgoHandle point_irregular_map_handle, uint64_t* out_len);

	/************************************
	@brief  : 将模块数据导出到buffer
	@note   :
	@param  : handle   有效的句柄
	@param  : out_buffer        已经申请好申请空间
	@param  : in_out_len        外部申请好的空间大小,会用真实大小修改
	@return	: AlgoError
	@Author : GuoHao@2024/07/29
	************************************/
	C_API AlgoError C_CALL PointIrMapToBuffer(AlgoHandle point_irregular_map_handle, char* out_buffer, uint64_t* in_out_len);

	/************************************
	@brief  : 将buffer数据导入到模块
	@note   :
	@param  : handle   有效的句柄
	@param  : in_buffer
	@param  : in_len
	@return	: AlgoError
	@Author : GuoHao@2024/07/29
	************************************/
	C_API AlgoError C_CALL PointIrMapFromBuffer(AlgoHandle point_irregular_map_handle, const char* in_buffer, uint64_t in_len);

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
	@Author : GuoHao@2024/07/29
	 ************************************/
	C_API AlgoError C_CALL PointIrMapRelease(AlgoHandle handle);
	C_API void C_CALL PointIrMapDestroyHandle(AlgoHandle handle);



#ifdef __cplusplus
}
#endif // __cplusplus
#endif