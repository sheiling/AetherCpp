#ifndef __CI_DECLARE_H__
#define __CI_DECLARE_H__


#if (defined _WIN32 || defined _WIN64)
    #ifdef WIN_EXPORTS
        #define C_API __declspec(dllexport)
    #else
        #define C_API __declspec(dllimport)
    #endif
    //#define C_CALL __stdcall
    #define C_CALL 
#else
    #define C_API
    #define C_CALL
#endif




#endif