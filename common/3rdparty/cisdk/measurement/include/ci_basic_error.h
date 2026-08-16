#ifndef __CI_BASIC_ERROR_H__
#define __CI_BASIC_ERROR_H__
#include <string>
#include <iostream>

/*用于定义全局错误码
*|--------------------------------- uint32-------------------------------------|
*|--------高8位----------|--------- 中8位 ---------|--------- 低16位-----------|
*|------- 预留 ----------|--------- 模块名 --------|--------- 错误码 -----------|
*/


#define ALGO_PASS                               0
#define ERROR_CODE(module,err) ((module << 16) | err)
#define ERROR_MODULE_NAME(err) (err>>16)
#define ERROR_CODE_NO_MODULE(err) (err&0x0000FFFF)


//basic error---------------------------------------------- 1-99
#define ALGO_INPUT_NULLPTR(module)              ERROR_CODE(module,1)   //输入的指针为空
#define ALGO_INPUT_EMPTY(module)                ERROR_CODE(module,2)   //输入数据为空
#define ALGO_INPUT_INVALID(module)              ERROR_CODE(module,3)   //输入错误值
#define ALGO_INPUT_MISMATCH(module)             ERROR_CODE(module,4)   //输入数据不匹配

#define ALGO_ERROR_FILE_VERSION(module)         ERROR_CODE(module,10)  //文件版本错误
#define ALGO_ERROR_EXTNAME(module)              ERROR_CODE(module,11)  //文件后缀错误
#define ALGO_ERROR_FILE_ENDIANNESS(module)      ERROR_CODE(module,12)  //文件大小端不一致
#define ALGO_ERROR_READ_FILE(module)            ERROR_CODE(module,13)  //读取文件时发生错误
#define ALGO_ERROR_WRITE_FILE(module)           ERROR_CODE(module,14)  //写入文件时发生错误
#define ALGO_MISMATCH_FILE_SIZE(module)         ERROR_CODE(module,15)  //文件大小与输入不一致
#define ALGO_BUFFER_SIZE_INSUFFICIENT(module)   ERROR_CODE(module,16)  //buffer 大小不够
#define ALGO_ERROR_MODCODE_MISMATCH(module)     ERROR_CODE(module,17)  //校验模块码不匹配

#define ALGO_EMPTY_DATA(module)                 ERROR_CODE(module,20)  //数据为空
#define ALGO_ERROR_HEADER(module)               ERROR_CODE(module,21)  //结构体信息头错误
#define ALGO_INVALID_METHOD(module)             ERROR_CODE(module,22)  //无效方法


#endif
