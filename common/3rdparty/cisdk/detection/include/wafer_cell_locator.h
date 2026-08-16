#ifndef ____WAFER_CELL_LOCATOR_H__
#define ____WAFER_CELL_LOCATOR_H__

#include "ci_declare.h"
#include "ci_define.h"
#include "ci_data.h"
#include "wafer_cell_locator_data.h"
#include "wafer_cell_locator_errors.h"
#include "algo_mat.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


    /************************************
    @brief    : 创建句柄
    @note     : 
    @return   : AlgoHandle
    @Modified : songshaojun@2023/07/03
    ************************************/
	C_API AlgoHandle C_CALL WaferCellCreateHandle();


	/**********************************************
	@brief   : 设置wafer上Cell信息
	@note    :
	@param   : locator_handle   有效的定位句柄
	@param   : in_cell_info     输入cell的信息，详见wafer_cell_pad_locator_data.h
	@return  : AlgoError
	@Author  : songshaojun@2023/07/02
	***********************************************/
	C_API AlgoError C_CALL WaferCellSetInfo(AlgoHandle locator_handle, WaferCellPadInfo* in_cell_info);


	/********************************************
	@brief   :将计算得到的像素坐标转换到载台坐标
	@note    :
			  
	@param   : locator_handle，有效的定位句柄
	@param   : in_trans_mat1，转换矩阵1，即线阵相机自身的坐标转换矩阵
	@param   : in_trans_mat2，转换矩阵2，坐标转换矩阵
	@param   : in_cam_plat_coordinate，线阵相机在载台坐标系中的位置
	@param   : lin_cam_width，线扫相机宽度
	@param   : in_plat_coordinate，当前载台在载台坐标系中的位置
	@return  : trans_idx，需要转换的坐标在当前列线扫样本中是第几行
	@Author  : songshaojun@2023/10/25
	*********************************************/
	C_API AlgoError C_CALL WaferCellCoordinateTrans(AlgoHandle locator_handle, AlgoMat* in_trans_mat1,AlgoMat* in_trans_mat2,
		AlgoPoint in_cam_plat_coordinate,int lin_cam_width,
		AlgoPoint* in_plat_coordinate,int trans_idx);

	/********************************************
	@brief   :将计算得到的像素坐标转换到载台坐标
	@note    :将转换完成的坐标暴露出来
			  
	@param   : locator_handle，有效的定位句柄
	@param   : in_trans_mat1，转换矩阵1，即线阵相机自身的坐标转换矩阵
	@param   : in_trans_mat2，转换矩阵2，坐标转换矩阵
	@param   : in_cam_plat_coordinate，线阵相机在载台坐标系中的位置
	@param   : lin_cam_width，线扫相机宽度
	@param   : in_plat_coordinate，当前载台在载台坐标系中的位置
	@return  : trans_idx，需要转换的坐标在当前列线扫样本中是第几行
	@Author  : songshaojun@2023/10/25
	*********************************************/
	C_API AlgoError C_CALL WaferCellCoordinateTrans2(AlgoHandle locator_handle, AlgoMat* in_trans_mat1,AlgoMat* in_trans_mat2,
		AlgoPoint in_cam_plat_coordinate,int lin_cam_width,
		AlgoPoint* in_plat_coordinate,int trans_idx,int rows_num, int col_idx);








	/********************************************
	@brief   :将计算得到的像素坐标转换到载台坐标
	@note    :作为工具，不从handle里面取数据和存数据			  
	@param   : locator_handle，有效的定位句柄
	@param   : in_trans_mat1，转换矩阵1，即线阵相机自身的坐标转换矩阵
	@param   : in_trans_mat2，转换矩阵2，坐标转换矩阵
	@param   : in_cam_plat_coordinate，线阵相机在载台坐标系中的位置
	@param   : lin_cam_width，线扫相机宽度
	@param   : in_plat_coordinate，当前载台在载台坐标系中的位置
	@param   : in_out_pad_result,需要转换的pad点坐标
	@param   : in_out_vertex_result,需要转换的cell顶点坐标
	@Author  : songshaojun@2023/10/25
	*********************************************/
	C_API AlgoError C_CALL WaferCellCoordinateTransNew(AlgoMat* in_trans_mat1,AlgoMat* in_trans_mat2,
		AlgoPoint in_cam_plat_coordinate,int lin_cam_width,
		AlgoPoint* in_plat_coordinate,AlgoMat* in_out_pad_result,AlgoMat* in_out_vertex_result);





	/********************************************
	@brief   : 设置两组相邻pad点合并为同一组pad点的阈值
	@note    : 当两组pad点距离较近时，将其合并为同一组
			   两组pad点是指因样本切割，原本就是属于一组pad的数据被分成了两组
	@param   : locator_handle     有效的定位句柄
	@param   : in_merge_thresh    根据pad组的高度确定
	@return  : AlgoError
	@Author  : songshaojun@2023/07/03
	*********************************************/
	C_API AlgoError C_CALL WaferCellSetMergeTh(AlgoHandle locator_handle, int in_merge_thresh);
	//C_API AlgoError C_CALL WaferCellSetMergeTh(AlgoHandle locator_handle, int in_merge_thresh,int in_segment_thresh,float in_match_thresh);


	/******************************************
	@brief   : 给单张的测试样本加偏移，主要是给pad点，cell顶点加偏移
	@note    : 加完片以后，直接存储在算法内部！
	@param   : locator_handle          有效的定位句柄
	@param   : offset_info             给样本结果加偏移需要的参数结构体，详见wafer_cell_locator_data.h
	@param   : in_out_pad_result       输入的单张样本pad结果
	@param   : in_out_vertex_result    输入的单张样本vertex结果
	@Author  : songshaojun@2023/08/16
	*******************************************/
	C_API AlgoError C_CALL WaferCellAddOffSet(AlgoHandle locator_handle, WaferCellOffSetInfo* offset_info, 
		AlgoMat* in_pad_result, AlgoMat* in_vertex_result);



	/******************************************
	@brief   : 给单张的测试样本加偏移，主要是给pad点，cell顶点加偏移，没有handle
	@note    : 加完偏移后，直接将加完偏移的结果吐出去
	@param   : offset_info             给样本结果加偏移需要的参数结构体，详见wafer_cell_locator_data.h
	@param   : in_out_pad_result       输入的单张样本pad结果
	@param   : in_out_vertex_result    输入的单张样本vertex结果
	@Author  : songshaojun@2023/08/16
	*******************************************/
	C_API AlgoError C_CALL WaferCellPadVertexAddOffSet(WaferCellOffSetInfo* offset_info, 
		AlgoMat* in_pad_result, AlgoMat* in_vertex_result);


	/******************************************
		@brief   : 计算所有样本的最终检测结果,计算哪些pad点和cell点同属于一个cell
		@note    :
		@param   : locator_handle               有效的定位句柄
		@param   : mech_move_acc                机械移动精度，换算为像素个数
		@param   : out_cell_pad_result			吐出的cell上pad点的结果
		@param   : out_cell_vertex_result       吐出的cell上顶点的结果
		@note    :

		//顶点的数据结构形式,out_cell_vertex_result
		N*14 ,N代表有几个Cell
		0   1  2  3  4  5  6  7      8               9        10         11        12          13
		x0,y0,x1,y1,x2,y2,x3,y3,cell_center_x,cell_center_y,cell_row,cell_col,cell_corner_num,cell_idx;

		//pad点的数据结构形式,out_cell_pad_result
		M*7 ，任意一行是一个pad点
		0 1 2 3 4     5       6
		x,y,w,h,α,group_idx,cell_idx;

		@Author  : songshaojun@2023/08/16
		*******************************************/
	C_API AlgoError C_CALL WaferCellMergeResult(AlgoHandle locator_handle,
		const float mech_move_acc, AlgoMat *out_cell_pad_result, AlgoMat *out_cell_vertex_result);


	/******************************************
	@brief   : 计算所有样本的最终检测结果,计算哪些pad点和cell点同属于一个cell
	@note    :
	@param   : locator_handle               有效的定位句柄
	@param   : mech_move_acc                机械移动精度
	@param   : out_cell_pad_result_num	    计算得到的Cell个数
	@Author  : songshaojun@2023/08/16
	*******************************************/
	C_API AlgoError C_CALL WaferCellMergeResultNew(AlgoHandle locator_handle,
		const float mech_move_acc, int *out_cell_pad_result_num);


	/******************************************
    @brief   : 获取所有样本的拼接结果
    @note    :
    @param   : locator_handle               有效的定位句柄
    @param   : mech_move_acc                机械移动精度，换算为像素个数
    @param   : out_all_cell			        所有检测到的cell结果，数据结构类型为WaferOneCell
    @Author  : songshaojun@2023/08/16
    *******************************************/
	C_API AlgoError C_CALL WaferCellGetMergeResult(AlgoHandle locator_handle,WaferOneCell *out_all_cell);


	///********************buffer相关的
	/************************************
	@Brief		：	载入和保存cellpad定位和拼接结果
	@Note		：	文件名后缀必须为.cell
	@Parameter	: 	locator_handle		---区域定位句柄
	@Parameter	: 	in_locator_fullname		--文件后缀必须为.cell
	@Returns	:   AlgoError
	@Author		：	songshaojun@2023/11/21
	************************************/
	C_API AlgoError C_CALL WaferCellLoad(AlgoHandle locator_handle, const char* in_locator_fullname);
	C_API AlgoError C_CALL WaferCellSave(AlgoHandle locator_handle, const char* in_locator_fullname);






	
	/******************************************
	@brief   : 从最终计算的结果里，筛选出pad点亮cell需要的点
	@note    : in_cell_pad_result		   送进来的cell上pad点的结果
	@param   : in_cell_vertex_result       送进来的cell上顶点的结果
	@param   : select_flag                 筛选标志位
	@param   : max_pad_size                最大pad点的大小
	@param   :  out_cell_pad_result			吐出的筛选过的cell上pad点的结果
	@note    :  out_cell_vertex_result       吐出的筛选过的cell上顶点的结果 


	@Author  : songshaojun@2023/08/16
	*******************************************/
	//C_API AlgoError C_CALL WaferCellSelectMergeResult(AlgoMat *in_cell_pad_result, AlgoMat *in_cell_vertex_result,int select_flag,AlgoSize max_pad_size,
	//	AlgoMat *out_cell_pad_result,AlgoMat *out_cell_vertex_pad_result);


	/******************************************
	@brief   : 导出所有结果的定位结果
	@note    : 仅供测试使用!!! 
	@param   : locator_handle       有效的定位句柄
	@param   : out_result_dir       结果存放路径
	@Author  : songshaojun@2023/08/16
	*******************************************/
	//C_API AlgoError C_CALL WaferCellExportCellResult(AlgoHandle locator_handle, const char* out_result_dir);

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
	C_API AlgoError C_CALL WaferCellRelease(AlgoHandle locator_handle);
	C_API void C_CALL WaferCellDestroyHandle(AlgoHandle locator_handle);





#ifdef __cplusplus
}
#endif // __cplusplus
#endif