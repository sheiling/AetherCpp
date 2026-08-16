#ifndef _DISPLAY_CONTROL_DATA_HH
#define _DISPLAY_CONTROL_DATA_HH

#include <common/3rdparty/nlohmann/json.hpp>
#include <common/utility/json_tool/reflector/reflector.h>
#include <common/utility/thread_tool/thread_listener/thread_listener.h>
#include <stdint.h>
#include <string>
#include <functional>
#include <string>

namespace common
{
namespace device
{

#define SETTER(Type, name, Name)  void set##Name(const Type &name)  {  this->name = name;  }
#define GETTER(Type, name, Name)  Type get##Name() const  {  return this->name;  }

#define DCU_CHECK_BOOL_RTN_INFO(pDCU_Fun, resultComm, funStr)\
do\
{\
	bool rtn = pDCU_Fun;\
	if (!rtn)\
	{\
		CHECK_RTN_INFO(checkError((long)CommResults::DataSendError, funStr));\
	}\
	else\
	{\
		CHECK_RTN_INFO(checkError((long)resultComm, funStr));\
	}\
}while(false)

//#define SETTER_IMPL(Class, Type, name, Name) void Class::set##Name(const Type &name) { this->name = name; }
//#define GETTER_IMPL(Class, Type, name, Name) Type Class::get##Name() const { return this->name; }

//返回值对象
enum class CommResults
{
	OK = 0,						//通讯成功
	DataSendError,				//发送数据错误
	ParaErr,                    //参数错误
	Timeout,                    //超时
	RecvErrResp,                //收到错误回馈
	HardwareExcp,               //硬件操作失败
	NetErr,                     //网络故障
	FileErr,                    //文件操作失败
	LctRecvDataErr,             //接收数据错误
	LctRecvFileErr,             //接收文件错误
	ScrInfoErr,              	//显示屏信息不匹配
	NotSupport,					//不支持
	Cancelled,					//已取消
	FileSendNotExist,			//发送文件不存在
	Unkonwn,

};

//内部的一些错误
enum class PackageResults
{
	ok = 0,
	timeout,				//超时
	request_replyErr,		//发送的包对应的回包非法-不是回包
	file_errOpend,			//发送文件时文件打开失败，
	file_connectionNotOpend,//发送文件时端口未打开，
	file_FileErrWhenSend,	//发送文件时发生错误，

};

//包类型
enum class PackTypes
{
	Pack_OutCMD = 0,      //外部命令
	Pack_InnerCMD = 1     //内部命令
};

//包命令类型
enum class PackCMDTypes
{
	Pack_Common = 0,      //普通命令
	Pack_File = 1         //文件命令
};

//包来源类型
enum class PackFromTypes
{
	Pack_FromSocket = 0,  	//来自socket
	Pack_FromInnerErr = 1 	//来自内部处理
};

enum class enum_HardwareAccessType
{
	HardwareAccess_Lct = 0,			//socket->LCT
	HardwareAccess_Coex				//Coex
};

//请求类型
enum class enum_RequestType
{
	Read = 0,
	Send = 1
};

//通信回包状态
enum class enum_CommReplySta
{
	Wait = 0,
	RecvACK,
	RecvData
};

//标示透传请求成功与否的状态
enum class enum_PassthroughRequestResult
{
	NoRespond = 0,
	Fail = 1,
	Success = 2
};

//LCT支持功能列表
enum class enum_LCTSupportFeatureList
{
	F_XBITMODE = 0X00,  		//不是22bit+
	F_MULTILAYER = 0X01,		//全灰阶
	F_GRAYSCALE = 0X02,			//精细灰度
	F_MAGICGRAY = 0X03,			//.MG
	F_IS18BITMODE = 0X04,		//false为22bit+ true为18bit
	F_ADAPTIVE = 0X05,			//是否支持自适应
	// ...
	F_ProgramIsSimplyVersion = 31,
	F_FineGrayAdjustVer3 = 32,
	F_MatrixColorMangement2 = 33,
	F_FactoryBackUp = 34,
	F_BrightDarkLineFix = 35,
	F_BrightDarkLineFixCoefUploadSpeedUp = 36,
	F_CorrectionCoefCompress = 37,
	F_SetBrightDarkLineFixState = 38,
	F_ReadBrightDarkLineFixCoef = 39,
	F_MultiLayerCorrection = 40,
	F_MultiLayerCorrection_BOE = 41
};

//lct需传入的系数类型
enum class enum_LCTCoefType
{
	unKnow = 0,
	/// <summary>
	/// 正常校正
	/// </summary>
	Common = 1,
	/// <summary>
	///  低灰校正:模式一 (80801/11202)
	/// </summary>
	Lowlum = 2,
	/// <summary>
	///  蓝白校正
	/// </summary>  
	Bluewhite = 3,
	///<summary>
	/// 全灰阶校正
	///</summary>
	MultiLayer = 4,
	///<summary>
	/// 亮暗线
	///</summary
	DarkLine = 5,
	///< summary
	/// 低灰校正:模式二(9005/9937)
	///</summary>
	ChipLowlum = 6,
	///< summary
	/// 自适应校正
	/// summary
	SelfAdaption = 7
};

//命令码
enum class enum_LCTCommands
{
	INVALID_ID = 0X00,
	//Kill Display
	CP_KILLDISPLAY = 0X01,
	//Set Display Section
	CP_SETDISPLAYSECTION = 0X02,
	//Set Display Section RGB
	CP_SETDISPLAYSECTIONRGB = 0X03,
	//Write Coefficients
	CP_WEITECOEF = 0X04,
	//Write Coefficients to Flash
	CP_WRITECOEFTOFLASH = 0X05,
	//Check Screen Information
	CP_CHECKSCREENINFO = 0X06,
	//Set Correction Mode
	CP_SETCORRECTIONMODE = 0X07,
	//2.0.0.1104.801添加按像素校正
	/// <summary>
	/// 按像素设置显示区域
	/// </summary>
	CP_SETDISPLAYSECTIONBYPIXEL = 0X08,
	/// <summary>
	/// 按像素设置显示区域RGB
	/// </summary>
	CP_SETDISPLAYSECTIONRGBBYPIXEL = 0X09,
	/// <summary>
	/// 按像素写点的系数
	/// </summary>
	CP_WEITECOEFBYPIXEL = 0X0A,
	/// <summary>
	/// 按像素将点的系数保存到Falsh （异步接口 不再使用）
	/// </summary>
	CP_WRITECOEFTOFLASHBYPIXEL = 0X0B,
	/// <summary>
	/// <summary>
	/// 低灰模式获取
	/// </summary>
	CP_LOWGRAY_GETMODE = 0X81,
	/// <summary>
	/// 低灰参数设置
	/// </summary>
	CP_LOWGRAY_SETMODE = 0X86,
	/// 关闭或开启校正
	/// </summary>
	CP_ENABELCORRECTION = 0X0C,
	/// <summary>
	/// 设置校正屏的索引
	/// </summary>
	CP_SETCURRENTDISPLAYINDEX = 0X0D,
	/// <summary>
	/// 获得当前控制系统带载的显示屏个数
	/// </summary>
	CP_GETDISPLAYCOUNT = 0X0E,
	/// <summary>
	/// 获得指定序号显示屏的映射位置、大小和类型
	/// </summary>
	CP_GETDISPLAYINFO = 0X0F,
	/// <summary>
	/// 发送阉割版配置文件 已弃用
	/// </summary>
	CP_SENDFILE = 0X10,
	/// <summary>
	///获取文件
	/// </summary>
	CP_GETFILE = 0x11,
	/// <summary>
	/// 发送数据
	/// </summary>
	CP_SENDDATA = 0x12,
	/// <summary>
	/// 获取数据
	/// </summary>
	CP_GETDATA = 0x13,
	/// <summary>
	/// 设置LCT上传校正系数的模式
	/// </summary>
	CP_SETUPLOADCOEFSMODE = 0x14,
	/// <summary>
	/// 通知上传校正系数百分比变化
	/// </summary>
	CP_NOTIFYUPLOADCOEFSPERCENTCHANGED = 0X15,
	/// <summary>
	/// 获取控制系统版本号
	/// </summary>
	CP_GETCONTROLLERVERSION = 0x16,
	/// <summary>
	/// 设置自定义打屏图片
	/// </summary>
	CP_SETCUSTOMBITMAP = 0x17,
	/// <summary>
	/// 给定区域的所在的接收卡从Flash重新载入校正系数
	/// </summary>
	CP_RELOADCOEFFICIENT = 0x18,
	/// <summary>
	/// 设置校正模式
	/// </summary>
	CP_SETCORRECTMODE_PIXEL = 0x19,
	/// <summary>
	/// 获取显示屏某个矩形位置的平均校正系数
	/// </summary>
	CP_GETSCRRECT_AVERCOEFS = 0x1a,
	/// <summary>
	/// 回复某个矩形位置的平均校正数据
	/// </summary>
	CP_REPLAYSCRRECT_AVERCOEFS = 0x1b,
	/// <summary>
	/// 通知自定义图片显示完成
	/// </summary>
	CP_NOTIFYCUSTOMBITMAP_SETTED = 0x1c,
	/// <summary>
	/// 灯板flash的操作
	/// </summary>
	CP_MODULEFLASH_OPEARTION = 0x1d,
	/// <summary>
	/// 通知灯板flash的操作结果
	/// </summary>
	CP_NOTIFY_MODULEFLASH_RES = 0x1e,
	/// <summary>
	/// 指定区域的灯板Flash操作
	/// </summary>
	CP_MODULEFLASH_OPEARTION_RECT = 0x1f,
	/// <summary>
	/// 通知指定区域的灯板Flash的操作结果
	/// </summary>
	CP_MODULEFLASH_OPEARTION_RES_RECT = 0x20,

	CP_GET_FIRSTSCANNERID = 0x21,

	CP_SET_FIRSTSCANNERID = 0x22,
	/// <summary>
	/// 通知指定区域的灯板Flash的操作结果,具体到flash带载区域
	/// </summary>
	CP_MODULEFLASH_OPEARTION_RES_RECT_EX = 0x23,
	/// <summary>
	/// 固化接收卡参数和配置文件（异步老接口 不再使用）
	/// </summary>
	CP_SOLIDIFY = 0x24,
	/// <summary>
	/// 亮暗线调节
	/// </summary>
	CP_QUICKLYADJUSTLINES = 0x25,
	/// <summary>
	/// 设置校正屏的名称
	/// </summary>
	CP_SETCURRENTDISPLAYNAME = 0X26,
	/// <summary>
	/// 获得当前控制系统带载的显示屏个数:新屏
	/// </summary>
	CP_GETDISPLAYCOUNTNEW = 0X27,
	/// <summary>
	/// 获得指定名称的显示屏的映射位置、大小和类型
	/// </summary>
	CP_GETDISPLAYINFONEW = 0X28,
	/// <summary>
	/// 获得是否支持硬件打屏
	/// </summary>
	CP_GET_HWDISPLAYSCREEN = 0X29,
	/// <summary>
	/// 关闭或开启2053IP核
	/// </summary>
	CP_ENABEL2053IPCORE = 0X33,

	/// <summary>
	/// 获得当前屏体操作的模式
	/// </summary>
	CP_GET_CURRENTSCREENUSEMODE = 0X2A,
	/// <summary>
	/// 设置当前屏体操作的模式
	/// </summary>
	CP_SET_CURRENTSCREENUSEMODE = 0X2B,
	/// <summary>
	/// 设置当前屏体连接DVI的信息
	/// </summary>
	CP_GET_DVICONNECTRES = 0X2C,
	/// <summary>
	/// 设置硬件打屏
	/// </summary>
	CP_SET_HWDISPLAYSCREEN = 0X2D,
	/// <summary>
	/// 通知灯板flash写入系数结果
	/// </summary>
	CP_MODULEFLASH_WRITECOEF_RES = 0x2E,
	/// <summary>
	/// gamma发送
	/// </summary>
	CP_SET_GAMMA = 0x2F,
	/// <summary>
	/// 写入接收卡ID
	/// </summary>
	CP_SET_MRVID = 0x30,
	/// <summary>
	/// Flash id
	/// </summary>
	CP_PASSTHROUGH = 0x31,
	/// <summary>
	/// 新版本的拿到单屏的COM口
	/// </summary>
	CP_GETDISPLAY_COM = 0X32,

	/// <summary>
	/// 计算出来的延时时间
	/// </summary>
	CP_DELAYTIME = 0X34,

	//add by weikai 2022-03-30
	// <summary>
	/// 发送文件同步回复
	/// </summary>
	CP_SCANBDPROPRESULT = 0X36,

	//add by weikai 2021-05-31
	/// <summary>
	/// 低灰校正的系数
	/// </summary>
	CP_LOWGRAYWEITECOEF = 0X3B,

	/// <summary>
	/// 设置低灰校正模式
	/// </summary>
	CP_SETLOWLUMCORRECTMODE_PIXEL = 0x3c,

	/// <summary>
	/// 亮暗线功能
	/// </summary>
	CP_BRIGHT_DARK_LINE_FIX_COEF_UPLOAD = 0X42, //系数上传
	CP_CANCEL_BRIGHT_DARK_LINE_FIX_COEF_UPLOAD = 0X43,
	CP_SET_BRIGHT_DARK_LINE_FIX_STATE = 0X44,//亮暗线开关
	CP_SOLID_BRIGHT_DARK_LINE_FIX_STATE = 0X45,
	CP_SOLID_BRIGHT_DARK_LINE_FIX_COEF = 0X46,//固化系数

	CP_GETSCANNERFUNCTIONINFOS = 0x47,
	CP_POINTTOPOINT_DISPLAY_SCREEN = 0X48,
	/// <summary>
	/// 全灰阶系数上传
	/// </summary>
	CP_MULTILAYER_CORRECTION_COEF_UPLOAD = 0X49,
	CP_SET_MULTILAYER_CORRECTION_STATE = 0X4A,
	CP_SOLID_MULTILAYER_CORRECTION_STATE = 0X4B,
	/// <summary>
	/// 全灰阶系数固化
	/// </summary>
	CP_SOLID_MULTILAYER_CORRECTION_COEF = 0X4C,

	/// <summary>
	/// //效果开关
	/// </summary>
	CP_SET_EFFECT_SWITCH_STATE = 0X4D,
	CP_GET_GAMMA_TABLE = 0X4E,
	/// <summary>
	/// 获取模组拓扑信息
	/// </summary>
	CP_GET_MODULETOPOLOGY_INFO = 0X64,
	/// <summary>
	/// 获取模组拓扑新接口带模组基于屏幕的坐标
	/// </summary>
	CP_GETMODEL_TOPOLOGY = 0x9D,
	/// <summary>
	/// 获取全局亮度
	/// </summary>
	CP_GET_GLOBALBRIGHT = 0X66,
	/// <summary>
	/// 写入全局亮度
	/// </summary>
	CP_SET_GLOBALBRIGHT = 0X67,
	/// <summary>
	/// 逐点系数上传取消
	/// </summary>
	CP_CANCELCOMMONCORRECTIONCOEFUPLOAD = 0X68,
	/// <summary>
	/// 全灰阶系数上传取消
	/// </summary>
	CP_CANCELMULTILAYERBRIGHTCOEFUPLOAD = 0X69,
	/// <summary>
	/// 棋盘格打屏
	/// </summary>
	CP_SET_CHECKERBOARD_DISPLAY = 0X6A,
	/// <summary>
	/// 是否支持棋盘格打屏
	/// </summary>
	CP_GET_CHESSBOARDSCREEN_SUPPORT = 0X6B,
	/// <summary>
	/// 获取上传系数层数
	/// </summary>
	CP_GET_UPLOADLAYERS = 0X6D,
	/// <summary>
	/// 获取接收卡类型+版本
	/// </summary>
	CP_GET_SCANBOARDINFO = 0X72,

	//add by wyl
	/// <summary>
	/// 获取亚像素屏体信息
	/// </summary>
	CP_OBTAIN_SUBPIXELSCREEN_INFORMATION = 0X74,
	/// <summary>
	/// 亚像素逐点打屏
	/// </summary>
	CP_SUBPIXELPOINT_BYPOINT_DISPLAY = 0X75,
	/// <summary>
	/// 亚像素棋盘格打屏
	/// </summary>
	CP_SUBPIXELCHECKERBOARD_SCREEN = 0X76,
	/// <summary>
	/// 保存校正系数到灯板flash
	/// </summary>
	CP_COEFSOLIDTOFLASH = 0X82,
	/// <summary>
	/// 获取通讯口设备状态
	/// </summary>
	CP_DEVICESTATUS = 0X83,
	/// <summary>
	/// 9005/9937芯片低灰 自适应 校正系数上传
	/// </summary>
	CP_COEFUPLOAD = 0X7e,
	/// <summary>
	///校正系数开关
	/// </summary>
	CP_COEFSTATE = 0X7f,
	/// <summary>
	/// 校正系数固化（同步接口）
	/// </summary>
	CP_COEFSOLID = 0X80,
	/// <summary>
	/// 9005/9937芯片获取低灰校正模式 
	///0：模式一，普通的低灰校正；
	///1：模式二、9005 / 9937芯片低灰校正
	/// 是否支持灯板flash
	/// </summary>
	CP_GET_ISSUPPORT_MODULE_FLASH = 0x84,
	/// <summary>
	/// 硬件回形屏
	/// </summary>
	CP_HWDISPLAYSCREENRETURN = 0x85,
	/// <summary>
	/// 相机修缝系数上传
	/// </summary>
	CP_CAMERASEAMREPAIR = 0x87,
	/// <summary>
	/// 获取开关状态
	/// </summary>
	CP_GETSWITCHSTATE = 0x8d,
	/// <summary>
	/// 发送全量rcfgx文件到LCT端
	/// </summary>
	CP_SEND_CONPLETE_RCFGX = 0x8e,
	/// <summary>
	/// 检测NCP是否可用
	/// </summary>
	CP_ISVALID_NCP = 0x88,
	/// <summary>
	/// CC3导入NCP标准文件
	/// </summary>
	CP_SEND_CHECKFIlE_NCP = 0x89,
	/// <summary>
	/// NCP文件对比结果
	/// </summary>
	CP_CHECKNCP_IS_SAME = 0x8a,
	/// <summary>
	/// 是否做过画质引擎
	/// </summary>
	CP_IMAGEENGINE_ISDONE = 0x8f,
	/// <summary>
	/// 画质引擎开关状态
	/// </summary>
	CP_IMAGEENGINE_READ_SWITCH = 0x90,
	/// <summary>
	/// 读取箱体编号
	/// </summary>
	CP_READ_CABINETID = 0x92,
	/// <summary>
	/// 设置模组编号
	/// </summary>
	CP_MODULEIDSETINGS = 0x94,
	/// <summary>
	/// 设置低灰均匀性
	/// </summary>
	CP_LOWASHUNIFORMITY = 0x98,
	/// <summary>
	/// 设置低灰开关
	/// </summary>
	CP_LOWCORRECTIONSTATE = 0x99,
	/// <summary>
	/// 设置低灰模式
	/// </summary>
	CP_LOWCORRECTIONMODE = 0x9a,
	/// <summary>
	/// 设置指定灰阶gamma值
	/// </summary>
	CP_SETCURRENTGAMMAVALUE = 0x9b,
	/// <summary>
	/// 系数擦除
	/// </summary>
	CP_ERASECOFE_SPIFLASH = 0x9c,
	/// <summary>
	/// 设置白色阈值
	/// </summary>
	CP_SETTHRESHOLD_WHITE = 0x9e,
	/// <summary>
	/// 固化接收卡参数和配置文件
	/// </summary>
	CP_SOLIDSCANNER = 0xA0,
};

//组合码
enum class enum_LCTUnionCommon
{
	Union_Default = 0X00,
};

//操作码
enum class enum_LCTOperations
{
	OPeration_Default = 0X00,
};

//数据透传操作码
enum class enum_PassthroughDataType
{
	PassthroughData_Read = 0X00,
	PassthroughData_Send = 0X01
};

//图像显示时的显示方式
enum class enum_BmpShowModeType
{
	Stretch = 0,
	Center = 1,
	Tile = 2
};

enum class enum_UploadCoefModeType
{
	DVI = 0,
	SerialPort = 1
};

//校正模式
enum class enum_CorrectMode
{
	CloseAll = 0,               //关闭所有效果
	CorrectModeChroma = 1,      //色度
	CorrectModeLum = 2,         //亮度
	CorrectMultiLayer = 4,      //全灰阶
	GrayScale = 8,              //精细灰度
	ColorManager = 16,          //色彩管理
	SeamLum = 32,               //边缝
	MagicGray = 64,             //MG
	ABL = 128,                  //电流调节
	Bit22 = 256,                //22Bit+
	CorrectLowGray = 512        //低灰
};

//校正模式类型
enum class enum_CorrectType {
	Close_Correct = 0x00,      // 关闭所有校正效果 
	Bright_Correct = 0x01,     // 只开启亮度校正
	Chroma_Correct = 0x02,     // 只开启色度校正 
	Fine_Correct = 0x04,       // 只开启精细灰度
	Colour_Correct = 0x08,     // 只开启色彩管理效
	AllGray_Correct = 0x10,    // 只开多层校正
};

//全灰阶开关类型
enum class enum_MultiLayerSwitchType
{
	__Unknown = 0,
	Bright = 1,  //全灰阶亮度
	Chorma = 2,  //色度
};

//画质引擎
enum class enum_ImageEngineMode
{
	CloseEngineMode = 0,            //关闭所有效果
	GrayScale = 1,                  //精细灰度
	ColorManager = 2,               //色彩管理
	MagicGray = 4,                  //MG
	Bit22 = 8,                      //22Bit+
	ABL = 16,                       //电流调节
	SeamLum = 32                    //边缝
};

enum class enum_ModuleFlashOperation
{
	SaveToModuleFlash = 0,
	LoadFromModuleFlash = 1,
	SaveToModuleFlashFast = 2,
	SaveToModuleFlashFastEx = 3
};

// 亚像素打屏亮点 注:LCT最多只能识别24位
enum class enum_SubPixLightLoc {
	enNotSubPixel = 0,	   // 非亚像素屏（默认值）
	enAllLight = 0xffffff, // 全亮
	enLoc1 = 0x00000002,   // G1
	enLoc2 = 0x00000004	   // G2
};

//屏体模式
enum class enum_ScreenOperationMode
{
	OneScreen = 1,  //单屏
	CombScreen = 2, //组合屏
	UnKnow = 255
};

//打屏方式
enum class enum_DisplayPatternType
{
	/// <summary>
	/// 隔点打屏
	/// </summary>
	DotScreen = 1,
	/// <summary>
	/// 隔块打屏
	/// </summary>
	BlockScreen = 2,
	/// <summary>
	/// 棋盘格打屏
	/// </summary>
	ChessBoardScreen = 3,
	/// <summary>
	/// 亚像素隔点打屏
	/// </summary>
	SubpixelDotSpacingScreen = 4,
	/// <summary>
	/// 亚像素棋盘格打屏
	/// </summary>
	SubpixelChessboardScreen = 5,
	/// <summary>
	/// 关闭硬件打屏
	/// </summary>
	CloseScreen = 0
};

//屏体类型
enum class enum_DisplayScreenType
{
	simple = 0, //简单显示屏
	complex,    //复杂显示屏
	standard    //标准显示屏

};

//屏体校正类型
enum class enum_ScreenCorrectType
{
	COB = 1,                	//COB校正
	Normal = 2,             	//普通屏校正
	Custom = 4,              	//自定义
	MultiLayer = 8,         	//全灰阶校正
	LowGray = 16				//低灰校正
};

//写入灯板falsh反馈结果
enum class enum_ModuleFlashOperationRes
{
	ModuleFlashOperationResOK,
	ModuleFlashOperationResCheckSumErr,
	ModuleFlashOperationResComunicationErr,
	ModuleFlashOperationResHardwareErr,
	ModuleFlashOperationResDataError,
	ModuleFlashOperationResSettingTopologyErr,
	ModuleFlashOperationResNoCoefs
};

//是否连接DVI
enum class enum_DVIConnectRes
{
	DVIConnectResOK = 0,
	DVIConnectResPartialConnect = 1,
	DVIConnectResError = 2
};

//低灰模式
enum class enum_LowMode {
	modeOne = 0,
	modeTwo = 1,
	modeThree = 2
};

enum class enum_GammaSetMode {
	setMode_Operate = 0,	//操作 0
	setMode_Reset = 1		//复原 1
};

//低灰亮度模式
enum class enum_BrightnessMode {
	brightNone = 0x00,
	brightOne = 0x01,
	brightTwo = 0x01 << 1,
	brightThree = 0x01 << 2,
	brightFour = 0x01 << 3,
};

//灯点颜色
enum class enum_LedColorType
{
	LedColor_Red = 0,		//红色
	LedColor_Green,			//绿色
	LedColor_Blue,			//蓝色
	LedColor_White,			//白色
	LedColor_UnKnown		//未知
};
NLOHMANN_JSON_SERIALIZE_ENUM(enum_LedColorType, {
{ enum_LedColorType::LedColor_Red,   "LedColor_Red" },
{ enum_LedColorType::LedColor_Green, "LedColor_Green" },
{ enum_LedColorType::LedColor_Blue, "LedColor_Blue" },
{ enum_LedColorType::LedColor_White, "LedColor_White" },
{ enum_LedColorType::LedColor_UnKnown, "LedColor_UnKnown" }
    });

enum class enum_LCTScannerComErr
{
	ScannerComSuccess = 0,
	ScannerComNetError,
	ScannerComTimeOut,
	ScannerComParaError,
	cabinetNameNoUse,
	CheckGammaTypeError
};

enum class enum_DataBaseCoefType
{
	Coef_Normal = 0,            	// 常规3*3系数
	Coef_MultiGray,         		// 全灰阶系数
	Coef_LowGray,           		// 低灰系数
	Coef_LowGray_9005_9937,        	// 9005/9937芯片低灰阶校正系数
    Coef_Adaptive,         			// 自适应系数
	Coef_ALL_TYPE,					// 所有类型

};

enum class enum_SubPixLightNum {
	enThree = 1,
	enFour = 2,
	enSix = 3,
};

enum class enum_LightLayout {
	enUnknow = 0,
	enTriangleRowDiff = 1, 		// 三角排布相邻行不同 三灯模式1
	enTriangleColDiff = 2, 		// 三角排布相邻列不同 三灯模式2
	enLineRowDiff = 3,	   		// 线性排布相邻行不同 六灯模式1
	enLineColDiff = 4,	   		// 线性排布相邻列不同 六灯模式2
	enSquare = 5		   		// 正方形排布         四灯模式
};

enum class enum_LightPointLayout { //算法目前不需要

	unknow = 0,
	RGB = 1,
	RBG,
	GRB,
	GBR,
	BRG,
	BGR,
	//四灯使用
	RGGB = 7,
	BGGR,
	GRBG,
	GBRG
};

enum class enum_SubPixSup {

	enNormal = 0,	//常规屏
	enSubDevSup,	//亚像素屏且硬件支持
	enSubDevNotSup, //亚像素屏但是硬件不支持
};
// 打屏模式
enum class enum_DisplayMode
{
	VisualInterface = 0,                       // 视频源打屏
	SerialPortCmd = 1                           // 硬件打屏
};



//区域基本信息类
struct ST_LocationInfo
{
public:
	ST_LocationInfo() {};

	GETTER(std::string, m_areaID, AreaID);
	SETTER(std::string, m_areaID, AreaID);

	GETTER(int, m_begPixCol, BegPixCol);
	SETTER(int, m_begPixCol, BegPixCol);

	GETTER(int, m_begPixRow, BegPixRow);
	SETTER(int, m_begPixRow, BegPixRow);

	GETTER(int, m_endPixCol, EndPixCol);
	SETTER(int, m_endPixCol, EndPixCol);

	GETTER(int, m_endPixRow, EndPixRow);
	SETTER(int, m_endPixRow, EndPixRow);

	GETTER(std::string, m_cabinetName, CabinetName);
	SETTER(std::string, m_cabinetName, CabinetName);

	ark::utility::Size getAreaSize()
	{
		int col = m_endPixCol - m_begPixCol + 1;
		int row = m_endPixRow - m_begPixRow + 1;
		return ark::utility::Size(col, row);
	}
	int getAreaPixCount()
	{
		ark::utility::Size areasize = getAreaSize();
		return areasize.width * areasize.height;
	}
private:
	std::string m_areaID = "1";     //(全屏指分区编号，箱体指箱体编号）
	int m_begPixCol = 0;            //开始列
	int m_begPixRow = 0;            //开始行
	int m_endPixCol = 0;            //结束列
	int m_endPixRow = 0;            //结束行

	std::string m_cabinetName = ""; //箱体名称
};

//显示图像区域信息 以及颜色 以及模式
struct ST_DisplayLoactionInfo : public ST_LocationInfo
{
	unsigned int m_intensity;                            	//亮度
	enum_LedColorType m_color;                          	//颜色
	int m_pattern;                                      	//0代表全部显示
	ST_DisplayLoactionInfo(const ST_DisplayLoactionInfo& info) :ST_LocationInfo(info)
	{
		m_intensity = info.m_intensity;
		m_color = info.m_color;
		m_pattern = info.m_pattern;
	}

	ST_DisplayLoactionInfo& operator =(const ST_DisplayLoactionInfo& info)
	{
		if (this == &info)
			return *this;

		m_intensity = info.m_intensity;
		m_color = info.m_color;
		m_pattern = info.m_pattern;
		return *this;
	}

	ST_DisplayLoactionInfo()
	{
		m_intensity = 50;
		m_color = enum_LedColorType::LedColor_Green;
		m_pattern = 0;
	}
};

//LCT显示棋盘格参数信息
struct ST_DisplayCheckerBoard
{
	unsigned short m_flag;							// 奇偶性：1 : 第一个棋盘块为亮色（默认值）；0: 第一个棋盘块为暗色
	ark::utility::Rect m_screenArea;				// 选择区域
	ark::utility::Size m_checkerBoardSize;			// 棋盘格大小
	ark::utility::Size m_endWAndEndH;				// 最后一列宽与最后一行高
	ark::utility::Color m_rgbValue;

	ST_DisplayCheckerBoard(const ST_DisplayCheckerBoard& r)
	{
		m_flag = r.m_flag;
		m_screenArea = r.m_screenArea;
		m_checkerBoardSize = r.m_checkerBoardSize;
		m_endWAndEndH = r.m_endWAndEndH;
		m_rgbValue = r.m_rgbValue;
	}

	ST_DisplayCheckerBoard& operator =(const ST_DisplayCheckerBoard& r)
	{
		if (this == &r)
		{
			return *this;
		}

		m_flag = r.m_flag;
		m_screenArea = r.m_screenArea;
		m_checkerBoardSize = r.m_checkerBoardSize;
		m_endWAndEndH = r.m_endWAndEndH;
		m_rgbValue = r.m_rgbValue;

		return *this;
	}

	ST_DisplayCheckerBoard()
	{
		m_flag = 1; // 1: 第一个棋盘块为亮色（默认值）;
		m_screenArea = ark::utility::Rect();
		m_checkerBoardSize = ark::utility::Size(0, 0);
		m_endWAndEndH = ark::utility::Size(0, 0);
		m_rgbValue = ark::utility::Color(0, 0, 0);
	}

};

struct ST_DisplayUnitInfo
{
	//隔点信息
	int m_UintCol;
	int m_UintRow;
	int m_CurrentCol;
	int m_CurrentRow;

	ST_DisplayUnitInfo(const ST_DisplayUnitInfo& info)
	{
		m_UintCol = info.m_UintCol;
		m_UintRow = info.m_UintRow;
		m_CurrentCol = info.m_CurrentCol;
		m_CurrentRow = info.m_CurrentRow;
	}

	ST_DisplayUnitInfo& operator =(const ST_DisplayUnitInfo& info)
	{
		if (this == &info)
			return *this;

		m_UintCol = info.m_UintCol;
		m_UintRow = info.m_UintRow;
		m_CurrentCol = info.m_CurrentCol;
		m_CurrentRow = info.m_CurrentRow;

		return *this;
	}

	ST_DisplayUnitInfo()
	{
		m_UintCol = 0;
		m_UintRow = 0;
		m_CurrentCol = 0;
		m_CurrentRow = 0;
	}
};

//固化接收卡参数时的地址信息
struct ST_SolidifyAddressInfo
{
	CommResults m_replyPackageResult;
	int m_sendAddr = 0;
	int m_portAddr = 0;
	int m_scanAddr = 0;
	ST_SolidifyAddressInfo(const ST_SolidifyAddressInfo& info)
	{
		m_sendAddr = info.m_sendAddr;
		m_portAddr = info.m_portAddr;
		m_scanAddr = info.m_scanAddr;
		m_replyPackageResult = info.m_replyPackageResult;
	}
	ST_SolidifyAddressInfo& operator =(const ST_SolidifyAddressInfo& info)
	{
		if (this == &info)
			return *this;

		m_sendAddr = info.m_sendAddr;
		m_portAddr = info.m_portAddr;
		m_scanAddr = info.m_scanAddr;
		m_replyPackageResult = info.m_replyPackageResult;
		return *this;
	}

	ST_SolidifyAddressInfo()
	{
		m_sendAddr = 0;
		m_portAddr = 0;
		m_scanAddr = 0;
		m_replyPackageResult = CommResults::Unkonwn;
	}
};

//发送、获取文件命令以及信息
struct ST_OperFileInfo
{
	CommResults m_netFreamResults;
	// 使用第一二标识码，表征所作的操作 第一操作码“0”已经占用
	char m_group;               //第一标识码
	char m_cmd;                 //第二标识码
	std::string m_addInfo;		//其它附带信息 CC3用来做文件区分，发送时删除
	std::string m_filePath;		//需要文件路径
	int m_nFileType;            //发送文件为0  获取文件为1

	ST_OperFileInfo(const ST_OperFileInfo& info)
	{
		m_netFreamResults = info.m_netFreamResults;
		m_group = info.m_group;
		m_cmd = info.m_cmd;
		m_addInfo = info.m_addInfo;
		m_filePath = info.m_filePath;
		m_nFileType = info.m_nFileType;

	}
	ST_OperFileInfo& operator =(const ST_OperFileInfo& info)
	{
		if (this == &info)
			return *this;
		m_netFreamResults = info.m_netFreamResults;
		m_group = info.m_group;
		m_cmd = info.m_cmd;
		m_addInfo = info.m_addInfo;
		m_filePath = info.m_filePath;
		m_nFileType = info.m_nFileType;
		return *this;
	}

	ST_OperFileInfo()
	{
		m_netFreamResults = CommResults::OK;
		m_group = 0;
		m_cmd = 0;
		m_nFileType = 0;
	}
};

struct ST_ScannerID
{
	std::string m_userModeID;           //用户识别ID
	std::string m_scannerGUID;			//接收卡guid
	bool m_isValid;						//数据是否有效

	ST_ScannerID(const ST_ScannerID& info)
	{
		m_userModeID = info.m_userModeID;
		m_scannerGUID = info.m_scannerGUID;
		m_isValid = info.m_isValid;

	}
	ST_ScannerID& operator =(const ST_ScannerID& info)
	{
		if (this == &info)
			return *this;
		m_userModeID = info.m_userModeID;
		m_scannerGUID = info.m_scannerGUID;
		m_isValid = info.m_isValid;
		return *this;
	}

	ST_ScannerID()
	{
		m_isValid = false;
	}
};

struct ST_GammaInfo
{
	float m_GammaValue;
	char* m_GammaTable;

	ST_GammaInfo(const ST_GammaInfo& info)
	{
		m_GammaValue = info.m_GammaValue;
		m_GammaTable = info.m_GammaTable;
	}
	ST_GammaInfo& operator =(const ST_GammaInfo& info)
	{
		if (this == &info)
			return *this;
		m_GammaValue = info.m_GammaValue;
		m_GammaTable = info.m_GammaTable;
		return *this;
	}

	ST_GammaInfo()
	{
		m_GammaValue = 10;
		m_GammaTable = nullptr;
	}
};

struct ST_MrvInfo
{
public:
	std::string m_MrvID;		//id信息
	bool m_IsValid = true;		//是否启用
	std::string m_Guid;
	//     X位置(在其所在Port带载区域的X坐标)
	short m_XInPort;
	//     Y位置(在其所在Port带载区域的Y坐标)
	short m_YInPort;
	//     位于所在输出口的第几个
	short m_ConnectIndex;
	//     带载像素高度
	short m_Height;
	//     所属输出口序号，若为0xffff，表示留空
	short m_PortIndex;
	//     所属发送卡序号
	short m_SenderIndex;
	//     带载像素宽度
	short m_Width;
	//     X位置(在其所在显卡的X坐标)
	short m_X;
	//     Y位置(在其所在显卡的Y坐标)
	short m_Y;
};

//直传通道接口参数
struct ST_PassthroughPack
{
public:
	enum_RequestType m_RequestType;
	std::string m_ComPort;

	short m_SenderIndex;
	short m_PortAddr;
	uint16_t m_ScanAddr;
	uint32_t m_RegisterAddr;

	std::string m_Data;
	int m_DataLength;
	std::string m_PackSN;
	int m_TimeOut;
	enum_PassthroughRequestResult m_PassthroughRequestResult;
	//在C#中此变量为Object类型，因暂未发现其作用，序列化是默认为null
	void* m_UserToken;

	ST_PassthroughPack()
	{
		m_RequestType = enum_RequestType::Read;
		m_UserToken = nullptr;
		m_PassthroughRequestResult = enum_PassthroughRequestResult::NoRespond;
	}

	ST_PassthroughPack(const ST_PassthroughPack& info)
	{
		m_RequestType = info.m_RequestType;
		m_ComPort = info.m_ComPort;
		m_SenderIndex = info.m_SenderIndex;
		m_PortAddr = info.m_PortAddr;
		m_ScanAddr = info.m_ScanAddr;
		m_Data = info.m_Data;
		m_DataLength = info.m_DataLength;
		m_PackSN = info.m_PackSN;

		m_RegisterAddr = info.m_RegisterAddr;
		m_TimeOut = info.m_TimeOut;
		m_PassthroughRequestResult = info.m_PassthroughRequestResult;
		m_UserToken = info.m_UserToken;
	}
	ST_PassthroughPack& operator =(const ST_PassthroughPack& info)
	{
		if (this == &info)
			return *this;
		m_RequestType = info.m_RequestType;
		m_ComPort = info.m_ComPort;
		m_SenderIndex = info.m_SenderIndex;
		m_PortAddr = info.m_PortAddr;
		m_ScanAddr = info.m_ScanAddr;
		m_Data = info.m_Data;
		m_DataLength = info.m_DataLength;
		m_PackSN = info.m_PackSN;

		m_RegisterAddr = info.m_RegisterAddr;
		m_TimeOut = info.m_TimeOut;
		m_PassthroughRequestResult = info.m_PassthroughRequestResult;
		m_UserToken = info.m_UserToken;
		return *this;
	}

	~ST_PassthroughPack()
	{
	}
};

//硬件打屏参数
struct ST_HWDisplayParameterBase
{
public:
	ark::utility::Rect m_Rect;					 	///<打屏区域
	enum_DisplayPatternType m_PatternType;          ///<打屏模式
	unsigned int m_BrightValue;                     ///<打屏亮度
	unsigned int m_RedBrightValue;
	unsigned int m_GreenBrightValue;
	unsigned int m_BlueBrightValue;

	ST_HWDisplayParameterBase()
	{
		m_BrightValue = 100;
		m_RedBrightValue = 0;
		m_GreenBrightValue = 0;
		m_BlueBrightValue = 0;
	}

	ST_HWDisplayParameterBase(const ST_HWDisplayParameterBase& info)
	{
		m_Rect = info.m_Rect;
		m_PatternType = info.m_PatternType;
		m_BrightValue = info.m_BrightValue;
		m_RedBrightValue = info.m_RedBrightValue;
		m_GreenBrightValue = info.m_GreenBrightValue;
		m_BlueBrightValue = info.m_BlueBrightValue;
	}
	ST_HWDisplayParameterBase& operator =(const ST_HWDisplayParameterBase& info)
	{
		if (this == &info)
			return *this;
		m_Rect = info.m_Rect;
		m_PatternType = info.m_PatternType;
		m_BrightValue = info.m_BrightValue;
		m_RedBrightValue = info.m_RedBrightValue;
		m_GreenBrightValue = info.m_GreenBrightValue;
		m_BlueBrightValue = info.m_BlueBrightValue;
		return *this;
	}
};

//关闭硬件打屏参数
struct ST_HWDisplayParameterClose : public ST_HWDisplayParameterBase
{
public:
	ST_HWDisplayParameterClose() :ST_HWDisplayParameterBase()
	{
		m_PatternType = enum_DisplayPatternType::CloseScreen;
	}
};

//隔点打屏参数
struct ST_HWDisplayParameterPattern : public ST_HWDisplayParameterBase
{
public:
	int m_IntervalPixelsX;			//大分区打屏水平隔点像素
	int m_IntervalPixelsY;			//大分区打屏垂直隔点像素
	ark::utility::Point m_Position;	//大分区打屏亮点位置

	ST_HWDisplayParameterPattern() :ST_HWDisplayParameterBase()
	{
		m_PatternType = enum_DisplayPatternType::DotScreen;
	}

	ST_HWDisplayParameterPattern(const ST_HWDisplayParameterPattern& info) :ST_HWDisplayParameterBase(info)
	{
		m_IntervalPixelsX = info.m_IntervalPixelsX;
		m_IntervalPixelsY = info.m_IntervalPixelsY;
		m_Position = info.m_Position;
	}
	ST_HWDisplayParameterPattern& operator =(const ST_HWDisplayParameterPattern& info)
	{
		if (this == &info)
			return *this;
		m_IntervalPixelsX = info.m_IntervalPixelsX;
		m_IntervalPixelsY = info.m_IntervalPixelsY;
		m_Position = info.m_Position;
		return *this;
	}
};

//隔块打屏参数
struct ST_HWDisplayParameterBlock : public ST_HWDisplayParameterBase
{
public:
	int m_IntervalPixelsX;					//大分区打屏水平隔块
	int m_IntervalPixelsY;					//大分区打屏垂直隔块
	ark::utility::Point m_Position;            //大分区打屏亮点位置
	ark::utility::Size m_BlackSize;            //块的宽高

	ST_HWDisplayParameterBlock()
	{
		m_IntervalPixelsX = 1;
		m_IntervalPixelsY = 1;
		m_PatternType = enum_DisplayPatternType::BlockScreen;
	}

	ST_HWDisplayParameterBlock(const ST_HWDisplayParameterBlock& info) :ST_HWDisplayParameterBase(info)
	{
		m_IntervalPixelsX = info.m_IntervalPixelsX;
		m_IntervalPixelsY = info.m_IntervalPixelsY;
		m_Position = info.m_Position;
		m_BlackSize = info.m_BlackSize;
	}
	ST_HWDisplayParameterBlock& operator =(const ST_HWDisplayParameterBlock& info)
	{
		if (this == &info)
			return *this;
		m_IntervalPixelsX = info.m_IntervalPixelsX;
		m_IntervalPixelsY = info.m_IntervalPixelsY;
		m_Position = info.m_Position;
		m_BlackSize = info.m_BlackSize;
		return *this;
	}

};

struct ST_BaseOper
{
	CommResults m_replyPackageResult;
	ST_BaseOper()
	{

	}
};

//获取指定序号屏信息
struct ST_DisplayScreenInfo : public ST_BaseOper
{
public:
	int m_Index;									///<该屏对应的序号
	std::string m_ScreenID;
	std::string m_ScreenName;                       ///<屏体名称
	std::string m_CommPort;                         ///<该屏对应的端口号
	ark::utility::Rect m_ScreenGeometry;				///<屏体几何信息
	ark::utility::Size m_ScanSize;						///<接收卡宽高
	ark::utility::Size m_MoudelSize;					///<灯板宽高
	enum_DisplayScreenType m_ScreenType;			///<屏的类型
	std::vector<ST_MrvInfo> m_MrvInfoList;			///<接收卡配置信息
	std::map<enum_LedColorType, std::vector<unsigned int>>   m_gammaList;   //gamma表

	ST_DisplayScreenInfo(CommResults replyPackageResult) : ST_BaseOper()
	{
		m_replyPackageResult = replyPackageResult;
	}
	ST_DisplayScreenInfo() {}
};

struct ST_DeviceStatusInfo
{
	std::string portName = "";
	int status = 1;
};

struct ST_MrvConfig
{
	int m_number;
	int m_sendeCard;
	int m_EthernetInterface;
	int m_mrvCard;
	std::string m_mrvConfigFileName;
};

//平均校正系数
struct ST_NotifyRectAverCoefs
{
	CommResults m_NetFreamResults;     				//获取平均校正系数结果
	std::vector<uint16_t> m_AvgCoef;              	//平均校正系数

	ST_NotifyRectAverCoefs& operator =(const ST_NotifyRectAverCoefs& info)
	{
		if (this == &info)
			return *this;
		m_NetFreamResults = info.m_NetFreamResults;
		m_AvgCoef = info.m_AvgCoef;
		return *this;
	}
};

struct ST_ErrorModuleFlashInfo
{
	enum_ModuleFlashOperationRes m_ModuleFlashOperationRes;
	ark::utility::Rect m_Rect;
	int m_PortIndex;
	int m_ScannerIndex;
	int m_SenderIndex;
};

struct ST_ModuleFlashNotifyParamsEx
{
	bool m_isAllOK = false;
	std::vector<ST_ErrorModuleFlashInfo> m_AllErrorInfo;

};

struct ST_EnableCalibration
{
	bool m_enableCalibration;
	int m_result;
	ST_EnableCalibration(bool enableCalibration, int result)
	{
		m_enableCalibration = enableCalibration;
		m_result = result;
	}
};

//单屏信息
struct ST_DisplayOneScreenInfo
{
	int ScreenX;                    //在组合屏中的X坐标
	int ScreenY;                    //在组合屏中的Y坐标
	ST_DisplayScreenInfo m_DisplayScreenInfo;       //原单屏的信息
public:
	ST_DisplayOneScreenInfo() {}
	ST_DisplayOneScreenInfo(int screenX, int screenY, ST_DisplayScreenInfo displayScreenInfo)
	{
		ScreenX = screenX;
		ScreenY = screenY;
		m_DisplayScreenInfo = displayScreenInfo;
	}
};

//组合屏信息
struct ST_DisplayCombScreenInfo
{
public:
	int m_index;						//该屏对应的序号
	std::string m_screenName;			//该屏对应的名称
	std::string m_screenSN;				//该屏对应的SN
	ark::utility::Rect m_screenGeometry;   //屏体尺寸;
	std::vector<ST_DisplayOneScreenInfo> m_displayScreenInfos;    //单屏信息

	ST_DisplayCombScreenInfo()
	{

	}
	~ST_DisplayCombScreenInfo()
	{
		if (m_displayScreenInfos.size() > 0)
		{
			m_displayScreenInfos.clear();
		}
	}
};

//屏体模式
struct ST_ScreenOperationMode
{
	enum_ScreenOperationMode screenOperationMode = enum_ScreenOperationMode::UnKnow;
	enum_DisplayMode displayMode = enum_DisplayMode::VisualInterface;
};

//屏体列表
struct ST_DisplayScreenList
{
public:
	std::string m_curScreenName = "0";
	int m_curIndex = 0;

	// 屏体设备信息
	std::vector<std::string> m_combScreenList; // 组合屏列表
	int m_singleScreenCount; // 单屏个数

	ST_DisplayScreenList()
	{

	}
	~ST_DisplayScreenList()
	{
		if (m_combScreenList.size() > 0)
		{
			m_combScreenList.clear();
		}
	}
};

struct ST_WriteCoefficients
{
	CommResults m_results;
	std::string m_FilePath;
};

struct ST_FlashTopologyMessage
{
public:
	int     m_flashSerialNumOnThisBus;
	int     m_groupNumInModule;
	int     m_pixelColsOfThisFlash;
	int     m_pixelRowsOfThisFlash;
	int     m_serialNumOfThisBus;
	int     m_startXOfThisFlash;
	int     m_startYOfThisFlash;
	bool    m_bThisFlashExist;
	int     m_totalFlashCascadedOnThisBus;
};

struct ST_FlashPublicTopology
{
public:
	int m_flashCols;
	int m_flashRows;
	int m_validH;
	int m_validW;
};

struct ST_FlashTopology
{
public:
	ST_FlashTopology() {}
	ST_FlashPublicTopology					m_flashPubTopology;
	std::vector<ST_FlashTopologyMessage>	m_flashMessageList;
};

//模组信息
struct ST_ModuleTopologyInfo
{
	std::string moduleID;   // 模组的flag
	int modulex;            // 模组的x
	int moduley;            // 模组的y
	int modulew;            // 模组的w
	int moduleh;            // 模组的h
};

struct ST_SubPixInfo
{
	enum_SubPixSup m_isSubPix = enum_SubPixSup::enNormal;
	enum_SubPixLightNum m_subPixLTNum = enum_SubPixLightNum::enThree;
	enum_LightLayout m_LTLayout = enum_LightLayout::enUnknow;
	enum_LightPointLayout m_LPLayout = enum_LightPointLayout::RGGB;
};

// 亚像素逐点打屏
struct ST_DispLocInfoExtend : public ST_DisplayLoactionInfo
{
	ST_DispLocInfoExtend(const ST_DispLocInfoExtend& info) :
		ST_DisplayLoactionInfo(info),
		m_dispLightLocFlag(info.m_dispLightLocFlag)
	{
	}
	ST_DispLocInfoExtend& operator=(const ST_DispLocInfoExtend& info)
	{
		if (this == &info) {
			return *this;
		}
		ST_DisplayLoactionInfo::operator=(info);
		m_dispLightLocFlag = info.m_dispLightLocFlag;
		return *this;
	}

	// 新增要亮灯点位置标记 : 默认全亮,可用“位或|”方式指定要亮多个灯的位置
	long m_dispLightLocFlag = static_cast<long>(enum_SubPixLightLoc::enAllLight);
	ST_SubPixInfo m_SubPixInfo;

	ST_DispLocInfoExtend()
	{
	}

	ST_DispLocInfoExtend(const ST_DisplayLoactionInfo& info) :
		ST_DisplayLoactionInfo(info)
	{
	}
};

//亚像素硬件打屏
//隔点打屏参数
struct ST_HWDisplayParameterPatternExtend : public ST_HWDisplayParameterPattern
{
	ST_HWDisplayParameterPatternExtend(const ST_HWDisplayParameterPatternExtend& info) :
		ST_HWDisplayParameterPattern(info),
		m_dispLightLocFlag(info.m_dispLightLocFlag)
	{
	}
	ST_HWDisplayParameterPatternExtend& operator=(const ST_HWDisplayParameterPatternExtend& info)
	{
		if (this == &info) {
			return *this;
		}
		ST_HWDisplayParameterPattern::operator=(info);
		m_dispLightLocFlag = info.m_dispLightLocFlag;
		return *this;
	}


	// 新增要亮灯点位置标记 : 默认全亮,可用“位或|”方式指定要亮多个灯的位置
	long m_dispLightLocFlag = static_cast<long>(enum_SubPixLightLoc::enAllLight);
public:
	ST_SubPixInfo m_SubPixInfo;

	ST_HWDisplayParameterPatternExtend()
	{
	}

	ST_HWDisplayParameterPatternExtend(const ST_HWDisplayParameterPattern& info) :
		ST_HWDisplayParameterPattern(info)
	{
	}
};
//隔块打屏参数
struct ST_HWDisplayParameterBlockExtend : public ST_HWDisplayParameterBlock
{
   ST_HWDisplayParameterBlockExtend(const ST_HWDisplayParameterBlockExtend& info) :
	   ST_HWDisplayParameterBlock(info),
       m_dispLightLocFlag(info.m_dispLightLocFlag) {}
   ST_HWDisplayParameterBlockExtend& operator=(const ST_HWDisplayParameterBlockExtend& info) {
       if (this == &info)
       {
           return *this;
       }
       ST_HWDisplayParameterBlock::operator=(info);
       m_dispLightLocFlag = info.m_dispLightLocFlag;
       return *this;
   }

   // 新增要亮灯点位置标记 : 默认全亮,可用“位或|”方式指定要亮多个灯的位置
   long m_dispLightLocFlag = static_cast<long>(
       enum_SubPixLightLoc::enAllLight);
   ST_SubPixInfo m_SubPixInfo;

   ST_HWDisplayParameterBlockExtend() {

   }

   ST_HWDisplayParameterBlockExtend(const ST_HWDisplayParameterBlock& info) :
	   ST_HWDisplayParameterBlock(info) {
   }
};

// 亚像素棋盘格
struct ST_DisplayCheckerBoardExtend : public ST_DisplayCheckerBoard
{
	ST_DisplayCheckerBoardExtend(const ST_DisplayCheckerBoardExtend& info) :
		ST_DisplayCheckerBoard(info),
		m_dispLightLocFlag(info.m_dispLightLocFlag)
	{
	}
	ST_DisplayCheckerBoardExtend& operator=(const ST_DisplayCheckerBoardExtend& info)
	{
		if (this == &info) {
			return *this;
		}
		ST_DisplayCheckerBoard::operator=(info);
		m_dispLightLocFlag = info.m_dispLightLocFlag;
		return *this;
	}
	// 新增要亮灯点位置标记 : 默认全亮,可用“位或|”方式指定要亮多个灯的位置
	long m_dispLightLocFlag = static_cast<long>(enum_SubPixLightLoc::enAllLight);
	ST_SubPixInfo m_SubPixInfo;

	ST_DisplayCheckerBoardExtend()
	{
	}

	ST_DisplayCheckerBoardExtend(const ST_DisplayCheckerBoard& info) :
		ST_DisplayCheckerBoard(info)
	{
	}
};

//配置信息
class ST_DeviceConfigInfo
{
public:
	void setMrvInfo(std::vector<ST_MrvInfo>& mrvInfo) { m_mrvInfoList = mrvInfo; }
	std::vector<ST_MrvInfo>& getMrvInfoList() { return m_mrvInfoList; }

	void setComPort(std::string& comPort) { m_comPort = comPort; }
	std::string& getComPort() { return m_comPort; }

	void setIdList(std::vector<std::string>& idList) { m_idList = idList; }
	std::vector<std::string>& getIdList() { return m_idList; }

	void setCabinetID(std::string& cabinetID) { m_cabinetID = cabinetID; }
	std::string& getCabinetID() { return m_cabinetID; }

	void setScreenInfo(ST_DisplayScreenInfo& screenInfo)
	{
		m_displayScreenInfo.m_CommPort = screenInfo.m_CommPort;
		m_displayScreenInfo.m_ScreenGeometry = screenInfo.m_ScreenGeometry;
		m_displayScreenInfo.m_MrvInfoList = screenInfo.m_MrvInfoList;
	}
	ST_DisplayScreenInfo& getScreenInfo() { return m_displayScreenInfo; }

	void setFlashTopology(ST_FlashTopology& flashTopoInfo) { m_flashTopoInfo = flashTopoInfo; }
	ST_FlashTopology& getFlashTopology() { return m_flashTopoInfo; }

protected:
	ST_DisplayScreenInfo		m_displayScreenInfo; // 屏体信息
	std::string					m_pbScreenInfo;      // 屏体信息protobuf
	std::string					m_comPort;           // 获取当前屏体的串口号
	std::vector<ST_MrvInfo>		m_mrvInfoList;       // 发送卡配置信息
	std::vector<std::string>	m_idList;            // 灯板ID列表
	std::string					m_cabinetID;         // 箱体ID
	ST_FlashTopology			m_flashTopoInfo;     // 灯板flash拓扑信息
};

//回字图
struct ST_DisplayKBImage
{
	ark::utility::Color m_color;                 	//外接矩形打屏颜色与亮度信息
	ark::utility::Point m_point;                 	//外接矩形打屏起始位置x、y坐标
	ark::utility::Size m_size;                   	//外接矩形打屏区域的大小

	ark::utility::Color m_innercolor;            	//内接矩形打屏颜色与亮度信息
	ark::utility::Point m_innerpoint;            	//内接矩形打屏起始位置x、y坐标
	ark::utility::Size m_innersize;              	//内接矩形打屏区域的大小
	uint64_t m_brightnessRatio;						//设置亮度衰减百分比

	ST_DisplayKBImage(const ST_DisplayKBImage& r)
	{
		m_color = r.m_color;
		m_point = r.m_point;
		m_size = r.m_size;
		m_innercolor = r.m_innercolor;
		m_innerpoint = r.m_innerpoint;
		m_innersize = r.m_innersize;
		m_brightnessRatio = r.m_brightnessRatio;
	}

	ST_DisplayKBImage& operator =(const ST_DisplayKBImage& r)
	{
		if (this == &r)
		{
			return *this;
		}

		m_color = r.m_color;
		m_point = r.m_point;
		m_size = r.m_size;
		m_innercolor = r.m_innercolor;
		m_innerpoint = r.m_innerpoint;
		m_innersize = r.m_innersize;
		m_brightnessRatio = r.m_brightnessRatio;
		return *this;
	}

	ST_DisplayKBImage()
	{
		m_color = ark::utility::Color(0, 0, 0);
		m_point = ark::utility::Point(0, 0);
		m_size = ark::utility::Size(0, 0);
		m_innercolor = ark::utility::Color(0, 0, 0);
		m_innerpoint = ark::utility::Point(0, 0);
		m_innersize = ark::utility::Size(0, 0);
		m_brightnessRatio = 100;
	}

};


// 切换校正开关参数
struct ST_ToggleCorrectionSwitch
{
	bool isEnableCorrect;											// 开关
	enum_CorrectMode correctMode = enum_CorrectMode::CloseAll;   	// 校正模式
	bool isChromaCorrentionLowGray = false;         				// 是否低灰
};

// 上传系数参数
struct ST_UploadCoefficientsParam
{
	enum_DataBaseCoefType coefType = enum_DataBaseCoefType::Coef_ALL_TYPE;
	ST_LocationInfo locaitonInfo;                 			// 屏区域

	std::vector<uint16_t> coefArray;                		// 系数
	std::vector<std::vector<uint16_t>> multiCoefArray;    	// 全灰阶系数
    std::vector<std::vector<uint16_t>> adaptCoefArray;     	// 自适应系数	

    std::vector<unsigned int> grayLayerList;                // 全灰阶Lsit
    std::vector<unsigned int> adaptGrayLayerList;           // 自适应灰阶List

    std::vector<unsigned char> multiLayerZipCoefs;          // 压缩全灰阶系数
    std::vector<unsigned char> adaptLayerZipCoefs;          // 压缩自适应系数
};

class CommandTimeOut
{
public:
	static const int SENDER_SAVEPARAMS_TIMEOUT = 8000;
	static const int SENDER_SIMPLYCOMMAND_TIMEOUT = 1000;
	static const int SENDER_SIMPLYFILE_TIMEOUT = 10000;
	static const int SENDER_SAVEMORE_TIMEOUT = 50000;
	static const int SENDER_GETGAMMA_TIMEOUT = 10000;
	static const int SENDER_UPLOADCOEFF_TIMEOUT = 900000;//系数上传最短等待时间 15min
};

//消息回复封装
class CorrectionReplyPack
{
public:
	std::string sData;
	enum_LCTCommands requestCMD;
	CommResults resultComm;

	void* parseData = nullptr;
};


using ReadLCTSendPackageFuncCB = std::function<void(std::shared_ptr<CorrectionReplyPack> pack)>;





}

}

#endif
