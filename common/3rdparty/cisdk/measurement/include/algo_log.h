/*******************
开启日志方法

在主函数中加入:
int main()
{
    //可选: 日志输出的控制台
    AlgoLogAddConsoleSink(0);

    //可选: 日志输出到rotating文件
    AlgoLogAddRotatingFileSink(0,"D:/test/rotating.log");

    //可选: 日志输出到basic文件
    AlgoLogAddBasicFileSink(0,"D:/test/basic.log");

    //应用
    AlgoLogApplySink(2);


    //单独使用
    ALGOLOG_TRACE("this is a trace-level-log    %d  %s  %f", 1, "ASG", 2.36);
    ALGOLOG_DEBUG("this is a debug-level-log %d  %s  %f", 1, "ASG", 2.36);
    ALGOLOG_INFO("this is a info-level-log %d  %s  %f", 1, "ASG", 2.36);
    ALGOLOG_WARN("this is a warn-level-log %d  %s  %f", 1, "ASG", 2.36);
    ALGOLOG_ERROR("this is a error-level-log %d  %s  %f", 1, "ASG", 2.36);
    ALGOLOG_CRITICAL("this is a critical-level-log %d  %s  %f", 1, "ASG", 2.36);

}

*******************/

#ifndef __ALGO_LOG_H__
#define __ALGO_LOG_H__
#include <stdint.h>
#include "ci_declare.h"


/*********************      日志宏      ****************************/

/*在windows下将std::string 转为 std::wstring 解决中文路径的不识别问题*/
//#define  SPDLOG_WCHAR_FILENAMES


/*log所有信息输出 文件名 函数名 行号*/
//#define  ALGOLOG_OUTPUT_FILE_LINE


//打开日志 通过编译宏来控制 ALGO_LOG_ON


/******************************************************************/


#define ALGO_LOG_LEVEL_TRACE    0
#define ALGO_LOG_LEVEL_DEBUG    1
#define ALGO_LOG_LEVEL_INFO     2
#define ALGO_LOG_LEVEL_WARN     3
#define ALGO_LOG_LEVEL_ERROR    4
#define ALGO_LOG_LEVEL_CRITICAL 5
#define ALGO_LOG_LEVEL_OFF      6




#ifdef __cplusplus
extern "C" {
#endif // __cplusplus




    /************************************
    @brief  : 添加控制台日志输出接收器
    @note   : 
    @param  : in_log_level      日志输出级别 trace = 0,debug = 1,info = 2,warn = 3,err = 4,critical = 5,off = 6
    @param  : in_log_pattern    日志输出格式
    @return	: void
    @Author : chengjiayi@2021/11/24
    ************************************/
    C_API void C_CALL AlgoLogAddConsoleSink(int in_log_level, const char* in_log_pattern = "%D %H:%M:%S.%e [%L] [%n] %v%$");



    /************************************
    @brief    : 添加单日志文件接收器
    @note     : 
    @param    : in_log_level    --日志输出级别 trace = 0,debug = 1,info = 2,warn = 3,err = 4,critical = 5,off = 6
    @param    : in_log_fullname --日志文件完整路径，包括日志文件名和日志文件后缀，指定的日志文件目录必须已经存在。
    @param    : in_log_pattern  --日志输出格式
    @param    : is_truncate     --是否截断,0=否,1=是
    @return   : AlgoError
    @Modified : chengjiayi@2022/11/02
    ************************************/
    C_API void C_CALL AlgoLogAddBasicFileSink(int in_log_level,
        const char* in_log_fullname,
        const char* in_log_pattern = "%D %H:%M:%S.%e [%L] [%n]  %v",
        int in_is_truncate=0);


    /************************************
    @brief  : 添加以文件大小为单位的日志文件接收器
    @note   : 
    @param  : in_log_level      日志输出级别 trace = 0,debug = 1,info = 2,warn = 3,err = 4,critical = 5,off = 6
    @param  : in_full_filename  日志文件完整路径，包括日志文件名和日志文件后缀，指定的日志文件目录必须已经存在。
    @param  : in_log_pattern    日志输出格式
    @param  : in_file_size      单个日志文件的最大大小，字节为单位
    @param  : in_file_num       最大日志文件个数
    @return	: void
    @Author : chengjiayi@2021/11/24
    ************************************/
    C_API void C_CALL AlgoLogAddRotatingFileSink(int in_log_level, const char* in_full_filename,
        const char*in_log_pattern = "%D %H:%M:%S.%e [%L] [%n]  %v", 
        uint64_t in_file_size= 1024*1024*5,
        uint64_t in_file_num = 5);



    /************************************
    @brief  : 应用日志接收器
    @note   : 如果没有add任何sink, 默认add一个ConsoleSink
    @param  : in_flush_level    设置当立刻刷新日志到disk的日志级别
    @param  : in_global_level   全局的日志级别, 每个sink的level 和此处的 level 哪个高以哪个为主
    @param  : in_intro_str      说明字符
    @return	: void
    @Author : chengjiayi@2021/11/24
    ************************************/
    C_API void C_CALL AlgoLogApplySink(int in_flush_level, int in_global_level=0, const char* in_intro_str="");


    /*输出日志, 建议用以下的宏 ALGOLOG_TRACE ALGOLOG_DEBUG .......*/
    C_API void C_CALL AlgoLogTrace(const char* _Format, ...);
    C_API void C_CALL AlgoLogDebug(const char* _Format, ...);
    C_API void C_CALL AlgoLogInfo(const char* _Format, ...);
    C_API void C_CALL AlgoLogWarn(const char* _Format, ...);
    C_API void C_CALL AlgoLogError(const char* _Format, ...);
    C_API void C_CALL AlgoLogCritical(const char* _Format, ...);
    //C_API void C_CALL AlgoLogPrint(const std::string& in_level, const char* _Format, ...);

#ifdef __cplusplus
}
#endif // __cplusplus


#ifdef ALGO_LOG_ON
    #define ALGOLOG_TRACE(...) {AlgoLogTrace(__VA_ARGS__);}
    #define ALGOLOG_DEBUG(...) {AlgoLogDebug(__VA_ARGS__);}
    #define ALGOLOG_INFO(...) {AlgoLogInfo(__VA_ARGS__);}
    #define ALGOLOG_WARN(...) {AlgoLogWarn(__VA_ARGS__);}
    #define ALGOLOG_ERROR(...) {AlgoLogError(__VA_ARGS__);}
    #define ALGOLOG_CRITICAL(...) {AlgoLogCritical(__VA_ARGS__);}
#else
    #define ALGOLOG_TRACE(...)
    #define ALGOLOG_DEBUG(...)
    #define ALGOLOG_INFO(...) 
    #define ALGOLOG_WARN(...) 
    #define ALGOLOG_ERROR(...)
    #define ALGOLOG_CRITICAL(...) 
#endif


#define SHOW_STEP(...) ALGOLOG_TRACE(__VA_ARGS__)

#endif