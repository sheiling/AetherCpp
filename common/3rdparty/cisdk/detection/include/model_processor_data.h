#ifndef __MODEL_PROCESSOR_DATA_H__
#define __MODEL_PROCESSOR_DATA_H__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus



    typedef enum
    {
        MODEL_PROC_UNDEFINE = 0,
        MODEL_PROC_MARK = 1,
        MODEL_PROC_CHIP = 2,
        MODEL_PROC_DIE = 3,
        MODEL_PROC_SOLDER = 4,
		MODEL_PROC_SOCKET = 5,
		MODEL_PROC_MICRO_CELL = 6,

    }ModelProcType;//模板类型




#ifdef __cplusplus
}
#endif // __cplusplus



#endif