#ifndef __CI_UTILS_FILESYSTEM_H__
#define __CI_UTILS_FILESYSTEM_H__

#include "ci_declare.h"
#include "ci_define.h"
#include "ci_utils_errors.h"


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#ifdef _WIN32
    /************************************
    @brief    : 创建文件系统句柄
    @note     : 
    @return   : AlgoError
    @Modified : chengjiayi@2022/11/03
    ************************************/
    C_API AlgoHandle C_CALL CIFileSysCreate();


    /************************************
    @brief    : 捕获文件或者文件夹列表
    @note     : 
    @param    : hanlde          --有效的句柄
    @param    : in_path         --待捕获的目录
    @param    : in_file_pattern --文件类型,例如"*.txt"==捕获txt文件;特别的"*.*"或者"*"==捕获所有文件;"/"==只捕获文件夹;
    @return   : int             --1==获取错误;0==ok
    @Modified : chengjiayi@2022/11/03 
    ************************************/
    C_API AlgoError C_CALL CIFileSysCapture(AlgoHandle hanlde, const char* in_path, const char* in_file_pattern);

    /************************************
    @brief    : 捕获文件或者文件夹列表数量
    @note     : 
    @param    : hanlde --有效的句柄
    @return   : size_t --文件或者文件夹列表数量
    @Modified : chengjiayi@2022/11/03
    ************************************/
    C_API size_t C_CALL CIFileSysGetNum(AlgoHandle hanlde);


    /************************************
    @brief    : 获取第idx个文件(文件夹)名
    @note     : 
    @param    : hanlde      --有效的句柄
    @param    : idx         --序号
    @return   : const char* --输出字符串
    @Modified : chengjiayi@2022/11/03
    ************************************/
    C_API const char* C_CALL CIFileSysGet(AlgoHandle hanlde, size_t idx);


    
    /************************************
    @brief    : 释放句柄
    @note     : 
    @param    : hanlde 
    @return   : void
    @Modified : chengjiayi@2022/11/03
    ************************************/
    C_API void C_CALL CIFileSysDestroy(AlgoHandle hanlde);




    /************************************
    @brief    : 创建文件夹 
    @note     : 只能创建一层, 即保证待创建的文件夹的上层文件夹存在
    @param    : in_path 
    @return   : int         0=成功;1=失败
    @Modified : chengjiayi@2022/11/03
    ************************************/
    C_API int C_CALL CIFileSysMakeDir(const char* in_path);

#endif

    /************************************
    @brief    : 拆分文件名
    @note     : 
    @param    : in_fullname     文件全名
    @param    : in_split_type   拆分类型,0=完整文件名没后缀;1=完整文件夹;2=仅文件名;3=仅无后缀文件名;4=仅后缀名
                                例如:"D:/test/some.txt", 0="D:/test/some";
                                                        1="D:/test/"
                                                        2="some.txt"
                                                        3="some"
                                                        4="txt"
    @param    : out_result 
    @return   : int             0=成功;1=失败
    @Modified : chengjiayi@2022/11/03
    ************************************/
    C_API int C_CALL CIFileSysSplitFullname(const char* in_fullname, int in_split_type, char* out_result);

  
    /************************************
    @brief    : 检查文件文件夹是否存在
    @note     : 
    @param    : in_fullname 
    @return   : int         0=不存在;1=文件存在;2=文件夹存在
    @Modified : chengjiayi@2022/11/03
    ************************************/
    C_API int C_CALL CIFileSysCheckFullName(const char* in_fullname);



    /************************************
    @brief    : 获取文件夹上层文件夹
    @note     : 
    @param    : in_fullname 
    @param    : layer_idx   第几层,例如 0=当前文件夹;-1=上一层;以此类推
    @param    : out_result 
    @return   : AlgoError
    @Modified : chengjiayi@2022/11/03
    ************************************/
    C_API void C_CALL CIFileSysGetParentDir(const char* in_fullname, int layer_idx, char* out_result);

#ifdef __cplusplus
}

#endif // __cplusplus
#endif