#ifndef __CI_DEFINE_H__
#define __CI_DEFINE_H__
#include <stdint.h>


typedef uint32_t AlgoError;
typedef void* AlgoHandle;

struct AlgoObject{
	void* obj;
	int obj_type;
};


#define ALGO_DEVICE_STR_LEN         64
#define OMP_RESERVED_THREADS_NUM    1 //默认OMP多线程保留的线程数

#if (defined _WIN32 || defined _WIN64)
    #pragma warning(disable:4099)
#endif

//#if _MSC_VER >= 1600
//    #pragma execution_character_set("utf-8")
//#endif

#endif