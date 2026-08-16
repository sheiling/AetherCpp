/****************************************************************************************************
*  SolidTech Motion Library									    									*
*  Copyright (C) 2020 Shenzhen Solid Technology Co.,Ltd			            						*
*																									*
*																									*
*  @file     sldm.h																					*
*  @brief    defs of struct and function															*
*																									*
*																									*
*  @version  3.1.1.2(版本号)																		*
*  @date     2021.05.21																				*
*																									*
*																									*
****************************************************************************************************/

#pragma once
#ifdef SLDM_API
#else
#define SLDM_API extern "C" __declspec(dllimport) short __cdecl
#endif

#pragma region 函数返回值及其含义

/***************************************************************************************************
*			函数返回值及其含义
***************************************************************************************************/

#define SLDM_STAT_MC_OPENING			2				/*正在打开控制器*/
#define SLDM_STAT_MC_OPENED			1				/*控制器已打开*/

#define SLDM_ERR_OK					0				/*无错误*/

#define SLDM_ERR_PMVAL				-1				/*参数值错误*/
#define SLDM_ERR_PULSEOVERFLOW		-2				/*脉冲发生器溢出*/
#define SLDM_ERR_PMID					-3				/*参数ID不存在，无此参数*/
#define SLDM_ERR_MCNOTEXIST		    -4				/*控制器不存在，超出系统支持的控制器索引*/
#define SLDM_ERR_CHMAX				-5				/*通道号不存在，超出系统最大通道数*/
#define SLDM_ERR_AXISMAX				-6				/*轴号或单元号不存在，超出系统最大轴数或单元数*/
#define SLDM_ERR_NOAUTH				-7				/*控制器无授权*/
#define SLDM_ERR_PLCADDROVERFLOW		-8				/*PLC地址溢出*/
#define SLDM_ERR_NOFLAG				-9				/*没有此状态或标志*/
#define SLDM_ERR_NONSTOPPED			-10				/*运动没有停止*/
#define SLDM_ERR_MCNOFILE				-11				/*文件号打开失败。文件号不存在，或SD卡不存在。*/
#define SLDM_ERR_HOSTNOFILE			-12				/*HOST打开文件失败*/
#define SLDM_ERR_AXISALM				-13				/*轴伺服报警*/
#define SLDM_ERR_AXISPOT				-14				/*轴正向硬限位*/
#define SLDM_ERR_AXISNOT				-15				/*轴负向硬限位*/
#define SLDM_ERR_AXISPSL				-16				/*轴正向软限位*/
#define SLDM_ERR_AXISNSL				-17				/*轴负向软限位*/
#define SLDM_ERR_AXISESTOP			-18				/*硬急停*/
#define SLDM_ERR_AXISOT				-19				/*轴硬限位*/
#define SLDM_ERR_AXISSL				-20				/*轴软限位*/

#define SLDM_ERR_HDBUSY				-51				/*大数据传送忙*/
#define SLDM_ERR_HDERROR				-52				/*大数据传送错误*/

#define SLDM_ERR_FILEOPEND			-61				/*文件已打开*/
#define SLDM_ERR_NOFILEOPEN			-62				/*文件未打开*/

#define SLDM_ERR_CMDBUFFFULL          -100            /*指令缓冲区满*/

#define SLDM_ERR_COM_ADDR				-101			/*与HOST通讯的设备地址错误*/
#define SLDM_ERR_COM_CHECKSUM			-102			/*与HOST通讯的校验和错误*/
#define SLDM_ERR_COM_INVCMD			-103			/*与HOST通讯的命令ID错误*/
#define SLDM_ERR_SOCKET				-104			/*HOST库中，SOCKET初始化失败*/
#define SLDM_ERR_SHM					-105			/*HOST库中，SHM初始化失败*/
#define SLDM_ERR_PIPE					-106			/*HOST库中，PIPE初始化失败*/
#define SLDM_ERR_NOLIBINITD			-107			/*HOST库没有初始化*/
#define SLDM_ERR_MCCLOSE_FAIL			-108			/*控制器关闭失败*/
#define SLDM_ERR_MC_NONCONNECT		-110			/*HOST与控制器通讯超时，未连接*/
#define SLDM_ERR_THREAD				-112			/*HOST库中，线程初始化失败*/
#define SLDM_ERR_BUFFIDX				-113			/*标准命令块缓冲区索引号错误*/
#define SLDM_ERR_BUFFFULL				-114			/*标准命令块缓冲区满*/
#define SLDM_ERR_PREBUFFFULL          -115            /*标准命令块的预缓冲区满*/

#define SLDM_ERR_FB_TIMEOUT			-121			/*伺服现场总线超时*/
#define SLDM_ERR_FB_NCYCWNG			-122			/*伺服总线中非周期命令执行报警*/
#define SLDM_ERR_FB_CCYCERR			-123			/*伺服总线中非周期命令执行错误*/

#define SLDM_ERR_SERVICE_STOP			-230			/*控制器服务程序停止*/

#define SLDM_ERR_UPDATE				-255			/*内部使用，更新命令*/

#pragma endregion



/***************************************************************************************************
*			函数接口
***************************************************************************************************/

#pragma region 控制器操作

/**********************************************************************************
* @Begin 控制器操作
*********************************************************************************/
/**
* @brief 搜索IP段的控制器
* @param ips：       搜索到的IP地址数组
*        ip_num：    搜索到的IP地址的个数
*        timeout：   搜索一个IP地址的超时时间，单位：ms
*        ipseg3：    IP地址第三段，192.168.m.0，m取值范围：0~255，默认值为100
* @return 成功返回0
* @remark 控制器可以处于未连接状态
*/
SLDM_API SLDM_SearchMcList(_Out_ char** ips, _Out_ short* ip_num, _In_ short timeout = 50, _In_ short ipseg3 = 100);

/**
* @brief 打开控制器
* @param ip：        控制器的IP地址，是C类IP地址私有范围地址，192.168.0.0~192.168.255.255。
*        mc：        服务分配并返回的控制器索引，作为其它接口函数的控制器索引参数，0表示无效控制器。
*		 timeout：   服务与控制器通讯握手超时时间，单位是ms，0表示超时无穷大。
* @return 成功返回0.
* @remark
*/
SLDM_API SLDM_Open(_In_ const char* ip, _Out_ short* mc, _In_ short timeout = 500);

/**
* @brief 利用索引号关闭控制器
* @param mc：        控制器索引
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_Close(_In_ short mc);

/**
* @brief 利用IP关闭控制器
* @param ip：        控制器IP地址
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_CloseIP(_In_ const char* ip);

/**
* @brief 控制器热复位
* @param mc：        控制器索引
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_Reset(_In_ short mc);

/**
* @brief 读取控制器连接状态
* @param ip：        控制器的IP地址
*        mc：        服务返回的IP地址对应的控制器索引，作为其它接口函数的控制器索引参数，0表示无效控制器。
* @return 返回控制器连接状态，0-未连接，1-已连接，2-正在连接
* @remark
*/
SLDM_API SLDM_GetConnectionStatus(_In_ const char* ip, _Out_ short* mc);

/**
* @brief 设置控制器的IP地址
* @param oldip：     原来的IP地址
*        newip：     新的IP地址
* @return 成功返回0
* @remark 控制器应处于未连接状态
*/
SLDM_API SLDM_SetIP(_In_ const char* oldip, _Inout_ char* newip);

/**
* @brief 搜索IP地址对应的控制器是否存在
* @param ip：        IP地址
*        timeout：   超时时间，单位：ms
* @return 成功返回0
* @remark 控制器可以处于未连接状态
*/
SLDM_API SLDM_FindIP(_In_ const char* ip, _In_ short timeout = 500);

/**
* @brief 建立伺服系统总线链接
* @param mc：        控制器索引
*		 fb_type：   伺服总线类型
*		 slavenum：  总线上从站数量
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_SetFBConnection(_In_ short mc, _In_ unsigned short fb_type, _In_ short slavenum);

/**
* @brief 读取伺服系统总线状态
* @param mc：        控制器索引
*		 axis：      轴号 4轴卡0~3 8轴卡0~7
*		 cmd_err：   命令错误状态
*		 timeout：   超时状态
*		 macid：     mac地址ID
*		 runstat：   运行状态
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_GetFBStatus(_In_ short mc, _In_ short axis, _Out_ short* cmd_err, _Out_ short* timeout, _Out_ short* macid, _Out_ short* runstat);

/**********************************************************************************
* @End 控制器操作
*********************************************************************************/

#pragma endregion


#pragma region 回零操作

/*************************************************************************
* @Begin 回零操作
************************************************************************/

/**
* @brief 设置回零参数
* @param mc：        控制器索引
*        axis：      轴号 4轴卡0~3 8轴卡0~7
*        dir：       回零方向，1-正向，0-负向
*		 mode：      回零模式，取值范围：
*                       0-无效
*                       1-HOME，零点开关一次回零
*                       2-LMT，限位开关一次回零
*                       3-Z，编码器Z相信号一次回零
*                       4-HOME+Z，原点与编码器Z相信号联合回零
*                       5-LMT+Z，限位开关与编码器Z相信号联合回零
*                       6-LMT+HOME，限位开关与零点开关联合回零
*                       7-LMT+HOME+Z，限位开关、零点开关与编码器Z相信号联合回零
*                       8-原地，仅仅将轴的位置清零
*        lo_vel：    回零低速，单位：p(或物理量纲分辨率)/ms，取值范围：0.001~8000.0
*        hi_vel：    回零高速，单位：p(或物理量纲分辨率)/ms，取值范围：0.001~8000.0
*        acc：       回零加速度，单位：p(或物理量纲分辨率)/ms^2，取值范围：0.001~4000.0
*        offset：    零点偏置，单位：p或物理量纲分辨率，取值范围：±2^31，默认值为0
*        latch_dist：锁存方式回零时搜索距离，如果值大于零，表示锁存方式回零，单位：p或物理量纲分辨率，
*                       取值范围：0~2^31，默认值为0，表示低速搜索信号，立即停止。
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_SetHomeParam(_In_ short mc, _In_ short axis, _In_ short dir, _In_ short mode, _In_ float lo_vel, _In_ float hi_vel, _In_ float acc, _In_ int offset = 0, _In_ int latch_dist = 0);

/**
* @brief 回零启动
* @param mc：        控制器索引
*		  axis：      轴号 4轴卡0~3 8轴卡0~7
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_Home(_In_ short mc, _In_ short axis);

/**
* @brief 设置轴正向软限位位置
* @param axis：      轴号，取值范围：控制器支持的轴号
*		 pos：       正向软限位，单位：p或物理量纲分辨率，取值范围：±2^31
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_SetPSL(_In_ short mc, _In_ short axis, _In_ int pos);

/**
* @brief 设置轴负向软限位位置
* @param axis：      轴号，取值范围：控制器支持的轴号
*		 pos：       负向软限位，单位：p或物理量纲分辨率，取值范围：±2^31
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_SetNSL(_In_ short mc, _In_ short axis, _In_ int pos);

/*************************************************************************
* @End 回零操作
************************************************************************/

#pragma endregion


#pragma region 点位运动

/**********************************************************************************
* @Begin 点位运动
*********************************************************************************/

/**
* @brief 绝对式P2P运动，单轴运动
* @param mc：        控制器索引
*		 axis：      轴号，取值范围：控制器支持的轴号
*		 pos：       绝对目标位置，单位：p或物理量纲分辨率，取值范围：±2^31
*        str_vel：   起跳速度，单位：p(或物理量纲分辨率)/ms，取值范围：0.001~8000.000
*        max_vel：   目标速度，单位：p(或物理量纲分辨率)/ms，取值范围：0.001~8000.000
*        acc：       目标加速度，单位：p(或物理量纲分辨率)/ms^2，取值范围：0.001~4000.000
*        dec：       目标减速度，单位：p(或物理量纲分辨率)/ms^2，取值范围：0.001~4000.000
*        end_vel：   截止速度，单位：p(或物理量纲分辨率)/ms，取值范围：0.001~8000.000
*        smth：      平滑时间，单位：ms，取值范围：0~65535，默认值0，表示梯形运动，大于0则为S曲线运动
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_MoveA(_In_ short mc, _In_ short axis, _In_ int pos, _In_ float str_vel, _In_ float max_vel, _In_ float acc, _In_ float dec, _In_ float end_vel, _In_ unsigned short smth = 0);

/**
* @brief 相对式P2P运动，单轴运动
* @param mc：        控制器索引
*		 axis：      轴号，取值范围：控制器支持的轴号
*		 pos：       相对目标位置，单位：p或物理量纲分辨率，取值范围：±2^31
*        str_vel：   起跳速度，单位：p(或物理量纲分辨率)/ms，取值范围：0.001~8000.000
*        max_vel：   目标速度，单位：p(或物理量纲分辨率)/ms，取值范围：0.001~8000.000
*        acc：       目标加速度，单位：p(或物理量纲分辨率)/ms^2，取值范围：0.001~4000.000
*        dec：       目标减速度，单位：p(或物理量纲分辨率)/ms^2，取值范围：0.001~4000.000
*        end_vel：   截止速度，单位：p(或物理量纲分辨率)/ms，取值范围：0.001~8000.000
*        smth：      平滑时间，单位：ms，取值范围：0~65535，默认值0，表示梯形运动，大于0则为S曲线运动
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_MoveR(_In_ short mc, _In_ short axis, _In_ int pos, _In_ float str_vel, _In_ float max_vel, _In_ float acc, _In_ float dec, _In_ float end_vel, _In_ unsigned short smth = 0);

/**
* @brief 绝对式P2P运动，多轴运动
* @param mc：        控制器索引
*		 axes：      按位掩码的轴选择
*		 pos：       绝对目标位置数组，单位：p或物理量纲分辨率，取值范围：±2^31
*        str_vel：   起跳速度数组，单位：p(或物理量纲分辨率)/ms，取值范围：0.001~8000.000
*        max_vel：   目标速度数组，单位：p(或物理量纲分辨率)/ms，取值范围：0.001~8000.000
*        acc：       目标加速度数组，单位：p(或物理量纲分辨率)/ms^2，取值范围：0.001~4000.000
*        dec：       目标减速度数组，单位：p(或物理量纲分辨率)/ms^2，取值范围：0.001~4000.000
*        end_vel：   截止速度数组，单位：p(或物理量纲分辨率)/ms，取值范围：0.001~8000.000
*        smth：      平滑时间数组，单位：ms，取值范围：0~65535，默认值0，表示梯形运动，大于0则为S曲线运动
* @return 成功返回0
* @remark
*/
//SLDM_API SLDM_MoveAM(_In_ short mc, _In_ unsigned int axes, _In_ int* pos, _In_ float* str_vel, _In_ float* max_vel, _In_ float* acc, _In_ float* dec, _In_ float* end_vel, _In_ unsigned short* smth);

/**
* @brief 相对式P2P运动，多轴运动
* @param mc：控制器索引
*		 axes：      按位掩码的轴选择
*		 pos：       相对目标位置数组，单位：p或物理量纲分辨率，取值范围：±2^31
*        str_vel：   起跳速度数组，单位：p(或物理量纲分辨率)/ms，取值范围：0.001~8000.000
*        max_vel：   目标速度数组，单位：p(或物理量纲分辨率)/ms，取值范围：0.001~8000.000
*        acc：       目标加速度数组，单位：p(或物理量纲分辨率)/ms^2，取值范围：0.001~4000.000
*        dec：       目标减速度数组，单位：p(或物理量纲分辨率)/ms^2，取值范围：0.001~4000.000
*        end_vel：   截止速度数组，单位：p(或物理量纲分辨率)/ms，取值范围：0.001~8000.000
*        smth：      平滑时间数组，单位：ms，取值范围：0~65535，默认值0，表示梯形运动，大于0则为S曲线运动
* @return 成功返回0
* @remark
*/
//SLDM_API SLDM_MoveRM(_In_ short mc, _In_  unsigned int axes, _In_ int* pos, _In_ float* str_vel, _In_ float* max_vel, _In_ float* acc, _In_ float* dec, _In_ float* end_vel, _In_ unsigned short* smth);

/**
* @brief 轴运动平滑停止
* @param mc：        控制器索引
*		  axis：      轴号 4轴卡0~3 8轴卡0~7
* @return 成功返回0
* @remark 用于P2P、点动、回零，等运动方式的平滑停止
*/
SLDM_API SLDM_Stop(_In_ short mc, _In_ short axis);

/**
* @brief 轴运动平滑停止
* @param mc：        控制器索引
*		  axes：     按位掩码的轴选择
* @return 成功返回0
* @remark 用于P2P、点动、回零，等运动方式的平滑停止
*/
//SLDM_API SLDM_Stop(_In_ short mc, unsigned int axes);

/**
* @brief 轴运动立即停止
* @param mc：        控制器索引
*		  axis：      轴号 4轴卡0~3 8轴卡0~7
* @return 成功返回0
* @remark 用于P2P、点动、回零，等运动方式的立即停止，无减速过程
*/
SLDM_API SLDM_EStop(_In_ short mc, _In_ short axis);

/**
* @brief 轴运动立即停止
* @param mc：        控制器索引
*		   axes：     按位掩码的轴选择
* @return 成功返回0
* @remark 用于P2P、点动、回零，等运动方式的立即停止，无减速过程
*/
//SLDM_API SLDM_EStop(_In_ short mc, _In_ unsigned int axes);

/**
* @brief 设置脉冲位置
* @param mc：        控制器索引
*        axis：      轴号，取值范围：控制器支持的轴号
*        pos：       命令位置，单位：p或物理量纲分辨率，取值范围：±2^31
* @return 成功返回0
* @remark 控制器内部，根据运动参数计算的位置，称为命令位置或脉冲位置
*/
SLDM_API SLDM_SetPosPulse(_In_ short mc, _In_ short axis, _In_ int pos);

/**
* @brief 设置编码器位置
* @param mc：        控制器索引
*        axis：      轴号，取值范围：控制器支持的轴号
*        pos：       实际位置，单位：p或物理量纲分辨率，取值范围：±2^31
* @return 成功返回0
* @remark 控制器内部，根据编码器值换算得的位置，称为实际位置或编码器位置
*/
SLDM_API SLDM_SetPosEncoder(_In_ short mc, _In_ short axis, _In_ int pos);

/**
* @brief 读取轴运动的当前脉冲位置
* @param mc：        控制器索引
*        axis：      轴号，取值范围：控制器支持的轴号
*        pos：       命令位置，单位：p或物理量纲分辨率
* @return 成功返回0
* @remark 控制器内部，根据运动参数计算的位置，称为命令位置或脉冲位置
*/
SLDM_API SLDM_GetPosPulse(_In_ short mc, _In_ short axis, _Out_ int* pos);

/**
* @brief 读取轴运动的当前编码器位置
* @param mc：        控制器索引
*        axis：      轴号，取值范围：控制器支持的轴号
*        pos：       实际位置，单位：p或物理量纲分辨率
* @return 成功返回0
* @remark 控制器内部，根据编码器值换算得的位置，称为实际位置或编码器位置
*/
SLDM_API SLDM_GetPosEncoder(_In_ short mc, _In_ short axis, _Out_ int* pos);

/**
* @brief 读取轴运动的当前脉冲速度
* @param mc：        控制器索引
*        axis：      轴号，取值范围：控制器支持的轴号
*        vel：       命令速度，单位：p(或物理量纲分辨率)/ms
* @return 成功返回0
* @remark 控制器内部，根据运动参数计算的速度，称为命令速度或脉冲速度
*/
SLDM_API SLDM_GetVelPulse(_In_ short mc, _In_ short axis, _Out_ float* vel);

/**
* @brief 读取轴运动的当前实际速度
* @param mc：        控制器索引
*        axis：      轴号，取值范围：控制器支持的轴号
*        vel：       实际速度，单位：p(或物理量纲分辨率)/ms
* @return 成功返回0
* @remark 控制器内部，根据编码器值换算的速度，称为实际速度或编码器速度
*/
SLDM_API SLDM_GetVelEncoder(_In_ short mc, _In_ short axis, _Out_ float* vel);

/**
* @brief 读取轴运动的当前脉冲加速度
* @param mc：        控制器索引
*        axis：      轴号，取值范围：控制器支持的轴号
*        acc：       命令加速度，单位：p(或物理量纲分辨率)/ms^2
* @return 成功返回0
* @remark 控制器内部，根据运动参数计算的加速度，称为命令加速度或脉冲加速度
*/
SLDM_API SLDM_GetAccPulse(_In_ short mc, _In_ short axis, _Out_ float* acc);

/**
* @brief 读取轴运动的当前编码器加速度
* @param mc：        控制器索引
*        axis：      轴号，取值范围：控制器支持的轴号
*        acc：       实际加速度，单位：p(或物理量纲分辨率)/ms^2
* @return 成功返回0
* @remark 控制器内部，根据编码器值换算的加速度，称为实际加速度或编码器加速度
*/
SLDM_API SLDM_GetAccEncoder(_In_ short mc, _In_ short axis, _Out_ float* acc);

/**
* @brief 设置轴运动的最大速度
* @param mc：        控制器索引
*        axis：      轴号，取值范围：控制器支持的轴号
*        vel：       轴最大速度，单位：p(或物理量纲分辨率)/ms，取值范围：0.001~8000.0
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_SetMaxVel(_In_ short mc, _In_ short axis, _In_ float vel);

/**
* @brief 设置轴运动的最大加速度
* @param mc：        控制器索引
*        axis：      轴号，取值范围：控制器支持的轴号
*        acc：       最大加速度，单位：p(或物理量纲分辨率)/ms^2，取值范围：0.001~4000.0
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_SetMaxAcc(_In_ short mc, _In_ short axis, _In_ float acc);

/**
* @brief 设置轴运动的最大减速度
* @param mc：        控制器索引
*        axis：      轴号，取值范围：控制器支持的轴号
*        dec：       最大减速度，单位：p(或物理量纲分辨率)/ms^2，取值范围：0.001~4000.0
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_SetMaxDec(_In_ short mc, _In_ short axis, _In_ float dec);

/**
* @brief 设值辅助编码器的初值
* @param mc：        控制器索引
*        auxenc：    辅助编码器索引，取值范围：0,1
*        pos：       初值，单位：p或物理量纲分辨率
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_SetPosAux(_In_ short mc, _In_ short auxenc, _In_ int pos);

/**
* @brief 读取辅助编码器的计数值
* @param mc：        控制器索引
*        auxenc：    辅助编码器索引，取值范围：0,1
*        pos：       计数值，单位：p或物理量纲分辨率
* @return 成功返回0
@remark
*/
SLDM_API SLDM_GetPosAux(_In_ short mc, _In_ short auxenc, _Out_ int* pos);


//----------------------------- @Begin 兼容旧版本函数 -----------------------------

/**
* @brief 绝对式P2P运动启动
* @param mc：        控制器索引
*		  axis：      轴号 4轴卡0~3 8轴卡0~7
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_JogA(_In_ short mc, _In_ short axis);

/**
* @brief 相对式P2P运动启动
* @param mc：        控制器索引
*		  axis：      轴号 4轴卡0~3 8轴卡0~7
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_JogR(_In_ short mc, _In_ short axis);

/**
* @brief 正向连续点动
* @param mc：        控制器索引
*		  axis：      轴号 4轴卡0~3 8轴卡0~7
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_JogPlus(_In_ short mc, _In_ short axis);

/**
* @brief 负向连续点动
* @param mc：        控制器索引
*		  axis：      轴号 4轴卡0~3 8轴卡0~7
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_JogMinus(_In_ short mc, _In_ short axis);

/**
* @brief 设置轴运动的目标位置
* @param  axis：      轴号 4轴卡0~3 8轴卡0~7
*		 pos：       绝对或增量的目标位置，单位：p或物理量纲分辨率，取值范围：±2^31
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_SetPosT(_In_ short mc, _In_ short axis, _In_ int pos);

/**
* @brief 设置轴运动的目标速度
* @param  axis：      轴号 4轴卡0~3 8轴卡0~7
*		 vel：       目标速度，单位：p或物理量纲分辨率每ms，取值范围：0.001~8000.0
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_SetVelT(_In_ short mc, _In_ short axis, _In_ float vel);

/**
* @brief 设置轴运动的目标加速度
* @param  axis：      轴号 4轴卡0~3 8轴卡0~7
*		 acc：       目标加速度，单位：p或物理量纲分辨率每ms^2，取值范围：0.001~8000.0
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_SetAccT(_In_ short mc, _In_ short axis, _In_ float acc);

/**
* @brief 设置轴运动的目标减速度
* @param  axis：      轴号 4轴卡0~3 8轴卡0~7
*		 dec：       目标减速度，单位：p或物理量纲分辨率每ms^2，取值范围：0.001~8000.0
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_SetDecT(_In_ short mc, _In_ short axis, _In_ float dec);

/**
* @brief 设置轴运动的速度平滑时间
* @param  axis：      轴号 4轴卡0~3 8轴卡0~7
*		 tm：        平滑时间，单位：ms，取值范围：0~65535
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_SetSmthT(_In_ short mc, _In_ short axis, _In_ unsigned short tm);

/**
* @brief 设置轴运动的启动速度
* @param  axis：      轴号 4轴卡0~3 8轴卡0~7
*		 vel：       启动速度，单位：p或物理量纲分辨率每ms，取值范围：0.001~8000.0
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_SetVelS(_In_ short mc, _In_ short axis, _In_ float vel);

/**
* @brief 设置轴运动的截止速度
* @param  axis：      轴号 4轴卡0~3 8轴卡0~7
*		 vel：       截止速度，单位：p或物理量纲分辨率每ms，取值范围：0.001~8000.0
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_SetVelE(_In_ short mc, _In_ short axis, _In_ float vel);


/**
* @brief 读取轴运动的目标位置
* @param axis：      轴号 4轴卡0~3 8轴卡0~7
pos: 目标位置
* @return 成功返回0
*/
SLDM_API SLDM_GetPosT(short mc, short axis, int* pos);
/**
* @brief 读取轴运动的目标速度
* @param axis：      轴号 4轴卡0~3 8轴卡0~7
vel：目标速度
* @return 成功返回0
*/
SLDM_API SLDM_GetVelT(short mc, short axis, float* vel);
/**
* @brief 读取轴运动的目标加速度
* @param axis：      轴号 4轴卡0~3 8轴卡0~7
acc：目标加速度
* @return 成功返回0
*/
SLDM_API SLDM_GetAccT(short mc, short axis, float* acc);

/**
* @brief 读取轴运动的目标减速度
* @param axis：      轴号 4轴卡0~3 8轴卡0~7
dec:目标减速度
* @return 成功返回0
*/
SLDM_API SLDM_GetDecT(short mc, short axis, float* dec);

/**
* @brief 读取轴运动的速度平滑时间
* @param axis：      轴号 4轴卡0~3 8轴卡0~7
tm：平滑时间
* @return 成功返回0
*/
SLDM_API SLDM_GetSmthT(short mc, short axis, unsigned short* tm);

/**
* @brief 读取轴运动的最大设置速度
* @param axis：      轴号 4轴卡0~3 8轴卡0~7
vel：最大速度
* @return 成功返回0
*/
SLDM_API SLDM_GetVelM(short mc, short axis, float* vel);

/**
* @brief 读取轴运动的最大设定加速度
* @param axis：      轴号 4轴卡0~3 8轴卡0~7
acc：最大加速度
* @return 成功返回0
*/
SLDM_API SLDM_GetAccM(short mc, short axis, float* acc);
/**
* @brief 读取轴运动的最大设定减速度
* @param axis：      轴号 4轴卡0~3 8轴卡0~7
dec：最大减速度
* @return 成功返回0
*/
SLDM_API SLDM_GetDecM(short mc, short axis, float* dec);

//----------------------------- @End 兼容旧版本函数 -----------------------------

/**********************************************************************************
* @End 点位运动
*********************************************************************************/

#pragma endregion


#pragma region 位置锁存

/********************************************************************************
* @Begin 位置锁存
*******************************************************************************/

/**
* @brief 启动锁存功能
* @param mc：        控制器索引
*        unit：     锁存单元选择 0~1
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_LatchBegin(_In_ short mc, _In_ short unit);

/**
* @brief 结束锁存功能
* @param mc：        控制器索引
*        unit：     锁存单元选择 0~1
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_LatchEnd(_In_ short mc, _In_ short unit);

/**
* @brief 设置位置锁存的参数
* @param mc：        控制器索引
*        unit：      锁存单元号，取值范围：0~7
*        src：       位置源，1-编码器位置，0-命令位置
*        axis：      位置源的轴号，取值范围：控制器支持的轴号
*        trigger：   触发信号源，取值范围：
*                       0-无效
*                       1-编码器Z相信号
*                       2-零点开关
*				        3-通用输入
*                       4-位置比较输出
*        edge：      触发边沿：0-上升沿，1-下降沿，2-上升下降沿
*        index：     触发信号源的索引，取值范围：控制器支持的轴号或输入
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_SetLatchParam(_In_ short mc, _In_ short unit, _In_ short src, _In_ short axis, _In_ short trigger, _In_ short edge, _In_ short index);

/**
* @brief 读取锁存单元的锁存的位置
* @param mc：        控制器索引
*        unit：      锁存单元号,取值范围：0~7
*		 pos：       锁存位置数组，其大小取决于用户，锁存的最大位值数256
*		 numofcapt： 本次锁存过程中，已锁存的位置数
* @return 成功返回0
* @remark 在调用此函数前，必须调用SLDM_LatchEnd
*/
SLDM_API SLDM_GetLatchPos(_In_ short mc, _In_ short unit, _Out_ int* pos, _Out_ short* numofcapt);

/**
* @brief 读取锁存单元的已锁存的位置个数
* @param mc：        控制器索引
*        unit：      锁存单元号，取值范围：0~7
*		 numofcapt： 已捕捉的位置个数
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_GetLatchNum(_In_ short mc, _In_ short unit, _Out_ short* numofcapt);

/********************************************************************************
* @End 位置锁存
*******************************************************************************/

#pragma endregion


#pragma region 位置比较

/********************************************************************************
* @Begin 位置比较
*******************************************************************************/

/**
* @brief 启动位置比较功能
* @param mc：        控制器索引
*         unit：     位置比较单元选择 0~1
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_CompareBegin(_In_ short mc, _In_ short unit);

/**
* @brief 结束位置比较功能
* @param mc：        控制器索引
*       unit：     位置比较单元选择 0~1
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_CompareEnd(_In_ short mc, _In_ short unit);

/**
* @brief 设置位置比较的参数
* @param mc：        控制器索引
*        unit：      比较单元号，取值范围：0~7
*		 src：       位置源，0-命令位置，1-编码器位置
*		 axis：      位置源的轴号，取值范围：控制器支持的轴号
*        portbit：   比较输出端口选择，取值范围：0~15
*        portpol：   比较输出端口极性，1-正逻辑，上升沿；0-负逻辑，下降沿
*        porten：    比较输出端口使能，1-比较输出，0-通用输出
*        width：     比较输出的宽度，取值范围：1~2^32，单位：us
* @return 成功返回0
* @remark 位置比较单元可以关联不同的轴号
*/
SLDM_API SLDM_SetCompareParam(_In_ short mc, _In_ short unit, _In_ short src, _In_ short axis, _In_ short portbit, _In_ short portpol, _In_ short porten, _In_ unsigned int width);

/**
* @brief 设置比较的位置
* @param mc：        控制器索引
*        unit：      比较单元选择，取值范围：0~7
*		 pos：       比较的位置数据数组
*		 numtocmpr： 比较位置的个数，取值范围：1~256
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_SetComparePos(_In_ short mc, _In_ short unit, _In_ int* pos, _In_ short numtocmpr);

/**
* @brief 读取已完成的比较位置的个数
* @param mc：        控制器索引
*        unit：      比较单元号，取值范围：0~7
*        numofcmpr： 返回已完成比较位置的个数
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_GetCompareNum(_In_ short mc, _In_ short unit, _Out_ short* numofcmpr);

/*************************************************************************
 * @End 位置比较
 ************************************************************************/

#pragma endregion


#pragma region 在线更改位置和速度

/*************************************************************************
* @Begin 在线更改位置和速度
************************************************************************/

/**
* @brief 在线更改位置
* @param mc：        控制器索引
*        axis：      轴号，取值范围：控制器支持的轴号
*        pos：       绝对位置，单位：p或物理量纲分辨率，取值范围：+/-2^31
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_ChangeFlyPos(_In_ short mc, _In_ short axis, _In_ int pos);

/**
* @brief 在线更改速度
* @param mc：        控制器索引
*        axis：      轴号，取值范围：控制器支持的轴号或输入
*        vel：       速度，单位：p(或物理量纲分辨率)/ms，取值范围：0.001~8000.0
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_ChangeFlySpeed(_In_ short mc, _In_ short axis, _In_ float vel);

/*************************************************************************
* @End 在线更改位置和速度
************************************************************************/

#pragma endregion

 
#pragma region 属性设置

/*************************************************************************
* @Begin 属性设置
************************************************************************/

/**
* @brief 报警复位
* @param mc：        控制器索引
*		  axis：      轴号 4轴卡0~3 8轴卡0~7
*		 on：        复位信号，1-设置复位，0-关闭复位
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_ResetAlarm(_In_ short mc, _In_ short axis, _In_ short on);

/**
* @brief 伺服激活
* @param mc：        控制器索引
*		  axis：      轴号 4轴卡0~3 8轴卡0~7
*		 on：        伺服激活，1-激活，0-失效
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_SetServoOn(_In_ short mc, _In_ short axis, _In_ short on);

/**
* @brief 配置EMG
* @param mc：        控制器索引
*		 en：        EMG使能，0-无效，仅作为通用输入
*		 pol：       EMG的极性，0-正极性，1-负极性
*        port：      EMG端口，取值范围：-1 - EMG专用端口，0~15 - 通用输入口0~15
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_SetEMGConfig(_In_ short mc, _In_ short en, _In_ short pol, _In_ short port = -1);

/**
* @brief 设置零点开关极性
* @param mc：        控制器索引
*         axis：      轴号 4轴卡0~3 8轴卡0~7
*		 pol：       零点开关极性，1-正，0-负
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_SetHomePol(_In_ short mc, _In_ short axis, _In_ short pol);

/**
* @brief 设置正向限位开关有效性
* @param mc：        控制器索引
*         axis：      轴号 4轴卡0~3 8轴卡0~7
*		 on：        开关，1-开，0-关
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_SetPOTOn(_In_ short mc, _In_ short axis, _In_ short on);

/**
* @brief 设置负向限位开关有效性
* @param mc：        控制器索引
*         axis：      轴号 4轴卡0~3 8轴卡0~7
*		 on：        开关，1-开，0-关
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_SetNOTOn(_In_ short mc, _In_ short axis, _In_ short on);

/**
* @brief 设置正向限位开关极性
* @param mc：        控制器索引
*         axis：      轴号 4轴卡0~3 8轴卡0~7
*		 pol：       极性，1-正，0-负
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_SetPOTPol(_In_ short mc, _In_ short axis, _In_ short pol);

/**
* @brief 设置负向限位开关极性
* @param mc：        控制器索引
*         axis：      轴号 4轴卡0~3 8轴卡0~7
*		 pol：       极性，1-正，0-负
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_SetNOTPol(_In_ short mc, _In_ short axis, _In_ short pol);

/**
* @brief 设置编码器有效性
* @param mc：        控制器索引
*         axis：      轴号 4轴卡0~3 8轴卡0~7
*		 on：        有效性，1-有效，0-无效
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_SetEncoderOn(_In_ short mc, _In_ short axis, _In_ short on);

/**
* @brief 设置编码器的计数极性
* @param mc：        控制器索引
*         axis：      轴号 4轴卡0~3 8轴卡0~7
*		 pol：       极性，1-正，0-负
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_SetEncoderPol(_In_ short mc, _In_ short axis, _In_ short pol);

/**
* @brief 设置编码器索引信号极性
* @param mc：        控制器索引
*         axis：      轴号 4轴卡0~3 8轴卡0~7
*		 pol：       极性，1-正，0-负
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_SetEncoderIndexPol(_In_ short mc, _In_ short axis, _In_ short pol);

/**
* @brief 设置报警信号有效性
* @param mc：        控制器索引
*         axis：      轴号 4轴卡0~3 8轴卡0~7
*		 on：        有效性，1-有效，0-无效
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_SetAlarmOn(_In_ short mc, _In_ short axis, _In_ short on);

/**
* @brief 设置报警信号的极性
* @param mc：        控制器索引
*         axis：      轴号 4轴卡0~3 8轴卡0~7
*		 pol：       极性，1-正，0-负
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_SetAlarmPol(_In_ short mc, _In_ short axis, _In_ short pol);

/**
* @brief 设置脉冲输出方式
* @param mc：        控制器索引
*        axis：      轴号，取值范围：控制器支持的轴号
*		 mode：      模式，1-CCW+CW，0-PULSE+DIR
*		 edge：      边沿，0-上升沿，1-下降沿，默认值为0
*		 dirpol：    脉冲方向的极性，1-正极性，0-负极性，默认值为0
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_SetPulseMode(_In_ short mc, _In_ short axis, _In_ short mode, _In_ short edge = 0, _In_ short dirpol = 0);

/**
* @brief 设置轴位移是按模数运动
* @param mc：        控制器索引
*         axis：      轴号 4轴卡0~3 8轴卡0~7
*		 mod：       是否按位移模数运动，1-按模数位移，0-按广义直线位移
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_SetModular(_In_ short mc, _In_ short axis, _In_ short mod);

/**
* @brief 设置电子齿轮比
* @param mc：        控制器索引
*        axis：      轴号，取值范围：控制器支持的轴号
*        master：    主动轮齿数，取值范围：1~2^32
*        slave：     从动轮齿数，取值范围：1~2^32
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_SetGearRatio(_In_ short mc, _In_ short axis, _In_ unsigned int master, _In_ unsigned int slave);

/**
* @brief 设置轴的脉冲当量
* @param mc：        控制器索引
*        axis：      轴号，取值范围：控制器支持的轴号
*        pulse：     脉冲数，单位：p，取值范围：1~2^32
*        disp：      位移，单位：物理量纲分辨率，取值范围：1~2^32
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_SetPulseRatio(_In_ short mc, _In_ short axis, _In_ unsigned int pulse, _In_ unsigned int disp);

/**
* @brief 设置辅助编码器的脉冲当量
* @param mc：       控制器索引
*        auxenc：   辅助编码器索引，取值范围：0,1
*        pulse：    脉冲数，单位：p，取值范围：1~2^32
*        disp：     位移，单位：物理量纲分辨率，取值范围：1~2^32
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_SetPulseRatioAux(_In_ short mc, _In_ short auxenc, _In_ unsigned int pulse, _In_ unsigned int disp);

/*************************************************************************
* @End 属性设置
************************************************************************/

#pragma endregion

 
#pragma region 电子齿轮操作

/****************************************************************************************
* @Begin 电子齿轮操作
****************************************************************************************/
/**
* @brief 设置电子齿轮的属性
* @param mc：        控制器索引
*        slave：     从动轴号，取值范围：控制器支持的轴号
*		 master：    主动轴号，取值范围：控制器支持的轴号
*		 dir：       跟随方向，取值范围：0 - 同向，1 - 反向
*		 possrc：    跟随位置源，取值范围：0 - 命令位置，1 - 实际位置
*		 mode：      电子齿轮跟随模式，取值范围：0 - 位置跟随，1 - 速度跟随
*		 trans：     飞行改变齿轮比的平滑过渡距离
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_EGearSetParam(_In_ short mc, _In_ short slave, _In_ short master, _In_ short dir, _In_ short possrc, _In_ short mode, _In_ int trans);

/**
* @brief 启用电子齿轮
* @param mc：        控制器索引
*         axis：      轴号 4轴卡0~3 8轴卡0~7
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_EGearStart(_In_ short mc, _In_ short axis);

/**
* @brief 停止电子齿轮
* @param mc：        控制器索引
*         axis：      轴号 4轴卡0~3 8轴卡0~7
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_EGearStop(_In_ short mc, _In_ short axis);

/**
* @brief 读取各轴的电子齿轮激活状态
* @param mc：        控制器索引
*        stat：      按位掩码表示的各轴电子齿轮生效状态，各位含义：0-失效，1-激活
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_EGearGetStatus(_In_ short mc, _Out_ unsigned int* stat);

/****************************************************************************************
* @End 电子齿轮操作
****************************************************************************************/

#pragma endregion


#pragma region IO操作

/****************************************************************************************
* @Begin IO操作
****************************************************************************************/

/**
* @brief 设置IO站的配置信息
* @param mc：        控制器索引
*        node：      站地址，取值范围：0-控制器本地，1~15：扩展站点
*		 type：      站点IO类型，取值范围：0-数字IO，其它值-预留
*		 bytesofin： 本站点的输入字节数
*		 bytesofout：本站点输出字节数
*        valid：     站点有效性，取值范围：1-本站点有效，0-本站点无效，默认值为1
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_SetIOConfig(_In_ short mc, _In_ short node, _In_ short type, _In_ short bytesofin, _In_ short bytesofout, _In_ short valid = 1);

/**
* @brief 读取IO站的配置状态
* @param mc：        控制器索引
*        node：      站地址，取值范围：0-控制器本地，1~15：扩展站点
*		 type：      站点IO类型，取值范围：0-数字IO，其它值-预留
*		 bytesofin： 本站点的输入字节数
*		 bytesofout：本站点输出字节数
*        stat：      站点的配置状态，值含义：-1 - 站通讯超时；0 - 无效站点；1 - 站通讯正常
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_GetIOConfig(_In_ short mc, _In_ short node, _Out_ short* type, _Out_ short* bytesofin, _Out_ short* bytesofout, _Out_ short* stat);

/**
* @brief 保存IO站的配置信息
* @param mc：        控制器索引
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_SaveIOConfig(_In_ short mc);

/**
* @brief 启动当前配置
* @param mc：        控制器索引
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_StartIOConfig(_In_ short mc);

/**
* @brief 恢复出厂配置信息
* @param mc：        控制器索引
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_ResetIOConfig(_In_ short mc);

/**
* @brief 按位设置输出的端口状态
* @param mc：        控制器索引
*		 node：      站地址，取值范围：0~15
*		 index：     本站内的输出端口位索引
*		 data：      输出值，取值范围：0，1
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_SetOutBit(_In_ short mc, _In_ short node, _In_ short index, _In_ unsigned short data);

/**
* @brief 按字节设置输出的端口状态
* @param mc：        控制器索引
*		 node：      站地址，取值范围：0~15
*		 index：     本站内的输出端口的字节索引
*		 data：      输出值，取值：0~0xFF
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_SetOutByte(_In_ short mc, _In_ short node, _In_ short index, _In_ unsigned short data);

/**
* @brief 按字设置输出的端口状态
* @param mc：        控制器索引
*		 node：      站地址，取值范围：0~15
*		 index：     本站内的输出端口的字索引
*		 data：      输出值，取值：0~0xFFFF
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_SetOutWord(_In_ short mc, _In_ short node, _In_ short index, _In_ unsigned short data);

/**
* @brief 按双字设置输出的端口状态
* @param mc：        控制器索引
*		 node：      站地址，取值范围：0~15
*		 index：     本站内的输出端口的双字索引
*		 data：      输出值，取值：0~0xFFFFFFFF
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_SetOutDWord(_In_ short mc, _In_ short node, _In_ short index, _In_ unsigned int data);

/**
* @brief 按位读取输出端口状态
* @param mc：        控制器索引
*        node：      站地址，取值范围：0~15
*		 index：     本站内的输出端口的位索引
*		 data：      端口状态值，返回值：0,1
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_GetOutBit(_In_ short mc, _In_ short node, _In_ short index, _Out_ unsigned short* data);

/**
* @brief 按字节读取输出端口状态
* @param mc：        控制器索引
*        node：      站地址，取值范围：0~15
*		 index：     本站内的输出端口的字节索引
*		 data：      端口状态值，返回值：0~0xFF
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_GetOutByte(_In_ short mc, _In_ short node, _In_ short index, _Out_ unsigned short* data);

/**
* @brief 按字读取输出端口状态
* @param mc：        控制器索引
*        node：      站地址，取值范围：0~15
*		 index：     本站内的输出端口的字索引
*		 data：      端口状态值，返回值：0~0xFFFF
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_GetOutWord(_In_ short mc, _In_ short node, _In_ short index, _Out_ unsigned short* data);

/**
* @brief 按双字读取输出端口状态
* @param mc：        控制器索引
*        node：      站地址，取值范围：0~15
*		 index：     端口状态值双字索引
*		 data：      端口状态值，返回值：0~0xFFFFFFFF
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_GetOutDWord(_In_ short mc, _In_ short node, _In_ short index, _Out_ unsigned int* data);

/**
* @brief 按位读取输入端口状态
* @param mc：        控制器索引
*        node：      站地址，取值范围：0~15
*		 index：     本站内输入端口的位索引
*		 data：      端口状态值，返回值：0，1
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_GetInBit(_In_ short mc, _In_ short node, _In_ short index, _Out_ unsigned short* data);

/**
* @brief 按字节读取输入端口状态
* @param mc：        控制器索引
*        node：      站地址，取值范围：0~15
*		 index：     本站内输入端口的字节索引
*		 data：      端口状态值，返回值：0~0xFF
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_GetInByte(_In_ short mc, _In_ short node, _In_ short index, _Out_ unsigned short* data);

/**
* @brief 按字读取输入端口状态
* @param mc：        控制器索引
*        node：      站地址，取值范围：0~15
*		 index：     本站内输入端口的字索引
*		 data：      端口状态值，返回值：0~0xFFFF
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_GetInWord(_In_ short mc, _In_ short node, _In_ short index, _Out_ unsigned short* data);

/**
* @brief 按双字读取输入端口状态
* @param mc：        控制器索引
*        node：      站地址，取值范围：0~15
*		 index：     本站内输入端口的双字索引
*		 data：      端口状态值，返回值：0~0xFFFFFFFF
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_GetInDWord(_In_ short mc, _In_ short node, _In_ short index, _Out_ unsigned int* data);

/**
* @brief 读取EMG的状态
* @param mc：        控制器索引
*        level：     所选择的急停端口的电平
*        status：    急停的作用状态，0-无效，1-有效
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_GetEMG(_In_ short mc, _Out_ short* level, _Out_ short* status);


/***********************************************************************************************
* @End IO操作
**********************************************************************************************/

#pragma endregion


#pragma region 状态操作

/***********************************************************************************************
 * @Begin 状态操作
 **********************************************************************************************/

 /**
 * @brief 读取各轴正向超程开关状态
 * @param mc：        控制器索引
 *        POT：       按位掩码表示的正向超程开关状态
 * @return 成功返回0
 * @remark
 */
SLDM_API SLDM_GetPOT(_In_ short mc, _Out_ unsigned int* POT);

/**
* @brief 读取各轴负向超程开关状态
* @param mc：        控制器索引
*        NOT：       按位掩码表示的负向超程开关状态
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_GetNOT(_In_ short mc, _Out_ unsigned int* NOT);

/**
* @brief 读取各轴的零点开关状态
* @param mc：        控制器索引
*        home：      按位掩码表示的各轴零点开关状态
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_GetHome(_In_ short mc, _Out_ unsigned int* home);

/**
* @brief 读取各轴的运行状态
* @param mc：        控制器索引
*        running：   按位掩码表示的各轴运行状态，各位含义：1-运行中，0-停止
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_GetRunning(_In_ short mc, _Out_ unsigned int* running);

/**
* @brief 读取各轴运动到位标志
* @param mc：        控制器索引
*        reached：   按位掩码表示的运动到位标志，各位含义：1-到位，0-未到位
* @return 成功返回0
* @remark 此标志反映的是伺服驱动器输出的到位信号
*/
SLDM_API SLDM_GetReached(_In_ short mc, _Out_ unsigned int* reached);

/**
* @brief 读取各轴正在回零过程的状态
* @param mc：        控制器索引
*        homing：    按位掩码表示的各轴正在回零状态，各位含义：1-正在回零过程，0-未在回零过程
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_GetHoming(_In_ short mc, _Out_ unsigned int* homing);

/**
* @brief 读取各轴已完成回零的状态
* @param mc：        控制器索引
*        homed：     按位掩码表示的各轴已完成回零的状态，各位含义：1-已完成回零，0-未完成回零
* @return 成功返回0
* @remark
*/
SLDM_API	SLDM_GetHomed(_In_ short mc, _Out_ unsigned int* homed);

/**
* @brief 读取各轴电机报警信号状态
* @param mc：        控制器索引
*        alm：       按位掩码表示的电机报警状态，各位含义：1-已报警，0-未报警
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_GetAlarm(_In_ short mc, _Out_ unsigned int* alm);

/**
* @brief 读取各轴电机报警信号
* @param mc：        控制器索引
*        axis：      轴号，取值范围：控制器支持的轴号
*        maincode：  主报警号
*		 subcode：   次报警号
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_GetAlarmCode(_In_ short mc, _In_ short axis, _Out_ short* maincode, _Out_ short* subcode);

/**
* @brief 读取各轴正向软限位标志
* @param mc：        控制器索引
*        psl：       按位掩码表示的各轴正向软限位标志，各位含义：1-处于正向软限位，0-未处于正向软限位
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_GetPSLFlag(_In_ short mc, _Out_ unsigned int* psl);

/**
* @brief 读取各轴负向软限位标志
* @param mc：        控制器索引
*        nsl：       按位掩码表示的各轴负向软限位标志，各位含义：1-处于负向软限位，0-未处于负向软限位
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_GetNSLFlag(_In_ short mc, _Out_ unsigned int* nsl);

/**
* @brief 读取各轴伺服使能状态
* @param mc：        控制器索引
*        status：    按位掩码表示的各轴伺服激活状态，各位含义：1-使能，0-未使能
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_GetServoStatus(_In_ short mc, _Out_ unsigned int* status);

/**
* @brief 读取各轴伺服监测数据
* @param mc：        控制器索引
*        axis：      轴号，取值范围：控制器支持的轴号
*        data：      监测数据
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_GetMonitorData(_In_ short mc, _In_ short axis, _Out_ int* data);

/***********************************************************************************************
* @End 状态操作
**********************************************************************************************/

#pragma endregion


#pragma region 大数据传送状态

/***********************************************************************************
* @Begin 大数据传送状态检测
***********************************************************************************/

/**
* @brief 大数据传送的一次往返循环是否结束
* @param mc：        控制器索引
* @return 0：一次传送结束，1：一次传送未结束
* @remark
*/
SLDM_API SLDM_GetHugeDataBusy(_In_ short mc);

/**
* @brief 大数据传送的一次往返循环是否成功
* @param mc：        控制器索引
* @return 0：成功，非0：错误
* @remark
*/
SLDM_API SLDM_GetHugeDataError(_In_ short mc);

/***********************************************************************************
* @End 大数据传送状态检测
***********************************************************************************/

#pragma endregion


#pragma region 文件操作


/***********************************************************************************************
* @Begin 文件操作
**********************************************************************************************/

/**
* @brief 读取文件状态
* @param mc：        控制器索引
*		 index：     文件索引，取值范围：
*		 status：    文件状态，各位的含义：
*                       bit0 - 文件打开标志，1：文件已打开，0：文件未打开
*                       bit1 - 文件传送标志，1：文件正在传送，0：文件停止传送
*                       bit2 - 文件头标志，1：文件头
*                       bit3 - 文件尾标志，1：文件尾
*                       bit4 - 保留
*                       bit5 - 保留
*                       bit6 - 文件错误标志，1：文件错误，0：文件正确
*                       bit7 - 保留
*                       bit8~15 - 保留
*                    bytesofrw：已传送的字节数
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_FileGetStatus(_In_ short mc, _In_ short index, _Out_ unsigned short* status, _Out_ int* bytesofrw);

/**
* @brief 打开读写文件
* @param mc：        控制器索引
*		 index：     文件索引，取值范围：
*		 buf：       文件缓冲区
*		 bytestorw： 需要传送的字节数
*		 access：    访问方式，取值范围：0（读） - 从控制器传送到服务，1（写）- 从服务传送到控制器
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_FileOpen(_In_ short mc, _In_ short index, _In_ char* buf, _In_ int bytestorw, _In_ short access);

/**
* @brief 关闭读写文件
* @param mc：        控制器索引
*		 index：     文件索引，取值范围：
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_FileClose(_In_ short mc, _In_ short index);

/**
* @brief 文件传送启动
* @param mc：        控制器索引
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_FileRW(_In_ short mc);


/***********************************************************************************************
* @End 文件操作
**********************************************************************************************/

#pragma endregion


#pragma region 串口通讯

/*************************************************************************
* @Begin 串口通讯
************************************************************************/

/**
* @brief 设置串口通讯的参数
* @param mc：        控制器索引
*		  com：      串口索引，0-RS485，1-RS232A，2-RS232B
*		  baud：     串口波特率，2400,4800,9600,19200,38400,57600,115200,等
*		  databits： 串口数据位数，5,6,7,8
*		  stopbits： 串口数据停止位数，1,2
*		  partiy：   出口数据奇偶校验，0-无校验，1-奇校验，2-偶校验
*		  mode：     串口工作模式，0-自由协议模式，1-Modebus RTU Slave，2-Modbus RTU Master，15-单指令运动命令
*		  timeout：  串口通讯超时设定，0~65535
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_SetComParam(_In_ short mc, _In_ short com, _In_ int baud, _In_ short databits, _In_ short stopbits, _In_ short parity, _In_ short mode, _In_ unsigned short timeout);

/**
* @brief 串口通讯中数据交换的字节读
* @param mc：        控制器索引
*		  bptr：     需要读取的数据交换区的字节地址，取值范围：0~63
*		  numtoread：需要读取的字节数
*		  data：     用户声明的数据缓冲区
*		  numofread：已读取的字节数
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_GetComDataByte(_In_ short mc, _In_ short bptr, _In_ short numtoread, _Out_ short* data, _Out_ short* numofread);

/**
* @brief 串口通讯中数据交换的字读
* @param mc：        控制器索引
*		  wptr：     需要读取的数据交换区的字地址，取值范围：0~31
*		  numtoread：需要读取的字数
*		  data：     用户声明的数据缓冲区
*		  numofread：已读取的字数
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_GetComDataWord(_In_ short mc, _In_ short wptr, _In_ short numtoread, _Out_ unsigned short* data, _Out_ short* numofread);

/**
* @brief 串口通讯中数据交换的双字读
* @param mc：        控制器索引
*		  dwptr：    需要读取的数据交换区的双字地址，取值范围：0~15
*		  numtoread：需要读取的双字数
*		  data：     用户声明的数据缓冲区
*		  numofread：已读取的双字数
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_GetComDataDWord(_In_ short mc, _In_ short dwptr, _In_ short numtoread, _Out_ unsigned int* data, _Out_ short* numofread);

/**
* @brief 串口通讯中数据交换的浮点数读
* @param mc：        控制器索引
*		  fptr：     需要读取的数据交换区的浮点数地址，取值范围：0~15
*		  numtoread：需要读取的浮点数的数量
*		  data：     用户声明的数据缓冲区
*		  numofread：已读取的浮点数的数量
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_GetComDataFloat(_In_ short mc, _In_ short fptr, _In_ short numtoread, _Out_ float* data, _Out_ short* numofread);

/**
* @brief 串口通讯数据交换的字节写
* @param mc：        控制器索引
*		  bptr：     需要写的数据交换区的字节地址，取值范围：0~63
*		  num：      需要写的字节数
*		  data：     用户声明的数据缓冲区
*		  numofwritten：已写的字节数
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_SetComDataByte(_In_ short mc, _In_ short bptr, _In_ short numtowrite, _In_ short* data, _In_ short* numofwritten);

/**
* @brief 串口通讯数据交换的字写
* @param mc：        控制器索引
*		  wptr：     需要写的数据交换区的字地址，取值范围：0~31
*		  num：      需要写的字数
*		  data：     用户声明的数据缓冲区
*		  numofwritten：已写的字数
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_SetComDataWord(_In_ short mc, _In_ short wptr, _In_ short numtowrite, _In_ unsigned short* data, _In_ short* numofwritten);

/**
* @brief 串口通讯数据交换的双字写
* @param mc：        控制器索引
*		  dwptr：    需要写的数据交换区的双字地址，取值范围：0~15
*		  num：      需要写的双字数
*		  data：     用户声明的数据缓冲区
*		  numofwritten：已写的双字数
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_SetComDataDWord(_In_ short mc, _In_ short dwptr, _In_ short numtowrite, _In_ unsigned int* data, _In_ short* numofwritten);

/**
* @brief 串口通讯数据交换的浮点数写
* @param mc：        控制器索引
*		  fptr：     需要写的数据交换区的浮点数地址，取值范围：0~15
*		  num：      需要写的浮点数的数量
*		  data：     用户声明的数据缓冲区
*		  numofwritten：已写的浮点数的数量
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_SetComDataFloat(_In_ short mc, _In_ short fptr, _In_ short numtowrite, _In_ float* data, _In_ short* numofwritten);

/**
* @brief Modbus寄存器(16bit)读命令
* @param mc：        控制器索引
*		  com：      串口索引，取值范围：0-RS485， 1-RS232A，2-RS232B
*		  addr：     从站地址
*		  reg：      寄存器起始地址
*		  num：      寄存器数量
*		  wptr：     数据交换区索引，取值范围：0~31
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_ModbusReadReg(_In_ short mc, _In_ short com, _In_ short addr, _In_ unsigned short reg, _In_ unsigned short num, _In_ short wptr);

/**
* @brief Modbus寄存器(16bit)写命令
* @param mc：        控制器索引
*		  com：      串口索引，取值范围：0-RS485， 1-RS232A，2-RS232B
*		  addr：     从站地址
*		  reg：      寄存器起始地址
*		  num：      寄存器数量
*		  wptr：     数据交换区索引，取值范围：0~31
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_ModbusWriteReg(_In_ short mc, _In_ short com, _In_ short addr, _In_ unsigned short reg, _In_ unsigned short num, _In_ short wptr);

/**
* @brief 启动串口通讯
* @param mc：        控制器索引
*		  com：      串口索引，取值范围：0-RS485， 1-RS232A，2-RS232B
*		  oneshot：  通讯握手方式，取值范围：0-连续通讯方式，1-单次通讯方式
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_ComStart(_In_ short mc, _In_ short com, _In_ short oneshot);

/**
* @brief 停止串口通讯
* @param mc：        控制器索引
*		  com：      串口索引，取值范围：0-RS485， 1-RS232A，2-RS232B
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_ComStop(_In_ short mc, _In_ short com);

/**
* @brief 读串口通讯状态
* @param mc：        控制器索引
*		  com：      串口索引，取值范围：0-RS485，1-RS232A，2-RS232B
*		  stat：     串口通讯状态，状态值含义，
*				        bit 0 - 串口运行状态，0：空闲，1：正在通讯
*				        bit 1 -
*				        bit 2 -
*				        bit 3 - modbus校验错误，0：无错，1：CRC错误
*				        bit 4 -
*				        bit 5 -
*				        bit 6 -
*				        bit 7 - 串口通讯超时，0：无超时，1：超时
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_GetComStatus(_In_ short mc, _In_ short com, _Out_ short* stat);

/*************************************************************************
* @End 串口通讯
************************************************************************/

#pragma endregion


#pragma region 系统信息


/***********************************************************************************
* @Begin 系统信息指令
***********************************************************************************/

/**
* @brief 读取控制器的控制周期计数
* @param mc：        控制器索引
*        ticks：     控制周期计数
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_GetMcTicks(_In_ short mc, _Out_ unsigned int* ticks);

/**
* @brief 读取控制器的特性参数
* @param mc：        控制器索引
*		 period：    控制周期，单位：ns
*        axis_num：  控制器的最大轴数
*        ch_max：    控制器的最大通道数
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_GetMcProperty(_In_ short mc, _Out_ unsigned int* period, _Out_ short* axis_num, _Out_ short* ch_max);

/**
* @brief 读取控制器的接口资源
* @param mc：        控制器索引
*		 di_num：    数字输入端口数
*		 do_num：    数字输出端口数
*		 ai_num：    模拟输入端口数
*		 ao_num：    模拟输出端口数
*		 pi_num：    脉冲输入端口数
*		 po_num：    脉冲输出端口数
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_GetMcIOResource(_In_ short mc, _Out_ short* di_num, _Out_ short* do_num, _Out_ short* ai_num, _Out_ short* ao_num, _Out_ short* pi_num, _Out_ short* po_num);

/**
* @brief 读取控制器的型号
* @param mc：        控制器索引
*        model：     控制器型号
*        type：      控制器功能类型
*		 ver：       版本号
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_GetMcModel(_In_ short mc, _Out_ short* model, _Out_ short* type, _Out_ unsigned short* ver);

/**
* @brief 设置控制器时钟
* @param mc：        控制器索引
*		  year：     年
*		  month：    月
*		  mday：     日
*		  wday：     星期
*		  hour：     小时
*		  min：      分
*		  sec：      秒
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_SetRTC(_In_ short mc, _In_ short year, _In_ short month, _In_ short mday, _In_ short wday, _In_ short hour, _In_ short min, _In_ short sec);

/**
* @brief 读取控制器时钟
* @param mc：        控制器索引
*		  year：     年
*		  month：    月
*		  mday：     日
*		  wday：     星期
*		  hour：     小时
*		  min：      分
*		  sec：      秒
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_GetRTC(_In_ short mc, _Out_ short* year, _Out_ short* month, _Out_ short* mday, _Out_ short* wday, _Out_ short* hour, _Out_ short* min, _Out_ short* sec);

/**
* @brief 读取控制器动态握手周期
* @param mc：        控制器索引
*		  span：     握手间隔，单位：us
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_GetHandshakeSpan(_In_ short mc, _Out_ int* span);

/**
* @brief 读取控制器的产品序列号
* @param pid：产品序列号
* @return 成功返回0
*/
SLDM_API SLDM_GetSysPID(short mc, unsigned int * pid);

/**
* @brief 读取控制器的IP地址第四段
* @param ip4：第四段地址指针
* @return 成功返回0
*/
SLDM_API SLDM_GetSysIP(short mc, short * ip4);

/**
* @brief 读取控制器功能类型
* @param 
* @return 成功返回0
*/
SLDM_API SLDM_GetSysFnType(short mc, short * fn);

/***********************************************************************************
* @End 系统信息指令
***********************************************************************************/

#pragma endregion


#pragma region 其它

/*DLL测试函数，内部使用*/
SLDM_API SLDM_Test(_In_ short mc);

/**
* @brief 日志打印服务
* @param bLogEnable：        是否启用信息打印，信息 输出到debugview
*        bKeepLogFile：      操作操作信息是否保存到文件 文件名字SOLID_ECard_(YMD).txt
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_LogEnable(_In_ bool bLogEnable,_In_ bool bKeepLogFile);

#pragma endregion

/**
* @brief 绝对式P2P运动，单轴运动
* @param mc：        控制器索引
*		 axis：      轴号数组，取值范围：控制器支持的轴号
*		 pos：       目标位置数组，单位：p或物理量纲分辨率，取值范围：±2^31
*        s_vel：   起跳速度，单位：p(或物理量纲分辨率)/ms，取值范围：0.001~8000.000
*        m_vel：   目标速度，单位：p(或物理量纲分辨率)/ms，取值范围：0.001~8000.000
*        acc：       目标加速度，单位：p(或物理量纲分辨率)/ms^2，取值范围：0.001~4000.000
*        dec：       目标减速度，单位：p(或物理量纲分辨率)/ms^2，取值范围：0.001~4000.000
*        smth：      平滑时间，单位：ms，取值范围：0~65535，默认值0，表示梯形运动，大于0则为S曲线运动
*        dist_mode:0 表示相对运动，其他数表示绝对运动
* @return 成功返回0
* @remark
*/
SLDM_API SLDM_PTP_move(_In_ short mc,_In_ short *p_axis,_In_ int *pos,_In_ float s_vel, _In_ float m_vel,_In_  float acc, _In_ float dec,_In_ unsigned short nSmooth, _In_ int dist_mode);


/**
 * @brief 注册响应输入信号的回调函数
 * @param fn：回调函数指针
 *		  node：IO站点
 *		  byte：本站的输入字节索引
 *		  bit： 本站的输入字节的位索引
 *		  edge：边沿，0-上升沿，1-下降沿
 * @return 成功返回0
 */

SLDM_API SLDM_RegisterCallback(short mc,void(*fn)(), short node, short byte, short bit, short edge);

/**
 * @brief 取消响应输入信号的回调函数
 * @param fn：回调函数指针
 * @return 成功返回0
 */

SLDM_API SLDM_UnregisterCallback(short mc,void(*fn)());

