#ifndef __MODEL_PROCESSOR_H__
#define __MODEL_PROCESSOR_H__

#include "ci_declare.h"
#include "ci_define.h"
#include "algo_mat.h"
#include "model_processor_data.h"
#include "model_processor_errors.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

    /************************************
    @brief    : 创建model句柄
    @note     : 
    @return   : AlgoError
    @Modified : chengjiayi@2022/11/08
    ************************************/
    C_API AlgoHandle C_CALL ModelCreateHandle();


    /************************************
    @brief    : 设置model类型
    @note     : 
    @param    : model_handle    有效的model句柄
    @param    : in_model_type 
    @return   : AlgoError
    @Modified : chengjiayi@2022/11/08
    ************************************/
    C_API AlgoError C_CALL ModelSetType(AlgoHandle model_handle, ModelProcType in_model_type);
    

	/************************************
	@brief    : 获取model类型
	@note     :
	@param    : model_handle    有效的model句柄
	@param    : out_model_type
	@return   : AlgoError
	@Modified : haopanxia@2023/03/06
	************************************/
	C_API AlgoError C_CALL ModelGetType(AlgoHandle model_handle, ModelProcType* out_model_type);





    /************************************
    @brief    : 添加模板
    @note     : 
    @param    : model_handle    有效的model句柄
    @param    : in_model_name   模板名字
    @param    : in_tmpl         模板数据
    @return   : AlgoError
    @Modified : chengjiayi@2022/11/08
    ************************************/
    C_API AlgoError C_CALL ModelAdd(AlgoHandle model_handle, const char * in_model_name, const AlgoMat* in_tmpl);
    
    /************************************
    @brief    : 移除模型中模板
    @note     : 
    @param    : model_handle    有效的model句柄
    @param    : in_model_name   模板名字
    @return   : AlgoError
    @Modified : chengjiayi@2022/11/08
    ************************************/
    C_API AlgoError C_CALL ModelRemove(AlgoHandle model_handle, const char * in_model_name);
    
    /************************************
    @brief    : 模型中模板改名
    @note     : 
    @param    : model_handle 有效的model句柄
    @param    : in_orig_name 原模板名字
    @param    : in_new_name  新模板名字
    @return   : AlgoError
    @Modified : chengjiayi@2022/11/08
    ************************************/
    C_API AlgoError C_CALL ModelChangeName(AlgoHandle model_handle, const char * in_orig_name, const char * in_new_name);
    
    /************************************
    @brief    : 载入模板库
    @note     : 
    @param    : model_handle    有效的model句柄
    @param    : in_model_file   模板名字
    @return   : AlgoError
    @Modified : chengjiayi@2022/11/08
    ************************************/
    C_API AlgoError C_CALL ModelLoad(AlgoHandle model_handle, const char * in_model_file);
    
    /************************************
    @brief    : 保存模板库到.model文件
    @note     : 
    @param    : model_handle    有效的model句柄
    @param    : in_model_file   .model文件完整路径
    @return   : AlgoError
    @Modified : chengjiayi@2022/11/08
    ************************************/
    C_API AlgoError C_CALL ModelSave(AlgoHandle model_handle, const char * in_model_file);
    
    /************************************
    @brief    : 获取buffer大小
    @note     : 
    @param    : model_handle    有效的model句柄
    @param    : out_len         buffer大小
    @return   : AlgoError
    @Modified : chengjiayi@2022/11/08
    ************************************/
    C_API AlgoError C_CALL ModelGetBufferSize(AlgoHandle model_handle, uint64_t* out_len);
    
    /************************************
    @brief    : model数据导出到buffer
    @note     : 
    @param    : model_handle    有效的model句柄
    @param    : out_buffer      已经申请好申请空间
    @param    : in_out_len      外部申请好的空间大小,会用真实大小修改
    @return   : AlgoError
    @Modified : chengjiayi@2022/11/08
    ************************************/
    C_API AlgoError C_CALL ModelToBuffer(AlgoHandle model_handle, char* out_buffer, uint64_t* in_out_len);
    
    /************************************
    @brief    : 将buffer数据导入到model句柄
    @note     : 
    @param    : model_handle    有效的model句柄
    @param    : in_buffer       
    @param    : in_len 
    @return   : AlgoError
    @Modified : chengjiayi@2022/11/08
    ************************************/
    C_API AlgoError C_CALL ModelFromBuffer(AlgoHandle model_handle, const char* in_buffer, uint64_t in_len);
    
    /************************************
    @brief    : 获取模板数据
    @note     : 
    @param    : model_handle    有效的model句柄
    @param    : in_model_name   模板名字
    @param    : out_tmpl        模板数据
    @param    : in_is_deep_copy 是否深拷贝, 默认浅拷贝  0=浅拷贝,1=深拷贝;深拷贝需要手动释放out_tmpl
    @return   : AlgoError
    @Modified : chengjiayi@2022/11/08
    ************************************/
    C_API AlgoError C_CALL ModelGet(AlgoHandle model_handle, const char * in_model_name, AlgoMat* out_tmpl, int in_is_deep_copy=0);
    
    
	/************************************
	@brief    : 获取model文件中所有模板名称
	@note     :
	@param    : model_handle          有效的model句柄
	@param    : out_model_name_list   文件中所有图像名称的集合
	@return   : AlgoError
	@Modified : haopanxia@2023/01/12
	************************************/
	C_API AlgoError C_CALL ModelGetAllName(AlgoHandle model_handle, char* out_model_name_list);


	/************************************
	@brief    : 获取model文件中指定位置的模板名称
	@note     :
	@param    : model_handle          有效的model句柄
	@param    : in_model_pos          图像在文件中的位置，从0开始
	@param    : out_model_name        文件中指定图像名称
	@return   : AlgoError
	@Modified : haopanxia@2023/01/12
	************************************/
	C_API AlgoError C_CALL ModelGetOneName(AlgoHandle model_handle, int* in_model_pos, char* out_model_name);




	
	/************************************
    @brief    : 显示model信息到console
    @note     : 
    @param    : model_handle    有效的model句柄
    @return   : AlgoError
    @Modified : chengjiayi@2022/11/08
    ************************************/
    C_API AlgoError C_CALL ModelInfo(AlgoHandle model_handle);
    
    /************************************
    @brief    : model大小
    @note     : 
    @param    : model_handle     有效的model句柄
	@param    : out_num          文件中的模板数量
    @return   : AlgoError
    @Modified : haopanxia@2023/01/13
    ************************************/
    C_API AlgoError C_CALL ModelGetNum(AlgoHandle model_handle, int* out_num);

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
    C_API AlgoError C_CALL ModelRelease(AlgoHandle model_handle);
    C_API void C_CALL ModelDestroyHandle(AlgoHandle model_handle);









#ifdef __cplusplus
}

#endif // __cplusplus



#endif