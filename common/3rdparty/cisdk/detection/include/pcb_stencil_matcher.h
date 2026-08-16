#ifndef ____PCB_STENCIL_MATCHER_H__
#define ____PCB_STENCIL_MATCHER_H__

#include "ci_declare.h"
#include "ci_define.h"
#include "ci_data.h"
#include "pcb_stencil_matcher_errors.h"
#include "pcb_stencil_matcher_data.h"
#include "algo_mat.h"


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

    //PCB钢网匹配有


    /************************************
    @brief    : 创建句柄
    @note     :
    @return   : 
    @Modified : haopanxia@2024/06/06
    ************************************/
    C_API AlgoHandle C_CALL PCBStencilCreateHandle();

    /************************************
    @brief    : 设置PCBStencil检测flag信息
    @note     : 
    @param    : match_handle                 有效的PCBStencil句柄
    @param    : in_flag                      PCBStencil数据匹配策略 ,详见pcb_stencil_matcher_data.h
    @return   : AlgoError
    @Modified : haopanxia@2024/06/06
    ************************************/
    C_API AlgoError C_CALL PCBStencilSetFlag(AlgoHandle match_handle, PCBStencilMatchFlag in_flag);


	/************************************
	@brief    : 设置每张图像所在空间信息
	@note     : 设置每张图像所在空间信息,详见PCBStenciSpaceInfo结构体
	@param    : match_handle                 有效的PCBStencil句柄
	@param    : in_space_Info                输入的空间信息，详见PCBStenciSpaceInfo结构体
	@return   : AlgoError
	@Modified : wushaoshuang@2024/06/18
	************************************/
	C_API AlgoError C_CALL PCBStencilSetSpaceInfo(AlgoHandle match_handle, const PCBStencilSpaceInfo *in_space_Info);

	/************************************
	@brief    : 设置机械坐标类型
	@note     : 设置机械坐标类型，目前支持两种坐标类型，默认为PCBStencilCoordType枚举中的第一种。
	@param    : match_handle                 有效的PCBStencil句柄
	@param    : in_coord_type                机械坐标类型，详见PCBStencilCoordType枚举
	@return   : AlgoError
	@Modified : wushaoshuang@2024/06/12
	************************************/
	C_API AlgoError C_CALL PCBStencilSetMachineCoord(AlgoHandle match_handle, const PCBStencilCoordType *in_coord_type);

	/************************************
	@brief    : 设置相机信息
	@note     : 设置相机信息，详见PCBStencilCameraInfo结构体。
	@param    : match_handle                 有效的PCBStencil句柄
	@param    : in_camera_info               输入的相机信息结构体
	@return   : AlgoError
	@Modified : wushaoshuang@2024/06/18
	************************************/
	C_API AlgoError C_CALL PCBStencilSetCameraInfo(AlgoHandle match_handle, const PCBStencilCameraInfo *in_camera_info);

	/************************************
	@brief    : 每张图像进行坐标转换
	@note     :
	@param    : match_handle           有效的PCBStencil句柄
	@param    : in_img                 原始数据
	@param    : out_img                转换后的数据
	@return   : AlgoError
	@Modified : haopanxia@2024/06/06
	************************************/
	C_API AlgoError C_CALL PCBStencilImageConverse(AlgoHandle match_handle, AlgoMat *in_img, AlgoMat *out_img);


	/************************************
	@brief    : 所有gerber信息进行数据转换
	@note     :
	@param    : match_handle           有效的PCBStencil句柄
	@param    : in_mat_num             这一组Gerber mat的数量
	@param    : in_gerber_group        原始Gerber数据
	@param    : out_gerber_group       转换后的Gerber数据
	@return   : AlgoError
	@Modified : haopanxia@2024/06/06
	************************************/
	C_API AlgoError C_CALL PCBStencilGerberConverse(AlgoHandle match_handle, const int *in_mat_num,
		AlgoMat *in_gerber_group, AlgoMat *out_gerber_group);

	/************************************
	@brief    : Gerber信息与图像中数据进行匹配
	@note     :
	@param    : match_handle           有效的PCBStencil句柄
	@param    : in_mat_num             这一组Gerber mat的数量
	@param    : in_gerber_group        Gerber数据组
	@param    : in_img_group           图像数据组
	@param    : out_match_val          匹配数值
	@return   : AlgoError
	@Modified : haopanxia@2024/06/06
	************************************/
	C_API AlgoError C_CALL PCBStencilMatch(AlgoHandle match_handle, const int *in_mat_num,
		AlgoMat *in_gerber_group, AlgoMat *in_img_group, double *out_match_val);

	/************************************
	@brief    : Gerber信息与图像中数据进行差异计算
	@note     : 计算gerber数据与图像数据差异,调用该接口前请先调用PCBStencilSetFlag接口，设置flag为SINGLE_DIFF_DEVIATION。
	@param    : match_handle           有效的PCBStencil句柄
	@param    : in_mat_num             这一组Gerber mat的数量
	@param    : in_gerber_group        Gerber数据组
	@param    : in_img_group           图像数据组
	@param    : out_diff_val           x、y综合差异数据，以mat形式输出，数量与Gerber mat的数量一致，需要外部new空间来存储
	@param    : out_x_diff             x坐标差异数据，以mat形式输出，数量与Gerber mat的数量一致，需要外部new空间来存储
	@param    : out_y_diff             x坐标差异数据，以mat形式输出，数量与Gerber mat的数量一致，需要外部new空间来存储
	@return   : AlgoError
	@Modified : wushaoshuang@2024/07/05
	************************************/
	C_API AlgoError C_CALL PCBStencilDiffCalc(AlgoHandle match_handle, const int *in_mat_num,
		AlgoMat *in_gerber_group, AlgoMat *in_img_group, AlgoMat *out_diff_val, AlgoMat *out_x_diff, AlgoMat *out_y_diff);
	

	/************************************
	@brief    : 获取PCBStencil检测flag信息
	@note     : 外部根据需求调用
	@param    : match_handle                 有效的PCBStencil句柄
	@param    : out_flag                     输出PCBStencil数据匹配策略 ,详见pcb_stencil_matcher_data.h
	@return   : AlgoError
	@Modified : wushaoshuang@2024/06/13
	************************************/
	C_API AlgoError C_CALL PCBStencilGetFlag(AlgoHandle match_handle, PCBStencilMatchFlag* out_flag);

	/************************************
	@brief    : 获取每张图像所在空间信息
	@note     : 外部根据需求调用
	@param    : match_handle                 有效的PCBStencil句柄
	@param    : out_space_Info               获取图像对应的相关信息，详见PCBStenciSpaceInfo结构体
	@return   : AlgoError
	@Modified : wushaoshuang@2024/06/13
	************************************/
	C_API AlgoError C_CALL PCBStencilGetSpaceInfo(AlgoHandle match_handle, PCBStencilSpaceInfo* out_space_Info);

	/************************************
	@brief    : 获取机械坐标类型
	@note     : 获取机械坐标类型，外部根据需求调用
	@param    : match_handle                 有效的PCBStencil句柄
	@param    : out_coord_type               输出机械坐标类型，详见PCBStencilCoordType枚举
	@return   : AlgoError
	@Modified : wushaoshuang@2024/06/13
	************************************/
	C_API AlgoError C_CALL PCBStencilGetMachineCoord(AlgoHandle match_handle, PCBStencilCoordType* out_coord_type);

	/************************************
	@brief    : 获取相机信息
	@note     : 获取相机信息，外部根据需求调用
	@param    : match_handle                 有效的PCBStencil句柄
	@param    : out_camera_info              输出的相机信息，详见PCBStencilCameraInfo
	@return   : AlgoError
	@Modified : wushaoshuang@2024/06/18
	************************************/
	C_API AlgoError C_CALL PCBStencilGetCameraInfo(AlgoHandle match_handle, PCBStencilCameraInfo* out_camera_info);

	///************************************
	//@brief    : 获取PCBStencil的轮廓点集点数量
	//@note     :
	//@param    : match_handle          有效的PCBStencil句柄
	//@param    : out_len              PCBStencil的轮廓点集的点数量
	//@return   : AlgoError
	//@Modified : haopanxia@2024/06/06
	//************************************/
	//C_API AlgoError C_CALL PCBStencilGetBorderSize(AlgoHandle match_handle, int* out_len);


 //   /************************************
 //   @brief    : 获取PCBStencil的轮廓点集
 //   @note     :
 //   @param    : match_handle          有效的PCBStencil句柄
	//@param    : in_len               PCBStencil的轮廓点集的点数量
 //   @param    : out_PCBStencil_border      PCBStencil的轮廓点集
 //   @return   : AlgoError
 //   @Modified : haopanxia@2024/06/06
 //   ************************************/
 //   C_API AlgoError C_CALL PCBStencilGetBorder(AlgoHandle match_handle, int* in_len, AlgoPoint* out_PCBStencil_border);




    /************************************
    @brief    : 获取buffer大小
    @note     :
    @param    : match_handle          有效的PCBStencil句柄
    @param    : out_len              buffer大小
    @return   : AlgoError
    @Modified : haopanxia@2024/06/06
    ************************************/
    C_API AlgoError C_CALL PCBStencilGetBufferSize(AlgoHandle match_handle, uint64_t* out_len);


    /************************************
    @brief    : 将PCBStencilDetect里面的数据导出到buffer
    @note     :
    @param    : match_handle         有效的PCBStencil句柄
    @param    : out_buffer          已经申请好申请空间
    @param    : in_out_len          外部申请好的空间大小,会用真实大小修改
    @return   : AlgoError
    @Modified : haopanxia@2024/06/06
    ************************************/
    C_API AlgoError C_CALL PCBStencilToBuffer(AlgoHandle match_handle, char* out_buffer, uint64_t* in_out_len);


    /************************************
    @brief    : 将buffer数据导入到PCBStencilDetect
    @note     :
    @param    : match_handle          有效的PCBStencil句柄
    @param    : in_buffer
    @param    : in_len
    @return   : AlgoError
    @Modified : haopanxia@2024/06/06
    ************************************/
    C_API AlgoError C_CALL PCBStencilFromBuffer(AlgoHandle match_handle, const char* in_buffer, uint64_t in_len);

	/*********************************************
	@brief   : 载入PCBStencil文件
	@note    :
	@param   : match_handle          有效的PCBStencil句柄
	@param   : in_load_path          输入的文件路径，后缀必须为.stencil
	@return  : AlgoError
	@Author  : wushaoshuang@2024/06/12
	*********************************************/
	C_API AlgoError C_CALL PCBStencilLoad(AlgoHandle match_handle, const char* in_load_path);

	/*********************************************
	@brief   : 保存PCBStencil文件
	@note    :
	@param   : match_handle          有效的PCBStencil句柄
	@param   : in_save_path          输入要保存的文件路径，后缀必须为.stencil
	@return  : AlgoError
	@Author  : wushaoshuang@2024/06/12
	*********************************************/
	C_API AlgoError C_CALL PCBStencilSave(AlgoHandle match_handle, const char* in_save_path);


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
    C_API AlgoError C_CALL PCBStencilRelease(AlgoHandle match_handle);
    C_API void C_CALL PCBStencilDestroyHandle(AlgoHandle match_handle);





#ifdef __cplusplus
}
#endif // __cplusplus
#endif