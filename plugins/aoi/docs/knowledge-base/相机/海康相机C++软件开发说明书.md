首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

### 首页

### 概述

工业相机SDK提供一套通用的编程接口，兼容GigE Vision协议、USB3 Vision协议、Camera Link

协议、CoaXPress协议，XoFLink协议，屏蔽设备接口的差异，可以方便地实现对多种不同的采

集卡和工业相机的管理。

工业相机SDK的基本功能包括：参数配置、图像采集、图像处理、事件功能。

- 参数设置：支持设备所有参数（如：宽、高、曝光时间等）的获取和设置。

- 图像获取：SDK接收工业相机发送的图像，并提供主动查询、回调两种方式供上层软件使

用。

- 图像处理：支持图像渲染、格式转换、存图、录像等。

- 事件功能：支持设备事件，异常状态上报。

### 开始使用

发版说明 介绍SDK各版本的更新摘要。

编程引导 介绍C接口取图流程和取图方式，以及采集卡接口调用流程。

API参考 介绍所有C接口和状态码，可与示例程序、编程引导配合使用。

结构体定义 介绍所有接口函数内结构体的定义。

相机参数节点表 包含相机的常用节点，相机实际支持的参数以客户端显示为准。

示例程序 介绍常见用例和接口的基本使用方法。

常见问题 介绍SDK问题的排查思路和其他常见问题。

### 运行环境

Windows 7 (32/64)、Windows 10 (32/64)、Windows 11、Windows Server 2008 R2及以上

### 使用说明

客户端默认安装在 C:/Program Files (x86)/（软件名称）路径下，用户也可指定路径进行安装。

安装完成后，在客户端文件夹下包含有Development文件夹，其中包括进行SDK二次开发的依赖

文件。目录结构如下：

- Bin（编译后的成果物）

- Documentations（二次开发文档资料，包括SDK开发指南和Demo使用说明等）

- DotNet（.NET开发的部分动态库）

- Includes（头文件）

- Libraries（静态库）

- Samples（各种编程语言的示例代码）

- ThirdPartyPlatformAdapter（第三方平台插件）

注解 不同版本的客户端开发目录文件可能存在差异，请以实际内容为准。

工业相机Windows版本SDK安装包包含了SDK接口说明和示例部分及相机驱动两个部分：相机驱

动程序在安装工业相机客户端过程中可选择是否安装。在C:/Program Files (x86)/Common

Files/ (软件名称）路径中，存在三个文件夹，分别是Drivers（驱动）、Runtime（包含了32位/64

位的动态链接库DLL）、Service（相机日志服务）。SDK接口是提供给用户的函数接口，以动态

链接库DLL方式提供，存放默认路径为：C:/Program Files (x86)/Common Files/（软件名称）

/Runtime，区分Win32_i86和Win64_x64版本，分别支持32位和64位程序开发。在客户端安装完

成后，已默认将该路径写入环境变量Path内，用户不需要额外拷贝。

### 注意事项

- 本手册中提供的代码段仅作为示例代码提供。不正确的代码可能会导致您的相机功能

与预期不同，并可能危及您的应用程序。为确保代码段在您的应用程序中正常运行，

您必须对其进行调整以满足您的特定需求，并且在使用前对其进行彻底测试；

- 不是所有的接口所有的相机都支持，以实际相机支持的功能为准。

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

### 发版说明

### V4.4.1

【功能新增】

- 新增获取Enum属性值扩展接口 MV_CC_GetEnumValueEx() 和对应的枚举类型值结构

体 MVCC_ENUMVALUE_EX 。该接口最大支持到256个节点。

- 新增保存图像到文件扩展接口 MV_CC_SaveImageToFileEx2() 和对应的保存图片所需

参数结构体 MV_CC_SAVE_IMAGE_PARAM 。该接口支持以PNG或TIFF格式保存4GB

以上的超大图。

- 新增通过采集卡句柄枚举相机接口 MV_CC_EnumDevicesByInterface() 。

- 新增开启设备指定事件接口 MV_CC_EventNotificationOn() 和关闭设备指定事件接口

MV_CC_EventNotificationOff() 。

- 新增Windows控制台demo，包含MultiLightCtrl_ImageStitching.cpp 、

Events_Interface.cpp 、ImageSave.cpp 和

ParametrizeCamera_LineScanIOSettings.cpp 。

- 支持4GB以上大小的图像取流和渲染。

o 新增显示一帧图像扩展接口 MV_CC_DisplayOneFrameEx2() 和对应的图像信息

结构体 MV_CC_IMAGE 。该接口支持4GB以上超大图渲染。

o 支持通过OpenGL方式渲染4GB以上超大图，您可通过

MV_DISPLAY_FRAME_INFO_EX() 显示帧信息结构体中enRenderMode字段实

现该功能。

o 更新 MV_FRAME_OUT_INFO_EX() 输出帧的信息结构体，添加nFrameLenEx

字段，支持输出4GB以上图像帧的长度。

【功能更新】

- 更新部分Windows控制台demo，包含GrabImage_Display.cpp 、

InterfaceAndDeviceDemo.cpp 等。

- 新增部分错误码 。

o 0x80000015：表示解码错误。

o 0x80000016：表示图像大小超出unsigned int数据类型上限值。

o 0x8000020E：表示在固定IP模式下不支持修改设备IP模式。

【功能废弃】

- 废弃相机初始化相关接口： MV_CC_EnumerateTls() 和

MV_CC_CreateHandleWithoutLog() 。

- 废弃图像采集相关接口： MV_CC_RegisterImageCallBackForRGB() 、

MV_CC_RegisterImageCallBackForBGR() 、 MV_CC_GetImageForRGB() 和

MV_CC_GetImageForBGR() 。

- 废弃显示相关接口和结构体： MV_CC_DisplayOneFrame() 和

MV_DISPLAY_FRAME_INFO 。建议您使用 MV_CC_DisplayOneFrameEx() 接口或

MV_CC_DisplayOneFrameEx2() 接口替代。

- 废弃相机属性相关接口： MV_CC_OpenParamsGUI() 。

- 废弃3D点云图像相关接口、结构体、枚举类型及控制台demo：

MV_CC_SavePointCloudData() 、 MV_SAVE_POINT_CLOUD_PARAM 、

MV_SAVE_POINT_CLOUD_FILE_TYPE() 和SavePointCloudData_3D.cpp。

### V4.4.0

【功能新增】

- 新增导入设备属性扩展接口 MV_CC_FeatureLoadEx() 。

- 新增节点错误信息结构体 MVCC_NODE_ERROR 、节点错误信息列表结构体

MVCC_NODE_ERROR_LIST 。

- 支持采集卡快速软触发功能，并新增示例程序：QuickSoftwareTrigger.cpp 。

【功能更新】

- 更新部分Windows控制台demo，包含GrabImage_Display.cpp 、MultiCast.cpp 等。

- 新增部分错误码 。

【文档调整】

- 合并错误码 至接口说明中介绍。

### V4.3.0

【功能新增】

- 新增初始化SDK接口 MV_CC_Initialize() 、反初始化SDK接口 MV_CC_Finalize() 。

- 新增枚举采集卡接口 MV_CC_EnumInterfaces() 、创建采集卡句柄接口

MV_CC_CreateInterface() 和 MV_CC_CreateInterfaceByID() 。

- 新增打开采集卡接口 MV_CC_OpenInterface() 、关闭采集卡接口

MV_CC_CloseInterface() 、销毁采集卡句柄接口 MV_CC_DestroyInterface() 。

- 新增获取串口信息列表接口 MV_CAML_GetSerialPortList() 、设置指定串口列表接口

MV_CAML_SetEnumSerialPorts() 。

- 相机初始化 和 图像采集 接口支持对双USB3.0接口工业相机进行枚举、打开、取流、获

取图像、关闭等操作。

- 图像渲染 接口支持对大图（宽高上限为16384*163840）进行渲染。

- 图像处理 中设置插值算法类型接口 MV_CC_SetBayerCvtQuality() 新增“最优+”插值方法

。

- 图像处理 和属性配置 接口支持采集卡句柄。

- 设备升级 接口支持采集卡固件升级。

- 事件及异常 接口支持采集卡事件。

- 新增采集卡信息结构体 MV_INTERFACE_INFO 、采集卡信息列表结构体

MV_INTERFACE_INFO_LIST 。

- 新增串口信息结构体 MV_CAML_SERIAL_PORT 、串口信息列表结构体

MV_CAML_SERIAL_PORT_LIST 。

- 新增CoaXPress相机信息结构体 MV_CXP_DEVICE_INFO 、CameraLink相机信息结构

体 MV_CML_DEVICE_INFO 、XoFLink相机信息结构体 MV_XOF_DEVICE_INFO 。

- 新增Windows控制台demo：ForceIPEx.cpp 、Grab_Asynchronous.cpp 、

InterfaceAndDeviceDemo.cpp 、InterfaceDemo.cpp 。

【功能更新】

- SDK不再支持Windows XP系统。

- 更新保存图片到文件参数结构体 MV_SAVE_IMAGE_TO_FILE_PARAM_EX 、图片保存

参数结构体 MV_SAVE_IMAGE_PARAM_EX3 。

- 更新显示帧信息结构体 MV_DISPLAY_FRAME_INFO 和

MV_DISPLAY_FRAME_INFO_EX 。

- 更新CCM参数结构体 MV_CC_CCM_PARAM 和 MV_CC_CCM_PARAM_EX 。

- 更新设备信息结构体 MV_CC_DEVICE_INFO 。

- 更新锐化结构体 MV_CC_SHARPEN_PARAM 。

- 更新错误码 。

【文档调整】

- 调整接口说明的模块划分。

### V4.2.0

【功能新增】

- 支持采集卡GE1104。

### V4.1.0

【功能新增】

- 新增图像保存接口： MV_CC_SaveImageEx3()，废弃旧接口 MV_CC_SaveImageEx2()

。

- 新增保存图像到文件接口： MV_CC_SaveImageToFileEx()，废弃旧接口

MV_CC_SaveImageToFile()。

- 新增像素格式转换接口： MV_CC_ConvertPixelTypeEx()，废弃旧接口

MV_CC_ConvertPixelType()。

- 新增图片保存参数结构体： MV_SAVE_IMAGE_PARAM_EX3 。

- 新增保存图片到文件参数结构体： MV_SAVE_IMAGE_TO_FILE_PARAM_EX 。

- 新增像素转换结构体： MV_CC_PIXEL_CONVERT_PARAM_EX 。

- 新增相机参数节点表 章节。

- 增加配置参数，SDK崩溃时，可配置是否生成dump文件。

### V4.0.1

【功能新增】

- 新增显示一帧图像接口： MV_CC_DisplayOneFrameEx() 。

- 新增设置枚举超时时间接口： MV_GIGE_SetEnumDevTimeout()。

- 新增设置U3V相机同步读写超时时间接口： MV_USB_SetSyncTimeOut()。

- 新增获取U3V相机同步读写超时时间接口： MV_USB_GetSyncTimeOut()。

- 新增设置Mono8/bayer格式的Gamma值接口： MV_CC_SetGammaValue()。

- 新增从设备读取文件接口： MV_CC_FileAccessReadEx() 。

- 新增将文件写入设备接口： MV_CC_FileAccessWriteEx() 。

### V4.0.0

【功能新增】

- 新增枚举设备的扩展接口： MV_CC_EnumDevicesEx2()。

- 新增获取当前图像缓存区的有效图像个数接口： MV_CC_GetValidImageNum() 。

- 新增获取Enum型节点指定值符号接口： MV_CC_GetEnumEntrySymbolic() 。

- 新增打开获取或设置相机参数的GUI界面接口： MV_CC_OpenParamsGUI() 。

- 在图像渲染功能中新增绘制矩形接口： MV_CC_DrawRect() 、绘制圆形接口：

MV_CC_DrawCircle() 和绘制线条接口： MV_CC_DrawLines() 。

- 新增重构图像接口： MV_CC_ReconstructImage() 。

- 更新图像对比度调节接口： MV_CC_ImageContrast() 。

- 新增设置插值算法平滑使能接口： MV_CC_SetBayerFilterEnable() 。

- 新增注册流异常消息回调接口： MV_USB_RegisterStreamExceptionCallBack() ，只

支持U3V相机。

- 新增设置U3V事件缓存节点个数接口： MV_USB_SetEventNodeNum() 。

- 新增卸载cti库接口： MV_CC_UnloadGenTLLibrary() 。

- 组件包中新增采集卡SDK的Runtime和开发包。

- 新增支持Windows Server系统。

- 支持GigE相机的抢占权限功能。

- 重载java注册回调接口，注册的回调函数新增相机句柄形参。

- 新增示例程序：OpenParamsGUI.cpp 。

- 支持串口方式配置Camera Link相机参数，新增MvSerial串口配置工具示例demo。

- 新增支持虚拟相机。

### V3.5.0

【功能新增】

- 支持通过ISP Tool实现图像处理：ISP Tool调用SDK生成各种算法的标定文件和算法配置

参数，SDK通过ISP Tool生成的配置文件在所有取图接口（包括主动取图和回调取图）内

部对图像进行ISP算法处理，详见图像采集 。配置文件生成方法见ISP Tool使用说明。

【功能优化】

- 优化LSC 示例程序，删除色彩校正、图像增强及空域降噪示例。

- 支持U3V相机无损码流的获取。

- 支持多线程，实现ISP算子效率优化。

【功能调整】

- 以下接口将逐渐废弃： MV_CC_SetBayerCLUTParam()、 MV_CC_ImageContrast()、

MV_CC_ImageSharpen()、 MV_CC_ColorCorrect()、 MV_CC_NoiseEstimate()、

MV_CC_SpatialDenoise()、 MV_CC_SavePointCloudData()、

MV_CC_SavePointCloudData()。

### V3.4.0

【功能新增】

- 新增设置Bayer格式的Gamma信息接口： MV_CC_SetBayerGammaParam() 。

- 新增设置Bayer格式的CCM使能和矩阵接口： MV_CC_SetBayerCCMParamEx() 。

- 新增设置Bayer格式的CLUT使能和信息接口： MV_CC_SetBayerCLUTParam() 。

- 新增色彩校正（包括CCM和CLUT）接口： MV_CC_ColorCorrect() 。

- 新增图像对比度调节接口： MV_CC_ImageContrast() 。

- 新增图像锐化接口： MV_CC_ImageSharpen() 。

- 新增噪声估计接口： MV_CC_NoiseEstimate() 。

- 新增空域降噪接口： MV_CC_SpatialDenoise() 。

- 新增LSC标定接口： MV_CC_LSCCalib() 。

- 新增LSC校正接口： MV_CC_LSCCorrect() 。

- 新增示例程序：色彩校正、图像增强 、LSC 及空域降噪 。

- 新增算法错误码。

### V3.3.0

【功能新增】

- 新增设置枚举设备回复包是否为广播包接口： MV_GIGE_SetDiscoveryMode()。

- 新增设置GVSP取流超时时间接口： MV_GIGE_SetGvspTimeout()。

- 新增获取GVSP取流超时时间接口： MV_GIGE_GetGvspTimeout()。

- 新增设置重传命令最大尝试次数接口： MV_GIGE_SetResendMaxRetryTimes()。

- 新增获取重传命令最大尝试次数接口： MV_GIGE_GetResendMaxRetryTimes()。

- 新增设置同一重传包多次请求之间的时间间隔接口：

MV_GIGE_SetResendTimeInterval()。

- 新增获取同一重传包多次请求之间的时间间隔接口：

MV_GIGE_GetResendTimeInterval()。

- 新增图像旋转接口： MV_CC_RotateImage()。

- 新增图像翻转接口： MV_CC_FlipImage()。

- 新增设置Bayer格式插值后的Gamma值接口： MV_CC_SetBayerGammaValue()。

- 新增设置Bayer格式插值后的色彩校正使能和矩阵接口：

MV_CC_SetBayerCCMParam()。

- 新增无损解码接口： MV_CC_HB_Decode()。

- 新增Bayer噪声估计接口： MV_CC_BayerNoiseEstimate()。

- 新增Bayer空域降噪接口： MV_CC_BayerSpatialDenoise()。

### V3.2.0

【功能新增】

- 新增获取组播状态函数接口： MV_GIGE_GetMulticastStatus()。

- 新增保存点云格式函数接口： MV_CC_SavePointCloudData()。

- 新增保存图像到文件函数接口： MV_CC_SaveImageToFile()。

- 新增通过GenTL枚举Interfaces函数接口： MV_CC_EnumInterfacesByGenTL()。

- 新增通过GenTL Interface枚举设备函数接口： MV_CC_EnumDevicesByGenTL()。

- 新增通过GenTL设备信息创建设备（句柄）函数接口：

MV_CC_CreateHandleByGenTL()。

【文档调整】

- 删除88个废弃函数接口。

### V3.1.0

【功能新增】

- 新增设置取流策略函数接口： MV_CC_SetGrabStrategy()。

- 新增设置输出缓存个数函数接口： MV_CC_SetOutputQueueSize()。

- 新增获得当前节点的类型函数接口： MV_XML_GetNodeInterfaceType()。

- 新增获得当前节点的访问模式据函数接口： MV_XML_GetNodeAccessMode()。

- 新增设置重传GVCP命令次数函数接口： MV_GIGE_SetRetryGvcpTimes()。

- 新增获取重传GVCP命令次数函数接口： MV_GIGE_GetRetryGvcpTimes()。

- 新增获取GVCP命令超时时间函数接口： MV_GIGE_GetGvcpTimeout()。

- 新增清除取流数据缓存数接口： MV_CC_ClearImageBuffer()。

- 新增设置U3V的传输包大小据函数接口： MV_USB_SetTransferSize()。

- 新增获取U3V的传输包大小据函数接口： MV_USB_GetTransferSize()。

- 新增设置U3V的传输通道个数据函数接口： MV_USB_SetTransferWays()

- 新增获取U3V的传输通道个数据函数接口： MV_USB_GetTransferWays()

### V3.0.0

【功能新增】

- 新增发出PTP时钟拍照动作命令函数接口： MV_GIGE_IssueActionCommand()

- 新增判断相机是否处于连接状态函数接口： MV_CC_IsDeviceConnected()

- 新增设置设备波特率函数接口： MV_CAML_SetDeviceBaudrate()（此接口为

CameraLink设备接口）。

- 新增获取设备波特率函数接口： MV_CAML_GetDeviceBaudrate()（此接口为

CameraLink设备接口）。

- 新增获取设备与主机间连接支持的波特率函数接口：

MV_CAML_GetSupportBaudrates()（此接口为CameraLink设备接口）。

- 新增开始录像函数接口： MV_CC_StartRecord()。

- 新增停止录像函数接口： MV_CC_StopRecord()。

- 新增输入录像数据函数接口： MV_CC_InputOneFrame()。

### V2.4.1

【功能新增】

- 新增显示一帧图像接口： MV_CC_DisplayOneFrame()。

- 新增高性能取流接口： MV_CC_GetImageBuffer()。

- 新增释放图像缓存接口： MV_CC_FreeImageBuffer()。

- 新增获取相机64位int型节点值接口： MV_CC_GetIntValueEx()。

- 新增设置相机64位int型节点值接口： MV_CC_SetIntValueEx()。

- 新增清除GenICam节点缓存接口： MV_CC_InvalidateNodes()。

### V2.4.0

【功能新增】

- 新增140个DotNet接口。

- 新增注册设备Event事件回调函数接口： MV_CC_RegisterAllEventCallBack()和

MV_CC_RegisterEventCallBackEx()。

- 新增GigE相机传输模式设置接口： MV_GIGE_SetTransmissionType()。

- 新增相机文件读写接口： MV_CC_FileAccessRead()和 MV_CC_FileAccessWrite()。

- 新增图片保存接口： MV_CC_SaveImageEx2()。

- 新增SDK日志路径设置接口： MV_CC_SetSDKLogPath()。

- 新增相机Enum型节点设置接口： MV_CC_SetEnumValueByString()。

### V2.2.0.3

【功能新增】

- 在Win10系统对接Gige相机，新增驱动支持。

- MV_CC_GetOneFrameTimeout() 新增支持U3V相机。

- 新增枚举子网内指定的传输协议和指定厂商的所有设备的接口：

MV_CC_EnumDevicesEx()。

- 新增强制设置相机网络参数扩展接口： MV_GIGE_ForceIpEx()。

### V2.1.2.2

【功能新增】

- 新增Win64版本SDK。

- 新增转换像素格式的接口： MV_CC_ConvertPixelType()。

- 新增获取RGB数据的接口： MV_CC_GetImageForRGB()、

MV_CC_RegisterImageCallBackForRGB()。

- 新增获取BGR数据的接口： MV_CC_GetImageForBGR()、

MV_CC_RegisterImageCallBackForBGR()。

### V2.1.1.4

【功能新增】

- 新增采用超时机制获取一帧图片的接口： MV_CC_GetOneFrameTimeout()，仅GIGE相

机支持。

- SDK中GenICam库版本从V2.4更新到V3.0。

- 在win10系统对接Gige相机，不支持安装驱动，走套接字模式通过网络协议对接。

### V2.1.0.1

【功能新增】

- 新增无日志句柄创建接口： MV_CC_CreateHandleWithoutLog()

- 新增相机心跳超时时间配置接口： MV_CC_GetHeartBeatTimeout()、

MV_CC_SetHeartBeatTimeout()。

- 新增设置GVCP命令超时时间接口： MV_GIGE_SetGvcpTimeout()。

- 新增图片数据格式转换接口： MV_CC_SaveImage()、 MV_CC_SaveImageEx()（支持

设置JPEG编码质量）。

- 新增相机行频配置接口： MV_CC_GetAcquisitionLineRate()、

MV_CC_SetAcquisitionLineRate()。

- 新增相机一次触发帧数配置接口： MV_CC_GetBurstFrameCount()、

MV_CC_SetBurstFrameCount()。

- 新增获取图像数据的扩展接口（支持获取chunk信息）： MV_CC_GetOneFrameEx()、

MV_CC_RegisterImageCallBackEx()。

- 新增设置重发包属性接口： MV_GIGE_SetResend()。

- 新增注册Event事件回调接口： MV_CC_RegisterEventCallBack()。

- 新增通用配置接口：

o int类型： MV_CC_GetIntValue()、 MV_CC_SetIntValue()。

o string类型： MV_CC_GetStringValue()、 MV_CC_SetStringValue()

o bool类型： MV_CC_GetBoolValue()、 MV_CC_SetBoolValue()。

o float类型： MV_CC_GetFloatValue()、 MV_CC_SetFloatValue()。

o Enum类型： MV_CC_GetEnumValue()、 MV_CC_GetEnumValue()。

o Command类型： MV_CC_SetCommandValue()。

- MV_CC_DEVICE_INFO （设备信息）中联合体SpecialInfo里面新增结构体参数

stUsb3VInfo（USB设备信息）。

### V2.0.0.3

【功能新增】

- 新增注册图像数据回调函数接口： MV_CC_RegisterImageCallBack()。

- 新增设备用户ID配置接口： MV_CC_GetDeviceUserID()、 MV_CC_SetDeviceUserID()

。

- 新增SDK内部图像缓存节点个数设置接口： MV_CC_SetImageNodeNum()。

- 新增白平衡参数配置接口： MV_CC_GetBalanceWhiteAuto()、

MV_CC_SetBalanceWhiteAuto()、 MV_CC_GetBalanceRatioRed()、

MV_CC_SetBalanceRatioRed()、 MV_CC_GetBalanceRatioGreen()、

MV_CC_SetBalanceRatioGreen()、 MV_CC_GetBalanceRatioBlue()、

MV_CC_SetBalanceRatioBlue()。

- 新增Gamma参数配置接口： MV_CC_GetGammaSelector()、

MV_CC_SetGammaSelector()、 MV_CC_GetGamma()、 MV_CC_SetGamma()。

- 新增色彩参数配置接口： MV_CC_GetSharpness()、 MV_CC_SetSharpness()、

MV_CC_GetHue()、 MV_CC_SetHue()、 MV_CC_GetSaturation()、

MV_CC_SetSaturation()。

- 新增触发参数配置接口： MV_CC_GetTriggerSource()、 MV_CC_SetTriggerSource()

、 MV_CC_GetTriggerDelay()、 MV_CC_SetTriggerDelay()。

- 新增软件触发控制接口： MV_CC_TriggerSoftwareExecute()。

【文档调整】

- 修改接口返回值说明，包括宏定义、宏定义值和描述。

### V2.0.0.1

【功能新增】

- 新增C接口的实现，包括SDK版本信息获取、设备连接、图像采集和显示、参数配置、升

级、寄存器读写、XML解析等相关接口。

- 新增支持U3V设备。

- 新增错误码：0x800000A1~0x800000A5、0x800000ff、0x80000210~0x80000213、

0x80000301~0x80000304。

【功能调整】

- 获取SDK版本接口 MV_CC_GetSDKVision() 名称修改为 MV_CC_GetSDKVersion() 。

### V1.0.0.1

【功能新增】

- 实现C++接口，包括传输层工厂类、设备控制类、GenApi基本接口。

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

### 编程引导

本章节介绍了C接口取图流程和取图方式，以及采集卡接口调用流程。获取图像有两种取流方式， 分别为主动式取流和回调取流。

- 设备连接接口流程
- 采集卡接口调用流程
- 主动取流流程
- 回调取流流程

### 一、设备连接接口流程

对设备进行操作，实现图像采集、参数配置等功能，需要先连接设备（打开设备），具体流程如下 图所示。

详细步骤：

1. 调用 MV_CC_Initialize() 初始化该SDK的运行环境，以便于提前申请和分配所必须的系统 资源。
2. （可选）调用 MV_CC_EnumDevices() 枚举子网内指定传输协议对应的所有设备。 可通 过nTLayerType在结构 MV_CC_DEVICE_INFO() 中获取设备信息。
3. （可选）在打开指定设备前，调用 MV_CC_IsDeviceAccessible() 检查指定设备是否可访 问。
4. 调用 MV_CC_CreateHandle() 创建设备句柄。
5. 调用 MV_CC_OpenDevice() 打开设备。
6. （可选）调用 MV_CC_GetAllMatchInfo()以获取设备信息。
7. 调用 MV_CC_CloseDevice() 关闭设备。
8. 调用 MV_CC_DestroyHandle() 销毁句柄并释放资源。
9. 调用 MV_CC_Finalize() 释放SDK资源。

### 二、采集卡接口调用流程

对自研采集卡进行操作，实现采集卡的打开、参数配置等功能，具体流程如下图所示。

详细步骤：

1. 调用 MV_CC_Initialize() 初始化SDK资源。
2. 调用 MV_CC_EnumInterfaces() 枚举当前环境中的自研采集卡，可通过输入nTLayerType 参数枚举指定类型的采集卡，支持多种类型采集卡同时枚举，通过nTLayerType在结构 MV_INTERFACE_INFO 中获取采集卡的信息。。 可通过nTLayerType在结构 MV_CC_DEVICE_INFO() 中获取设备信息。
3. 调用 MV_CC_CreateInterface() 或者 MV_CC_CreateInterfaceByID() 创建采集卡句柄。
4. 调用 MV_CC_OpenInterface() 打开采集卡。
5. （可选）执行以下一个或多个操作以获取/设置采集卡不同类型的参数。 o 获取/设置Int类型节点值：调用 MV_CC_GetIntValue() / MV_CC_SetIntValue() o 获取/设置Float类型节点值：调用 MV_CC_GetFloatValue() / MV_CC_SetFloatValue() o 获取/设置Enum类型节点值：调用 MV_CC_GetEnumValue() / MV_CC_SetEnumValue() o 获取/设置Bool类型节点值：调用 MV_CC_GetBoolValue() / MV_CC_SetBoolValue() o 获取/设置String类型节点值：调用 MV_CC_GetStringValue() / MV_CC_SetStringValue() o 设置Command类型节点值：调用 MV_CC_SetCommandValue()
6. 调用 MV_CC_CloseInterface() 关闭采集卡。
7. 调用 MV_CC_DestroyInterface() 销毁采集卡句柄。
8. 调用 MV_CC_Finalize() 释放SDK资源。

### 三、主动取流流程

SDK提供主动获取图像的接口，用户可以在开启取流后直接调用此接口获取图像，也可以使用异步 方式（线程、定时器等）获取图像。

- 主动获取图像有两种方式（两种方式不能同时使用）：
1. 调用 MV_CC_StartGrabbing() 开始采集，需要自己开启一个buffer，然后在应用 层循环调用 MV_CC_GetOneFrameTimeout() 获取指定像素格式的帧数据，获取 帧数据时上层应用程序需要根据帧率控制好调用该接口的频率。
2. 调用 MV_CC_StartGrabbing() 开始采集，然后在应用层调用 MV_CC_GetImageBuffer() 获取指定像素格式的帧数据，然后调用 MV_CC_FreeImageBuffer() 释放buffer，获取帧数据时上层应用程序需要根据帧率 控制好调用该接口的频率。
- 主动取图方式使用场景： 需要先调用 MV_CC_StartGrabbing() 启动图像采集。上层应用程序需要根据帧率，控制

好调用主动取图接口的频率。两种主动取图方式都支持设置超时时间，SDK内部等待直到 有数据时返回，可以增加取流平稳性，适合用于对平稳性要求较高的场合。
- 两种主动取图方式的区别：
1. MV_CC_GetImageBuffer() 需要与 MV_CC_FreeImageBuffer() 配套使用，当处 理完取到的数据后，需要用 MV_CC_FreeImageBuffer() 接口将pstFrame内的数据 指针权限进行释放。
2. MV_CC_GetImageBuffer() 与 MV_CC_GetOneFrameTimeout() 相比，有着更高 的效率。且其取流缓存的分配是由sdk内部自动分配的，而 MV_CC_GetOneFrameTimeout() 接口是需要客户自行分配。 注意 ￿ 两种主动取图方式不能同时使用，且不能与后面的回调取图方式同时 使用，三种取图方式只能使用其中一种。 ￿ pData返回的是一个地址指针，建议将pData里面的数据copy出来另 建线程使用。

详细步骤：

1. 调用 MV_CC_Initialize() 初始化该SDK的运行环境，以便于提前申请和分配所必须的系统 资源。
2. （可选）调用 MV_CC_EnumDevices() 枚举子网内指定传输协议对应的所有设备。 可通 过nTLayerType在结构 MV_CC_DEVICE_INFO() 中获取设备信息。
3. （可选）打开指定设备前，调用 MV_CC_IsDeviceAccessible() 检查指定设备是否可访问 。
4. 调用 MV_CC_CreateHandle() 创建设备句柄。
5. 调用 MV_CC_OpenDevice() 打开设备。
6. （可选）执行以下一个或多个操作以获取/设置相机不同类型的参数。 o 获取/设置Int类型节点值：调用 MV_CC_GetIntValue() / MV_CC_SetIntValue() o 获取/设置Float类型节点值：调用 MV_CC_GetFloatValue() / MV_CC_SetFloatValue() o 获取/设置Enum类型节点值：调用 MV_CC_GetEnumValue() / MV_CC_SetEnumValue() o 获取/设置Bool类型节点值：调用 MV_CC_GetBoolValue() / MV_CC_SetBoolValue() o 获取/设置String类型节点值：调用 MV_CC_GetStringValue() / MV_CC_SetStringValue() o 设置Command类型节点值：调用 MV_CC_SetCommandValue()
7. 图像采集： o （可选）调用 MV_CC_SetImageNodeNum() 设置图像缓存节点个数。当获取的图 像数超过这个设定值，最早的图像数据会被自动丢弃。 o 调用 MV_CC_StartGrabbing() 开始取流。 o 对于原始图像数据，可调用 MV_CC_ConvertPixelTypeEx() 转换图像的像素格式 ，也可调用 MV_CC_SaveImageEx3() 转换成JPEG或BMP格式的图片，并保存成 图片文件。 o 在应用程序层中重复调用 MV_CC_GetOneFrameTimeout() 来获取图片数据。
8. 调用 MV_CC_StopGrabbing() 停止采集。
9. 调用 MV_CC_CloseDevice() 关闭设备。
10. 调用 MV_CC_DestroyHandle() 销毁句柄并释放资源。
11. 调用 MV_CC_Finalize() 释放SDK资源。

### 四、回调取流流程

SDK提供注册回调函数的接口，用户可以将自己定义的回调函数注册在SDK内部，SDK获取到相机 图像会主动回调到应用程序供用户处理，此方式可以简化用户程序逻辑。

回调取图方式使用场景：可以使用 MV_CC_RegisterImageCallBackEx() 接口注册回调函数。回 调函数可以自定义，用于自动回调获取的图像。此方法可以简化应用程序逻辑。如果你想设置获取 RGB24或BGR24格式的图像数据，也可以使用 MV_CC_RegisterImageCallBackForRGB() 或 MV_CC_RegisterImageCallBackForBGR() 来注册回调函数。

注意

- 回调函数中不建议有耗时操作、线程锁等，容易造成回调阻塞。
- pData返回的是一个地址指针，建议将pData里面的数据copy出来另建线程使用。

详细步骤：

1. 调用 MV_CC_Initialize() 初始化该SDK的运行环境，以便于提前申请和分配所必须的系统 资源。
2. （可选）调用 MV_CC_EnumDevices() 枚举子网内指定传输协议对应的所有设备。 可通 过nTLayerType在结构 MV_CC_DEVICE_INFO() 中获取设备的信息。
3. （可选）打开指定设备前，调用 MV_CC_IsDeviceAccessible() 检查指定设备是否可访问 。
4. 调用 MV_CC_CreateHandle() 创建设备句柄。
5. 调用 MV_CC_OpenDevice() 打开设备。
6. （可选）执行以下一个或多个操作以获取/设置相机不同类型的参数。 o 获取/设置Int类型节点值 调用 MV_CC_GetIntValue() / MV_CC_SetIntValue() o 获取/设置Float类型节点值 调用 MV_CC_GetFloatValue() / MV_CC_SetFloatValue() o 获取/设置Enum类型节点值 调用 MV_CC_GetEnumValue() / MV_CC_SetEnumValue() o 获取/设置Bool类型节点值 调用 MV_CC_GetBoolValue() / MV_CC_SetBoolValue() o 获取/设置String类型节点值 调用 MV_CC_GetStringValue() / MV_CC_SetStringValue() o 设置Command类型节点值 调用 MV_CC_SetCommandValue()

7. 图像采集：

o （可选）调用 MV_CC_SetImageNodeNum() 设置图像缓存节点个数。当获取的图 像数超过这个设定值，最早的图像数据会被自动丢弃。 o 调用 MV_CC_RegisterImageCallBack() 来设置数据回调函数。 o 调用 MV_CC_StartGrabbing() 开始采集。 o 对于原始图像数据，可调用 MV_CC_ConvertPixelTypeEx() 转换图像的像素格式 ，也可以调用 MV_CC_SaveImageEx3() 转换为JPEG或BMP格式图片，并保存成 图片文件。
8. 调用 MV_CC_StopGrabbing() 停止采集。
9. 调用 MV_CC_CloseDevice() 关闭设备。
10. 调用 MV_CC_DestroyHandle() 销毁句柄并释放资源。
11. 调用 MV_CC_Finalize() 释放SDK资源。

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

### API参考

这里列出了所有模块:

[详情级别 123] SDK版本信息 获取SDK版本信息接口 SDK初始化 包括初始化SDK和反初始化SDK接口 相机初始化 包含枚举相机、创建句柄、打开相机等接口 采集卡初始化 包含枚举采集卡、创建句柄、打开采集卡等接口 ▼参数设置 属性配置 获取和设置设备参数的万能接口 读写寄存器 通过寄存器地址获取和设置相机寄存器数据 设备XML文件 包含获取设备XML文件接口 读写相机文件 包含从设备读取文件、将文件写入设备等接口 图像采集 包含图像获取（回调取图、主动取图）、开始/结束取流接口 等。 支持通过ISP工具生成的配置文件在取图接口内部对图像进行 ISP算法处理，使用时需要先安装ISP插件库，并在C盘个人目 录下创建“ISPTool”文件夹（例如C:/Users/zhangsan/ISPTool ），并将连接相机生成的配置文件以相机型号或序列号命名的

文件夹形式放置在ISPTool下。具体配置文件请联系技术支持 获取。 图像渲染 包含显示一帧图像接口 图像处理 包括图像显示、图像保存和图像格式转换接口等 事件及异常 包括事件及异常接口 GigE相机 仅GigE相机支持的接口 U3V相机 仅U3V相机支持的接口 CameraLink相机 仅Camera Link相机支持的接口 GenTL GenTL相关接口 设备升级 包含设备升级和获取升级进度接口 废弃接口 废弃原因：新开发的万能接口代替了具体节点的设置、获取参 数功能；效率低的接口由新的接口替代；实际开发中部分接口 使用频率较低。 类型定义 包含不同类型的枚举信息 宏定义 包含一些常量定义 ▼错误码 介绍SDK使用过程中不同状态码对应的值和说明 ▼SDK内部自定义状态码 正确码 正确码为0x00000000 通用错误码 范围为0x80000000-0x800000FF GenICam错误码 范围为0x80000100-0x800001FF GigE_STATUS错误码 范围为0x80000200-0x800002FF USB_STATUS错误码 范围为0x80000300-0x800003FF 升级错误码 范围为0x80000400-0x800004FF 异常错误码 0x00008001、0x00008002 ▼算法库透传状态码 通用类型错误码 0x00000000、0x10000000 能力检查错误码 0x10000001 内存检查错误码 范围为0x10000002-0x10000006 图像检查错误码 范围为0x10000007-0x1000000A 输入输出参数检查错误码 范围为0x1000000B-0x10000010 能运行参数检查错误码 范围为0x10000011-0x10000013 接口调用检查错误码 范围为0x10000014-0x10000016 算法库加密相关检查错误码 范围为0x10000017-0x10000018 基本错误类型错误码 范围为0x10000019-0x1000001B 降噪特有错误码 范围为0x10402001-0x1040200f 其他错误码 范围为0x1000001C-0x1000002C

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

函数

### SDK版本信息

获取SDK版本信息接口 更多...

### 函数

MV_CAMCTRL_API unsigned int __stdcall MV_CC_GetSDKVersion () 获取SDK版本号 更多...

### 详细描述

获取SDK版本信息接口

### 函数说明

### ￿ MV_CC_GetSDKVersion()

MV_CAMCTRL_API unsigned int __stdcall MV_CC_GetSDKVersion ( )

获取SDK版本号

返回 始终返回4字节版本号 主 次 修正 测试

8bits 8bits 8bits 8bits 注解
- 比如返回值为0x01000001，即SDK版本号为V1.0.0.1。
- MV_CC_GetSDKVersion() 引入头文件、Lib文件后，SDK环境搭建完毕，可直接调 用。

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

函数

### SDK初始化

包括初始化SDK和反初始化SDK接口 更多...

### 函数

MV_CAMCTRL_API int __stdcall MV_CC_Initialize () 初始化SDK 更多...

MV_CAMCTRL_API int __stdcall MV_CC_Finalize () 反初始化SDK，释放资源 更多...

### 详细描述

包括初始化SDK和反初始化SDK接口

### 函数说明

### ￿ MV_CC_Initialize()

MV_CAMCTRL_API int __stdcall MV_CC_Initialize ( )

初始化SDK

返回 成功，返回MV_OK；失败，返回错误码

示例 ChunkData.cpp, ConnectSpecCamera.cpp, ConvertPixelType.cpp, Events.cpp, Events_Interface.cpp, ForceIPEx.cpp, Grab_ActionCommand.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, InterfaceAndDeviceDemo.cpp, InterfaceDemo.cpp, LensShadingCorrection.cpp, MultiCast.cpp, MultiLightCtrl_ImageStitching.cpp, ParametrizeCamera_FileAccess.cpp, ParametrizeCamera_LineScanIOSettings.cpp, ParametrizeCamera_LoadAndSave.cpp, QuickSoftwareTrigger.cpp , 以及 Recording.cpp.

### ￿ MV_CC_Finalize()

MV_CAMCTRL_API int __stdcall MV_CC_Finalize ( )

反初始化SDK，释放资源

返回 成功，返回MV_OK；失败，返回错误码

注解 需在main函数退出前调用。

示例 ChunkData.cpp, ConnectSpecCamera.cpp, ConvertPixelType.cpp, Events.cpp, Events_Interface.cpp, ForceIPEx.cpp, Grab_ActionCommand.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, InterfaceAndDeviceDemo.cpp, InterfaceDemo.cpp, LensShadingCorrection.cpp, MultiCast.cpp, MultiLightCtrl_ImageStitching.cpp, ParametrizeCamera_FileAccess.cpp, ParametrizeCamera_LineScanIOSettings.cpp, ParametrizeCamera_LoadAndSave.cpp, QuickSoftwareTrigger.cpp , 以及 Recording.cpp.

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

函数

### 相机初始化

包含枚举相机、创建句柄、打开相机等接口 更多...

### 函数

MV_CAMCTRL_API int __stdcall MV_CC_EnumDevices (IN unsigned int nTLayerType, IN OUT MV_CC_DEVICE_INFO_LIST *pstDevList) 枚举设备，支持枚举对应采集卡上的相机 更多...

MV_CAMCTRL_API int __stdcall MV_CC_EnumDevicesEx (IN unsigned int nTLayerType, IN OUT MV_CC_DEVICE_INFO_LIST *pstDevList, IN const char *strManufacturerName) 根据厂商名字枚举设备 更多...

MV_CAMCTRL_API int __stdcall MV_CC_EnumDevicesEx2 (IN unsigned int nTLayerType, IN OUT MV_CC_DEVICE_INFO_LIST *pstDevList, IN const char *strManufacturerName, IN MV_SORT_METHOD enSortMethod) 枚举设备扩展（可指定排序方式枚举、根据厂商名字过滤） ，支持枚举对应采集卡上的相机 更多...

MV_CAMCTRL_API int __stdcall MV_CC_EnumDevicesByInterface (IN void *handle, OUT MV_CC_DEVICE_INFO_LIST *pstDevList) 通过采集卡句柄枚举设备 更多...

MV_CAMCTRL_API bool __stdcall MV_CC_IsDeviceAccessible (IN MV_CC_DEVICE_INFO *pstDevInfo, IN unsigned int nAccessMode) 设备是否可连接 更多...

MV_CAMCTRL_API int __stdcall MV_CC_CreateHandle (IN OUT void **handle, IN const MV_CC_DEVICE_INFO *pstDevInfo) 创建设备句柄 更多...

MV_CAMCTRL_API bool __stdcall MV_CC_IsDeviceConnected (IN void *handle) 判断设备是否处于连接状态 更多...

MV_CAMCTRL_API int __stdcall MV_CC_OpenDevice (IN void *handle, IN unsigned int nAccessMode, IN unsigned short nSwitchoverKey) 打开设备 更多...

MV_CAMCTRL_API int __stdcall MV_CC_GetAllMatchInfo (IN void *handle, IN OUT MV_ALL_MATCH_INFO *pstInfo) 获取各种类型的信息 更多...

MV_CAMCTRL_API int __stdcall MV_CC_GetDeviceInfo (IN void *handle, IN OUT MV_CC_DEVICE_INFO *pstDevInfo) 获取设备信息，取流之前调用 更多...

MV_CAMCTRL_API int __stdcall MV_CC_CloseDevice (IN void *handle) 关闭设备 更多...

MV_CAMCTRL_API int __stdcall MV_CC_DestroyHandle (IN void *handle) 销毁设备句柄 更多...

### 详细描述

包含枚举相机、创建句柄、打开相机等接口

### 函数说明

### ￿ MV_CC_EnumDevices()

MV_CAMCTRL_API int __stdcall MV_CC_EnumDevices ( IN unsigned int nTLayerType,

IN OUT MV_CC_DEVICE_INFO_LIST * pstDevList

)

枚举设备，支持枚举对应采集卡上的相机

参数

nTLayerType [IN] 枚举传输层，参数定义参见CameraParams.h 传输层协议类型定义 值 说明

MV_GIGE_DEVICE 0x00000001 GigE设备

MV_1394_DEVICE 0x00000002 1394-a/b设备

MV_USB_DEVICE 0x00000004 USB设备

CameraLink设备 MV_CAMERALINK_DEVICE 0x00000008 （串口）

MV_VIR_GIGE_DEVICE 0x00000010 虚拟GigE设备

MV_VIR_USB_DEVICE 0x00000020 虚拟USB设备

自研网卡下GigE MV_GENTL_GIGE_DEVICE 0x00000040 设备

MV_GENTL_CAMERALINK_DEVICE 0x00000080 CameraLink设备

MV_GENTL_CXP_DEVICE 0x00000100 CoaXPress设备

MV_GENTL_XOF_DEVICE 0x00000200 XoF设备

pstDevList [IN][OUT] 设备列表 返回 成功，返回MV_OK；失败，返回错误码

注解
- 设备列表的内存是在SDK内部分配的，多线程调用该接口时会进行设备列表内存的释 放和申请，建议尽量避免多线程枚举操作。
- 参数枚举传输层，适配传入MV_GIGE_DEVICE、MV_1394_DEVICE、 MV_USB_DEVICE、MV_CAMERALINK_DEVICE；MV_GIGE_DEVICE该参数传出所 有GiGE相关的设备信息（包含虚拟GiGE和GenTL下的GiGE设备）， MV_USB_DEVICE该参数传出所有USB设备，包含虚拟USB设备。

示例 ChunkData.cpp, ConvertPixelType.cpp, Events.cpp, ForceIPEx.cpp, Grab_ActionCommand.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, LensShadingCorrection.cpp, MultiCast.cpp, MultiLightCtrl_ImageStitching.cpp, ParametrizeCamera_FileAccess.cpp,

ParametrizeCamera_LineScanIOSettings.cpp, ParametrizeCamera_LoadAndSave.cpp, QuickSoftwareTrigger.cpp , 以及 Recording.cpp.

### ￿ MV_CC_EnumDevicesEx()

MV_CAMCTRL_API int __stdcall MV_CC_EnumDevicesEx ( IN unsigned int nTLayerType,

IN OUT MV_CC_DEVICE_INFO_LIST * pstDevList,

IN const char * strManufacturerName

)

根据厂商名字枚举设备

参数

nTLayerType [IN] 枚举传输层，参数定义参见CameraParams.h 传输层协议类型定义 值 说明

0x0000000 MV_GIGE_DEVICE GigE设备

0x00000001394-a/b设 MV_1394_DEVICE 2备

0x0000000 MV_USB_DEVICE USB设备

CameraLin 0x0000000 MV_CAMERALINK_DEVICE k设备（串 口）

0x0000001虚拟GigE MV_VIR_GIGE_DEVICE 0设备

0x0000002虚拟USB设 MV_VIR_USB_DEVICE 0备

0x0000004自研网卡下 MV_GENTL_GIGE_DEVICE 0GigE设备

MV_GENTL_CAMERALINK_DEVI0x0000008CameraLin CE0k设备

0x0000010CoaXPress MV_GENTL_CXP_DEVICE 0设备

0x0000020 MV_GENTL_XOF_DEVICE XoF设备

pstDevList [IN][OUT] 设备列表

strManufacturerNam [IN] 厂商名字 e 返回 成功，返回MV_OK；失败，返回错误码

注解
- 参数枚举传输层，适配传入MV_GIGE_DEVICE、MV_1394_DEVICE、 MV_USB_DEVICE、MV_CAMERALINK_DEVICE；MV_GIGE_DEVICE该参数传出所 有GiGE相关的设备信息（包含虚拟GiGE和GenTL下的GiGE设备）， MV_USB_DEVICE该参数传出所有USB设备，包含虚拟USB设备。
- 设备列表的内存是在SDK内部分配的，多线程调用该接口时会进行设备列表内存的释 放和申请，建议尽量避免多线程枚举操作。
- MV_GENTL_GIGE_DEVICE、MV_GENTL_CAMERALINK_DEVICE、 MV_GENTL_CXP_DEVICE、MV_GENTL_XOF_DEVICE传输层可以返回对采集卡下的 相机信息。

### ￿ MV_CC_EnumDevicesEx2()

MV_CAMCTRL_API int __stdcall MV_CC_EnumDevicesEx2 ( IN unsigned int nTLayerType,

IN OUT MV_CC_DEVICE_INFO_LIST * pstDevList,

IN const char * strManufacturerName,

IN MV_SORT_METHOD enSortMethod

)

枚举设备扩展（可指定排序方式枚举、根据厂商名字过滤），支持枚举对应采集卡上的相机

参数

nTLayerType [IN] 枚举传输层（区分每一种传输层类型，不耦合），参数定义参 见CameraParams.h 传输层协议类型定义 值 说明

0x0000000 MV_GIGE_DEVICE GigE设备

0x00000001394-a/b设 MV_1394_DEVICE 2备

0x0000000 MV_USB_DEVICE USB设备

CameraLin 0x0000000 MV_CAMERALINK_DEVICE k设备（串 口）

0x0000001虚拟GigE MV_VIR_GIGE_DEVICE 0设备

0x0000002虚拟USB设 MV_VIR_USB_DEVICE 0备

0x0000004自研网卡下 MV_GENTL_GIGE_DEVICE 0GigE设备

MV_GENTL_CAMERALINK_DEVI0x0000008CameraLin CE0k设备

0x0000010CoaXPress MV_GENTL_CXP_DEVICE 0设备

0x0000020 MV_GENTL_XOF_DEVICE XoF设备

pstDevList [IN][OUT] 设备列表

strManufacturerNam [IN] 厂商名字（可传NULL，即不过滤） e

enSortMethod [IN] 排序方式 返回 成功，返回MV_OK；失败，返回错误码

注解
- 设备列表的内存是在SDK内部分配的，多线程调用该接口时会进行设备列表内存的释 放和申请，建议尽量避免多线程枚举操作。
- strManufacturerName可传入NULL，若传入NULL则返回排好序的所有设备列表，若 不为NULL则只返回排好序的指定厂商设备列表。
- MV_GENTL_GIGE_DEVICE、MV_GENTL_CAMERALINK_DEVICE、 MV_GENTL_CXP_DEVICE、MV_GENTL_XOF_DEVICE传输层可以返回对采集卡下的

相机信息。
- MV_CC_EnumDevicesEx2() 与 MV_CC_EnumDevicesEx() 接口的差异如下：

- MV_CC_EnumDevicesEx() 传入MV_GIGE_DEVICE，除了枚举网段内的网口 相机以外，还会枚举虚拟网口相机和自研采集卡下的网口相机；若传入 MV_USB_DEVICE，则会枚举USB口相机和虚拟USB口相机。
- MV_CC_EnumDevicesEx2() 传入MV_GIGE_DEVICE，仅枚举网段内的网口 相机；若传入MV_USB_DEVICE，则仅枚举USB口相机。
- MV_CC_EnumDevicesEx2() 多出排序的功能，由参数MV_SORT_METHOD enSortMethod决定排序方式。

### ￿ MV_CC_EnumDevicesByInterface()

MV_CAMCTRL_API int __stdcall MV_CC_EnumDevicesByInterface ( IN void * handle,

OUT MV_CC_DEVICE_INFO_LIST * pstDevList

)

通过采集卡句柄枚举设备

参数

handle [IN] 采集卡句柄

pstDevList [OUT] 设备列表 返回 成功，返回MV_OK；失败，返回错误码

注解
- 设备列表的内存是在SDK内部分配的，多线程调用该接口时会进行设备列表内存的释 放和申请，
- 建议尽量避免多线程枚举操作。

示例 Events_Interface.cpp , 以及 InterfaceAndDeviceDemo.cpp.

### ￿ MV_CC_IsDeviceAccessible()

MV_CAMCTRL_API bool __stdcall MV_CC_IsDeviceAccessible ( IN MV_CC_DEVICE_INFO * pstDevInfo,

IN unsigned int nAccessMode

)

设备是否可连接

参数

pstDevInfo [IN] 设备信息结构体

nAccessMode [IN] 访问权限 宏 定 宏定义含义 义 值

独占权限，其 MV_ACCESS_Exclusive 1他APP只允许 读CCP寄存器

可以从5模式下 抢占权限，然 MV_ACCESS_ExclusiveWithSwitch 2 后以独占权限 打开

控制权限，其 MV_ACCESS_Control 3他APP允许读 所有寄存器

可以从5模式下 抢占权限，然 MV_ACCESS_ControlWithSwitch 4 后以控制权限 打开

以可被抢占的 MV_ACCESS_ControlSwitchEnable 5 控制权限打开

可以从5模式下 抢占权限，然 MV_ACCESS_ControlSwitchEnableWithKey 6后以可被抢占 的控制权限打 开

读模式打开设 MV_ACCESS_Monitor 7备，适用于控 制权限下

返回

可达，返回true；不可达，返回false

注解
- 读取设备CCP寄存器的值，判断当前状态是否具有某种访问权限。
- 如果设备不支持MV_ACCESS_ExclusiveWithSwitch、 MV_ACCESS_ControlWithSwitch、MV_ACCESS_ControlSwitchEnableWithKey这三 种模式，接口返回false。目前设备不支持这3种抢占模式，国际上主流的厂商的设备也 都暂不支持这3种模式。

示例 ForceIPEx.cpp.

### ￿ MV_CC_CreateHandle()

MV_CAMCTRL_API int __stdcall MV_CC_CreateHandle ( IN OUT void ** handle,

IN const MV_CC_DEVICE_INFO * pstDevInfo

)

创建设备句柄

参数

handle [IN][OUT] 设备句柄

pstDevInfo [IN] 设备信息结构体 返回 成功，返回MV_OK；失败，返回错误码

注解
- 根据输入的设备信息，创建库内部必须的资源和初始化内部模块。
- 通过该接口创建句柄，调用SDK接口，会默认生成SDK日志文件。

示例 ChunkData.cpp, ConnectSpecCamera.cpp, ConvertPixelType.cpp, Events.cpp, Events_Interface.cpp, ForceIPEx.cpp, Grab_ActionCommand.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, InterfaceAndDeviceDemo.cpp, LensShadingCorrection.cpp, MultiCast.cpp, MultiLightCtrl_ImageStitching.cpp, ParametrizeCamera_FileAccess.cpp, ParametrizeCamera_LineScanIOSettings.cpp,

ParametrizeCamera_LoadAndSave.cpp, QuickSoftwareTrigger.cpp , 以及 Recording.cpp.

### ￿ MV_CC_IsDeviceConnected()

MV_CAMCTRL_API bool __stdcall MV_CC_IsDeviceConnected ( IN void * handle )

判断设备是否处于连接状态

参数

handle [IN] 设备句柄 返回 设备处于连接状态，返回true；没连接或失去连接，返回false。

### ￿ MV_CC_OpenDevice()

MV_CAMCTRL_API int __stdcall MV_CC_OpenDevice ( IN void * handle,

IN unsigned int nAccessMode,

IN unsigned short nSwitchoverKey

)

打开设备

参数

handle [IN] 设备句柄

nAccessMode [IN] 访问权限 宏 定 宏定义含义 义 值

独占权限，其 他APP只允许 MV_ACCESS_Exclusive 1 读CCP寄存 器

可以从5模式 MV_ACCESS_ExclusiveWithSwitch 2 下抢占权限，

然后以独占权 限打开

控制权限，其 MV_ACCESS_Control 3他APP允许读 所有寄存器

可以从5模式 下抢占权限， MV_ACCESS_ControlWithSwitch 4 然后以控制权 限打开

以可被抢占的 MV_ACCESS_ControlSwitchEnable 5 控制权限打开

可以从5模式 下抢占权限， MV_ACCESS_ControlSwitchEnableWithKey 6然后以可被抢 占的控制权限 打开

读模式打开设 MV_ACCESS_Monitor 7备，适用于控 制权限下

nSwitchoverKey [IN] 切换访问权限时的密钥 返回 成功，返回MV_OK；失败，返回错误码

注解
- 根据设置的设备参数，找到对应的设备，连接设备。
- 调用接口时可不传入nAccessMode和nSwitchoverKey，此时默认设备访问模式为独占 权限
- 目前GigE设备的相机固件暂不支持MV_ACCESS_ExclusiveWithSwitch、 MV_ACCESS_ControlWithSwitch、MV_ACCESS_ControlSwitchEnable、 MV_ACCESS_ControlSwitchEnableWithKey这四种抢占模式
- GenTL设备只支持 nAccessMode 是 MV_ACCESS_Exclusive 、 MV_ACCESS_Control 、MV_ACCESS_Monitor权限
- 对于U3V设备，CXP，CameraLink设备，Xof设备，虚拟GEV，虚拟U3V设备， nAccessMode、nSwitchoverKey这两个参数无效；默认以控制权限 MV_ACCESS_Control 打开设备
- 该接口支持网口设备不枚举直接打开，不支持U口和GenTL设备不枚举打开设备

示例

ChunkData.cpp, ConnectSpecCamera.cpp, ConvertPixelType.cpp, Events.cpp, Events_Interface.cpp, ForceIPEx.cpp, Grab_ActionCommand.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, InterfaceAndDeviceDemo.cpp, LensShadingCorrection.cpp, MultiCast.cpp, MultiLightCtrl_ImageStitching.cpp, ParametrizeCamera_FileAccess.cpp, ParametrizeCamera_LineScanIOSettings.cpp, ParametrizeCamera_LoadAndSave.cpp, QuickSoftwareTrigger.cpp , 以及 Recording.cpp.

### ￿ MV_CC_GetAllMatchInfo()

MV_CAMCTRL_API int __stdcall MV_CC_GetAllMatchInfo ( IN void * handle,

IN OUT MV_ALL_MATCH_INFO * pstInfo

)

获取各种类型的信息

参数

handle [IN] 设备句柄

pstInfo [IN][OUT] 返回给调用者有关设备各种类型的信息结构体指针 返回 成功，返回MV_OK；失败，返回错误码

注解
- 接口里面输入需要获取的信息类型（指定 MV_ALL_MATCH_INFO 结构体中的nType 类型），获取对应的信息（在 MV_ALL_MATCH_INFO 结构体中pInfo里返回）。
- 该接口的调用前置条件取决于所获取的信息类型，获取GigE设备的 MV_MATCH_TYPE_NET_DETECT 信息需在开启抓图之后调用，获取U3V设备的 MV_MATCH_TYPE_USB_DETECT 信息需在打开设备之后调用。
- 该接口不支持Camera Link设备。

### ￿ MV_CC_GetDeviceInfo()

MV_CAMCTRL_API int __stdcall MV_CC_GetDeviceInfo ( IN void * handle,

IN OUT MV_CC_DEVICE_INFO * pstDevInfo

)

获取设备信息，取流之前调用

参数

handle [IN] 设备句柄

pstDevInfo [IN][OUT] 返回给调用者有关设备信息结构体指针 返回 成功，返回MV_OK；失败，返回错误码

注解
- 支持用户在打开设备后获取设备信息，不支持GenTL设备。
- 若该设备是GigE设备，则调用该接口存在阻塞风险，因此不建议在取流过程中调用该 接口。

### ￿ MV_CC_CloseDevice()

MV_CAMCTRL_API int __stdcall MV_CC_CloseDevice ( IN void * handle )

关闭设备

参数

handle [IN] 设备句柄 返回 成功，返回MV_OK；失败，返回错误码

注解 通过 MV_CC_OpenDevice() 连接设备后，可以通过该接口断开设备连接，释放资源。

示例 ChunkData.cpp, ConnectSpecCamera.cpp, ConvertPixelType.cpp, Events.cpp, Events_Interface.cpp, ForceIPEx.cpp, Grab_ActionCommand.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, InterfaceAndDeviceDemo.cpp, LensShadingCorrection.cpp, MultiCast.cpp, MultiLightCtrl_ImageStitching.cpp, ParametrizeCamera_FileAccess.cpp, ParametrizeCamera_LineScanIOSettings.cpp,

ParametrizeCamera_LoadAndSave.cpp, QuickSoftwareTrigger.cpp , 以及 Recording.cpp.

### ￿ MV_CC_DestroyHandle()

MV_CAMCTRL_API int __stdcall MV_CC_DestroyHandle ( IN void * handle )

销毁设备句柄

参数

handle [IN] 设备句柄 返回 成功，返回MV_OK；失败，返回错误码

示例 ChunkData.cpp, ConnectSpecCamera.cpp, ConvertPixelType.cpp, Events.cpp, Events_Interface.cpp, ForceIPEx.cpp, Grab_ActionCommand.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, InterfaceAndDeviceDemo.cpp, LensShadingCorrection.cpp, MultiCast.cpp, MultiLightCtrl_ImageStitching.cpp, ParametrizeCamera_FileAccess.cpp, ParametrizeCamera_LineScanIOSettings.cpp, ParametrizeCamera_LoadAndSave.cpp, QuickSoftwareTrigger.cpp , 以及 Recording.cpp.

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

函数

### 采集卡初始化

包含枚举采集卡、创建句柄、打开采集卡等接口 更多...

### 函数

MV_CAMCTRL_API int __stdcall MV_CC_EnumInterfaces (IN unsigned int nTLayerType, IN OUT MV_INTERFACE_INFO_LIST *pInterfaceInfoList) 枚举采集卡 更多...

MV_CAMCTRL_API int __stdcall MV_CC_CreateInterface (IN OUT void **handle, IN MV_INTERFACE_INFO *pInterfaceInfo) 创建采集卡句柄 更多...

MV_CAMCTRL_API int __stdcall MV_CC_CreateInterfaceByID (IN OUT void **handle, IN char *pInterfaceID) 通过采集卡ID创建采集卡句柄 更多...

MV_CAMCTRL_API int __stdcall MV_CC_OpenInterface (IN void *handle, IN char *pReserved) 打开采集卡 更多...

MV_CAMCTRL_API int __stdcall MV_CC_CloseInterface (IN void *handle) 关闭采集卡 更多...

MV_CAMCTRL_API int __stdcall MV_CC_DestroyInterface (IN void *handle) 销毁采集卡句柄 更多...

### 详细描述

包含枚举采集卡、创建句柄、打开采集卡等接口

### 函数说明

### ￿ MV_CC_EnumInterfaces()

MV_CAMCTRL_API int __stdcall MV_CC_EnumInterfaces ( IN unsigned int nTLayerType,

IN OUT MV_INTERFACE_INFO_LIST * pInterfaceInfoList

)

枚举采集卡

参数

nTLayerType [IN] 采集卡接口类型 采集卡接口类型定义 值 说明

GigE Vision采集 MV_GIGE_INTERFACE 0x00000001 卡

Camera Link采 MV_CAMERALINK_INTERFACE 0x00000004 集卡

CoaXPress采集 MV_CXP_INTERFACE 0x00000008 卡

MV_XOF_INTERFACE 0x00000010 XoFLink采集卡

pInterfaceInfoList [IN][OUT] 采集卡列表 返回 成功，返回MV_OK；失败，返回错误码

示例 Events_Interface.cpp, InterfaceAndDeviceDemo.cpp, InterfaceDemo.cpp , 以及 QuickSoftwareTrigger.cpp.

### ￿ MV_CC_CreateInterface()

MV_CAMCTRL_API int __stdcall MV_CC_CreateInterface ( IN OUT void ** handle,

IN MV_INTERFACE_INFO * pInterfaceInfo

)

创建采集卡句柄

参数

handle [OUT] 采集卡句柄

pInterfaceInfo [IN] 采集卡信息 返回 成功，返回MV_OK；失败，返回错误码

示例 Events_Interface.cpp, InterfaceAndDeviceDemo.cpp, InterfaceDemo.cpp , 以及 QuickSoftwareTrigger.cpp.

### ￿ MV_CC_CreateInterfaceByID()

MV_CAMCTRL_API int __stdcall MV_CC_CreateInterfaceByID ( IN OUT void ** handle,

IN char * pInterfaceID

)

通过采集卡ID创建采集卡句柄

参数

handle [IN][OUT] 采集卡句柄

pInterfaceID [IN] 采集卡ID，比如采集卡序列号 返回 成功，返回MV_OK；失败，返回错误码

### ￿ MV_CC_OpenInterface()

MV_CAMCTRL_API int __stdcall MV_CC_OpenInterface ( IN void * handle,

IN char * pReserved

)

打开采集卡

参数

handle [IN] 采集卡句柄

pReserved [IN] 预留，直接填NULL 返回 成功，返回MV_OK；失败，返回错误码

示例 Events_Interface.cpp, InterfaceAndDeviceDemo.cpp, InterfaceDemo.cpp , 以及 QuickSoftwareTrigger.cpp.

### ￿ MV_CC_CloseInterface()

MV_CAMCTRL_API int __stdcall MV_CC_CloseInterface ( IN void * handle )

关闭采集卡

参数

handle [IN] 采集卡句柄 返回 成功，返回MV_OK；失败，返回错误码

示例 Events_Interface.cpp, InterfaceAndDeviceDemo.cpp, InterfaceDemo.cpp , 以及 QuickSoftwareTrigger.cpp.

### ￿ MV_CC_DestroyInterface()

MV_CAMCTRL_API int __stdcall MV_CC_DestroyInterface ( IN void * handle )

销毁采集卡句柄

参数

handle [IN] 采集卡句柄 返回 成功，返回MV_OK；失败，返回错误码

注解

如果传入相机句柄，其效果和 MV_CC_DestroyHandle 相同;

示例 Events_Interface.cpp, InterfaceAndDeviceDemo.cpp, InterfaceDemo.cpp , 以及 QuickSoftwareTrigger.cpp.

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

模块

### 参数设置

### 模块

属性配置 获取和设置设备参数的万能接口

读写寄存器 通过寄存器地址获取和设置相机寄存器数据

设备XML文件 包含获取设备XML文件接口

读写相机文件 包含从设备读取文件、将文件写入设备等接口

### 详细描述

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

函数

### 属性配置

参数设置

获取和设置设备参数的万能接口 更多...

### 函数

MV_CAMCTRL_API int __stdcall MV_CC_GetIntValueEx (IN void *handle, IN const char *strKey, IN OUT MVCC_INTVALUE_EX *pstIntValue) 获取Integer属性值 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SetIntValueEx (IN void *handle, IN const char *strKey, IN int64_t nValue) 设置Integer型属性值 更多...

MV_CAMCTRL_API int __stdcall MV_CC_GetEnumValue (IN void *handle, IN const char *strKey, IN OUT MVCC_ENUMVALUE *pstEnumValue) 获取Enum属性值 更多...

MV_CAMCTRL_API int __stdcall MV_CC_GetEnumValueEx (IN void *handle, IN const char *strKey, IN OUT MVCC_ENUMVALUE_EX *pstEnumValue) 获取Enum属性值(扩展接口) 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SetEnumValue (IN void *handle, IN const char *strKey, IN unsigned int nValue) 设置Enum型属性值 更多...

MV_CAMCTRL_API int __stdcall MV_CC_GetEnumEntrySymbolic (IN void *handle, IN const char *strKey, IN OUT MVCC_ENUMENTRY *pstEnumEntry) 获取Enum型节点指定值的符号 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SetEnumValueByString (IN void *handle, IN const char *strKey, IN const char *strValue) 设置Enum型属性值 更多...

MV_CAMCTRL_API int __stdcall MV_CC_GetFloatValue (IN void *handle, IN const char *strKey, IN OUT MVCC_FLOATVALUE *pstFloatValue) 获取Float属性值 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SetFloatValue (IN void *handle, IN const char *strKey, IN float fValue) 设置Float型属性值 更多...

MV_CAMCTRL_API int __stdcall MV_CC_GetBoolValue (IN void *handle, IN const char *strKey, IN OUT bool *pbValue) 获取Boolean属性值 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SetBoolValue (IN void *handle, IN const char *strKey, IN bool bValue)

设置Boolean型属性值 更多...

MV_CAMCTRL_API int __stdcall MV_CC_GetStringValue (IN void *handle, IN const char *strKey, IN OUT MVCC_STRINGVALUE *pstStringValue) 获取String属性值 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SetStringValue (IN void *handle, IN const char *strKey, IN const char *strValue) 设置String型属性值 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SetCommandValue (IN void *handle, IN const char *strKey) 设置Command型属性值 更多...

MV_CAMCTRL_API int __stdcall MV_CC_FeatureLoad (IN void *handle, IN const char *strFileName) 导入设备属性 更多...

MV_CAMCTRL_API int __stdcall MV_CC_FeatureLoadEx (IN void *handle, IN const char *strFileName, IN OUT MVCC_NODE_ERROR_LIST *pstNodeErrorList) 导入设备属性并保存错误信息列表 更多...

MV_CAMCTRL_API int __stdcall MV_CC_FeatureSave (IN void *handle, IN const char *strFileName) 保存设备属性 更多...

### 详细描述

获取和设置设备参数的万能接口

### 函数说明

### ￿ MV_CC_GetIntValueEx()

MV_CAMCTRL_API int __stdcall MV_CC_GetIntValueEx ( IN void * handle,

IN const char * strKey,

IN OUT MVCC_INTVALUE_EX * pstIntValue

)

获取Integer属性值

参数

handle [IN] 设备句柄

strKey [IN] 属性键值，如获取宽度信息则为"Width"

pstIntValue [IN][OUT] 返回给调用者有关设备属性结构体指针 返回 成功，返回MV_OK；失败，返回错误码

注解 连接设备之后调用该接口可以获取int类型的指定节点的值，具体可以查看客户端属性描 述，如下图所示：

示例 Grab_Asynchronous.cpp , 以及 InterfaceDemo.cpp.

### ￿ MV_CC_SetIntValueEx()

MV_CAMCTRL_API int __stdcall MV_CC_SetIntValueEx ( IN void * handle,

IN const char * strKey,

IN int64_t nValue

)

设置Integer型属性值

参数

handle [IN] 设备句柄

strKey [IN] 属性键值，如设置宽度信息则为"Width"

nValue [IN] 想要设置的设备的属性值 返回 成功，返回MV_OK；失败，返回错误码

注解 连接设备之后调用该接口可以设置int类型的指定节点的值，具体可以查看客户端属性描 述，如下图所示：

示例 ConnectSpecCamera.cpp, ConvertPixelType.cpp, Events.cpp, Grab_ActionCommand.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, InterfaceAndDeviceDemo.cpp, InterfaceDemo.cpp, LensShadingCorrection.cpp, MultiCast.cpp, ParametrizeCamera_LineScanIOSettings.cpp , 以及 Recording.cpp.

### ￿ MV_CC_GetEnumValue()

MV_CAMCTRL_API int __stdcall MV_CC_GetEnumValue ( IN void * handle,

IN const char * strKey,

IN OUT MVCC_ENUMVALUE * pstEnumValue

)

获取Enum属性值

参数

handle [IN] 设备句柄

strKey [IN] 属性键值，如获取像素格式信息则为"PixelFormat"

pstEnumValue [IN][OUT] 返回给调用者有关设备属性结构体指针 返回 成功，返回MV_OK；失败，返回错误码

注解 连接设备之后调用该接口可以获取Enum类型的指定节点的值。

示例 InterfaceDemo.cpp , 以及 Recording.cpp.

### ￿ MV_CC_GetEnumValueEx()

MV_CAMCTRL_API int __stdcall MV_CC_GetEnumValueEx ( IN void * handle,

IN const char * strKey,

IN OUT MVCC_ENUMVALUE_EX * pstEnumValue

)

获取Enum属性值(扩展接口)

参数

handle [IN] 设备句柄/采集卡句柄

strKey [IN] 属性键值，如获取像素格式信息则为"PixelFormat"

pstEnumValue [IN][OUT] 返回给调用者有关设备属性结构体指针 返回 成功，返回MV_OK；失败，返回错误码

注解

- 连接设备之后调用该接口可以获取Enum类型的指定节点的值。
- 该接口返回的枚举有效个数扩展到256个。

### ￿ MV_CC_SetEnumValue()

MV_CAMCTRL_API int __stdcall MV_CC_SetEnumValue ( IN void * handle,

IN const char * strKey,

IN unsigned int nValue

)

设置Enum型属性值

参数

handle [IN] 设备句柄

strKey [IN] 属性键值，如设置像素格式信息则为"PixelFormat"

nValue [IN] 想要设置的设备的属性值 返回 成功，返回MV_OK；失败，返回错误码

注解 连接设备之后调用该接口可以设置Enum类型的指定节点的值。

示例 ChunkData.cpp, ConnectSpecCamera.cpp, ConvertPixelType.cpp, Events.cpp, Grab_ActionCommand.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, GrabImage.cpp, GrabImage_Display.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, InterfaceAndDeviceDemo.cpp, InterfaceDemo.cpp, LensShadingCorrection.cpp, MultiLightCtrl_ImageStitching.cpp, ParametrizeCamera_LineScanIOSettings.cpp, QuickSoftwareTrigger.cpp , 以及 Recording.cpp.

### ￿ MV_CC_GetEnumEntrySymbolic()

MV_CAMCTRL_API int __stdcall MV_CC_GetEnumEntrySymbolic ( IN void * handle,

IN const char * strKey,

IN OUT MVCC_ENUMENTRY * pstEnumEntry

)

获取Enum型节点指定值的符号

参数

handle [IN] 设备句柄

strKey [IN] 属性键值，如获取像素格式信息则为"PixelFormat"

pstEnumEntry [IN][OUT] 想要获取的设备的属性符号 返回 成功，返回MV_OK；失败，返回错误码

注解 连接设备之后调用该接口可以获取Enum类型的指定节点的值。

示例 InterfaceDemo.cpp.

### ￿ MV_CC_SetEnumValueByString()

MV_CAMCTRL_API int __stdcall MV_CC_SetEnumValueByString ( IN void * handle,

IN const char * strKey,

IN const char * strValue

)

设置Enum型属性值

参数

handle [IN] 设备句柄

strKey [IN] 属性键值，如设置像素格式信息则为"PixelFormat"

strValue [IN] 想要设置的设备的属性字符串 返回 成功，返回MV_OK；失败，返回错误码

注解 连接设备之后调用该接口可以设置Enum类型的指定节点的值。

示例 ChunkData.cpp, Grab_ActionCommand.cpp, GrabStrategies.cpp , 以及 QuickSoftwareTrigger.cpp.

### ￿ MV_CC_GetFloatValue()

MV_CAMCTRL_API int __stdcall MV_CC_GetFloatValue ( IN void * handle,

IN const char * strKey,

IN OUT MVCC_FLOATVALUE * pstFloatValue

)

获取Float属性值

参数

handle [IN] 设备句柄

strKey [IN] 属性键值

pstFloatValue [IN][OUT] 返回给调用者有关设备属性结构体指针 返回 成功，返回MV_OK；失败，返回错误码

注解 连接设备之后调用该接口可以获取float类型的指定节点的值。

示例 InterfaceDemo.cpp , 以及 Recording.cpp.

### ￿ MV_CC_SetFloatValue()

MV_CAMCTRL_API int __stdcall MV_CC_SetFloatValue ( IN void * handle,

IN const char * strKey,

IN float fValue

)

设置Float型属性值

参数

handle [IN] 设备句柄

strKey [IN] 属性键值

fValue [IN] 想要设置的设备的属性值 返回 成功，返回MV_OK；失败，返回错误码

注解

连接设备之后调用该接口可以设置float类型的指定节点的值。

示例 InterfaceDemo.cpp.

### ￿ MV_CC_GetBoolValue()

MV_CAMCTRL_API int __stdcall MV_CC_GetBoolValue ( IN void * handle,

IN const char * strKey,

IN OUT bool * pbValue

)

获取Boolean属性值

参数

handle [IN] 设备句柄

strKey [IN] 属性键值

pbValue [OUT] 返回给调用者有关设备属性值 返回 成功，返回MV_OK；失败，返回错误码

注解 连接设备之后调用该接口可以获取bool类型的指定节点的值。

示例 InterfaceDemo.cpp.

### ￿ MV_CC_SetBoolValue()

MV_CAMCTRL_API int __stdcall MV_CC_SetBoolValue ( IN void * handle,

IN const char * strKey,

IN bool bValue

)

设置Boolean型属性值

参数

handle [IN] 设备句柄

strKey [IN] 属性键值

bValue [IN] 想要设置的设备的属性值 返回 成功，返回MV_OK；失败，返回错误码

注解 连接设备之后调用该接口可以设置bool类型的指定节点的值。

示例 ChunkData.cpp , 以及 InterfaceDemo.cpp.

### ￿ MV_CC_GetStringValue()

MV_CAMCTRL_API int __stdcall MV_CC_GetStringValue ( IN void * handle,

IN const char * strKey,

IN OUT MVCC_STRINGVALUE * pstStringValue

)

获取String属性值

参数

handle [IN] 设备句柄

strKey [IN] 属性键值

pstStringValue [IN][OUT] 返回给调用者有关设备属性结构体指针 返回 成功，返回MV_OK；失败，返回错误码

注解 连接设备之后调用该接口可以获取string类型的指定节点的值。

示例 InterfaceDemo.cpp.

### ￿ MV_CC_SetStringValue()

MV_CAMCTRL_API int __stdcall MV_CC_SetStringValue ( IN void * handle,

IN const char * strKey,

IN const char * strValue

)

设置String型属性值

参数

handle [IN] 设备句柄

strKey [IN] 属性键值

strValue [IN] 想要设置的设备的属性值 返回 成功，返回MV_OK；失败，返回错误码

注解 连接设备之后调用该接口可以设置string类型的指定节点的值。

示例 InterfaceDemo.cpp.

### ￿ MV_CC_SetCommandValue()

MV_CAMCTRL_API int __stdcall MV_CC_SetCommandValue ( IN void * handle,

IN const char * strKey

)

设置Command型属性值

参数

handle [IN] 设备句柄

strKey [IN] 属性键值 返回 成功，返回MV_OK；失败，返回错误码

注解 连接设备之后调用该接口可以设置指定的Command类型节点。

示例 GrabStrategies.cpp , 以及 QuickSoftwareTrigger.cpp.

### ￿ MV_CC_FeatureLoad()

MV_CAMCTRL_API int __stdcall MV_CC_FeatureLoad ( IN void * handle,

IN const char * strFileName

)

导入设备属性

参数

handle [IN] 设备句柄

strFileName [IN] 属性文件名 返回 成功，返回MV_OK ；失败，返回错误码

示例 ParametrizeCamera_LoadAndSave.cpp.

### ￿ MV_CC_FeatureLoadEx()

MV_CAMCTRL_API int __stdcall MV_CC_FeatureLoadEx ( IN void * handle,

IN const char * strFileName,

IN OUT MVCC_NODE_ERROR_LIST * pstNodeErrorList

)

导入设备属性并保存错误信息列表

参数

handle [IN] 设备句柄/采集卡句柄

strFileName [IN] 属性文件名

pstNodeErrorList [IN][OUT] 错误信息列表 返回 成功，返回MV_OK ；失败，返回错误码

注解
- 部分节点导入失败时，接口返回MV_OK ，通过错误信息列表中stNodeError获取出错 节点及失败原因

- pstNodeErrorList由用户在外部申请并由内部填充数据，该参数可接受填入null，代表 用户不关心导入时的错误信息

### ￿ MV_CC_FeatureSave()

MV_CAMCTRL_API int __stdcall MV_CC_FeatureSave ( IN void * handle,

IN const char * strFileName

)

保存设备属性

参数

handle [IN] 设备句柄

strFileName [IN] 属性文件名 返回 成功，返回MV_OK ；失败，返回错误码

示例 ParametrizeCamera_LoadAndSave.cpp.

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

函数

### 读写寄存器

参数设置

通过寄存器地址获取和设置相机寄存器数据 更多...

### 函数

MV_CAMCTRL_API int __stdcall MV_CC_ReadMemory (IN void *handle, IN OUT void *pBuffer, IN int64_t nAddress, IN int64_t nLength) 读内存 更多...

MV_CAMCTRL_API int __stdcall MV_CC_WriteMemory (IN void *handle, IN const void *pBuffer, IN int64_t nAddress, IN int64_t nLength) 写内存 更多...

### 详细描述

通过寄存器地址获取和设置相机寄存器数据

### 函数说明

### ￿ MV_CC_ReadMemory()

MV_CAMCTRL_API int __stdcall MV_CC_ReadMemory ( IN void * handle,

IN OUT void * pBuffer,

IN int64_t nAddress,

IN int64_t nLength

)

读内存

参数

handle [IN] 设备句柄

pBuffer [IN][OUT] 作为返回值使用，保存读到的内存值（GEV设备内存值是按照大端模 式存储的，其它协议设备按照小端存储）

nAddress [IN] 待读取的内存地址，该地址可以从设备的Camera.xml文件中获取，形如 xxx_RegAddr的xml节点值

nLength [IN] 待读取的内存长度 返回 成功，返回MV_OK；失败，返回错误码

注解 访问设备，读取某段寄存器的数据。

### ￿ MV_CC_WriteMemory()

MV_CAMCTRL_API int __stdcall MV_CC_WriteMemory ( IN void * handle,

IN const void * pBuffer,

IN int64_t nAddress,

IN int64_t nLength

)

写内存

参数

handle [IN] 设备句柄

pBuffer [IN] 待写入的内存值（注意GEV设备内存值要按照大端模式存储，其它协议设备 按照小端存储）

nAddress [IN] 待写入的内存地址，该地址可以从设备的Camera.xml文件中获取，形如 xxx_RegAddr的xml节点值

nLength [IN] 待写入的内存长度 返回 成功，返回MV_OK；失败，返回错误码

注解 访问设备，把一段数据写入某段寄存器。

示例 ForceIPEx.cpp.

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

函数

### 设备XML文件

参数设置

包含获取设备XML文件接口 更多...

### 函数

MV_CAMCTRL_API int __stdcall MV_CC_InvalidateNodes (IN void *handle) 清除GenICam节点缓存 更多...

MV_CAMCTRL_API int __stdcall MV_XML_GetGenICamXML (IN void *handle, IN OUT unsigned char *pData, IN unsigned int nDataSize, IN OUT unsigned int *pnDataLen) 获取设备属性树XML 更多...

MV_CAMCTRL_API int __stdcall MV_XML_GetNodeAccessMode (IN void *handle, IN const char *strName, IN OUT MV_XML_AccessMode *penAccessMode) 获得当前节点的访问模式 更多...

MV_CAMCTRL_API int __stdcall MV_XML_GetNodeInterfaceType (IN void *handle, IN const char *strName, IN OUT MV_XML_InterfaceType *penInterfaceType) 获得当前节点的类型 更多...

### 详细描述

包含获取设备XML文件接口

### 函数说明

### ￿ MV_CC_InvalidateNodes()

MV_CAMCTRL_API int __stdcall MV_CC_InvalidateNodes ( IN void * handle )

清除GenICam节点缓存

参数

handle [IN] 设备句柄 返回 成功，返回MV_OK；失败，返回错误码

注解 在加载工业相机节点时需要读取GenICam配置文件，该接口可以起到清除GenICam缓存 的功能。

### ￿ MV_XML_GetGenICamXML()

MV_CAMCTRL_API int __stdcall MV_XML_GetGenICamXML ( IN void * handle,

IN OUT unsigned char * pData,

IN unsigned int nDataSize,

IN OUT unsigned int * pnDataLen

)

获取设备属性树XML

参数

handle [IN] 设备句柄

pData [IN][OUT] XML数据接收缓存

nDataSize [IN] 接收缓存大小

pnDataLen [IN][OUT] 实际数据大小 返回 成功，返回MV_OK；失败，返回错误码

注解
- 当pData为NULL或nDataSize比实际的xml文件小时，不拷贝数据，由pnDataLen返回 xml文件大小。
- 当pData为有效缓存地址，且缓存足够大时，拷贝完整数据保存在该缓存里面，并由 pnDataLen返回xml文件实际大小。

### ￿ MV_XML_GetNodeAccessMode()

MV_CAMCTRL_API int __stdcall MV_XML_GetNodeAccessMode ( IN void * handle,

IN const char * strName,

IN OUT MV_XML_AccessMode * penAccessMode

)

获得当前节点的访问模式

参数

handle [IN] 设备句柄

strName [IN] 节点名称

penAccessMode [IN][OUT] 节点的访问模式 返回 成功，返回MV_OK；失败，返回错误码

示例 InterfaceDemo.cpp.

### ￿ MV_XML_GetNodeInterfaceType()

MV_CAMCTRL_API int __stdcall MV_XML_GetNodeInterfaceType ( IN void * handle,

IN const char * strName,

IN OUT MV_XML_InterfaceType * penInterfaceType

)

获得当前节点的类型

参数

handle [IN] 设备句柄

strName [IN] 节点名称

penInterfaceType [IN][OUT] 节点的类型 返回 成功，返回MV_OK；失败，返回错误码

注解 该接口可以在调用万能接口之前，提前知道节点类型，方便用户选择合适的万能接口进 行节点值的设置和获取。

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

函数

### 读写相机文件

参数设置

包含从设备读取文件、将文件写入设备等接口 更多...

### 函数

MV_CAMCTRL_API int __stdcall MV_CC_FileAccessRead (IN void *handle, IN MV_CC_FILE_ACCESS *pstFileAccess) 从设备读取文件 更多...

MV_CAMCTRL_API int __stdcall MV_CC_FileAccessReadEx (IN void *handle, IN OUT MV_CC_FILE_ACCESS_EX *pstFileAccessEx) 从设备读取文件（扩展接口，文件是Data数据） 更多...

MV_CAMCTRL_API int __stdcall MV_CC_FileAccessWrite (IN void *handle, IN MV_CC_FILE_ACCESS *pstFileAccess) 将文件写入设备 更多...

MV_CAMCTRL_API int __stdcall MV_CC_FileAccessWriteEx (IN void *handle, IN OUT MV_CC_FILE_ACCESS_EX *pstFileAccessEx) 将缓存（buffer）写入设备 更多...

MV_CAMCTRL_API int __stdcall MV_CC_GetFileAccessProgress (IN void *handle, IN OUT MV_CC_FILE_ACCESS_PROGRESS *pstFileAccessProgress) 获取文件存取的进度 更多...

### 详细描述

包含从设备读取文件、将文件写入设备等接口

### 函数说明

### ￿ MV_CC_FileAccessRead()

MV_CAMCTRL_API int __stdcall MV_CC_FileAccessRead ( IN void * handle,

IN MV_CC_FILE_ACCESS * pstFileAccess

)

从设备读取文件

参数

handle [IN] 设备句柄

pstFileAccess [IN] 文件存取结构体 返回 成功，返回MV_OK；失败，返回错误码

示例 ParametrizeCamera_FileAccess.cpp.

### ￿ MV_CC_FileAccessReadEx()

MV_CAMCTRL_API int __stdcall MV_CC_FileAccessReadEx ( IN void * handle,

IN OUT MV_CC_FILE_ACCESS_EX * pstFileAccessEx

)

从设备读取文件（扩展接口，文件是Data数据）

参数

handle [IN] 设备句柄

pstFileAccessEx [IN][OUT] 文件存取结构体 返回 成功，返回MV_OK ；失败，返回错误码

注解 该接口直接使用缓存数据，进行读写操作，避免直接操作文件出现无权限的问题。该接 口是 MV_CC_FileAccessRead() 的扩展接口。

### ￿ MV_CC_FileAccessWrite()

MV_CAMCTRL_API int __stdcall MV_CC_FileAccessWrite ( IN void * handle,

IN MV_CC_FILE_ACCESS * pstFileAccess

)

将文件写入设备

参数

handle [IN] 设备句柄

pstFileAccess [IN] 文件存取结构体 返回 成功，返回MV_OK；失败，返回错误码

示例 ParametrizeCamera_FileAccess.cpp.

### ￿ MV_CC_FileAccessWriteEx()

MV_CAMCTRL_API int __stdcall MV_CC_FileAccessWriteEx ( IN void * handle,

IN OUT MV_CC_FILE_ACCESS_EX * pstFileAccessEx

)

将缓存（buffer）写入设备

参数

handle [IN] 设备句柄

pstFileAccessEx [IN][OUT] 文件存取结构体 返回 成功，返回MV_OK；失败，返回错误码

注解 该接口直接使用缓存数据，进行写操作，避免直接将文件写入C盘，系统保护出现写失 败。该接口是 MV_CC_FileAccessWrite() 的扩展接口。

### ￿ MV_CC_GetFileAccessProgress()

MV_CAMCTRL_API int __stdcall MV_CC_GetFileAccessProgr ess ( IN void * handle,

IN OUTpstFileAccessProgre MV_CC_FILE_ACCESS_PROGRESS *ss

)

获取文件存取的进度

参数

handle [IN] 设备句柄

pstFileAccessProgress [IN][OUT] 进度内容 返回 成功，返回MV_OK；失败，返回错误码 （当前文件存取的状态）。

示例 ParametrizeCamera_FileAccess.cpp.

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

函数

### 图像采集

包含图像获取（回调取图、主动取图）、开始/结束取流接口等。 支持通过ISP工具生成的配置文件在取图接口内部对图像进行ISP算法处理，使用时需要先安装ISP 插件库，并在C盘个人目录下创建“ISPTool”文件夹（例如C:/Users/zhangsan/ISPTool），并将连接 相机生成的配置文件以相机型号或序列号命名的文件夹形式放置在ISPTool下。具体配置文件请联 系技术支持获取。 更多...

### 函数

MV_CAMCTRL_API int __stdcall MV_CC_RegisterImageCallBackEx (IN void *handle, IN void(__stdcall *cbOutput)(unsigned char *pData, MV_FRAME_OUT_INFO_EX *pFrameInfo, void *pUser), IN void *pUser) 注册图像数据回调 更多...

MV_CAMCTRL_API int __stdcall MV_CC_StartGrabbing (IN void *handle) 开始取流 更多...

MV_CAMCTRL_API int __stdcall MV_CC_StopGrabbing (IN void *handle) 停止取流 更多...

MV_CAMCTRL_API int __stdcall MV_CC_GetImageBuffer (IN void *handle, IN OUT MV_FRAME_OUT *pstFrame, IN unsigned int nMsec) 使用内部缓存获取一帧图片（与 MV_CC_Display() 不能同时 使用） 更多...

MV_CAMCTRL_API int __stdcall MV_CC_FreeImageBuffer (IN void *handle, IN MV_FRAME_OUT *pstFrame) 释放图像缓存（此接口用于释放不再使用的图像缓存，与 MV_CC_GetImageBuffer() 配套使用） 更多...

MV_CAMCTRL_API int __stdcall MV_CC_GetOneFrameTimeout (IN void *handle, IN OUT unsigned char *pData, IN unsigned int nDataSize, IN OUT

MV_FRAME_OUT_INFO_EX *pstFrameInfo, IN unsigned int nMsec) 采用超时机制获取一帧图片，SDK内部等待直到有数据时返回 更多...

MV_CAMCTRL_API int __stdcall MV_CC_ClearImageBuffer (IN void *handle) 清除取流数据缓存 更多...

MV_CAMCTRL_API int __stdcall MV_CC_GetValidImageNum (IN void *handle, IN OUT unsigned int *pnValidImageNum) 获取当前图像缓存区的有效图像个数 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SetImageNodeNum (IN void *handle, IN unsigned int nNum) 设置SDK内部图像缓存节点个数，大于等于1，在抓图前调用 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SetGrabStrategy (IN void *handle, IN MV_GRAB_STRATEGY enGrabStrategy) 设置取流策略 更多...

### 详细描述

包含图像获取（回调取图、主动取图）、开始/结束取流接口等。 支持通过ISP工具生成的配置文件在取图接口内部对图像进行ISP算法处理，使用时需要先安装ISP 插件库，并在C盘个人目录下创建“ISPTool”文件夹（例如C:/Users/zhangsan/ISPTool），并将连接 相机生成的配置文件以相机型号或序列号命名的文件夹形式放置在ISPTool下。具体配置文件请联 系技术支持获取。

### 函数说明

### ￿ MV_CC_RegisterImageCallBackEx()

MV_CAMCTRL_API int __stdcallIN MV_CC_RegisterImavoid geCallBackEx (* handle,

void__stdcall *cbOutput)(unsigned char *pData, INMV_FRAME_OUT_INFO_EX *pFrameInfo, void *pUser,

IN void
* pUser

)

注册图像数据回调

参数

handle [IN] 设备句柄

cbOutput [IN] 回调函数指针

pUser [IN] 用户自定义变量 返回 成功，返回MV_OK；失败，返回错误码

注解
- 通过该接口可以设置图像数据回调函数，在 MV_CC_CreateHandle() 之后即可调用 。
- 图像数据采集有两种方式，两种方式不能复用：

1. 调用 MV_CC_RegisterImageCallBackEx() 设置图像数据回调函数，然后调用 MV_CC_StartGrabbing() 开始采集，采集的图像数据在设置的回调函数中返回 。
2. 调用 MV_CC_StartGrabbing() 开始采集，然后在应用层循环调用 MV_CC_GetOneFrameTimeout() 获取指定像素格式的帧数据，获取帧数据时 上层应用程序需要根据帧率控制好调用该接口的频率。

- 该接口不支持 MV_CAMERALINK_DEVICE() 的设备。

示例 ChunkData.cpp, Events_Interface.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, InterfaceAndDeviceDemo.cpp, LensShadingCorrection.cpp , 以及 ParametrizeCamera_LineScanIOSettings.cpp.

### ￿ MV_CC_StartGrabbing()

MV_CAMCTRL_API int __stdcall MV_CC_StartGrabbing ( IN void * handle )

开始取流

参数

handle [IN] 设备句柄 返回 成功，返回MV_OK；失败，返回错误码

注解 该接口不支持CameraLink设备。

示例 ChunkData.cpp, ConnectSpecCamera.cpp, ConvertPixelType.cpp, Events.cpp, Events_Interface.cpp, Grab_ActionCommand.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, InterfaceAndDeviceDemo.cpp, LensShadingCorrection.cpp, MultiCast.cpp, MultiLightCtrl_ImageStitching.cpp, ParametrizeCamera_LineScanIOSettings.cpp, QuickSoftwareTrigger.cpp , 以及 Recording.cpp.

### ￿ MV_CC_StopGrabbing()

MV_CAMCTRL_API int __stdcall MV_CC_StopGrabbing ( IN void * handle )

停止取流

参数

handle [IN] 设备句柄 返回 成功，返回MV_OK；失败，返回错误码

注解 该接口不支持CameraLink设备。

示例

ChunkData.cpp, ConnectSpecCamera.cpp, ConvertPixelType.cpp, Events.cpp, Events_Interface.cpp, Grab_ActionCommand.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, InterfaceAndDeviceDemo.cpp, LensShadingCorrection.cpp, MultiCast.cpp, MultiLightCtrl_ImageStitching.cpp, ParametrizeCamera_LineScanIOSettings.cpp, QuickSoftwareTrigger.cpp , 以及 Recording.cpp.

### ￿ MV_CC_GetImageBuffer()

MV_CAMCTRL_API int __stdcall MV_CC_GetImageBuffer ( IN void * handle,

IN OUT MV_FRAME_OUT * pstFrame,

IN unsigned int nMsec

)

使用内部缓存获取一帧图片（与 MV_CC_Display() 不能同时使用）

参数

handle [IN] 设备句柄

pstFrame [IN][OUT] 图像数据和图像信息

nMsec [IN] 等待超时时间，输入INFINITE时表示无限等待，直到收到一帧数据或者停止 取流

返回 成功，返回MV_OK；失败，返回错误码

注解
- 调用该接口获取图像数据帧之前需要先调用 MV_CC_StartGrabbing() 启动图像采集 。该接口为主动式获取帧数据，上层应用程序需要根据帧率，控制好调用该接口的频率 。该接口支持设置超时时间，SDK内部等待直到有数据时返回，可以增加取流平稳性， 适合用于对平稳性要求较高的场合。
- 该接口与 MV_CC_FreeImageBuffer() 配套使用，当处理完取到的数据后，需要用 MV_CC_FreeImageBuffer() 接口将pstFrame内的数据指针权限进行释放。
- 该接口与 MV_CC_GetOneFrameTimeout() 相比，有着更高的效率。且其取流缓存 的分配是由sdk内部自动分配的，而 MV_CC_GetOneFrameTimeout() 接口是需要客户 自行分配。
- 该接口在调用 MV_CC_Display() 后无法取流。

- 该接口对于U3V、GIGE设备均可支持。
- 该接口不支持CameraLink设备。

示例 ConnectSpecCamera.cpp, ConvertPixelType.cpp, Grab_ActionCommand.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, MultiCast.cpp, MultiLightCtrl_ImageStitching.cpp, QuickSoftwareTrigger.cpp , 以及 Recording.cpp.

### ￿ MV_CC_FreeImageBuffer()

MV_CAMCTRL_API int __stdcall MV_CC_FreeImageBuffer ( IN void * handle,

IN MV_FRAME_OUT * pstFrame

)

释放图像缓存（此接口用于释放不再使用的图像缓存，与 MV_CC_GetImageBuffer() 配套使 用）

参数

handle [IN] 设备句柄

pstFrame [IN] 图像数据和图像数据 返回 成功，返回MV_OK；失败，返回错误码

注解
- 该接口与 MV_CC_GetImageBuffer() 配套使用，使用 MV_CC_GetImageBuffer() 接 口取到的图像数据pstFrame，需要用 MV_CC_FreeImageBuffer() 接口进行权限释放。
- 该接口对于取流效率高于 MV_CC_GetOneFrameTimeout() 接口，且 MV_CC_GetImageBuffer() 在不进行Free的情况下，最大支持输出的节点数与 SetImageNode接口所设置的节点数相等，默认节点数是1。
- 该接口对于U3V、GIGE设备均可支持。
- 该接口不支持CameraLink设备。

示例 ConnectSpecCamera.cpp, ConvertPixelType.cpp, Grab_ActionCommand.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, MultiCast.cpp,

MultiLightCtrl_ImageStitching.cpp, QuickSoftwareTrigger.cpp , 以及 Recording.cpp.

### ￿ MV_CC_GetOneFrameTimeout()

MV_CAMCTRL_API int __stdcall MV_CC_GetOneFrameTimeout ( IN void * handle,

IN OUT unsigned char * pData,

IN unsigned int nDataSize,

IN OUT MV_FRAME_OUT_INFO_EX * pstFrameInfo,

IN unsigned int nMsec

)

采用超时机制获取一帧图片，SDK内部等待直到有数据时返回

参数

handle [IN] 设备句柄

pData [IN][OUT] 图像数据接收指针

nDataSize [IN] 接收缓存大小

pstFrameInfo [IN][OUT] 图像信息结构体

nMsec [IN] 等待超时时间 返回 成功，返回MV_OK；失败，返回错误码

注解
- 调用该接口获取图像数据帧之前需要先调用 MV_CC_StartGrabbing() 启动图像采集 。该接口为主动式获取帧数据，上层应用程序需要根据帧率，控制好调用该接口的频率 。该接口支持设置超时时间，SDK内部等待直到有数据时返回，可以增加取流平稳性， 适合用于对平稳性要求较高的场合。
- 该接口对于U3V、GIGE设备均可支持。
- 该接口不支持CameraLink设备。

### ￿ MV_CC_ClearImageBuffer()

MV_CAMCTRL_API int __stdcall MV_CC_ClearImageBuffer ( IN void * handle )

清除取流数据缓存

参数

handle [IN] 设备句柄 返回 成功，返回MV_OK；失败，返回错误码

注解
- 该接口允许用户在不停止取流的时候，就能清除缓存中不需要的图像。
- 该接口在连续模式切触发模式后，可以清除历史数据。
- 该接口当前仅支持清除SDK内部的图像缓存，暂不支持清除采集卡内的缓存。

### ￿ MV_CC_GetValidImageNum()

MV_CAMCTRL_API int __stdcall MV_CC_GetValidImageNum ( IN void * handle,

IN OUT unsigned int * pnValidImageNum

)

获取当前图像缓存区的有效图像个数

参数

handle [IN] 设备句柄

pnValidImageNum [IN][OUT] 当前图像缓存区中有效图像个数的指针 返回 成功，返回MV_OK；错误，返回错误码

注解 该接口只统计SDK内部的有效图像个数，不包括采集卡缓存内的有效图像个数

### ￿ MV_CC_SetImageNodeNum()

MV_CAMCTRL_API int __stdcall MV_CC_SetImageNodeNum ( IN void * handle,

IN unsigned int nNum

)

设置SDK内部图像缓存节点个数，大于等于1，在抓图前调用

参数

handle [IN] 设备句柄

nNum [IN] 缓存节点个数 返回 成功，返回MV_OK；失败，返回错误码

注解
- 调用该接口可以设置SDK内部图像缓存节点个数，在调用 MV_CC_StartGrabbing() 开始抓图前调用。
- 在SDK中默认是1个节点。
- 该接口不支持CameraLink设备。CameraLink设备可以通过GenTL方式连接并设置缓 存节点个数。

示例 GrabStrategies.cpp.

### ￿ MV_CC_SetGrabStrategy()

MV_CAMCTRL_API int __stdcall MV_CC_SetGrabStrategy ( IN void * handle,

IN MV_GRAB_STRATEGY enGrabStrategy

)

设置取流策略

参数

handle [IN] 设备句柄

enGrabStrategy [IN] 策略枚举值 返回 成功，返回MV_OK；失败，返回 错误码

注解 该接口定义了四种取流策略，用户可以根据实际需求进行选择。具体描述如下：

- OneByOne：从旧到新一帧一帧的从输出缓存列表中获取图像，打开设备后默认 为该策略
- LatestImagesOnly：仅从输出缓存列表中获取最新的一帧图像，同时清空输出缓 存列表
- LatestImages：从输出缓存列表中获取最新的OutputQueueSize帧图像，其中 OutputQueueSize范围为1-ImageNodeNum，可用 MV_CC_SetOutputQueueSize() 接口设置，ImageNodeNum默认为1，可用

MV_CC_SetImageNodeNum() 接口设置 OutputQueueSize设置成1等同于 LatestImagesOnly策略，OutputQueueSize设置成ImageNodeNum等同于 OneByOne策略
- UpcomingImage：在调用取流接口时忽略输出缓存列表中所有图像，并等待设 备即将生成的一帧图像。该策略只支持GigE设备，不支持USB设备

- 该接口仅支持GigE和USB设备。

示例 GrabStrategies.cpp.

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

函数

### 图像渲染

包含显示一帧图像接口 更多...

### 函数

MV_CAMCTRL_API int __stdcall MV_CC_DisplayOneFrameEx (IN void *handle, IN void *hWnd, IN MV_DISPLAY_FRAME_INFO_EX *pstDisplayInfo)

显示一帧图像（扩展接口） 更多...

MV_CAMCTRL_API int __stdcall MV_CC_DisplayOneFrameEx2 (IN void *handle, IN void *hWnd, IN MV_CC_IMAGE *pstImage, unsigned int enRenderMode) 显示一帧图像（扩展接口2） 更多...

### 详细描述

包含显示一帧图像接口

### 函数说明

### ￿ MV_CC_DisplayOneFrameEx()

MV_CAMCTRL_API int __stdcall MV_CC_DisplayOneFrameEx ( IN void * handle,

IN void * hWnd,

IN MV_DISPLAY_FRAME_INFO_EX * pstDisplayInfo

)

显示一帧图像（扩展接口）

参数

handle [IN] 设备句柄

hWnd [IN] 窗口句柄

pstDisplayInfo [IN] 图像信息 返回 成功，返回MV_OK；失败，返回错误码

注解
- 渲染方式可选择GDI或D3D，默认选择为GDI模式。

- GDI模式对电脑的显卡性能没有要求，适用于所有电脑

- D3D模式适用于安装显卡驱动且显卡内存大于1GB的电脑，该模式下客户端预览 的图像效果会优于GDI模式下的图像效果
- 渲染模式为D3D时，支持的最大分辨率为16384 * 163840

### ￿ MV_CC_DisplayOneFrameEx2()

MV_CAMCTRL_API int __stdcall MV_CC_DisplayOneFrameEx2 ( IN void * handle,

IN void * hWnd,

IN MV_CC_IMAGE * pstImage,

unsigned int enRenderMode

)

显示一帧图像（扩展接口2）

参数

handle [IN] 设备句柄

hWnd [IN] 窗口句柄

pstImage [IN] 图像信息

enRenderMode [IN] 渲染方式：0-GDI 1-D3D 2-OpenGL 返回 成功，返回MV_OK；失败，返回错误码

注解
- 此接口支持4G以上超大图渲染，调用时需要输入 MV_CC_IMAGE 结构体中 nImageBufLen的值。
- D3D和OpenGL模式适用于安装显卡驱动且显卡内存大于1GB的电脑，该模式下客户 端预览的图像效果会优于GDI模式下的图像效果。
- 根据图像大小是否超过4G，该接口可选的渲染方式和支持渲染宽高不同，详情如下：

- 若图像大小超过4G：仅支持通过OpenGL方式渲染。可渲染RGB8_Packed， BGR8_Packed和Mono8格式的图像。
- 对于其他图像，支持渲染宽高至int类型的RGB8_Packed，BGR8_Packed， Mono8像素格式图像。支持渲染宽高至short类型的其余像素格式图像。

示例 GrabImage_Display.cpp.

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

函数

### 图像处理

包括图像显示、图像保存和图像格式转换接口等 更多...

### 函数

MV_CAMCTRL_API int __stdcall MV_CC_SaveImageEx3 (IN void *handle, IN OUT MV_SAVE_IMAGE_PARAM_EX3 *pstSaveParam) 保存图片，支持Bmp和Jpeg 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SaveImageToFileEx (IN void *handle, IN OUT MV_SAVE_IMAGE_TO_FILE_PARAM_EX *pstSaveFileParam) 保存图像到文件 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SaveImageToFileEx2 (IN void *handle, IN MV_CC_IMAGE *pstImage, IN

MV_CC_SAVE_IMAGE_PARAM *pSaveImageParam, IN const char *pcImagePath) 保存图像到文件 更多...

MV_CAMCTRL_API int __stdcall MV_CC_RotateImage (IN void *handle, IN OUT MV_CC_ROTATE_IMAGE_PARAM *pstRotateParam) 图像旋转 更多...

MV_CAMCTRL_API int __stdcall MV_CC_FlipImage (IN void *handle, IN OUT MV_CC_FLIP_IMAGE_PARAM *pstFlipParam) 图像翻转 更多...

MV_CAMCTRL_API int __stdcall MV_CC_ConvertPixelTypeEx (IN void *handle, IN OUT MV_CC_PIXEL_CONVERT_PARAM_EX *pstCvtParam) 像素格式转换 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SetBayerCvtQuality (IN void *handle, IN unsigned int nBayerCvtQuality) 设置插值算法类型 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SetBayerFilterEnable (IN void *handle, IN bool bFilterEnable) 插值算法平滑使能设置 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SetBayerGammaValue (IN void *handle, IN float fBayerGammaValue) 设置Bayer格式的Gamma值 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SetGammaValue (IN void *handle, IN enum MvGvspPixelType enSrcPixelType, IN float fGammaValue) 设置Mono8/Bayer8/10/12/16格式的Gamma值 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SetBayerGammaParam (IN void *handle, IN MV_CC_GAMMA_PARAM *pstGammaParam) 设置Bayer格式的Gamma信息 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SetBayerCCMParam (IN void *handle, IN MV_CC_CCM_PARAM *pstCCMParam) 设置Bayer格式的CCM使能和矩阵，量化系数默认1024 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SetBayerCCMParamEx (IN void *handle, IN MV_CC_CCM_PARAM_EX *pstCCMParam) 设置Bayer格式的CCM使能和矩阵 更多...

MV_CAMCTRL_API int __stdcall MV_CC_ImageContrast (IN void *handle, IN OUT MV_CC_CONTRAST_PARAM *pstContrastParam) 图像对比度调节 更多...

MV_CAMCTRL_API int __stdcall MV_CC_HB_Decode (IN void *handle, IN OUT MV_CC_HB_DECODE_PARAM *pstDecodeParam) 无损解码 更多...

MV_CAMCTRL_API int __stdcall MV_CC_DrawRect (IN void *handle, IN MVCC_RECT_INFO *pRectInfo) 在图像上绘制矩形框辅助线 更多...

MV_CAMCTRL_API int __stdcall MV_CC_StartRecord (IN void *handle, IN MV_CC_RECORD_PARAM *pstRecordParam) 开始录像 更多...

MV_CAMCTRL_API int __stdcall MV_CC_InputOneFrame (IN void *handle, IN MV_CC_INPUT_FRAME_INFO *pstInputFrameInfo) 输入录像数据 更多...

MV_CAMCTRL_API int __stdcall MV_CC_StopRecord (IN void *handle) 停止录像 更多...

MV_CAMCTRL_API int __stdcall MV_CC_ReconstructImage (IN void *handle, IN OUT MV_RECONSTRUCT_IMAGE_PARAM *pstReconstructParam) 重构图像（用于分时曝光功能） 更多...

### 详细描述

包括图像显示、图像保存和图像格式转换接口等

### 函数说明

### ￿ MV_CC_SaveImageEx3()

MV_CAMCTRL_API int __stdcall MV_CC_SaveImageEx3 ( IN void * handle,

IN OUT MV_SAVE_IMAGE_PARAM_EX3 * pstSaveParam

)

保存图片，支持Bmp和Jpeg

参数

handle [IN] 设备句柄

pstSaveParam [IN][OUT] 保存图片参数结构体 返回 成功，返回MV_OK；失败，返回错误码

注解
- 通过该接口可以将从设备采集到的原始图像数据转换成JPEG或者BMP等格式并存放 在指定内存中，然后用户可以将转换之后的数据直接保存成图片文件。
- 该接口调用无接口顺序要求，有图像源数据就可以进行转换，可以先调用 MV_CC_GetOneFrameTimeout() 或者 MV_CC_RegisterImageCallBackEx() 设置回 调函数，获取一帧图像数据，然后再通过该接口转换格式。
- 该接口支持长乘宽至UINT_MAX，其中 MV_CC_SaveImageEx2() 支持长乘宽最大至 USHRT_MAX，JPEG格式最大支持宽高为65500。

### ￿ MV_CC_SaveImageToFileEx()

MV_CAMCTRL_API int __stdcall MV_CC_SaveImageToFile Ex ( IN void * handle,

IN OUTpstSaveFilePara MV_SAVE_IMAGE_TO_FILE_PARAM_EX *m

)

保存图像到文件

参数

handle [IN] 设备句柄

pstSaveFileParam [IN][OUT] 保存图片文件参数结构体 返回 成功，返回MV_OK；失败，返回错误码

注解
- 该接口支持BMP/JPEG/PNG/TIFF。
- 该接口支持保存的图像长乘宽至UINT_MAX， MV_CC_SaveImageToFile() 支持长 乘宽最大至USHRT_MAX。JPEG格式最大支持宽高为65500。
- 该接口是 MV_CC_SaveImageToFile() 接口的扩展接口。

### ￿ MV_CC_SaveImageToFileEx2()

MV_CAMCTRL_API int __stdcall MV_CC_SaveImageToFileEx2 ( IN void * handle,

IN MV_CC_IMAGE * pstImage,

IN MV_CC_SAVE_IMAGE_PARAM * pSaveImageParam,

IN const char * pcImagePath

)

保存图像到文件

参数

handle [IN] 设备句柄

pstImage [IN] 图像信息

pSaveImageParam [IN] 存图参数

pcImagePath [IN] 存图路径 返回 成功，返回MV_OK；失败，返回错误码

注解
- 该接口支持4G以上超大图的PNG/TIFF存图，其他非超大图像支持 BMP/JPEG/TIFF/PNG。
- JPEG格式最大支持宽高为65500。

示例

HighBandwidthDecode.cpp, ImageSave.cpp , 以及 MultiLightCtrl_ImageStitching.cpp.

### ￿ MV_CC_RotateImage()

MV_CAMCTRL_API int __stdcall MV_CC_RotateImage ( IN void * handle,

IN OUT MV_CC_ROTATE_IMAGE_PARAM * pstRotateParam

)

图像旋转

参数

handle [IN] 设备句柄

pstRotateParam [IN][OUT] 图像旋转参数结构体 返回 成功，返回MV_OK ；失败，返回错误码

注解 该接口只支持MONO8/RGB24/BGR24格式数据的90/180/270度旋转。

### ￿ MV_CC_FlipImage()

MV_CAMCTRL_API int __stdcall MV_CC_FlipImage ( IN void * handle,

IN OUT MV_CC_FLIP_IMAGE_PARAM * pstFlipParam

)

图像翻转

参数

handle [IN] 设备句柄

pstFlipParam [IN][OUT] 图像翻转结构体 返回 成功，返回MV_OK ；失败，返回错误码

注解

该接口只支持MONO8/RGB24/BGR24格式数据的垂直和水平翻转。

### ￿ MV_CC_ConvertPixelTypeEx()

MV_CAMCTRL_API int __stdcall MV_CC_ConvertPixelTypeE x ( IN void * handle,

IN OUT MV_CC_PIXEL_CONVERT_PARAM_EXpstCvtParam *

)

像素格式转换

参数

handle [IN] 设备句柄

pstCvtParam [IN][OUT] 像素格式转换参数结构体 返回 成功，返回MV_OK；失败，返回错误码

注解
- 通过该接口可以将从设备采集到的原始图像数据转换成用户所需的像素格式并存放在 指定内存中。
- 该接口调用无接口顺序要求，有图像源数据就可以进行转换，可以先调用 MV_CC_GetOneFrameTimeout() 或者 MV_CC_RegisterImageCallBackEx() 设置回 调函数，获取一帧图像数据，然后再通过该接口转换格式。如果设备当前采集图像是 JPEG压缩的格式，则不支持调用该接口进行转换。
- 该接口支持转换的图像长乘宽至UINT_MAX。
- 像素格式转换能力的详情，如下表所示。第一列为输入像素格式，第一行为转换后的 像素格式。“ √ ”代表可以转换为目标像素格式。 输入格式\输出格YUV422 Mono8 RGB24 BGR24 YUV422 YV12 式 YUYV

Mono8 × √ √ √ √ ×

Mono10 √ √ √ √ √ ×

Mono10P √ √ √ √ √ ×

Mono12 √ √ √ √ √ ×

Mono12P √ √ √ √ √ ×

BayerGR8 √ √ √ √ √ ×

BayerRG8 √ √ √ √ √ ×

BayerGB8 √ √ √ √ √ ×

BayerBG8 √ √ √ √ √ ×

BayerRBGG8 × √ √ × × ×

BayerGR10 √ √ √ √ √ ×

BayerRG10 √ √ √ √ √ ×

BayerGB10 √ √ √ √ √ ×

BayerBG10 √ √ √ √ √ ×

BayerGR12 √ √ √ √ √ ×

BayerRG12 √ √ √ √ √ ×

BayerGB12 √ √ √ √ √ ×

BayerBG12 √ √ √ √ √ ×

BayerGR10P √ √ √ √ √ ×

BayerRG10P √ √ √ √ √ ×

BayerGB10P √ √ √ √ √ ×

BayerBG10P √ √ √ √ √ ×

BayerGR12P √ √ √ √ √ ×

BayerRG12P √ √ √ √ √ ×

BayerGB12P √ √ √ √ √ ×

BayerBG12P √ √ √ √ √ ×

RGB8P √ × √ √ √ ×

BGR8P √ √ × √ √ ×

YUV422P √ √ √ × √ ×

YUV422 YUYV √ √ √ √ √ × 示例 ConvertPixelType.cpp.

### ￿ MV_CC_SetBayerCvtQuality()

MV_CAMCTRL_API int __stdcall MV_CC_SetBayerCvtQuality ( IN void * handle,

IN unsigned int nBayerCvtQuality

)

设置插值算法类型

参数

handle [IN] 设备句柄

BayerCvtQuality [IN] Bayer的插值方法 0：快速 1：均衡 2：最优（默认为最优） 3：最优 + 返回 成功，返回MV_OK ；失败，返回错误码

注解 设置内部图像转换接口的Bayer插值算法类型参数， MV_CC_ConvertPixelTypeEx() 、 MV_CC_GetImageForRGB() 和 MV_CC_GetImageForBGR() 接口内部使用的插值算 法是该接口所设定的。

示例 ConvertPixelType.cpp.

### ￿ MV_CC_SetBayerFilterEnable()

MV_CAMCTRL_API int __stdcall MV_CC_SetBayerFilterEnable ( IN void * handle,

IN bool bFilterEnable

)

插值算法平滑使能设置

参数

handle [IN] 设备句柄

bFilterEnable [IN] 平滑使能（默认关闭） 返回 成功，返回MV_OK ；失败，返回错误码

注解 设置内部图像转换接口的贝尔插值平滑使能参数， MV_CC_ConvertPixelTypeEx() 、 MV_CC_SaveImageEx3() 接口内部使用的插值算法是该接口所设定的。

### ￿ MV_CC_SetBayerGammaValue()

MV_CAMCTRL_API int __stdcall MV_CC_SetBayerGammaValue ( IN void * handle,

IN float fBayerGammaValue

)

设置Bayer格式的Gamma值

参数

handle [IN] 设备句柄

fBayerGammaValue [IN] Gamma值：0.1 ~ 4.0 返回 成功，返回MV_OK ；失败，返回错误码

注解 设置该值后，在调用 MV_CC_ConvertPixelTypeEx() 、 MV_CC_SaveImageEx3() 、 MV_CC_SaveImageToFile() 接口将Bayer8/10/12/16格式转成RGB24/48， RGBA32/64，BGR24/48，BGRA32/64时起效。

### ￿ MV_CC_SetGammaValue()

MV_CAMCTRL_API int __stdcall MV_CC_SetGammaValue ( IN void * handle,

IN enum MvGvspPixelType enSrcPixelType,

IN float fGammaValue

)

设置Mono8/Bayer8/10/12/16格式的Gamma值

参数

handle [IN] 设备句柄

MvGvspPixelType enSrcPixelType [IN] 像素格式,支持 PixelType_Gvsp_Mono8,Bayer8/10/12/16

fGammaValue [IN] Gamma值:0.1 ~ 4.0 返回 成功，返回MV_OK ；失败，返回错误码

注解
- 设置Mono8的gamma值后，在调用 MV_CC_ConvertPixelType() 接口将Mono8转成 Mono8时gamma值起效。
- 设置Bayer8/10/12/16的gamma值后，在调用 MV_CC_ConvertPixelType() 、 MV_CC_SaveImageToFile() 、 MV_CC_SaveImageEx3() 接口将Bayer8/10/12/16转 RGB24/48， RGBA32/64，BGR24/48，BGRA32/64时gamma值起效。
- 该接口兼容 MV_CC_SetBayerGammaValue() 接口，新增支持Mono8像素格式。

### ￿ MV_CC_SetBayerGammaParam()

MV_CAMCTRL_API int __stdcall MV_CC_SetBayerGammaParam ( IN void * handle,

IN MV_CC_GAMMA_PARAM * pstGammaParam

)

设置Bayer格式的Gamma信息

参数

handle [IN] 设备句柄

pstGammaParam [IN] Gamma信息 返回 成功，返回MV_OK ；错误，返回错误码

注解
- 设置该信息后，在调用 MV_CC_ConvertPixelTypeEx() 、 MV_CC_SaveImageEx3() 接口将Bayer8/10/12/16格式转成RGB24/48， RGBA32/64 ，BGR24/48，BGRA32/64时起效。
- 以相机实际支持为准。

### ￿ MV_CC_SetBayerCCMParam()

MV_CAMCTRL_API int __stdcall MV_CC_SetBayerCCMParam ( IN void * handle,

IN MV_CC_CCM_PARAM * pstCCMParam

)

设置Bayer格式的CCM使能和矩阵，量化系数默认1024

参数

handle [IN] 设备句柄

pstCCMParam [IN] CCM参数 返回 成功，返回MV_OK ；错误，返回错误码

注解 开启CCM并设置CCM矩阵后，在调用 MV_CC_ConvertPixelTypeEx() 、 MV_CC_SaveImageEx3() 接口将Bayer8/10/12/16格式转成RGB24/48， RGBA32/64 ，BGR24/48，BGRA32/64时起效。

### ￿ MV_CC_SetBayerCCMParamEx()

MV_CAMCTRL_API int __stdcall MV_CC_SetBayerCCMParamEx ( IN void * handle,

IN MV_CC_CCM_PARAM_EX * pstCCMParam

)

设置Bayer格式的CCM使能和矩阵

参数

handle [IN] 设备句柄

pstCCMParam [IN] CCM参数 返回 成功，返回MV_OK ；错误，返回错误码

注解 开启CCM并设置CCM矩阵后，在调用 MV_CC_ConvertPixelTypeEx() 、 MV_CC_SaveImageEx3() 接口将Bayer8/10/12/16格式转成RGB24/48， RGBA32/64 ，BGR24/48，BGRA32/64时起效。

### ￿ MV_CC_ImageContrast()

MV_CAMCTRL_API int __stdcall MV_CC_ImageContrast ( IN void * handle,

IN OUT MV_CC_CONTRAST_PARAM * pstContrastParam

)

图像对比度调节

参数

handle [IN] 设备句柄

pstContrastParam [IN][OUT] 对比度调节参数 返回 成功，返回MV_OK ；错误，返回错误码

示例 ImageContrast.cpp.

### ￿ MV_CC_HB_Decode()

MV_CAMCTRL_API int __stdcall MV_CC_HB_Decode ( IN void * handle,

IN OUT MV_CC_HB_DECODE_PARAM * pstDecodeParam

)

无损解码

参数

handle [IN] 设备句柄

pstDecodeParam [IN][OUT] 无损解码参数结构体 返回 成功，返回MV_OK ；失败，返回错误码

注解
- 将从相机中取到的无损压缩码流解码成裸数据，同时支持解析当前相机实时图像的水 印信息（如果输入的无损码流不是当前相机或者不是实时取流的，则水印解析可能异常 ）；
- 若解码失败，请依次检查以下情况：

1. CPU是否支持 SSE AVX指令集。
2. 当前帧是否异常（丢包等），若存在，可能导致解码异常。
3. 相机出图是否异常，即使不丢包也会异常。

示例 HighBandwidthDecode.cpp, ImageSave.cpp , 以及 MultiLightCtrl_ImageStitching.cpp.

### ￿ MV_CC_DrawRect()

MV_CAMCTRL_API int __stdcall MV_CC_DrawRect ( IN void * handle,

IN MVCC_RECT_INFO * pRectInfo

)

在图像上绘制矩形框辅助线

参数

handle [IN] 设备句柄

pRectInfo [IN] 矩形辅助线的信息 返回 成功，返回MV_OK ；失败，返回错误码

### ￿ MV_CC_StartRecord()

MV_CAMCTRL_API int __stdcall MV_CC_StartRecord ( IN void * handle,

IN MV_CC_RECORD_PARAM * pstRecordParam

)

开始录像

参数

handle [IN] 设备句柄

pstRecordParam [IN] 录像参数结构体 返回 成功，返回MV_OK ；失败，返回错误码

注解 该接口最大支持Width*Height为8000*8000大小。若超出，会导致调用 MV_CC_InputOneFrame() 接口错误。

示例 Recording.cpp.

### ￿ MV_CC_InputOneFrame()

MV_CAMCTRL_API int __stdcall MV_CC_InputOneFrame ( IN void * handle,

IN MV_CC_INPUT_FRAME_INFO * pstInputFrameInfo

)

输入录像数据

参数

handle [IN] 设备句柄

pstInputFrameInfo [IN] 录像数据结构体 返回 成功，返回MV_OK ；失败，返回错误码

示例 Recording.cpp.

### ￿ MV_CC_StopRecord()

MV_CAMCTRL_API int __stdcall MV_CC_StopRecord ( IN void * handle )

停止录像

参数

handle [IN] 设备句柄 返回 成功，返回MV_OK ；失败，返回错误码

示例 Recording.cpp.

### ￿ MV_CC_ReconstructImage()

MV_CAMCTRL_API int __stdcall MV_CC_ReconstructIma ge ( IN void * handle,

IN OUTpstReconstructPara MV_RECONSTRUCT_IMAGE_PARAM *m

)

重构图像（用于分时曝光功能）

参数

handle [IN] 设备句柄

pstRestruParam [IN][OUT] 重构图像参数 返回 成功，返回MV_OK ；失败，返回错误码

注解 图像分割功能可将多个不同曝光值所对应的图像交叠合并为1张图像。 使用时，需与线阵相机的“MultiLightControl”节点搭配。假设设置该节点为2，则相机会 将2个不同曝光值所对应的两张图像交叠合并为1张图像（实际高度为2张图像的高度） 发送给上层应用程序。 此时，调用该接口并传入分时曝光值nExposureNum为2，可将相机发送的1张图像分割 为2张图像，并且这2张图像分别对应1个曝光值。 若使用普通相机或未打开线阵相机的“MultiLightControl”节点，且nExposureNum设置为 n，则图像分割无意义，只是将图像按行分割为n张图像，每张图像的高度变为原图像的 1/n。

示例 MultiLightCtrl_ImageStitching.cpp.

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

函数

### 事件及异常

包括事件及异常接口 更多...

### 函数

MV_CAMCTRL_API int __stdcall MV_CC_RegisterAllEventCallBack (IN void *handle, IN void(__stdcall *cbEvent)(MV_EVENT_OUT_INFO *pEventInfo, void *pUser), IN void *pUser) 注册全部事件回调，在打开设备之后调用 更多...

MV_CAMCTRL_API int __stdcall MV_CC_RegisterEventCallBackEx (IN void *handle, IN const char *strEventName, IN void(__stdcall *cbEvent)(MV_EVENT_OUT_INFO *pEventInfo, void *pUser), IN void *pUser) 注册单个事件回调，在打开设备之后调用 更多...

MV_CAMCTRL_API int __stdcall MV_CC_EventNotificationOn (IN void *handle, IN const char *strEventName) 开启设备指定事件 更多...

MV_CAMCTRL_API int __stdcall MV_CC_EventNotificationOff (IN void *handle, IN const char *strEventName) 关闭设备指定事件 更多...

MV_CAMCTRL_API int __stdcall MV_CC_RegisterExceptionCallBack (IN void *handle, IN void(__stdcall *cbException)(unsigned int nMsgType, void *pUser), IN void *pUser) 注册异常消息回调，在打开设备之后调用 更多...

### 详细描述

包括事件及异常接口

### 函数说明

### ￿ MV_CC_RegisterAllEventCallBack()

MV_CAMCTRL_API int __stdcall MV_CC_RegisterAllEventCIN void allBack (* handle,

void__stdcall *cbEvent)(MV_EVENT_OUT_INFO IN*pEventInfo, void *pUser,

IN void
* pUser

)

注册全部事件回调，在打开设备之后调用

参数

handle [IN] 设备句柄

cbEvent [IN] 事件回调函数指针

pUser [IN] 用户自定义变量 返回 成功，返回MV_OK；失败，返回错误码

注解
- 通过该接口设置事件回调，可以在回调函数里面获取采集、曝光等事件信息。
- 该接口不支持CameraLink设备。

### ￿ MV_CC_RegisterEventCallBackEx()

MV_CAMCTRL_API int __stdcall MV_CC_RegisterEventCa llBackEx ( IN void * handle,

IN const char * strEventName,

void__stdcall *cbEvent)(MV_EVENT_OUT_INFO IN*pEventInfo, void *pUser,

IN void * pUser

)

注册单个事件回调，在打开设备之后调用

参数

handle [IN] 设备句柄

strEventName [IN] 事件名称

cbEvent [IN] 事件回调函数指针

pUser [IN] 用户自定义变量 返回 成功，返回MV_OK；失败，返回错误码

注解
- 通过该接口设置事件回调，可以在回调函数里面获取采集、曝光等事件信息。
- 该接口不支持CameraLink设备。

示例 Events.cpp , 以及 Events_Interface.cpp.

### ￿ MV_CC_EventNotificationOn()

MV_CAMCTRL_API int __stdcall MV_CC_EventNotificationOn ( IN void * handle,

IN const char * strEventName

)

开启设备指定事件

参数

handle [IN] 设备句柄

strEventName [IN] 事件名称 返回 成功，返回MV_OK；失败，返回错误码

示例 Events.cpp , 以及 Events_Interface.cpp.

### ￿ MV_CC_EventNotificationOff()

MV_CAMCTRL_API int __stdcall MV_CC_EventNotificationOff ( IN void * handle,

IN const char * strEventName

)

关闭设备指定事件

参数

handle [IN] 设备句柄

strEventName [IN] 事件名称 返回 成功，返回MV_OK；失败，返回错误码

### ￿ MV_CC_RegisterExceptionCallBack()

MV_CAMCTRL_API int __stdcall MV_CC_RegisterExceptionCalIN void lBack (* handle,

void__stdcall *cbException)(unsigned int nMsgType, INvoid *pUser,

IN void
* pUser

)

注册异常消息回调，在打开设备之后调用

参数

handle [IN] 设备句柄

cbException [IN] 异常回调函数指针

pUser [IN] 用户自定义变量 返回 成功，返回MV_OK；失败，返回错误码

注解
- 该接口需要在 MV_CC_OpenDevice() 打开设备之后调用。
- 设备异常断开连接后可以在回调里面获取到异常消息，GigE设备掉线之后需要先调用 MV_CC_CloseDevice() 接口关闭设备，再调用 MV_CC_OpenDevice() 接口重新打开 设备。 宏定义 宏定义值 含义

MV_EXCEPTION_DEV_DISCONNECT 0x00008001 设备断开连接

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

函数

### GigE相机

仅GigE相机支持的接口 更多...

### 函数

MV_CAMCTRL_API int __stdcall MV_CC_GetOptimalPacketSize (IN void *handle) 获取最佳的packet size，该接口目前只支持GigE相机 更多...

MV_CAMCTRL_API int __stdcall MV_GIGE_SetEnumDevTimeout (IN unsigned int nMilTimeout) 设置枚举超时时间，仅支持GigE协议，范围 1-UINT_MAX（ 包括1，不包括UINT_MAX） 更多...

MV_CAMCTRL_API int __stdcall MV_GIGE_ForceIpEx (IN void *handle, IN unsigned int nIP, IN unsigned int nSubNetMask, IN unsigned int nDefaultGateWay) 强制IP 更多...

MV_CAMCTRL_API int __stdcall MV_GIGE_SetIpConfig (IN void *handle, IN unsigned int nType) 配置IP方式 更多...

MV_CAMCTRL_API int __stdcall MV_GIGE_SetNetTransMode (IN void *handle, IN unsigned int nType) 设置仅使用某种模式，type: MV_NET_TRANS_x，不设置时 ，默认优先使用driver 更多...

MV_CAMCTRL_API int __stdcall MV_GIGE_GetNetTransInfo (IN void *handle, IN OUT MV_NETTRANS_INFO *pstInfo) 获取网络传输信息 更多...

MV_CAMCTRL_API int __stdcall MV_GIGE_SetDiscoveryMode (IN unsigned int nMode) 设置枚举命令的回复包类型 更多...

MV_CAMCTRL_API int __stdcall MV_GIGE_SetGvspTimeout (IN void *handle, IN unsigned int nMillisec) 设置GVSP取流超时时间 更多...

MV_CAMCTRL_API int __stdcall MV_GIGE_GetGvspTimeout (IN void *handle, IN OUT unsigned int *pnMillisec) 获取GVSP取流超时时间 更多...

MV_CAMCTRL_API int __stdcall MV_GIGE_SetGvcpTimeout (IN void *handle, IN unsigned int nMillisec) 设置GVCP命令超时时间 更多...

MV_CAMCTRL_API int __stdcall MV_GIGE_GetGvcpTimeout (IN void *handle, IN OUT unsigned int *pnMillisec) 获取GVCP命令超时时间 更多...

MV_CAMCTRL_API int __stdcall MV_GIGE_SetRetryGvcpTimes (IN void *handle, IN unsigned int nRetryGvcpTimes) 设置重传GVCP命令次数 更多...

MV_CAMCTRL_API int __stdcall MV_GIGE_GetRetryGvcpTimes (IN void *handle, IN OUT unsigned int *pnRetryGvcpTimes) 获取重传GVCP命令次数 更多...

MV_CAMCTRL_API int __stdcall MV_GIGE_SetResend (IN void *handle, IN unsigned int bEnable, IN unsigned int nMaxResendPercent, IN unsigned int nResendTimeout) 设置是否打开重发包支持，及重发包设置 更多...

MV_CAMCTRL_API int __stdcall MV_GIGE_SetResendMaxRetryTimes (IN void *handle, IN unsigned int nRetryTimes) 设置重传命令最大尝试次数 更多...

MV_CAMCTRL_API int __stdcall MV_GIGE_GetResendMaxRetryTimes (IN void *handle, IN OUT unsigned int *pnRetryTimes) 获取重传命令最大尝试次数 更多...

MV_CAMCTRL_API int __stdcall MV_GIGE_SetResendTimeInterval (IN void *handle, IN unsigned int nMillisec) 设置同一重传包多次请求之间的时间间隔 更多...

MV_CAMCTRL_API int __stdcall MV_GIGE_GetResendTimeInterval (IN void *handle, IN OUT unsigned int *pnMillisec) 获取同一重传包多次请求之间的时间间隔 更多...

MV_CAMCTRL_API int __stdcall MV_GIGE_SetTransmissionType (IN void *handle, IN MV_TRANSMISSION_TYPE *pstTransmissionType) 设置传输模式，可以为单播模式、组播模式等 更多...

MV_CAMCTRL_API int __stdcall MV_GIGE_IssueActionCommand (IN MV_ACTION_CMD_INFO *pstActionCmdInfo, IN OUT MV_ACTION_CMD_RESULT_LIST *pstActionCmdResults) 发出动作命令 更多...

MV_CAMCTRL_API int __stdcall MV_GIGE_GetMulticastStatus (IN MV_CC_DEVICE_INFO *pstDevInfo, IN OUT bool *pbStatus) 获取组播状态 更多...

### 详细描述

仅GigE相机支持的接口

### 函数说明

### ￿ MV_CC_GetOptimalPacketSize()

MV_CAMCTRL_API int __stdcall MV_CC_GetOptimalPacketSize ( IN void * handle )

获取最佳的packet size，该接口目前只支持GigE相机

参数

handle [IN] 设备句柄 返回 最佳packetsize

注解
- 获取最佳的packet size，对应GigEVision设备是SCPS，对应U3V设备是每次从驱动读 取的包大小，该大小即网络上传输一个包的大小。
- 该接口需要在 MV_CC_OpenDevice() 之后、 MV_CC_StartGrabbing() 之前调用。
- 该接口不支持CameraLink设备、U3V设备。
- 该接口不支持GenTL设备（协议不支持），如果是GenTL方式添加的网口相机，建议 根据网络实际情况配置GevSCPSPacketSize，或者配置1500。

示例

ConnectSpecCamera.cpp, ConvertPixelType.cpp, Events.cpp, Grab_ActionCommand.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, InterfaceAndDeviceDemo.cpp, LensShadingCorrection.cpp, MultiCast.cpp, MultiLightCtrl_ImageStitching.cpp, ParametrizeCamera_LineScanIOSettings.cpp , 以及 Recording.cpp.

### ￿ MV_GIGE_SetEnumDevTimeout()

MV_CAMCTRL_API int __stdcall MV_GIGE_SetEnumDevTimeout ( IN unsigned int nMilTimeout )

设置枚举超时时间，仅支持GigE协议，范围 1-UINT_MAX（包括1，不包括UINT_MAX）

参数

nMilTimeout [IN] 超时时间（ms），需设置整数，默认100 返回 成功，返回MV_OK ；失败，返回错误码

注解
- 在调用 MV_CC_EnumDevices() 等枚举接口前使用该接口，可设置枚举GIGE设备的 网卡最大超时时间（默认100ms），可以减少最大超时时间，以加快枚举GIGE设备的速 度。
- 该接口仅支持输入无符号整数。 • 仅支持GigEVision设备。

### ￿ MV_GIGE_ForceIpEx()

MV_CAMCTRL_API int __stdcall MV_GIGE_ForceIpEx ( IN void * handle,

IN unsigned int nIP,

IN unsigned int nSubNetMask,

IN unsigned int nDefaultGateWay

)

强制IP

参数

handle [IN] 设备句柄

nIP [IN] 设置的IP

nSubNetMask [IN] 子网掩码

nDefaultGateWay [IN] 默认网关 返回 成功，返回MV_OK ；失败，返回错误码

注解
- 强制设置设备网络参数（包括IP、子网掩码、默认网关），强制设置之后将需要重新 创建设备句柄，仅GigEVision设备支持。
- 如果设备为DHCP的状态，调用该接口强制设置设备网络参数之后设备将会重启。

示例 ForceIPEx.cpp.

### ￿ MV_GIGE_SetIpConfig()

MV_CAMCTRL_API int __stdcall MV_GIGE_SetIpConfig ( IN void * handle,

IN unsigned int nType

)

配置IP方式

参数

handle [IN] 设备句柄

nType [IN] IP类型，见 MV_IP_CFG_x 宏定义 宏定义值 含义

MV_IP_CFG_STATIC 0x05000000 固定IP地址模式

MV_IP_CFG_DHCP 0x06000000 DHCP自动获取IP模式

MV_IP_CFG_LLA 0x04000000 LLA(Link-local address)，链路本地地址

返回 成功，返回MV_OK ；失败，返回错误码

注解 发送命令设置设备的IP方式，如DHCP、LLA等，仅GigEVision设备支持。

示例 ForceIPEx.cpp.

### ￿ MV_GIGE_SetNetTransMode()

MV_CAMCTRL_API int __stdcall MV_GIGE_SetNetTransMode ( IN void * handle,

IN unsigned int nType

)

设置仅使用某种模式，type: MV_NET_TRANS_x，不设置时，默认优先使用driver

参数

handle [IN] 设备句柄

nType [IN] 网络传输模式，见MV_NET_TRANS_x 宏定义 宏定义值 含义

MV_NET_TRANS_DRIVER 0x00000001 驱动模式

MV_NET_TRANS_SOCKET 0x00000002 Socket模式

返回 成功，返回MV_OK ；失败，返回错误码

注解 通过该接口可以设置SDK内部优先使用的网络模式，默认优先使用驱动模式，仅 GigEVision设备支持。

### ￿ MV_GIGE_GetNetTransInfo()

MV_CAMCTRL_API int __stdcall MV_GIGE_GetNetTransInfo ( IN void * handle,

IN OUT MV_NETTRANS_INFO * pstInfo

)

获取网络传输信息

参数

handle [IN] 设备句柄

pstInfo [IN][OUT] 信息结构体 返回 成功，返回MV_OK ；失败，返回错误码

注解

通过该接口可以获取网络传输相关信息，包括已接收数据大小、丢帧数量等，在 MV_CC_StartGrabbing() 开启采集之后调用。仅GigEVision相机支持。

### ￿ MV_GIGE_SetDiscoveryMode()

MV_CAMCTRL_API int __stdcall MV_GIGE_SetDiscoveryMode ( IN unsigned int nMode )

设置枚举命令的回复包类型

参数

nMode [IN] 回复包类型（默认广播），0-单播，1-广播 返回 成功，返回MV_OK ；失败，返回错误码

注解 该接口只对GigE相机有效。

### ￿ MV_GIGE_SetGvspTimeout()

MV_CAMCTRL_API int __stdcall MV_GIGE_SetGvspTimeout ( IN void * handle,

IN unsigned int nMillisec

)

设置GVSP取流超时时间

参数

handle [IN] 设备句柄

nMillisec [IN] 超时时间，默认30ms，范围：>10ms 返回 成功，返回MV_OK ；失败，返回错误码

注解 连接设备之后，取流动作发生前，调用该接口可以设置GVSP取流超时时间。GVSP取流 超时设置过短可能造成图像异常，设置过长可能造成取流时间变长。

### ￿ MV_GIGE_GetGvspTimeout()

MV_CAMCTRL_API int __stdcall MV_GIGE_GetGvspTimeout ( IN void * handle,

IN OUT unsigned int * pnMillisec

)

获取GVSP取流超时时间

参数

handle [IN] 设备句柄

pnMillisec [IN][OUT] 超时时间指针，以毫秒为单位 返回 成功，返回MV_OK ；失败，返回错误码

注解 该接口用于获取当前的GVSP取流超时时间。

### ￿ MV_GIGE_SetGvcpTimeout()

MV_CAMCTRL_API int __stdcall MV_GIGE_SetGvcpTimeout ( IN void * handle,

IN unsigned int nMillisec

)

设置GVCP命令超时时间

参数

handle [IN] 设备句柄

nMillisec [IN] 超时时间，默认500ms，范围：0-10000ms 返回 成功，返回MV_OK ；失败，返回错误码

注解 连接设备之后调用该接口可以设置GVCP命令超时时间。

### ￿ MV_GIGE_GetGvcpTimeout()

MV_CAMCTRL_API int __stdcall MV_GIGE_GetGvcpTimeout ( IN void * handle,

IN OUT unsigned int * pnMillisec

)

获取GVCP命令超时时间

参数

handle [IN] 设备句柄

pnMillisec [IN][OUT] 超时时间指针，以毫秒为单位 返回 成功，返回MV_OK ；失败，返回错误码

注解 该接口用于获取当前的GVCP超时时间。

### ￿ MV_GIGE_SetRetryGvcpTimes()

MV_CAMCTRL_API int __stdcall MV_GIGE_SetRetryGvcpTimes ( IN void * handle,

IN unsigned int nRetryGvcpTimes

)

设置重传GVCP命令次数

参数

handle [IN] 设备句柄

nRetryGvcpTimes [IN] 重传次数，范围：0-100 返回 成功，返回MV_OK ；失败，返回错误码

注解 该接口用于在GVCP包传输异常时，增加重传的次数，在一定程度上可以避免设备掉线 ，范围为0-100。

### ￿ MV_GIGE_GetRetryGvcpTimes()

MV_CAMCTRL_API int __stdcall MV_GIGE_GetRetryGvcpTimes ( IN void * handle,

IN OUT unsigned int * pnRetryGvcpTimes

)

获取重传GVCP命令次数

参数

handle [IN] 设备句柄

pnRetryGvcpTimes [IN][OUT] 重传次数指针 返回 成功，返回MV_OK ；失败，返回错误码

注解 该接口用于获取当前的GVCP重传次数，默认3次。

### ￿ MV_GIGE_SetResend()

MV_CAMCTRL_API int __stdcall MV_GIGE_SetResend ( IN void * handle,

IN unsigned int bEnable,

IN unsigned int nMaxResendPercent,

IN unsigned int nResendTimeout

)

设置是否打开重发包支持，及重发包设置

参数

handle [IN] 设备句柄

bEnable [IN] 是否支持重发包

nMaxResendPercent [IN] 最大重发比

nResendTimeout [IN] 重发超时时间 返回 成功，返回MV_OK ；失败，返回错误码

注解 连接设备之后调用该接口可以设置重发包属性，仅GigEVision相机支持。

### ￿ MV_GIGE_SetResendMaxRetryTimes()

MV_CAMCTRL_API int __stdcall MV_GIGE_SetResendMaxRetryTimes ( IN void * handle,

IN unsigned int nRetryTimes

)

设置重传命令最大尝试次数

参数

handle [IN] 设备句柄

nRetryTimes [IN] 重传命令最大尝试次数，默认值20 返回 成功，返回MV_OK ；失败，返回错误码

注解 该接口必须在调用 MV_GIGE_SetResend() 开启重传包功能之后调用，否则失败且返回 MV_E_CALLORDER。

### ￿ MV_GIGE_GetResendMaxRetryTimes()

MV_CAMCTRL_API int __stdcall MV_GIGE_GetResendMaxRetryTimes ( IN void * handle,

IN OUT unsigned int * pnRetryTimes

)

获取重传命令最大尝试次数

参数

handle [IN] 设备句柄

pnRetryTimes [IN][OUT] 重传命令最大尝试次数 返回 成功，返回MV_OK ；失败，返回错误码

注解 该接口必须在调用 MV_GIGE_SetResend() 开启重传包功能之后调用，否则失败且返回 MV_E_CALLORDER。

### ￿ MV_GIGE_SetResendTimeInterval()

MV_CAMCTRL_API int __stdcall MV_GIGE_SetResendTimeInterval ( IN void * handle,

IN unsigned int nMillisec

)

设置同一重传包多次请求之间的时间间隔

参数

handle [IN] 设备句柄

nMillisec [IN] 同一重传包多次请求之间的时间间隔，默认10ms 返回 成功，返回MV_OK ；失败，返回错误码

注解 该接口必须在调用 MV_GIGE_SetResend() 开启重传包功能之后调用，否则失败且返回 MV_E_CALLORDER。

### ￿ MV_GIGE_GetResendTimeInterval()

MV_CAMCTRL_API int __stdcall MV_GIGE_GetResendTimeInterval ( IN void * handle,

IN OUT unsigned int * pnMillisec

)

获取同一重传包多次请求之间的时间间隔

参数

handle [IN] 设备句柄

pnMillisec [IN][OUT] 同一重传包多次请求之间的时间间隔 返回 成功，返回MV_OK ；失败，返回错误码

注解 该接口必须在调用 MV_GIGE_SetResend() 开启重传包功能之后调用，否则失败且返回 MV_E_CALLORDER。

### ￿ MV_GIGE_SetTransmissionType()

MV_CAMCTRL_API int __stdcall MV_GIGE_SetTransmissionType ( IN void * handle,

IN MV_TRANSMISSION_TYPE * pstTransmissionType

)

设置传输模式，可以为单播模式、组播模式等

参数

handle [IN] 设备句柄

pstTransmissionT[IN] 传输模式结构体 ype宏定义含 宏定义 值义

单 MV_GIGE_TRANSTYPE_UNICAST 0 播

组 MV_GIGE_TRANSTYPE_MULTICAST 1 播

表 示 局 域 MV_GIGE_TRANSTYPE_LIMITEDBROADCAST 2 网 内 广 播

表 示 子 MV_GIGE_TRANSTYPE_SUBNETBROADCAS 3网 T 内 广 播

表 示 从 MV_GIGE_TRANSTYPE_CAMERADEFINED 4相 机 获 取

表 MV_GIGE_TRANSTYPE_UNICAST_DEFINED_示 PORT 用 户

自 定 义 应 用 端 接 收 图 像 数 据 Por t号

表 示 设 置 了 单 播 ， 但 MV_GIGE_TRANSTYPE_UNICAST_WITHOUT_000100 本 RECV00 实 例 不 接 收 图 像 数 据

表 示 MV_GIGE_TRANSTYPE_MULTICAST_WITHOU000100 组 T_RECV01 播 模

式 ， 但 本 实 例 不 接 收 图 像 数 据

返回 成功，返回MV_OK ；失败，返回错误码

注解 通过该接口可以设置传输模式为单播、组播等模式，仅GigEVision相机支持。

示例 MultiCast.cpp.

### ￿ MV_GIGE_IssueActionCommand()

MV_CAMCTRL_API int __stdcall MV_GIGE_IssueActionComm and ( IN MV_ACTION_CMD_INFO * pstActionCmdInfo,

IN OUTpstActionCmdResult MV_ACTION_CMD_RESULT_LIST *s

)

发出动作命令

参数

pstActionCmdInfo [IN] 动作命令信息

pstActionCmdResults [IN][OUT] 动作命令返回信息列表 返回

成功，返回MV_OK ；失败，返回错误码

注解 仅GigEVision相机支持。

示例 Grab_ActionCommand.cpp.

### ￿ MV_GIGE_GetMulticastStatus()

MV_CAMCTRL_API int __stdcall MV_GIGE_GetMulticastStatus ( IN MV_CC_DEVICE_INFO * pstDevInfo,

IN OUT bool * pbStatus

)

获取组播状态

参数

pstDevInfo [IN] 设备信息结构体

pbStatus [IN][OUT] 组播状态（true：组播状态；false：非组播） 返回 成功，返回MV_OK ；失败，返回错误码

注解 该接口用于判断设备当前是否处于组播状态，解决客户端枚举时需要打开设备判断组播 的问题。

仅支持标准GigEVision设备。

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

函数

### U3V相机

仅U3V相机支持的接口 更多...

### 函数

MV_CAMCTRL_API int __stdcall MV_USB_SetTransferSize (IN void *handle, IN unsigned int nTransferSize) 设置U3V的传输包大小 更多...

MV_CAMCTRL_API int __stdcall MV_USB_GetTransferSize (IN void *handle, IN OUT unsigned int *pnTransferSize) 获取U3V的传输包大小 更多...

MV_CAMCTRL_API int __stdcall MV_USB_SetTransferWays (IN void *handle, IN unsigned int nTransferWays) 设置U3V的传输通道个数 更多...

MV_CAMCTRL_API int __stdcall MV_USB_GetTransferWays (IN void *handle, unsigned int *pnTransferWays) 获取U3V的传输通道个数 更多...

MV_CAMCTRL_API int __stdcall MV_USB_RegisterStreamExceptionCallBack (IN void *handle, IN void(__stdcall *cbException)(MV_CC_STREAM_EXCEPTION_TYPE enExceptionType, void *pUser), IN void *pUser) 注册流异常消息回调，在打开设备之后调用（只支持U3V相机 ，不支持GenTL设备） 更多...

MV_CAMCTRL_API int __stdcall MV_USB_SetEventNodeNum (IN void *handle, IN unsigned int nEventNodeNum) 设置U3V的事件缓存节点个数 更多...

MV_CAMCTRL_API int __stdcall MV_USB_SetSyncTimeOut (IN void *handle, IN unsigned int nMills) 设置U3V相机同步读写超时时间，范围：1000-UINT_MAX， （包含1000，不包含UNIT_MAX），默认1000 ms 更多...

MV_CAMCTRL_API int __stdcall MV_USB_GetSyncTimeOut (IN void *handle, IN OUT unsigned int *pnMills) 获取U3V相机同步读写超时时间 更多...

### 详细描述

仅U3V相机支持的接口

### 函数说明

### ￿ MV_USB_SetTransferSize()

MV_CAMCTRL_API int __stdcall MV_USB_SetTransferSize ( IN void * handle,

IN unsigned int nTransferSize

)

设置U3V的传输包大小

参数

handle [IN] 设备句柄

nTransferSize [IN] 传输的包大小，Byte，默认为1M，rang：>=0x400，建议最大值： [windows] rang <= 0x400000；[Linux] rang <= 0x200000

返回 成功，返回MV_OK ；失败，返回错误码

注解

增加传输包大小可以适当降低取流时的CPU占用率。但不同的PC和不同USB扩展卡存在 不同的兼容性，如果该参数设置过大可能会出现取不到图像的风险。

### ￿ MV_USB_GetTransferSize()

MV_CAMCTRL_API int __stdcall MV_USB_GetTransferSize ( IN void * handle,

IN OUT unsigned int * pnTransferSize

)

获取U3V的传输包大小

参数

handle [IN] 设备句柄

pnTransferSize [IN][OUT] 传输的包大小指针，Byte 返回 成功，返回MV_OK ；失败，返回错误码

注解 该接口用于获取当前的U3V传输包大小，默认1M。

### ￿ MV_USB_SetTransferWays()

MV_CAMCTRL_API int __stdcall MV_USB_SetTransferWays ( IN void * handle,

IN unsigned int nTransferWays

)

设置U3V的传输通道个数

参数

handle [IN] 设备句柄

nTransferWays [IN] 传输通道个数，范围：1-10 返回 成功，返回MV_OK ；失败，返回错误码

注解 可根据PC的性能、设备出图帧率、图像大小和内存使用率等因素对该参数进行调节。但 不同的PC和不同的USB扩展卡存在不同的兼容性。

### ￿ MV_USB_GetTransferWays()

MV_CAMCTRL_API int __stdcall MV_USB_GetTransferWays ( IN void * handle,

unsigned int * pnTransferWays

)

获取U3V的传输通道个数

参数

handle [IN] 设备句柄

pnTransferWays [IN][OUT] 传输通道个数指针 返回 成功，返回MV_OK ；失败，返回错误码

注解 该接口用于获取当前的U3V异步取流节点个数，U口相机传输通道个数和像素格式对应 的负载包大小相关，可通过（最大异步注册长度/像素格式对应的负载包大小）计算得出 。 2000W设备的MONO8默认为3个，YUV为默认2个，RGB为默认1个，其它情况默认8个 节点。

### ￿ MV_USB_RegisterStreamExceptionCallBack()

MV_CAMCTRL_API int __stdcallIN MV_USB_RegisterStreamvoid ExceptionCallBack (* handle,

void__stdcall *cbException)(MV_CC_STREAM_EXCEPTION_TYPE INenExceptionType, void *pUser,

IN void
* pUser

)

注册流异常消息回调，在打开设备之后调用（只支持U3V相机，不支持GenTL设备）

参数

handle [IN] 设备句柄

cbException [IN] 异常回调函数指针

pUser [IN] 用户自定义变量 返回 成功，返回MV_OK ；失败，返回错误码

### ￿ MV_USB_SetEventNodeNum()

MV_CAMCTRL_API int __stdcall MV_USB_SetEventNodeNum ( IN void * handle,

IN unsigned int nEventNodeNum

)

设置U3V的事件缓存节点个数

参数

handle [IN] 设备句柄

nEventNodeNum [IN] 事件缓存节点个数，范围：1-64 返回 成功，返回MV_OK ；失败，返回错误码

注解 该接口用于设置当前的U3V事件缓存节点个数，默认情况下为5个。

### ￿ MV_USB_SetSyncTimeOut()

MV_CAMCTRL_API int __stdcall MV_USB_SetSyncTimeOut ( IN void * handle,

IN unsigned int nMills

)

设置U3V相机同步读写超时时间，范围：1000-UINT_MAX，（包含1000，不包含UNIT_MAX ），默认1000 ms

参数

handle [IN] 设备句柄

nMills [IN] 设置同步读写超时时间（ms），默认值为1000 返回

成功，返回MV_OK ；失败，返回错误码

注解 增加设置同步读取时间接口，兼容部分相机配置参数很慢，超过1s的情况。

### ￿ MV_USB_GetSyncTimeOut()

MV_CAMCTRL_API int __stdcall MV_USB_GetSyncTimeOut ( IN void * handle,

IN OUT unsigned int * pnMills

)

获取U3V相机同步读写超时时间

参数

handle [IN] 设备句柄

pnMills [IN][OUT] 获取的超时时间（ms） 返回 成功，返回MV_OK ；失败，返回错误码

注解 该接口用于获取当前的U3V同步读写超时时间大小，默认1000ms。

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

函数

### CameraLink相机

仅Camera Link相机支持的接口 更多...

### 函数

MV_CAMCTRL_API int __stdcall MV_CAML_SetDeviceBaudrate (IN void *handle, IN unsigned int nBaudrate) 设置设备波特率 更多...

MV_CAMCTRL_API int __stdcall MV_CAML_GetDeviceBaudrate (IN void *handle, IN OUT unsigned int *pnCurrentBaudrate) 获取设备波特率 更多...

MV_CAMCTRL_API int __stdcall MV_CAML_GetSupportBaudrates (IN void *handle, IN OUT unsigned int *pnBaudrateAblity) 获取设备与主机间连接支持的波特率 更多...

MV_CAMCTRL_API int __stdcall MV_CAML_SetGenCPTimeOut (IN void *handle, IN unsigned int nMillisec) 设置串口操作等待时长 更多...

MV_CAMCTRL_API int __stdcall MV_CAML_GetSerialPortList (IN OUT MV_CAML_SERIAL_PORT_LIST *pstSerialPortList) 获取串口信息列表 更多...

MV_CAMCTRL_API int __stdcall MV_CAML_SetEnumSerialPorts (IN MV_CAML_SERIAL_PORT_LIST *pstSerialPortList) 设置指定枚举串口列表 更多...

### 详细描述

仅Camera Link相机支持的接口

### 函数说明

### ￿ MV_CAML_SetDeviceBaudrate()

MV_CAMCTRL_API int __stdcall MV_CAML_SetDeviceBaudrate ( IN void * handle,

IN unsigned int nBaudrate

)

设置设备波特率

参数

handle [IN] 设备句柄

nBaudrate [IN] 设置的波特率值，数值参考CameraParams.h中宏定义 CamercaLink波特率定义 值 对应的波特率值

MV_CAML_BAUDRATE_9600 0x00000001 9600

MV_CAML_BAUDRATE_19200 0x00000002 19200

MV_CAML_BAUDRATE_38400 0x00000004 38400

MV_CAML_BAUDRATE_57600 0x00000008 57600

MV_CAML_BAUDRATE_115200 0x00000010 115200

MV_CAML_BAUDRATE_230400 0x00000020 230400

MV_CAML_BAUDRATE_460800 0x00000040 460800

MV_CAML_BAUDRATE_921600 0x00000080 921600

MV_CAML_BAUDRATE_AUTOMAX 0x40000000 最大值

返回 成功，返回MV_OK ；失败，返回错误码

注解 该接口支持在设备未连接时调用。通过GenTL协议访问设备时，需要先连接设备，才能 调用该接口。 因硬件/系统/外部干扰等因素，配置高波特率可能导致通信异常，建议配置波特率最大小 于115200。

### ￿ MV_CAML_GetDeviceBaudrate()

MV_CAMCTRL_API int __stdcall MV_CAML_GetDeviceBaudrate ( IN void * handle,

IN OUT unsigned int * pnCurrentBaudrate

)

获取设备波特率

参数

handle [IN] 设备句柄

pnCurrentBaudrate [IN][OUT] 波特率信息指针，数值参考CameraParams.h中宏定义 返回 成功，返回MV_OK ；失败，返回错误码

注解 该接口支持在设备未连接时调用。

### ￿ MV_CAML_GetSupportBaudrates()

MV_CAMCTRL_API int __stdcall MV_CAML_GetSupportBaudrates ( IN void * handle,

IN OUT unsigned int * pnBaudrateAblity

)

获取设备与主机间连接支持的波特率

参数

handle [IN] 设备句柄

pnBaudrateAblity [IN][OUT] 支持的波特率信息的指针。所支持波特率的或运算结果，单 个数值参考CameraParams.h中宏定义 返回 成功，返回MV_OK ；失败，返回错误码

注解 该接口支持在设备未连接时调用。

### ￿ MV_CAML_SetGenCPTimeOut()

MV_CAMCTRL_API int __stdcall MV_CAML_SetGenCPTimeOut ( IN void * handle,

IN unsigned int nMillisec

)

设置串口操作等待时长

参数

handle [IN] 设备句柄

nMillisec [IN] 串口操作的等待时长，单位为ms 返回 成功，返回MV_OK ；失败，返回错误码

### ￿ MV_CAML_GetSerialPortList()

MV_CAMCTRL_API int __stdcall MV_CAML_GetSerialPortList ( IN OUT MV_CAML_SERIAL_PORT_LIST * pstSerialPortList )

获取串口信息列表

参数

pstSerialPortList [IN][OUT] 串口信息列表 返回 成功，返回MV_OK；失败，返回错误码

注解 该接口用于获取本地的串口信息。

### ￿ MV_CAML_SetEnumSerialPorts()

MV_CAMCTRL_API int __stdcall MV_CAML_SetEnumSerialPorts ( IN MV_CAML_SERIAL_PORT_LIST * pstSerialPortList )

设置指定枚举串口列表

参数

pstSerialPortList [IN] 串口信息列表 返回 成功，返回MV_OK；失败，返回错误码

注解 该接口用于设置枚举CameraLink设备的指定串口。

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

函数

### GenTL

GenTL相关接口 更多...

### 函数

MV_CAMCTRL_API int __stdcall MV_CC_EnumInterfacesByGenTL (IN OUT MV_GENTL_IF_INFO_LIST *pstIFList, IN const char *strGenTLPath) 通过GenTL枚举Interfaces 更多...

MV_CAMCTRL_API int __stdcall MV_CC_UnloadGenTLLibrary (IN const char *pGenTLPath) 卸载cti库 更多...

MV_CAMCTRL_API int __stdcall MV_CC_EnumDevicesByGenTL (IN MV_GENTL_IF_INFO *pstIFInfo, IN OUT MV_GENTL_DEV_INFO_LIST *pstDevList) 通过GenTL Interface枚举设备 更多...

MV_CAMCTRL_API int __stdcall MV_CC_CreateHandleByGenTL (IN OUT void **handle, IN const MV_GENTL_DEV_INFO *pstDevInfo) 通过GenTL设备信息创建设备句柄 更多...

### 详细描述

GenTL相关接口

### 函数说明

### ￿ MV_CC_EnumInterfacesByGenTL()

MV_CAMCTRL_API int __stdcall MV_CC_EnumInterfacesByGenTL ( IN OUT MV_GENTL_IF_INFO_LIST * pstIFList,

IN const char * strGenTLPath

)

通过GenTL枚举Interfaces

参数

pstIFList [IN][OUT] Interfaces列表

strGenTLPath [IN] GenTL的cti文件路径 返回 成功，返回MV_OK ；失败，返回错误码

注解
- Interfaces列表的内存是在SDK内部分配的，多线程调用该接口时会进行设备列表内存 的释放和申请，建议尽量避免多线程枚举操作。
- 暂不支持直接调用MvProducerU3V.cti和MvProducerGEV.cti， 支持调用其他.cti文件

### ￿ MV_CC_UnloadGenTLLibrary()

MV_CAMCTRL_API int __stdcall MV_CC_UnloadGenTLLibrary ( IN const char * pGenTLPath )

卸载cti库

参数

pGenTLPath [IN] 枚举卡时加载的cti文件路径 返回 成功，返回MV_OK ；失败，返回错误码

注解 卸载前需要保证通过该cti枚举出的相机已全部关闭，否则报错前置条件错误。

### ￿ MV_CC_EnumDevicesByGenTL()

MV_CAMCTRL_API int __stdcall MV_CC_EnumDevicesByGenTL ( IN MV_GENTL_IF_INFO * pstIFInfo,

IN OUT MV_GENTL_DEV_INFO_LIST * pstDevList

)

通过GenTL Interface枚举设备

参数

pstIFInfo [IN] Interface信息

pstDevList [IN][OUT] 设备列表 返回 成功，返回MV_OK ；失败，返回错误码

注解 设备列表的内存是在SDK内部分配的，多线程调用该接口时会进行设备列表内存的释放 和申请，建议尽量避免多线程枚举操作。

### ￿ MV_CC_CreateHandleByGenTL()

MV_CAMCTRL_API int __stdcall MV_CC_CreateHandleByGenTL ( IN OUT void ** handle,

IN const MV_GENTL_DEV_INFO * pstDevInfo

)

通过GenTL设备信息创建设备句柄

参数

handle [IN][OUT] 设备句柄

pstDevInfo [IN] 设备信息结构体指针 返回 成功，返回MV_OK ；失败，返回错误码

注解 根据输入的设备信息，创建库内部必须的资源和初始化内部模块。

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

函数

### 设备升级

包含设备升级和获取升级进度接口 更多...

### 函数

MV_CAMCTRL_API int __stdcall MV_CC_LocalUpgrade (IN void *handle, IN const void *strFilePathName) 设备本地升级 更多...

MV_CAMCTRL_API int __stdcall MV_CC_GetUpgradeProcess (IN void *handle, IN OUT unsigned int *pnProcess) 获取升级进度 更多...

### 详细描述

包含设备升级和获取升级进度接口

### 函数说明

### ￿ MV_CC_LocalUpgrade()

MV_CAMCTRL_API int __stdcall MV_CC_LocalUpgrade ( IN void * handle,

IN const void * strFilePathName

)

设备本地升级

参数

handle [IN] 设备句柄

strFilePathName [IN] 文件名 返回 成功，返回MV_OK；失败，返回错误码

注解 通过该接口可以将升级固件文件发送给设备进行升级。该接口需要等待升级固件文件成 功传给设备端之后再返回，响应时间可能较长。

### ￿ MV_CC_GetUpgradeProcess()

MV_CAMCTRL_API int __stdcall MV_CC_GetUpgradeProcess ( IN void * handle,

IN OUT unsigned int * pnProcess

)

获取升级进度

参数

handle [IN] 设备句柄

pnProcess [IN][OUT] 进度接收地址 返回 成功，返回MV_OK；失败，返回错误码

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

函数

### 废弃接口

废弃原因：新开发的万能接口代替了具体节点的设置、获取参数功能；效率低的接口由新的接口替 代；实际开发中部分接口使用频率较低。 更多...

### 函数

MV_CAMCTRL_API int __stdcall MV_CC_GetImageInfo (IN void *handle, IN OUT MV_IMAGE_BASIC_INFO *pstInfo) 获取图像基本信息 更多...

MV_CAMCTRL_API void *__stdcall MV_CC_GetTlProxy (IN void *handle) 获取GenICam代理 更多...

MV_CAMCTRL_API int __stdcall MV_XML_GetRootNode (IN void *handle, IN OUT MV_XML_NODE_FEATURE *pstNode) 获取根节点 更多...

MV_CAMCTRL_API int __stdcall MV_XML_GetChildren (IN void *handle, IN MV_XML_NODE_FEATURE *pstNode, IN OUT MV_XML_NODES_LIST *pstNodesList) 从xml中获取指定节点的所有子节点，根节点为Root 更多...

MV_CAMCTRL_API int __stdcall MV_XML_GetNodeFeature (IN void *handle, IN MV_XML_NODE_FEATURE *pstNode, IN OUT void *pstFeature) 获得当前节点的属性 更多...

MV_CAMCTRL_API int __stdcall MV_XML_UpdateNodeFeature (IN void *handle, IN enum MV_XML_InterfaceType enType, IN void *pstFeature) 更新节点 更多...

MV_CAMCTRL_API int __stdcall MV_XML_RegisterUpdateCallBack (IN void *handle, IN void(__stdcall *cbUpdate)(enum MV_XML_InterfaceType enType, void *pstFeature, MV_XML_NODES_LIST *pstNodesList, void *pUser), IN void *pUser) 注册更新回调 更多...

MV_CAMCTRL_API int __stdcall MV_CC_GetOneFrame (IN void *handle, IN OUT unsigned char *pData, IN unsigned int nDataSize, IN OUT MV_FRAME_OUT_INFO *pFrameInfo)

获取一帧图像（该接口已弃用，建议改用 MV_CC_GetOneFrameTimeout() 接口） 更多...

MV_CAMCTRL_API int __stdcall MV_CC_GetOneFrameEx (IN void *handle, IN OUT unsigned char *pData, IN unsigned int nDataSize, IN OUT MV_FRAME_OUT_INFO_EX *pFrameInfo) 获取一帧trunck数据（该接口已弃用，建议改用 MV_CC_GetOneFrameTimeout() 接口） 更多...

MV_CAMCTRL_API int __stdcall MV_CC_RegisterImageCallBack (void *handle, void(__stdcall *cbOutput)(unsigned char *pData, MV_FRAME_OUT_INFO *pFrameInfo, void *pUser), void *pUser) 注册图像数据回调（该接口已弃用，建议改用 MV_CC_RegisterImageCallBackEx() 接口） 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SaveImageEx (IN OUT MV_SAVE_IMAGE_PARAM_EX *pSaveParam) 保存图片，支持Bmp和Jpeg，编码质量在50-99之前 （该接 口已弃用，建议改用 MV_CC_SaveImageEx3() 接口） 更 多...

MV_CAMCTRL_API int __stdcall MV_CC_BayerNoiseEstimate (IN void *handle, IN OUT MV_CC_BAYER_NOISE_ESTIMATE_PARAM *pstNoiseEstimateParam) Bayer噪声估计 更多...

MV_CAMCTRL_API int __stdcall MV_CC_BayerSpatialDenoise (IN void *handle, IN OUT MV_CC_BAYER_SPATIAL_DENOISE_PARAM *pstSpatialDenoiseParam) Bayer空域降噪 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SetBayerCLUTParam (IN void *handle, IN MV_CC_CLUT_PARAM *pstCLUTParam) 设置Bayer格式的CLUT使能和信息 更多...

MV_CAMCTRL_API int __stdcall MV_CC_ImageSharpen (IN void *handle, IN OUT MV_CC_SHARPEN_PARAM *pstSharpenParam) 图像锐化 更多...

MV_CAMCTRL_API int __stdcall MV_CC_ColorCorrect (IN void *handle, IN OUT MV_CC_COLOR_CORRECT_PARAM *pstColorCorrectParam) 色彩校正（包括CCM和CLUT） 更多...

MV_CAMCTRL_API int __stdcall MV_CC_NoiseEstimate (IN void *handle, IN OUT MV_CC_NOISE_ESTIMATE_PARAM *pstNoiseEstimateParam) 噪声估计 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SpatialDenoise (IN void *handle, IN OUT MV_CC_SPATIAL_DENOISE_PARAM *pstSpatialDenoiseParam) 空域降噪 更多...

MV_CAMCTRL_API int __stdcall MV_CC_LSCCalib (IN void *handle, IN OUT MV_CC_LSC_CALIB_PARAM *pstLSCCalibParam) LSC标定 更多...

MV_CAMCTRL_API int __stdcall MV_CC_LSCCorrect (IN void *handle, IN OUT MV_CC_LSC_CORRECT_PARAM *pstLSCCorrectParam) LSC校正 更多...

MV_CAMCTRL_API int __stdcall MV_GIGE_ForceIp (IN void *handle, unsigned int nIP) 强制IP（该接口已弃用，建议改用 MV_GIGE_ForceIpEx() 接口） 更多...

MV_CAMCTRL_API int __stdcall MV_CC_RegisterEventCallBack (void *handle, void(__stdcall *cbEvent)(unsigned int nExternalEventId, void *pUser), void *pUser) 注册事件回调（该接口已弃用，建议改用 MV_CC_RegisterEventCallBackEx() 接口） 更多...

MV_CAMCTRL_API int __stdcall MV_CC_Display (IN void *handle, void *hWnd) 显示图像，注册显示窗口，内部自动显示（该接口已弃用， 建议改用 MV_CC_DisplayOneFrameEx() 接口） 更多...

MV_CAMCTRL_API int __stdcall MV_CC_DisplayOneFrame (IN void *handle, IN MV_DISPLAY_FRAME_INFO *pstDisplayInfo) 显示一帧图像（该接口已弃用，建议改用 MV_CC_DisplayOneFrameEx2() 接口） 更多...

MV_CAMCTRL_API int __stdcall MV_CC_GetIntValue (IN void *handle, IN const char *strKey, OUT MVCC_INTVALUE *pIntValue) 获取Integer属性值 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SetIntValue (IN void *handle, IN const char *strKey, IN unsigned int nValue) 设置Integer型属性值 更多...

MV_CAMCTRL_API int __stdcall MV_CC_GetWidth (IN void *handle, IN OUT MVCC_INTVALUE *pstValue) 获取图像宽度 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SetWidth (IN void *handle, IN const unsigned int nValue) 设置图像宽度 更多...

MV_CAMCTRL_API int __stdcall MV_CC_GetHeight (IN void *handle, IN OUT MVCC_INTVALUE *pstValue) 获取图像高度 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SetHeight (IN void *handle, IN const unsigned int nValue) 设置图像高度 更多...

MV_CAMCTRL_API int __stdcall MV_CC_GetAOIoffsetX (IN void *handle, IN OUT MVCC_INTVALUE *pstValue) 获取图像X偏移 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SetAOIoffsetX (IN void *handle, IN const unsigned int nValue) 设置图像AOI偏移 更多...

MV_CAMCTRL_API int __stdcall MV_CC_GetAOIoffsetY (IN void *handle, IN OUT MVCC_INTVALUE *pstValue)

获取图像Y偏移 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SetAOIoffsetY (IN void *handle, IN const unsigned int nValue) 设置图像AOI偏移 更多...

MV_CAMCTRL_API int __stdcall MV_CC_GetAutoExposureTimeLower (IN void *handle, IN OUT MVCC_INTVALUE *pstValue) 获取曝光下限 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SetAutoExposureTimeLower (IN void *handle, IN const unsigned int nValue) 设置曝光值下限 更多...

MV_CAMCTRL_API int __stdcall MV_CC_GetAutoExposureTimeUpper (IN void *handle, IN OUT MVCC_INTVALUE *pstValue) 获取曝光上限 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SetAutoExposureTimeUpper (IN void *handle, IN const unsigned int nValue) 设置曝光值上限 更多...

MV_CAMCTRL_API int __stdcall MV_CC_GetBrightness (IN void *handle, IN OUT MVCC_INTVALUE *pstValue) 获取亮度值 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SetBrightness (IN void *handle, IN const unsigned int nValue) 设置亮度值 更多...

MV_CAMCTRL_API int __stdcall MV_CC_GetFrameRate (IN void *handle, IN OUT MVCC_FLOATVALUE *pstValue) 获取帧率 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SetFrameRate (IN void *handle, IN const float fValue) 设置帧率 更多...

MV_CAMCTRL_API int __stdcall MV_CC_GetGain (IN void *handle, IN OUT MVCC_FLOATVALUE *pstValue) 获取增益 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SetGain (IN void *handle, IN const float fValue) 设置增益 更多...

MV_CAMCTRL_API int __stdcall MV_CC_GetExposureTime (IN void *handle, IN OUT MVCC_FLOATVALUE *pstValue) 获取曝光时间 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SetExposureTime (IN void *handle, IN const float fValue) 设置曝光时间 更多...

MV_CAMCTRL_API int __stdcall MV_CC_GetPixelFormat (IN void *handle, IN OUT MVCC_ENUMVALUE *pstValue) 获取像素格式 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SetPixelFormat (IN void *handle, IN const unsigned int nValue) 设置像素格式 更多...

MV_CAMCTRL_API int __stdcall MV_CC_GetAcquisitionMode (IN void *handle, IN OUT MVCC_ENUMVALUE *pstValue) 获取采集模式 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SetAcquisitionMode (IN void *handle, IN const unsigned int nValue) 设置像素格式 更多...

MV_CAMCTRL_API int __stdcall MV_CC_GetGainMode (IN void *handle, IN OUT MVCC_ENUMVALUE *pstValue) 获取增益模式 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SetGainMode (IN void *handle, IN const unsigned int nValue) 设置增益模式 更多...

MV_CAMCTRL_API int __stdcall MV_CC_GetExposureAutoMode (IN void *handle, IN OUT MVCC_ENUMVALUE *pstValue) 获取自动曝光模式 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SetExposureAutoMode (IN void *handle, IN const unsigned int nValue) 设置自动曝光模式 更多...

MV_CAMCTRL_API int __stdcall MV_CC_GetTriggerMode (IN void *handle, IN OUT MVCC_ENUMVALUE *pstValue) 获取触发模式 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SetTriggerMode (IN void *handle, IN const unsigned int nValue) 设置触发模式 更多...

MV_CAMCTRL_API int __stdcall MV_CC_GetTriggerDelay (IN void *handle, IN OUT MVCC_FLOATVALUE *pstValue) 获取触发延时 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SetTriggerDelay (IN void *handle, IN const float fValue) 设置触发延时 更多...

MV_CAMCTRL_API int __stdcall MV_CC_GetTriggerSource (IN void *handle, IN OUT MVCC_ENUMVALUE *pstValue) 获取触发源 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SetTriggerSource (IN void *handle, IN const unsigned int nValue) 设置触发源 更多...

MV_CAMCTRL_API int __stdcall MV_CC_TriggerSoftwareExecute (IN void *handle) 软触发一次（接口仅在已选择的触发源为软件触发时有效） 更多...

MV_CAMCTRL_API int __stdcall MV_CC_GetGammaSelector (IN void *handle, IN OUT MVCC_ENUMVALUE *pstValue) 获取Gamma类型 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SetGammaSelector (IN void *handle, IN const unsigned int nValue) 设置Gamma类型 更多...

MV_CAMCTRL_API int __stdcall MV_CC_GetGamma (IN void *handle, IN OUT MVCC_FLOATVALUE *pstValue) 获取Gamma值 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SetGamma (IN void *handle, IN const float fValue) 设置Gamma值 更多...

MV_CAMCTRL_API int __stdcall MV_CC_GetSharpness (IN void *handle, IN OUT MVCC_INTVALUE *pstValue) 获取锐度 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SetSharpness (IN void *handle, IN const unsigned int nValue) 设置锐度 更多...

MV_CAMCTRL_API int __stdcall MV_CC_GetHue (IN void *handle, IN OUT MVCC_INTVALUE *pstValue) 获取灰度 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SetHue (IN void *handle, IN const unsigned int nValue) 设置灰度 更多...

MV_CAMCTRL_API int __stdcall MV_CC_GetSaturation (IN void *handle, IN OUT MVCC_INTVALUE *pstValue) 获取饱和度 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SetSaturation (IN void *handle, IN const unsigned int nValue) 设置饱和度 更多...

MV_CAMCTRL_API int __stdcall MV_CC_GetBalanceWhiteAuto (IN void *handle, IN OUT MVCC_ENUMVALUE *pstValue)

获取自动白平衡 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SetBalanceWhiteAuto (IN void *handle, IN const unsigned int nValue) 设置自动白平衡 更多...

MV_CAMCTRL_API int __stdcall MV_CC_GetBalanceRatioRed (IN void *handle, IN OUT MVCC_INTVALUE *pstValue) 获取白平衡 红 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SetBalanceRatioRed (IN void *handle, IN const unsigned int nValue) 设置白平衡 红 更多...

MV_CAMCTRL_API int __stdcall MV_CC_GetBalanceRatioGreen (IN void *handle, IN OUT MVCC_INTVALUE *pstValue) 获取白平衡 绿 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SetBalanceRatioGreen (IN void *handle, IN const unsigned int nValue) 设置白平衡 绿 更多...

MV_CAMCTRL_API int __stdcall MV_CC_GetBalanceRatioBlue (IN void *handle, IN OUT MVCC_INTVALUE *pstValue) 获取白平衡 蓝 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SetBalanceRatioBlue (IN void *handle, IN const unsigned int nValue) 设置白平衡 蓝 更多...

MV_CAMCTRL_API int __stdcall MV_CC_GetFrameSpecInfoAbility (IN void *handle, IN OUT MVCC_INTVALUE *pstValue) 获取水印信息内包含的信息类型 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SetFrameSpecInfoAbility (IN void *handle, IN const unsigned int nValue) 设置水印信息内包含的信息类型 更多...

MV_CAMCTRL_API int __stdcall MV_CC_GetDeviceUserID (IN void *handle, IN OUT MVCC_STRINGVALUE *pstValue) 获取设备自定义名字 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SetDeviceUserID (IN void *handle, IN const char *chValue) 设置设备自定义名字 更多...

MV_CAMCTRL_API int __stdcall MV_CC_GetBurstFrameCount (IN void *handle, IN OUT MVCC_INTVALUE *pstValue) 获取一次触发的帧数 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SetBurstFrameCount (IN void *handle, IN const unsigned int nValue) 设置一次触发的帧数 更多...

MV_CAMCTRL_API int __stdcall MV_CC_GetAcquisitionLineRate (IN void *handle, IN OUT MVCC_INTVALUE *pstValue) 获取行频 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SetAcquisitionLineRate (IN void *handle, IN const unsigned int nValue) 设置行频 更多...

MV_CAMCTRL_API int __stdcall MV_CC_GetHeartBeatTimeout (IN void *handle, IN OUT MVCC_INTVALUE *pstValue) 获取心跳信息 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SetHeartBeatTimeout (IN void *handle, IN const unsigned int nValue) 设置心跳信息 更多...

MV_CAMCTRL_API int __stdcall MV_GIGE_GetGevSCPSPacketSize (IN void *handle, IN OUT MVCC_INTVALUE *pstValue) 获取网络包大小 更多...

MV_CAMCTRL_API int __stdcall MV_GIGE_SetGevSCPSPacketSize (IN void *handle, IN const unsigned int nValue) 设置网络包大小 更多...

MV_CAMCTRL_API int __stdcall MV_GIGE_GetGevSCPD (IN void *handle, IN OUT MVCC_INTVALUE *pstValue) 获取网络包发送间隔 更多...

MV_CAMCTRL_API int __stdcall MV_GIGE_SetGevSCPD (IN void *handle, IN const unsigned int nValue) 设置网络包发送间隔 更多...

MV_CAMCTRL_API int __stdcall MV_GIGE_GetGevSCDA (IN void *handle, unsigned int *pnIP) 获取接收端IP地址，0xa9fe0102 表示 169.254.1.2 更多...

MV_CAMCTRL_API int __stdcall MV_GIGE_SetGevSCDA (IN void *handle, unsigned int nIP) 设置接收端IP地址 更多...

MV_CAMCTRL_API int __stdcall MV_GIGE_GetGevSCSP (IN void *handle, unsigned int *pnPort) 获取发送端的端口号 更多...

MV_CAMCTRL_API int __stdcall MV_GIGE_SetGevSCSP (IN void *handle, unsigned int nPort) 设置发送端的端口号 更多...

MV_CAMCTRL_API int __stdcall MV_CAML_SetDeviceBauderate (IN void *handle, unsigned int nBaudrate) 设置设备波特率 更多...

MV_CAMCTRL_API int __stdcall MV_CAML_GetDeviceBauderate (IN void *handle, unsigned int *pnCurrentBaudrate) 获取设备波特率 更多...

MV_CAMCTRL_API int __stdcall MV_CAML_GetSupportBauderates (IN void *handle, unsigned int *pnBaudrateAblity) 获取设备与主机间连接支持的波特率 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SaveImageEx2 (IN void *handle, MV_SAVE_IMAGE_PARAM_EX *pstSaveParam)

保存图片，支持Bmp和Jpeg，编码质量在50-99之前（该接 口已弃用，建议改用 MV_CC_SaveImageEx3() 接口） 更 多...

MV_CAMCTRL_API int __stdcall MV_CC_SaveImageToFile (IN void *handle, MV_SAVE_IMG_TO_FILE_PARAM *pstSaveFileParam) 保存图像到文件（该接口已弃用，建议改用 MV_CC_SaveImageToFileEx() 接口） 更多...

MV_CAMCTRL_API int __stdcall MV_CC_ConvertPixelType (IN void *handle, IN OUT MV_CC_PIXEL_CONVERT_PARAM *pstCvtParam) 像素格式转换（该接口已弃用，建议改用 MV_CC_ConvertPixelTypeEx() 接口） 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SetSDKLogPath (IN const char *strSDKLogPath) 设置SDK日志路径 更多...

MV_CAMCTRL_API int __stdcall MV_CC_EnumerateTls () 获取支持的传输层 更多...

MV_CAMCTRL_API int __stdcall MV_CC_CreateHandleWithoutLog (IN OUT void **handle, IN const MV_CC_DEVICE_INFO *pstDevInfo) 创建设备句柄，不生成日志 更多...

MV_CAMCTRL_API int __stdcall MV_CC_RegisterImageCallBackForRGB (IN void *handle, IN void(__stdcall *cbOutput)(unsigned char *pData, MV_FRAME_OUT_INFO_EX *pFrameInfo, void *pUser), IN void *pUser) 注册图像数据回调，RGB 更多...

MV_CAMCTRL_API int __stdcall MV_CC_RegisterImageCallBackForBGR (IN void *handle, IN void(__stdcall *cbOutput)(unsigned char *pData, MV_FRAME_OUT_INFO_EX *pFrameInfo, void *pUser), IN void *pUser) 注册图像数据回调，BGR 更多...

MV_CAMCTRL_API int __stdcall MV_CC_GetImageForRGB (IN void *handle, IN OUT unsigned char *pData, IN unsigned int nDataSize, IN OUT MV_FRAME_OUT_INFO_EX *pstFrameInfo, IN int nMsec)

获取一帧RGB数据，此函数为查询式获取，每次调用查询 内部 缓存有无数据，有数据则获取数据，无数据返回错误 码 更多...

MV_CAMCTRL_API int __stdcall MV_CC_GetImageForBGR (IN void *handle, IN OUT unsigned char *pData, IN unsigned int nDataSize, IN OUT MV_FRAME_OUT_INFO_EX *pstFrameInfo, IN int nMsec) 获取一帧BGR数据，此函数为查询式获取，每次调用查询 内部 缓存有无数据，有数据则获取数据，无数据返回错误 码 更多...

MV_CAMCTRL_API int __stdcall MV_CC_OpenParamsGUI (IN void *handle) 打开获取或设置相机参数的GUI界面 更多...

MV_CAMCTRL_API int __stdcall MV_CC_SavePointCloudData (IN void *handle, IN OUT MV_SAVE_POINT_CLOUD_PARAM *pstPointDataParam) 保存3D点云数据，支持PLY、CSV和OBJ三种格式 更多...

### 详细描述

废弃原因：新开发的万能接口代替了具体节点的设置、获取参数功能；效率低的接口由新的接口替 代；实际开发中部分接口使用频率较低。

### 函数说明

### ￿ MV_CC_GetImageInfo()

MV_CAMCTRL_API int __stdcall MV_CC_GetImageInfo ( IN void * handle,

IN OUT MV_IMAGE_BASIC_INFO * pstInfo

)

获取图像基本信息

参数

handle [IN] 相机句柄

pstInfo [IN][OUT] 返回给调用者有关相机图像基本信息结构体指针 返回 成功，返回MV_OK；失败，返回错误码

### ￿ MV_CC_GetTlProxy()

MV_CAMCTRL_API void* __stdcall MV_CC_GetTlProxy ( IN void * handle )

获取GenICam代理

参数

handle [IN] 句柄地址 返回 GenICam代理类指针 ，正常返回值非NULL；异常返回NULL

### ￿ MV_XML_GetRootNode()

MV_CAMCTRL_API int __stdcall MV_XML_GetRootNode ( IN void * handle,

IN OUT MV_XML_NODE_FEATURE * pstNode

)

获取根节点

参数

handle [IN] 句柄

pstNode [OUT] 根节点信息结构体 返回 成功，返回MV_OK；失败，返回错误码

### ￿ MV_XML_GetChildren()

MV_CAMCTRL_API int __stdcall MV_XML_GetChildren ( IN void * handle,

IN MV_XML_NODE_FEATURE * pstNode,

IN OUT MV_XML_NODES_LIST * pstNodesList

)

从xml中获取指定节点的所有子节点，根节点为Root

参数

handle [IN] 句柄

pstNode [IN] 根节点信息结构体

pstNodesList [OUT] 节点列表结构体 返回 成功，返回MV_OK；失败，返回错误码

### ￿ MV_XML_GetNodeFeature()

MV_CAMCTRL_API int __stdcall MV_XML_GetNodeFeature ( IN void * handle,

IN MV_XML_NODE_FEATURE * pstNode,

IN OUT void * pstFeature

)

获得当前节点的属性

参数

handle [IN] 句柄

pstNode [IN] 根节点信息结构体

pstFeature [OUT] 当前节点属性结构体，pstFeature 具体结构体内容参考 MV_XML_FEATURE_x 返回 成功，返回MV_OK；失败，返回错误码

### ￿ MV_XML_UpdateNodeFeature()

MV_CAMCTRL_API int __stdcall MV_XML_UpdateNodeFeature ( IN void * handle,

IN enum MV_XML_InterfaceType enType,

IN void * pstFeature

)

更新节点

参数

handle [IN] 句柄

enType [IN] 节点类型

pstFeature [OUT] 当前节点属性结构体 返回 成功，返回MV_OK；失败，返回错误码

### ￿ MV_XML_RegisterUpdateCallBack()

MV_CAMCTRL_A PI int __stdcallIN MV_XML_Registevoid rUpdateCallBack (* handle,

void__stdcall *cbUpdate)(enum MV_XML_InterfaceType enType, void IN*pstFeature, MV_XML_NODES_LIST *pstNodesList, void *pUser,

IN void
* pUser

)

注册更新回调

参数

handle [IN] 句柄

cbUpdate [IN] 回调函数指针

pUser [IN] 用户自定义变量 返回 成功，返回MV_OK；失败，返回错误码

### ￿ MV_CC_GetOneFrame()

MV_CAMCTRL_API int __stdcall MV_CC_GetOneFrame ( IN void * handle,

IN OUT unsigned char * pData,

IN unsigned int nDataSize,

IN OUT MV_FRAME_OUT_INFO * pFrameInfo

)

获取一帧图像（该接口已弃用，建议改用 MV_CC_GetOneFrameTimeout() 接口）

参数

handle [IN] 句柄

pData [OUT] 图像数据接收指针

nDataSize [IN] 接收缓存大小

pFrameInfo [OUT] 图像信息结构体 返回 成功，返回MV_OK ；失败，返回错误码

注解 此函数为查询式获取，每次调用查询内部缓存有无数据，有数据则范围数据，无数据返 回错误码

### ￿ MV_CC_GetOneFrameEx()

MV_CAMCTRL_API int __stdcall MV_CC_GetOneFrameEx ( IN void * handle,

IN OUT unsigned char * pData,

IN unsigned int nDataSize,

IN OUT MV_FRAME_OUT_INFO_EX * pFrameInfo

)

获取一帧trunck数据（该接口已弃用，建议改用 MV_CC_GetOneFrameTimeout() 接口）

参数

handle [IN] 句柄

pData [OUT] 图像数据接收指针

nDataSize [IN] 接收缓存大小

pFrameInfo [OUT] 图像信息结构体 返回 成功，返回MV_OK ；失败，返回错误码

注解

此函数为查询式获取，每次调用查询内部缓存有无数据，有数据则范围数据，无数据返 回错误码

### ￿ MV_CC_RegisterImageCallBack()

MV_CAMCTRL_API int __stdcall MV_CC_RegisterImahan geCallBack ( void *dle,

void(__stdcall *cbOutput)(unsigned char *pData, MV_FRAME_OUT_INFO *pFrameInfo, void *pUser) ,

pUs void *er

)

注册图像数据回调（该接口已弃用，建议改用 MV_CC_RegisterImageCallBackEx() 接口）

参数

handle [IN] 句柄

cbOutput [IN] 回调函数指针

pUser [IN] 用户自定义变量 返回 成功，返回MV_OK ；失败，返回错误码

### ￿ MV_CC_SaveImageEx()

MV_CAMCTRL_API int __stdcall MV_CC_SaveImageEx ( IN OUT MV_SAVE_IMAGE_PARAM_EX * pSaveParam )

保存图片，支持Bmp和Jpeg，编码质量在50-99之前 （该接口已弃用，建议改用 MV_CC_SaveImageEx3() 接口）

参数

pSaveParam [IN][OUT] 保存图片参数结构体 返回 成功，返回MV_OK ；失败，返回错误码

### ￿ MV_CC_BayerNoiseEstimate()

MV_CAMCTRL_API int __stdcall MV_CC_BayerNoiseEsti mate ( IN void * handle,

IN OUT MV_CC_BAYER_NOISE_ESTIMATE_PARAMpstNoiseEstimatePa *ram

)

Bayer噪声估计

参数

handle [IN] 设备句柄

pstNoiseEstimateParam [IN][OUT] Bayer噪声估计参数结构体 返回 成功，返回MV_OK ；失败，返回错误码

注解 该接口只支持Bayer格式，不支持Bayerxx_Packed；以相机实际支持为准。

### ￿ MV_CC_BayerSpatialDenoise()

MV_CAMCTRL_API int __stdcall MV_CC_BayerSpatialDe noise ( IN void * handle,

IN OUT MV_CC_BAYER_SPATIAL_DENOISE_PARAMpstSpatialDenoisePa *ram

)

Bayer空域降噪

参数

handle [IN] 设备句柄

pstSpatialDenoiseParam [IN][OUT] Bayer空域降噪参数结构体

返回 成功，返回MV_OK ；失败，返回错误码

注解 该接口只支持Bayer格式，不支持Bayerxx_Packed；以相机实际支持为准。

### ￿ MV_CC_SetBayerCLUTParam()

MV_CAMCTRL_API int __stdcall MV_CC_SetBayerCLUTParam ( IN void * handle,

IN MV_CC_CLUT_PARAM * pstCLUTParam

)

设置Bayer格式的CLUT使能和信息

参数

handle [IN] 设备句柄

pstCLUTParam [IN] CLUT参数 返回 成功，返回MV_OK ；错误，返回错误码

注解
- 开启CLUT并设置CLUT信息后，在调用 MV_CC_ConvertPixelType() 、 MV_CC_SaveImageEx2() 接口将Bayer8/10/12/16格式转成RGB24/48， RGBA32/64 ，BGR24/48，BGRA32/64时起效。
- 以相机实际支持为准。

### ￿ MV_CC_ImageSharpen()

MV_CAMCTRL_API int __stdcall MV_CC_ImageSharpen ( IN void * handle,

IN OUT MV_CC_SHARPEN_PARAM * pstSharpenParam

)

图像锐化

参数

handle [IN] 设备句柄

pstSharpenParam [IN] 锐化参数 返回 成功，返回MV_OK ；错误，返回错误码

注解 以相机实际支持为准。

### ￿ MV_CC_ColorCorrect()

MV_CAMCTRL_API int __stdcall MV_CC_ColorCorrect ( IN void * handle,

IN OUT MV_CC_COLOR_CORRECT_PARAM * pstColorCorrectParam

)

色彩校正（包括CCM和CLUT）

参数

handle [IN] 设备句柄

pstColorCorrectParam [IN] 色彩校正参数 返回 成功，返回MV_OK ；错误，返回错误码

注解
- 该接口支持单独CCM或者CLUT，也支持同时进行CCM和CLUT，用户可以通过CCM 和CLUT信息中的使能开关进行选择。
- 以相机实际支持为准。

### ￿ MV_CC_NoiseEstimate()

MV_CAMCTRL_API int __stdcall MV_CC_NoiseEstimat e ( IN void * handle,

IN OUT MV_CC_NOISE_ESTIMATE_PARAMpstNoiseEstimatePara *m

)

噪声估计

参数

handle [IN] 设备句柄

pstNoiseEstimateParam [IN] 噪声估计参数 返回 成功，返回MV_OK ；错误，返回错误码

注解
- 如果用户选择全图做噪声估计，nROINum可输入0，pstROIRect可置空。
- 以相机实际支持为准。

### ￿ MV_CC_SpatialDenoise()

MV_CAMCTRL_API int __stdcall MV_CC_SpatialDenois e ( IN void * handle,

IN OUTpstSpatialDenoisePara MV_CC_SPATIAL_DENOISE_PARAM *m

)

空域降噪

参数

handle [IN] 设备句柄

pstSpatialDenoiseParam [IN] 空域降噪参数 返回 成功，返回MV_OK ；错误，返回错误码

注解 以相机实际支持为准。

### ￿ MV_CC_LSCCalib()

MV_CAMCTRL_API int __stdcall MV_CC_LSCCalib ( IN void * handle,

IN OUT MV_CC_LSC_CALIB_PARAM * pstLSCCalibParam

)

LSC标定

参数

handle [IN] 设备句柄

pstLSCCalibParam [IN] 标定参数 返回 成功，返回MV_OK ；错误，返回错误码

注解 以相机实际支持为准。

示例 LensShadingCorrection.cpp.

### ￿ MV_CC_LSCCorrect()

MV_CAMCTRL_API int __stdcall MV_CC_LSCCorrect ( IN void * handle,

IN OUT MV_CC_LSC_CORRECT_PARAM * pstLSCCorrectParam

)

LSC校正

参数

handle [IN] 设备句柄

pstLSCCorrectParam [IN] 校正参数 返回 成功，返回MV_OK ；错误，返回错误码

注解 以相机实际支持为准。

示例 LensShadingCorrection.cpp.

### ￿ MV_GIGE_ForceIp()

MV_CAMCTRL_API int __stdcall MV_GIGE_ForceIp ( IN void * handle,

unsigned int nIP

)

强制IP（该接口已弃用，建议改用 MV_GIGE_ForceIpEx() 接口）

参数

handle [IN] 设备句柄

nIP [IN] 设置的IP 返回 成功，返回MV_OK ；失败，返回错误码

### ￿ MV_CC_RegisterEventCallBack()

MV_CAMCTRL_API int __stdcall MV_CC_RegisterEventCallhandl Back ( void *e,

void(__stdcall *cbEvent)(unsigned int nExternalEventId, void *pUser) ,

pUser void *

)

注册事件回调（该接口已弃用，建议改用 MV_CC_RegisterEventCallBackEx() 接口）

参数

handle [IN] 设备句柄

cbEvent [IN] 事件回调函数指针

pUser [IN] 用户自定义变量 返回 成功，返回MV_OK ；失败，返回错误码

### ￿ MV_CC_Display()

MV_CAMCTRL_API int __stdcall MV_CC_Display ( IN void * handle,

void * hWnd

)

显示图像，注册显示窗口，内部自动显示（该接口已弃用，建议改用 MV_CC_DisplayOneFrameEx() 接口）

参数

handle [IN] 句柄

hWnd [IN] 显示窗口句柄 返回 成功，返回MV_OK ；失败，返回错误码

### ￿ MV_CC_DisplayOneFrame()

MV_CAMCTRL_API int __stdcall MV_CC_DisplayOneFrame ( IN void * handle,

IN MV_DISPLAY_FRAME_INFO * pstDisplayInfo

)

显示一帧图像（该接口已弃用，建议改用 MV_CC_DisplayOneFrameEx2() 接口）

显示一帧图像（该接口已启用，建议改用 MV_CC_DisplayOneFrameEx2() 接口）

参数

handle [IN] 设备句柄

pstDisplayInfo [IN] 图像信息 返回 成功，返回MV_OK；失败，返回错误码

注解
- 渲染方式可选择GDI或D3D，默认选择为GDI模式。渲染引擎仅在客户端不连接相机的 情况下可以进行设置。

- GDI模式对电脑的显卡性能没有要求，适用于所有电脑。
- D3D模式适用于安装显卡驱动且显卡内存大于1GB的电脑，该模式下客户端预览 的图像效果会优于GDI模式下的图像效果。

参数

handle [IN] 设备句柄

pstDisplayInfo [IN] 图像信息 返回 成功，返回MV_OK；失败，返回错误码

注解 与设备类型无关，渲染模式为D3D时，支持的最大分辨率为16384 * 163840

### ￿ MV_CC_GetIntValue()

MV_CAMCTRL_API int __stdcall MV_CC_GetIntValue ( IN void * handle,

IN const char * strKey,

OUT MVCC_INTVALUE * pIntValue

)

获取Integer属性值

参数

handle [IN] 设备句柄

strKey [IN] 属性键值，如获取宽度信息则为"Width"

pIntValue [IN][OUT] 返回给调用者有关相机属性结构体指针 返回 成功，返回MV_OK；失败，返回错误码

注解 连接设备之后调用该接口可以获取int类型的指定节点的值，具体可以查看客户端属性描 述，如下图所示：

示例 HighBandwidthDecode.cpp, ImageSave.cpp, MultiLightCtrl_ImageStitching.cpp , 以及 Recording.cpp.

### ￿ MV_CC_SetIntValue()

MV_CAMCTRL_API int __stdcall MV_CC_SetIntValue ( IN void * handle,

IN const char * strKey,

IN unsigned int nValue

)

设置Integer型属性值

参数

handle [IN] 设备句柄

strKey [IN] 属性键值，如获取宽度信息则为"Width"

nValue [IN] 想要设置的相机的属性值 返回 成功，返回MV_OK；失败，返回错误码

注解 连接设备之后调用该接口可以设置int类型的指定节点的值，具体可以查看客户端属性描 述，如下图所示：

示例 ImageSave.cpp , 以及 MultiLightCtrl_ImageStitching.cpp.

### ￿ MV_CC_GetWidth()

MV_CAMCTRL_API int __stdcall MV_CC_GetWidth ( IN void * handle,

IN OUT MVCC_INTVALUE * pstValue

)

获取图像宽度

参数

handle [IN] 相机句柄

pstValue [IN][OUT] 返回给调用者有关相机宽度的信息结构体指针 返回的pstValue结构体的意义： 变量 意义

- unsigned 代表相机当前的宽度值 int nCurValue

- unsigned int nMax 表示相机允许的最大可设置的宽度值

- unsigned int nMin 表示相机允许的最小可设置的宽度值

表示相机设置的宽度增量必须是nInc的倍数，否
- unsigned int nInc 则无效

返回 成功，返回MV_OK ，并获得相应参数信息的结构体；失败，返回错误码

注解 其他整型结构体参数的接口可参照此接口

### ￿ MV_CC_SetWidth()

MV_CAMCTRL_API int __stdcall MV_CC_SetWidth ( IN void * handle,

IN const unsigned int nValue

)

设置图像宽度

参数

handle [IN] 相机句柄

nValue [IN] 想要设置的相机宽度的值,注意此宽度值必须是 MV_CC_GetWidth() 接口返回 的pstValue中的nInc的倍数才能设置成功

返回 成功，返回MV_OK ，并且相机宽度将会更改为相应值；失败，返回错误码

### ￿ MV_CC_GetHeight()

MV_CAMCTRL_API int __stdcall MV_CC_GetHeight ( IN void * handle,

IN OUT MVCC_INTVALUE * pstValue

)

获取图像高度

参数

handle [IN] 相机句柄

pstValue [IN][OUT] 返回给调用者有关相机高度的信息结构体指针 返回 成功，返回MV_OK ,并将高度信息返回到结构体中；失败，返回错误码

注解 可参照接口 MV_CC_GetWidth()

### ￿ MV_CC_SetHeight()

MV_CAMCTRL_API int __stdcall MV_CC_SetHeight ( IN void * handle,

IN const unsigned int nValue

)

设置图像高度

参数

handle [IN] 相机句柄

nValue [IN] 想要设置的相机宽度的值,注意此宽度值必须是 MV_CC_GetWidth() 接口返回 的pstValue中的nInc的倍数才能设置成功

返回 成功，返回MV_OK ，并且相机高度将会更改为相应值；失败，返回错误码

### ￿ MV_CC_GetAOIoffsetX()

MV_CAMCTRL_API int __stdcall MV_CC_GetAOIoffsetX ( IN void * handle,

IN OUT MVCC_INTVALUE * pstValue

)

获取图像X偏移

参数

handle [IN] 相机句柄

pstValue [IN][OUT] 返回给调用者有关相机X偏移的信息结构体指针 返回 成功，返回MV_OK ；失败，返回错误码

注解 可参照接口 MV_CC_GetWidth()

### ￿ MV_CC_SetAOIoffsetX()

MV_CAMCTRL_API int __stdcall MV_CC_SetAOIoffsetX ( IN void * handle,

IN const unsigned int nValue

)

设置图像AOI偏移

参数

handle [IN] 相机句柄

nValue [IN] 想要设置的相机AOI的值 返回 成功，返回MV_OK ，并且相机AOI偏移将会更改为相应值；失败，返回错误码

### ￿ MV_CC_GetAOIoffsetY()

MV_CAMCTRL_API int __stdcall MV_CC_GetAOIoffsetY ( IN void * handle,

IN OUT MVCC_INTVALUE * pstValue

)

获取图像Y偏移

参数

handle [IN] 相机句柄

pstValue [IN][OUT] 返回给调用者有关相机Y偏移的信息结构体指针 返回 成功，返回MV_OK ；失败，返回错误码

注解 可参照接口 MV_CC_GetWidth()

### ￿ MV_CC_SetAOIoffsetY()

MV_CAMCTRL_API int __stdcall MV_CC_SetAOIoffsetY ( IN void * handle,

IN const unsigned int nValue

)

设置图像AOI偏移

参数

handle [IN] 相机句柄

nValue [IN] 想要设置的相机AOI的值 返回 成功，返回MV_OK ，并且相机AOI偏移将会更改为相应值；失败，返回错误码

### ￿ MV_CC_GetAutoExposureTimeLower()

MV_CAMCTRL_API int __stdcall MV_CC_GetAutoExposureTimeLower ( IN void * handle,

IN OUT MVCC_INTVALUE * pstValue

)

获取曝光下限

参数

handle [IN] 相机句柄

pstValue [IN][OUT] 返回给调用者有关相机曝光值下限结构体指针 返回 成功，返回MV_OK ；失败，返回错误码

注解 可参照接口 MV_CC_GetWidth()

### ￿ MV_CC_SetAutoExposureTimeLower()

MV_CAMCTRL_API int __stdcall MV_CC_SetAutoExposureTimeLower ( IN void * handle,

IN const unsigned int nValue

)

设置曝光值下限

参数

handle [IN] 相机句柄

nValue [IN] 想要设置的曝光值下限 返回 成功，返回MV_OK ，并且相机曝光下限将会更改为相应值；失败，返回错误码

### ￿ MV_CC_GetAutoExposureTimeUpper()

MV_CAMCTRL_API int __stdcall MV_CC_GetAutoExposureTimeUpper ( IN void * handle,

IN OUT MVCC_INTVALUE * pstValue

)

获取曝光上限

参数

handle [IN] 相机句柄

pstValue [IN][OUT] 返回给调用者有关相机曝光值上限结构体指针 返回 成功，返回MV_OK ；失败，返回错误码

注解

可参照接口 MV_CC_GetWidth()

### ￿ MV_CC_SetAutoExposureTimeUpper()

MV_CAMCTRL_API int __stdcall MV_CC_SetAutoExposureTimeUpper ( IN void * handle,

IN const unsigned int nValue

)

设置曝光值上限

参数

handle [IN] 相机句柄

nValue [IN] 想要设置的曝光值上限 返回 成功，返回MV_OK ，并且相机曝光上限将会更改为相应值；失败，返回错误码

### ￿ MV_CC_GetBrightness()

MV_CAMCTRL_API int __stdcall MV_CC_GetBrightness ( IN void * handle,

IN OUT MVCC_INTVALUE * pstValue

)

获取亮度值

参数

handle [IN] 相机句柄

pstValue [IN][OUT] 返回给调用者有关相机亮度结构体指针 返回 成功，返回MV_OK ；失败，返回错误码

注解 可参照接口 MV_CC_GetWidth()

### ￿ MV_CC_SetBrightness()

MV_CAMCTRL_API int __stdcall MV_CC_SetBrightness ( IN void * handle,

IN const unsigned int nValue

)

设置亮度值

参数

handle [IN] 相机句柄

nValue [IN] 想要设置的亮度值 返回 成功，返回MV_OK ，并且相机亮度将会更改为相应值；失败，返回错误码

### ￿ MV_CC_GetFrameRate()

MV_CAMCTRL_API int __stdcall MV_CC_GetFrameRate ( IN void * handle,

IN OUT MVCC_FLOATVALUE * pstValue

)

获取帧率

参数

handle [IN] 相机句柄

pstValue [IN][OUT] 返回给调用者有关相机帧率的信息结构体指针 返回的pstValue结构体的意义： 变量 意义

- float fCurValue 表示相机当前的帧率

- float fMax 表示相机允许设置的最大帧率

float fMin 表示相机允许设置的最小帧率

返回 成功，返回MV_OK ，并获得相应参数信息的结构体；失败，返回错误码

注解 其他浮点型结构体参数的接口可参照此接口

### ￿ MV_CC_SetFrameRate()

MV_CAMCTRL_API int __stdcall MV_CC_SetFrameRate ( IN void * handle,

IN const float fValue

)

设置帧率

参数

handle [IN] 相机句柄

fValue [IN] 想要设置的相机帧率 返回 成功，返回MV_OK ，并且相机帧率将会更改为相应值；失败，返回错误码

### ￿ MV_CC_GetGain()

MV_CAMCTRL_API int __stdcall MV_CC_GetGain ( IN void * handle,

IN OUT MVCC_FLOATVALUE * pstValue

)

获取增益

参数

handle [IN] 相机句柄

pstValue [IN][OUT] 返回给调用者有关相机增益的信息结构体指针 返回的pstValue结构体的意义： 变量 意义

- float fCurValue 表示相机当前的增益

- float fMax 表示相机允许设置的最大增益

- float &nbsp 表示相机允许设置的最小增益

返回 成功，返回MV_OK ，并获得相应参数信息的结构体；失败，返回错误码

注解 其他浮点型结构体参数的接口可参照此接口

### ￿ MV_CC_SetGain()

MV_CAMCTRL_API int __stdcall MV_CC_SetGain ( IN void * handle,

IN const float fValue

)

设置增益

参数

handle [IN] 相机句柄

fValue [IN] 想要设置的相机增益 返回 成功，返回MV_OK ，并且相机帧率将会更改为相应值；失败，返回错误码

### ￿ MV_CC_GetExposureTime()

MV_CAMCTRL_API int __stdcall MV_CC_GetExposureTime ( IN void * handle,

IN OUT MVCC_FLOATVALUE * pstValue

)

获取曝光时间

参数

handle [IN] 相机句柄

pstValue [IN][OUT] 返回给调用者有关相机曝光时间的信息结构体指针 返回 成功，返回MV_OK ，并获得相应参数信息的结构体；失败，返回错误码

### ￿ MV_CC_SetExposureTime()

MV_CAMCTRL_API int __stdcall MV_CC_SetExposureTime ( IN void * handle,

IN const float fValue

)

设置曝光时间

参数

handle [IN] 相机句柄

fValue [IN] 想要设置的曝光时间 返回 成功，返回MV_OK ，并且相机帧率将会更改为相应值；失败，返回错误码

### ￿ MV_CC_GetPixelFormat()

MV_CAMCTRL_API int __stdcall MV_CC_GetPixelFormat ( IN void * handle,

IN OUT MVCC_ENUMVALUE * pstValue

)

获取像素格式

参数

handle [IN] 相机句柄

pstValue [IN][OUT] 返回给调用者的有关像素格式的信息结构体指针 返回 成功，返回MV_OK ，并获得相应参数信息的结构体；失败，返回错误码

### ￿ MV_CC_SetPixelFormat()

MV_CAMCTRL_API int __stdcall MV_CC_SetPixelFormat ( IN void * handle,

IN const unsigned int nValue

)

设置像素格式

参数

handle [IN] 相机句柄

nValue [IN] 要设置的像素格式对应的整型值，调用此接口时可以直接填写枚举值，如 MV_CC_SetPixelFormat() ; 返回 成功，返回MV_OK ，并且相机像素格式将会更改为相应值；失败，返回错误码

注解 要设置的枚举类型必须是Get接口返回的 nSupportValue[MV_MAX_XML_SYMBOLIC_NUM]中的一种，否则会失败

### ￿ MV_CC_GetAcquisitionMode()

MV_CAMCTRL_API int __stdcall MV_CC_GetAcquisitionMode ( IN void * handle,

IN OUT MVCC_ENUMVALUE * pstValue

)

获取采集模式

参数

handle [IN] 相机句柄

pstValue [IN][OUT] 返回给调用者的有关采集模式的信息结构体指针 返回 成功，返回MV_OK ，并获得相应参数信息的结构体；失败，返回错误码

注解 可参照接口 MV_CC_GetPixelFormat() ，参考 CameraParams.h 中的 MV_CAM_ACQUISITION_MODE 定义

### ￿ MV_CC_SetAcquisitionMode()

MV_CAMCTRL_API int __stdcall MV_CC_SetAcquisitionMode ( IN void * handle,

IN const unsigned int nValue

)

设置像素格式

参数

handle [IN] 相机句柄

nValue [IN] 要设置的采集模式对应的整型值 返回 成功，返回MV_OK ，并且相机采集模式将会更改为相应值；失败，返回错误码

### ￿ MV_CC_GetGainMode()

MV_CAMCTRL_API int __stdcall MV_CC_GetGainMode ( IN void * handle,

IN OUT MVCC_ENUMVALUE * pstValue

)

获取增益模式

参数

handle [IN] 相机句柄

pstValue [IN][OUT] 返回给调用者的有关增益模式的信息结构体指针 返回 成功，返回MV_OK ，并获得相应参数信息的结构体；失败，返回错误码

注解 可参照接口 MV_CC_GetPixelFormat() ，参考 CameraParams.h 中的 MV_CAM_GAIN_MODE 定义

### ￿ MV_CC_SetGainMode()

MV_CAMCTRL_API int __stdcall MV_CC_SetGainMode ( IN void * handle,

IN const unsigned int nValue

)

设置增益模式

参数

handle [IN] 相机句柄

nValue [IN] 要设置的增益模式对应的整型值 返回 成功，返回MV_OK ，并且相机增益模式将会更改为相应值；失败，返回错误码

### ￿ MV_CC_GetExposureAutoMode()

MV_CAMCTRL_API int __stdcall MV_CC_GetExposureAutoMode ( IN void * handle,

IN OUT MVCC_ENUMVALUE * pstValue

)

获取自动曝光模式

参数

handle [IN] 相机句柄

pstValue [IN][OUT] 返回给调用者的有关自动曝光模式的信息结构体指针 返回 成功，返回MV_OK ，并获得相应参数信息的结构体；失败，返回错误码

注解 可参照接口 MV_CC_GetPixelFormat() ，参考 CameraParams.h 中的 ::MV_CAM_EXPOSURE_AUTO_MODE 定义

### ￿ MV_CC_SetExposureAutoMode()

MV_CAMCTRL_API int __stdcall MV_CC_SetExposureAutoMode ( IN void * handle,

IN const unsigned int nValue

)

设置自动曝光模式

参数

handle [IN] 相机句柄

nValue [IN] 要设置的自动曝光模式对应的整型值 返回 成功，返回MV_OK ，并且相机自动曝光模式将会更改为相应值；失败，返回错误码

### ￿ MV_CC_GetTriggerMode()

MV_CAMCTRL_API int __stdcall MV_CC_GetTriggerMode ( IN void * handle,

IN OUT MVCC_ENUMVALUE * pstValue

)

获取触发模式

参数

handle [IN] 相机句柄

pstValue [IN][OUT] 返回给调用者的有关触发模式的信息结构体指针

返回 成功，返回MV_OK ，并获得相应参数信息的结构体；失败，返回错误码

注解 可参照接口 MV_CC_GetPixelFormat() ，参考 CameraParams.h 中的 MV_CAM_TRIGGER_MODE 定义

### ￿ MV_CC_SetTriggerMode()

MV_CAMCTRL_API int __stdcall MV_CC_SetTriggerMode ( IN void * handle,

IN const unsigned int nValue

)

设置触发模式

参数

handle [IN] 相机句柄

nValue [IN] 要设置的触发模式对应的整型值 返回 成功，返回MV_OK ，并且相机触发模式将会更改为相应值；失败，返回错误码

### ￿ MV_CC_GetTriggerDelay()

MV_CAMCTRL_API int __stdcall MV_CC_GetTriggerDelay ( IN void * handle,

IN OUT MVCC_FLOATVALUE * pstValue

)

获取触发延时

参数

handle [IN] 相机句柄

pstValue [IN][OUT] 返回给调用者有关相机触发延时的信息结构体指针 返回 成功，返回MV_OK ，并获得相应参数信息的结构体；失败，返回错误码

注解 可参照接口 MV_CC_GetFrameRate()

### ￿ MV_CC_SetTriggerDelay()

MV_CAMCTRL_API int __stdcall MV_CC_SetTriggerDelay ( IN void * handle,

IN const float fValue

)

设置触发延时

参数

handle [IN] 相机句柄

fValue [IN] 想要设置的相机触发延时 返回 成功，返回MV_OK ，并且相机触发延时将会更改为相应值；失败，返回错误码

### ￿ MV_CC_GetTriggerSource()

MV_CAMCTRL_API int __stdcall MV_CC_GetTriggerSource ( IN void * handle,

IN OUT MVCC_ENUMVALUE * pstValue

)

获取触发源

参数

handle [IN] 相机句柄

pstValue [IN][OUT] 返回给调用者的有关触发源的信息结构体指针 返回 成功，返回MV_OK ，并获得相应参数信息的结构体；失败，返回错误码

注解 可参照接口 MV_CC_GetPixelFormat() ，参考 CameraParams.h 中的 MV_CAM_TRIGGER_SOURCE 定义

### ￿ MV_CC_SetTriggerSource()

MV_CAMCTRL_API int __stdcall MV_CC_SetTriggerSource ( IN void * handle,

IN const unsigned int nValue

)

设置触发源

参数

handle [IN] 相机句柄

nValue [IN] 要设置的触发源对应的整型值 返回 成功，返回MV_OK ，并且相机触发源将会更改为相应值；失败，返回错误码

### ￿ MV_CC_TriggerSoftwareExecute()

MV_CAMCTRL_API int __stdcall MV_CC_TriggerSoftwareExecute ( IN void * handle )

软触发一次（接口仅在已选择的触发源为软件触发时有效）

参数

handle [IN] 相机句柄 返回 成功，返回MV_OK , 失败,返回错误码

### ￿ MV_CC_GetGammaSelector()

MV_CAMCTRL_API int __stdcall MV_CC_GetGammaSelector ( IN void * handle,

IN OUT MVCC_ENUMVALUE * pstValue

)

获取Gamma类型

参数

handle [IN] 相机句柄

pstValue [IN][OUT] 返回给调用者的有关Gamma类型的信息结构体指针 返回 成功，返回MV_OK ，并获得相应参数信息的结构体；失败，返回错误码

注解

可参照接口 MV_CC_GetPixelFormat() ，参考 CameraParams.h 中的 MV_CAM_GAMMA_SELECTOR 定义

### ￿ MV_CC_SetGammaSelector()

MV_CAMCTRL_API int __stdcall MV_CC_SetGammaSelector ( IN void * handle,

IN const unsigned int nValue

)

设置Gamma类型

参数

handle [IN] 相机句柄

nValue [IN] 要设置的Gamma类型对应的整型值 返回 成功，返回MV_OK ，并且相机Gamma类型将会更改为相应值；失败，返回错误码

### ￿ MV_CC_GetGamma()

MV_CAMCTRL_API int __stdcall MV_CC_GetGamma ( IN void * handle,

IN OUT MVCC_FLOATVALUE * pstValue

)

获取Gamma值

参数

handle [IN] 相机句柄

pstValue [IN][OUT] 返回给调用者有关相机Gamma值的信息结构体指针 返回 成功，返回MV_OK ，并获得相应参数信息的结构体；失败，返回错误码

注解 可参照接口 MV_CC_GetExposureTime()

### ￿ MV_CC_SetGamma()

MV_CAMCTRL_API int __stdcall MV_CC_SetGamma ( IN void * handle,

IN const float fValue

)

设置Gamma值

参数

handle [IN] 相机句柄

fValue [IN] 想要设置的相机Gamma值 返回 成功，返回MV_OK ，并且相机Gamma值将会更改为相应值；失败，返回错误码

### ￿ MV_CC_GetSharpness()

MV_CAMCTRL_API int __stdcall MV_CC_GetSharpness ( IN void * handle,

IN OUT MVCC_INTVALUE * pstValue

)

获取锐度

参数

handle [IN] 相机句柄

pstValue [IN][OUT] 返回给调用者有关相机锐度结构体指针 返回 成功，返回MV_OK ；失败，返回错误码

注解 可参照接口 MV_CC_GetWidth()

### ￿ MV_CC_SetSharpness()

MV_CAMCTRL_API int __stdcall MV_CC_SetSharpness ( IN void * handle,

IN const unsigned int nValue

)

设置锐度

参数

handle [IN] 相机句柄

nValue [IN] 想要设置的锐度 返回 成功，返回MV_OK ，并且相机锐度将会更改为相应值；失败，返回错误码

### ￿ MV_CC_GetHue()

MV_CAMCTRL_API int __stdcall MV_CC_GetHue ( IN void * handle,

IN OUT MVCC_INTVALUE * pstValue

)

获取灰度

参数

handle [IN] 相机句柄

pstValue [IN][OUT] 返回给调用者有关相机灰度结构体指针 返回 成功，返回MV_OK ；失败，返回错误码

注解 可参照接口 MV_CC_GetWidth()

### ￿ MV_CC_SetHue()

MV_CAMCTRL_API int __stdcall MV_CC_SetHue ( IN void * handle,

IN const unsigned int nValue

)

设置灰度

参数

handle [IN] 相机句柄

nValue [IN] 想要设置的灰度 返回 成功，返回MV_OK ，并且相机灰度将会更改为相应值；失败，返回错误码

### ￿ MV_CC_GetSaturation()

MV_CAMCTRL_API int __stdcall MV_CC_GetSaturation ( IN void * handle,

IN OUT MVCC_INTVALUE * pstValue

)

获取饱和度

参数

handle [IN] 相机句柄

pstValue [IN][OUT] 返回给调用者有关相机饱和度结构体指针 返回 成功，返回MV_OK ；失败，返回错误码

注解 可参照接口 MV_CC_GetWidth()

### ￿ MV_CC_SetSaturation()

MV_CAMCTRL_API int __stdcall MV_CC_SetSaturation ( IN void * handle,

IN const unsigned int nValue

)

设置饱和度

参数

handle [IN] 相机句柄

nValue [IN] 想要设置的饱和度 返回 成功，返回MV_OK ，并且相机饱和度将会更改为相应值；失败，返回错误码

### ￿ MV_CC_GetBalanceWhiteAuto()

MV_CAMCTRL_API int __stdcall MV_CC_GetBalanceWhiteAuto ( IN void * handle,

IN OUT MVCC_ENUMVALUE * pstValue

)

获取自动白平衡

参数

handle [IN] 相机句柄

pstValue [IN][OUT] 返回给调用者的有关自动白平衡的信息结构体指针 返回 成功，返回MV_OK ，并获得相应参数信息的结构体；失败，返回错误码

注解 可参照接口 MV_CC_GetPixelFormat() ，参考 CameraParams.h 中的 MV_CAM_BALANCEWHITE_AUTO 定义

### ￿ MV_CC_SetBalanceWhiteAuto()

MV_CAMCTRL_API int __stdcall MV_CC_SetBalanceWhiteAuto ( IN void * handle,

IN const unsigned int nValue

)

设置自动白平衡

参数

handle [IN] 相机句柄

nValue [IN] 要设置的自动白平衡对应的整型值 返回 成功，返回MV_OK ，并且相机自动白平衡将会更改为相应值；失败，返回错误码

### ￿ MV_CC_GetBalanceRatioRed()

MV_CAMCTRL_API int __stdcall MV_CC_GetBalanceRatioRed ( IN void * handle,

IN OUT MVCC_INTVALUE * pstValue

)

获取白平衡 红

参数

handle [IN] 相机句柄

pstValue [IN][OUT] 返回给调用者有关相机白平衡 红结构体指针 返回 成功，返回MV_OK ；失败，返回错误码

注解 可参照接口 MV_CC_GetWidth()

### ￿ MV_CC_SetBalanceRatioRed()

MV_CAMCTRL_API int __stdcall MV_CC_SetBalanceRatioRed ( IN void * handle,

IN const unsigned int nValue

)

设置白平衡 红

参数

handle [IN] 相机句柄

nValue [IN] 想要设置的白平衡 红 返回 成功，返回MV_OK ，并且相机白平衡 红将会更改为相应值；失败，返回错误码

### ￿ MV_CC_GetBalanceRatioGreen()

MV_CAMCTRL_API int __stdcall MV_CC_GetBalanceRatioGreen ( IN void * handle,

IN OUT MVCC_INTVALUE * pstValue

)

获取白平衡 绿

参数

handle [IN] 相机句柄

pstValue [IN][OUT] 返回给调用者有关相机白平衡 绿结构体指针 返回 成功，返回MV_OK ；失败，返回错误码

注解

可参照接口 MV_CC_GetWidth()

### ￿ MV_CC_SetBalanceRatioGreen()

MV_CAMCTRL_API int __stdcall MV_CC_SetBalanceRatioGreen ( IN void * handle,

IN const unsigned int nValue

)

设置白平衡 绿

参数

handle [IN] 相机句柄

nValue [IN] 想要设置的白平衡 绿 返回 成功，返回MV_OK ，并且相机白平衡 绿将会更改为相应值；失败，返回错误码

### ￿ MV_CC_GetBalanceRatioBlue()

MV_CAMCTRL_API int __stdcall MV_CC_GetBalanceRatioBlue ( IN void * handle,

IN OUT MVCC_INTVALUE * pstValue

)

获取白平衡 蓝

参数

handle [IN] 相机句柄

pstValue [IN][OUT] 返回给调用者有关相机白平衡 蓝结构体指针 返回 成功，返回MV_OK ；失败，返回错误码

注解 可参照接口 MV_CC_GetWidth()

### ￿ MV_CC_SetBalanceRatioBlue()

MV_CAMCTRL_API int __stdcall MV_CC_SetBalanceRatioBlue ( IN void * handle,

IN const unsigned int nValue

)

设置白平衡 蓝

参数

handle [IN] 相机句柄

nValue [IN] 想要设置的白平衡 蓝 返回 成功，返回MV_OK ，并且相机白平衡 蓝将会更改为相应值；失败，返回错误码

### ￿ MV_CC_GetFrameSpecInfoAbility()

MV_CAMCTRL_API int __stdcall MV_CC_GetFrameSpecInfoAbility ( IN void * handle,

IN OUT MVCC_INTVALUE * pstValue

)

获取水印信息内包含的信息类型

参数

handle [IN] 相机句柄

pstValue [IN][OUT] 返回给调用者有关相机水印信息内包含的信息类型结构体指针 返回 成功，返回MV_OK ；失败，返回错误码

注解 可参照接口 MV_CC_GetWidth()

### ￿ MV_CC_SetFrameSpecInfoAbility()

MV_CAMCTRL_API int __stdcall MV_CC_SetFrameSpecInfoAbility ( IN void * handle,

IN const unsigned int nValue

)

设置水印信息内包含的信息类型

参数

handle [IN] 相机句柄

nValue [IN] 想要设置的水印信息内包含的信息类型 返回 成功，返回MV_OK ，并且相机水印信息内包含的信息类型会更改为相应值；失败，返 回错误码

### ￿ MV_CC_GetDeviceUserID()

MV_CAMCTRL_API int __stdcall MV_CC_GetDeviceUserID ( IN void * handle,

IN OUT MVCC_STRINGVALUE * pstValue

)

获取设备自定义名字

参数

handle [IN] 相机句柄

pstValue [IN][OUT] 返回给调用者有关相机名字结构体指针 返回 成功，返回MV_OK ，并且获取到相机的自定义名字；失败，返回错误码

### ￿ MV_CC_SetDeviceUserID()

MV_CAMCTRL_API int __stdcall MV_CC_SetDeviceUserID ( IN void * handle,

IN const char * chValue

)

设置设备自定义名字

参数

handle [IN] 相机句柄

chValue [IN] 设备名字 返回 成功，返回MV_OK ，并且设置设备自定义名字；失败，返回错误码

### ￿ MV_CC_GetBurstFrameCount()

MV_CAMCTRL_API int __stdcall MV_CC_GetBurstFrameCount ( IN void * handle,

IN OUT MVCC_INTVALUE * pstValue

)

获取一次触发的帧数

参数

handle [IN] 相机句柄

pstValue [IN][OUT] 返回给调用者有关相机一次触发的帧数结构体指针 返回 成功，返回MV_OK ；失败，返回错误码

注解 可参照接口 MV_CC_GetWidth()

### ￿ MV_CC_SetBurstFrameCount()

MV_CAMCTRL_API int __stdcall MV_CC_SetBurstFrameCount ( IN void * handle,

IN const unsigned int nValue

)

设置一次触发的帧数

参数

handle [IN] 相机句柄

nValue [IN] 想要设置的一次触发的帧数 返回 成功，返回MV_OK ，并且相机一次触发的帧数会更改为相应值；失败，返回错误码

### ￿ MV_CC_GetAcquisitionLineRate()

MV_CAMCTRL_API int __stdcall MV_CC_GetAcquisitionLineRate ( IN void * handle,

IN OUT MVCC_INTVALUE * pstValue

)

获取行频

参数

handle [IN] 相机句柄

pstValue [IN][OUT] 返回给调用者有关相机行频结构体指针 返回 成功，返回MV_OK ；失败，返回错误码

注解 可参照接口 MV_CC_GetWidth()

### ￿ MV_CC_SetAcquisitionLineRate()

MV_CAMCTRL_API int __stdcall MV_CC_SetAcquisitionLineRate ( IN void * handle,

IN const unsigned int nValue

)

设置行频

参数

handle [IN] 相机句柄

nValue [IN] 想要设置的行频 返回 成功，返回MV_OK ，并且相机行频会更改为相应值；失败，返回错误码

### ￿ MV_CC_GetHeartBeatTimeout()

MV_CAMCTRL_API int __stdcall MV_CC_GetHeartBeatTimeout ( IN void * handle,

IN OUT MVCC_INTVALUE * pstValue

)

获取心跳信息

参数

handle [IN] 相机句柄

pstValue [IN][OUT] 返回给调用者有关相机心跳信息结构体指针 返回 成功，返回MV_OK ；失败，返回错误码

注解 可参照接口 MV_CC_GetWidth()

### ￿ MV_CC_SetHeartBeatTimeout()

MV_CAMCTRL_API int __stdcall MV_CC_SetHeartBeatTimeout ( IN void * handle,

IN const unsigned int nValue

)

设置心跳信息

参数

handle [IN] 相机句柄

nValue [IN] 想要设置的心跳信息 返回 成功，返回MV_OK ，并且相机心跳信息会更改为相应值；失败，返回错误码

### ￿ MV_GIGE_GetGevSCPSPacketSize()

MV_CAMCTRL_API int __stdcall MV_GIGE_GetGevSCPSPacketSize ( IN void * handle,

IN OUT MVCC_INTVALUE * pstValue

)

获取网络包大小

参数

handle [IN] 相机句柄

pstValue [IN][OUT] 返回给调用者有关相机网络包大小结构体指针 返回 成功，返回MV_OK ；失败，返回错误码

注解

可参照接口 MV_CC_GetWidth()

### ￿ MV_GIGE_SetGevSCPSPacketSize()

MV_CAMCTRL_API int __stdcall MV_GIGE_SetGevSCPSPacketSize ( IN void * handle,

IN const unsigned int nValue

)

设置网络包大小

参数

handle [IN] 相机句柄

nValue [IN] 想要设置的网络包大小 返回 成功，返回MV_OK ，并且相机网络包大小会更改为相应值；失败，返回错误码

### ￿ MV_GIGE_GetGevSCPD()

MV_CAMCTRL_API int __stdcall MV_GIGE_GetGevSCPD ( IN void * handle,

IN OUT MVCC_INTVALUE * pstValue

)

获取网络包发送间隔

参数

handle [IN] 相机句柄

pstValue [IN][OUT] 返回给调用者有关相机网络包发送间隔结构体指针 返回 成功，返回MV_OK ；失败，返回错误码

注解 可参照接口 MV_CC_GetWidth()

### ￿ MV_GIGE_SetGevSCPD()

MV_CAMCTRL_API int __stdcall MV_GIGE_SetGevSCPD ( IN void * handle,

IN const unsigned int nValue

)

设置网络包发送间隔

参数

handle [IN] 相机句柄

nValue [IN] 想要设置的网络包发送间隔 返回 成功，返回MV_OK ，并且相机网络包发送间隔会更改为相应值；失败，返回错误码

### ￿ MV_GIGE_GetGevSCDA()

MV_CAMCTRL_API int __stdcall MV_GIGE_GetGevSCDA ( IN void * handle,

unsigned int * pnIP

)

获取接收端IP地址，0xa9fe0102 表示 169.254.1.2

参数

handle [IN] 相机句柄

pnIP [IN][OUT] 返回给调用者接收端IP地址 返回 成功，返回MV_OK ；失败，返回错误码

### ￿ MV_GIGE_SetGevSCDA()

MV_CAMCTRL_API int __stdcall MV_GIGE_SetGevSCDA ( IN void * handle,

unsigned int nIP

)

设置接收端IP地址

参数

handle [IN] 相机句柄

nIP [IN] 想要设置的接收端IP地址

返回 成功，返回MV_OK ，并且相机接收端IP地址会更改为相应值；失败，返回错误码

### ￿ MV_GIGE_GetGevSCSP()

MV_CAMCTRL_API int __stdcall MV_GIGE_GetGevSCSP ( IN void * handle,

unsigned int * pnPort

)

获取发送端的端口号

参数

handle [IN] 相机句柄

pnPort [IN][OUT] 返回给调用者发送端的端口号 返回 成功，返回MV_OK ；失败，返回错误码

### ￿ MV_GIGE_SetGevSCSP()

MV_CAMCTRL_API int __stdcall MV_GIGE_SetGevSCSP ( IN void * handle,

unsigned int nPort

)

设置发送端的端口号

参数

handle [IN] 相机句柄

nPort [IN] 想要设置的发送端的端口号 返回 成功，返回MV_OK ，并且相机发送端的端口号会更改为相应值；失败，返回错误码

### ￿ MV_CAML_SetDeviceBauderate()

MV_CAMCTRL_API int __stdcall MV_CAML_SetDeviceBauderate ( IN void * handle,

unsigned int nBaudrate

)

设置设备波特率

参数

handle [IN] 设备句柄

nBaudrate [IN] 设置的波特率值，数值参考CameraParams.h中宏定义 返回 成功，返回MV_OK ；失败，返回错误码

注解 （该接口已弃用，建议改用 MV_CAML_SetDeviceBaudrate() 接口）

### ￿ MV_CAML_GetDeviceBauderate()

MV_CAMCTRL_API int __stdcall MV_CAML_GetDeviceBauderate ( IN void * handle,

unsigned int * pnCurrentBaudrate

)

获取设备波特率

参数

handle [IN] 设备句柄

pnCurrentBaudrate [OUT] 波特率信息指针，数值参考CameraParams.h中宏定义 返回 成功，返回MV_OK ；失败，返回错误码

注解 （该接口已弃用，建议改用 MV_CAML_GetDeviceBaudrate() 接口）

### ￿ MV_CAML_GetSupportBauderates()

MV_CAMCTRL_API int __stdcall MV_CAML_GetSupportBauderates ( IN void * handle,

unsigned int * pnBaudrateAblity

)

获取设备与主机间连接支持的波特率

参数

handle [IN] 设备句柄

pnBaudrateAblity [OUT] 支持的波特率信息的指针。所支持波特率的或运算结果，单个数 值参考CameraParams.h中宏定义，如MV_CAML_BAUDRATE_9600 0x00000001 返回 成功，返回MV_OK ；失败，返回错误码

注解 （该接口已弃用，建议改用 MV_CAML_GetSupportBaudrates() 接口）

### ￿ MV_CC_SaveImageEx2()

MV_CAMCTRL_API int __stdcall MV_CC_SaveImageEx2 ( IN void * handle,

MV_SAVE_IMAGE_PARAM_EX * pstSaveParam

)

保存图片，支持Bmp和Jpeg，编码质量在50-99之前（该接口已弃用，建议改用 MV_CC_SaveImageEx3() 接口）

参数

handle [IN] 设备句柄

pstSaveParam [IN][OUT] 保存图片参数结构体 返回 成功，返回MV_OK ；失败，返回错误码

注解
- 通过该接口可以将从设备采集到的原始图像数据转换成JPEG或者BMP等格式并存放 在指定内存中，然后用户可以将转换之后的数据直接保存成图片文件。该接口调用无接 口顺序要求，有图像源数据就可以进行转换，可以先调用图像采集 接口，获取一帧图像 数据，然后再通过该接口转换格式。
- MV_CC_SaveImageEx2() 比 MV_CC_SaveImageEx() 增加参数handle，为了保证 与其他接口的统一。

### ￿ MV_CC_SaveImageToFile()

MV_CAMCTRL_API int __stdcall MV_CC_SaveImageToFile ( IN void * handle,

MV_SAVE_IMG_TO_FILE_PARAM * pstSaveFileParam

)

保存图像到文件（该接口已弃用，建议改用 MV_CC_SaveImageToFileEx() 接口）

参数

handle [IN] 设备句柄

pstSaveFileParam [IN][OUT] 保存图片文件参数结构体 返回 成功，返回MV_OK ；失败，返回错误码

注解 该接口支持BMP/JPEG/PNG/TIFF。

示例 LensShadingCorrection.cpp.

### ￿ MV_CC_ConvertPixelType()

MV_CAMCTRL_API int __stdcall MV_CC_ConvertPixelType ( IN void * handle,

IN OUT MV_CC_PIXEL_CONVERT_PARAM * pstCvtParam

)

像素格式转换（该接口已弃用，建议改用 MV_CC_ConvertPixelTypeEx() 接口）

参数

handle [IN] 设备句柄

pstCvtParam [IN][OUT] 像素格式转换参数结构体 返回 成功，返回MV_OK ；失败，返回错误码

注解
- 通过该接口可以将从设备采集到的原始图像数据转换成用户所需的像素格式并存放在 指定内存中。
- 该接口调用无接口顺序要求，有图像源数据就可以进行转换，可以先调用 MV_CC_GetOneFrameTimeout() 或者 MV_CC_RegisterImageCallBackEx() 设置回

调函数，获取一帧图像数据，然后再通过该接口转换格式。如果设备当前采集图像是 JPEG压缩的格式，则不支持调用该接口进行转换。

### ￿ MV_CC_SetSDKLogPath()

MV_CAMCTRL_API int __stdcall MV_CC_SetSDKLogPath ( IN const char * strSDKLogPath )

设置SDK日志路径

参数

strSDKLogPath [IN] SDK日志路径 返回 成功，返回MV_OK；失败，返回错误码

注解 设置路径之后，可以指定路径存放日志，V2.4.1版本新增日志服务，开启服务之后该接 口无效，默认日志服务为开启状态。

### ￿ MV_CC_EnumerateTls()

MV_CAMCTRL_API int __stdcall MV_CC_EnumerateTls ( )

获取支持的传输层

返回 当前支持的传输层协议类型和数值总和

注解 包含GigE、U3V、Camera Link、CoaXPress、XoFLink传输层。

### ￿ MV_CC_CreateHandleWithoutLog()

MV_CAMCTRL_API int __stdcall MV_CC_CreateHandleWithoutLog ( IN OUT void ** handle,

IN const MV_CC_DEVICE_INFO * pstDevInfo

)

创建设备句柄，不生成日志

参数

handle [IN][OUT] 设备句柄

pstDevInfo [IN] 设备信息结构体 返回 成功，返回MV_OK；失败，返回错误码

注解
- 根据输入的设备信息，创建库内部必须的资源和初始化内部模块。
- 通过该接口创建句柄，调用SDK接口，不会默认生成SDK日志文件，如果需要生成日 志文件可以通过 MV_CC_CreateHandle() 创建句柄，日志文件自动生成。

### ￿ MV_CC_RegisterImageCallBackForRGB()

MV_CAMCTRL_API int __stdcallIN MV_CC_RegisterImagevoid CallBackForRGB (* handle,

void__stdcall *cbOutput)(unsigned char *pData, INMV_FRAME_OUT_INFO_EX *pFrameInfo, void *pUser,

IN void
* pUser

)

注册图像数据回调，RGB

参数

handle [IN] 设备句柄

cbOutput [IN] 回调函数指针

pUser [IN] 用户自定义变量 返回 成功，返回MV_OK；失败，返回错误码

注解
- 通过该接口可以设置图像数据回调函数，在 MV_CC_CreateHandle() 之后即可调用 。
- 图像数据采集有两种方式，两种方式不能复用：

1. 调用 MV_CC_RegisterImageCallBackEx() 设置图像数据回调函数，然后调用 MV_CC_StartGrabbing() 开始采集，采集的图像数据在设置的回调函数中返回 。
2. 调用 MV_CC_StartGrabbing() 开始采集，然后在应用层循环调用 MV_CC_GetImageForRGB() 获取RGB24格式的帧数据，获取帧数据时上层应 用程序需要根据帧率控制好调用该接口的频率。

- 该接口不支持CameraLink设备。

### ￿ MV_CC_RegisterImageCallBackForBGR()

MV_CAMCTRL_API int __stdcallIN MV_CC_RegisterImagevoid CallBackForBGR (* handle,

void__stdcall *cbOutput)(unsigned char *pData, INMV_FRAME_OUT_INFO_EX *pFrameInfo, void *pUser,

IN void
* pUser

)

注册图像数据回调，BGR

参数

handle [IN] 设备句柄

cbOutput [IN] 回调函数指针

pUser [IN] 用户自定义变量 返回 成功，返回MV_OK；失败，返回错误码

注解
- 通过该接口可以设置图像数据回调函数，在 MV_CC_CreateHandle() 之后即可调用 。
- 图像数据采集有两种方式，两种方式不能复用：

1. 调用 MV_CC_RegisterImageCallBackForBGR() 设置BGR24图像数据回调函 数，然后调用 MV_CC_StartGrabbing() 开始采集，采集的图像数据在设置的回 调函数中返回。

2. 调用 MV_CC_StartGrabbing() 开始采集，然后在应用层循环调用 MV_CC_GetImageForBGR() 获取BGR24格式的帧数据，获取帧数据时上层应 用程序需要根据帧率控制好调用该接口的频率。

- 该接口不支持CameraLink设备。

### ￿ MV_CC_GetImageForRGB()

MV_CAMCTRL_API int __stdcall MV_CC_GetImageForRGB ( IN void * handle,

IN OUT unsigned char * pData,

IN unsigned int nDataSize,

IN OUT MV_FRAME_OUT_INFO_EX * pstFrameInfo,

IN int nMsec

)

获取一帧RGB数据，此函数为查询式获取，每次调用查询内部 缓存有无数据，有数据则获取 数据，无数据返回错误码

参数

handle [IN] 设备句柄

pData [IN][OUT] 图像数据接收指针

nDataSize [IN] 接收缓存大小

pstFrameInfo [IN][OUT] 图像信息结构体

nMsec [IN] 等待超时时间 返回 成功，返回MV_OK；失败，返回错误码

注解
- 每次调用该接口，将查询内部缓存是否有数据，如果有数据则转换成RGB24格式返回 ，如果没有数据则返回错误码。因为图像转换成RGB24格式有耗时，所以当数据帧率过 高时该接口可能会导致丢帧。
- 调用该接口获取图像数据帧之前需要先调用 MV_CC_StartGrabbing() 启动图像采集 。该接口为主动式获取帧数据，上层应用程序需要根据帧率，控制好调用该接口的频率 。
- 该接口不支持CameraLink设备。

### ￿ MV_CC_GetImageForBGR()

MV_CAMCTRL_API int __stdcall MV_CC_GetImageForBGR ( IN void * handle,

IN OUT unsigned char * pData,

IN unsigned int nDataSize,

IN OUT MV_FRAME_OUT_INFO_EX * pstFrameInfo,

IN int nMsec

)

获取一帧BGR数据，此函数为查询式获取，每次调用查询内部 缓存有无数据，有数据则获取 数据，无数据返回错误码

参数

handle [IN] 设备句柄

pData [IN][OUT] 图像数据接收指针

nDataSize [IN] 接收缓存大小

pstFrameInfo [IN][OUT] 图像信息结构体

nMsec [IN] 等待超时时间 返回 成功，返回MV_OK；失败，返回错误码

注解
- 每次调用该接口，将查询内部缓存是否有数据，如果有数据则转换成BGR24格式返回 ，如果没有数据则返回错误码。因为图像转换成BGR24格式有耗时，所以当数据帧率过 高时该接口可能会导致丢帧。
- 调用该接口获取图像数据帧之前需要先调用 MV_CC_StartGrabbing() 启动图像采集 。该接口为主动式获取帧数据，上层应用程序需要根据帧率，控制好调用该接口的频率 。
- 该接口不支持CameraLink设备。

### ￿ MV_CC_OpenParamsGUI()

MV_CAMCTRL_API int __stdcall MV_CC_OpenParamsGUI ( IN void * handle )

打开获取或设置相机参数的GUI界面

参数

handle [IN] 设备句柄 返回 成功，返回MV_OK；失败，返回错误码

注解 通过 MV_CC_OpenDevice() 连接设备后，可以通过该接口获取或设置设备参数。

### ￿ MV_CC_SavePointCloudData()

MV_CAMCTRL_API int __stdcall MV_CC_SavePointCloudDat a ( IN void * handle,

IN OUT MV_SAVE_POINT_CLOUD_PARAMpstPointDataPara *m

)

保存3D点云数据，支持PLY、CSV和OBJ三种格式

参数

handle [IN] 设备句柄

pstPointDataParam [IN][OUT] 保存点云数据参数结构体 返回 成功，返回MV_OK；失败，返回错误码

注解
- 3D数据格式保存成3D文件格式，支持PLY/CSV/OBJ。
- 目前支持PixelType_Gvsp_Coord3D_ABC32、PixelType_Gvsp_Coord3D_ABC32f、 PixelType_Gvsp_Coord3D_AB32、PixelType_Gvsp_Coord3D_AB32f、 PixelType_Gvsp_Coord3D_AC32、PixelType_Gvsp_Coord3D_AC32f，暂不支持其他 3D格式。

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

枚举

### 类型定义

包含不同类型的枚举信息 更多...

### 枚举

enum MV_GRAB_STRATEGY { MV_GrabStrategy_OneByOne = 0, MV_GrabStrategy_LatestImagesOnly = 1, MV_GrabStrategy_LatestImages = 2, MV_GrabStrategy_UpcomingImage = 3 } 取流策略 更多...

enum MV_SAVE_IAMGE_TYPE { MV_Image_Undefined = 0, MV_Image_Bmp = 1, MV_Image_Jpeg = 2, MV_Image_Png = 3, MV_Image_Tif = 4 } 保存图片格式 更多...

enum MV_IMG_ROTATION_ANGLE { MV_IMAGE_ROTATE_90 = 1, MV_IMAGE_ROTATE_180 = 2, MV_IMAGE_ROTATE_270 = 3 } 旋转角度 更多...

enum MV_IMG_FLIP_TYPE { MV_FLIP_VERTICAL = 1, MV_FLIP_HORIZONTAL = 2 } 翻转类型 更多...

enum MV_CC_GAMMA_TYPE { MV_CC_GAMMA_TYPE_NONE = 0, MV_CC_GAMMA_TYPE_VALUE = 1, MV_CC_GAMMA_TYPE_USER_CURVE = 2, MV_CC_GAMMA_TYPE_LRGB2SRGB = 3, MV_CC_GAMMA_TYPE_SRGB2LRGB = 4 } Gamma类型 更多...

enum MV_RECORD_FORMAT_TYPE { MV_FormatType_Undefined = 0, MV_FormatType_AVI = 1 } 录像格式定义 更多...

enum MV_CAM_ACQUISITION_MODE { MV_ACQ_MODE_SINGLE = 0, MV_ACQ_MODE_MUTLI = 1, MV_ACQ_MODE_CONTINUOUS = 2 } 采集模式 更多...

enum MV_CAM_GAIN_MODE { MV_GAIN_MODE_OFF = 0, MV_GAIN_MODE_ONCE = 1,

MV_GAIN_MODE_CONTINUOUS = 2 } 增益模式 更多...

enum MV_CAM_EXPOSURE_MODE { MV_EXPOSURE_MODE_TIMED = 0, MV_EXPOSURE_MODE_TRIGGER_WIDTH = 1 } 曝光模式 更多...

enum MV_CAM_EXPOSURE_AUTO_MODE { MV_EXPOSURE_AUTO_MODE_OFF = 0, MV_EXPOSURE_AUTO_MODE_ONCE = 1, MV_EXPOSURE_AUTO_MODE_CONTINUOUS = 2 } 自动曝光模式 更多...

enum MV_CAM_TRIGGER_MODE { MV_TRIGGER_MODE_OFF = 0, MV_TRIGGER_MODE_ON = 1 } 触发模式 更多...

enum MV_CAM_GAMMA_SELECTOR { MV_GAMMA_SELECTOR_USER = 1, MV_GAMMA_SELECTOR_SRGB = 2 } Gamma选择器 更多...

enum MV_CAM_BALANCEWHITE_AUTO { MV_BALANCEWHITE_AUTO_OFF = 0, MV_BALANCEWHITE_AUTO_ONCE = 2, MV_BALANCEWHITE_AUTO_CONTINUOUS = 1 } 白平衡 更多...

enum MV_CAM_TRIGGER_SOURCE { MV_TRIGGER_SOURCE_LINE0 = 0, MV_TRIGGER_SOURCE_LINE1 = 1,

MV_TRIGGER_SOURCE_LINE2 = 2, MV_TRIGGER_SOURCE_LINE3 = 3, MV_TRIGGER_SOURCE_COUNTER0 = 4, MV_TRIGGER_SOURCE_SOFTWARE = 7, MV_TRIGGER_SOURCE_FrequencyConverter = 8 } 触发源 更多...

enum MV_CC_STREAM_EXCEPTION_TYPE { MV_CC_STREAM_EXCEPTION_ABNORMAL_IMAGE = 0x4001, MV_CC_STREAM_EXCEPTION_LIST_OVERFLOW = 0x4002, MV_CC_STREAM_EXCEPTION_LIST_EMPTY = 0x4003, MV_CC_STREAM_EXCEPTION_RECONNECTION = 0x4004, MV_CC_STREAM_EXCEPTION_DISCONNECTED = 0x4005, MV_CC_STREAM_EXCEPTION_DEVICE = 0x4006 } U3V流异常类型 更多...

enum MV_GIGE_TRANSMISSION_TYPE { MV_GIGE_TRANSTYPE_UNICAST = 0x0, MV_GIGE_TRANSTYPE_MULTICAST = 0x1, MV_GIGE_TRANSTYPE_LIMITEDBROADCAST = 0x2, MV_GIGE_TRANSTYPE_SUBNETBROADCAST = 0x3, MV_GIGE_TRANSTYPE_CAMERADEFINED = 0x4, MV_GIGE_TRANSTYPE_UNICAST_DEFINED_PORT = 0x5, MV_GIGE_TRANSTYPE_UNICAST_WITHOUT_RECV = 0x00010000, MV_GIGE_TRANSTYPE_MULTICAST_WITHOUT_RECV = 0x00010001 } Gige的传输类型 更多...

enum MV_SORT_METHOD { SortMethod_SerialNumber = 0, SortMethod_UserID = 1, SortMethod_CurrentIP_ASC = 2, SortMethod_CurrentIP_DESC = 3 } 排序方式 更多...

enum MV_XML_InterfaceType { IFT_IValue, IFT_IBase, IFT_IInteger, IFT_IBoolean, IFT_ICommand, IFT_IFloat, IFT_IString, IFT_IRegister, IFT_ICategory, IFT_IEnumeration, IFT_IEnumEntry, IFT_IPort } 每个节点对应的接口类型 更多...

enum MV_XML_AccessMode { AM_NI, AM_NA, AM_WO, AM_RO, AM_RW, AM_Undefined, AM_CycleDetect } 节点的访问模式 更多...

enum MVCC_NODE_ERR_TYPE { MVCC_NODE_ERR_NODE_INVALID = 1, MVCC_NODE_ERR_ACCESS = 2, MVCC_NODE_ERR_OUT_RANGE = 3, MVCC_NODE_ERR_VERIFY_FAILD = 4, MVCC_NODE_ERR_OTHER = 100 } 导入参数报错时的原因 更多...

enum MV_IMAGE_RECONSTRUCTION_METHOD { MV_SPLIT_BY_LINE = 1 } 图像重构方式 更多...

enum MvGvspPixelType { PixelType_Gvsp_Undefined = 0xFFFFFFFF, PixelType_Gvsp_Mono1p = (MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(1) | 0x0037), PixelType_Gvsp_Mono2p = (MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(2) | 0x0038), PixelType_Gvsp_Mono4p = (MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(4) | 0x0039), PixelType_Gvsp_Mono8 = (MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(8) | 0x0001), PixelType_Gvsp_Mono8_Signed = (MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(8) | 0x0002), PixelType_Gvsp_Mono10 = (MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(16) | 0x0003), PixelType_Gvsp_Mono10_Packed = (MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(12) | 0x0004), PixelType_Gvsp_Mono12 = (MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(16) | 0x0005), PixelType_Gvsp_Mono12_Packed = (MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(12) | 0x0006), PixelType_Gvsp_Mono14 = (MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(16) | 0x0025), PixelType_Gvsp_Mono16 = (MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(16) | 0x0007), PixelType_Gvsp_BayerGR8 = (MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(8) | 0x0008), PixelType_Gvsp_BayerRG8 = (MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(8) | 0x0009), PixelType_Gvsp_BayerGB8 = (MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(8) | 0x000A), PixelType_Gvsp_BayerBG8 = (MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(8) | 0x000B), PixelType_Gvsp_BayerRBGG8 = (MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(8) | 0x0046), PixelType_Gvsp_BayerGR10 = (MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(16) | 0x000C), PixelType_Gvsp_BayerRG10 = (MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(16) | 0x000D), PixelType_Gvsp_BayerGB10 = (MV_GVSP_PIX_MONO |

MV_PIXEL_BIT_COUNT(16) | 0x000E), PixelType_Gvsp_BayerBG10 = (MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(16) | 0x000F), PixelType_Gvsp_BayerGR12 = (MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(16) | 0x0010), PixelType_Gvsp_BayerRG12 = (MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(16) | 0x0011), PixelType_Gvsp_BayerGB12 = (MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(16) | 0x0012), PixelType_Gvsp_BayerBG12 = (MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(16) | 0x0013), PixelType_Gvsp_BayerGR10_Packed = (MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(12) | 0x0026), PixelType_Gvsp_BayerRG10_Packed = (MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(12) | 0x0027), PixelType_Gvsp_BayerGB10_Packed = (MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(12) | 0x0028), PixelType_Gvsp_BayerBG10_Packed = (MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(12) | 0x0029), PixelType_Gvsp_BayerGR12_Packed = (MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(12) | 0x002A), PixelType_Gvsp_BayerRG12_Packed = (MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(12) | 0x002B), PixelType_Gvsp_BayerGB12_Packed = (MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(12) | 0x002C), PixelType_Gvsp_BayerBG12_Packed = (MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(12) | 0x002D), PixelType_Gvsp_BayerGR16 = (MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(16) | 0x002E), PixelType_Gvsp_BayerRG16 = (MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(16) | 0x002F), PixelType_Gvsp_BayerGB16 = (MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(16) | 0x0030), PixelType_Gvsp_BayerBG16 = (MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(16) | 0x0031), PixelType_Gvsp_RGB8_Packed = (MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(24) | 0x0014), PixelType_Gvsp_BGR8_Packed = (MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(24) | 0x0015),

PixelType_Gvsp_RGBA8_Packed = (MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(32) | 0x0016), PixelType_Gvsp_BGRA8_Packed = (MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(32) | 0x0017), PixelType_Gvsp_RGB10_Packed = (MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(48) | 0x0018), PixelType_Gvsp_BGR10_Packed = (MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(48) | 0x0019), PixelType_Gvsp_RGB12_Packed = (MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(48) | 0x001A), PixelType_Gvsp_BGR12_Packed = (MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(48) | 0x001B), PixelType_Gvsp_RGB16_Packed = (MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(48) | 0x0033), PixelType_Gvsp_BGR16_Packed = (MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(48) | 0x004B), PixelType_Gvsp_RGBA16_Packed = (MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(64) | 0x0064), PixelType_Gvsp_BGRA16_Packed = (MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(64) | 0x0051), PixelType_Gvsp_RGB10V1_Packed = (MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(32) | 0x001C), PixelType_Gvsp_RGB10V2_Packed = (MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(32) | 0x001D), PixelType_Gvsp_RGB12V1_Packed = (MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(36) | 0X0034), PixelType_Gvsp_RGB565_Packed = (MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(16) | 0x0035), PixelType_Gvsp_BGR565_Packed = (MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(16) | 0X0036), PixelType_Gvsp_YUV411_Packed = (MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(12) | 0x001E), PixelType_Gvsp_YUV422_Packed = (MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(16) | 0x001F), PixelType_Gvsp_YUV422_YUYV_Packed = (MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(16) | 0x0032), PixelType_Gvsp_YUV444_Packed = (MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(24) | 0x0020), PixelType_Gvsp_YCBCR8_CBYCR = (MV_GVSP_PIX_COLOR |

MV_PIXEL_BIT_COUNT(24) | 0x003A), PixelType_Gvsp_YCBCR422_8 = (MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(16) | 0x003B), PixelType_Gvsp_YCBCR422_8_CBYCRY = (MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(16) | 0x0043), PixelType_Gvsp_YCBCR411_8_CBYYCRYY = (MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(12) | 0x003C), PixelType_Gvsp_YCBCR601_8_CBYCR = (MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(24) | 0x003D), PixelType_Gvsp_YCBCR601_422_8 = (MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(16) | 0x003E), PixelType_Gvsp_YCBCR601_422_8_CBYCRY = (MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(16) | 0x0044), PixelType_Gvsp_YCBCR601_411_8_CBYYCRYY = (MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(12) | 0x003F), PixelType_Gvsp_YCBCR709_8_CBYCR = (MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(24) | 0x0040), PixelType_Gvsp_YCBCR709_422_8 = (MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(16) | 0x0041), PixelType_Gvsp_YCBCR709_422_8_CBYCRY = (MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(16) | 0x0045), PixelType_Gvsp_YCBCR709_411_8_CBYYCRYY = (MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(12) | 0x0042), PixelType_Gvsp_YUV420SP_NV12 = (MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(12) | 0x8001), PixelType_Gvsp_YUV420SP_NV21 = (MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(12) | 0x8002), PixelType_Gvsp_RGB8_Planar = (MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(24) | 0x0021), PixelType_Gvsp_RGB10_Planar = (MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(48) | 0x0022), PixelType_Gvsp_RGB12_Planar = (MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(48) | 0x0023), PixelType_Gvsp_RGB16_Planar = (MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(48) | 0x0024), PixelType_Gvsp_Jpeg = (MV_GVSP_PIX_CUSTOM | MV_PIXEL_BIT_COUNT(24) | 0x0001), PixelType_Gvsp_Coord3D_ABC32f = (MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(96) | 0x00C0),

PixelType_Gvsp_Coord3D_ABC32f_Planar = (MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(96) | 0x00C1), PixelType_Gvsp_Coord3D_ABC32 = (MV_GVSP_PIX_CUSTOM | MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(96) | 0x3001), PixelType_Gvsp_Coord3D_AB32f = (MV_GVSP_PIX_CUSTOM | MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(64) | 0x3002), PixelType_Gvsp_Coord3D_AB32 = (MV_GVSP_PIX_CUSTOM | MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(64) | 0x3003), PixelType_Gvsp_Coord3D_AC32f_64 = (MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(64) | 0x00C2), PixelType_Gvsp_Coord3D_AC32f_Planar = (MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(64) | 0x00C3), PixelType_Gvsp_Coord3D_AC32 = (MV_GVSP_PIX_CUSTOM | MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(64) | 0x3004), PixelType_Gvsp_Coord3D_A32f = (MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(32) | 0x00BD), PixelType_Gvsp_Coord3D_A32 = (MV_GVSP_PIX_CUSTOM | MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(32) | 0x3005), PixelType_Gvsp_Coord3D_C32f = (MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(32) | 0x00BF), PixelType_Gvsp_Coord3D_C32 = (MV_GVSP_PIX_CUSTOM | MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(32) | 0x3006), PixelType_Gvsp_Coord3D_ABC16 = (MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(48) | 0x00B9), PixelType_Gvsp_Coord3D_C16 = (MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(16) | 0x00B8), PixelType_Gvsp_HB_Mono8 = (MV_GVSP_PIX_CUSTOM | MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(8) | 0x0001), PixelType_Gvsp_HB_Mono10 = (MV_GVSP_PIX_CUSTOM | MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(16) | 0x0003), PixelType_Gvsp_HB_Mono10_Packed = (MV_GVSP_PIX_CUSTOM | MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(12) | 0x0004), PixelType_Gvsp_HB_Mono12 = (MV_GVSP_PIX_CUSTOM | MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(16) | 0x0005), PixelType_Gvsp_HB_Mono12_Packed = (MV_GVSP_PIX_CUSTOM | MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(12) | 0x0006), PixelType_Gvsp_HB_Mono16 = (MV_GVSP_PIX_CUSTOM | MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(16) | 0x0007), PixelType_Gvsp_HB_BayerGR8 = (MV_GVSP_PIX_CUSTOM |

MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(8) | 0x0008), PixelType_Gvsp_HB_BayerRG8 = (MV_GVSP_PIX_CUSTOM | MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(8) | 0x0009), PixelType_Gvsp_HB_BayerGB8 = (MV_GVSP_PIX_CUSTOM | MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(8) | 0x000A), PixelType_Gvsp_HB_BayerBG8 = (MV_GVSP_PIX_CUSTOM | MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(8) | 0x000B), PixelType_Gvsp_HB_BayerRBGG8 = (MV_GVSP_PIX_CUSTOM | MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(8) | 0x0046), PixelType_Gvsp_HB_BayerGR10 = (MV_GVSP_PIX_CUSTOM | MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(16) | 0x000C), PixelType_Gvsp_HB_BayerRG10 = (MV_GVSP_PIX_CUSTOM | MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(16) | 0x000D), PixelType_Gvsp_HB_BayerGB10 = (MV_GVSP_PIX_CUSTOM | MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(16) | 0x000E), PixelType_Gvsp_HB_BayerBG10 = (MV_GVSP_PIX_CUSTOM | MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(16) | 0x000F), PixelType_Gvsp_HB_BayerGR12 = (MV_GVSP_PIX_CUSTOM | MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(16) | 0x0010), PixelType_Gvsp_HB_BayerRG12 = (MV_GVSP_PIX_CUSTOM | MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(16) | 0x0011), PixelType_Gvsp_HB_BayerGB12 = (MV_GVSP_PIX_CUSTOM | MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(16) | 0x0012), PixelType_Gvsp_HB_BayerBG12 = (MV_GVSP_PIX_CUSTOM | MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(16) | 0x0013), PixelType_Gvsp_HB_BayerGR10_Packed = (MV_GVSP_PIX_CUSTOM | MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(12) | 0x0026), PixelType_Gvsp_HB_BayerRG10_Packed = (MV_GVSP_PIX_CUSTOM | MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(12) | 0x0027), PixelType_Gvsp_HB_BayerGB10_Packed = (MV_GVSP_PIX_CUSTOM | MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(12) | 0x0028), PixelType_Gvsp_HB_BayerBG10_Packed = (MV_GVSP_PIX_CUSTOM | MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(12) | 0x0029), PixelType_Gvsp_HB_BayerGR12_Packed = (MV_GVSP_PIX_CUSTOM | MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(12) | 0x002A), PixelType_Gvsp_HB_BayerRG12_Packed = (MV_GVSP_PIX_CUSTOM | MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(12) | 0x002B), PixelType_Gvsp_HB_BayerGB12_Packed = (MV_GVSP_PIX_CUSTOM | MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(12) | 0x002C),

PixelType_Gvsp_HB_BayerBG12_Packed = (MV_GVSP_PIX_CUSTOM | MV_GVSP_PIX_MONO | MV_PIXEL_BIT_COUNT(12) | 0x002D), PixelType_Gvsp_HB_YUV422_Packed = (MV_GVSP_PIX_CUSTOM | MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(16) | 0x001F), PixelType_Gvsp_HB_YUV422_YUYV_Packed = (MV_GVSP_PIX_CUSTOM | MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(16) | 0x0032), PixelType_Gvsp_HB_RGB8_Packed = (MV_GVSP_PIX_CUSTOM | MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(24) | 0x0014), PixelType_Gvsp_HB_BGR8_Packed = (MV_GVSP_PIX_CUSTOM | MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(24) | 0x0015), PixelType_Gvsp_HB_RGBA8_Packed = (MV_GVSP_PIX_CUSTOM | MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(32) | 0x0016), PixelType_Gvsp_HB_BGRA8_Packed = (MV_GVSP_PIX_CUSTOM | MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(32) | 0x0017), PixelType_Gvsp_HB_RGB16_Packed = (MV_GVSP_PIX_CUSTOM | MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(48) | 0x0033), PixelType_Gvsp_HB_BGR16_Packed = (MV_GVSP_PIX_CUSTOM | MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(48) | 0x004B), PixelType_Gvsp_HB_RGBA16_Packed = (MV_GVSP_PIX_CUSTOM | MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(64) | 0x0064), PixelType_Gvsp_HB_BGRA16_Packed = (MV_GVSP_PIX_CUSTOM | MV_GVSP_PIX_COLOR | MV_PIXEL_BIT_COUNT(64) | 0x0051) } 图片格式定义 更多...

enum MV_SAVE_POINT_CLOUD_FILE_TYPE { MV_PointCloudFile_Undefined = 0, MV_PointCloudFile_PLY = 1, MV_PointCloudFile_CSV = 2, MV_PointCloudFile_OBJ = 3 } 保存的3D数据格式（自4.4.1版本废弃） 更多...

### 详细描述

包含不同类型的枚举信息

### 枚举类型说明

### ￿ MV_GRAB_STRATEGY

enum MV_GRAB_STRATEGY

取流策略

枚举值

MV_GrabStrategy_OneByOne 从旧到新一帧一帧的获取图像（默认为该策略）

MV_GrabStrategy_LatestImagesOnly 获取列表中最新的一帧图像（同时清除列表中的其余 图像）

MV_GrabStrategy_LatestImages 获取列表中最新的图像，个数由OutputQueueSize决定 ，范围为1-ImageNodeNum，设置成1等同于 LatestImagesOnly,设置成ImageNodeNum等同于 OneByOne

MV_GrabStrategy_UpcomingImage 等待下一帧图像

### ￿ MV_SAVE_IAMGE_TYPE

enum MV_SAVE_IAMGE_TYPE

保存图片格式

枚举值

MV_Image_Undefined 未定义的图像格式

MV_Image_Bmp BMP图像格式

MV_Image_Jpeg JPEG图像格式

MV_Image_Png PNG图像格式

MV_Image_Tif TIFF图像格式

### ￿ MV_IMG_ROTATION_ANGLE

enum MV_IMG_ROTATION_ANGLE

旋转角度

枚举值

MV_IMAGE_ROTATE_90 90°

MV_IMAGE_ROTATE_180 180°

MV_IMAGE_ROTATE_270 270°

### ￿ MV_IMG_FLIP_TYPE

enum MV_IMG_FLIP_TYPE

翻转类型

枚举值

MV_FLIP_VERTICAL 垂直翻转

MV_FLIP_HORIZONTAL 水平翻转

### ￿ MV_CC_GAMMA_TYPE

enum MV_CC_GAMMA_TYPE

Gamma类型

枚举值

MV_CC_GAMMA_TYPE_NONE 不启用

MV_CC_GAMMA_TYPE_VALUE Gamma值

MV_CC_GAMMA_TYPE_USER_CURVE Gamma曲线 8位，长度：256*sizeof(unsigned char) 10位，长度：1024*sizeof(unsigned short) 12位，长度：4096*sizeof(unsigned short) 16位，长度：65536*sizeof(unsigned short)

MV_CC_GAMMA_TYPE_LRGB2SRGB linear RGB to sRGB

MV_CC_GAMMA_TYPE_SRGB2LRGB sRGB to linear RGB（仅色彩插值时支持，色彩校 正时无效）

### ￿ MV_RECORD_FORMAT_TYPE

enum MV_RECORD_FORMAT_TYPE

录像格式定义

枚举值

MV_FormatType_Undefined 未定义的录像格式

MV_FormatType_AVI AVI录像格式

### ￿ MV_CAM_ACQUISITION_MODE

enum MV_CAM_ACQUISITION_MODE

采集模式

枚举值

MV_ACQ_MODE_SINGLE 单帧模式

MV_ACQ_MODE_MUTLI 多帧模式

MV_ACQ_MODE_CONTINUOUS 持续采集模式

### ￿ MV_CAM_GAIN_MODE

enum MV_CAM_GAIN_MODE

增益模式

枚举值

MV_GAIN_MODE_OFF 关闭

MV_GAIN_MODE_ONCE 一次

MV_GAIN_MODE_CONTINUOUS 连续

### ￿ MV_CAM_EXPOSURE_MODE

enum MV_CAM_EXPOSURE_MODE

曝光模式

枚举值

MV_EXPOSURE_MODE_TIMED 时间

MV_EXPOSURE_MODE_TRIGGER_WIDTH 触发脉冲宽度

### ￿ MV_CAM_EXPOSURE_AUTO_MODE

enum MV_CAM_EXPOSURE_AUTO_MODE

自动曝光模式

枚举值

MV_EXPOSURE_AUTO_MODE_OFF 关闭

MV_EXPOSURE_AUTO_MODE_ONCE 一次

MV_EXPOSURE_AUTO_MODE_CONTINUOUS 连续

### ￿ MV_CAM_TRIGGER_MODE

enum MV_CAM_TRIGGER_MODE

触发模式

枚举值

MV_TRIGGER_MODE_OFF 关闭

MV_TRIGGER_MODE_ON 打开

### ￿ MV_CAM_GAMMA_SELECTOR

enum MV_CAM_GAMMA_SELECTOR

Gamma选择器

枚举值

MV_GAMMA_SELECTOR_USER 用户

MV_GAMMA_SELECTOR_SRGB sRGB

### ￿ MV_CAM_BALANCEWHITE_AUTO

enum MV_CAM_BALANCEWHITE_AUTO

白平衡

枚举值

MV_BALANCEWHITE_AUTO_OFF 关闭

MV_BALANCEWHITE_AUTO_ONCE 一次

MV_BALANCEWHITE_AUTO_CONTINUOUS 连续

### ￿ MV_CAM_TRIGGER_SOURCE

enum MV_CAM_TRIGGER_SOURCE

触发源

枚举值

MV_TRIGGER_SOURCE_LINE0 Line0

MV_TRIGGER_SOURCE_LINE1 Line1

MV_TRIGGER_SOURCE_LINE2 Line2

MV_TRIGGER_SOURCE_LINE3 Line3

MV_TRIGGER_SOURCE_COUNTER0 Conuter0

MV_TRIGGER_SOURCE_SOFTWARE 软触发

MV_TRIGGER_SOURCE_FrequencyConverter 变频器

### ￿ MV_CC_STREAM_EXCEPTION_TYPE

enum MV_CC_STREAM_EXCEPTION_TYPE

U3V流异常类型

枚举值

MV_CC_STREAM_EXCEPTION_ABNORMAL_IMAGE 异常的图像，该帧被丢弃

MV_CC_STREAM_EXCEPTION_LIST_OVERFLOW 缓存列表溢出，清除最旧的一帧

MV_CC_STREAM_EXCEPTION_LIST_EMPTY 缓存列表为空，该帧被丢弃

MV_CC_STREAM_EXCEPTION_RECONNECTION 断流恢复

MV_CC_STREAM_EXCEPTION_DISCONNECTED 断流，恢复失败,取流被中止

MV_CC_STREAM_EXCEPTION_DEVICE 设备异常，取流被中止

### ￿ MV_GIGE_TRANSMISSION_TYPE

enum MV_GIGE_TRANSMISSION_TYPE

Gige的传输类型

枚举值

MV_GIGE_TRANSTYPE_UNICAST 表示单播（默认）

MV_GIGE_TRANSTYPE_MULTICAST 表示组播

MV_GIGE_TRANSTYPE_LIMITEDBROADCAST 表示局域网内广播，暂不支持

MV_GIGE_TRANSTYPE_SUBNETBROADCAST 表示子网内广播，暂不支持

MV_GIGE_TRANSTYPE_CAMERADEFINED 表示从设备获取，暂不支持

MV_GIGE_TRANSTYPE_UNICAST_DEFINED_PORT 表示用户自定义应用端接收图像 数据Port号

MV_GIGE_TRANSTYPE_UNICAST_WITHOUT_RECV 表示设置了单播，但本实例不接 收图像数据

MV_GIGE_TRANSTYPE_MULTICAST_WITHOUT_RECV 表示组播模式，但本实例不接收 图像数据

### ￿ MV_SORT_METHOD

enum MV_SORT_METHOD

排序方式

枚举值

SortMethod_SerialNumber 按序列号排序

SortMethod_UserID 按用户自定义名字排序

SortMethod_CurrentIP_ASC 按当前IP地址排序（升序）

SortMethod_CurrentIP_DESC 按当前IP地址排序（降序）

### ￿ MV_XML_InterfaceType

enum MV_XML_InterfaceType

每个节点对应的接口类型

枚举值

IFT_IValue Value

IFT_IBase Base

IFT_IInteger Integer

IFT_IBoolean Boolean

IFT_ICommand Command

IFT_IFloat Float

IFT_IString String

IFT_IRegister Register

IFT_ICategory Category

IFT_IEnumeration Enumeration

IFT_IEnumEntry EnumEntry

IFT_IPort Port

### ￿ MV_XML_AccessMode

enum MV_XML_AccessMode

节点的访问模式

枚举值

AM_NI 不可实现

AM_NA 不可用

AM_WO 只写

AM_RO 只读

AM_RW 读写

AM_Undefined 未定义

AM_CycleDetect 内部用于AccessMode循环检测

### ￿ MVCC_NODE_ERR_TYPE

enum MVCC_NODE_ERR_TYPE

导入参数报错时的原因

枚举值

MVCC_NODE_ERR_NODE_INVALID 节点不存在

MVCC_NODE_ERR_ACCESS 访问条件错误,通常是节点不可读写

MVCC_NODE_ERR_OUT_RANGE 写入越界,超出该节点支持的范围

MVCC_NODE_ERR_VERIFY_FAILD 校验失败,通常是写入的值与文件中的值不匹配

MVCC_NODE_ERR_OTHER 其它错误,可查阅日志

### ￿ MV_IMAGE_RECONSTRUCTION_METHOD

enum MV_IMAGE_RECONSTRUCTION_METHOD

图像重构方式

枚举值

MV_SPLIT_BY_LINE 源图像按行拆分成多张图像

### ￿ MvGvspPixelType

enum MvGvspPixelType

图片格式定义

枚举值

PixelType_Gvsp_Undefined 未定义的像素类型

PixelType_Gvsp_Mono1p Mono1p

PixelType_Gvsp_Mono2p Mono2p

PixelType_Gvsp_Mono4p Mono4p

PixelType_Gvsp_Mono8 Mono8

PixelType_Gvsp_Mono8_Signed Mono8_Signed

PixelType_Gvsp_Mono10 Mono10

PixelType_Gvsp_Mono10_Packed Mono10_Packed

PixelType_Gvsp_Mono12 Mono12

PixelType_Gvsp_Mono12_Packed Mono12_Packed

PixelType_Gvsp_Mono14 Mono14

PixelType_Gvsp_Mono16 Mono16

PixelType_Gvsp_BayerGR8 BayerGR8

PixelType_Gvsp_BayerRG8 BayerRG8

PixelType_Gvsp_BayerGB8 BayerGB8

PixelType_Gvsp_BayerBG8 BayerBG8

PixelType_Gvsp_BayerRBGG8 BayerRBGG8

PixelType_Gvsp_BayerGR10 BayerGR10

PixelType_Gvsp_BayerRG10 BayerRG10

PixelType_Gvsp_BayerGB10 BayerGB10

PixelType_Gvsp_BayerBG10 BayerBG10

PixelType_Gvsp_BayerGR12 BayerGR12

PixelType_Gvsp_BayerRG12 BayerRG12

PixelType_Gvsp_BayerGB12 BayerGB12

PixelType_Gvsp_BayerBG12 BayerBG12

PixelType_Gvsp_BayerGR10_Packed BayerGR10_Packed

PixelType_Gvsp_BayerRG10_Packed BayerRG10_Packed

PixelType_Gvsp_BayerGB10_Packed BayerGB10_Packed

PixelType_Gvsp_BayerBG10_Packed BayerBG10_Packed

PixelType_Gvsp_BayerGR12_Packed BayerGR12_Packed

PixelType_Gvsp_BayerRG12_Packed BayerRG12_Packed

PixelType_Gvsp_BayerGB12_Packed BayerGB12_Packed

PixelType_Gvsp_BayerBG12_Packed BayerBG12_Packed

PixelType_Gvsp_BayerGR16 BayerGR16

PixelType_Gvsp_BayerRG16 BayerRG16

PixelType_Gvsp_BayerGB16 BayerGB16

PixelType_Gvsp_BayerBG16 BayerBG16

PixelType_Gvsp_RGB8_Packed RGB8_Packed

PixelType_Gvsp_BGR8_Packed BGR8_Packed

PixelType_Gvsp_RGBA8_Packed RGBA8_Packed

PixelType_Gvsp_BGRA8_Packed BGRA8_Packed

PixelType_Gvsp_RGB10_Packed RGB10_Packed

PixelType_Gvsp_BGR10_Packed BGR10_Packed

PixelType_Gvsp_RGB12_Packed RGB12_Packed

PixelType_Gvsp_BGR12_Packed BGR12_Packed

PixelType_Gvsp_RGB16_Packed RGB16_Packed

PixelType_Gvsp_BGR16_Packed BGR16_Packed

PixelType_Gvsp_RGBA16_Packed RGBA16_Packed

PixelType_Gvsp_BGRA16_Packed BGRA16_Packed

PixelType_Gvsp_RGB10V1_Packed RGB10V1_Packed

PixelType_Gvsp_RGB10V2_Packed RGB10V2_Packed

PixelType_Gvsp_RGB12V1_Packed RGB12V1_Packed

PixelType_Gvsp_RGB565_Packed RGB565_Packed

PixelType_Gvsp_BGR565_Packed BGR565_Packed

PixelType_Gvsp_YUV411_Packed YUV411_Packed

PixelType_Gvsp_YUV422_Packed YUV422_Packed

PixelType_Gvsp_YUV422_YUYV_Packed YUV422_YUYV_Packed

PixelType_Gvsp_YUV444_Packed YUV444_Packed

PixelType_Gvsp_YCBCR8_CBYCR YCBCR8_CBYCR

PixelType_Gvsp_YCBCR422_8 YCBCR422_8

PixelType_Gvsp_YCBCR422_8_CBYCRY YCBCR422_8_CBYCRY

PixelType_Gvsp_YCBCR411_8_CBYYCRYY YCBCR411_8_CBYYCRYY

PixelType_Gvsp_YCBCR601_8_CBYCR YCBCR601_8_CBYCR

PixelType_Gvsp_YCBCR601_422_8 YCBCR601_422_8

PixelType_Gvsp_YCBCR601_422_8_CBYCRY YCBCR601_422_8_CBYCRY

PixelType_Gvsp_YCBCR601_411_8_CBYYCRYY YCBCR601_411_8_CBYYCRYY

PixelType_Gvsp_YCBCR709_8_CBYCR YCBCR709_8_CBYCR

PixelType_Gvsp_YCBCR709_422_8 YCBCR709_422_8

PixelType_Gvsp_YCBCR709_422_8_CBYCRY YCBCR709_422_8_CBYCRY

PixelType_Gvsp_YCBCR709_411_8_CBYYCRYY YCBCR709_411_8_CBYYCRYY

PixelType_Gvsp_YUV420SP_NV12 YUV420SP_NV12

PixelType_Gvsp_YUV420SP_NV21 YUV420SP_NV21

PixelType_Gvsp_RGB8_Planar RGB8_Planar

PixelType_Gvsp_RGB10_Planar RGB10_Planar

PixelType_Gvsp_RGB12_Planar RGB12_Planar

PixelType_Gvsp_RGB16_Planar RGB16_Planar

PixelType_Gvsp_Jpeg Jpeg

PixelType_Gvsp_Coord3D_ABC32f Coord3D_ABC32f

PixelType_Gvsp_Coord3D_ABC32f_Planar Coord3D_ABC32f_Planar

PixelType_Gvsp_Coord3D_ABC32 Coord3D_ABC32

PixelType_Gvsp_Coord3D_AB32f Coord3D_AB32f

PixelType_Gvsp_Coord3D_AB32 Coord3D_AB32

PixelType_Gvsp_Coord3D_AC32f_64 Coord3D_AC32f_64

PixelType_Gvsp_Coord3D_AC32f_Planar Coord3D_AC32f_Planar

PixelType_Gvsp_Coord3D_AC32 Coord3D_AC32

PixelType_Gvsp_Coord3D_A32f Coord3D_A32f

PixelType_Gvsp_Coord3D_A32 Coord3D_A32

PixelType_Gvsp_Coord3D_C32f Coord3D_C32f

PixelType_Gvsp_Coord3D_C32 Coord3D_C32

PixelType_Gvsp_Coord3D_ABC16 Coord3D_ABC16

PixelType_Gvsp_Coord3D_C16 Coord3D_C16

PixelType_Gvsp_HB_Mono8 HB_Mono8

PixelType_Gvsp_HB_Mono10 HB_Mono10

PixelType_Gvsp_HB_Mono10_Packed HB_Mono10_Packed

PixelType_Gvsp_HB_Mono12 HB_Mono12

PixelType_Gvsp_HB_Mono12_Packed HB_Mono12_Packed

PixelType_Gvsp_HB_Mono16 HB_Mono16

PixelType_Gvsp_HB_BayerGR8 HB_BayerGR8

PixelType_Gvsp_HB_BayerRG8 HB_BayerRG8

PixelType_Gvsp_HB_BayerGB8 HB_BayerGB8

PixelType_Gvsp_HB_BayerBG8 HB_BayerBG8

PixelType_Gvsp_HB_BayerRBGG8 HB_BayerRBGG8

PixelType_Gvsp_HB_BayerGR10 HB_BayerGR10

PixelType_Gvsp_HB_BayerRG10 HB_BayerRG10

PixelType_Gvsp_HB_BayerGB10 HB_BayerGB10

PixelType_Gvsp_HB_BayerBG10 HB_BayerBG10

PixelType_Gvsp_HB_BayerGR12 HB_BayerGR12

PixelType_Gvsp_HB_BayerRG12 HB_BayerRG12

PixelType_Gvsp_HB_BayerGB12 HB_BayerGB12

PixelType_Gvsp_HB_BayerBG12 HB_BayerBG12

PixelType_Gvsp_HB_BayerGR10_Packed HB_BayerGR10_Packed

PixelType_Gvsp_HB_BayerRG10_Packed HB_BayerRG10_Packed

PixelType_Gvsp_HB_BayerGB10_Packed HB_BayerGB10_Packed

PixelType_Gvsp_HB_BayerBG10_Packed HB_BayerBG10_Packed

PixelType_Gvsp_HB_BayerGR12_Packed HB_BayerGR12_Packed

PixelType_Gvsp_HB_BayerRG12_Packed HB_BayerRG12_Packed

PixelType_Gvsp_HB_BayerGB12_Packed HB_BayerGB12_Packed

PixelType_Gvsp_HB_BayerBG12_Packed HB_BayerBG12_Packed

PixelType_Gvsp_HB_YUV422_Packed HB_YUV422_Packed

PixelType_Gvsp_HB_YUV422_YUYV_Packed HB_YUV422_YUYV_Packed

PixelType_Gvsp_HB_RGB8_Packed HB_RGB8_Packed

PixelType_Gvsp_HB_BGR8_Packed HB_BGR8_Packed

PixelType_Gvsp_HB_RGBA8_Packed HB_RGBA8_Packed

PixelType_Gvsp_HB_BGRA8_Packed HB_BGRA8_Packed

PixelType_Gvsp_HB_RGB16_Packed HB_RGB16_Packed

PixelType_Gvsp_HB_BGR16_Packed HB_BGR16_Packed

PixelType_Gvsp_HB_RGBA16_Packed HB_RGBA16_Packed

PixelType_Gvsp_HB_BGRA16_Packed HB_BGRA16_Packed

示例 Recording.cpp.

### ￿ MV_SAVE_POINT_CLOUD_FILE_TYPE

enum MV_SAVE_POINT_CLOUD_FILE_TYPE

保存的3D数据格式（自4.4.1版本废弃）

枚举值

MV_PointCloudFile_Undefined 未定义的点云格式

MV_PointCloudFile_PLY PLY点云格式

MV_PointCloudFile_CSV CSV点云格式

MV_PointCloudFile_OBJ OBJ点云格式

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

宏定义

### 宏定义

包含一些常量定义 更多...

### 宏定义

#define MV_UNKNOW_DEVICE 0x00000000 未知设备类型，保留意义 更多...

#define MV_GIGE_DEVICE 0x00000001 GigE设备 更多...

#define MV_1394_DEVICE 0x00000002 1394-a/b 设备 更多...

#define MV_USB_DEVICE 0x00000004 USB设备 更多...

#define MV_CAMERALINK_DEVICE 0x00000008

CameraLink设备（串口） 更多...

#define MV_VIR_GIGE_DEVICE 0x00000010 虚拟GigE设备 更多...

#define MV_VIR_USB_DEVICE 0x00000020 虚拟USB设备 更多...

#define MV_GENTL_GIGE_DEVICE 0x00000040 自研网卡下GigE设备 更多...

#define MV_GENTL_CAMERALINK_DEVICE 0x00000080 CameraLink设备 更多...

#define MV_GENTL_CXP_DEVICE 0x00000100 CoaXPress设备 更多...

#define MV_GENTL_XOF_DEVICE 0x00000200 XoF设备 更多...

#define INFO_MAX_BUFFER_SIZE 64 最大的数据信息大小 更多...

#define MV_MAX_TLS_NUM 8 最多支持的传输层实例个数 更多...

#define MV_MAX_DEVICE_NUM 256 最大支持的设备个数 更多...

#define MV_MAX_INTERFACE_NUM 64 最大支持的采集卡数量 更多...

#define MV_MAX_GENTL_IF_NUM 256 最大支持的GenTL接口数量 更多...

#define MV_MAX_GENTL_DEV_NUM 256 最大支持的GenTL设备数量 更多...

#define MV_MAX_SERIAL_PORT_NUM 64

最大支持的串口数量 更多...

#define MV_MAX_NODE_NUM 256 最大节点个数 更多...

#define MV_MAX_NODE_NAME_LEN 64 节点名称的最大长度 更多...

#define MV_MAX_NODE_ERROR_NUM 64 最大节点错误个数 更多...

#define MV_GIGE_INTERFACE 0x00000001 GigE Vision采集卡 更多...

#define MV_CAMERALINK_INTERFACE 0x00000004 Camera Link采集卡 更多...

#define MV_CXP_INTERFACE 0x00000008 CoaXPress采集卡 更多...

#define MV_XOF_INTERFACE 0x00000010 XoFLink采集卡 更多...

#define MV_ACCESS_Exclusive 1 独占权限，其他APP只允许读CCP寄存器 更多...

#define MV_ACCESS_ExclusiveWithSwitch 2 可以从5模式下抢占权限，然后以独占权限打开 更多...

#define MV_ACCESS_Control 3 控制权限，其他APP允许读所有寄存器 更多...

#define MV_ACCESS_ControlWithSwitch 4 可以从5模式下抢占权限，然后以控制权限打开 更多...

#define MV_ACCESS_ControlSwitchEnable 5 以可被抢占的控制权限打开 更多...

#define MV_ACCESS_ControlSwitchEnableWithKey 6

可以从5模式下抢占权限，然后以可被抢占的控制权限打开 更多...

#define MV_ACCESS_Monitor 7 读模式打开设备，适用于控制权限下 更多...

#define MV_MATCH_TYPE_NET_DETECT 0x00000001 网络流量和丢包信息 更多...

#define MV_MATCH_TYPE_USB_DETECT 0x00000002 host接收到来自U3V设备的字节总数 更多...

#define MV_IP_CFG_STATIC 0x05000000 静态 更多...

#define MV_IP_CFG_DHCP 0x06000000 DHCP 更多...

#define MV_IP_CFG_LLA 0x04000000 LLA 更多...

#define MV_NET_TRANS_DRIVER 0x00000001 驱动 更多...

#define MV_NET_TRANS_SOCKET 0x00000002 Socket 更多...

#define MV_CAML_BAUDRATE_9600 0x00000001 9600 更多...

#define MV_CAML_BAUDRATE_19200 0x00000002 19200 更多...

#define MV_CAML_BAUDRATE_38400 0x00000004 38400 更多...

#define MV_CAML_BAUDRATE_57600 0x00000008 57600 更多...

#define MV_CAML_BAUDRATE_115200 0x00000010

115200 更多...

#define MV_CAML_BAUDRATE_230400 0x00000020 230400 更多...

#define MV_CAML_BAUDRATE_460800 0x00000040 460800 更多...

#define MV_CAML_BAUDRATE_921600 0x00000080 921600 更多...

#define MV_CAML_BAUDRATE_AUTOMAX 0x40000000 最大值 更多...

#define MV_EXCEPTION_DEV_DISCONNECT 0x00008001 设备断开连接 更多...

#define MV_EXCEPTION_VERSION_CHECK 0x00008002 SDK与驱动版本不匹配 更多...

#define MAX_EVENT_NAME_SIZE 128 设备Event事件名称最大长度 更多...

#define MV_MAX_XML_SYMBOLIC_NUM 64 最大XML符号数 更多...

#define MV_MAX_SYMBOLIC_LEN 64 最大枚举条目对应的符号长度 更多...

#define MV_MAX_SPLIT_NUM 8 分时曝光时最多将源图像拆分的个数 更多...

#define MV_MAX_ENUM_SYMBOLIC_NUM 256 最大枚举条目对应的符号数量 更多...

#define MV_GVSP_PIX_MONO 0x01000000 Mono格式 更多...

#define MV_GVSP_PIX_RGB 0x02000000

RGB格式 更多...

#define MV_GVSP_PIX_COLOR 0x02000000 COLOR格式 更多...

#define MV_GVSP_PIX_CUSTOM 0x80000000 CUSTOM格式 更多...

#define MV_GVSP_PIX_COLOR_MASK 0xFF000000 COLOR_MASK格式 更多...

#define MV_PIXEL_BIT_COUNT(n) ((n) << 16) 像素占用的有效位数，可用来计算存储图像所需的内存量 更多...

### 详细描述

包含一些常量定义

### 宏定义说明

### ￿ MV_UNKNOW_DEVICE

#define MV_UNKNOW_DEVICE 0x00000000

未知设备类型，保留意义

### ￿ MV_GIGE_DEVICE

#define MV_GIGE_DEVICE 0x00000001

GigE设备

示例 ChunkData.cpp, ConnectSpecCamera.cpp, ConvertPixelType.cpp, DynamicallyLoadDLL.cpp, Events.cpp, Events_Interface.cpp, ForceIPEx.cpp,

Grab_ActionCommand.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, InterfaceAndDeviceDemo.cpp, LensShadingCorrection.cpp, MultiCast.cpp, MultiLightCtrl_ImageStitching.cpp, ParametrizeCamera_FileAccess.cpp, ParametrizeCamera_LineScanIOSettings.cpp, ParametrizeCamera_LoadAndSave.cpp , 以及 Recording.cpp.

### ￿ MV_1394_DEVICE

#define MV_1394_DEVICE 0x00000002

1394-a/b 设备

### ￿ MV_USB_DEVICE

#define MV_USB_DEVICE 0x00000004

USB设备

示例 ChunkData.cpp, ConvertPixelType.cpp, DynamicallyLoadDLL.cpp, Events.cpp, Events_Interface.cpp, ForceIPEx.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, InterfaceAndDeviceDemo.cpp, LensShadingCorrection.cpp, MultiLightCtrl_ImageStitching.cpp, ParametrizeCamera_FileAccess.cpp, ParametrizeCamera_LineScanIOSettings.cpp, ParametrizeCamera_LoadAndSave.cpp , 以及 Recording.cpp.

### ￿ MV_CAMERALINK_DEVICE

#define MV_CAMERALINK_DEVICE 0x00000008

CameraLink设备（串口）

### ￿ MV_VIR_GIGE_DEVICE

#define MV_VIR_GIGE_DEVICE 0x00000010

虚拟GigE设备

### ￿ MV_VIR_USB_DEVICE

#define MV_VIR_USB_DEVICE 0x00000020

虚拟USB设备

### ￿ MV_GENTL_GIGE_DEVICE

#define MV_GENTL_GIGE_DEVICE 0x00000040

自研网卡下GigE设备

示例 ChunkData.cpp, ConvertPixelType.cpp, DynamicallyLoadDLL.cpp, Events.cpp, Events_Interface.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, InterfaceAndDeviceDemo.cpp, LensShadingCorrection.cpp, MultiLightCtrl_ImageStitching.cpp, ParametrizeCamera_LineScanIOSettings.cpp , 以及 Recording.cpp.

### ￿ MV_GENTL_CAMERALINK_DEVICE

#define MV_GENTL_CAMERALINK_DEVICE 0x00000080

CameraLink设备

示例 ChunkData.cpp, ConvertPixelType.cpp, DynamicallyLoadDLL.cpp, Events.cpp, Events_Interface.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, InterfaceAndDeviceDemo.cpp, LensShadingCorrection.cpp,

MultiLightCtrl_ImageStitching.cpp, ParametrizeCamera_LineScanIOSettings.cpp , 以及 Recording.cpp.

### ￿ MV_GENTL_CXP_DEVICE

#define MV_GENTL_CXP_DEVICE 0x00000100

CoaXPress设备

示例 ChunkData.cpp, ConvertPixelType.cpp, DynamicallyLoadDLL.cpp, Events.cpp, Events_Interface.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, InterfaceAndDeviceDemo.cpp, LensShadingCorrection.cpp, MultiLightCtrl_ImageStitching.cpp, ParametrizeCamera_LineScanIOSettings.cpp, QuickSoftwareTrigger.cpp , 以及 Recording.cpp.

### ￿ MV_GENTL_XOF_DEVICE

#define MV_GENTL_XOF_DEVICE 0x00000200

XoF设备

示例 ChunkData.cpp, ConvertPixelType.cpp, DynamicallyLoadDLL.cpp, Events.cpp, Events_Interface.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, InterfaceAndDeviceDemo.cpp, LensShadingCorrection.cpp, MultiLightCtrl_ImageStitching.cpp, ParametrizeCamera_LineScanIOSettings.cpp, QuickSoftwareTrigger.cpp , 以及 Recording.cpp.

### ￿ INFO_MAX_BUFFER_SIZE

#define INFO_MAX_BUFFER_SIZE 64

最大的数据信息大小

### ￿ MV_MAX_TLS_NUM

#define MV_MAX_TLS_NUM 8

最多支持的传输层实例个数

### ￿ MV_MAX_DEVICE_NUM

#define MV_MAX_DEVICE_NUM 256

最大支持的设备个数

### ￿ MV_MAX_INTERFACE_NUM

#define MV_MAX_INTERFACE_NUM 64

最大支持的采集卡数量

### ￿ MV_MAX_GENTL_IF_NUM

#define MV_MAX_GENTL_IF_NUM 256

最大支持的GenTL接口数量

### ￿ MV_MAX_GENTL_DEV_NUM

#define MV_MAX_GENTL_DEV_NUM 256

最大支持的GenTL设备数量

### ￿ MV_MAX_SERIAL_PORT_NUM

#define MV_MAX_SERIAL_PORT_NUM 64

最大支持的串口数量

### ￿ MV_MAX_NODE_NUM

#define MV_MAX_NODE_NUM 256

最大节点个数

### ￿ MV_MAX_NODE_NAME_LEN

#define MV_MAX_NODE_NAME_LEN 64

节点名称的最大长度

### ￿ MV_MAX_NODE_ERROR_NUM

#define MV_MAX_NODE_ERROR_NUM 64

最大节点错误个数

### ￿ MV_GIGE_INTERFACE

#define MV_GIGE_INTERFACE 0x00000001

GigE Vision采集卡

示例 Events_Interface.cpp, InterfaceAndDeviceDemo.cpp , 以及 InterfaceDemo.cpp.

### ￿ MV_CAMERALINK_INTERFACE

#define MV_CAMERALINK_INTERFACE 0x00000004

Camera Link采集卡

示例 Events_Interface.cpp, InterfaceAndDeviceDemo.cpp , 以及 InterfaceDemo.cpp.

### ￿ MV_CXP_INTERFACE

#define MV_CXP_INTERFACE 0x00000008

CoaXPress采集卡

示例 Events_Interface.cpp, InterfaceAndDeviceDemo.cpp, InterfaceDemo.cpp , 以及 QuickSoftwareTrigger.cpp.

### ￿ MV_XOF_INTERFACE

#define MV_XOF_INTERFACE 0x00000010

XoFLink采集卡

示例 Events_Interface.cpp, InterfaceAndDeviceDemo.cpp, InterfaceDemo.cpp , 以及 QuickSoftwareTrigger.cpp.

### ￿ MV_ACCESS_Exclusive

#define MV_ACCESS_Exclusive 1

独占权限，其他APP只允许读CCP寄存器

示例 DynamicallyLoadDLL.cpp , 以及 ForceIPEx.cpp.

### ￿ MV_ACCESS_ExclusiveWithSwitch

#define MV_ACCESS_ExclusiveWithSwitch 2

可以从5模式下抢占权限，然后以独占权限打开

### ￿ MV_ACCESS_Control

#define MV_ACCESS_Control 3

控制权限，其他APP允许读所有寄存器

示例 MultiCast.cpp.

### ￿ MV_ACCESS_ControlWithSwitch

#define MV_ACCESS_ControlWithSwitch 4

可以从5模式下抢占权限，然后以控制权限打开

### ￿ MV_ACCESS_ControlSwitchEnable

#define MV_ACCESS_ControlSwitchEnable 5

以可被抢占的控制权限打开

### ￿ MV_ACCESS_ControlSwitchEnableWithKey

#define MV_ACCESS_ControlSwitchEnableWithKey 6

可以从5模式下抢占权限，然后以可被抢占的控制权限打开

### ￿ MV_ACCESS_Monitor

#define MV_ACCESS_Monitor 7

读模式打开设备，适用于控制权限下

示例 MultiCast.cpp.

### ￿ MV_MATCH_TYPE_NET_DETECT

#define MV_MATCH_TYPE_NET_DETECT 0x00000001

网络流量和丢包信息

### ￿ MV_MATCH_TYPE_USB_DETECT

#define MV_MATCH_TYPE_USB_DETECT 0x00000002

host接收到来自U3V设备的字节总数

### ￿ MV_IP_CFG_STATIC

#define MV_IP_CFG_STATIC 0x05000000

静态

示例 ForceIPEx.cpp.

### ￿ MV_IP_CFG_DHCP

#define MV_IP_CFG_DHCP 0x06000000

DHCP

### ￿ MV_IP_CFG_LLA

#define MV_IP_CFG_LLA 0x04000000

LLA

### ￿ MV_NET_TRANS_DRIVER

#define MV_NET_TRANS_DRIVER 0x00000001

驱动

### ￿ MV_NET_TRANS_SOCKET

#define MV_NET_TRANS_SOCKET 0x00000002

Socket

### ￿ MV_CAML_BAUDRATE_9600

#define MV_CAML_BAUDRATE_9600 0x00000001

### ￿ MV_CAML_BAUDRATE_19200

#define MV_CAML_BAUDRATE_19200 0x00000002

### ￿ MV_CAML_BAUDRATE_38400

#define MV_CAML_BAUDRATE_38400 0x00000004

### ￿ MV_CAML_BAUDRATE_57600

#define MV_CAML_BAUDRATE_57600 0x00000008

### ￿ MV_CAML_BAUDRATE_115200

#define MV_CAML_BAUDRATE_115200 0x00000010

### ￿ MV_CAML_BAUDRATE_230400

#define MV_CAML_BAUDRATE_230400 0x00000020

### ￿ MV_CAML_BAUDRATE_460800

#define MV_CAML_BAUDRATE_460800 0x00000040

### ￿ MV_CAML_BAUDRATE_921600

#define MV_CAML_BAUDRATE_921600 0x00000080

### ￿ MV_CAML_BAUDRATE_AUTOMAX

#define MV_CAML_BAUDRATE_AUTOMAX 0x40000000

最大值

### ￿ MV_EXCEPTION_DEV_DISCONNECT

#define MV_EXCEPTION_DEV_DISCONNECT 0x00008001

设备断开连接

### ￿ MV_EXCEPTION_VERSION_CHECK

#define MV_EXCEPTION_VERSION_CHECK 0x00008002

SDK与驱动版本不匹配

### ￿ MAX_EVENT_NAME_SIZE

#define MAX_EVENT_NAME_SIZE 128

设备Event事件名称最大长度

### ￿ MV_MAX_XML_SYMBOLIC_NUM

#define MV_MAX_XML_SYMBOLIC_NUM 64

最大XML符号数

### ￿ MV_MAX_SYMBOLIC_LEN

#define MV_MAX_SYMBOLIC_LEN 64

最大枚举条目对应的符号长度

### ￿ MV_MAX_SPLIT_NUM

#define MV_MAX_SPLIT_NUM 8

分时曝光时最多将源图像拆分的个数

### ￿ MV_MAX_ENUM_SYMBOLIC_NUM

#define MV_MAX_ENUM_SYMBOLIC_NUM 256

最大枚举条目对应的符号数量

### ￿ MV_GVSP_PIX_MONO

#define MV_GVSP_PIX_MONO 0x01000000

Mono格式

### ￿ MV_GVSP_PIX_RGB

#define MV_GVSP_PIX_RGB 0x02000000

RGB格式

### ￿ MV_GVSP_PIX_COLOR

#define MV_GVSP_PIX_COLOR 0x02000000

COLOR格式

### ￿ MV_GVSP_PIX_CUSTOM

#define MV_GVSP_PIX_CUSTOM 0x80000000

CUSTOM格式

### ￿ MV_GVSP_PIX_COLOR_MASK

#define MV_GVSP_PIX_COLOR_MASK 0xFF000000

COLOR_MASK格式

### ￿ MV_PIXEL_BIT_COUNT

#define MV_PIXEL_BIT_COUNT ( n ) ((n) << 16)

像素占用的有效位数，可用来计算存储图像所需的内存量

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

模块

### 错误码

介绍SDK使用过程中不同状态码对应的值和说明 更多...

### 模块

SDK内部自定义状态码

算法库透传状态码

### 详细描述

介绍SDK使用过程中不同状态码对应的值和说明

当SDK接口被调用时，返回一个代表int型的状态码。这个状态码用于表示接口的调用状态。状态码 以16进制形式呈现，格式为“0x%x”。 您可在本节中查找各个状态码及其对应解释。例如，状态码“0x00000000”表示接口调用成功，没有 遇到错误。 如下示例代码通过打开相机设备，演示了检查函数异常返回的过程。

int nRet = MV_OK; nRet = MV_CC_OpenDevice(handle); if (MV_OK != nRet) { printf("Open Device fail! nRet [0x%x]\n", nRet); return nRet; }

在该例中，若相机设备已连接至第三方软件，则再次连接设备时报错：

Open Device fail! nRet [0x80000203]

通过查找本节内容，您可知道“0x80000203”代表“设备无访问权限”。

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

模块

### SDK内部自定义状态码

错误码

### 模块

正确码 正确码为0x00000000

通用错误码 范围为0x80000000-0x800000FF

GenICam错误码 范围为0x80000100-0x800001FF

GigE_STATUS错误码 范围为0x80000200-0x800002FF

USB_STATUS错误码

范围为0x80000300-0x800003FF

升级错误码 范围为0x80000400-0x800004FF

异常错误码 0x00008001、0x00008002

### 详细描述

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

宏定义

### 正确码

错误码 » SDK内部自定义状态码

正确码为0x00000000 更多...

### 宏定义

#define MV_OK 0x00000000 成功，无错误 更多...

### 详细描述

正确码为0x00000000

### 宏定义说明

### ￿ MV_OK

#define MV_OK 0x00000000

成功，无错误

示例 ChunkData.cpp, ConnectSpecCamera.cpp, ConvertPixelType.cpp, DynamicallyLoadDLL.cpp, Events.cpp, Events_Interface.cpp, ForceIPEx.cpp, Grab_ActionCommand.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, InterfaceAndDeviceDemo.cpp, InterfaceDemo.cpp, LensShadingCorrection.cpp, MultiCast.cpp, MultiLightCtrl_ImageStitching.cpp, ParametrizeCamera_FileAccess.cpp, ParametrizeCamera_LineScanIOSettings.cpp, ParametrizeCamera_LoadAndSave.cpp, QuickSoftwareTrigger.cpp , 以及 Recording.cpp.

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

宏定义

### 通用错误码

错误码 » SDK内部自定义状态码

范围为0x80000000-0x800000FF 更多...

### 宏定义

#define MV_E_HANDLE 0x80000000 错误或无效的句柄 更多...

#define MV_E_SUPPORT 0x80000001 不支持的功能 更多...

#define MV_E_BUFOVER 0x80000002 缓存已满 更多...

#define MV_E_CALLORDER 0x80000003 函数调用顺序有误 更多...

#define MV_E_PARAMETER 0x80000004 错误的参数 更多...

#define MV_E_RESOURCE 0x80000006 资源申请失败 更多...

#define MV_E_NODATA 0x80000007 无数据 更多...

#define MV_E_PRECONDITION 0x80000008 前置条件有误，或运行环境已发生变化 更多...

#define MV_E_VERSION 0x80000009 版本不匹配 更多...

#define MV_E_NOENOUGH_BUF 0x8000000A 传入的内存空间不足 更多...

#define MV_E_ABNORMAL_IMAGE 0x8000000B 异常图像，可能是丢包导致图像不完整 更多...

#define MV_E_LOAD_LIBRARY 0x8000000C 动态导入DLL失败 更多...

#define MV_E_NOOUTBUF 0x8000000D 没有可输出的缓存 更多...

#define MV_E_ENCRYPT 0x8000000E 加密错误 更多...

#define MV_E_OPENFILE 0x8000000F 打开文件出现错误 更多...

#define MV_E_BUF_IN_USE 0x80000010 缓存地址已使用 更多...

#define MV_E_BUF_INVALID 0x80000011 无效的缓存地址 更多...

#define MV_E_NOALIGN_BUF 0x80000012 缓存对齐异常 更多...

#define MV_E_NOENOUGH_BUF_NUM 0x80000013 缓存个数不足 更多...

#define MV_E_PORT_IN_USE 0x80000014 串口被占用 更多...

#define MV_E_IMAGE_DECODEC 0x80000015 解码错误(SDK校验图像异常) 更多...

#define MV_E_UINT32_LIMIT 0x80000016 图像大小超出unsigned int数据类型上限值 更多...

#define MV_E_UNKNOW 0x800000FF 未知的错误 更多...

### 详细描述

范围为0x80000000-0x800000FF

### 宏定义说明

### ￿ MV_E_HANDLE

#define MV_E_HANDLE 0x80000000

错误或无效的句柄

### ￿ MV_E_SUPPORT

#define MV_E_SUPPORT 0x80000001

不支持的功能

### ￿ MV_E_BUFOVER

#define MV_E_BUFOVER 0x80000002

缓存已满

示例 Grab_Asynchronous.cpp.

### ￿ MV_E_CALLORDER

#define MV_E_CALLORDER 0x80000003

函数调用顺序有误

### ￿ MV_E_PARAMETER

#define MV_E_PARAMETER 0x80000004

错误的参数

### ￿ MV_E_RESOURCE

#define MV_E_RESOURCE 0x80000006

资源申请失败

示例 Grab_Asynchronous.cpp, HighBandwidthDecode.cpp, ImageSave.cpp , 以及 MultiLightCtrl_ImageStitching.cpp.

### ￿ MV_E_NODATA

#define MV_E_NODATA 0x80000007

无数据

示例 Grab_Asynchronous.cpp.

### ￿ MV_E_PRECONDITION

#define MV_E_PRECONDITION 0x80000008

前置条件有误，或运行环境已发生变化

### ￿ MV_E_VERSION

#define MV_E_VERSION 0x80000009

版本不匹配

### ￿ MV_E_NOENOUGH_BUF

#define MV_E_NOENOUGH_BUF 0x8000000A

传入的内存空间不足

### ￿ MV_E_ABNORMAL_IMAGE

#define MV_E_ABNORMAL_IMAGE 0x8000000B

异常图像，可能是丢包导致图像不完整

### ￿ MV_E_LOAD_LIBRARY

#define MV_E_LOAD_LIBRARY 0x8000000C

动态导入DLL失败

### ￿ MV_E_NOOUTBUF

#define MV_E_NOOUTBUF 0x8000000D

没有可输出的缓存

### ￿ MV_E_ENCRYPT

#define MV_E_ENCRYPT 0x8000000E

加密错误

### ￿ MV_E_OPENFILE

#define MV_E_OPENFILE 0x8000000F

打开文件出现错误

### ￿ MV_E_BUF_IN_USE

#define MV_E_BUF_IN_USE 0x80000010

缓存地址已使用

### ￿ MV_E_BUF_INVALID

#define MV_E_BUF_INVALID 0x80000011

无效的缓存地址

### ￿ MV_E_NOALIGN_BUF

#define MV_E_NOALIGN_BUF 0x80000012

缓存对齐异常

### ￿ MV_E_NOENOUGH_BUF_NUM

#define MV_E_NOENOUGH_BUF_NUM 0x80000013

缓存个数不足

### ￿ MV_E_PORT_IN_USE

#define MV_E_PORT_IN_USE 0x80000014

串口被占用

### ￿ MV_E_IMAGE_DECODEC

#define MV_E_IMAGE_DECODEC 0x80000015

解码错误(SDK校验图像异常)

### ￿ MV_E_UINT32_LIMIT

#define MV_E_UINT32_LIMIT 0x80000016

图像大小超出unsigned int数据类型上限值

### ￿ MV_E_UNKNOW

#define MV_E_UNKNOW 0x800000FF

未知的错误

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

宏定义

### GenICam错误码

错误码 » SDK内部自定义状态码

范围为0x80000100-0x800001FF 更多...

### 宏定义

#define MV_E_GC_GENERIC 0x80000100 通用错误 更多...

#define MV_E_GC_ARGUMENT 0x80000101 参数非法 更多...

#define MV_E_GC_RANGE 0x80000102 值超出范围 更多...

#define MV_E_GC_PROPERTY 0x80000103 属性 更多...

#define MV_E_GC_RUNTIME 0x80000104 运行环境有问题 更多...

#define MV_E_GC_LOGICAL 0x80000105 逻辑错误 更多...

#define MV_E_GC_ACCESS 0x80000106 节点访问条件有误 更多...

#define MV_E_GC_TIMEOUT 0x80000107 超时 更多...

#define MV_E_GC_DYNAMICCAST 0x80000108

转换异常 更多...

#define MV_E_GC_UNKNOW 0x800001FF GenICam未知错误 更多...

### 详细描述

范围为0x80000100-0x800001FF

### 宏定义说明

### ￿ MV_E_GC_GENERIC

#define MV_E_GC_GENERIC 0x80000100

通用错误

### ￿ MV_E_GC_ARGUMENT

#define MV_E_GC_ARGUMENT 0x80000101

参数非法

### ￿ MV_E_GC_RANGE

#define MV_E_GC_RANGE 0x80000102

值超出范围

### ￿ MV_E_GC_PROPERTY

#define MV_E_GC_PROPERTY 0x80000103

属性

### ￿ MV_E_GC_RUNTIME

#define MV_E_GC_RUNTIME 0x80000104

运行环境有问题

### ￿ MV_E_GC_LOGICAL

#define MV_E_GC_LOGICAL 0x80000105

逻辑错误

### ￿ MV_E_GC_ACCESS

#define MV_E_GC_ACCESS 0x80000106

节点访问条件有误

### ￿ MV_E_GC_TIMEOUT

#define MV_E_GC_TIMEOUT 0x80000107

超时

### ￿ MV_E_GC_DYNAMICCAST

#define MV_E_GC_DYNAMICCAST 0x80000108

转换异常

### ￿ MV_E_GC_UNKNOW

#define MV_E_GC_UNKNOW 0x800001FF

GenICam未知错误

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

宏定义

### GigE_STATUS错误码

错误码 » SDK内部自定义状态码

范围为0x80000200-0x800002FF 更多...

### 宏定义

#define MV_E_NOT_IMPLEMENTED 0x80000200 命令不被设备支持 更多...

#define MV_E_INVALID_ADDRESS 0x80000201 访问的目标地址不存在 更多...

#define MV_E_WRITE_PROTECT 0x80000202 目标地址不可写 更多...

#define MV_E_ACCESS_DENIED 0x80000203 设备无访问权限 更多...

#define MV_E_BUSY 0x80000204 设备忙，或网络断开 更多...

#define MV_E_PACKET 0x80000205 网络包数据错误 更多...

#define MV_E_NETER 0x80000206 网络相关错误 更多...

#define MV_E_SUPPORT_MODIFY_DEVICE_IP 0x8000020E 在固定IP模式下不支持修改设备IP模式 更多...

#define MV_E_KEY_VERIFICATION 0x8000020F 秘钥校验错误 更多...

#define MV_E_IP_CONFLICT 0x80000221 设备IP冲突 更多...

### 详细描述

范围为0x80000200-0x800002FF

### 宏定义说明

### ￿ MV_E_NOT_IMPLEMENTED

#define MV_E_NOT_IMPLEMENTED 0x80000200

命令不被设备支持

### ￿ MV_E_INVALID_ADDRESS

#define MV_E_INVALID_ADDRESS 0x80000201

访问的目标地址不存在

### ￿ MV_E_WRITE_PROTECT

#define MV_E_WRITE_PROTECT 0x80000202

目标地址不可写

### ￿ MV_E_ACCESS_DENIED

#define MV_E_ACCESS_DENIED 0x80000203

设备无访问权限

### ￿ MV_E_BUSY

#define MV_E_BUSY 0x80000204

设备忙，或网络断开

### ￿ MV_E_PACKET

#define MV_E_PACKET 0x80000205

网络包数据错误

### ￿ MV_E_NETER

#define MV_E_NETER 0x80000206

网络相关错误

### ￿ MV_E_SUPPORT_MODIFY_DEVICE_IP

#define MV_E_SUPPORT_MODIFY_DEVICE_IP 0x8000020E

在固定IP模式下不支持修改设备IP模式

### ￿ MV_E_KEY_VERIFICATION

#define MV_E_KEY_VERIFICATION 0x8000020F

秘钥校验错误

### ￿ MV_E_IP_CONFLICT

#define MV_E_IP_CONFLICT 0x80000221

设备IP冲突

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

宏定义

### USB_STATUS错误码

错误码 » SDK内部自定义状态码

范围为0x80000300-0x800003FF 更多...

### 宏定义

#define MV_E_USB_READ 0x80000300 读USB出错 更多...

#define MV_E_USB_WRITE 0x80000301 写USB出错 更多...

#define MV_E_USB_DEVICE 0x80000302 设备异常 更多...

#define MV_E_USB_GENICAM 0x80000303 GenICam相关错误 更多...

#define MV_E_USB_BANDWIDTH 0x80000304 带宽不足 更多...

#define MV_E_USB_DRIVER 0x80000305 驱动不匹配或者未装驱动 更多...

#define MV_E_USB_UNKNOW 0x800003FF USB未知的错误 更多...

### 详细描述

范围为0x80000300-0x800003FF

### 宏定义说明

### ￿ MV_E_USB_READ

#define MV_E_USB_READ 0x80000300

读USB出错

### ￿ MV_E_USB_WRITE

#define MV_E_USB_WRITE 0x80000301

写USB出错

### ￿ MV_E_USB_DEVICE

#define MV_E_USB_DEVICE 0x80000302

设备异常

### ￿ MV_E_USB_GENICAM

#define MV_E_USB_GENICAM 0x80000303

GenICam相关错误

### ￿ MV_E_USB_BANDWIDTH

#define MV_E_USB_BANDWIDTH 0x80000304

带宽不足

### ￿ MV_E_USB_DRIVER

#define MV_E_USB_DRIVER 0x80000305

驱动不匹配或者未装驱动

### ￿ MV_E_USB_UNKNOW

#define MV_E_USB_UNKNOW 0x800003FF

USB未知的错误

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

宏定义

### 升级错误码

错误码 » SDK内部自定义状态码

范围为0x80000400-0x800004FF 更多...

### 宏定义

#define MV_E_UPG_FILE_MISMATCH 0x80000400 升级固件不匹配 更多...

#define MV_E_UPG_LANGUSGE_MISMATCH 0x80000401 升级固件语言不匹配 更多...

#define MV_E_UPG_CONFLICT 0x80000402 升级冲突（设备已经在升级了再次请求升级即返回此错误） 更多...

#define MV_E_UPG_INNER_ERR 0x80000403 升级时设备内部出现错误 更多...

#define MV_E_UPG_UNKNOW 0x800004FF 升级时未知错误 更多...

### 详细描述

范围为0x80000400-0x800004FF

### 宏定义说明

### ￿ MV_E_UPG_FILE_MISMATCH

#define MV_E_UPG_FILE_MISMATCH 0x80000400

升级固件不匹配

### ￿ MV_E_UPG_LANGUSGE_MISMATCH

#define MV_E_UPG_LANGUSGE_MISMATCH 0x80000401

升级固件语言不匹配

### ￿ MV_E_UPG_CONFLICT

#define MV_E_UPG_CONFLICT 0x80000402

升级冲突（设备已经在升级了再次请求升级即返回此错误）

### ￿ MV_E_UPG_INNER_ERR

#define MV_E_UPG_INNER_ERR 0x80000403

升级时设备内部出现错误

### ￿ MV_E_UPG_UNKNOW

#define MV_E_UPG_UNKNOW 0x800004FF

升级时未知错误

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

宏定义

### 异常错误码

错误码 » SDK内部自定义状态码

0x00008001、0x00008002 更多...

### 宏定义

#define MV_EXCEPTION_DEV_DISCONNECT 0x00008001 设备断开连接 更多...

#define MV_EXCEPTION_VERSION_CHECK 0x00008002 SDK与驱动版本不匹配 更多...

### 详细描述

0x00008001、0x00008002

### 宏定义说明

### ￿ MV_EXCEPTION_DEV_DISCONNECT

#define MV_EXCEPTION_DEV_DISCONNECT 0x00008001

设备断开连接

### ￿ MV_EXCEPTION_VERSION_CHECK

#define MV_EXCEPTION_VERSION_CHECK 0x00008002

SDK与驱动版本不匹配

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

模块

### 算法库透传状态码

错误码

### 模块

通用类型错误码 0x00000000、0x10000000

能力检查错误码 0x10000001

内存检查错误码 范围为0x10000002-0x10000006

图像检查错误码 范围为0x10000007-0x1000000A

输入输出参数检查错误码 范围为0x1000000B-0x10000010

能运行参数检查错误码 范围为0x10000011-0x10000013

接口调用检查错误码 范围为0x10000014-0x10000016

算法库加密相关检查错误码 范围为0x10000017-0x10000018

基本错误类型错误码 范围为0x10000019-0x1000001B

降噪特有错误码

范围为0x10402001-0x1040200f

其他错误码 范围为0x1000001C-0x1000002C

### 详细描述

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

宏定义

### 通用类型错误码

错误码 » 算法库透传状态码

0x00000000、0x10000000 更多...

### 宏定义

#define MV_ALG_OK 0x00000000 处理正确 更多...

#define MV_ALG_ERR 0x10000000 不确定类型错误 更多...

### 详细描述

0x00000000、0x10000000

### 宏定义说明

### ￿ MV_ALG_OK

#define MV_ALG_OK 0x00000000

处理正确

### ￿ MV_ALG_ERR

#define MV_ALG_ERR 0x10000000

不确定类型错误

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

宏定义

### 能力检查错误码

错误码 » 算法库透传状态码

0x10000001 更多...

### 宏定义

#define MV_ALG_E_ABILITY_ARG 0x10000001 能力集中存在无效参数 更多...

### 详细描述

0x10000001

### 宏定义说明

### ￿ MV_ALG_E_ABILITY_ARG

#define MV_ALG_E_ABILITY_ARG 0x10000001

能力集中存在无效参数

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

宏定义

### 内存检查错误码

错误码 » 算法库透传状态码

范围为0x10000002-0x10000006 更多...

### 宏定义

#define MV_ALG_E_MEM_NULL 0x10000002 内存地址为空 更多...

#define MV_ALG_E_MEM_ALIGN 0x10000003 内存对齐不满足要求 更多...

#define MV_ALG_E_MEM_LACK 0x10000004 内存空间大小不够 更多...

#define MV_ALG_E_MEM_SIZE_ALIGN 0x10000005 内存空间大小不满足对齐要求 更多...

#define MV_ALG_E_MEM_ADDR_ALIGN 0x10000006 内存地址不满足对齐要求 更多...

### 详细描述

范围为0x10000002-0x10000006

### 宏定义说明

### ￿ MV_ALG_E_MEM_NULL

#define MV_ALG_E_MEM_NULL 0x10000002

内存地址为空

### ￿ MV_ALG_E_MEM_ALIGN

#define MV_ALG_E_MEM_ALIGN 0x10000003

内存对齐不满足要求

### ￿ MV_ALG_E_MEM_LACK

#define MV_ALG_E_MEM_LACK 0x10000004

内存空间大小不够

### ￿ MV_ALG_E_MEM_SIZE_ALIGN

#define MV_ALG_E_MEM_SIZE_ALIGN 0x10000005

内存空间大小不满足对齐要求

### ￿ MV_ALG_E_MEM_ADDR_ALIGN

#define MV_ALG_E_MEM_ADDR_ALIGN 0x10000006

内存地址不满足对齐要求

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

宏定义

### 图像检查错误码

错误码 » 算法库透传状态码

范围为0x10000007-0x1000000A 更多...

### 宏定义

#define MV_ALG_E_IMG_FORMAT 0x10000007 图像格式不正确或者不支持 更多...

#define MV_ALG_E_IMG_SIZE 0x10000008 图像宽高不正确或者超出范围 更多...

#define MV_ALG_E_IMG_STEP 0x10000009 图像宽高与step参数不匹配 更多...

#define MV_ALG_E_IMG_DATA_NULL 0x1000000A 图像数据存储地址为空（某个分量） 更多...

### 详细描述

范围为0x10000007-0x1000000A

### 宏定义说明

### ￿ MV_ALG_E_IMG_FORMAT

#define MV_ALG_E_IMG_FORMAT 0x10000007

图像格式不正确或者不支持

### ￿ MV_ALG_E_IMG_SIZE

#define MV_ALG_E_IMG_SIZE 0x10000008

图像宽高不正确或者超出范围

### ￿ MV_ALG_E_IMG_STEP

#define MV_ALG_E_IMG_STEP 0x10000009

图像宽高与step参数不匹配

### ￿ MV_ALG_E_IMG_DATA_NULL

#define MV_ALG_E_IMG_DATA_NULL 0x1000000A

图像数据存储地址为空（某个分量）

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

宏定义

### 输入输出参数检查错误码

错误码 » 算法库透传状态码

范围为0x1000000B-0x10000010 更多...

### 宏定义

#define MV_ALG_E_CFG_TYPE 0x1000000B 设置或者获取参数类型不正确 更多...

#define MV_ALG_E_CFG_SIZE 0x1000000C 设置或者获取参数的输入、输出结构体大小不正确 更多...

#define MV_ALG_E_PRC_TYPE 0x1000000D 处理类型不正确 更多...

#define MV_ALG_E_PRC_SIZE 0x1000000E 处理时输入、输出参数大小不正确 更多...

#define MV_ALG_E_FUNC_TYPE 0x1000000F 子处理类型不正确 更多...

#define MV_ALG_E_FUNC_SIZE 0x10000010 子处理时输入、输出参数大小不正确 更多...

### 详细描述

范围为0x1000000B-0x10000010

### 宏定义说明

### ￿ MV_ALG_E_CFG_TYPE

#define MV_ALG_E_CFG_TYPE 0x1000000B

设置或者获取参数类型不正确

### ￿ MV_ALG_E_CFG_SIZE

#define MV_ALG_E_CFG_SIZE 0x1000000C

设置或者获取参数的输入、输出结构体大小不正确

### ￿ MV_ALG_E_PRC_TYPE

#define MV_ALG_E_PRC_TYPE 0x1000000D

处理类型不正确

### ￿ MV_ALG_E_PRC_SIZE

#define MV_ALG_E_PRC_SIZE 0x1000000E

处理时输入、输出参数大小不正确

### ￿ MV_ALG_E_FUNC_TYPE

#define MV_ALG_E_FUNC_TYPE 0x1000000F

子处理类型不正确

### ￿ MV_ALG_E_FUNC_SIZE

#define MV_ALG_E_FUNC_SIZE 0x10000010

子处理时输入、输出参数大小不正确

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

宏定义

### 能运行参数检查错误码

错误码 » 算法库透传状态码

范围为0x10000011-0x10000013 更多...

### 宏定义

#define MV_ALG_E_PARAM_INDEX 0x10000011 index参数不正确 更多...

#define MV_ALG_E_PARAM_VALUE 0x10000012 value参数不正确或者超出范围 更多...

#define MV_ALG_E_PARAM_NUM 0x10000013 param_num参数不正确 更多...

### 详细描述

范围为0x10000011-0x10000013

### 宏定义说明

### ￿ MV_ALG_E_PARAM_INDEX

#define MV_ALG_E_PARAM_INDEX 0x10000011

index参数不正确

### ￿ MV_ALG_E_PARAM_VALUE

#define MV_ALG_E_PARAM_VALUE 0x10000012

value参数不正确或者超出范围

### ￿ MV_ALG_E_PARAM_NUM

#define MV_ALG_E_PARAM_NUM 0x10000013

param_num参数不正确

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

宏定义

### 接口调用检查错误码

错误码 » 算法库透传状态码

范围为0x10000014-0x10000016 更多...

### 宏定义

#define MV_ALG_E_NULL_PTR 0x10000014 函数参数指针为空 更多...

#define MV_ALG_E_OVER_MAX_MEM 0x10000015 超过限定的最大内存 更多...

#define MV_ALG_E_CALL_BACK 0x10000016 回调函数出错 更多...

### 详细描述

范围为0x10000014-0x10000016

### 宏定义说明

### ￿ MV_ALG_E_NULL_PTR

#define MV_ALG_E_NULL_PTR 0x10000014

函数参数指针为空

### ￿ MV_ALG_E_OVER_MAX_MEM

#define MV_ALG_E_OVER_MAX_MEM 0x10000015

超过限定的最大内存

### ￿ MV_ALG_E_CALL_BACK

#define MV_ALG_E_CALL_BACK 0x10000016

回调函数出错

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

宏定义

### 算法库加密相关检查错误码

错误码 » 算法库透传状态码

范围为0x10000017-0x10000018 更多...

### 宏定义

#define MV_ALG_E_ENCRYPT 0x10000017 加密错误 更多...

#define MV_ALG_E_EXPIRE 0x10000018 算法库使用期限错误 更多...

### 详细描述

范围为0x10000017-0x10000018

### 宏定义说明

### ￿ MV_ALG_E_ENCRYPT

#define MV_ALG_E_ENCRYPT 0x10000017

加密错误

### ￿ MV_ALG_E_EXPIRE

#define MV_ALG_E_EXPIRE 0x10000018

算法库使用期限错误

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

宏定义

### 基本错误类型错误码

错误码 » 算法库透传状态码

范围为0x10000019-0x1000001B 更多...

### 宏定义

#define MV_ALG_E_BAD_ARG 0x10000019 参数范围不正确 更多...

#define MV_ALG_E_DATA_SIZE 0x1000001A 数据大小不正确 更多...

#define MV_ALG_E_STEP 0x1000001B 数据step不正确 更多...

### 详细描述

范围为0x10000019-0x1000001B

### 宏定义说明

### ￿ MV_ALG_E_BAD_ARG

#define MV_ALG_E_BAD_ARG 0x10000019

参数范围不正确

### ￿ MV_ALG_E_DATA_SIZE

#define MV_ALG_E_DATA_SIZE 0x1000001A

数据大小不正确

### ￿ MV_ALG_E_STEP

#define MV_ALG_E_STEP 0x1000001B

数据step不正确

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

宏定义

### 降噪特有错误码

错误码 » 算法库透传状态码

范围为0x10402001-0x1040200f 更多...

### 宏定义

#define MV_ALG_E_DENOISE_NE_IMG_FORMAT 0x10402001 噪声特性图像格式错误 更多...

#define MV_ALG_E_DENOISE_NE_FEATURE_TYPE 0x10402002 噪声特性类型错误 更多...

#define MV_ALG_E_DENOISE_NE_PROFILE_NUM 0x10402003 噪声特性个数错误 更多...

#define MV_ALG_E_DENOISE_NE_GAIN_NUM 0x10402004 噪声特性增益个数错误 更多...

#define MV_ALG_E_DENOISE_NE_GAIN_VAL 0x10402005 噪声曲线增益值输入错误 更多...

#define MV_ALG_E_DENOISE_NE_BIN_NUM 0x10402006 噪声曲线柱数错误 更多...

#define MV_ALG_E_DENOISE_NE_INIT_GAIN 0x10402007 噪声估计初始化增益设置错误 更多...

#define MV_ALG_E_DENOISE_NE_NOT_INIT 0x10402008 噪声估计未初始化 更多...

#define MV_ALG_E_DENOISE_COLOR_MODE 0x10402009 颜色空间模式错误 更多...

#define MV_ALG_E_DENOISE_ROI_NUM 0x1040200a 图像ROI个数错误 更多...

#define MV_ALG_E_DENOISE_ROI_ORI_PT 0x1040200b 图像ROI原点错误 更多...

#define MV_ALG_E_DENOISE_ROI_SIZE 0x1040200c 图像ROI大小错误 更多...

#define MV_ALG_E_DENOISE_GAIN_NOT_EXIST 0x1040200d 输入的相机增益不存在（增益个数已达上限） 更多...

#define MV_ALG_E_DENOISE_GAIN_BEYOND_RANGE 0x1040200e 输入的相机增益不在范围内 更多...

#define MV_ALG_E_DENOISE_NP_BUF_SIZE 0x1040200f 输入的噪声特性内存大小错误 更多...

### 详细描述

范围为0x10402001-0x1040200f

### 宏定义说明

### ￿ MV_ALG_E_DENOISE_NE_IMG_FORMAT

#define MV_ALG_E_DENOISE_NE_IMG_FORMAT 0x10402001

噪声特性图像格式错误

### ￿ MV_ALG_E_DENOISE_NE_FEATURE_TYPE

#define MV_ALG_E_DENOISE_NE_FEATURE_TYPE 0x10402002

噪声特性类型错误

### ￿ MV_ALG_E_DENOISE_NE_PROFILE_NUM

#define MV_ALG_E_DENOISE_NE_PROFILE_NUM 0x10402003

噪声特性个数错误

### ￿ MV_ALG_E_DENOISE_NE_GAIN_NUM

#define MV_ALG_E_DENOISE_NE_GAIN_NUM 0x10402004

噪声特性增益个数错误

### ￿ MV_ALG_E_DENOISE_NE_GAIN_VAL

#define MV_ALG_E_DENOISE_NE_GAIN_VAL 0x10402005

噪声曲线增益值输入错误

### ￿ MV_ALG_E_DENOISE_NE_BIN_NUM

#define MV_ALG_E_DENOISE_NE_BIN_NUM 0x10402006

噪声曲线柱数错误

### ￿ MV_ALG_E_DENOISE_NE_INIT_GAIN

#define MV_ALG_E_DENOISE_NE_INIT_GAIN 0x10402007

噪声估计初始化增益设置错误

### ￿ MV_ALG_E_DENOISE_NE_NOT_INIT

#define MV_ALG_E_DENOISE_NE_NOT_INIT 0x10402008

噪声估计未初始化

### ￿ MV_ALG_E_DENOISE_COLOR_MODE

#define MV_ALG_E_DENOISE_COLOR_MODE 0x10402009

颜色空间模式错误

### ￿ MV_ALG_E_DENOISE_ROI_NUM

#define MV_ALG_E_DENOISE_ROI_NUM 0x1040200a

图像ROI个数错误

### ￿ MV_ALG_E_DENOISE_ROI_ORI_PT

#define MV_ALG_E_DENOISE_ROI_ORI_PT 0x1040200b

图像ROI原点错误

### ￿ MV_ALG_E_DENOISE_ROI_SIZE

#define MV_ALG_E_DENOISE_ROI_SIZE 0x1040200c

图像ROI大小错误

### ￿ MV_ALG_E_DENOISE_GAIN_NOT_EXIST

#define MV_ALG_E_DENOISE_GAIN_NOT_EXIST 0x1040200d

输入的相机增益不存在（增益个数已达上限）

### ￿ MV_ALG_E_DENOISE_GAIN_BEYOND_RANGE

#define MV_ALG_E_DENOISE_GAIN_BEYOND_RANGE 0x1040200e

输入的相机增益不在范围内

### ￿ MV_ALG_E_DENOISE_NP_BUF_SIZE

#define MV_ALG_E_DENOISE_NP_BUF_SIZE 0x1040200f

输入的噪声特性内存大小错误

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

宏定义

### 其他错误码

错误码 » 算法库透传状态码

范围为0x1000001C-0x1000002C 更多...

### 宏定义

#define MV_ALG_E_CPUID 0x1000001C cpu不支持优化代码中的指令集 更多...

#define MV_ALG_WARNING 0x1000001D 警告 更多...

#define MV_ALG_E_TIME_OUT 0x1000001E 算法库超时 更多...

#define MV_ALG_E_LIB_VERSION 0x1000001F 算法版本号出错 更多...

#define MV_ALG_E_MODEL_VERSION 0x10000020 模型版本号出错 更多...

#define MV_ALG_E_GPU_MEM_ALLOC 0x10000021 GPU内存分配错误 更多...

#define MV_ALG_E_FILE_NON_EXIST 0x10000022 文件不存在 更多...

#define MV_ALG_E_NONE_STRING 0x10000023 字符串为空 更多...

#define MV_ALG_E_IMAGE_CODEC 0x10000024 图像解码器错误 更多...

#define MV_ALG_E_FILE_OPEN 0x10000025 打开文件错误 更多...

#define MV_ALG_E_FILE_READ 0x10000026

文件读取错误 更多...

#define MV_ALG_E_FILE_WRITE 0x10000027 文件写错误 更多...

#define MV_ALG_E_FILE_READ_SIZE 0x10000028 文件读取大小错误 更多...

#define MV_ALG_E_FILE_TYPE 0x10000029 文件类型错误 更多...

#define MV_ALG_E_MODEL_TYPE 0x1000002A 模型类型错误 更多...

#define MV_ALG_E_MALLOC_MEM 0x1000002B 分配内存错误 更多...

#define MV_ALG_E_BIND_CORE_FAILED 0x1000002C 线程绑核失败 更多...

### 详细描述

范围为0x1000001C-0x1000002C

### 宏定义说明

### ￿ MV_ALG_E_CPUID

#define MV_ALG_E_CPUID 0x1000001C

cpu不支持优化代码中的指令集

### ￿ MV_ALG_WARNING

#define MV_ALG_WARNING 0x1000001D

警告

### ￿ MV_ALG_E_TIME_OUT

#define MV_ALG_E_TIME_OUT 0x1000001E

算法库超时

### ￿ MV_ALG_E_LIB_VERSION

#define MV_ALG_E_LIB_VERSION 0x1000001F

算法版本号出错

### ￿ MV_ALG_E_MODEL_VERSION

#define MV_ALG_E_MODEL_VERSION 0x10000020

模型版本号出错

### ￿ MV_ALG_E_GPU_MEM_ALLOC

#define MV_ALG_E_GPU_MEM_ALLOC 0x10000021

GPU内存分配错误

### ￿ MV_ALG_E_FILE_NON_EXIST

#define MV_ALG_E_FILE_NON_EXIST 0x10000022

文件不存在

### ￿ MV_ALG_E_NONE_STRING

#define MV_ALG_E_NONE_STRING 0x10000023

字符串为空

### ￿ MV_ALG_E_IMAGE_CODEC

#define MV_ALG_E_IMAGE_CODEC 0x10000024

图像解码器错误

### ￿ MV_ALG_E_FILE_OPEN

#define MV_ALG_E_FILE_OPEN 0x10000025

打开文件错误

### ￿ MV_ALG_E_FILE_READ

#define MV_ALG_E_FILE_READ 0x10000026

文件读取错误

### ￿ MV_ALG_E_FILE_WRITE

#define MV_ALG_E_FILE_WRITE 0x10000027

文件写错误

### ￿ MV_ALG_E_FILE_READ_SIZE

#define MV_ALG_E_FILE_READ_SIZE 0x10000028

文件读取大小错误

### ￿ MV_ALG_E_FILE_TYPE

#define MV_ALG_E_FILE_TYPE 0x10000029

文件类型错误

### ￿ MV_ALG_E_MODEL_TYPE

#define MV_ALG_E_MODEL_TYPE 0x1000002A

模型类型错误

### ￿ MV_ALG_E_MALLOC_MEM

#define MV_ALG_E_MALLOC_MEM 0x1000002B

分配内存错误

### ￿ MV_ALG_E_BIND_CORE_FAILED

#define MV_ALG_E_BIND_CORE_FAILED 0x1000002C

线程绑核失败

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

### 结构体定义

这里列出了所有结构体，并附带简要说明:

CMV_ACTION_CMD_INFO 动作命令信息 CMV_ACTION_CMD_RESULT 动作命令返回信息 CMV_ACTION_CMD_RESULT_LIST 动作命令返回信息列表 CMV_ALL_MATCH_INFO 全匹配的一种信息结构体 CMV_CamL_DEV_INFO CameraLink设备信息 CMV_CAML_SERIAL_PORT 串口信息 CMV_CAML_SERIAL_PORT_LIST 串口信息列表 CMV_CC_CCM_PARAM CCM参数 CMV_CC_CCM_PARAM_EX CCM参数（适用于例如线阵相机等宽高大 于65535的设备） CMV_CC_CONTRAST_PARAM 对比度调节结构体 CMV_CC_DEVICE_INFO 设备信息 CMV_CC_DEVICE_INFO_LIST 设备信息列表 CMV_CC_FILE_ACCESS 文件存取 CMV_CC_FILE_ACCESS_EX 文件存取 CMV_CC_FILE_ACCESS_PROGRESS 文件存取进度 CMV_CC_FLIP_IMAGE_PARAM 图像翻转结构体 CMV_CC_FRAME_SPEC_INFO 水印信息 CMV_CC_GAMMA_PARAM Gamma信息结构体 CMV_CC_HB_DECODE_PARAM 无损解码参数 CMV_CC_IMAGE 图像信息 CMV_CC_INPUT_FRAME_INFO 传入的图像数据 CMV_CC_LSC_CALIB_PARAM LSC标定结构体 CMV_CC_LSC_CORRECT_PARAM LSC校正结构体 CMV_CC_NOISE_ESTIMATE_PARAM 噪声估计结构体 CMV_CC_PIXEL_CONVERT_PARAM_EX 像素转换结构体 CMV_CC_RECORD_PARAM 录像参数 CMV_CC_RECT_I 矩形ROI结构体 CMV_CC_ROTATE_IMAGE_PARAM 图像旋转结构体 CMV_CC_SAVE_IMAGE_PARAM 保存图片所需参数 CMV_CC_SHARPEN_PARAM 锐化结构体 CMV_CHUNK_DATA_CONTENT Chunk内容 CMV_CML_DEVICE_INFO Camera Link相机信息 CMV_CXP_DEVICE_INFO CoaXPress相机信息 CMV_DISPLAY_FRAME_INFO 显示帧信息（自4.4.1版本废弃）

CMV_DISPLAY_FRAME_INFO_EX 显示帧信息（适用于例如线阵相机等宽高大 于65535的设备） CMV_EVENT_OUT_INFO Event事件回调信息 CMV_FRAME_OUT 图像结构体，输出图像地址及图像信息 CMV_FRAME_OUT_INFO_EX 输出帧的信息 CMV_GENTL_DEV_INFO 通过GenTL枚举到的设备信息 CMV_GENTL_DEV_INFO_LIST 通过GenTL枚举到的设备信息列表 CMV_GENTL_IF_INFO 通过GenTL枚举到的接口信息 CMV_GENTL_IF_INFO_LIST 通过GenTL枚举到的接口信息列表 CMV_GIGE_DEVICE_INFO GigE设备信息 CMV_INTERFACE_INFO 采集卡信息 CMV_INTERFACE_INFO_LIST 采集卡信息列表 CMV_MATCH_INFO_NET_DETECT 网络流量和丢包信息反馈结构体，对应类型 为 MV_MATCH_TYPE_NET_DETECT CMV_MATCH_INFO_USB_DETECT Host收到从u3v设备端的总字节数，对应类 型为 MV_MATCH_TYPE_USB_DETECT CMV_NETTRANS_INFO 网络传输的相关信息 CMV_OUTPUT_IMAGE_INFO 图像重构后的图像列表 CMV_RECONSTRUCT_IMAGE_PARAM 重构图像参数信息 CMV_SAVE_IMAGE_PARAM_EX3 图片保存参数 CMV_SAVE_IMAGE_TO_FILE_PARAM_EX 保存图片到文件参数 CMV_SAVE_POINT_CLOUD_PARAM 保存3D数据到缓存（自4.4.1版本废弃） CMV_TRANSMISSION_TYPE 网络传输模式 CMV_USB3_DEVICE_INFO USB设备信息 CMV_XOF_DEVICE_INFO XoFLink相机信息 CMVCC_CIRCLE_INFO 圆形框区域信息 CMVCC_COLORF 辅助线颜色 CMVCC_ENUMENTRY 枚举类型条目 CMVCC_ENUMVALUE 枚举类型值 CMVCC_ENUMVALUE_EX 枚举类型值 CMVCC_FLOATVALUE Float类型值 CMVCC_INTVALUE Int类型值 CMVCC_INTVALUE_EX Int类型值Ex CMVCC_LINES_INFO 线条辅助线信息 CMVCC_NODE_ERROR 节点错误信息 CMVCC_NODE_ERROR_LIST 节点错误信息列表 CMVCC_POINTF 自定义点 CMVCC_RECT_INFO 矩形框区域信息 CMVCC_STRINGVALUE String类型值

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_ACTION_CMD_INFO结构体 参考

动作命令信息 更多...

### 成员变量

unsigned int nDeviceKey [IN] 设备密钥 更多...

unsigned int nGroupKey [IN] 组键 更多...

unsigned int nGroupMask [IN] 组掩码 更多...

unsigned int bActionTimeEnable [IN] 只有设置成1时Action Time才有效，非1时无效 更多...

int64_t nActionTime

[IN] 预定的时间，和主频有关 更多...

const char * pBroadcastAddress [IN] 广播包地址 更多...

unsigned int nTimeOut [IN] 等待ACK的超时时间，如果为0表示不需要ACK 更多...

unsigned int bSpecialNetEnable [IN] 只有设置成1时指定的网卡IP才有效，非1时无效 更多...

unsigned int nSpecialNetIP [IN] 指定的网卡IP 更多...

unsigned int nReserved [14] 预留 更多...

### 详细描述

动作命令信息

示例 DynamicallyLoadDLL.cpp , 以及 Grab_ActionCommand.cpp.

### 结构体成员变量说明

### ￿ nDeviceKey

unsigned int nDeviceKey

[IN] 设备密钥

示例 Grab_ActionCommand.cpp.

### ￿ nGroupKey

unsigned int nGroupKey

[IN] 组键

示例 Grab_ActionCommand.cpp.

### ￿ nGroupMask

unsigned int nGroupMask

[IN] 组掩码

示例 Grab_ActionCommand.cpp.

### ￿ bActionTimeEnable

unsigned int bActionTimeEnable

[IN] 只有设置成1时Action Time才有效，非1时无效

示例 Grab_ActionCommand.cpp.

### ￿ nActionTime

int64_t nActionTime

[IN] 预定的时间，和主频有关

### ￿ pBroadcastAddress

const char* pBroadcastAddress

[IN] 广播包地址

示例 Grab_ActionCommand.cpp.

### ￿ nTimeOut

unsigned int nTimeOut

[IN] 等待ACK的超时时间，如果为0表示不需要ACK

示例 Grab_ActionCommand.cpp.

### ￿ bSpecialNetEnable

unsigned int bSpecialNetEnable

[IN] 只有设置成1时指定的网卡IP才有效，非1时无效

### ￿ nSpecialNetIP

unsigned int nSpecialNetIP

[IN] 指定的网卡IP

### ￿ nReserved

unsigned int nReserved[14]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_ACTION_CMD_RESULT结构体 参考

动作命令返回信息 更多...

### 成员变量

unsigned char strDeviceAddress [12+3+1] [OUT] 设备IP 更多...

int nStatus [OUT] 状态码 更多...

unsigned int nReserved [4] 预留 更多...

### 详细描述

动作命令返回信息

示例 Grab_ActionCommand.cpp.

### 结构体成员变量说明

### ￿ strDeviceAddress

unsigned char strDeviceAddress[12+3+1]

[OUT] 设备IP

示例 Grab_ActionCommand.cpp.

### ￿ nStatus

int nStatus

[OUT] 状态码

示例 Grab_ActionCommand.cpp.

### ￿ nReserved

unsigned int nReserved[4]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_ACTION_CMD_RESULT_LIST结构体 参考

动作命令返回信息列表 更多...

### 成员变量

unsigned int nNumResults [OUT] 返回值个数 更多...

MV_ACTION_CMD_RESULT * pResults [OUT] 动作命令结果 更多...

### 详细描述

动作命令返回信息列表

示例 DynamicallyLoadDLL.cpp , 以及 Grab_ActionCommand.cpp.

### 结构体成员变量说明

### ￿ nNumResults

unsigned int nNumResults

[OUT] 返回值个数

示例 Grab_ActionCommand.cpp.

### ￿ pResults

MV_ACTION_CMD_RESULT* pResults

[OUT] 动作命令结果

示例 Grab_ActionCommand.cpp.

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_ALL_MATCH_INFO结构体 参考

全匹配的一种信息结构体 更多...

### 成员变量

unsigned int nType [IN] 需要输出的信息类型，e.g. MV_MATCH_TYPE_NET_DETECT 更多...

void * pInfo [OUT] 输出的信息缓存，由调用者分配 更多...

unsigned int nInfoSize [IN] 信息缓存的大小 更多...

### 详细描述

全匹配的一种信息结构体

示例 DynamicallyLoadDLL.cpp.

### 结构体成员变量说明

### ￿ nType

unsigned int nType

[IN] 需要输出的信息类型，e.g. MV_MATCH_TYPE_NET_DETECT

### ￿ pInfo

void* pInfo

[OUT] 输出的信息缓存，由调用者分配

### ￿ nInfoSize

unsigned int nInfoSize

[IN] 信息缓存的大小

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_CamL_DEV_INFO结构体 参考

CameraLink设备信息 更多...

### 成员变量

unsigned char chPortID [INFO_MAX_BUFFER_SIZE] [OUT] 端口号 更多...

unsigned char chModelName [INFO_MAX_BUFFER_SIZE] [OUT] 型号名字 更多...

unsigned char chFamilyName [INFO_MAX_BUFFER_SIZE] [OUT] 名称 更多...

unsigned char chDeviceVersion [INFO_MAX_BUFFER_SIZE] [OUT] 设备版本 更多...

unsigned char chManufacturerName [INFO_MAX_BUFFER_SIZE] [OUT] 制造商名字 更多...

unsigned char chSerialNumber [INFO_MAX_BUFFER_SIZE] [OUT] 序列号 更多...

unsigned int nReserved [38] 预留 更多...

### 详细描述

CameraLink设备信息

### 结构体成员变量说明

### ￿ chPortID

unsigned char chPortID[INFO_MAX_BUFFER_SIZE]

[OUT] 端口号

### ￿ chModelName

unsigned char chModelName[INFO_MAX_BUFFER_SIZE]

[OUT] 型号名字

### ￿ chFamilyName

unsigned char chFamilyName[INFO_MAX_BUFFER_SIZE]

[OUT] 名称

### ￿ chDeviceVersion

unsigned char chDeviceVersion[INFO_MAX_BUFFER_SIZE]

[OUT] 设备版本

### ￿ chManufacturerName

unsigned char chManufacturerName[INFO_MAX_BUFFER_SIZE]

[OUT] 制造商名字

### ￿ chSerialNumber

unsigned char chSerialNumber[INFO_MAX_BUFFER_SIZE]

[OUT] 序列号

### ￿ nReserved

unsigned int nReserved[38]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_CAML_SERIAL_PORT结构体 参考

串口信息 更多...

### 成员变量

unsigned char chSerialPort [INFO_MAX_BUFFER_SIZE] [OUT] 串口号 更多...

unsigned int nRes [4] 预留 更多...

### 详细描述

串口信息

### 结构体成员变量说明

### ￿ chSerialPort

unsigned char chSerialPort[INFO_MAX_BUFFER_SIZE]

[OUT] 串口号

### ￿ nRes

unsigned int nRes[4]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_CAML_SERIAL_PORT_LIST结构体 参考

串口信息列表 更多...

### 成员变量

unsigned int nSerialPortNum [OUT] 串口数量 更多...

MV_CAML_SERIAL_PORT stSerialPort [MV_MAX_SERIAL_PORT_NUM] [IN][OUT] 串口信息 更多...

unsigned int nRes [4] 预留 更多...

### 详细描述

串口信息列表

### 结构体成员变量说明

### ￿ nSerialPortNum

unsigned int nSerialPortNum

[OUT] 串口数量

### ￿ stSerialPort

MV_CAML_SERIAL_PORT stSerialPort[MV_MAX_SERIAL_PORT_NUM]

[IN][OUT] 串口信息

### ￿ nRes

unsigned int nRes[4]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_CC_CCM_PARAM结构体 参考

CCM参数 更多...

### 成员变量

bool bCCMEnable [IN] 是否启用CCM 更多...

int nCCMat [9] [IN] CCM矩阵[-8192~8192] 更多...

unsigned int nRes [8] 预留 更多...

### 详细描述

CCM参数

### 结构体成员变量说明

### ￿ bCCMEnable

bool bCCMEnable

[IN] 是否启用CCM

### ￿ nCCMat

int nCCMat[9]

[IN] CCM矩阵[-8192~8192]

### ￿ nRes

unsigned int nRes[8]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_CC_CCM_PARAM_EX结构体 参考

CCM参数（适用于例如线阵相机等宽高大于65535的设备） 更多...

### 成员变量

bool bCCMEnable [IN] 是否启用CCM 更多...

int nCCMat [9] [IN] CCM矩阵[-65536~65536] 更多...

unsigned int nCCMScale [IN] 量化系数（2的整数幂，最大65536） 更多...

unsigned int nRes [8] 预留 更多...

### 详细描述

CCM参数（适用于例如线阵相机等宽高大于65535的设备）

### 结构体成员变量说明

### ￿ bCCMEnable

bool bCCMEnable

[IN] 是否启用CCM

### ￿ nCCMat

int nCCMat[9]

[IN] CCM矩阵[-65536~65536]

### ￿ nCCMScale

unsigned int nCCMScale

[IN] 量化系数（2的整数幂，最大65536）

### ￿ nRes

unsigned int nRes[8]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_CC_CONTRAST_PARAM结构体 参考

对比度调节结构体 更多...

### 成员变量

unsigned int nWidth

[IN] 图像宽度（最小8） 更多...

unsigned int nHeight [IN] 图像高度（最小8） 更多...

unsigned char * pSrcBuf [IN] 输入数据缓存 更多...

unsigned int nSrcBufLen [IN] 输入数据大小 更多...

enum MvGvspPixelType enPixelType [IN] 像素格式 更多...

unsigned char * pDstBuf [OUT] 输出数据缓存 更多...

unsigned int nDstBufSize [IN] 提供的输出缓冲区大小 更多...

unsigned int nDstBufLen [OUT] 输出数据长度 更多...

unsigned int nContrastFactor [IN] 对比度值，[1, 10000] 更多...

unsigned int nRes [8] 预留 更多...

### 详细描述

对比度调节结构体

示例 ImageContrast.cpp.

### 结构体成员变量说明

### ￿ nWidth

unsigned int nWidth

[IN] 图像宽度（最小8）

示例 ImageContrast.cpp.

### ￿ nHeight

unsigned int nHeight

[IN] 图像高度（最小8）

示例 ImageContrast.cpp.

### ￿ pSrcBuf

unsigned char* pSrcBuf

[IN] 输入数据缓存

示例 ImageContrast.cpp.

### ￿ nSrcBufLen

unsigned int nSrcBufLen

[IN] 输入数据大小

示例 ImageContrast.cpp.

### ￿ enPixelType

enum MvGvspPixelType enPixelType

[IN] 像素格式

示例 ImageContrast.cpp.

### ￿ pDstBuf

unsigned char* pDstBuf

[OUT] 输出数据缓存

示例 ImageContrast.cpp.

### ￿ nDstBufSize

unsigned int nDstBufSize

[IN] 提供的输出缓冲区大小

示例 ImageContrast.cpp.

### ￿ nDstBufLen

unsigned int nDstBufLen

[OUT] 输出数据长度

### ￿ nContrastFactor

unsigned int nContrastFactor

[IN] 对比度值，[1, 10000]

示例 ImageContrast.cpp.

### ￿ nRes

unsigned int nRes[8]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_CC_DEVICE_INFO结构体 参考

设备信息 更多...

### 成员变量

unsigned short nMajorVer [OUT] 主要版本 更多...

unsigned short nMinorVer [OUT] 次要版本 更多...

unsigned int nMacAddrHigh [OUT] 高MAC地址 更多...

unsigned int nMacAddrLow [OUT] 低MAC地址 更多...

unsigned int nTLayerType [OUT] 设备传输层协议类型 更多...

unsigned int nDevTypeInfo [OUT] 设备类型信息 设备类型规则： 7 - 0 bit: 预留 15 - 8 bit：产品子类别 23 - 16 bit：产品类型 31 - 24bit：产品线（如: 0x01 标准产品；0x02 3D 产品；0x03 智能ID产品） 更多...

unsigned int nReserved [3] 预留 更多...

union { MV_GIGE_DEVICE_INFO stGigEInfo [OUT] GigE设备信息 更多...

MV_USB3_DEVICE_INFO stUsb3VInfo [OUT] USB设备信息 更多...

MV_CamL_DEV_INFO stCamLInfo [OUT] CameraLink设备信息 更多...

MV_CML_DEVICE_INFO stCMLInfo [OUT] 采集卡CameraLink设备信息 更多...

MV_CXP_DEVICE_INFO stCXPInfo [OUT] 采集卡CoaXPress设备信息 更多...

MV_XOF_DEVICE_INFO stXoFInfo [OUT] 采集卡XoF设备信息 更多...

} SpecialInfo

### 详细描述

设备信息

示例 ChunkData.cpp, ConnectSpecCamera.cpp, ConvertPixelType.cpp, DynamicallyLoadDLL.cpp, Events.cpp, Events_Interface.cpp, ForceIPEx.cpp, Grab_ActionCommand.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, InterfaceAndDeviceDemo.cpp, LensShadingCorrection.cpp, MultiCast.cpp, MultiLightCtrl_ImageStitching.cpp, ParametrizeCamera_FileAccess.cpp, ParametrizeCamera_LineScanIOSettings.cpp, ParametrizeCamera_LoadAndSave.cpp, QuickSoftwareTrigger.cpp , 以及 Recording.cpp.

### 结构体成员变量说明

### ￿ nMajorVer

unsigned short nMajorVer

[OUT] 主要版本

### ￿ nMinorVer

unsigned short nMinorVer

[OUT] 次要版本

### ￿ nMacAddrHigh

unsigned int nMacAddrHigh

[OUT] 高MAC地址

### ￿ nMacAddrLow

unsigned int nMacAddrLow

[OUT] 低MAC地址

### ￿ nTLayerType

unsigned int nTLayerType

[OUT] 设备传输层协议类型

示例 ChunkData.cpp, ConnectSpecCamera.cpp, ConvertPixelType.cpp, DynamicallyLoadDLL.cpp, Events.cpp, Events_Interface.cpp, ForceIPEx.cpp, Grab_ActionCommand.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, InterfaceAndDeviceDemo.cpp, LensShadingCorrection.cpp, MultiCast.cpp, MultiLightCtrl_ImageStitching.cpp, ParametrizeCamera_FileAccess.cpp, ParametrizeCamera_LineScanIOSettings.cpp, ParametrizeCamera_LoadAndSave.cpp, QuickSoftwareTrigger.cpp , 以及 Recording.cpp.

### ￿ nDevTypeInfo

unsigned int nDevTypeInfo

[OUT] 设备类型信息 设备类型规则： 7 - 0 bit: 预留 15 - 8 bit：产品子类别

23 - 16 bit：产品类型 31 - 24bit：产品线（如: 0x01 标准产品；0x02 3D产品；0x03 智能ID产品）

### ￿ nReserved

unsigned int nReserved[3]

预留

### ￿ stGigEInfo

MV_GIGE_DEVICE_INFO stGigEInfo

[OUT] GigE设备信息

示例 ChunkData.cpp, ConnectSpecCamera.cpp, ConvertPixelType.cpp, DynamicallyLoadDLL.cpp, Events.cpp, Events_Interface.cpp, ForceIPEx.cpp, Grab_ActionCommand.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, InterfaceAndDeviceDemo.cpp, LensShadingCorrection.cpp, MultiCast.cpp, MultiLightCtrl_ImageStitching.cpp, ParametrizeCamera_FileAccess.cpp, ParametrizeCamera_LineScanIOSettings.cpp, ParametrizeCamera_LoadAndSave.cpp , 以及 Recording.cpp.

### ￿ stUsb3VInfo

MV_USB3_DEVICE_INFO stUsb3VInfo

[OUT] USB设备信息

示例 ChunkData.cpp, ConvertPixelType.cpp, DynamicallyLoadDLL.cpp, Events.cpp, Events_Interface.cpp, ForceIPEx.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, InterfaceAndDeviceDemo.cpp, LensShadingCorrection.cpp, MultiLightCtrl_ImageStitching.cpp, ParametrizeCamera_FileAccess.cpp,

ParametrizeCamera_LineScanIOSettings.cpp, ParametrizeCamera_LoadAndSave.cpp , 以及 Recording.cpp.

### ￿ stCamLInfo

MV_CamL_DEV_INFO stCamLInfo

[OUT] CameraLink设备信息

### ￿ stCMLInfo

MV_CML_DEVICE_INFO stCMLInfo

[OUT] 采集卡CameraLink设备信息

示例 ChunkData.cpp, ConvertPixelType.cpp, DynamicallyLoadDLL.cpp, Events.cpp, Events_Interface.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, InterfaceAndDeviceDemo.cpp, LensShadingCorrection.cpp, MultiLightCtrl_ImageStitching.cpp, ParametrizeCamera_LineScanIOSettings.cpp , 以及 Recording.cpp.

### ￿ stCXPInfo

MV_CXP_DEVICE_INFO stCXPInfo

[OUT] 采集卡CoaXPress设备信息

示例 ChunkData.cpp, ConvertPixelType.cpp, DynamicallyLoadDLL.cpp, Events.cpp, Events_Interface.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, InterfaceAndDeviceDemo.cpp, LensShadingCorrection.cpp, MultiLightCtrl_ImageStitching.cpp, ParametrizeCamera_LineScanIOSettings.cpp, QuickSoftwareTrigger.cpp , 以及 Recording.cpp.

### ￿ stXoFInfo

MV_XOF_DEVICE_INFO stXoFInfo

[OUT] 采集卡XoF设备信息

示例 ChunkData.cpp, ConvertPixelType.cpp, DynamicallyLoadDLL.cpp, Events.cpp, Events_Interface.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, InterfaceAndDeviceDemo.cpp, LensShadingCorrection.cpp, MultiLightCtrl_ImageStitching.cpp, ParametrizeCamera_LineScanIOSettings.cpp, QuickSoftwareTrigger.cpp , 以及 Recording.cpp.

### ￿ SpecialInfo

union { ... } SpecialInfo

示例

ChunkData.cpp, ConnectSpecCamera.cpp, ConvertPixelType.cpp, DynamicallyLoadDLL.cpp, Events.cpp, Events_Interface.cpp, ForceIPEx.cpp, Grab_ActionCommand.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, InterfaceAndDeviceDemo.cpp, LensShadingCorrection.cpp, MultiCast.cpp, MultiLightCtrl_ImageStitching.cpp, ParametrizeCamera_FileAccess.cpp, ParametrizeCamera_LineScanIOSettings.cpp, ParametrizeCamera_LoadAndSave.cpp, QuickSoftwareTrigger.cpp , 以及 Recording.cpp.

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_CC_DEVICE_INFO_LIST结构体 参考

设备信息列表 更多...

### 成员变量

unsigned int nDeviceNum [OUT] 在线设备数量 更多...

MV_CC_DEVICE_INFO * pDeviceInfo [MV_MAX_DEVICE_NUM] [OUT] 支持最多256个设备 更多...

### 详细描述

设备信息列表

示例 ChunkData.cpp, ConvertPixelType.cpp, DynamicallyLoadDLL.cpp, Events.cpp, Events_Interface.cpp, ForceIPEx.cpp, Grab_ActionCommand.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, InterfaceAndDeviceDemo.cpp, LensShadingCorrection.cpp, MultiCast.cpp, MultiLightCtrl_ImageStitching.cpp,

ParametrizeCamera_FileAccess.cpp, ParametrizeCamera_LineScanIOSettings.cpp, ParametrizeCamera_LoadAndSave.cpp, QuickSoftwareTrigger.cpp , 以及 Recording.cpp.

### 结构体成员变量说明

### ￿ nDeviceNum

unsigned int nDeviceNum

[OUT] 在线设备数量

示例 ChunkData.cpp, ConvertPixelType.cpp, DynamicallyLoadDLL.cpp, Events.cpp, Events_Interface.cpp, ForceIPEx.cpp, Grab_ActionCommand.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, InterfaceAndDeviceDemo.cpp, LensShadingCorrection.cpp, MultiCast.cpp, MultiLightCtrl_ImageStitching.cpp, ParametrizeCamera_FileAccess.cpp, ParametrizeCamera_LineScanIOSettings.cpp, ParametrizeCamera_LoadAndSave.cpp, QuickSoftwareTrigger.cpp , 以及 Recording.cpp.

### ￿ pDeviceInfo

MV_CC_DEVICE_INFO* pDeviceInfo[MV_MAX_DEVICE_NUM]

[OUT] 支持最多256个设备

示例 ChunkData.cpp, ConvertPixelType.cpp, DynamicallyLoadDLL.cpp, Events.cpp, Events_Interface.cpp, ForceIPEx.cpp, Grab_ActionCommand.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, InterfaceAndDeviceDemo.cpp, LensShadingCorrection.cpp, MultiCast.cpp, MultiLightCtrl_ImageStitching.cpp, ParametrizeCamera_FileAccess.cpp,

ParametrizeCamera_LineScanIOSettings.cpp, ParametrizeCamera_LoadAndSave.cpp, QuickSoftwareTrigger.cpp , 以及 Recording.cpp.

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_CC_FILE_ACCESS结构体 参考

文件存取 更多...

### 成员变量

const char * pUserFileName [IN] 用户文件名 更多...

const char * pDevFileName [IN] 设备文件名 更多...

unsigned int nReserved [32] 预留 更多...

### 详细描述

文件存取

示例 DynamicallyLoadDLL.cpp , 以及 ParametrizeCamera_FileAccess.cpp.

### 结构体成员变量说明

### ￿ pUserFileName

const char* pUserFileName

[IN] 用户文件名

示例 ParametrizeCamera_FileAccess.cpp.

### ￿ pDevFileName

const char* pDevFileName

[IN] 设备文件名

示例 ParametrizeCamera_FileAccess.cpp.

### ￿ nReserved

unsigned int nReserved[32]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_CC_FILE_ACCESS_EX结构体 参考

文件存取 更多...

### 成员变量

char * pUserFileBuf [IN] 用户数据缓存 更多...

unsigned int pFileBufSize [IN] 用户数据缓存大小 更多...

unsigned int pFileBufLen [OUT][IN] 文件数据缓存总长度(读取时输出文件数据总长度，写入时输入文件数据 总长度) 更多...

const char * pDevFileName [IN] 设备文件名 更多...

unsigned int nReserved [32] 预留 更多...

### 详细描述

文件存取

### 结构体成员变量说明

### ￿ pUserFileBuf

char* pUserFileBuf

[IN] 用户数据缓存

### ￿ pFileBufSize

unsigned int pFileBufSize

[IN] 用户数据缓存大小

### ￿ pFileBufLen

unsigned int pFileBufLen

[OUT][IN] 文件数据缓存总长度(读取时输出文件数据总长度，写入时输入文件数据总长度)

### ￿ pDevFileName

const char* pDevFileName

[IN] 设备文件名

### ￿ nReserved

unsigned int nReserved[32]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_CC_FILE_ACCESS_PROGRESS结构体 参考

文件存取进度 更多...

### 成员变量

int64_t nCompleted [OUT] 已完成的长度 更多...

int64_t nTotal [OUT] 总长度 更多...

unsigned int nReserved [8] 预留 更多...

### 详细描述

文件存取进度

示例 DynamicallyLoadDLL.cpp , 以及 ParametrizeCamera_FileAccess.cpp.

### 结构体成员变量说明

### ￿ nCompleted

int64_t nCompleted

[OUT] 已完成的长度

示例 ParametrizeCamera_FileAccess.cpp.

### ￿ nTotal

int64_t nTotal

[OUT] 总长度

示例 ParametrizeCamera_FileAccess.cpp.

### ￿ nReserved

unsigned int nReserved[8]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_CC_FLIP_IMAGE_PARAM结构体 参考

图像翻转结构体 更多...

### 成员变量

enum MvGvspPixelType enPixelType [IN] 像素格式 更多...

unsigned int nWidth [IN] 图像宽 更多...

unsigned int nHeight [IN] 图像高 更多...

unsigned char * pSrcData [IN] 输入数据缓存 更多...

unsigned int nSrcDataLen [IN] 输入数据长度 更多...

unsigned char * pDstBuf [OUT] 输出数据缓存 更多...

unsigned int nDstBufLen [OUT] 输出数据长度 更多...

unsigned int nDstBufSize [IN] 提供的输出缓冲区大小 更多...

MV_IMG_FLIP_TYPE enFlipType [IN] 翻转类型 更多...

unsigned int nRes [8] 预留 更多...

### 详细描述

图像翻转结构体

### 结构体成员变量说明

### ￿ enPixelType

enum MvGvspPixelType enPixelType

[IN] 像素格式

### ￿ nWidth

unsigned int nWidth

[IN] 图像宽

### ￿ nHeight

unsigned int nHeight

[IN] 图像高

### ￿ pSrcData

unsigned char* pSrcData

[IN] 输入数据缓存

### ￿ nSrcDataLen

unsigned int nSrcDataLen

[IN] 输入数据长度

### ￿ pDstBuf

unsigned char* pDstBuf

[OUT] 输出数据缓存

### ￿ nDstBufLen

unsigned int nDstBufLen

[OUT] 输出数据长度

### ￿ nDstBufSize

unsigned int nDstBufSize

[IN] 提供的输出缓冲区大小

### ￿ enFlipType

MV_IMG_FLIP_TYPE enFlipType

[IN] 翻转类型

### ￿ nRes

unsigned int nRes[8]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_CC_FRAME_SPEC_INFO结构体 参考

水印信息 更多...

### 成员变量

unsigned int nSecondCount [OUT] 秒数 更多...

unsigned int nCycleCount [OUT] 周期数 更多...

unsigned int nCycleOffset [OUT] 周期偏移量 更多...

float fGain [OUT] 增益 更多...

float fExposureTime [OUT] 曝光时间 更多...

unsigned int nAverageBrightness [OUT] 平均亮度 更多...

unsigned int nRed [OUT] 红色 更多...

unsigned int nGreen [OUT] 绿色 更多...

unsigned int nBlue [OUT] 蓝色 更多...

unsigned int nFrameCounter [OUT] 总帧数 更多...

unsigned int nTriggerIndex [OUT] 触发计数 更多...

unsigned int nInput [OUT] 输入 更多...

unsigned int nOutput [OUT] 输出 更多...

unsigned short nOffsetX [OUT] 水平偏移量 更多...

unsigned short nOffsetY [OUT] 垂直偏移量 更多...

unsigned short nFrameWidth [OUT] 水印宽 更多...

unsigned short nFrameHeight [OUT] 水印高 更多...

unsigned int nReserved [16] 预留 更多...

### 详细描述

水印信息

### 结构体成员变量说明

### ￿ nSecondCount

unsigned int nSecondCount

[OUT] 秒数

### ￿ nCycleCount

unsigned int nCycleCount

[OUT] 周期数

### ￿ nCycleOffset

unsigned int nCycleOffset

[OUT] 周期偏移量

### ￿ fGain

float fGain

[OUT] 增益

### ￿ fExposureTime

float fExposureTime

[OUT] 曝光时间

### ￿ nAverageBrightness

unsigned int nAverageBrightness

[OUT] 平均亮度

### ￿ nRed

unsigned int nRed

[OUT] 红色

### ￿ nGreen

unsigned int nGreen

[OUT] 绿色

### ￿ nBlue

unsigned int nBlue

[OUT] 蓝色

### ￿ nFrameCounter

unsigned int nFrameCounter

[OUT] 总帧数

### ￿ nTriggerIndex

unsigned int nTriggerIndex

[OUT] 触发计数

### ￿ nInput

unsigned int nInput

[OUT] 输入

### ￿ nOutput

unsigned int nOutput

[OUT] 输出

### ￿ nOffsetX

unsigned short nOffsetX

[OUT] 水平偏移量

### ￿ nOffsetY

unsigned short nOffsetY

[OUT] 垂直偏移量

### ￿ nFrameWidth

unsigned short nFrameWidth

[OUT] 水印宽

### ￿ nFrameHeight

unsigned short nFrameHeight

[OUT] 水印高

### ￿ nReserved

unsigned int nReserved[16]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_CC_GAMMA_PARAM结构体 参考

Gamma信息结构体 更多...

### 成员变量

MV_CC_GAMMA_TYPE enGammaType [IN] Gamma类型 更多...

float fGammaValue [IN] Gamma值[0.1, 4.0] 更多...

unsigned char * pGammaCurveBuf [IN] Gamma曲线缓存 更多...

unsigned int nGammaCurveBufLen [IN] Gamma曲线长度 更多...

unsigned int nRes [8] 预留 更多...

### 详细描述

Gamma信息结构体

### 结构体成员变量说明

### ￿ enGammaType

MV_CC_GAMMA_TYPE enGammaType

[IN] Gamma类型

### ￿ fGammaValue

float fGammaValue

[IN] Gamma值[0.1, 4.0]

### ￿ pGammaCurveBuf

unsigned char* pGammaCurveBuf

[IN] Gamma曲线缓存

### ￿ nGammaCurveBufLen

unsigned int nGammaCurveBufLen

[IN] Gamma曲线长度

### ￿ nRes

unsigned int nRes[8]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_CC_HB_DECODE_PARAM结构体 参考

无损解码参数 更多...

### 成员变量

unsigned char * pSrcBuf [IN] 输入数据缓存 更多...

unsigned int nSrcLen [IN] 输入数据大小 更多...

unsigned int nWidth [OUT] 图像宽 更多...

unsigned int nHeight [OUT] 图像高 更多...

unsigned char * pDstBuf [OUT] 输出数据缓存 更多...

unsigned int nDstBufSize [IN] 提供的输出缓冲区大小 更多...

unsigned int nDstBufLen

[OUT] 输出数据大小 更多...

enum MvGvspPixelType enDstPixelType [OUT] 输出的像素格式 更多...

MV_CC_FRAME_SPEC_INFO stFrameSpecInfo [OUT] 水印信息 更多...

unsigned int nRes [8] 预留 更多...

### 详细描述

无损解码参数

示例 HighBandwidthDecode.cpp, ImageSave.cpp , 以及 MultiLightCtrl_ImageStitching.cpp.

### 结构体成员变量说明

### ￿ pSrcBuf

unsigned char* pSrcBuf

[IN] 输入数据缓存

示例 HighBandwidthDecode.cpp, ImageSave.cpp , 以及 MultiLightCtrl_ImageStitching.cpp.

### ￿ nSrcLen

unsigned int nSrcLen

[IN] 输入数据大小

示例 HighBandwidthDecode.cpp, ImageSave.cpp , 以及 MultiLightCtrl_ImageStitching.cpp.

### ￿ nWidth

unsigned int nWidth

[OUT] 图像宽

示例 HighBandwidthDecode.cpp, ImageSave.cpp , 以及 MultiLightCtrl_ImageStitching.cpp.

### ￿ nHeight

unsigned int nHeight

[OUT] 图像高

示例 HighBandwidthDecode.cpp, ImageSave.cpp , 以及 MultiLightCtrl_ImageStitching.cpp.

### ￿ pDstBuf

unsigned char* pDstBuf

[OUT] 输出数据缓存

示例 HighBandwidthDecode.cpp, ImageSave.cpp , 以及 MultiLightCtrl_ImageStitching.cpp.

### ￿ nDstBufSize

unsigned int nDstBufSize

[IN] 提供的输出缓冲区大小

示例 HighBandwidthDecode.cpp, ImageSave.cpp , 以及 MultiLightCtrl_ImageStitching.cpp.

### ￿ nDstBufLen

unsigned int nDstBufLen

[OUT] 输出数据大小

示例 HighBandwidthDecode.cpp, ImageSave.cpp , 以及 MultiLightCtrl_ImageStitching.cpp.

### ￿ enDstPixelType

enum MvGvspPixelType enDstPixelType

[OUT] 输出的像素格式

示例 HighBandwidthDecode.cpp , 以及 MultiLightCtrl_ImageStitching.cpp.

### ￿ stFrameSpecInfo

MV_CC_FRAME_SPEC_INFO stFrameSpecInfo

[OUT] 水印信息

### ￿ nRes

unsigned int nRes[8]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_CC_IMAGE结构体 参考

图像信息 更多...

### 成员变量

unsigned int nWidth 图像宽 更多...

unsigned int nHeight 图像高 更多...

enum MvGvspPixelType enPixelType 像素格式 更多...

unsigned char * pImageBuf 图像缓存 更多...

uint64_t nImageBufSize

图像缓存大小 更多...

uint64_t nImageBufLen 图像长度 更多...

unsigned int nReserved [4] 预留字段 更多...

### 详细描述

图像信息

示例 GrabImage_Display.cpp, HighBandwidthDecode.cpp, ImageSave.cpp , 以及 MultiLightCtrl_ImageStitching.cpp.

### 结构体成员变量说明

### ￿ nWidth

unsigned int nWidth

图像宽

示例 GrabImage_Display.cpp, HighBandwidthDecode.cpp, ImageSave.cpp , 以及 MultiLightCtrl_ImageStitching.cpp.

### ￿ nHeight

unsigned int nHeight

图像高

示例

GrabImage_Display.cpp, HighBandwidthDecode.cpp, ImageSave.cpp , 以及 MultiLightCtrl_ImageStitching.cpp.

### ￿ enPixelType

enum MvGvspPixelType enPixelType

像素格式

示例 GrabImage_Display.cpp, HighBandwidthDecode.cpp, ImageSave.cpp , 以及 MultiLightCtrl_ImageStitching.cpp.

### ￿ pImageBuf

unsigned char* pImageBuf

图像缓存

示例 GrabImage_Display.cpp, HighBandwidthDecode.cpp, ImageSave.cpp , 以及 MultiLightCtrl_ImageStitching.cpp.

### ￿ nImageBufSize

uint64_t nImageBufSize

图像缓存大小

### ￿ nImageBufLen

uint64_t nImageBufLen

图像长度

示例 GrabImage_Display.cpp, HighBandwidthDecode.cpp, ImageSave.cpp , 以及 MultiLightCtrl_ImageStitching.cpp.

### ￿ nReserved

unsigned int nReserved[4]

预留字段

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_CC_INPUT_FRAME_INFO结构体 参考

传入的图像数据 更多...

### 成员变量

unsigned char * pData [IN] 图像数据指针 更多...

unsigned int nDataLen [IN] 图像大小 更多...

unsigned int nRes [8] 预留 更多...

### 详细描述

传入的图像数据

示例 DynamicallyLoadDLL.cpp , 以及 Recording.cpp.

### 结构体成员变量说明

### ￿ pData

unsigned char* pData

[IN] 图像数据指针

示例 Recording.cpp.

### ￿ nDataLen

unsigned int nDataLen

[IN] 图像大小

示例 Recording.cpp.

### ￿ nRes

unsigned int nRes[8]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_CC_LSC_CALIB_PARAM结构体 参考

LSC标定结构体 更多...

### 成员变量

unsigned int nWidth [IN] 图像宽度（16-65535） 更多...

unsigned int nHeight [IN] 图像高度（16-65535） 更多...

enum MvGvspPixelType enPixelType [IN] 像素格式 更多...

unsigned char * pSrcBuf [IN] 输入数据缓存 更多...

unsigned int nSrcBufLen [IN] 输入数据长度 更多...

unsigned char * pCalibBuf [OUT] 输出标定表缓存 更多...

unsigned int nCalibBufSize [IN] 提供的标定表缓冲大小(nWidth*nHeight*sizeof(unsigned short)) 更多...

unsigned int nCalibBufLen [OUT] 输出标定表缓存长度 更多...

unsigned int nSecNumW [IN] 宽度分块数 更多...

unsigned int nSecNumH [IN] 高度分块数 更多...

unsigned int nPadCoef [IN] 边缘填充系数[1-5] 更多...

unsigned int nCalibMethod [IN] 标定方式（0-中心为基准；1-最亮区域为基准；2-目标亮度） 更 多...

unsigned int nTargetGray

unsigned int nRes [8] 预留 更多...

### 详细描述

LSC标定结构体

示例 LensShadingCorrection.cpp.

### 结构体成员变量说明

### ￿ nWidth

unsigned int nWidth

[IN] 图像宽度（16-65535）

示例 LensShadingCorrection.cpp.

### ￿ nHeight

unsigned int nHeight

[IN] 图像高度（16-65535）

示例 LensShadingCorrection.cpp.

### ￿ enPixelType

enum MvGvspPixelType enPixelType

[IN] 像素格式

示例 LensShadingCorrection.cpp.

### ￿ pSrcBuf

unsigned char* pSrcBuf

[IN] 输入数据缓存

示例 LensShadingCorrection.cpp.

### ￿ nSrcBufLen

unsigned int nSrcBufLen

[IN] 输入数据长度

示例 LensShadingCorrection.cpp.

### ￿ pCalibBuf

unsigned char* pCalibBuf

[OUT] 输出标定表缓存

示例 LensShadingCorrection.cpp.

### ￿ nCalibBufSize

unsigned int nCalibBufSize

[IN] 提供的标定表缓冲大小(nWidth*nHeight*sizeof(unsigned short))

示例 LensShadingCorrection.cpp.

### ￿ nCalibBufLen

unsigned int nCalibBufLen

[OUT] 输出标定表缓存长度

示例 LensShadingCorrection.cpp.

### ￿ nSecNumW

unsigned int nSecNumW

[IN] 宽度分块数

示例 LensShadingCorrection.cpp.

### ￿ nSecNumH

unsigned int nSecNumH

[IN] 高度分块数

示例 LensShadingCorrection.cpp.

### ￿ nPadCoef

unsigned int nPadCoef

[IN] 边缘填充系数[1-5]

示例 LensShadingCorrection.cpp.

### ￿ nCalibMethod

unsigned int nCalibMethod

[IN] 标定方式（0-中心为基准；1-最亮区域为基准；2-目标亮度）

示例 LensShadingCorrection.cpp.

### ￿ nTargetGray

unsigned int nTargetGray

[IN] 目标亮度（标定方式为2时有效） 8位，范围：[0,255] 10位，范围：[0,1023] 12位，范围 ：[0,4095] 16位，范围：[0,65535]

示例 LensShadingCorrection.cpp.

### ￿ nRes

unsigned int nRes[8]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_CC_LSC_CORRECT_PARAM结构体 参考

LSC校正结构体 更多...

### 成员变量

unsigned int nWidth [IN] 图像宽度(16-65535) 更多...

unsigned int nHeight [IN] 图像高度(16-65535) 更多...

enum MvGvspPixelType enPixelType [IN] 像素格式 更多...

unsigned char * pSrcBuf [IN] 输入数据缓存 更多...

unsigned int nSrcBufLen [IN] 输入数据长度 更多...

unsigned char * pDstBuf [OUT] 输出数据缓存 更多...

unsigned int nDstBufSize [IN] 提供的输出缓冲区大小 更多...

unsigned int nDstBufLen [OUT] 输出数据长度 更多...

unsigned char * pCalibBuf [IN] 输入标定表缓存 更多...

unsigned int nCalibBufLen [IN] 输入标定表缓存长度 更多...

unsigned int nRes [8] 预留 更多...

### 详细描述

LSC校正结构体

示例 LensShadingCorrection.cpp.

### 结构体成员变量说明

### ￿ nWidth

unsigned int nWidth

[IN] 图像宽度(16-65535)

示例 LensShadingCorrection.cpp.

### ￿ nHeight

unsigned int nHeight

[IN] 图像高度(16-65535)

示例 LensShadingCorrection.cpp.

### ￿ enPixelType

enum MvGvspPixelType enPixelType

[IN] 像素格式

示例 LensShadingCorrection.cpp.

### ￿ pSrcBuf

unsigned char* pSrcBuf

[IN] 输入数据缓存

示例 LensShadingCorrection.cpp.

### ￿ nSrcBufLen

unsigned int nSrcBufLen

[IN] 输入数据长度

示例 LensShadingCorrection.cpp.

### ￿ pDstBuf

unsigned char* pDstBuf

[OUT] 输出数据缓存

示例 LensShadingCorrection.cpp.

### ￿ nDstBufSize

unsigned int nDstBufSize

[IN] 提供的输出缓冲区大小

示例 LensShadingCorrection.cpp.

### ￿ nDstBufLen

unsigned int nDstBufLen

[OUT] 输出数据长度

### ￿ pCalibBuf

unsigned char* pCalibBuf

[IN] 输入标定表缓存

示例 LensShadingCorrection.cpp.

### ￿ nCalibBufLen

unsigned int nCalibBufLen

[IN] 输入标定表缓存长度

示例 LensShadingCorrection.cpp.

### ￿ nRes

unsigned int nRes[8]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_CC_NOISE_ESTIMATE_PARAM结构体 参考

噪声估计结构体 更多...

### 成员变量

unsigned int nWidth [IN] 图像宽度 更多...

unsigned int nHeight [IN] 图像高度 更多...

enum MvGvspPixelType enPixelType [IN] 像素格式 更多...

unsigned char * pSrcBuf [IN] 输入数据缓存 更多...

unsigned int nSrcBufLen [IN] 输入数据大小 更多...

MV_CC_RECT_I * pstROIRect [IN] 图像ROI 更多...

unsigned int nROINum [IN] ROI个数 更多...

unsigned int nNoiseThreshold [IN] 噪声阈值[0,4095] 更多...

unsigned char * pNoiseProfile [OUT] 输出噪声特性 更多...

unsigned int nNoiseProfileSize [IN] 提供的输出缓冲区大小 更多...

unsigned int nNoiseProfileLen [OUT] 输出噪声特性长度 更多...

unsigned int nRes [8] 预留 更多...

### 详细描述

噪声估计结构体

### 结构体成员变量说明

### ￿ nWidth

unsigned int nWidth

[IN] 图像宽度

### ￿ nHeight

unsigned int nHeight

[IN] 图像高度

### ￿ enPixelType

enum MvGvspPixelType enPixelType

[IN] 像素格式

### ￿ pSrcBuf

unsigned char* pSrcBuf

[IN] 输入数据缓存

### ￿ nSrcBufLen

unsigned int nSrcBufLen

[IN] 输入数据大小

### ￿ pstROIRect

MV_CC_RECT_I* pstROIRect

[IN] 图像ROI

### ￿ nROINum

unsigned int nROINum

[IN] ROI个数

Bayer域噪声估计参数，Mono8/RGB域无效

### ￿ nNoiseThreshold

unsigned int nNoiseThreshold

[IN] 噪声阈值[0,4095]

### ￿ pNoiseProfile

unsigned char* pNoiseProfile

[OUT] 输出噪声特性

### ￿ nNoiseProfileSize

unsigned int nNoiseProfileSize

[IN] 提供的输出缓冲区大小

### ￿ nNoiseProfileLen

unsigned int nNoiseProfileLen

[OUT] 输出噪声特性长度

### ￿ nRes

unsigned int nRes[8]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_CC_PIXEL_CONVERT_PARAM_EX结构体 参考

像素转换结构体 更多...

### 成员变量

unsigned int nWidth [IN] 图像宽 更多...

unsigned int nHeight [IN] 图像高 更多...

enum MvGvspPixelType enSrcPixelType [IN] 源像素格式 更多...

unsigned char * pSrcData [IN] 输入数据缓存 更多...

unsigned int nSrcDataLen [IN] 输入数据长度 更多...

enum MvGvspPixelType enDstPixelType [IN] 目标像素格式 更多...

unsigned char * pDstBuffer [OUT] 输出数据缓存 更多...

unsigned int nDstLen [OUT] 输出数据长度 更多...

unsigned int nDstBufferSize [IN] 提供的输出缓冲区大小 更多...

unsigned int nRes [4] 预留 更多...

### 详细描述

像素转换结构体

示例 ConvertPixelType.cpp.

### 结构体成员变量说明

### ￿ nWidth

unsigned int nWidth

[IN] 图像宽

示例 ConvertPixelType.cpp.

### ￿ nHeight

unsigned int nHeight

[IN] 图像高

示例 ConvertPixelType.cpp.

### ￿ enSrcPixelType

enum MvGvspPixelType enSrcPixelType

[IN] 源像素格式

示例 ConvertPixelType.cpp.

### ￿ pSrcData

unsigned char* pSrcData

[IN] 输入数据缓存

示例 ConvertPixelType.cpp.

### ￿ nSrcDataLen

unsigned int nSrcDataLen

[IN] 输入数据长度

示例 ConvertPixelType.cpp.

### ￿ enDstPixelType

enum MvGvspPixelType enDstPixelType

[IN] 目标像素格式

示例 ConvertPixelType.cpp.

### ￿ pDstBuffer

unsigned char* pDstBuffer

[OUT] 输出数据缓存

示例 ConvertPixelType.cpp.

### ￿ nDstLen

unsigned int nDstLen

[OUT] 输出数据长度

示例 ConvertPixelType.cpp.

### ￿ nDstBufferSize

unsigned int nDstBufferSize

[IN] 提供的输出缓冲区大小

示例 ConvertPixelType.cpp.

### ￿ nRes

unsigned int nRes[4]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_CC_RECORD_PARAM结构体 参考

录像参数 更多...

### 成员变量

enum MvGvspPixelType enPixelType [IN] 输入数据的像素格式 更多...

unsigned short nWidth [IN] 图像宽（指定目标参数时需为2的倍数） 更多...

unsigned short nHeight [IN] 图像高（指定目标参数时需为2的倍数） 更多...

float fFrameRate [IN] 帧率fps(大于1/16) 更多...

unsigned int nBitRate [IN] 码率kbps(128-16*1024) 更多...

MV_RECORD_FORMAT_TYPE enRecordFmtType [IN] 录像格式 更多...

char * strFilePath [IN] 录像文件存放路径（如果路径中存在中文，需转成utf-8） 更多...

unsigned int nRes [8] 预留 更多...

### 详细描述

录像参数

示例 DynamicallyLoadDLL.cpp , 以及 Recording.cpp.

### 结构体成员变量说明

### ￿ enPixelType

enum MvGvspPixelType enPixelType

[IN] 输入数据的像素格式

示例 Recording.cpp.

### ￿ nWidth

unsigned short nWidth

[IN] 图像宽（指定目标参数时需为2的倍数）

示例 Recording.cpp.

### ￿ nHeight

unsigned short nHeight

[IN] 图像高（指定目标参数时需为2的倍数）

示例 Recording.cpp.

### ￿ fFrameRate

float fFrameRate

[IN] 帧率fps(大于1/16)

示例 Recording.cpp.

### ￿ nBitRate

unsigned int nBitRate

[IN] 码率kbps(128-16*1024)

示例 Recording.cpp.

### ￿ enRecordFmtType

MV_RECORD_FORMAT_TYPE enRecordFmtType

[IN] 录像格式

示例 Recording.cpp.

### ￿ strFilePath

char* strFilePath

[IN] 录像文件存放路径（如果路径中存在中文，需转成utf-8）

示例 Recording.cpp.

### ￿ nRes

unsigned int nRes[8]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_CC_RECT_I结构体 参考

矩形ROI结构体 更多...

### 成员变量

unsigned int nX 矩形左上角X轴坐标 更多...

unsigned int nY 矩形左上角Y轴坐标 更多...

unsigned int nWidth 矩形宽度 更多...

unsigned int nHeight 矩形高度 更多...

### 详细描述

矩形ROI结构体

### 结构体成员变量说明

### ￿ nX

unsigned int nX

矩形左上角X轴坐标

### ￿ nY

unsigned int nY

矩形左上角Y轴坐标

### ￿ nWidth

unsigned int nWidth

矩形宽度

### ￿ nHeight

unsigned int nHeight

矩形高度

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_CC_ROTATE_IMAGE_PARAM结构体 参考

图像旋转结构体 更多...

### 成员变量

enum MvGvspPixelType enPixelType [IN] 像素格式 更多...

unsigned int nWidth [IN][OUT] 图像宽 更多...

unsigned int nHeight [IN][OUT] 图像高 更多...

unsigned char * pSrcData [IN] 输入数据缓存 更多...

unsigned int nSrcDataLen [IN] 输入数据长度 更多...

unsigned char * pDstBuf [OUT] 输出数据缓存 更多...

unsigned int nDstBufLen [OUT] 输出数据长度 更多...

unsigned int nDstBufSize [IN] 提供的输出缓冲区大小 更多...

MV_IMG_ROTATION_ANGLE enRotationAngle [IN] 旋转角度 更多...

unsigned int nRes [8] 预留 更多...

### 详细描述

图像旋转结构体

### 结构体成员变量说明

### ￿ enPixelType

enum MvGvspPixelType enPixelType

[IN] 像素格式

### ￿ nWidth

unsigned int nWidth

[IN][OUT] 图像宽

### ￿ nHeight

unsigned int nHeight

[IN][OUT] 图像高

### ￿ pSrcData

unsigned char* pSrcData

[IN] 输入数据缓存

### ￿ nSrcDataLen

unsigned int nSrcDataLen

[IN] 输入数据长度

### ￿ pDstBuf

unsigned char* pDstBuf

[OUT] 输出数据缓存

### ￿ nDstBufLen

unsigned int nDstBufLen

[OUT] 输出数据长度

### ￿ nDstBufSize

unsigned int nDstBufSize

[IN] 提供的输出缓冲区大小

### ￿ enRotationAngle

MV_IMG_ROTATION_ANGLE enRotationAngle

[IN] 旋转角度

### ￿ nRes

unsigned int nRes[8]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_CC_SAVE_IMAGE_PARAM结构体 参考

保存图片所需参数 更多...

### 成员变量

MV_SAVE_IAMGE_TYPE enImageType [IN] 输入图片格式 更多...

unsigned int nQuality [IN] JPG编码质量(50-99]，其它格式无效 更多...

int iMethodValue [IN] 插值方法 0-快速 1-均衡（其它值默认为均衡） 2-最优 3-最优+ 更多...

unsigned int nReserved [8] 预留 更多...

### 详细描述

保存图片所需参数

示例 HighBandwidthDecode.cpp, ImageSave.cpp , 以及 MultiLightCtrl_ImageStitching.cpp.

### 结构体成员变量说明

### ￿ enImageType

MV_SAVE_IAMGE_TYPE enImageType

[IN] 输入图片格式

示例 HighBandwidthDecode.cpp, ImageSave.cpp , 以及 MultiLightCtrl_ImageStitching.cpp.

### ￿ nQuality

unsigned int nQuality

[IN] JPG编码质量(50-99]，其它格式无效

示例 ImageSave.cpp , 以及 MultiLightCtrl_ImageStitching.cpp.

### ￿ iMethodValue

int iMethodValue

[IN] 插值方法 0-快速 1-均衡（其它值默认为均衡） 2-最优 3-最优+

示例 HighBandwidthDecode.cpp, ImageSave.cpp , 以及 MultiLightCtrl_ImageStitching.cpp.

### ￿ nReserved

unsigned int nReserved[8]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_CC_SHARPEN_PARAM结构体 参考

锐化结构体 更多...

### 成员变量

unsigned int nWidth [IN] 图像宽度（最小8） 更多...

unsigned int nHeight [IN] 图像高度（最小8） 更多...

unsigned char * pSrcBuf [IN] 输入数据缓存 更多...

unsigned int nSrcBufLen [IN] 输入数据大小 更多...

enum MvGvspPixelType enPixelType [IN] 像素格式 更多...

unsigned char * pDstBuf [OUT] 输出数据缓存 更多...

unsigned int nDstBufSize [IN] 提供的输出缓冲区大小 更多...

unsigned int nDstBufLen [OUT] 输出数据长度 更多...

unsigned int nSharpenAmount [IN] 锐度调节强度[0,500] 更多...

unsigned int nSharpenRadius [IN] 锐度调节半径[1,21]（半径越大，耗时越长） 更多...

unsigned int nSharpenThreshold [IN] 锐度调节阈值[0,255] 更多...

unsigned int nSharpenPosAmount [IN] 锐度调节正向强度，范围:[0,500] 更多...

unsigned int nSharpenNegAmount [IN] 锐度调节负向强度，范围:[0,500] 更多...

unsigned int nRes [6] 预留 更多...

### 详细描述

锐化结构体

### 结构体成员变量说明

### ￿ nWidth

unsigned int nWidth

[IN] 图像宽度（最小8）

### ￿ nHeight

unsigned int nHeight

[IN] 图像高度（最小8）

### ￿ pSrcBuf

unsigned char* pSrcBuf

[IN] 输入数据缓存

### ￿ nSrcBufLen

unsigned int nSrcBufLen

[IN] 输入数据大小

### ￿ enPixelType

enum MvGvspPixelType enPixelType

[IN] 像素格式

### ￿ pDstBuf

unsigned char* pDstBuf

[OUT] 输出数据缓存

### ￿ nDstBufSize

unsigned int nDstBufSize

[IN] 提供的输出缓冲区大小

### ￿ nDstBufLen

unsigned int nDstBufLen

[OUT] 输出数据长度

### ￿ nSharpenAmount

unsigned int nSharpenAmount

[IN] 锐度调节强度[0,500]

### ￿ nSharpenRadius

unsigned int nSharpenRadius

[IN] 锐度调节半径[1,21]（半径越大，耗时越长）

### ￿ nSharpenThreshold

unsigned int nSharpenThreshold

[IN] 锐度调节阈值[0,255]

### ￿ nSharpenPosAmount

unsigned int nSharpenPosAmount

[IN] 锐度调节正向强度，范围:[0,500]

### ￿ nSharpenNegAmount

unsigned int nSharpenNegAmount

[IN] 锐度调节负向强度，范围:[0,500]

### ￿ nRes

unsigned int nRes[6]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_CHUNK_DATA_CONTENT结构体 参考

Chunk内容 更多...

### 成员变量

unsigned char * pChunkData [OUT] Chunk数据 更多...

unsigned int nChunkID [OUT] Chunk ID 更多...

unsigned int nChunkLen [OUT] Chunk的长度 更多...

unsigned int nReserved [8] 预留 更多...

### 详细描述

Chunk内容

示例 ChunkData.cpp.

### 结构体成员变量说明

### ￿ pChunkData

unsigned char* pChunkData

[OUT] Chunk数据

### ￿ nChunkID

unsigned int nChunkID

[OUT] Chunk ID

示例 ChunkData.cpp.

### ￿ nChunkLen

unsigned int nChunkLen

[OUT] Chunk的长度

示例 ChunkData.cpp.

### ￿ nReserved

unsigned int nReserved[8]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_CML_DEVICE_INFO结构体 参考

Camera Link相机信息 更多...

### 成员变量

unsigned char chInterfaceID [INFO_MAX_BUFFER_SIZE] 采集卡ID 更多...

unsigned char chVendorName [INFO_MAX_BUFFER_SIZE] 供应商名字 更多...

unsigned char chModelName [INFO_MAX_BUFFER_SIZE] 型号名字 更多...

unsigned char chManufacturerInfo [INFO_MAX_BUFFER_SIZE] 厂商信息 更多...

unsigned char chDeviceVersion [INFO_MAX_BUFFER_SIZE] 相机版本 更多...

unsigned char chSerialNumber [INFO_MAX_BUFFER_SIZE] 序列号 更多...

unsigned char chUserDefinedName [INFO_MAX_BUFFER_SIZE] 用户自定义名字 更多...

unsigned char chDeviceID [INFO_MAX_BUFFER_SIZE] 相机ID 更多...

unsigned int nReserved [7] 保留字段 更多...

### 详细描述

Camera Link相机信息

### 结构体成员变量说明

### ￿ chInterfaceID

unsigned char chInterfaceID[INFO_MAX_BUFFER_SIZE]

采集卡ID

### ￿ chVendorName

unsigned char chVendorName[INFO_MAX_BUFFER_SIZE]

供应商名字

### ￿ chModelName

unsigned char chModelName[INFO_MAX_BUFFER_SIZE]

型号名字

示例 ChunkData.cpp, ConvertPixelType.cpp, DynamicallyLoadDLL.cpp, Events.cpp, Events_Interface.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, InterfaceAndDeviceDemo.cpp, LensShadingCorrection.cpp, MultiLightCtrl_ImageStitching.cpp, ParametrizeCamera_LineScanIOSettings.cpp , 以及 Recording.cpp.

### ￿ chManufacturerInfo

unsigned char chManufacturerInfo[INFO_MAX_BUFFER_SIZE]

厂商信息

### ￿ chDeviceVersion

unsigned char chDeviceVersion[INFO_MAX_BUFFER_SIZE]

相机版本

### ￿ chSerialNumber

unsigned char chSerialNumber[INFO_MAX_BUFFER_SIZE]

序列号

示例 ChunkData.cpp, ConvertPixelType.cpp, DynamicallyLoadDLL.cpp, Events.cpp, Events_Interface.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, InterfaceAndDeviceDemo.cpp, LensShadingCorrection.cpp, MultiLightCtrl_ImageStitching.cpp, ParametrizeCamera_LineScanIOSettings.cpp , 以及 Recording.cpp.

### ￿ chUserDefinedName

unsigned char chUserDefinedName[INFO_MAX_BUFFER_SIZE]

用户自定义名字

示例 ChunkData.cpp, ConvertPixelType.cpp, DynamicallyLoadDLL.cpp, Events.cpp, Events_Interface.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, InterfaceAndDeviceDemo.cpp, LensShadingCorrection.cpp, MultiLightCtrl_ImageStitching.cpp, ParametrizeCamera_LineScanIOSettings.cpp , 以及 Recording.cpp.

### ￿ chDeviceID

unsigned char chDeviceID[INFO_MAX_BUFFER_SIZE]

相机ID

### ￿ nReserved

unsigned int nReserved[7]

保留字段

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_CXP_DEVICE_INFO结构体 参考

CoaXPress相机信息 更多...

### 成员变量

unsigned char chInterfaceID [INFO_MAX_BUFFER_SIZE] 采集卡ID 更多...

unsigned char chVendorName [INFO_MAX_BUFFER_SIZE] 供应商名字 更多...

unsigned char chModelName [INFO_MAX_BUFFER_SIZE] 型号名字 更多...

unsigned char chManufacturerInfo [INFO_MAX_BUFFER_SIZE] 厂商信息 更多...

unsigned char chDeviceVersion [INFO_MAX_BUFFER_SIZE] 相机版本 更多...

unsigned char chSerialNumber [INFO_MAX_BUFFER_SIZE] 序列号 更多...

unsigned char chUserDefinedName [INFO_MAX_BUFFER_SIZE] 用户自定义名字 更多...

unsigned char chDeviceID [INFO_MAX_BUFFER_SIZE] 相机ID 更多...

unsigned int nReserved [7] 保留字段 更多...

### 详细描述

CoaXPress相机信息

### 结构体成员变量说明

### ￿ chInterfaceID

unsigned char chInterfaceID[INFO_MAX_BUFFER_SIZE]

采集卡ID

示例 QuickSoftwareTrigger.cpp.

### ￿ chVendorName

unsigned char chVendorName[INFO_MAX_BUFFER_SIZE]

供应商名字

### ￿ chModelName

unsigned char chModelName[INFO_MAX_BUFFER_SIZE]

型号名字

示例 ChunkData.cpp, ConvertPixelType.cpp, DynamicallyLoadDLL.cpp, Events.cpp, Events_Interface.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, InterfaceAndDeviceDemo.cpp, LensShadingCorrection.cpp, MultiLightCtrl_ImageStitching.cpp, ParametrizeCamera_LineScanIOSettings.cpp, QuickSoftwareTrigger.cpp , 以及 Recording.cpp.

### ￿ chManufacturerInfo

unsigned char chManufacturerInfo[INFO_MAX_BUFFER_SIZE]

厂商信息

### ￿ chDeviceVersion

unsigned char chDeviceVersion[INFO_MAX_BUFFER_SIZE]

相机版本

### ￿ chSerialNumber

unsigned char chSerialNumber[INFO_MAX_BUFFER_SIZE]

序列号

示例 ChunkData.cpp, ConvertPixelType.cpp, DynamicallyLoadDLL.cpp, Events.cpp, Events_Interface.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, InterfaceAndDeviceDemo.cpp, LensShadingCorrection.cpp, MultiLightCtrl_ImageStitching.cpp, ParametrizeCamera_LineScanIOSettings.cpp, QuickSoftwareTrigger.cpp , 以及 Recording.cpp.

### ￿ chUserDefinedName

unsigned char chUserDefinedName[INFO_MAX_BUFFER_SIZE]

用户自定义名字

示例 ChunkData.cpp, ConvertPixelType.cpp, DynamicallyLoadDLL.cpp, Events.cpp, Events_Interface.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, InterfaceAndDeviceDemo.cpp, LensShadingCorrection.cpp, MultiLightCtrl_ImageStitching.cpp, ParametrizeCamera_LineScanIOSettings.cpp, QuickSoftwareTrigger.cpp , 以及 Recording.cpp.

### ￿ chDeviceID

unsigned char chDeviceID[INFO_MAX_BUFFER_SIZE]

相机ID

### ￿ nReserved

unsigned int nReserved[7]

保留字段

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_DISPLAY_FRAME_INFO结构体 参考

显示帧信息（自4.4.1版本废弃） 更多...

### 成员变量

void * hWnd [IN] 窗口句柄 更多...

unsigned char * pData [IN] 显示的数据 更多...

unsigned int nDataLen [IN] 数据长度 更多...

unsigned short nWidth [IN] 图像宽 更多...

unsigned short nHeight [IN] 图像高 更多...

enum MvGvspPixelType enPixelType [IN] 像素格式 更多...

unsigned int enRenderMode [IN] 图像渲染方式 0-GDI（默认）, 1-D3D, 2-OpenGL 更多...

unsigned int nRes [3] 保留 更多...

### 详细描述

显示帧信息（自4.4.1版本废弃）

示例 DynamicallyLoadDLL.cpp.

### 结构体成员变量说明

### ￿ hWnd

void* hWnd

[IN] 窗口句柄

### ￿ pData

unsigned char* pData

[IN] 显示的数据

### ￿ nDataLen

unsigned int nDataLen

[IN] 数据长度

### ￿ nWidth

unsigned short nWidth

[IN] 图像宽

### ￿ nHeight

unsigned short nHeight

[IN] 图像高

### ￿ enPixelType

enum MvGvspPixelType enPixelType

[IN] 像素格式

### ￿ enRenderMode

unsigned int enRenderMode

[IN] 图像渲染方式 0-GDI（默认）, 1-D3D, 2-OpenGL

### ￿ nRes

unsigned int nRes[3]

保留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_DISPLAY_FRAME_INFO_EX结构体 参考

显示帧信息（适用于例如线阵相机等宽高大于65535的设备） 更多...

### 成员变量

unsigned int nWidth [IN] 图像宽 更多...

unsigned int nHeight [IN] 图像高 更多...

enum MvGvspPixelType enPixelType [IN] 像素格式 更多...

unsigned char * pImageBuf [IN] 输入图像缓存 更多...

unsigned int nImageBufLen [IN] 输入图像长度 更多...

unsigned int enRenderMode [IN] 图像渲染方式。0表示GDI模式，1表示D3D模式，2表示OpenGL 模式 更多...

unsigned int nRes [3] 保留 更多...

### 详细描述

显示帧信息（适用于例如线阵相机等宽高大于65535的设备）

### 结构体成员变量说明

### ￿ nWidth

unsigned int nWidth

[IN] 图像宽

### ￿ nHeight

unsigned int nHeight

[IN] 图像高

### ￿ enPixelType

enum MvGvspPixelType enPixelType

[IN] 像素格式

### ￿ pImageBuf

unsigned char* pImageBuf

[IN] 输入图像缓存

### ￿ nImageBufLen

unsigned int nImageBufLen

[IN] 输入图像长度

### ￿ enRenderMode

unsigned int enRenderMode

[IN] 图像渲染方式。0表示GDI模式，1表示D3D模式，2表示OpenGL模式

### ￿ nRes

unsigned int nRes[3]

保留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_EVENT_OUT_INFO结构体 参考

Event事件回调信息 更多...

### 成员变量

char EventName [MAX_EVENT_NAME_SIZE] [OUT] Event名称 更多...

unsigned short nEventID [OUT] Event号 更多...

unsigned short nStreamChannel [OUT] 流通道序号 更多...

unsigned int nBlockIdHigh [OUT] 帧号高位 更多...

unsigned int nBlockIdLow [OUT] 帧号低位 更多...

unsigned int nTimestampHigh [OUT] 时间戳高位 更多...

unsigned int nTimestampLow

[OUT] 时间戳低位 更多...

void * pEventData [OUT] Event数据 更多...

unsigned int nEventDataSize [OUT] Event数据长度 更多...

unsigned int nReserved [16] 预留 更多...

### 详细描述

Event事件回调信息

示例 DynamicallyLoadDLL.cpp, Events.cpp , 以及 Events_Interface.cpp.

### 结构体成员变量说明

### ￿ EventName

char EventName[MAX_EVENT_NAME_SIZE]

[OUT] Event名称

示例 Events.cpp , 以及 Events_Interface.cpp.

### ￿ nEventID

unsigned short nEventID

[OUT] Event号

示例

Events.cpp , 以及 Events_Interface.cpp.

### ￿ nStreamChannel

unsigned short nStreamChannel

[OUT] 流通道序号

### ￿ nBlockIdHigh

unsigned int nBlockIdHigh

[OUT] 帧号高位

示例 Events.cpp.

### ￿ nBlockIdLow

unsigned int nBlockIdLow

[OUT] 帧号低位

示例 Events.cpp.

### ￿ nTimestampHigh

unsigned int nTimestampHigh

[OUT] 时间戳高位

示例 Events.cpp , 以及 Events_Interface.cpp.

### ￿ nTimestampLow

unsigned int nTimestampLow

[OUT] 时间戳低位

示例 Events.cpp , 以及 Events_Interface.cpp.

### ￿ pEventData

void* pEventData

[OUT] Event数据

### ￿ nEventDataSize

unsigned int nEventDataSize

[OUT] Event数据长度

### ￿ nReserved

unsigned int nReserved[16]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_FRAME_OUT结构体 参考

图像结构体，输出图像地址及图像信息 更多...

### 成员变量

unsigned char * pBufAddr [OUT] 图像指针地址 更多...

MV_FRAME_OUT_INFO_EX stFrameInfo [OUT] 图像信息 更多...

unsigned int nRes [16] 预留 更多...

### 详细描述

图像结构体，输出图像地址及图像信息

示例 ConnectSpecCamera.cpp, ConvertPixelType.cpp, DynamicallyLoadDLL.cpp, Grab_ActionCommand.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, MultiCast.cpp, MultiLightCtrl_ImageStitching.cpp, QuickSoftwareTrigger.cpp , 以及 Recording.cpp.

### 结构体成员变量说明

### ￿ pBufAddr

unsigned char* pBufAddr

[OUT] 图像指针地址

示例 ConvertPixelType.cpp, GrabImage_Display.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, MultiLightCtrl_ImageStitching.cpp , 以及 Recording.cpp.

### ￿ stFrameInfo

MV_FRAME_OUT_INFO_EX stFrameInfo

[OUT] 图像信息

示例 ConnectSpecCamera.cpp, ConvertPixelType.cpp, DynamicallyLoadDLL.cpp, Grab_ActionCommand.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, MultiCast.cpp, MultiLightCtrl_ImageStitching.cpp, QuickSoftwareTrigger.cpp , 以及 Recording.cpp.

### ￿ nRes

unsigned int nRes[16]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_FRAME_OUT_INFO_EX结构体 参考

输出帧的信息 更多...

### 成员变量

unsigned short nWidth [OUT] 图像宽(最大65535，超 出请用nExtendWidth) 更多...

unsigned short nHeight [OUT] 图像高(最大65535，超 出请用nExtendHeight) 更多...

enum MvGvspPixelType enPixelType [OUT] 像素格式 更多...

unsigned int nFrameNum [OUT] 帧号 更多...

unsigned int nDevTimeStampHigh [OUT] 时间戳高32位 更多...

unsigned int nDevTimeStampLow [OUT] 时间戳低32位 更多...

unsigned int nReserved0

[OUT] 保留，8字节对齐 更多 ...

int64_t nHostTimeStamp [OUT] 主机生成的时间戳 更多 ...

unsigned int nFrameLen [OUT] 帧的长度(4GB以上图像 使用nFrameLenEx替代) 更多 ...

unsigned int nSecondCount [OUT] 设备水印时标：秒数 更 多...

unsigned int nCycleCount [OUT] 设备水印时标：周期数 更多...

unsigned int nCycleOffset [OUT] 设备水印时标：周期偏 移量 更多...

float fGain [OUT] 增益 更多...

float fExposureTime [OUT] 曝光时间 更多...

unsigned int nAverageBrightness [OUT] 平均亮度 更多...

unsigned int nRed [OUT] 白平衡相关：红色 更多 ...

unsigned int nGreen

[OUT] 白平衡相关：绿色 更多 ...

unsigned int nBlue [OUT] 白平衡相关：蓝色 更多 ...

unsigned int nFrameCounter [OUT] 总帧数 更多...

unsigned int nTriggerIndex [OUT] 触发计数 更多...

unsigned int nInput [OUT] 输入 更多...

unsigned int nOutput [OUT] 输出 更多...

unsigned short nOffsetX [OUT] ROI区域水平偏移量 更 多...

unsigned short nOffsetY [OUT] ROI区域垂直偏移量 更 多...

unsigned short nChunkWidth [OUT] ROI区域Chunk宽 更多 ...

unsigned short nChunkHeight [OUT] ROI区域Chunk高 更多 ...

unsigned int nLostPacket [OUT] 本帧丢包数 更多...

unsigned int nUnparsedChunkNum

[OUT] 未解析的Chunkdata个 数 更多...

union { MV_CHUNK_DATA_CONTENT * pUnparsedChunkContent [OUT] 未解析的Chunk 更多...

int64_t nAligning [OUT] 校准 更多...

} UnparsedChunkList

unsigned int nExtendWidth [OUT] 图像宽(扩展变量) 更多 ...

unsigned int nExtendHeight [OUT] 图像高(扩展变量) 更多 ...

uint64_t nFrameLenEx [OUT] 帧的长度 更多...

unsigned int nReserved [32] 预留 更多...

### 详细描述

输出帧的信息

示例 ChunkData.cpp, DynamicallyLoadDLL.cpp, Events_Interface.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, InterfaceAndDeviceDemo.cpp, LensShadingCorrection.cpp , 以及 ParametrizeCamera_LineScanIOSettings.cpp.

### 结构体成员变量说明

### ￿ nWidth

unsigned short nWidth

[OUT] 图像宽(最大65535，超出请用nExtendWidth)

示例 ConnectSpecCamera.cpp, ConvertPixelType.cpp, DynamicallyLoadDLL.cpp, Grab_ActionCommand.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, GrabImage.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, InterfaceAndDeviceDemo.cpp, LensShadingCorrection.cpp, MultiCast.cpp, ParametrizeCamera_LineScanIOSettings.cpp, QuickSoftwareTrigger.cpp , 以及 Recording.cpp.

### ￿ nHeight

unsigned short nHeight

[OUT] 图像高(最大65535，超出请用nExtendHeight)

示例 ConnectSpecCamera.cpp, ConvertPixelType.cpp, DynamicallyLoadDLL.cpp, Grab_ActionCommand.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, GrabImage.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, InterfaceAndDeviceDemo.cpp, LensShadingCorrection.cpp, MultiCast.cpp, ParametrizeCamera_LineScanIOSettings.cpp, QuickSoftwareTrigger.cpp , 以及 Recording.cpp.

### ￿ enPixelType

enum MvGvspPixelType enPixelType

[OUT] 像素格式

示例

ConvertPixelType.cpp, GrabImage_Display.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, LensShadingCorrection.cpp , 以及 MultiLightCtrl_ImageStitching.cpp.

### ￿ nFrameNum

unsigned int nFrameNum

[OUT] 帧号

示例 ChunkData.cpp, ConnectSpecCamera.cpp, ConvertPixelType.cpp, DynamicallyLoadDLL.cpp, Grab_ActionCommand.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, InterfaceAndDeviceDemo.cpp, LensShadingCorrection.cpp, MultiCast.cpp, MultiLightCtrl_ImageStitching.cpp, ParametrizeCamera_LineScanIOSettings.cpp, QuickSoftwareTrigger.cpp , 以及 Recording.cpp.

### ￿ nDevTimeStampHigh

unsigned int nDevTimeStampHigh

[OUT] 时间戳高32位

### ￿ nDevTimeStampLow

unsigned int nDevTimeStampLow

[OUT] 时间戳低32位

### ￿ nReserved0

unsigned int nReserved0

[OUT] 保留，8字节对齐

### ￿ nHostTimeStamp

int64_t nHostTimeStamp

[OUT] 主机生成的时间戳

### ￿ nFrameLen

unsigned int nFrameLen

[OUT] 帧的长度(4GB以上图像使用nFrameLenEx替代)

示例 ConvertPixelType.cpp, Grab_Asynchronous.cpp, ImageContrast.cpp, ImageSave.cpp, LensShadingCorrection.cpp , 以及 Recording.cpp.

### ￿ nSecondCount

unsigned int nSecondCount

[OUT] 设备水印时标：秒数

示例 ChunkData.cpp.

### ￿ nCycleCount

unsigned int nCycleCount

[OUT] 设备水印时标：周期数

示例 ChunkData.cpp.

### ￿ nCycleOffset

unsigned int nCycleOffset

[OUT] 设备水印时标：周期偏移量

示例 ChunkData.cpp.

### ￿ fGain

float fGain

[OUT] 增益

### ￿ fExposureTime

float fExposureTime

[OUT] 曝光时间

示例 ChunkData.cpp.

### ￿ nAverageBrightness

unsigned int nAverageBrightness

[OUT] 平均亮度

### ￿ nRed

unsigned int nRed

[OUT] 白平衡相关：红色

### ￿ nGreen

unsigned int nGreen

[OUT] 白平衡相关：绿色

### ￿ nBlue

unsigned int nBlue

[OUT] 白平衡相关：蓝色

### ￿ nFrameCounter

unsigned int nFrameCounter

[OUT] 总帧数

### ￿ nTriggerIndex

unsigned int nTriggerIndex

[OUT] 触发计数

### ￿ nInput

unsigned int nInput

[OUT] 输入

### ￿ nOutput

unsigned int nOutput

[OUT] 输出

### ￿ nOffsetX

unsigned short nOffsetX

[OUT] ROI区域水平偏移量

### ￿ nOffsetY

unsigned short nOffsetY

[OUT] ROI区域垂直偏移量

### ￿ nChunkWidth

unsigned short nChunkWidth

[OUT] ROI区域Chunk宽

### ￿ nChunkHeight

unsigned short nChunkHeight

[OUT] ROI区域Chunk高

### ￿ nLostPacket

unsigned int nLostPacket

[OUT] 本帧丢包数

### ￿ nUnparsedChunkNum

unsigned int nUnparsedChunkNum

[OUT] 未解析的Chunkdata个数

示例 ChunkData.cpp.

### ￿ pUnparsedChunkContent

MV_CHUNK_DATA_CONTENT* pUnparsedChunkContent

[OUT] 未解析的Chunk

示例 ChunkData.cpp.

### ￿ nAligning

int64_t nAligning

[OUT] 校准

### ￿ UnparsedChunkList

union { ... } UnparsedChunkList

示例

ChunkData.cpp.

### ￿ nExtendWidth

unsigned int nExtendWidth

[OUT] 图像宽(扩展变量)

示例 GrabImage_Display.cpp , 以及 MultiLightCtrl_ImageStitching.cpp.

### ￿ nExtendHeight

unsigned int nExtendHeight

[OUT] 图像高(扩展变量)

示例 GrabImage_Display.cpp , 以及 MultiLightCtrl_ImageStitching.cpp.

### ￿ nFrameLenEx

uint64_t nFrameLenEx

[OUT] 帧的长度

示例

GrabImage_Display.cpp, HighBandwidthDecode.cpp, ImageSave.cpp , 以及 MultiLightCtrl_ImageStitching.cpp.

### ￿ nReserved

unsigned int nReserved[32]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_GENTL_DEV_INFO结构体 参考

通过GenTL枚举到的设备信息 更多...

### 成员变量

unsigned char chInterfaceID [INFO_MAX_BUFFER_SIZE] [OUT] GenTL接口ID 更多...

unsigned char chDeviceID [INFO_MAX_BUFFER_SIZE] [OUT] 设备ID 更多...

unsigned char chVendorName [INFO_MAX_BUFFER_SIZE] [OUT] 供应商名字 更多...

unsigned char chModelName [INFO_MAX_BUFFER_SIZE] [OUT] 型号名字 更多...

unsigned char chTLType [INFO_MAX_BUFFER_SIZE] [OUT] 传输层类型 更多...

unsigned char chDisplayName [INFO_MAX_BUFFER_SIZE] [OUT] 设备显示名称 更多...

unsigned char chUserDefinedName [INFO_MAX_BUFFER_SIZE] [OUT] 用户自定义名字 更多...

unsigned char chSerialNumber [INFO_MAX_BUFFER_SIZE] [OUT] 序列号 更多...

unsigned char chDeviceVersion [INFO_MAX_BUFFER_SIZE] [OUT] 设备版本号 更多...

unsigned int nCtiIndex [OUT] GenTL的cti文件索引 更多...

unsigned int nReserved [8] 预留 更多...

### 详细描述

通过GenTL枚举到的设备信息

### 结构体成员变量说明

### ￿ chInterfaceID

unsigned char chInterfaceID[INFO_MAX_BUFFER_SIZE]

[OUT] GenTL接口ID

### ￿ chDeviceID

unsigned char chDeviceID[INFO_MAX_BUFFER_SIZE]

[OUT] 设备ID

### ￿ chVendorName

unsigned char chVendorName[INFO_MAX_BUFFER_SIZE]

[OUT] 供应商名字

### ￿ chModelName

unsigned char chModelName[INFO_MAX_BUFFER_SIZE]

[OUT] 型号名字

### ￿ chTLType

unsigned char chTLType[INFO_MAX_BUFFER_SIZE]

[OUT] 传输层类型

### ￿ chDisplayName

unsigned char chDisplayName[INFO_MAX_BUFFER_SIZE]

[OUT] 设备显示名称

### ￿ chUserDefinedName

unsigned char chUserDefinedName[INFO_MAX_BUFFER_SIZE]

[OUT] 用户自定义名字

### ￿ chSerialNumber

unsigned char chSerialNumber[INFO_MAX_BUFFER_SIZE]

[OUT] 序列号

### ￿ chDeviceVersion

unsigned char chDeviceVersion[INFO_MAX_BUFFER_SIZE]

[OUT] 设备版本号

### ￿ nCtiIndex

unsigned int nCtiIndex

[OUT] GenTL的cti文件索引

### ￿ nReserved

unsigned int nReserved[8]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_GENTL_DEV_INFO_LIST结构体 参考

通过GenTL枚举到的设备信息列表 更多...

### 成员变量

unsigned int nDeviceNum [OUT] 在线设备数量 更多...

MV_GENTL_DEV_INFO * pDeviceInfo [MV_MAX_GENTL_DEV_NUM] [OUT] 支持最多256个设备 更多...

### 详细描述

通过GenTL枚举到的设备信息列表

### 结构体成员变量说明

### ￿ nDeviceNum

unsigned int nDeviceNum

[OUT] 在线设备数量

### ￿ pDeviceInfo

MV_GENTL_DEV_INFO* pDeviceInfo[MV_MAX_GENTL_DEV_NUM]

[OUT] 支持最多256个设备

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_GENTL_IF_INFO结构体 参考

通过GenTL枚举到的接口信息 更多...

### 成员变量

unsigned char chInterfaceID [INFO_MAX_BUFFER_SIZE] [OUT] GenTL接口ID 更多...

unsigned char chTLType [INFO_MAX_BUFFER_SIZE] [OUT] 传输层类型 更多...

unsigned char chDisplayName [INFO_MAX_BUFFER_SIZE] [OUT] Interface显示名称 更多...

unsigned int nCtiIndex [OUT] GenTL的cti文件索引 更多...

unsigned int nReserved [8] 预留 更多...

### 详细描述

通过GenTL枚举到的接口信息

### 结构体成员变量说明

### ￿ chInterfaceID

unsigned char chInterfaceID[INFO_MAX_BUFFER_SIZE]

[OUT] GenTL接口ID

### ￿ chTLType

unsigned char chTLType[INFO_MAX_BUFFER_SIZE]

[OUT] 传输层类型

### ￿ chDisplayName

unsigned char chDisplayName[INFO_MAX_BUFFER_SIZE]

[OUT] Interface显示名称

### ￿ nCtiIndex

unsigned int nCtiIndex

[OUT] GenTL的cti文件索引

### ￿ nReserved

unsigned int nReserved[8]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_GENTL_IF_INFO_LIST结构体 参考

通过GenTL枚举到的接口信息列表 更多...

### 成员变量

unsigned int nInterfaceNum [OUT] 在线接口数量 更多...

MV_GENTL_IF_INFO * pIFInfo [MV_MAX_GENTL_IF_NUM] [OUT] 支持最多256个接口 更多...

### 详细描述

通过GenTL枚举到的接口信息列表

### 结构体成员变量说明

### ￿ nInterfaceNum

unsigned int nInterfaceNum

[OUT] 在线接口数量

### ￿ pIFInfo

MV_GENTL_IF_INFO* pIFInfo[MV_MAX_GENTL_IF_NUM]

[OUT] 支持最多256个接口

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_GIGE_DEVICE_INFO结构体 参考

GigE设备信息 更多...

### 成员变量

unsigned int nIpCfgOption [OUT] IP配置选项 更多...

unsigned int nIpCfgCurrent [OUT] 当前IP配置 更多...

unsigned int nCurrentIp [OUT] 当前IP地址 更多...

unsigned int nCurrentSubNetMask [OUT] 当前子网掩码 更多...

unsigned int nDefultGateWay [OUT] 当前网关 更多...

unsigned char chManufacturerName [32] [OUT] 制造商名称 更多...

unsigned char chModelName [32]

[OUT] 型号名称 更多...

unsigned char chDeviceVersion [32] [OUT] 设备版本 更多...

unsigned char chManufacturerSpecificInfo [48] [OUT] 制造商的具体信息 更多...

unsigned char chSerialNumber [16] [OUT] 序列号 更多...

unsigned char chUserDefinedName [16] [OUT] 用户自定义名称 更多...

unsigned int nNetExport [OUT] 网口IP地址 更多...

unsigned int nReserved [4] 预留 更多...

### 详细描述

GigE设备信息

示例 ConnectSpecCamera.cpp.

### 结构体成员变量说明

### ￿ nIpCfgOption

unsigned int nIpCfgOption

[OUT] IP配置选项

### ￿ nIpCfgCurrent

unsigned int nIpCfgCurrent

[OUT] 当前IP配置

### ￿ nCurrentIp

unsigned int nCurrentIp

[OUT] 当前IP地址

示例 ChunkData.cpp, ConnectSpecCamera.cpp, ConvertPixelType.cpp, DynamicallyLoadDLL.cpp, Events.cpp, Events_Interface.cpp, ForceIPEx.cpp, Grab_ActionCommand.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, InterfaceAndDeviceDemo.cpp, LensShadingCorrection.cpp, MultiCast.cpp, MultiLightCtrl_ImageStitching.cpp, ParametrizeCamera_FileAccess.cpp, ParametrizeCamera_LineScanIOSettings.cpp, ParametrizeCamera_LoadAndSave.cpp , 以及 Recording.cpp.

### ￿ nCurrentSubNetMask

unsigned int nCurrentSubNetMask

[OUT] 当前子网掩码

示例 ForceIPEx.cpp.

### ￿ nDefultGateWay

unsigned int nDefultGateWay

[OUT] 当前网关

示例

ForceIPEx.cpp.

### ￿ chManufacturerName

unsigned char chManufacturerName[32]

[OUT] 制造商名称

### ￿ chModelName

unsigned char chModelName[32]

[OUT] 型号名称

示例 ChunkData.cpp, ConvertPixelType.cpp, DynamicallyLoadDLL.cpp, Events.cpp, Events_Interface.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, InterfaceAndDeviceDemo.cpp, LensShadingCorrection.cpp, MultiLightCtrl_ImageStitching.cpp, ParametrizeCamera_LineScanIOSettings.cpp , 以及 Recording.cpp.

### ￿ chDeviceVersion

unsigned char chDeviceVersion[32]

[OUT] 设备版本

### ￿ chManufacturerSpecificInfo

unsigned char chManufacturerSpecificInfo[48]

[OUT] 制造商的具体信息

### ￿ chSerialNumber

unsigned char chSerialNumber[16]

[OUT] 序列号

示例 ChunkData.cpp, ConvertPixelType.cpp, DynamicallyLoadDLL.cpp, Events.cpp, Events_Interface.cpp, ForceIPEx.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, InterfaceAndDeviceDemo.cpp, LensShadingCorrection.cpp, MultiLightCtrl_ImageStitching.cpp, ParametrizeCamera_LineScanIOSettings.cpp , 以及 Recording.cpp.

### ￿ chUserDefinedName

unsigned char chUserDefinedName[16]

[OUT] 用户自定义名称

示例 ChunkData.cpp, ConvertPixelType.cpp, DynamicallyLoadDLL.cpp, Events.cpp, Events_Interface.cpp, ForceIPEx.cpp, Grab_ActionCommand.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, InterfaceAndDeviceDemo.cpp, LensShadingCorrection.cpp, MultiCast.cpp, MultiLightCtrl_ImageStitching.cpp, ParametrizeCamera_FileAccess.cpp, ParametrizeCamera_LineScanIOSettings.cpp, ParametrizeCamera_LoadAndSave.cpp , 以及 Recording.cpp.

### ￿ nNetExport

unsigned int nNetExport

[OUT] 网口IP地址

示例 ConnectSpecCamera.cpp.

### ￿ nReserved

unsigned int nReserved[4]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_INTERFACE_INFO结构体 参考

采集卡信息 更多...

### 成员变量

unsigned int nTLayerType 采集卡类型 更多...

unsigned int nPCIEInfo 采集卡的PCIE插槽信息 更多...

unsigned char chInterfaceID [INFO_MAX_BUFFER_SIZE]

采集卡ID 更多...

unsigned char chDisplayName [INFO_MAX_BUFFER_SIZE] 显示名称 更多...

unsigned char chSerialNumber [INFO_MAX_BUFFER_SIZE] 序列号 更多...

unsigned char chModelName [INFO_MAX_BUFFER_SIZE] [OUT] 型号 更多...

unsigned char chManufacturer [INFO_MAX_BUFFER_SIZE] [OUT] 厂商 更多...

unsigned char chDeviceVersion [INFO_MAX_BUFFER_SIZE] [OUT] 版本号 更多...

unsigned char chUserDefinedName [INFO_MAX_BUFFER_SIZE] [OUT] 自定义名称 更多...

unsigned int nReserved [64] 保留字段 更多...

### 详细描述

采集卡信息

示例 Events_Interface.cpp, InterfaceAndDeviceDemo.cpp, InterfaceDemo.cpp , 以及 QuickSoftwareTrigger.cpp.

### 结构体成员变量说明

### ￿ nTLayerType

unsigned int nTLayerType

采集卡类型

示例 QuickSoftwareTrigger.cpp.

### ￿ nPCIEInfo

unsigned int nPCIEInfo

采集卡的PCIE插槽信息

### ￿ chInterfaceID

unsigned char chInterfaceID[INFO_MAX_BUFFER_SIZE]

采集卡ID

示例 QuickSoftwareTrigger.cpp.

### ￿ chDisplayName

unsigned char chDisplayName[INFO_MAX_BUFFER_SIZE]

显示名称

示例 Events_Interface.cpp, InterfaceAndDeviceDemo.cpp, InterfaceDemo.cpp , 以及 QuickSoftwareTrigger.cpp.

### ￿ chSerialNumber

unsigned char chSerialNumber[INFO_MAX_BUFFER_SIZE]

序列号

示例

Events_Interface.cpp, InterfaceAndDeviceDemo.cpp, InterfaceDemo.cpp , 以及 QuickSoftwareTrigger.cpp.

### ￿ chModelName

unsigned char chModelName[INFO_MAX_BUFFER_SIZE]

[OUT] 型号

示例 Events_Interface.cpp, InterfaceAndDeviceDemo.cpp, InterfaceDemo.cpp , 以及 QuickSoftwareTrigger.cpp.

### ￿ chManufacturer

unsigned char chManufacturer[INFO_MAX_BUFFER_SIZE]

[OUT] 厂商

### ￿ chDeviceVersion

unsigned char chDeviceVersion[INFO_MAX_BUFFER_SIZE]

[OUT] 版本号

### ￿ chUserDefinedName

unsigned char chUserDefinedName[INFO_MAX_BUFFER_SIZE]

[OUT] 自定义名称

### ￿ nReserved

unsigned int nReserved[64]

保留字段

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_INTERFACE_INFO_LIST结构体 参考

采集卡信息列表 更多...

### 成员变量

unsigned int nInterfaceNum [OUT] 采集卡数量 更多...

MV_INTERFACE_INFO * pInterfaceInfos [MV_MAX_INTERFACE_NUM] [OUT] 采集卡信息，支持最多64个设备 更多...

### 详细描述

采集卡信息列表

示例

Events_Interface.cpp, InterfaceAndDeviceDemo.cpp, InterfaceDemo.cpp , 以及 QuickSoftwareTrigger.cpp.

### 结构体成员变量说明

### ￿ nInterfaceNum

unsigned int nInterfaceNum

[OUT] 采集卡数量

示例 Events_Interface.cpp, InterfaceAndDeviceDemo.cpp, InterfaceDemo.cpp , 以及 QuickSoftwareTrigger.cpp.

### ￿ pInterfaceInfos

MV_INTERFACE_INFO* pInterfaceInfos[MV_MAX_INTERFACE_NUM]

[OUT] 采集卡信息，支持最多64个设备

示例 Events_Interface.cpp, InterfaceAndDeviceDemo.cpp, InterfaceDemo.cpp , 以及 QuickSoftwareTrigger.cpp.

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_MATCH_INFO_NET_DETECT结构体 参考

网络流量和丢包信息反馈结构体，对应类型为 MV_MATCH_TYPE_NET_DETECT 更多...

### 成员变量

int64_t nReceiveDataSize [OUT] 已接收数据大小 [统计StartGrabbing和StopGrabbing之间的数据量] 更多...

int64_t nLostPacketCount [OUT] 丢失的包数量 更多...

unsigned int nLostFrameCount [OUT] 丢帧数量 更多...

unsigned int nNetRecvFrameCount [OUT] 接收到的图像帧数 更多...

int64_t nRequestResendPacketCount [OUT] 请求重发包数 更多...

int64_t nResendPacketCount [OUT] 重发包数 更多...

### 详细描述

网络流量和丢包信息反馈结构体，对应类型为 MV_MATCH_TYPE_NET_DETECT

### 结构体成员变量说明

### ￿ nReceiveDataSize

int64_t nReceiveDataSize

[OUT] 已接收数据大小 [统计StartGrabbing和StopGrabbing之间的数据量]

### ￿ nLostPacketCount

int64_t nLostPacketCount

[OUT] 丢失的包数量

### ￿ nLostFrameCount

unsigned int nLostFrameCount

[OUT] 丢帧数量

### ￿ nNetRecvFrameCount

unsigned int nNetRecvFrameCount

[OUT] 接收到的图像帧数

### ￿ nRequestResendPacketCount

int64_t nRequestResendPacketCount

[OUT] 请求重发包数

### ￿ nResendPacketCount

int64_t nResendPacketCount

[OUT] 重发包数

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_MATCH_INFO_USB_DETECT结构体 参考

host收到从u3v设备端的总字节数，对应类型为 MV_MATCH_TYPE_USB_DETECT 更多...

### 成员变量

int64_t nReceiveDataSize [OUT] 已接收数据大小 [统计OpenDevicce和CloseDevice之间的数据量] 更多...

unsigned int nReceivedFrameCount [OUT] 已收到的帧数 更多...

unsigned int nErrorFrameCount [OUT] 错误帧数 更多...

unsigned int nReserved [2]

保留 更多...

### 详细描述

host收到从u3v设备端的总字节数，对应类型为 MV_MATCH_TYPE_USB_DETECT

### 结构体成员变量说明

### ￿ nReceiveDataSize

int64_t nReceiveDataSize

[OUT] 已接收数据大小 [统计OpenDevicce和CloseDevice之间的数据量]

### ￿ nReceivedFrameCount

unsigned int nReceivedFrameCount

[OUT] 已收到的帧数

### ￿ nErrorFrameCount

unsigned int nErrorFrameCount

[OUT] 错误帧数

### ￿ nReserved

unsigned int nReserved[2]

保留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_NETTRANS_INFO结构体 参考

网络传输的相关信息 更多...

### 成员变量

int64_t nReceiveDataSize [OUT] 已接收数据大小 [统计StartGrabbing和StopGrabbing之间的数据量] 更多...

int nThrowFrameCount [OUT] 丢帧数量 更多...

unsigned int nNetRecvFrameCount [OUT] 已接收的帧数 更多...

int64_t nRequestResendPacketCount [OUT] 请求重发包数 更多...

int64_t nResendPacketCount

[OUT] 重发包数 更多...

### 详细描述

网络传输的相关信息

示例 DynamicallyLoadDLL.cpp.

### 结构体成员变量说明

### ￿ nReceiveDataSize

int64_t nReceiveDataSize

[OUT] 已接收数据大小 [统计StartGrabbing和StopGrabbing之间的数据量]

### ￿ nThrowFrameCount

int nThrowFrameCount

[OUT] 丢帧数量

### ￿ nNetRecvFrameCount

unsigned int nNetRecvFrameCount

[OUT] 已接收的帧数

### ￿ nRequestResendPacketCount

int64_t nRequestResendPacketCount

[OUT] 请求重发包数

### ￿ nResendPacketCount

int64_t nResendPacketCount

[OUT] 重发包数

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_OUTPUT_IMAGE_INFO结构体 参考

图像重构后的图像列表 更多...

### 成员变量

unsigned int nWidth [OUT] 源图像宽 更多...

unsigned int nHeight [OUT] 源图像高 更多...

enum MvGvspPixelType enPixelType [OUT] 像素格式 更多...

unsigned char * pBuf [IN][OUT] 输出数据缓存 更多...

unsigned int nBufLen [OUT] 输出数据长度 更多...

unsigned int nBufSize [IN] 提供的输出缓冲区大小 更多...

unsigned int nRes [8] 预留 更多...

### 详细描述

图像重构后的图像列表

### 结构体成员变量说明

### ￿ nWidth

unsigned int nWidth

[OUT] 源图像宽

### ￿ nHeight

unsigned int nHeight

[OUT] 源图像高

### ￿ enPixelType

enum MvGvspPixelType enPixelType

[OUT] 像素格式

### ￿ pBuf

unsigned char* pBuf

[IN][OUT] 输出数据缓存

示例 MultiLightCtrl_ImageStitching.cpp.

### ￿ nBufLen

unsigned int nBufLen

[OUT] 输出数据长度

### ￿ nBufSize

unsigned int nBufSize

[IN] 提供的输出缓冲区大小

示例 MultiLightCtrl_ImageStitching.cpp.

### ￿ nRes

unsigned int nRes[8]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_RECONSTRUCT_IMAGE_PARAM结构体 参考

重构图像参数信息 更多...

### 成员变量

unsigned int nWidth [IN] 源图像宽 更多...

unsigned int nHeight [IN] 源图像高 更多...

enum MvGvspPixelType enPixelType [IN] 像素格式 更多...

unsigned char * pSrcData [IN] 输入数据缓存 更多...

unsigned int nSrcDataLen

[IN] 输入数据长度 更多...

unsigned int nExposureNum [IN] 曝光个数(1-8] 更多...

MV_IMAGE_RECONSTRUCTION_METHOD enReconstructMethod [IN] 图像重构方式 更多...

MV_OUTPUT_IMAGE_INFO stDstBufList [MV_MAX_SPLIT_NUM] [OUT] 输出数据缓存信息 更多...

unsigned int nRes [4] 预留 更多...

### 详细描述

重构图像参数信息

示例 MultiLightCtrl_ImageStitching.cpp.

### 结构体成员变量说明

### ￿ nWidth

unsigned int nWidth

[IN] 源图像宽

示例 MultiLightCtrl_ImageStitching.cpp.

### ￿ nHeight

unsigned int nHeight

[IN] 源图像高

示例 MultiLightCtrl_ImageStitching.cpp.

### ￿ enPixelType

enum MvGvspPixelType enPixelType

[IN] 像素格式

示例 MultiLightCtrl_ImageStitching.cpp.

### ￿ pSrcData

unsigned char* pSrcData

[IN] 输入数据缓存

示例 MultiLightCtrl_ImageStitching.cpp.

### ￿ nSrcDataLen

unsigned int nSrcDataLen

[IN] 输入数据长度

示例 MultiLightCtrl_ImageStitching.cpp.

### ￿ nExposureNum

unsigned int nExposureNum

[IN] 曝光个数(1-8]

示例

MultiLightCtrl_ImageStitching.cpp.

### ￿ enReconstructMethod

MV_IMAGE_RECONSTRUCTION_METHOD enReconstructMethod

[IN] 图像重构方式

示例 MultiLightCtrl_ImageStitching.cpp.

### ￿ stDstBufList

MV_OUTPUT_IMAGE_INFO stDstBufList[MV_MAX_SPLIT_NUM]

[OUT] 输出数据缓存信息

示例 MultiLightCtrl_ImageStitching.cpp.

### ￿ nRes

unsigned int nRes[4]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_SAVE_IMAGE_PARAM_EX3结构体 参考

图片保存参数 更多...

### 成员变量

unsigned char * pData [IN] 输入数据缓存 更多...

unsigned int nDataLen [IN] 输入数据长度 更多...

enum MvGvspPixelType enPixelType [IN] 输入数据的像素格式 更多...

unsigned int nWidth [IN] 图像宽 更多...

unsigned int nHeight [IN] 图像高 更多...

unsigned char * pImageBuffer [OUT] 输出图片缓存 更多...

unsigned int nImageLen [OUT] 输出图片长度 更多...

unsigned int nBufferSize [IN] 提供的输出缓冲区大小 更多...

enum MV_SAVE_IAMGE_TYPE enImageType [IN] 输出图片格式 更多...

unsigned int nJpgQuality [IN] JPG编码质量(50-99]，其它格式无效 更多...

unsigned int iMethodValue [IN] 插值方法 0：快速 1：均衡 2：最优（其它值默认为最优） 3：最优+ 更多...

unsigned int nReserved [3] 预留 更多...

### 详细描述

图片保存参数

### 结构体成员变量说明

### ￿ pData

unsigned char* pData

[IN] 输入数据缓存

### ￿ nDataLen

unsigned int nDataLen

[IN] 输入数据长度

### ￿ enPixelType

enum MvGvspPixelType enPixelType

[IN] 输入数据的像素格式

### ￿ nWidth

unsigned int nWidth

[IN] 图像宽

### ￿ nHeight

unsigned int nHeight

[IN] 图像高

### ￿ pImageBuffer

unsigned char* pImageBuffer

[OUT] 输出图片缓存

### ￿ nImageLen

unsigned int nImageLen

[OUT] 输出图片长度

### ￿ nBufferSize

unsigned int nBufferSize

[IN] 提供的输出缓冲区大小

### ￿ enImageType

enum MV_SAVE_IAMGE_TYPE enImageType

[IN] 输出图片格式

### ￿ nJpgQuality

unsigned int nJpgQuality

[IN] JPG编码质量(50-99]，其它格式无效

### ￿ iMethodValue

unsigned int iMethodValue

[IN] 插值方法 0：快速 1：均衡 2：最优（其它值默认为最优） 3：最优+

### ￿ nReserved

unsigned int nReserved[3]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_SAVE_IMAGE_TO_FILE_PARAM_EX结构体 参考

保存图片到文件参数 更多...

### 成员变量

unsigned int nWidth [IN] 图像宽 更多...

unsigned int nHeight [IN] 图像高 更多...

MvGvspPixelType enPixelType [IN] 输入数据的像素格式 更多...

unsigned char * pData [IN] 输入数据缓存 更多...

unsigned int nDataLen [IN] 输入数据大小 更多...

MV_SAVE_IAMGE_TYPE enImageType [IN] 输入图片格式 更多...

char * pcImagePath [IN] 输入文件路径 更多...

unsigned int nQuality [IN] JPG编码质量(50-99]，其他格式无效 更多...

int iMethodValue [IN] 插值方法 0：快速 1：均衡 2：最优（其它值默认为最优） 3： 最优+ 更多...

unsigned int nReserved [8] 预留 更多...

### 详细描述

保存图片到文件参数

### 结构体成员变量说明

### ￿ nWidth

unsigned int nWidth

[IN] 图像宽

### ￿ nHeight

unsigned int nHeight

[IN] 图像高

### ￿ enPixelType

MvGvspPixelType enPixelType

[IN] 输入数据的像素格式

### ￿ pData

unsigned char* pData

[IN] 输入数据缓存

### ￿ nDataLen

unsigned int nDataLen

[IN] 输入数据大小

### ￿ enImageType

MV_SAVE_IAMGE_TYPE enImageType

[IN] 输入图片格式

### ￿ pcImagePath

char* pcImagePath

[IN] 输入文件路径

### ￿ nQuality

unsigned int nQuality

[IN] JPG编码质量(50-99]，其他格式无效

### ￿ iMethodValue

int iMethodValue

[IN] 插值方法 0：快速 1：均衡 2：最优（其它值默认为最优） 3：最优+

### ￿ nReserved

unsigned int nReserved[8]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_SAVE_POINT_CLOUD_PARAM结构体 参考

保存3D数据到缓存（自4.4.1版本废弃） 更多...

### 成员变量

unsigned int nLinePntNum [IN] 行点数，即图像宽 更多...

unsigned int nLineNum [IN] 行数，即图像高 更多...

enum MvGvspPixelType enSrcPixelType [IN] 输入数据的像素格式 更多...

unsigned char * pSrcData [IN] 输入数据缓存 更多...

unsigned int nSrcDataLen [IN] 输入数据长度 更多...

unsigned char * pDstBuf [OUT] 输出像素数据缓存 更多...

unsigned int nDstBufSize

[IN] 提供的输出缓冲区大小(nLinePntNum * nLineNum * (16*3 + 4) + 2048) 更多...

unsigned int nDstBufLen [OUT] 输出像素数据缓存长度 更多...

MV_SAVE_POINT_CLOUD_FILE_TYPE enPointCloudFileType [IN] 提供输出的点云文件类型 更多...

unsigned int nReserved [8] 保留字段 更多...

### 详细描述

保存3D数据到缓存（自4.4.1版本废弃）

### 结构体成员变量说明

### ￿ nLinePntNum

unsigned int nLinePntNum

[IN] 行点数，即图像宽

### ￿ nLineNum

unsigned int nLineNum

[IN] 行数，即图像高

### ￿ enSrcPixelType

enum MvGvspPixelType enSrcPixelType

[IN] 输入数据的像素格式

### ￿ pSrcData

unsigned char* pSrcData

[IN] 输入数据缓存

### ￿ nSrcDataLen

unsigned int nSrcDataLen

[IN] 输入数据长度

### ￿ pDstBuf

unsigned char* pDstBuf

[OUT] 输出像素数据缓存

### ￿ nDstBufSize

unsigned int nDstBufSize

[IN] 提供的输出缓冲区大小(nLinePntNum * nLineNum * (16*3 + 4) + 2048)

### ￿ nDstBufLen

unsigned int nDstBufLen

[OUT] 输出像素数据缓存长度

### ￿ enPointCloudFileType

MV_SAVE_POINT_CLOUD_FILE_TYPE enPointCloudFileType

[IN] 提供输出的点云文件类型

### ￿ nReserved

unsigned int nReserved[8]

保留字段

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_TRANSMISSION_TYPE结构体 参考

网络传输模式 更多...

### 成员变量

MV_GIGE_TRANSMISSION_TYPE enTransmissionType [IN] 传输模式 更多...

unsigned int nDestIp [IN] 目标IP，组播模式下有意义 更多...

unsigned short nDestPort [IN] 目标Port，组播模式下有意义 更多...

unsigned int nReserved [32] 预留 更多...

### 详细描述

网络传输模式

示例 DynamicallyLoadDLL.cpp , 以及 MultiCast.cpp.

### 结构体成员变量说明

### ￿ enTransmissionType

MV_GIGE_TRANSMISSION_TYPE enTransmissionType

[IN] 传输模式

示例 MultiCast.cpp.

### ￿ nDestIp

unsigned int nDestIp

[IN] 目标IP，组播模式下有意义

示例 MultiCast.cpp.

### ￿ nDestPort

unsigned short nDestPort

[IN] 目标Port，组播模式下有意义

示例 MultiCast.cpp.

### ￿ nReserved

unsigned int nReserved[32]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_USB3_DEVICE_INFO结构体 参考

USB设备信息 更多...

### 成员变量

unsigned char CrtlInEndPoint

[OUT] 控制输入端点 更多...

unsigned char CrtlOutEndPoint [OUT] 控制输出端点 更多...

unsigned char StreamEndPoint [OUT] 流端点 更多...

unsigned char EventEndPoint [OUT] 事件端点 更多...

unsigned short idVendor [OUT] 供应商ID号 更多...

unsigned short idProduct [OUT] 产品ID号 更多...

unsigned int nDeviceNumber [OUT] 设备索引号 更多...

unsigned char chDeviceGUID [INFO_MAX_BUFFER_SIZE] [OUT] 设备GUID号 更多...

unsigned char chVendorName [INFO_MAX_BUFFER_SIZE] [OUT] 供应商名字 更多...

unsigned char chModelName [INFO_MAX_BUFFER_SIZE] [OUT] 型号名字 更多...

unsigned char chFamilyName [INFO_MAX_BUFFER_SIZE] [OUT] 家族名字 更多...

unsigned char chDeviceVersion [INFO_MAX_BUFFER_SIZE] [OUT] 设备版本 更多...

unsigned char chManufacturerName [INFO_MAX_BUFFER_SIZE] [OUT] 制造商名字 更多...

unsigned char chSerialNumber [INFO_MAX_BUFFER_SIZE]

[OUT] 序列号 更多...

unsigned char chUserDefinedName [INFO_MAX_BUFFER_SIZE] [OUT] 用户自定义名字 更多...

unsigned int nbcdUSB [OUT] 支持的USB协议 更多...

unsigned int nDeviceAddress [OUT] 设备地址 更多...

unsigned int nReserved [2] 预留 更多...

### 详细描述

USB设备信息

### 结构体成员变量说明

### ￿ CrtlInEndPoint

unsigned char CrtlInEndPoint

[OUT] 控制输入端点

### ￿ CrtlOutEndPoint

unsigned char CrtlOutEndPoint

[OUT] 控制输出端点

### ￿ StreamEndPoint

unsigned char StreamEndPoint

[OUT] 流端点

### ￿ EventEndPoint

unsigned char EventEndPoint

[OUT] 事件端点

### ￿ idVendor

unsigned short idVendor

[OUT] 供应商ID号

### ￿ idProduct

unsigned short idProduct

[OUT] 产品ID号

### ￿ nDeviceNumber

unsigned int nDeviceNumber

[OUT] 设备索引号

示例 ChunkData.cpp, ConvertPixelType.cpp, DynamicallyLoadDLL.cpp, Events.cpp, Events_Interface.cpp, ForceIPEx.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, InterfaceAndDeviceDemo.cpp, LensShadingCorrection.cpp, MultiLightCtrl_ImageStitching.cpp, ParametrizeCamera_FileAccess.cpp, ParametrizeCamera_LineScanIOSettings.cpp, ParametrizeCamera_LoadAndSave.cpp , 以及 Recording.cpp.

### ￿ chDeviceGUID

unsigned char chDeviceGUID[INFO_MAX_BUFFER_SIZE]

[OUT] 设备GUID号

### ￿ chVendorName

unsigned char chVendorName[INFO_MAX_BUFFER_SIZE]

[OUT] 供应商名字

### ￿ chModelName

unsigned char chModelName[INFO_MAX_BUFFER_SIZE]

[OUT] 型号名字

### ￿ chFamilyName

unsigned char chFamilyName[INFO_MAX_BUFFER_SIZE]

[OUT] 家族名字

### ￿ chDeviceVersion

unsigned char chDeviceVersion[INFO_MAX_BUFFER_SIZE]

[OUT] 设备版本

### ￿ chManufacturerName

unsigned char chManufacturerName[INFO_MAX_BUFFER_SIZE]

[OUT] 制造商名字

### ￿ chSerialNumber

unsigned char chSerialNumber[INFO_MAX_BUFFER_SIZE]

[OUT] 序列号

示例 ChunkData.cpp, ConvertPixelType.cpp, DynamicallyLoadDLL.cpp, Events.cpp, Events_Interface.cpp, ForceIPEx.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, InterfaceAndDeviceDemo.cpp, LensShadingCorrection.cpp, MultiLightCtrl_ImageStitching.cpp, ParametrizeCamera_FileAccess.cpp, ParametrizeCamera_LineScanIOSettings.cpp, ParametrizeCamera_LoadAndSave.cpp , 以及 Recording.cpp.

### ￿ chUserDefinedName

unsigned char chUserDefinedName[INFO_MAX_BUFFER_SIZE]

[OUT] 用户自定义名字

示例 ChunkData.cpp, ConvertPixelType.cpp, DynamicallyLoadDLL.cpp, Events.cpp, Events_Interface.cpp, ForceIPEx.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, InterfaceAndDeviceDemo.cpp, LensShadingCorrection.cpp, MultiLightCtrl_ImageStitching.cpp, ParametrizeCamera_FileAccess.cpp, ParametrizeCamera_LineScanIOSettings.cpp, ParametrizeCamera_LoadAndSave.cpp , 以及 Recording.cpp.

### ￿ nbcdUSB

unsigned int nbcdUSB

[OUT] 支持的USB协议

### ￿ nDeviceAddress

unsigned int nDeviceAddress

[OUT] 设备地址

### ￿ nReserved

unsigned int nReserved[2]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MV_XOF_DEVICE_INFO结构体 参考

XoFLink相机信息 更多...

### 成员变量

unsigned char chInterfaceID [INFO_MAX_BUFFER_SIZE] 采集卡ID 更多...

unsigned char chVendorName [INFO_MAX_BUFFER_SIZE] 供应商名字 更多...

unsigned char chModelName [INFO_MAX_BUFFER_SIZE] 型号名字 更多...

unsigned char chManufacturerInfo [INFO_MAX_BUFFER_SIZE] 厂商信息 更多...

unsigned char chDeviceVersion [INFO_MAX_BUFFER_SIZE] 相机版本 更多...

unsigned char chSerialNumber [INFO_MAX_BUFFER_SIZE] 序列号 更多...

unsigned char chUserDefinedName [INFO_MAX_BUFFER_SIZE] 用户自定义名字 更多...

unsigned char chDeviceID [INFO_MAX_BUFFER_SIZE] 相机ID 更多...

unsigned int nReserved [7] 保留字段 更多...

### 详细描述

XoFLink相机信息

### 结构体成员变量说明

### ￿ chInterfaceID

unsigned char chInterfaceID[INFO_MAX_BUFFER_SIZE]

采集卡ID

示例 QuickSoftwareTrigger.cpp.

### ￿ chVendorName

unsigned char chVendorName[INFO_MAX_BUFFER_SIZE]

供应商名字

### ￿ chModelName

unsigned char chModelName[INFO_MAX_BUFFER_SIZE]

型号名字

示例 ChunkData.cpp, ConvertPixelType.cpp, DynamicallyLoadDLL.cpp, Events.cpp, Events_Interface.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, InterfaceAndDeviceDemo.cpp, LensShadingCorrection.cpp, MultiLightCtrl_ImageStitching.cpp, ParametrizeCamera_LineScanIOSettings.cpp, QuickSoftwareTrigger.cpp , 以及 Recording.cpp.

### ￿ chManufacturerInfo

unsigned char chManufacturerInfo[INFO_MAX_BUFFER_SIZE]

厂商信息

### ￿ chDeviceVersion

unsigned char chDeviceVersion[INFO_MAX_BUFFER_SIZE]

相机版本

### ￿ chSerialNumber

unsigned char chSerialNumber[INFO_MAX_BUFFER_SIZE]

序列号

示例 ChunkData.cpp, ConvertPixelType.cpp, DynamicallyLoadDLL.cpp, Events.cpp, Events_Interface.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, InterfaceAndDeviceDemo.cpp, LensShadingCorrection.cpp, MultiLightCtrl_ImageStitching.cpp, ParametrizeCamera_LineScanIOSettings.cpp, QuickSoftwareTrigger.cpp , 以及 Recording.cpp.

### ￿ chUserDefinedName

unsigned char chUserDefinedName[INFO_MAX_BUFFER_SIZE]

用户自定义名字

示例 ChunkData.cpp, ConvertPixelType.cpp, DynamicallyLoadDLL.cpp, Events.cpp, Events_Interface.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, InterfaceAndDeviceDemo.cpp, LensShadingCorrection.cpp, MultiLightCtrl_ImageStitching.cpp, ParametrizeCamera_LineScanIOSettings.cpp, QuickSoftwareTrigger.cpp , 以及 Recording.cpp.

### ￿ chDeviceID

unsigned char chDeviceID[INFO_MAX_BUFFER_SIZE]

相机ID

### ￿ nReserved

unsigned int nReserved[7]

保留字段

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MVCC_CIRCLE_INFO结构体 参考

圆形框区域信息 更多...

### 成员变量

MVCC_POINTF stCenterPoint [IN] 圆心信息 更多...

float fR1 [IN] 宽向半径，根据图像的相对位置[0, 1.0]，半径与圆心的位置有关，需保证 画出的圆在显示框范围之内，否则报错 更多...

float fR2 [IN] 高向半径，根据图像的相对位置[0, 1.0]，半径与圆心的位置有关，需保证 画出的圆在显示框范围之内，否则报错 更多...

MVCC_COLORF stColor [IN] 辅助线颜色信息 更多...

unsigned int nLineWidth [IN] 辅助线宽度，宽度只能是1或2 更多...

unsigned int nReserved [4] 预留 更多...

### 详细描述

圆形框区域信息

### 结构体成员变量说明

### ￿ stCenterPoint

MVCC_POINTF stCenterPoint

[IN] 圆心信息

### ￿ fR1

float fR1

[IN] 宽向半径，根据图像的相对位置[0, 1.0]，半径与圆心的位置有关，需保证画出的圆在显示 框范围之内，否则报错

### ￿ fR2

float fR2

[IN] 高向半径，根据图像的相对位置[0, 1.0]，半径与圆心的位置有关，需保证画出的圆在显示 框范围之内，否则报错

### ￿ stColor

MVCC_COLORF stColor

[IN] 辅助线颜色信息

### ￿ nLineWidth

unsigned int nLineWidth

[IN] 辅助线宽度，宽度只能是1或2

### ￿ nReserved

unsigned int nReserved[4]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MVCC_COLORF结构体 参考

辅助线颜色 更多...

### 成员变量

float fR [IN] 红色，根据像素颜色的相对深度，范围为[0.0 , 1.0]，代表着[0, 255]的颜色深度 更多...

float fG [IN] 绿色，根据像素颜色的相对深度，范围为[0.0 , 1.0]，代表着[0, 255]的颜色深度 更多...

float fB [IN] 蓝色，根据像素颜色的相对深度，范围为[0.0 , 1.0]，代表着[0, 255]的颜色深度 更多...

float fAlpha [IN] 透明度，根据像素颜色的相对透明度，范围为[0.0 , 1.0] 更多...

unsigned int nReserved [4] 预留 更多...

### 详细描述

辅助线颜色

### 结构体成员变量说明

### ￿ fR

float fR

[IN] 红色，根据像素颜色的相对深度，范围为[0.0 , 1.0]，代表着[0, 255]的颜色深度

### ￿ fG

float fG

[IN] 绿色，根据像素颜色的相对深度，范围为[0.0 , 1.0]，代表着[0, 255]的颜色深度

### ￿ fB

float fB

[IN] 蓝色，根据像素颜色的相对深度，范围为[0.0 , 1.0]，代表着[0, 255]的颜色深度

### ￿ fAlpha

float fAlpha

[IN] 透明度，根据像素颜色的相对透明度，范围为[0.0 , 1.0]

### ￿ nReserved

unsigned int nReserved[4]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MVCC_ENUMENTRY结构体 参考

枚举类型条目 更多...

### 成员变量

unsigned int nValue [IN] 指定值 更多...

char chSymbolic [MV_MAX_SYMBOLIC_LEN] [OUT] 指定值对应的符号 更多...

unsigned int nReserved [4] 预留 更多...

### 详细描述

枚举类型条目

示例 InterfaceDemo.cpp.

### 结构体成员变量说明

### ￿ nValue

unsigned int nValue

[IN] 指定值

示例 InterfaceDemo.cpp.

### ￿ chSymbolic

char chSymbolic[MV_MAX_SYMBOLIC_LEN]

[OUT] 指定值对应的符号

示例 InterfaceDemo.cpp.

### ￿ nReserved

unsigned int nReserved[4]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MVCC_ENUMVALUE结构体 参考

枚举类型值 更多...

### 成员变量

unsigned int nCurValue [OUT] 当前值 更多...

unsigned int nSupportedNum [OUT] 数据的有效数据个数 更多...

unsigned int nSupportValue [MV_MAX_XML_SYMBOLIC_NUM] [OUT] 支持的枚举值 更多...

unsigned int nReserved [4] 预留 更多...

### 详细描述

枚举类型值

示例

DynamicallyLoadDLL.cpp, InterfaceDemo.cpp , 以及 Recording.cpp.

### 结构体成员变量说明

### ￿ nCurValue

unsigned int nCurValue

[OUT] 当前值

示例 InterfaceDemo.cpp , 以及 Recording.cpp.

### ￿ nSupportedNum

unsigned int nSupportedNum

[OUT] 数据的有效数据个数

### ￿ nSupportValue

unsigned int nSupportValue[MV_MAX_XML_SYMBOLIC_NUM]

[OUT] 支持的枚举值

### ￿ nReserved

unsigned int nReserved[4]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MVCC_ENUMVALUE_EX结构体 参考

枚举类型值 更多...

### 成员变量

unsigned int nCurValue [OUT] 当前值 更多...

unsigned int nSupportedNum [OUT] 数据的有效数据个数 更多...

unsigned int nSupportValue [MV_MAX_ENUM_SYMBOLIC_NUM] [OUT] 支持的枚举值 更多...

unsigned int nReserved [4] 预留 更多...

### 详细描述

枚举类型值

### 结构体成员变量说明

### ￿ nCurValue

unsigned int nCurValue

[OUT] 当前值

### ￿ nSupportedNum

unsigned int nSupportedNum

[OUT] 数据的有效数据个数

### ￿ nSupportValue

unsigned int nSupportValue[MV_MAX_ENUM_SYMBOLIC_NUM]

[OUT] 支持的枚举值

### ￿ nReserved

unsigned int nReserved[4]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MVCC_FLOATVALUE结构体 参考

Float类型值 更多...

### 成员变量

float fCurValue [OUT] 当前值 更多...

float fMax [OUT] 最大值 更多...

float fMin [OUT] 最小值 更多...

unsigned int nReserved [4] 预留 更多...

### 详细描述

Float类型值

示例 DynamicallyLoadDLL.cpp, InterfaceDemo.cpp , 以及 Recording.cpp.

### 结构体成员变量说明

### ￿ fCurValue

float fCurValue

[OUT] 当前值

示例 InterfaceDemo.cpp , 以及 Recording.cpp.

### ￿ fMax

float fMax

[OUT] 最大值

### ￿ fMin

float fMin

[OUT] 最小值

### ￿ nReserved

unsigned int nReserved[4]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MVCC_INTVALUE结构体 参考

Int类型值 更多...

### 成员变量

unsigned int nCurValue [OUT] 当前值 更多...

unsigned int nMax [OUT] 最大值 更多...

unsigned int nMin [OUT] 最小值 更多...

unsigned int nInc [OUT] Inc 更多...

unsigned int nReserved [4] 预留 更多...

### 详细描述

Int类型值

示例 HighBandwidthDecode.cpp, ImageSave.cpp, MultiLightCtrl_ImageStitching.cpp , 以及 Recording.cpp.

### 结构体成员变量说明

### ￿ nCurValue

unsigned int nCurValue

[OUT] 当前值

示例 HighBandwidthDecode.cpp, ImageSave.cpp, MultiLightCtrl_ImageStitching.cpp , 以及 Recording.cpp.

### ￿ nMax

unsigned int nMax

[OUT] 最大值

### ￿ nMin

unsigned int nMin

[OUT] 最小值

### ￿ nInc

unsigned int nInc

[OUT] Inc

### ￿ nReserved

unsigned int nReserved[4]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MVCC_INTVALUE_EX结构体 参考

Int类型值Ex 更多...

### 成员变量

int64_t nCurValue [OUT] 当前值 更多...

int64_t nMax [OUT] 最大值 更多...

int64_t nMin

[OUT] 最小值 更多...

int64_t nInc [OUT] Inc 更多...

unsigned int nReserved [16] 预留 更多...

### 详细描述

Int类型值Ex

示例 DynamicallyLoadDLL.cpp, Grab_Asynchronous.cpp , 以及 InterfaceDemo.cpp.

### 结构体成员变量说明

### ￿ nCurValue

int64_t nCurValue

[OUT] 当前值

示例 Grab_Asynchronous.cpp , 以及 InterfaceDemo.cpp.

### ￿ nMax

int64_t nMax

[OUT] 最大值

### ￿ nMin

int64_t nMin

[OUT] 最小值

### ￿ nInc

int64_t nInc

[OUT] Inc

### ￿ nReserved

unsigned int nReserved[16]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MVCC_LINES_INFO结构体 参考

线条辅助线信息 更多...

### 成员变量

MVCC_POINTF stStartPoint [IN] 线条辅助线的起始点坐标 更多...

MVCC_POINTF stEndPoint [IN] 线条辅助线的终点坐标 更多...

MVCC_COLORF stColor [IN] 辅助线颜色信息 更多...

unsigned int nLineWidth [IN] 辅助线宽度，宽度只能是1或2 更多...

unsigned int nReserved [4] 预留 更多...

### 详细描述

线条辅助线信息

### 结构体成员变量说明

### ￿ stStartPoint

MVCC_POINTF stStartPoint

[IN] 线条辅助线的起始点坐标

### ￿ stEndPoint

MVCC_POINTF stEndPoint

[IN] 线条辅助线的终点坐标

### ￿ stColor

MVCC_COLORF stColor

[IN] 辅助线颜色信息

### ￿ nLineWidth

unsigned int nLineWidth

[IN] 辅助线宽度，宽度只能是1或2

### ￿ nReserved

unsigned int nReserved[4]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MVCC_NODE_ERROR结构体 参考

节点错误信息 更多...

### 成员变量

char strName [MV_MAX_NODE_NAME_LEN] 节点名称 更多...

MVCC_NODE_ERR_TYPE enErrType 错误类型 更多...

unsigned int nReserved [4] 预留 更多...

### 详细描述

节点错误信息

### 结构体成员变量说明

### ￿ strName

char strName[MV_MAX_NODE_NAME_LEN]

节点名称

### ￿ enErrType

MVCC_NODE_ERR_TYPE enErrType

错误类型

### ￿ nReserved

unsigned int nReserved[4]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MVCC_NODE_ERROR_LIST结构体 参考

节点错误信息列表 更多...

### 成员变量

unsigned int nErrorNum 错误个数 更多...

MVCC_NODE_ERROR stNodeError [MV_MAX_NODE_ERROR_NUM] 错误信息 更多...

unsigned int nReserved [4] 预留 更多...

### 详细描述

节点错误信息列表

### 结构体成员变量说明

### ￿ nErrorNum

unsigned int nErrorNum

错误个数

### ￿ stNodeError

MVCC_NODE_ERROR stNodeError[MV_MAX_NODE_ERROR_NUM]

错误信息

### ￿ nReserved

unsigned int nReserved[4]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MVCC_POINTF结构体 参考

自定义点 更多...

### 成员变量

float fX [IN] 该点距离图像左边缘距离，根据图像的相对位置，范围为[0.0 , 1.0] 更多...

float fY [IN] 该点距离图像上边缘距离，根据图像的相对位置，范围为[0.0 , 1.0] 更多...

unsigned int nReserved [4] 预留 更多...

### 详细描述

自定义点

### 结构体成员变量说明

### ￿ fX

float fX

[IN] 该点距离图像左边缘距离，根据图像的相对位置，范围为[0.0 , 1.0]

### ￿ fY

float fY

[IN] 该点距离图像上边缘距离，根据图像的相对位置，范围为[0.0 , 1.0]

### ￿ nReserved

unsigned int nReserved[4]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MVCC_RECT_INFO结构体 参考

矩形框区域信息 更多...

### 成员变量

float fTop [IN] 矩形上边缘距离图像上边缘的距离，根据图像的相对位置，范围为[0.0 , 1.0] 更多...

float fBottom [IN] 矩形下边缘距离图像上边缘的距离，根据图像的相对位置，范围为[0.0 , 1.0] 更多...

float fLeft [IN] 矩形左边缘距离图像左边缘的距离，根据图像的相对位置，范围为[0.0 , 1.0] 更多...

float fRight [IN] 矩形右边缘距离图像左边缘的距离，根据图像的相对位置，范围为[0.0 , 1.0] 更多...

MVCC_COLORF stColor [IN] 辅助线颜色 更多...

unsigned int nLineWidth [IN] 辅助线宽度，宽度只能是1或2 更多...

unsigned int nReserved [4] 预留 更多...

### 详细描述

矩形框区域信息

### 结构体成员变量说明

### ￿ fTop

float fTop

[IN] 矩形上边缘距离图像上边缘的距离，根据图像的相对位置，范围为[0.0 , 1.0]

### ￿ fBottom

float fBottom

[IN] 矩形下边缘距离图像上边缘的距离，根据图像的相对位置，范围为[0.0 , 1.0]

### ￿ fLeft

float fLeft

[IN] 矩形左边缘距离图像左边缘的距离，根据图像的相对位置，范围为[0.0 , 1.0]

### ￿ fRight

float fRight

[IN] 矩形右边缘距离图像左边缘的距离，根据图像的相对位置，范围为[0.0 , 1.0]

### ￿ stColor

MVCC_COLORF stColor

[IN] 辅助线颜色

### ￿ nLineWidth

unsigned int nLineWidth

[IN] 辅助线宽度，宽度只能是1或2

### ￿ nReserved

unsigned int nReserved[4]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

成员变量

### MVCC_STRINGVALUE结构体 参考

String类型值 更多...

### 成员变量

char chCurValue [256] [OUT] 当前值 更多...

int64_t nMaxLength [OUT] 最大长度 更多...

unsigned int nReserved [2] 预留 更多...

### 详细描述

String类型值

示例 DynamicallyLoadDLL.cpp , 以及 InterfaceDemo.cpp.

### 结构体成员变量说明

### ￿ chCurValue

char chCurValue[256]

[OUT] 当前值

示例 InterfaceDemo.cpp.

### ￿ nMaxLength

int64_t nMaxLength

[OUT] 最大长度

### ￿ nReserved

unsigned int nReserved[2]

预留

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

### 相机参数节点表

本表格中包含了大部分相机的所有节点以及每个节点的名称、数据类型、访问模式等。相机不是所 有的节点都支持，请以客户端中的参数列表为准。 相机的参数持续更新中，表中可能不包含相机最新固件的支持的参数。

访 名称（ _GetNode: key数据类问 功能 数值范围定义描述 参数）型 模 式

0:Transmitter IEnumer1:Receiver设备 DeviceType R ation2:Transceiver类型 Device3:Peripheral

Control设备 IEnumer0:Areascansens DeviceScanType R ation1:Linescan or 的 扫描

方式 ，表 明是 线阵 相机 还是 面阵 相机

设备 的制 DeviceVendorName IString 任意空终止字符串 R 造商 名称

设备 DeviceModelName IString 任意空终止字符串 R 型号

设备 的制 DeviceManufacturerInfo IString 任意空终止字符串 R 造商 信息

设备 DeviceVersion IString 任意空终止字符串 R 版本

固件 DeviceFirmwareVersion IString 任意空终止字符串 R 版本

设备 DeviceSerialNumber IString 任意空终止字符串 R序列 号

设备 DeviceID IString 任意空终止字符串 R ID

用户 R/自定 DeviceUserID IString 任意空终止字符串 W义的 名称

设备 DeviceUptime Iinteger ≥ 0 R运行 时间

设备 BoardDeviceType Iinteger ≥ 0 R 类型

设备 R/( DeviceConnectionSelector Iinteger ≥ 0 连接 W) 选择

设备 DeviceConnectionSpeed Iinteger ≥ 0 ，单位 Mbps R连接 速度

设备 R/( DeviceLinkSelector IInteger ≥ 0 连接 W) 选择

传输 DeviceLinkSpeed IInteger ≥ 0 R链路 速度

设备 DeviceLinkConnectionCou IInteger ≥ 0 R连接 nt 数量

是否 IEnumer0:OnR/ DeviceLinkHeartbeatMode 需要 ation1:OffW 心跳

流通 DeviceStreamChannelCou Iinteger ≥ 0 R道数 nt 量

流通 DeviceStreamChannelSeleR/( Iinteger ≥ 0 道选 ctor W) 择

流通 DeviceStreamChannelTypIenumer0:Transmitter R道类 eation1:Receiver 型

流通 道连 DeviceStreamChannelLink Iinteger ≥ 0 R 接数 量

图像 DeviceStreamChannelEndiIenumer0: Little数据 R annessation1: Big 的字 节序

接收 端流 DeviceStreamChannelPac与相机相关。一般范围R/( IInteger 数据 ketSize 在 220-9156，步进为8W) 的包 大小

设备 DeviceEventChannelCount IInteger ≥ 0 R支持 的

设备 寄存 IEnumer1:UTF-8器中 DeviceCharacterSet R ation2: ASCII 使用 的字 符集

IComma重启 DeviceReset - W nd 设备

设备 最大 吞吐 DeviceMaxThroughput Iinteger ≥ 0 R 量（ 带宽 ）

图像 最大 宽度 ， WidthMax IInteger ＞ 0 R binni ng 之 Image 后的 Format 数据 Control 图像 最大 高度 HeightMax IInteger ＞ 0 R ， binni ng 之

后的 数据

0:Region0 ROI IEnumer1:Region1R/( RegionSelector 选择 ation2:Region2W) 器 8:All

该 0:Stream0ROI IEnumerR/( RegionDestination 1:Stream1对应 ationW) 2:Stream2的码 流

R/(ROI Width IInteger ＞ 0 W)的宽

R/(ROI Height IInteger ＞ 0 W)的高

ROI 的水 R/ OffsetX IInteger ≥ 0 平方 W 向偏 移量

ROI 的竖 R/ OffsetY IInteger ≥ 0 直方 W 向偏 移量

转换 R/( ReverseScanDirection Iboolean ≥ 0 扫描 W) 方向

0x01080001:Mono8图像 0x01100003:Mono10像素 0x010C0004:Mono10格式 IEnumerPackedR/(，不 PixelFormat ation0x01100005:Mono12W)同型 0x010C0006:Mono12号的 Packed相机 0x01100007:Mono16，支

0x02180014:RGB8Pa持的 cked像素 0x02100032:YUV422_格式 8有差 0x0210001F:YUV422_异， 8_UYVY以实 0x01080008:BayerGR际的 8为准 0x01080009:BayerRG

0x0108000A:BayerGB

0x0108000B:BayerBG

0x0110000e:BayerGB

0x01100012:BayerGB

0x010C002C:BayerGB 12Packed

8:Bpp8 一个 10:Bpp10 像素 IEnumer12:Bpp12R/( PixelSize 包含 ation16:Bpp16W) 的比 24:Bpp24 特数 32:Bpp32

图像 IEnumer0:OffR/( ImageCompressionMode 压缩 ation1:JPEGW) 模式

图像 R/( ImageCompressionQuality Iinteger ≥ 50 压缩 W) 质量

测试 8:Sensor 图像 TestPatternGeneratorSeleIEnumer0:Region0R/( 生成 ctoration1:Region1W) 器选 2:Region2 择

0 :Off 1:Black 2:White 3:GreyHorizontalRamp 4:GreyVerticalRamp 5:GreyHorizontalRamp Moving 6:GreyVerticalRampM oving 测试 IEnumer7:HorizontalLineMovinR/( TestPattern 图像 ationgW) 选择 8:VerticalLineMoving 9:ColorBar 10:FrameCounter 11:MonoBar 12:TestImage12 13:TestImage13 14:ObliqueMonoBar 15:ObliqueColorBar 16:GradualMonoBar

0 :Timestamp 1:Gain 2:Exposure 3:BrightnessInfo水印 IEnumerR/( FrameSpecInfoSelector 4:WhiteBalance信息 ationW) 5:Framecounter选择 6:ExtTriggerCount 7:LineInputOutput 8:ROIPosition

是否 使能 TrueR/ FrameSpecInfo IBoolean 该水 FalseW 印信 息

采集 Acquisi0:SingleFrame IEnumerR/(模式 tionAcquisitionMode1:MultiFrame ationW)，单 Control2:Continuous 帧、

多帧 、连 续

IComma开始 AcquisitionStart - W nd 取流

IComma结束 AcquisitionStop - W nd 取流

一次 触发 AcquisitionBurstFrameCouR/ IInteger ≥ 0 采集 nt W 的帧 数

R/行频 AcquisitionLineRate IInteger ≥ 1 W设置

行频 TrueR/ AcquisitionLineRateEnable IBoolean 控制 FalseW 使能

实际 ResultingLineRate IInteger ≥ 0，单位 hz R 行频

相机 的实 ResultingFrameRate IFloat ≥ 0.0 ，单位 fps R际采 集帧 率

0:AcquisitionStart 1:AcquisitionEnd 2:AcquisitionActive 3:FrameStart 4:FrameEnd触发 IEnumerR/ TriggerSelector 5:FrameActive事件 ationW 6:FrameBurstStart选择 7:FrameBurstEnd 8:FrameBurstActive 9:LineStart 10:ExposureStart

11:ExposureEnd 12:ExposureActive

IEnumer0:OffR/触发 TriggerMode ation1:OnW模式

0:Line0 1:Line1 2:Line2 IEnumerR/触发 TriggerSource 3.Line3 ationW源 4:Counter0 7:Software 8:FrequencyConverter

触发 上升 沿、 0:RisingEdge 下降 IEnumer1:FallingEdgeR/ TriggerActivation 沿、 ation2:LevelHighW 高电 3:LevelLow 平、 低电 平等

R/触发 TriggerDelay IFloat ≥ 0.0 ，单位 us W延时

曝光 IEnumer0:TimedR/ ExposureMode 模式 ation1:TriggerWidthW 选择

R/曝光 ExposureTime IFloat ≥ 0.0 ，单位 us W时间

0:Off IEnumerR/自动 ExposureAuto 1:Once ationW曝光 2:Continuous

自动 AutoExposureTimeLowerLiR/(曝光 IInteger ≥ 0 ，单位 us mit W)时间 下限

AutoExposureTimeupperLiR/(自动 IInteger ≥ 0 ，单位 us mit W)曝光

时间 上限

帧超 0:OffR/ FrameTimeoutEnable IBoolean 时使 1:OnW 能

帧超 R/ FrameTimeoutTime Iinteger ≥ 87 ，单位 ms 时时 W 间

R/增益 Gain IFloat ≥ 0.0 ，单位 dB W值

0:Off IEnumerR/自动 GainAuto 1:Once ationW增益 2:Continuous

自动 R/增益 AutoGainLowerLimit IFloat ≥ 0.0 ，单位 dB W值下 限

自动 R/增益 AutoGainupperLimit IFloat ≥ 0.0 ，单位 dB W值上 限 Analog ADC Control0:OffR/ ADCGainEnable IBoolean 增益 1:OnW 使能

数字 DigitalShift IFloat ≥ 0.0 R偏移 调节

数字 0:OffR/ DigitalShiftEnable IBoolean 偏移 1:OnW 使能

R/ Brightness Iinteger ≥ 0 亮度 W

黑电 R/ BlackLevel IFloat ≥ 0.0 平调 W 节

黑电 0:OffR/平调 BlackLevelEnable IBoolean 1:OnW节使 能

0:Off自动 IEnumerR/ BalanceWhiteAuto 1:Continuous白平 ationW 2:Once衡

白平 0:Red Ienumer衡比 BalanceRatioSelector 1:GreenR ation例选 2:Blue 择

白平 BalanceRatio Iinteger ≥ 0 R 衡值

R/伽马 Gamma IFloat ＞ 0.0 W调节

Gam Ienumer1:UserR/ GammaSelector ma ation2:sRGBW 选择

Gam 0:OffR/ GammaEnable IBoolean ma 1:OnW 使能

色度 Hue Iinteger ≥ 0 R值调 节

0:OffR/色度 HueEnable Iboolean 1:OnW使能

饱和 Saturation Iinteger ≥ 0 R度值 调节

饱和 0:OffR/ SaturationEnable Iboolean 度使 1:OnW 能

自动 Ienumer0:AOI1R/ AutoFunctionAOISelector AOI ation1:AOI2W 选择

自动 R/ AutoFunctionAOIWidth Iinteger ≥ 0 AOI W 宽

自动 R/ AutoFunctionAOIHeight Iinteger ≥ 0 AOI W 高

自动 AOI AutoFunctionAOIOffsetX Iinteger ≥ 0 R水平 方向 偏移

根据 AOI AutoFunctionAOIUsageInt0:OffR/ Iboolean 区域 ensity 1:OnW 自动 曝光

根据 AOI AutoFunctionAOIUsageWh0:Off区域 Iboolean R iteBalance 1:On 自动 白平 衡

0:Luminance亮度 IEnumer1:RedR/、 LUTSelector ation2:GreenWR/G/ 3:BlueB

LUTTrueR/ LUTEnable IBoolean 使能 ControlFalseW

R/索引 LUTIndex IInteger ≥ 0 W号

R/ LUTValue IInteger Device-specific 值 W

Encode0:Encoder0编码 IEnumerR/ rEncoderSelector1:Encoder1器选 ationW Control2:Encoder2择

0:Line0编码 IEnumer1:Line1R/器 A EncoderSourceA ation2:Line2W源选 3:Line3择

0:Line0编码 IEnumer1:Line1R/器 B EncoderSourceB ation2:Line2W源选 3:Line3择

编码 IEnumer0:AnyDirectionR/器触 EncoderTriggerMode ation1:ForwardOnlyW发模 式

编码 IEnumer0:IgnoreDirectionR/器计 EncoderCounterMode ation1:FollowDirectionW数模 式

编码 器计 EncoderCounter Iinteger ≥ 0 R数器 值调 节

编码 器计 R/ EncoderCounterMax Iinteger ≥ 1 数器 W 最大 值

编码 IcommanR/器计 EncoderCounterReset - d W数器 复位

编码 器最 EncoderMaxReverseCountR/大反 Iinteger ≥ 1 er W转计 数器 值

编码 器反 EncoderReverseCounterRIcommanR/
- 转计 esetd W 数器 复位

0:Line0 分频 Ienumer1:Line1R/ InputSource 器输 ation2:Line2W 入源 3:Line3

分频 Ienumer0:RisingEdgeR/器信 SignalAlignment ation1:FallingEdgeW号方 Freque向

ncy前置 ConvertR/分频 PreDivider Iinteger ≥ 1 erW器调 Control节

倍频 R/ Multiplier Iinteger ≥ 1 器调 W 节

后置 R/分频 PostDivider Iinteger ≥ 1 W器调 节

明暗 IEnumer0:FPNCCorrectionR/场校 ShadingSelector ation1:PRNUCCorrectionW正选 择 Shadin ICommaR/(主动 gActivateShading - nd W)校正 Correct NUC ion 0:OffR/ NUCEnable IBoolean 使能 1:ONW 开关

0:OffR/PRN PRNUCEnable IBoolean 1:ONWUC

状态 开关

0:Line0 1:Line1 IEnumerR/I/O LineSelector 2:Line2 ationW选择 3:Line3 4:Line4

0:Input IEnumer1:OutputR/(I/O LineMode Digitalation2:TriggerW)模式 IO8:Strobe

ControlR/(I/O LineStatus IBoolean - W)状态

所有 LineStatusAll IInteger ≥ 0 RI/O 状态

I/O R/ LineDebouncerTime IInteger - 去抖 W 时间

一帧 数据 PayloadSize IInteger ≥ 0 R 的大 小

GEV TranspGevVersionMajor IInteger GEV 主版本号 R主版 ort本号

LayerGEV ControlGevVersionMinor IInteger GEV 副版本号 R子版 本号

GevDeviceModeIsBigEndi0:not BigEndian IBoolean R 大端 an 1:Is BigEndian

GevDeviceModeCharacterIEnumer字符 1:UTF8 R Setation 集

GEV R/( GevInterfaceSelector IInteger >=0 接口 W) 数

MAC GevMACAddress IInteger Mac 地址 R 地址

IP 是 GevCurrentIPConfiguration0:Off IBoolean R否为 LLA 1:On LLA

IP 是 GevCurrentIPConfiguration0:OffR/否为 IBoolean DHCP 1:OnWDHC P

IP 是 GevCurrentIPConfiguration0:OffR/否为 IBoolean PersistentIP 1:OnW静态 IP

IP 地 GevCurrentIPAddress Iinteger ip 地址 R 址

子网 GevCurrentSubnetMask Iinteger 子网掩码 R 掩码

GevCurrentDefaultGatewa默认 Iinteger 默认网关 R y 网关

XML 第一 GevFirstURL IString - R 选择 路径

XML 第二 GevSecondURL IString - R 选择 路径

GEV GevNumberOfInterfaces Iinteger ≥ 0 R接口 数

静态 R/ GevPersistentIPAddress Iinteger ≥ 0 IP 地 W 址

静态 R/ GevPersistentSubnetMask Iinteger ≥ 0 子网 W 掩码

静态 GevPersistentDefaultGateR/ Iinteger ≥ 0 默认 way W 网关

网络 GevLinkSpeed IInteger ≥ 0 R 速率

消息 GevMessageChannelCoun Iinteger ≥ 0 R通道 t 数

流通 GevStreamChannelCount Iinteger ≥ 0 R 道

心跳 R/ GevHeartbeatTimeout Iinteger ≥ 0 超时 W 时间

GevGVCPHeartbeatDisabl0:OffR/关闭 IBoolean e 1:OnW心跳

时间 GevTimestampTickFreque Iinteger ≥ 0 ，单位 hz R戳频 ncy 率

获取 GevTimestampControlLatcIComma
- W时间 hnd 戳

复位 GevTimestampControlResIComma
- W时间 etnd 戳

复位 时间 GevTimestampControlLatcIComma戳同
- W hResetnd 时获 取时 间戳

时间 GevTimestampValue Iinteger - R 戳值

App 0:OpenAcess IEnumerR/端的 GevCCP 1:ExclusiveAccess ationW控制 2:ControlAccess 权限

流通 GevStreamChannelSelectoR/ Iinteger >=0 道选 r W 择

GEV GevSCPInterfaceIndex Iinteger >=0 R接口 索引

R/(主机 GevSCPHostPort Iinteger >=0 W)端口

表明 流通 GevSCPDirection Iinteger >=0 R 道方 向

Fire Test 0:OffR/( GevSCPSFireTestPacket IBoolean Pack 1:OnW) et 使 能

Fire Test 0:OffR/ GevSCPSDoNotFragment IBoolean Pack 1:OnW et 使 能

流数 0:Off GevSCPSBigEndian IBoolean R据大 1:On 小端

＞ 0，与相机相关。一网络 R/ GevSCPSPacketSize IInteger般范围在 220-9156，包大 W 步进为 8小

R/发包 GevSCPD IInteger ≥ 0 W延时

流数 GevSCDA IInteger IP 地址 R 据的

目的 地址

流数 据的 GevSCSP IInteger 端口号 R 源端 口

当前 UserSetCurrent Iinteger >=0 R用户 参数

0:Default设置 IEnumer1:UserSet1R/载入 UserSetSelector ation2:UserSet2W的参 User3:UserSet3数

SetIComma UserSetLoad - W 加载 Controlnd

IComma UserSetSave - W 保存 nd

0:Default IEnumer1:UserSet1R/默认 UserSetDefault ation2:UserSet2W状态 3:UserSet3

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

### 示例程序

这里列出了所有示例:

- ChunkData.cpp
- ConnectSpecCamera.cpp
- ConvertPixelType.cpp
- DynamicallyLoadDLL.cpp
- Events.cpp
- Events_Interface.cpp
- ForceIPEx.cpp
- Grab_ActionCommand.cpp
- Grab_Asynchronous.cpp
- Grab_Callback.cpp
- GrabImage.cpp
- GrabImage_Display.cpp
- GrabStrategies.cpp
- HighBandwidthDecode.cpp
- ImageContrast.cpp
- ImageSave.cpp
- InterfaceAndDeviceDemo.cpp
- InterfaceDemo.cpp
- LensShadingCorrection.cpp
- MultiCast.cpp
- MultiLightCtrl_ImageStitching.cpp
- ParametrizeCamera_FileAccess.cpp
- ParametrizeCamera_LineScanIOSettings.cpp
- ParametrizeCamera_LoadAndSave.cpp
- QuickSoftwareTrigger.cpp
- Recording.cpp

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

### ChunkData.cpp

获取Chunk信息

该示例程序说明如何打开ChunkData功能、如何配置ChunkData以及如何获取这些ChunkData信息 。ChunkData例如Timestamp 、Exposure 等。

#include <stdio.h> #include <Windows.h> #include <process.h> #include <conio.h> #include "MvCameraControl.h"

// ch:等待按键输入 | en:Wait for key press void WaitForKeyPress(void) { while(!_kbhit()) { Sleep(10); } _getch(); }

bool PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo) {

if (NULL == pstMVDevInfo) { printf("The Pointer of pstMVDevInfo is NULL!\n"); return false; } if (pstMVDevInfo->nTLayerType == MV_GIGE_DEVICE) { int nIp1 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24); int nIp2 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16); int nIp3 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8); int nIp4 = (pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

// ch:打印当前相机ip和用户自定义名字 | en:print current ip and user defined name printf("CurrentIp: %d.%d.%d.%d\n" , nIp1, nIp2, nIp3, nIp4); printf("UserDefinedName: %s\n\n" , pstMVDevInfo- >SpecialInfo.stGigEInfo.chUserDefinedName); } else if (pstMVDevInfo->nTLayerType == MV_USB_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.chSerialNumber); printf("Device Number: %d\n\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.nDeviceNumber); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_GIGE_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_CAMERALINK_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_CXP_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chSerialNumber);

printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_XOF_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chModelName); } else { printf("Not support.\n"); }

return true; }

/*Get Image buffer function, you can get the chunk infomation from frame infomation*/ void __stdcall ImageCallBackEx(unsigned char * pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser) { if (pFrameInfo) { //ch:打印ChunkData信息 | en:Print parse the timestamp information in the frame printf("ImageCallBack：FrameNum[%d], ExposureTime[%f], SecondCount[%d], CycleCount[%d], CycleOffset[%d]\n", pFrameInfo->nFrameNum, pFrameInfo->fExposureTime, pFrameInfo->nSecondCount, pFrameInfo->nCycleCount, pFrameInfo- >nCycleOffset);

MV_CHUNK_DATA_CONTENT* pUnparsedChunkContent = pFrameInfo- >UnparsedChunkList.pUnparsedChunkContent; for(unsigned int i = 0;i < pFrameInfo->nUnparsedChunkNum; i++) { //ch：只打印ID和长度，内容需要根据说明文档进行解析 printf("ChunkInfo[%d]: ChunkID[0x%x], ChunkLen[%d]\n", i, pUnparsedChunkContent->nChunkID, pUnparsedChunkContent- >nChunkLen); pUnparsedChunkContent++; } printf("***********************************\n"); } }

int main() { int nRet = MV_OK;

void* handle = NULL;

do { // ch:初始化SDK | en:Initialize SDK nRet = MV_CC_Initialize(); if (MV_OK != nRet) { printf("Initialize SDK fail! nRet [0x%x]\n", nRet); break; }

// ch:枚举设备 | en:Enum device MV_CC_DEVICE_INFO_LIST stDeviceList; memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST)); nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE | MV_GENTL_CAMERALINK_DEVICE | MV_GENTL_CXP_DEVICE | MV_GENTL_XOF_DEVICE, &stDeviceList); if (MV_OK != nRet) { printf("Enum Devices fail! nRet [0x%x]\n", nRet); break; }

if (stDeviceList.nDeviceNum > 0) { for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++) { printf("[device %d]:\n", i); MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i]; if (NULL == pDeviceInfo) { break; } PrintDeviceInfo(pDeviceInfo); } } else { printf("Find No Devices!\n"); break; }

printf("Please Input camera index(0-%d):", stDeviceList.nDeviceNum-1); unsigned int nIndex = 0; scanf_s("%d", &nIndex);

if (nIndex >= stDeviceList.nDeviceNum) { printf("Input error!\n");

break; }

// ch:选择设备并创建句柄 | en:Select device and create handle nRet = MV_CC_CreateHandle(&handle, stDeviceList.pDeviceInfo[nIndex]); if (MV_OK != nRet) { printf("Create Handle fail! nRet [0x%x]\n", nRet); break; }

// ch:打开设备 | en:Open device nRet = MV_CC_OpenDevice(handle); if (MV_OK != nRet) { printf("Open Device fail! nRet [0x%x]\n", nRet); break; }

// ch:注册抓图回调 | en:Register image callback nRet = MV_CC_RegisterImageCallBackEx(handle, ImageCallBackEx, handle); if (MV_OK != nRet) { printf("Register Image CallBack fail! nRet [0x%x]\n", nRet); break; }

// ch:开启Chunk Mode | en:Open Chunk Mode nRet = MV_CC_SetBoolValue(handle, "ChunkModeActive", true); if (MV_OK != nRet) { printf("Set Chunk Mode fail! nRet [0x%x]\n", nRet); break; }

// ch:Chunk Selector设为Exposure | en: Chunk Selector set as Exposure nRet = MV_CC_SetEnumValueByString(handle, "ChunkSelector", "Exposure"); if (MV_OK != nRet) { printf("Set Exposure Chunk fail! nRet [0x%x]\n", nRet); break; }

// ch:开启Chunk Enable | en:Open Chunk Enable nRet = MV_CC_SetBoolValue(handle, "ChunkEnable", true); if (MV_OK != nRet)

{ printf("Set Chunk Enable fail! nRet [0x%x]\n", nRet); break; }

// ch:Chunk Selector设为Timestamp | en: Chunk Selector set as Timestamp nRet = MV_CC_SetEnumValueByString(handle, "ChunkSelector", "Timestamp"); if (MV_OK != nRet) { printf("Set Timestamp Chunk fail! nRet [0x%x]\n", nRet); break; }

// ch:开启Chunk Enable | en:Open Chunk Enable nRet = MV_CC_SetBoolValue(handle, "ChunkEnable", true); if (MV_OK != nRet) { printf("Set Chunk Enable fail! nRet [0x%x]\n", nRet); break; }

// ch:设置触发模式为off | en:Set trigger mode as off nRet = MV_CC_SetEnumValue(handle, "TriggerMode", MV_TRIGGER_MODE_OFF); if (MV_OK != nRet) { printf("Set Trigger Mode fail! nRet [0x%x]\n", nRet); break; }

// ch:开始取流 | en:Start grab image nRet = MV_CC_StartGrabbing(handle); if (MV_OK != nRet) { printf("Start Grabbing fail! nRet [0x%x]\n", nRet); break; }

printf("Press a key to stop grabbing.\n"); WaitForKeyPress();

// ch:停止取流 | en:Stop grab image nRet = MV_CC_StopGrabbing(handle); if (MV_OK != nRet) { printf("Stop Grabbing fail! nRet [0x%x]\n", nRet); break; }

// ch:关闭设备 | Close device nRet = MV_CC_CloseDevice(handle); if (MV_OK != nRet) { printf("ClosDevice fail! nRet [0x%x]\n", nRet); break; }

// ch:销毁句柄 | Destroy handle nRet = MV_CC_DestroyHandle(handle); if (MV_OK != nRet) { printf("Destroy Handle fail! nRet [0x%x]\n", nRet); break; } handle = NULL; } while (0);

if (handle != NULL) { MV_CC_DestroyHandle(handle); handle = NULL; }

// ch:反初始化SDK | en:Finalize SDK MV_CC_Finalize();

printf("Press a key to exit.\n"); WaitForKeyPress();

return 0; }

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

### ConnectSpecCamera.cpp

通过IP地址连接相机

该示例程序说明如何通过IP地址连接网口相机。通过输入需要连接的相机IP（Camera Ip）和相机 对应的网卡IP（Export Ip）来连接相机，输入的格式为xx.xx.xx.xx。

#include <stdio.h> #include <Windows.h> #include <process.h> #include <conio.h> #include "MvCameraControl.h"

bool g_bExit = false;

// ch:等待按键输入 | en:Wait for key press void WaitForKeyPress(void) { while(!_kbhit()) { Sleep(10); } _getch(); }

static unsigned int __stdcall WorkThread(void* pUser) { int nRet = MV_OK; MV_FRAME_OUT stImageInfo = {0};

while(1) { nRet = MV_CC_GetImageBuffer(pUser, &stImageInfo, 1000); if (nRet == MV_OK) { printf("Get Image Buffer: Width[%d], Height[%d], FrameNum[%d]\n",

stImageInfo.stFrameInfo.nWidth, stImageInfo.stFrameInfo.nHeight, stImageInfo.stFrameInfo.nFrameNum);

nRet = MV_CC_FreeImageBuffer(pUser, &stImageInfo); if(nRet != MV_OK) { printf("Free Image Buffer fail! nRet [0x%x]\n", nRet); } } else { printf("Get Image fail! nRet [0x%x]\n", nRet); } if(g_bExit) { break; } }

return 0; }

int main() { int nRet = MV_OK; void* handle = NULL; MV_CC_DEVICE_INFO stDevInfo = {0}; MV_GIGE_DEVICE_INFO stGigEDev = {0};

// ch:需要连接的相机ip(根据实际填充) | en:The camera IP that needs to be connected (based on actual padding) printf("Please input Current Camera Ip : "); char nCurrentIp[128]; scanf("%s", &nCurrentIp); // ch:相机对应的网卡ip(根据实际填充) | en:The pc IP that needs to be connected (based on actual padding) printf("Please input Net Export Ip : "); char nNetExport[128]; scanf("%s", &nNetExport); unsigned int nIp1, nIp2, nIp3, nIp4, nIp;

sscanf_s(nCurrentIp, "%d.%d.%d.%d", &nIp1, &nIp2, &nIp3, &nIp4); nIp = (nIp1 << 24) | (nIp2 << 16) | (nIp3 << 8) | nIp4; stGigEDev.nCurrentIp = nIp;

sscanf_s(nNetExport, "%d.%d.%d.%d", &nIp1, &nIp2, &nIp3, &nIp4); nIp = (nIp1 << 24) | (nIp2 << 16) | (nIp3 << 8) | nIp4; stGigEDev.nNetExport = nIp;

stDevInfo.nTLayerType = MV_GIGE_DEVICE;// ch:仅支持GigE相机 | en:Only support GigE camera stDevInfo.SpecialInfo.stGigEInfo = stGigEDev;

do { // ch:初始化SDK | en:Initialize SDK nRet = MV_CC_Initialize(); if (MV_OK != nRet) { printf("Initialize SDK fail! nRet [0x%x]\n", nRet); break; }

// ch:选择设备并创建句柄 | en:Select device and create handle nRet = MV_CC_CreateHandle(&handle, &stDevInfo); if (MV_OK != nRet) { printf("Create Handle fail! nRet[0x%x]\n", nRet); break; }

// ch:打开设备 | en:Open device nRet = MV_CC_OpenDevice(handle); if (MV_OK != nRet) { printf("Open Device fail! nRet [0x%x]\n", nRet); break; }

// ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera) if (stDevInfo.nTLayerType == MV_GIGE_DEVICE) { int nPacketSize = MV_CC_GetOptimalPacketSize(handle); if (nPacketSize > 0) { nRet = MV_CC_SetIntValueEx(handle,"GevSCPSPacketSize",nPacketSize); if(nRet != MV_OK) { printf("Warning: Set Packet Size fail nRet [0x%x]!", nRet); } } else { printf("Warning: Get Packet Size fail nRet [0x%x]!", nPacketSize); } }

// ch:设置触发模式为off | en:Set trigger mode as off nRet = MV_CC_SetEnumValue(handle, "TriggerMode", MV_TRIGGER_MODE_OFF); if (MV_OK != nRet) { printf("Set Trigger Mode fail! nRet [0x%x]\n", nRet); break; }

// ch:开始取流 | en:Start grab image nRet = MV_CC_StartGrabbing(handle); if (MV_OK != nRet) { printf("Start Grabbing fail! nRet [0x%x]\n", nRet); break; }

unsigned int nThreadID = 0; void* hThreadHandle = (void*) _beginthreadex( NULL , 0 , WorkThread , handle, 0 , &nThreadID ); if (NULL == hThreadHandle) { break; }

printf("Press a key to stop grabbing.\n"); WaitForKeyPress();

g_bExit = true; Sleep(1000);

// ch:停止取流 | en:Stop grab image nRet = MV_CC_StopGrabbing(handle); if (MV_OK != nRet) { printf("Stop Grabbing fail! nRet [0x%x]\n", nRet); break; }

// ch:关闭设备 | en:Close device nRet = MV_CC_CloseDevice(handle); if (MV_OK != nRet) { printf("Close Device fail! nRet [0x%x]\n", nRet); break; }

// ch:销毁句柄 | en:Destroy handle

nRet = MV_CC_DestroyHandle(handle); if (MV_OK != nRet) { printf("Destroy Handle fail! nRet [0x%x]\n", nRet); break; } handle = NULL; } while (0);

if (handle != NULL) { MV_CC_DestroyHandle(handle); handle = NULL; }

// ch:反初始化SDK | en:Finalize SDK MV_CC_Finalize();

printf("Press a key to exit.\n"); WaitForKeyPress();

return 0; }

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

### ConvertPixelType.cpp

图像格式转换

该示例程序通过图像转换接口将相机取图的图像格式转换为用户需要的格式。

#include <stdio.h> #include <Windows.h> #include <conio.h> #include "MvCameraControl.h"

// ch:等待按键输入 | en:Wait for key press void WaitForKeyPress(void) { while(!_kbhit()) { Sleep(10); } _getch(); }

bool PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo) { if (NULL == pstMVDevInfo) { printf("The Pointer of pstMVDevInfo is NULL!\n"); return false; } if (pstMVDevInfo->nTLayerType == MV_GIGE_DEVICE) { int nIp1 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24); int nIp2 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16); int nIp3 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8); int nIp4 = (pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

// ch:打印当前相机ip和用户自定义名字 | en:print current ip and user defined name printf("CurrentIp: %d.%d.%d.%d\n" , nIp1, nIp2, nIp3, nIp4); printf("UserDefinedName: %s\n\n" , pstMVDevInfo- >SpecialInfo.stGigEInfo.chUserDefinedName); } else if (pstMVDevInfo->nTLayerType == MV_USB_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.chUserDefinedName);

printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.chSerialNumber); printf("Device Number: %d\n\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.nDeviceNumber); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_GIGE_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chSerialNumber); printf("Model Name %s\n\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_CAMERALINK_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_CXP_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_XOF_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chModelName); } else { printf("Not support.\n"); }

return true; }

bool IsColor(MvGvspPixelType enType) { switch(enType) { case PixelType_Gvsp_BGR8_Packed: case PixelType_Gvsp_YUV422_Packed:

case PixelType_Gvsp_YUV422_YUYV_Packed: case PixelType_Gvsp_BayerGR8: case PixelType_Gvsp_BayerRG8: case PixelType_Gvsp_BayerGB8: case PixelType_Gvsp_BayerBG8: case PixelType_Gvsp_BayerGB10: case PixelType_Gvsp_BayerGB10_Packed: case PixelType_Gvsp_BayerBG10: case PixelType_Gvsp_BayerBG10_Packed: case PixelType_Gvsp_BayerRG10: case PixelType_Gvsp_BayerRG10_Packed: case PixelType_Gvsp_BayerGR10: case PixelType_Gvsp_BayerGR10_Packed: case PixelType_Gvsp_BayerGB12: case PixelType_Gvsp_BayerGB12_Packed: case PixelType_Gvsp_BayerBG12: case PixelType_Gvsp_BayerBG12_Packed: case PixelType_Gvsp_BayerRG12: case PixelType_Gvsp_BayerRG12_Packed: case PixelType_Gvsp_BayerGR12: case PixelType_Gvsp_BayerGR12_Packed: case PixelType_Gvsp_BayerRBGG8: case PixelType_Gvsp_BayerGR16: case PixelType_Gvsp_BayerRG16: case PixelType_Gvsp_BayerGB16: case PixelType_Gvsp_BayerBG16: return true; default: return false; } }

bool IsMono(MvGvspPixelType enType) { switch(enType) { case PixelType_Gvsp_Mono10: case PixelType_Gvsp_Mono10_Packed: case PixelType_Gvsp_Mono12: case PixelType_Gvsp_Mono12_Packed: case PixelType_Gvsp_Mono14: case PixelType_Gvsp_Mono16: return true; default: return false; } }

int main() { int nRet = MV_OK; void* handle = NULL; unsigned char *pConvertData = NULL; unsigned int nConvertDataSize = 0;

do { // ch:初始化SDK | en:Initialize SDK nRet = MV_CC_Initialize(); if (MV_OK != nRet) { printf("Initialize SDK fail! nRet [0x%x]\n", nRet); break; }

// ch:枚举设备 | en:Enum device MV_CC_DEVICE_INFO_LIST stDeviceList; memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST)); nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE | MV_GENTL_CAMERALINK_DEVICE | MV_GENTL_CXP_DEVICE | MV_GENTL_XOF_DEVICE, &stDeviceList); if (MV_OK != nRet) { printf("Enum Devices fail! nRet [0x%x]\n", nRet); break; }

if (stDeviceList.nDeviceNum > 0) { for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++) { printf("[device %d]:\n", i); MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i]; if (NULL == pDeviceInfo) { break; } PrintDeviceInfo(pDeviceInfo); } } else { printf("Find No Devices!\n"); break; }

printf("Please Input camera index(0-%d):", stDeviceList.nDeviceNum-1); unsigned int nIndex = 0; scanf_s("%d", &nIndex);

if (nIndex >= stDeviceList.nDeviceNum) { printf("Input error!\n"); break; }

// ch:选择设备并创建句柄 | en:Select device and create handle nRet = MV_CC_CreateHandle(&handle, stDeviceList.pDeviceInfo[nIndex]); if (MV_OK != nRet) { printf("Create Handle fail! nRet [0x%x]\n", nRet); break; }

// ch:打开设备 | en:Open device nRet = MV_CC_OpenDevice(handle); if (MV_OK != nRet) { printf("Open Device fail! nRet [0x%x]\n", nRet); break; }

// ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera) if (stDeviceList.pDeviceInfo[nIndex]->nTLayerType == MV_GIGE_DEVICE) { int nPacketSize = MV_CC_GetOptimalPacketSize(handle); if (nPacketSize > 0) { nRet = MV_CC_SetIntValueEx(handle,"GevSCPSPacketSize",nPacketSize); if(nRet != MV_OK) { printf("Warning: Set Packet Size fail nRet [0x%x]!", nRet); } } else { printf("Warning: Get Packet Size fail nRet [0x%x]!", nPacketSize); } }

nRet = MV_CC_SetEnumValue(handle, "TriggerMode", MV_TRIGGER_MODE_OFF); if (MV_OK != nRet) { printf("Set Trigger Mode fail! nRet [0x%x]\n", nRet); break; }

// ch:开始取流 | en:Start grab image nRet = MV_CC_StartGrabbing(handle);

if (MV_OK != nRet) { printf("Start Grabbing fail! nRet [0x%x]\n", nRet); break; }

MV_FRAME_OUT stImageInfo = {0};

nRet = MV_CC_GetImageBuffer(handle, &stImageInfo, 1000); if (nRet == MV_OK) { printf("Get One Frame: Width[%d], Height[%d], nFrameNum[%d]\n", stImageInfo.stFrameInfo.nWidth, stImageInfo.stFrameInfo.nHeight, stImageInfo.stFrameInfo.nFrameNum);

MvGvspPixelType enDstPixelType = PixelType_Gvsp_Undefined; unsigned int nChannelNum = 0; char chFileName[MAX_PATH] = {0}; // ch:如果是彩色则转成RGB8 | en:if pixel type is color, convert it to RGB8 if (IsColor(stImageInfo.stFrameInfo.enPixelType)) { nChannelNum = 3; enDstPixelType = PixelType_Gvsp_RGB8_Packed; sprintf(chFileName, "AfterConvertRGB.raw"); } // ch:如果是黑白则转换成Mono8 | en:if pixel type is mono, convert it to mono8 else if (IsMono(stImageInfo.stFrameInfo.enPixelType)) { nChannelNum = 1; enDstPixelType = PixelType_Gvsp_Mono8; sprintf(chFileName, "AfterConvertMono8.raw"); } else { printf("Don't need to convert!\n"); }

if (enDstPixelType != PixelType_Gvsp_Undefined) { // ch:设置插值算法为均衡 | en:set interpolation algorithm type, 0-Fast 1-Equilibrium 2-Optimal 3-Optimal plus nRet = MV_CC_SetBayerCvtQuality(handle, 1); if (MV_OK != nRet) { printf("set Bayer convert quality fail! nRet [0x%x]\n", nRet); break; }

pConvertData = (unsigned char*)malloc(stImageInfo.stFrameInfo.nWidth * stImageInfo.stFrameInfo.nHeight * nChannelNum); if (NULL == pConvertData) { printf("malloc pConvertData fail!\n"); break; } nConvertDataSize = stImageInfo.stFrameInfo.nWidth * stImageInfo.stFrameInfo.nHeight * nChannelNum;

// ch:像素格式转换 | en:Convert pixel format MV_CC_PIXEL_CONVERT_PARAM_EX stConvertParam = {0};

stConvertParam.nWidth = stImageInfo.stFrameInfo.nWidth; //ch:图像宽 | en:image width stConvertParam.nHeight = stImageInfo.stFrameInfo.nHeight; //ch:图像高 | en:image height stConvertParam.pSrcData = stImageInfo.pBufAddr; //ch:输入数据缓存 | en:input data buffer stConvertParam.nSrcDataLen = stImageInfo.stFrameInfo.nFrameLen; //ch:输入数据大小 | en:input data size stConvertParam.enSrcPixelType = stImageInfo.stFrameInfo.enPixelType; //ch:输入像素格式 | en:input pixel format stConvertParam.enDstPixelType = enDstPixelType; //ch:输出像素格式 | en:output pixel format stConvertParam.pDstBuffer = pConvertData; //ch:输出数据缓存 | en:output data buffer stConvertParam.nDstBufferSize = nConvertDataSize; //ch:输出缓存大小 | en:output buffer size nRet = MV_CC_ConvertPixelTypeEx(handle, &stConvertParam); if (MV_OK != nRet) { printf("Convert Pixel Type fail! nRet [0x%x]\n", nRet); break; }

FILE* fp = NULL; errno_t err = fopen_s(&fp, chFileName, "wb"); if (0 != err || NULL == fp) { printf("Open file failed\n"); break; } fwrite(stConvertParam.pDstBuffer, 1, stConvertParam.nDstLen, fp);

fclose(fp); printf("Convert pixeltype succeed\n"); } MV_CC_FreeImageBuffer(handle, &stImageInfo); } else { printf("Get Image fail! nRet [0x%x]\n", nRet); }

// ch:停止取流 | en:Stop grab image nRet = MV_CC_StopGrabbing(handle); if (MV_OK != nRet) { printf("Stop Grabbing fail! nRet [0x%x]\n", nRet); break; }

// ch:关闭设备 | en:Close device nRet = MV_CC_CloseDevice(handle); if (MV_OK != nRet) { printf("Close Device fail! nRet [0x%x]\n", nRet); break; }

// ch:销毁句柄 | en:Destroy handle nRet = MV_CC_DestroyHandle(handle); if (MV_OK != nRet) { printf("Destroy Handle fail! nRet [0x%x]\n", nRet); break; } handle = NULL; } while (0);

if (pConvertData) { free(pConvertData); pConvertData = NULL; }

if (handle != NULL) { MV_CC_DestroyHandle(handle); handle = NULL; }

// ch:反初始化SDK | en:Finalize SDK MV_CC_Finalize();

printf("Press a key to exit.\n"); WaitForKeyPress();

return 0; }

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

### DynamicallyLoadDLL.cpp

动态调用

该示例程序说明按照动态调用的方式演示使用工业相机SDK。

#include <stdio.h> #include <Windows.h> #include <process.h> #include <conio.h> #include "CameraParams.h" #include "ObsoleteCamParams.h" #include "MvErrorDefine.h"

typedef int (__stdcall * DLL_Initialize) ();

typedef int (__stdcall * DLL_Finalize) (); typedef unsigned int (__stdcall * DLL_GetSDKVersion) (); typedef int (__stdcall * DLL_EnumerateTls) (); typedef int (__stdcall * DLL_EnumDevices) (IN unsigned int nTLayerType, IN OUT MV_CC_DEVICE_INFO_LIST * pstDevList); typedef int (__stdcall * DLL_EnumDevicesEx) (IN unsigned int nTLayerType, IN OUT MV_CC_DEVICE_INFO_LIST* pstDevList, IN const char* pManufacturerName); typedef bool (__stdcall * DLL_IsDeviceAccessible) (IN MV_CC_DEVICE_INFO* pstDevInfo, IN unsigned int nAccessMode); typedef int (__stdcall * DLL_CreateHandle) (OUT void ** handle, IN const MV_CC_DEVICE_INFO* pstDevInfo); typedef int (__stdcall * DLL_CreateHandleWithoutLog)(OUT void ** handle, IN const MV_CC_DEVICE_INFO* pstDevInfo); typedef int (__stdcall * DLL_DestroyHandle) (IN void
* handle); typedef int (__stdcall * DLL_OpenDevice) (IN void* handle, IN unsigned int nAccessMode, IN unsigned short nSwitchoverKey); typedef int (__stdcall * DLL_CloseDevice) (IN void* handle); typedef int (__stdcall * DLL_RegisterImageCallBackEx)(void* handle, void(__stdcall* cbOutput)(unsigned char * pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser),void* pUser); typedef int (__stdcall * DLL_RegisterImageCallBackForRGB)(void* handle, void(__stdcall* cbOutput)(unsigned char * pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser), void* pUser); typedef int (__stdcall * DLL_RegisterImageCallBackForBGR)(void* handle, void(__stdcall* cbOutput)(unsigned char * pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser), void* pUser); typedef int (__stdcall * DLL_StartGrabbing) (IN void* handle); typedef int (__stdcall * DLL_StopGrabbing) (IN void* handle); typedef int (__stdcall * DLL_GetImageForRGB) (IN void* handle, IN OUT unsigned char * pData , IN unsigned int nDataSize, IN OUT MV_FRAME_OUT_INFO_EX* pFrameInfo, int nMsec); typedef int (__stdcall * DLL_GetImageForBGR) (IN void* handle, IN OUT unsigned char * pData , IN unsigned int nDataSize, IN OUT MV_FRAME_OUT_INFO_EX* pFrameInfo, int nMsec); typedef int (__stdcall * DLL_GetImageBuffer) (IN void* handle, MV_FRAME_OUT* pFrameInfo, int nMsec); typedef int (__stdcall * DLL_FreeImageBuffer) (IN void* handle, MV_FRAME_OUT* pFrameInfo); typedef int (__stdcall * DLL_GetOneFrameTimeout) (IN void* handle, IN OUT unsigned char * pData , IN unsigned int nDataSize, IN OUT MV_FRAME_OUT_INFO_EX* pFrameInfo, int nMsec); typedef int (__stdcall * DLL_Display) (IN void* handle, void* hWnd); typedef int (__stdcall * DLL_DisplayOneFrame) (IN void* handle, IN MV_DISPLAY_FRAME_INFO* pDisplayInfo); typedef int (__stdcall * DLL_SetImageNodeNum) (IN void* handle, unsigned int nNum);

typedef int (__stdcall * DLL_GetDeviceInfo) (IN void
* handle, IN OUT MV_CC_DEVICE_INFO* pstDevInfo); typedef int (__stdcall * DLL_GetAllMatchInfo) (IN void* handle, IN OUT MV_ALL_MATCH_INFO* pstInfo);

typedef int (__stdcall * DLL_GetIntValueEx) (IN void* handle,IN const char* strKey,OUT MVCC_INTVALUE_EX *pIntValue); typedef int (__stdcall * DLL_SetIntValueEx) (IN void* handle,IN const char* strKey,IN int64_t nValue); typedef int (__stdcall * DLL_GetEnumValue) (IN void* handle,IN const char* strValue,OUT MVCC_ENUMVALUE *pEnumValue); typedef int (__stdcall * DLL_SetEnumValue) (IN void* handle,IN const char* strValue,IN unsigned int nValue); typedef int (__stdcall * DLL_SetEnumValueByString) (IN void* handle,IN const char* strValue,IN const char* sValue); typedef int (__stdcall * DLL_GetFloatValue) (IN void* handle,IN const char* strValue,OUT MVCC_FLOATVALUE *pFloatValue); typedef int (__stdcall * DLL_SetFloatValue) (IN void* handle,IN const char* strValue,IN float fValue); typedef int (__stdcall * DLL_GetBoolValue) (IN void* handle,IN const char* strValue,OUT bool *pBoolValue); typedef int (__stdcall * DLL_SetBoolValue) (IN void* handle,IN const char* strValue,IN bool bValue); typedef int (__stdcall * DLL_GetStringValue) (IN void* handle,IN const char* strKey,OUT MVCC_STRINGVALUE *pStringValue); typedef int (__stdcall * DLL_SetStringValue) (IN void* handle,IN const char* strKey,IN const char * sValue); typedef int (__stdcall * DLL_SetCommandValue) (IN void* handle,IN const char* strValue); typedef int (__stdcall * DLL_LocalUpgrade) (IN void* handle, const void *pFilePathName); typedef int (__stdcall * DLL_GetUpgradeProcess) (IN void* handle, unsigned int* pnProcess); typedef int (__stdcall * DLL_GetOptimalPacketSize) (IN void* handle); typedef int (__stdcall * DLL_ReadMemory) (IN void* handle , void *pBuffer, int64_t nAddress, int64_t nLength); typedef int (__stdcall * DLL_WriteMemory) (IN void* handle , const void *pBuffer, int64_t nAddress, int64_t nLength); typedef int (__stdcall * DLL_RegisterExceptionCallBack)(IN void* handle, void(__stdcall* cbException)(unsigned int nMsgType, void* pUser), void* pUser); typedef int (__stdcall * DLL_RegisterEventCallBackEx)(void* handle, const char* pEventName, void(__stdcall* cbEvent)(MV_EVENT_OUT_INFO * pEventInfo, void* pUser),void* pUser); typedef int (__stdcall * DLL_RegisterAllEventCallBack)(void* handle, void(__stdcall* cbEvent)(MV_EVENT_OUT_INFO * pEventInfo, void* pUser),void* pUser);

typedef int (__stdcall * DLL_ForceIpEx) (IN void* handle, unsigned int nIP, unsigned int nSubNetMask, unsigned int nDefaultGateWay); typedef int (__stdcall * DLL_SetIpConfig) (IN void* handle, unsigned int nType); typedef int (__stdcall * DLL_SetNetTransMode) (IN void* handle, unsigned int nType); typedef int (__stdcall * DLL_GetNetTransInfo) (IN void* handle, MV_NETTRANS_INFO* pstInfo); typedef int (__stdcall * DLL_SetGvcpTimeout) (IN void* handle, unsigned int nMillisec); typedef int (__stdcall * DLL_SetResend) (IN void* handle, unsigned int bEnable, unsigned int nMaxResendPercent, unsigned int nResendTimeout); typedef int (__stdcall * DLL_SetTransmissionType) (IN void* handle, MV_TRANSMISSION_TYPE * pstTransmissionType); typedef int (__stdcall * DLL_IssueActionCommand) (IN MV_ACTION_CMD_INFO* pstActionCmdInfo, OUT MV_ACTION_CMD_RESULT_LIST* pstActionCmdResults);

typedef int (__stdcall * DLL_SetDeviceBauderate) (IN void* handle, unsigned int nBaudrate); typedef int (__stdcall * DLL_GetDeviceBauderate) (IN void* handle,unsigned int* pnCurrentBaudrate); typedef int (__stdcall * DLL_GetSupportBauderates) (IN void* handle,unsigned int* pnBaudrateAblity); typedef int (__stdcall * DLL_SetGenCPTimeOut) (IN void* handle, unsigned int nMillisec);

typedef int (__stdcall * DLL_GetGenICamXML) (IN void* handle, IN OUT unsigned char* pData, IN unsigned int nDataSize, OUT unsigned int* pnDataLen); typedef int (__stdcall * DLL_SaveImageEx2) (IN void* handle, IN OUT MV_SAVE_IMAGE_PARAM_EX* pSaveParam); typedef int (__stdcall * DLL_ConvertPixelType) (IN void* handle, IN OUT MV_CC_PIXEL_CONVERT_PARAM* pstCvtParam); typedef int (__stdcall * DLL_SetBayerCvtQuality) (IN void* handle, IN unsigned int BayerCvtQuality); typedef int (__stdcall * DLL_FeatureSave) (IN void* handle, IN const char* pFileName); typedef int (__stdcall * DLL_FeatureLoad) (IN void* handle, IN const char* pFileName); typedef int (__stdcall * DLL_FileAccessRead) (IN void* handle, IN MV_CC_FILE_ACCESS * pstFileAccess); typedef int (__stdcall * DLL_FileAccessWrite) (IN void* handle, IN MV_CC_FILE_ACCESS * pstFileAccess); typedef int (__stdcall * DLL_GetFileAccessProgress) (IN void* handle, OUT MV_CC_FILE_ACCESS_PROGRESS * pstFileAccessProgress); typedef int (__stdcall * DLL_StartRecord) (IN void* handle, IN MV_CC_RECORD_PARAM* pstRecordParam); typedef int (__stdcall * DLL_InputOneFrame) (IN void* handle, IN MV_CC_INPUT_FRAME_INFO * pstInputFrameInfo); typedef int (__stdcall * DLL_StopRecord) (IN void* handle);

bool g_bExit = false;

struct MultiThrParam { void *pUser; HINSTANCE hDll; };

// ch:等待按键输入 | en:Wait for key press void WaitForKeyPress(void) { while(!_kbhit()) { Sleep(10); } _getch(); }

bool PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo) { if (NULL == pstMVDevInfo) { printf("The Pointer of pstMVDevInfo is NULL!\n"); return false; } if (pstMVDevInfo->nTLayerType == MV_GIGE_DEVICE) { int nIp1 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24); int nIp2 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16); int nIp3 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8); int nIp4 = (pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

// ch:打印当前相机ip和用户自定义名字 | en:print current ip and user defined name printf("CurrentIp: %d.%d.%d.%d\n" , nIp1, nIp2, nIp3, nIp4); printf("UserDefinedName: %s\n\n" , pstMVDevInfo- >SpecialInfo.stGigEInfo.chUserDefinedName); } else if (pstMVDevInfo->nTLayerType == MV_USB_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.chSerialNumber); printf("Device Number: %d\n\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.nDeviceNumber); }

else if (pstMVDevInfo->nTLayerType == MV_GENTL_GIGE_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_CAMERALINK_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_CXP_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_XOF_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chModelName); } else { printf("Not support.\n"); }

return true; }

static unsigned int __stdcall WorkThread(void* stMuthreadPar) { struct MultiThrParam *stMulThrPar = (struct MultiThrParam*)stMuthreadPar; int nRet = MV_OK; MV_FRAME_OUT stOutFrame = {0};

DLL_GetImageBuffer DLLGetImageBuffer = (DLL_GetImageBuffer)GetProcAddress(stMulThrPar->hDll, "MV_CC_GetImageBuffer");

DLL_FreeImageBuffer DLLFreeImageBuffer = (DLL_FreeImageBuffer)GetProcAddress(stMulThrPar->hDll, "MV_CC_FreeImageBuffer");

while(true) {

nRet = DLLGetImageBuffer(stMulThrPar->pUser, &stOutFrame, 1000); if (nRet == MV_OK) { printf("Get Image Buffer: Width[%d], Height[%d], nFrameNum[%d]\n", stOutFrame.stFrameInfo.nWidth, stOutFrame.stFrameInfo.nHeight, stOutFrame.stFrameInfo.nFrameNum);

nRet = DLLFreeImageBuffer(stMulThrPar->pUser, &stOutFrame); if(nRet != MV_OK) { printf("Free Image Buffer fail! nRet [0x%x]\n", nRet); } } else { printf("Get Image fail! nRet [0x%x]\n", nRet); } if(g_bExit) { break; } }

return 0; }

int main() { int nRet = MV_OK; void* handle = NULL;

HINSTANCE MvCamCtrlDll = NULL; // ch:以默认方式加载动态库 | en:Load dynamic libraries by default MvCamCtrlDll = LoadLibrary("MvCameraControl.dll"); if (MvCamCtrlDll) { do { // ch:初始化SDK | en:Initialize SDK DLL_Initialize DLLInitialize = (DLL_Initialize)GetProcAddress(MvCamCtrlDll, "MV_CC_Initialize"); nRet = DLLInitialize();

if (MV_OK != nRet) { printf("Initialize SDK fail! nRet [0x%x]\n", nRet); break; }

// ch:枚举设备 | en:Enum device MV_CC_DEVICE_INFO_LIST stDeviceList; memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST)); DLL_EnumDevices DLLEnumDevices = (DLL_EnumDevices) GetProcAddress (MvCamCtrlDll,"MV_CC_EnumDevices"); nRet = DLLEnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE | MV_GENTL_CAMERALINK_DEVICE | MV_GENTL_CXP_DEVICE | MV_GENTL_XOF_DEVICE, &stDeviceList); if (MV_OK != nRet) { printf("Enum Devices fail! nRet [0x%x]\n", nRet); break; }

if (stDeviceList.nDeviceNum > 0) { for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++) { printf("[device %d]:\n", i); MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i]; if (NULL == pDeviceInfo) { break; } PrintDeviceInfo(pDeviceInfo); } } else { printf("Find No Devices!\n"); break; }

printf("Please Input camera index(0-%d):", stDeviceList.nDeviceNum-1); unsigned int nIndex = 0; scanf_s("%d", &nIndex);

if (nIndex >= stDeviceList.nDeviceNum) { printf("Input error!\n"); break; }

// ch:选择设备并创建句柄 | en:Select device and create handle DLL_CreateHandle DLLCreateHandle = (DLL_CreateHandle)GetProcAddress(MvCamCtrlDll, "MV_CC_CreateHandle"); nRet = DLLCreateHandle(&handle, stDeviceList.pDeviceInfo[nIndex]); if (MV_OK != nRet) { printf("Create Handle fail! nRet [0x%x]\n", nRet); break; }

// ch:打开设备 | en:Open device DLL_OpenDevice DLLOpenDevice = (DLL_OpenDevice)GetProcAddress(MvCamCtrlDll, "MV_CC_OpenDevice"); nRet = DLLOpenDevice(handle, MV_ACCESS_Exclusive, 0); if (MV_OK != nRet) { printf("Open Device fail! nRet [0x%x]\n", nRet); break; }

// ch:设置触发模式为off | en:Set trigger mode as off DLL_SetEnumValue DLLSetEnumValue = (DLL_SetEnumValue)GetProcAddress(MvCamCtrlDll, "MV_CC_SetEnumValue"); nRet = DLLSetEnumValue(handle, "TriggerMode", MV_TRIGGER_MODE_OFF); if (MV_OK != nRet) { printf("Set Trigger Mode fail! nRet [0x%x]\n", nRet); break; }

// ch:开始取流 | en:Start grab image DLL_StartGrabbing DLLStartGrabbing = (DLL_StartGrabbing)GetProcAddress(MvCamCtrlDll, "MV_CC_StartGrabbing"); nRet = DLLStartGrabbing(handle); if (MV_OK != nRet) { printf("Start Grabbing fail! nRet [0x%x]\n", nRet); break; }

unsigned int nThreadID = 0; MultiThrParam stMuthreadPar; stMuthreadPar.pUser = handle; stMuthreadPar.hDll = MvCamCtrlDll; void* hThreadHandle = (void*) _beginthreadex( NULL , 0 , WorkThread , (void*)&stMuthreadPar, 0 , &nThreadID ); if (NULL == hThreadHandle)

{ break; }

printf("Press a key to stop grabbing.\n"); WaitForKeyPress();

g_bExit = true; Sleep(1000);

// ch:停止取流 | en:Stop grab image DLL_StopGrabbing DLLStopGrabbing = (DLL_StartGrabbing)GetProcAddress(MvCamCtrlDll, "MV_CC_StopGrabbing"); nRet = DLLStopGrabbing(handle); if (MV_OK != nRet) { printf("Stop Grabbing fail! nRet [0x%x]\n", nRet); break; }

// ch:关闭设备 | Close device DLL_CloseDevice DLLCloseDevice = (DLL_CloseDevice)GetProcAddress(MvCamCtrlDll, "MV_CC_CloseDevice"); nRet = DLLCloseDevice(handle); if (MV_OK != nRet) { printf("ClosDevice fail! nRet [0x%x]\n", nRet); break; }

// ch:销毁句柄 | Destroy handle DLL_DestroyHandle DLLDestroyHandle = (DLL_DestroyHandle)GetProcAddress(MvCamCtrlDll, "MV_CC_DestroyHandle"); nRet = DLLDestroyHandle(handle); if (MV_OK != nRet) { printf("Destroy Handle fail! nRet [0x%x]\n", nRet); break; } handle = NULL; } while (0);

if (handle != NULL) { DLL_DestroyHandle DLLDestroyHandle = (DLL_DestroyHandle)GetProcAddress(MvCamCtrlDll, "MV_CC_DestroyHandle");

nRet = DLLDestroyHandle(handle); handle = NULL; }

// ch:反初始化SDK | en:Finalize SDK DLL_Finalize DLLFinalize = (DLL_Finalize)GetProcAddress(MvCamCtrlDll, "MV_CC_Finalize"); DLLFinalize();

FreeLibrary(MvCamCtrlDll); printf("Press a key to exit.\n"); WaitForKeyPress(); } else { DWORD errCode = 0; errCode = GetLastError(); printf("error code %ld!\n",errCode); printf("Press a key to exit.\n"); WaitForKeyPress(); return -1; } return 0; }

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

### Events.cpp

获取相机事件

该示例程序说明如何配置相机事件功能、如何注册事件回调以及如何在事件回调函数中处理获取到 的事件信息。Event事件例如：ExposureEnd。

#include <stdio.h> #include <Windows.h> #include <conio.h> #include "MvCameraControl.h"

// ch:等待按键输入 | en:Wait for key press void WaitForKeyPress(void) { while(!_kbhit()) { Sleep(10); } _getch(); }

bool PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo) { if (NULL == pstMVDevInfo) { printf("The Pointer of pstMVDevInfo is NULL!\n"); return false; } if (pstMVDevInfo->nTLayerType == MV_GIGE_DEVICE) { int nIp1 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24); int nIp2 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16); int nIp3 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8); int nIp4 = (pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

// ch:打印当前相机ip和用户自定义名字 | en:print current ip and user defined name printf("CurrentIp: %d.%d.%d.%d\n" , nIp1, nIp2, nIp3, nIp4); printf("UserDefinedName: %s\n\n" , pstMVDevInfo- >SpecialInfo.stGigEInfo.chUserDefinedName); } else if (pstMVDevInfo->nTLayerType == MV_USB_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.chUserDefinedName);

printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.chSerialNumber); printf("Device Number: %d\n\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.nDeviceNumber); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_GIGE_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_CAMERALINK_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_CXP_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_XOF_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chModelName); } else { printf("Not support.\n"); }

return true; }

void __stdcall EventCallBack(MV_EVENT_OUT_INFO * pEventInfo, void* pUser) { if (pEventInfo) { __int64 nBlockId = pEventInfo->nBlockIdHigh;

nBlockId = (nBlockId << 32) + pEventInfo->nBlockIdLow;

__int64 nTimestamp = pEventInfo->nTimestampHigh; nTimestamp = (nTimestamp << 32) + pEventInfo->nTimestampLow;

printf("EventName[%s], EventID[%u], BlockId[%I64d], Timestamp[%I64d]\n", pEventInfo->EventName, pEventInfo- >nEventID,nBlockId,nTimestamp); } }

int main() { int nRet = MV_OK; void* handle = NULL;

do { // ch:初始化SDK | en:Initialize SDK nRet = MV_CC_Initialize(); if (MV_OK != nRet) { printf("Initialize SDK fail! nRet [0x%x]\n", nRet); break; }

// ch:枚举设备 | en:Enum device MV_CC_DEVICE_INFO_LIST stDeviceList; memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST)); nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE | MV_GENTL_CAMERALINK_DEVICE | MV_GENTL_CXP_DEVICE | MV_GENTL_XOF_DEVICE, &stDeviceList); if (MV_OK != nRet) { printf("Enum Devices fail! nRet [0x%x]\n", nRet); break; }

if (stDeviceList.nDeviceNum > 0) { for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++) { printf("[device %d]:\n", i); MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i]; if (NULL == pDeviceInfo) { break; } PrintDeviceInfo(pDeviceInfo);

} } else { printf("Find No Devices!\n"); break; }

printf("Please Input camera index(0-%d):", stDeviceList.nDeviceNum-1); unsigned int nIndex = 0; scanf_s("%d", &nIndex);

if (nIndex >= stDeviceList.nDeviceNum) { printf("Input error!\n"); break; }

// ch:选择设备并创建句柄 | en:Select device and create handle nRet = MV_CC_CreateHandle(&handle, stDeviceList.pDeviceInfo[nIndex]); if (MV_OK != nRet) { printf("Create Handle fail! nRet [0x%x]\n", nRet); break; }

// ch:打开设备 | en:Open device nRet = MV_CC_OpenDevice(handle); if (MV_OK != nRet) { printf("Open Device fail! nRet [0x%x]\n", nRet); break; }

// ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera) if (stDeviceList.pDeviceInfo[nIndex]->nTLayerType == MV_GIGE_DEVICE) { int nPacketSize = MV_CC_GetOptimalPacketSize(handle); if (nPacketSize > 0) { nRet = MV_CC_SetIntValueEx(handle,"GevSCPSPacketSize",nPacketSize); if(nRet != MV_OK) { printf("Warning: Set Packet Size fail nRet [0x%x]!", nRet); }

} else { printf("Warning: Get Packet Size fail nRet [0x%x]!", nPacketSize); } }

nRet = MV_CC_SetEnumValue(handle, "TriggerMode", MV_TRIGGER_MODE_OFF); if (MV_OK != nRet) { printf("Set Trigger Mode fail! nRet [0x%x]\n", nRet); break; }

// ch:开启Event | en:Set Event of ExposureEnd On nRet = MV_CC_EventNotificationOn(handle, "ExposureEnd"); if (MV_OK != nRet) { printf("Set Event Notification On fail! nRet [0x%x]\n", nRet); break; }

// ch:注册事件回调 | en:Register event callback nRet = MV_CC_RegisterEventCallBackEx(handle, "ExposureEnd", EventCallBack, handle); if (MV_OK != nRet) { printf("Register Event CallBack fail! nRet [0x%x]\n", nRet); break; }

// ch:开始取流 | en:Start grab image nRet = MV_CC_StartGrabbing(handle); if (MV_OK != nRet) { printf("Start Grabbing fail! nRet [0x%x]\n", nRet); break; }

printf("Press a key to stop grabbing.\n"); WaitForKeyPress();

// ch:停止取流 | en:Stop grab image nRet = MV_CC_StopGrabbing(handle); if (MV_OK != nRet) { printf("Stop Grabbing fail! nRet [0x%x]\n", nRet);

break; }

// ch:关闭设备 | en:Close device nRet = MV_CC_CloseDevice(handle); if (MV_OK != nRet) { printf("Close Device fail! nRet [0x%x]\n", nRet); break; }

// ch:销毁句柄 | en:Destroy handle nRet = MV_CC_DestroyHandle(handle); if (MV_OK != nRet) { printf("Destroy Handle fail! nRet [0x%x]\n", nRet); break; } handle = NULL; } while (0);

if (handle != NULL) { MV_CC_DestroyHandle(handle); handle = NULL; }

// ch:反初始化SDK | en:Finalize SDK MV_CC_Finalize();

printf("Press a key to exit.\n"); WaitForKeyPress();

return 0; }

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

### Events_Interface.cpp

打开并接受采集卡事件

该示例程序演示了如何打开并接收采集卡事件。

#include <stdio.h> #include <Windows.h> #include <process.h> #include <conio.h> #include "MvCameraControl.h"

//ch：打印设备信息 | en: Print the information of devices bool PrintDeviceInfo(MV_CC_DEVICE_INFO_LIST& stDeviceList); //ch：打印采集卡信息 | en: Print the information of frame grabbers bool PrintInterfaceInfo(MV_INTERFACE_INFO_LIST& stInterfaceInfoList); //ch: 事件回调函数 | en: Event callback function void __stdcall EventCallBack(MV_EVENT_OUT_INFO * pEventInfo, void* pUser); //ch: 取图回调函数 | en: Image callback function void __stdcall ImageCallBackEx(unsigned char * pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser);

// ch:等待按键输入 | en:Wait for key press void WaitForKeyPress(void) { while(!_kbhit()) { Sleep(10); } _getch(); }

int main() { int nRet = MV_OK;

//ch: 采集卡接口类型 | en: The interface type of frame grabber int nInterfaceType = MV_GIGE_INTERFACE | MV_CAMERALINK_INTERFACE | MV_CXP_INTERFACE | MV_XOF_INTERFACE;

void* hInterface = NULL; //ch: 采集卡句柄 | en: Handle of frame grabber void* hDevHandle = NULL; //ch: 设备句柄 | en: Handle of device

do { // ch:初始化SDK | en:Initialize SDK nRet = MV_CC_Initialize(); if (MV_OK != nRet) { printf("Initialize SDK fail! nRet [0x%x]\n", nRet); break; }

//ch: 枚举采集卡 | en: Enumerate frame grabbers MV_INTERFACE_INFO_LIST stInterfaceInfoList={0}; nRet = MV_CC_EnumInterfaces(nInterfaceType, &stInterfaceInfoList); if (MV_OK != nRet) { printf("Enum interfaces fail! nRet [0x%x]\n", nRet); break; }

if (stInterfaceInfoList.nInterfaceNum > 0) { PrintInterfaceInfo(stInterfaceInfoList); printf("Enum interfaces success!\n\n"); } else { printf("Find no interface!\n"); break; }

printf("Please input interfaces index(0-%d):", stInterfaceInfoList.nInterfaceNum-1); unsigned int nIndex = 0; scanf_s("%d", &nIndex);

if (nIndex >= stInterfaceInfoList.nInterfaceNum)

{ printf("Input error!\n"); break; }

//ch:创建采集卡句柄 | en: Create the handle of frame grabber nRet = MV_CC_CreateInterface(&hInterface, stInterfaceInfoList.pInterfaceInfos[nIndex]); if (MV_OK == nRet) { printf("Create interface success!\n"); } else { printf("Create interface Handle fail! nRet [0x%x]\n", nRet); break; }

//ch: 打开采集卡 | en: Open the frame grabber nRet = MV_CC_OpenInterface(hInterface, NULL); if (MV_OK == nRet) { printf("Open interface success!\n"); } else { printf("Open interface fail! nRet [0x%x]\n", nRet); break; }

//ch：开启采集卡ReceiveImageFrameStart0事件 | en: Turn on the ReceiveImageFrameStart0 event of frame grabber nRet = MV_CC_EventNotificationOn(hInterface, "ReceiveImageFrameStart0"); if (nRet != MV_OK) { printf("EventNotificationOn fail! nRet [0x%x]\n", nRet); break; }

// ch:注册采集卡事件回调 | en:Register event callback nRet = MV_CC_RegisterEventCallBackEx(hInterface, "ReceiveImageFrameStart0", EventCallBack, NULL); if (MV_OK != nRet) { printf("Register Event CallBack fail! nRet [0x%x]\n", nRet); break; }

//ch: 枚举采集卡上的相机 | en: Enumerate the devices on the frame grabber MV_CC_DEVICE_INFO_LIST stDeviceList = {0}; nRet = MV_CC_EnumDevicesByInterface(hInterface, &stDeviceList); if (nRet != MV_OK) { printf("MV_CC_EnumDevicesByInterface fail! nRet [0x%x]\n", nRet); break; }

if (stDeviceList.nDeviceNum == 0) { printf("no device! fail!\n"); break; }

PrintDeviceInfo(stDeviceList);

// ch:选择设备0并创建句柄 | en:Select device 0 and create handle nIndex = 0; nRet = MV_CC_CreateHandle(&hDevHandle, stDeviceList.pDeviceInfo[nIndex]); if (MV_OK != nRet) { printf("Create handle fail! nRet [0x%x]\n", nRet); break; }

// ch:打开设备 | en:Open device nRet = MV_CC_OpenDevice(hDevHandle); if (MV_OK != nRet) { printf("Open device fail! nRet [0x%x]\n", nRet); break; }

printf("Open device success!\n");

// ch:注册取图回调 | en:Register image callback nRet = MV_CC_RegisterImageCallBackEx(hDevHandle, ImageCallBackEx, NULL); if (MV_OK != nRet) { printf("Register Image CallBack fail! nRet [0x%x]\n", nRet); break; }

// ch:开始取流 | en:Start grab image nRet = MV_CC_StartGrabbing(hDevHandle); if (MV_OK != nRet) { printf("Start Grabbing fail! nRet [0x%x]\n", nRet); break; }

printf("Press a key to stop grabbing.\n"); WaitForKeyPress();

// ch:停止取流 | en:Stop grab image nRet = MV_CC_StopGrabbing(hDevHandle); if (MV_OK != nRet) { printf("Stop Grabbing fail! nRet [0x%x]\n", nRet); break; }

// ch:关闭设备 | Close device nRet = MV_CC_CloseDevice(hDevHandle); if (MV_OK != nRet) { printf("ClosDevice fail! nRet [0x%x]\n", nRet); break; }

printf("Close device success!\n");

// ch:销毁设备句柄 | Destroy handle of device nRet = MV_CC_DestroyHandle(hDevHandle); if (MV_OK != nRet) { printf("Destroy handle fail! nRet [0x%x]\n", nRet); break; }

printf("Destroy device handle success!\n");

hDevHandle = NULL;

//ch: 关闭采集卡 | en: Close the frame grabber nRet = MV_CC_CloseInterface(hInterface); if (MV_OK == nRet) { printf("Close interface success!\n"); }

else { printf("Close interface Handle fail! nRet [0x%x]\n", nRet); break; }

// ch:销毁采集卡句柄 | Destroy handle of frame grabber nRet = MV_CC_DestroyInterface(hInterface); if (MV_OK == nRet) { printf("Destroy interface success!\n"); } else { printf("Destroy interface Handle fail! nRet [0x%x]\n", nRet); break; } hInterface = NULL; } while (0);

if (hDevHandle != NULL) { MV_CC_CloseDevice(hDevHandle); MV_CC_DestroyHandle(hDevHandle); hDevHandle = NULL; }

if (hInterface != NULL) { MV_CC_CloseInterface(hInterface); MV_CC_DestroyInterface(hInterface); hInterface = NULL; }

// ch:反初始化SDK | en:Finalize SDK MV_CC_Finalize();

printf("Press a key to exit.\n"); WaitForKeyPress(); }

bool PrintDeviceInfo(MV_CC_DEVICE_INFO_LIST& stDeviceList) { for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++) { printf("[device %d]:\n", i); MV_CC_DEVICE_INFO* pstMVDevInfo = stDeviceList.pDeviceInfo[i];

if (pstMVDevInfo->nTLayerType == MV_GIGE_DEVICE)

{ int nIp1 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24); int nIp2 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16); int nIp3 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8); int nIp4 = (pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

// ch:打印当前相机ip和用户自定义名字 | en:print current ip and user defined name printf("CurrentIp: %d.%d.%d.%d\n" , nIp1, nIp2, nIp3, nIp4); printf("UserDefinedName: %s\n\n" , pstMVDevInfo- >SpecialInfo.stGigEInfo.chUserDefinedName); } else if (pstMVDevInfo->nTLayerType == MV_USB_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.chSerialNumber); printf("Device Number: %d\n\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.nDeviceNumber); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_GIGE_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chSerialNumber); printf("Model Name %s\n\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_CAMERALINK_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_CXP_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_XOF_DEVICE) {

printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chModelName); } else { printf("Not support.\n"); } }

return true; }

bool PrintInterfaceInfo(MV_INTERFACE_INFO_LIST& stInterfaceInfoList) { for (unsigned int i = 0; i < stInterfaceInfoList.nInterfaceNum; i++) { printf("[Interface %d]:\n", i); MV_INTERFACE_INFO* pstInterfaceInfo = stInterfaceInfoList.pInterfaceInfos[i]; if (NULL == pstInterfaceInfo) { break; } printf("Display name: %s\n",pstInterfaceInfo->chDisplayName); printf("Serial number: %s\n",pstInterfaceInfo->chSerialNumber); printf("model name: %s\n",pstInterfaceInfo->chModelName); printf("\n"); }

return true; }

void __stdcall EventCallBack(MV_EVENT_OUT_INFO * pEventInfo, void* pUser) { if (pEventInfo) { __int64 nTimestamp = pEventInfo->nTimestampHigh; nTimestamp = (nTimestamp << 32) + pEventInfo->nTimestampLow;

printf("EventName[%s], EventID[%u], Timestamp[%lld]\n", pEventInfo->EventName, pEventInfo->nEventID,nTimestamp); } }

void __stdcall ImageCallBackEx(unsigned char * pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser)

{ /*if (pFrameInfo) { printf("Get One Frame: Width[%d], Height[%d], nFrameNum[%d]\n", pFrameInfo->nWidth, pFrameInfo->nHeight, pFrameInfo- >nFrameNum); }*/ }

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

### ForceIPEx.cpp

强制设置相机IP

该示例程序说明如何强制设置网口相机IP，保证断电后重新连接，重新枚举后依旧生效。

#include <stdio.h> #include <Windows.h> #include <process.h> #include <conio.h> #include "MvCameraControl.h" #pragma comment(lib, "wsock32.lib")

#define PERSISTENT_CONFIG_REG 0x0014 #define PERSISTENT_IPADDR_REG 0x064C #define PERSISTENT_SUBNETMASK_REG 0x065C #define PERSISTENT_DEFAULTGATEWAY_REG 0x066C

// ch:等待按键输入 | en:Wait for key press void WaitForKeyPress(void) { while(!_kbhit()) { Sleep(10); } _getch(); }

bool PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo) { if (NULL == pstMVDevInfo) { printf("The Pointer of pstMVDevInfo is NULL!\n"); return false; } if (pstMVDevInfo->nTLayerType == MV_GIGE_DEVICE) { int nIp1 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24); int nIp2 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16); int nIp3 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8); int nIp4 = (pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

// ch:打印当前相机ip和用户自定义名字 | en:print current ip and user defined name printf("CurrentIp: %d.%d.%d.%d\n" , nIp1, nIp2, nIp3, nIp4); printf("UserDefinedName: %s\n\n" , pstMVDevInfo- >SpecialInfo.stGigEInfo.chUserDefinedName); } else if (pstMVDevInfo->nTLayerType == MV_USB_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.chSerialNumber); printf("Device Number: %d\n\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.nDeviceNumber); } else { printf("Not support.\n"); }

return true; }

//地址的转换函数 bool ConvertToHexIp(unsigned int *nHexIP, unsigned int *nDecIP, char c) { if ( nDecIP[0] < 0 || nDecIP[0] > 255 || nDecIP[1] < 0 || nDecIP[1] > 255 || nDecIP[2] < 0 || nDecIP[2] > 255 || nDecIP[3] < 0 || nDecIP[3] > 255 || c != '\n') { return false; } *nHexIP = (nDecIP[0] << 24) + (nDecIP[1] << 16) + (nDecIP[2] << 8) + nDecIP[3];

return true; } int main() { int nRet = MV_OK; void* handle = NULL; unsigned int nIP[4] = {0}; char c = '\0'; unsigned int nIpAddr = 0, nNetWorkMask = 0, nDefaultGateway = 0; do { // ch:初始化SDK | en:Initialize SDK nRet = MV_CC_Initialize(); if (MV_OK != nRet) { printf("Initialize SDK fail! nRet [0x%x]\n", nRet); break; }

// ch:枚举设备 | en:Enum device MV_CC_DEVICE_INFO_LIST stDeviceList; memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST)); nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &stDeviceList); if (MV_OK != nRet) { printf("Enum Devices fail! nRet [0x%x]\n", nRet); break; }

if (stDeviceList.nDeviceNum > 0) { for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++) { printf("[device %d]:\n", i); MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i]; if (NULL == pDeviceInfo) {

break; } PrintDeviceInfo(pDeviceInfo); } } else { printf("Find No Devices!\n"); break; }

printf("Please Input camera index(0-%d):", stDeviceList.nDeviceNum-1); unsigned int nIndex = 0; scanf_s("%d", &nIndex);

if (nIndex >= stDeviceList.nDeviceNum) { printf("Input error!\n"); break; }

// ch:选择设备并创建句柄 | en:Select device and create handle nRet = MV_CC_CreateHandle(&handle, stDeviceList.pDeviceInfo[nIndex]); if (MV_OK != nRet) { printf("Create Handle fail! nRet [0x%x]\n", nRet); break; }

//记录第一次的序列号 //Record the serial number for the first time char chSerialNumber[16] = {0}; memcpy(chSerialNumber, stDeviceList.pDeviceInfo[nIndex]- >SpecialInfo.stGigEInfo.chSerialNumber, sizeof(stDeviceList.pDeviceInfo[nIndex]- >SpecialInfo.stGigEInfo.chSerialNumber)); printf("serial number:[%s]\n", chSerialNumber);

// 输入IP 子网掩码 默认网关 // input ip, subnet mask and defaultway printf("Please input ip, example: 192.168.1.100\n");

if ( 5 != scanf("%d.%d.%d.%d%c", &nIP[0], &nIP[1], &nIP[2], &nIP[3], &c) ) { printf("input count error\n"); break; }

if (!ConvertToHexIp(&nIpAddr, nIP, c)) { printf("input IpAddr format is not correct\n"); break; }

printf("Please input NetMask, example: 255.255.255.0\n"); if ( 5 != scanf("%d.%d.%d.%d%c", &nIP[0], &nIP[1], &nIP[2], &nIP[3], &c) ) { printf("input count error\n"); break; } if (!ConvertToHexIp(&nNetWorkMask, nIP, c)) { printf("input NetMask format is not correct\n"); break; }

printf("Please input DefaultWay, example: 192.168.1.1\n"); if ( 5 != scanf("%d.%d.%d.%d%c", &nIP[0], &nIP[1], &nIP[2], &nIP[3], &c) ) { printf("input count error\n"); break; } if (!ConvertToHexIp(&nDefaultGateway, nIP, c)) { printf("input DefaultWay format is not correct\n"); break; }

//判断设备Ip是否可达 //Determine whether the IP address is reachable bool bAccessible = MV_CC_IsDeviceAccessible(stDeviceList.pDeviceInfo[nIndex], MV_ACCESS_Exclusive); if(bAccessible) { //ch:设置IP配置选项 | en:set IP config nRet = MV_GIGE_SetIpConfig(handle, MV_IP_CFG_STATIC); if (MV_OK != nRet) { printf("MV_GIGE_SetIpConfig fail! nRet [%x]\n", nRet); break; } printf("set IPConfig succeed\n");

//ch:设置强制IP | en:set force IP nRet = MV_GIGE_ForceIpEx(handle, nIpAddr, nNetWorkMask, nDefaultGateway); if (MV_OK != nRet)

{ printf("MV_GIGE_ForceIpEx fail! nRet [%x]\n", nRet); break; } printf("set IP succeed\n"); } else { //ch:设置强制IP | en:set force IP nRet = MV_GIGE_ForceIpEx(handle, nIpAddr, nNetWorkMask, nDefaultGateway); if (MV_OK != nRet) { printf("MV_GIGE_ForceIpEx fail! nRet [%x]\n", nRet); break; } printf("set IP succeed\n");

MV_CC_DestroyHandle(handle); handle = NULL;

//ch:需要重新创建句柄，设置为静态IP方式进行保存 | en:re-create the handle and set it to static IP for storage stDeviceList.pDeviceInfo[nIndex]- >SpecialInfo.stGigEInfo.nCurrentIp = nIpAddr; stDeviceList.pDeviceInfo[nIndex]- >SpecialInfo.stGigEInfo.nCurrentSubNetMask = nNetWorkMask; stDeviceList.pDeviceInfo[nIndex]- >SpecialInfo.stGigEInfo.nDefultGateWay = nDefaultGateway; nRet = MV_CC_CreateHandle(&handle, stDeviceList.pDeviceInfo[nIndex]); if (MV_OK != nRet) { printf("MV_CC_CreateHandle fail! nRet [%x]\n", nRet); break; } // ch:设置IP配置选项 | en:set IP config nRet = MV_GIGE_SetIpConfig(handle, MV_IP_CFG_STATIC); if (MV_OK != nRet) { printf("MV_GIGE_SetIpConfig fail! nRet [%x]\n", nRet); break; } printf("set IPConfig succeed\n"); } //ch:销毁句柄 | en:Destroy handle nRet = MV_CC_DestroyHandle(handle); if (MV_OK != nRet) { printf("Destroy Handle fail! nRet [0x%x]\n", nRet); break; } handle = NULL;

//设置完forceip后重新枚举一下 | en:After setting up ForceIP, re- enumerate it Sleep(500); nIndex = -1; memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST)); nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &stDeviceList); if (MV_OK != nRet) { printf("Enum Devices fail! nRet [0x%x]\n", nRet); break; }

if (stDeviceList.nDeviceNum > 0) { for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++) {

MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i]; if (NULL == pDeviceInfo) { printf("pointer invalid!\n"); break; }

if (!strcmp(chSerialNumber, (char*)(pDeviceInfo- >SpecialInfo.stGigEInfo.chSerialNumber))) { nIndex = i; printf("find it, serial number:[%s]\n", pDeviceInfo- >SpecialInfo.stGigEInfo.chSerialNumber); break; } } } else { printf("Find No Devices!\n"); break; } if (-1 == nIndex) { printf("Not Find First Device\n"); }

// ch:选择设备并创建句柄 | en:Select device and create handle nRet = MV_CC_CreateHandle(&handle, stDeviceList.pDeviceInfo[nIndex]); if (MV_OK != nRet) { printf("Create Handle fail! nRet [0x%x]\n", nRet);

break; }

// ch:打开设备 | en:Open device nRet = MV_CC_OpenDevice(handle); if (MV_OK != nRet) { printf("Open Device fail! nRet [0x%x]\n", nRet); break; } //ch:设置presistent IP | en: set presistent IP unsigned int nConfig = htonl(0x05); nRet = MV_CC_WriteMemory(handle, (const void*)&(nConfig), PERSISTENT_CONFIG_REG, sizeof(nConfig)); if (MV_OK != nRet) { printf("write config fail! nRet [0x%x]\n", nRet); break; } else { printf("write config success! nRet [0x%x]\n", nRet); } unsigned int nIpAddr1 = htonl(nIpAddr); nRet = MV_CC_WriteMemory(handle, (const void*)&(nIpAddr1), PERSISTENT_IPADDR_REG, sizeof(nIpAddr)); if (MV_OK != nRet) { printf("write ip fail! nRet [0x%x]\n", nRet); break; } unsigned int nNetWorkMask1 = htonl(nNetWorkMask); nRet = MV_CC_WriteMemory(handle, (const void*)&(nNetWorkMask1), PERSISTENT_SUBNETMASK_REG, sizeof(nNetWorkMask)); if (MV_OK != nRet) { printf("write nNetWorkMask fail! nRet [0x%x]\n", nRet); break; } unsigned int nDefaultGateway1 = htonl(nDefaultGateway); nRet = MV_CC_WriteMemory(handle, (const void*)&(nDefaultGateway1), PERSISTENT_DEFAULTGATEWAY_REG, sizeof(nDefaultGateway)); if (MV_OK != nRet) { printf("write nDefaultGateway fail! nRet [0x%x]\n", nRet); break; } // ch:关闭设备 | Close device nRet = MV_CC_CloseDevice(handle); if (MV_OK != nRet) { printf("Close device fail! nRet [0x%x]\n", nRet); break; }

// ch:销毁句柄 | Destroy handle nRet = MV_CC_DestroyHandle(handle); if (MV_OK != nRet) { printf("Destroy Handle fail! nRet [0x%x]\n", nRet); break; } handle = NULL; printf("ForceIp Success!\n"); } while (0);

if (handle != NULL) { MV_CC_DestroyHandle(handle); handle = NULL; }

// ch:反初始化SDK | en:Finalize SDK MV_CC_Finalize();

printf("Press a key to exit.\n"); WaitForKeyPress();

return 0; }

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

### Grab_ActionCommand.cpp

PTP时钟协议

该示例程序说明如何通过PTP时钟协议对相机进行采图。

#include <stdio.h> #include <Windows.h> #include <process.h> #include <conio.h> #include "MvCameraControl.h"

bool g_bExit = false; unsigned int g_DeviceKey = 1; unsigned int g_GroupKey = 1; unsigned int g_GroupMask= 1;

// ch:等待按键输入 | en:Wait for key press void WaitForKeyPress(void) { while(!_kbhit()) { Sleep(10); } _getch(); }

bool PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo) { if (NULL == pstMVDevInfo) { printf("The Pointer of pstMVDevInfo is NULL!\n"); return false; } if (pstMVDevInfo->nTLayerType == MV_GIGE_DEVICE) { int nIp1 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24); int nIp2 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16); int nIp3 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8); int nIp4 = (pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

// ch:打印当前相机ip和用户自定义名字 | en:print current ip and user defined name printf("CurrentIp: %d.%d.%d.%d\n" , nIp1, nIp2, nIp3, nIp4); printf("UserDefinedName: %s\n\n" , pstMVDevInfo- >SpecialInfo.stGigEInfo.chUserDefinedName); } else { printf("Not support.\n"); }

return true; }

static unsigned int __stdcall ActionCommandWorkThread(void* pUser) { int nRet = MV_OK; MV_ACTION_CMD_INFO stActionCmdInfo = {0}; MV_ACTION_CMD_RESULT_LIST stActionCmdResults = {0};

stActionCmdInfo.nDeviceKey = g_DeviceKey; stActionCmdInfo.nGroupKey = g_GroupKey; stActionCmdInfo.nGroupMask = g_GroupMask; stActionCmdInfo.pBroadcastAddress = "255.255.255.255"; stActionCmdInfo.nTimeOut = 100; stActionCmdInfo.bActionTimeEnable = 0;

while(!g_bExit) { //Send the PTP clock photo command nRet = MV_GIGE_IssueActionCommand(&stActionCmdInfo,&stActionCmdResults); if (MV_OK != nRet) { printf("Issue Action Command fail! nRet [0x%x]\n", nRet); continue; } printf("NumResults = %d\r\n",stActionCmdResults.nNumResults);

MV_ACTION_CMD_RESULT* pResults = stActionCmdResults.pResults; for (unsigned int i = 0;i < stActionCmdResults.nNumResults;i++) { //Print the device infomation printf("Ip == %s, Status == 0x%x\r\n",pResults- >strDeviceAddress,pResults->nStatus); pResults++; } }

return 0; }

static unsigned int __stdcall ReceiveImageWorkThread(void* pUser) { int nRet = MV_OK; MV_FRAME_OUT stImageInfo = {0};

while(1) { nRet = MV_CC_GetImageBuffer(pUser, &stImageInfo, 1000); if (nRet == MV_OK) { printf("Get Image Buffer: Width[%d], Height[%d], FrameNum[%d]\n", stImageInfo.stFrameInfo.nWidth, stImageInfo.stFrameInfo.nHeight, stImageInfo.stFrameInfo.nFrameNum);

nRet = MV_CC_FreeImageBuffer(pUser, &stImageInfo); if(nRet != MV_OK) { printf("Free Image Buffer fail! nRet [0x%x]\n", nRet); } } else { printf("Get Image fail! nRet [0x%x]\n", nRet); } if(g_bExit) { break; } }

return 0; }

int main() { int nRet = MV_OK; void* handle = NULL;

do { // ch:初始化SDK | en:Initialize SDK nRet = MV_CC_Initialize(); if (MV_OK != nRet) { printf("Initialize SDK fail! nRet [0x%x]\n", nRet); break; }

// ch:枚举设备 | en:Enum device MV_CC_DEVICE_INFO_LIST stDeviceList; memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST)); nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE, &stDeviceList); if (MV_OK != nRet) { printf("Enum Devices fail! nRet [0x%x]\n", nRet); break; }

if (stDeviceList.nDeviceNum > 0) { for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++) { printf("[device %d]:\n", i); MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i]; if (NULL == pDeviceInfo) { break; } PrintDeviceInfo(pDeviceInfo); } } else { printf("Find No Devices!\n"); break; }

printf("Please Input camera index(0-%d):", stDeviceList.nDeviceNum-1); unsigned int nIndex = 0; scanf_s("%d", &nIndex);

if (nIndex >= stDeviceList.nDeviceNum) { printf("Input error!\n"); break; }

// ch:选择设备并创建句柄 | en:Select device and create handle nRet = MV_CC_CreateHandle(&handle, stDeviceList.pDeviceInfo[nIndex]); if (MV_OK != nRet) { printf("Create Handle fail! nRet [0x%x]\n", nRet); break; }

// ch:打开设备 | en:Open device nRet = MV_CC_OpenDevice(handle); if (MV_OK != nRet) { printf("Open Device fail! nRet [0x%x]\n", nRet); break; }

// ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera) if (stDeviceList.pDeviceInfo[nIndex]->nTLayerType == MV_GIGE_DEVICE) { int nPacketSize = MV_CC_GetOptimalPacketSize(handle); if (nPacketSize > 0) { nRet = MV_CC_SetIntValueEx(handle,"GevSCPSPacketSize",nPacketSize); if(nRet != MV_OK) { printf("Warning: Set Packet Size fail nRet [0x%x]!", nRet); } } else { printf("Warning: Get Packet Size fail nRet [0x%x]!", nPacketSize); } }

// ch:设置触发模式为on | en:Set trigger mode as on nRet = MV_CC_SetEnumValue(handle, "TriggerMode", 1); if (MV_OK != nRet) { printf("Set Trigger Mode fail! nRet [0x%x]\n", nRet); break; }

// ch:设置触发源为Action1 | en:Set trigger source as Action1 nRet = MV_CC_SetEnumValueByString(handle, "TriggerSource", "Action1"); if (MV_OK != nRet) { printf("Set Trigger Source fail! nRet [0x%x]\n", nRet); break; }

// ch:设置Action Device Key | en:Set Action Device Key nRet = MV_CC_SetIntValueEx(handle, "ActionDeviceKey", g_DeviceKey); if (MV_OK != nRet)

{ printf("Set Action Device Key fail! nRet [0x%x]\n", nRet); break; }

// ch:设置Action Group Key | en:Set Action Group Key100 nRet = MV_CC_SetIntValueEx(handle, "ActionGroupKey", g_GroupKey); if (MV_OK != nRet) { printf("Set Action Group Key fail! nRet [0x%x]\n", nRet); break; }

// ch:设置Action Group Mask | en:Set Action Group Mask nRet = MV_CC_SetIntValueEx(handle, "ActionGroupMask", g_GroupMask); if (MV_OK != nRet) { printf("Set Action Group Mask fail! nRet [0x%x]\n", nRet); break; }

// ch:开始取流 | en:Start grab image nRet = MV_CC_StartGrabbing(handle); if (MV_OK != nRet) { printf("Start Grabbing fail! nRet [0x%x]\n", nRet); break; }

unsigned int nReceiveImageThreadID = 0; void* hReceiveImageThreadHandle = (void*) _beginthreadex( NULL , 0 , ReceiveImageWorkThread , handle, 0 , &nReceiveImageThreadID ); if (NULL == hReceiveImageThreadHandle) { break; }

unsigned int nActionCommandThreadID = 0; void* hActionCommandThreadHandle = (void*) _beginthreadex( NULL , 0 , ActionCommandWorkThread , NULL, 0 , &nActionCommandThreadID ); if (NULL == hActionCommandThreadHandle) { return 0; }

printf("Press a key to stop grabbing.\n");

WaitForKeyPress();

g_bExit = true; Sleep(1000);

// ch:停止取流 | en:Stop grab image nRet = MV_CC_StopGrabbing(handle); if (MV_OK != nRet) { printf("Stop Grabbing fail! nRet [0x%x]\n", nRet); break; }

// ch:关闭设备 | Close device nRet = MV_CC_CloseDevice(handle); if (MV_OK != nRet) { printf("ClosDevice fail! nRet [0x%x]\n", nRet); break; }

// ch:销毁句柄 | Destroy handle nRet = MV_CC_DestroyHandle(handle); if (MV_OK != nRet) { printf("Destroy Handle fail! nRet [0x%x]\n", nRet); break; } handle = NULL; } while (0);

if (handle != NULL) { MV_CC_DestroyHandle(handle); handle = NULL; }

// ch:反初始化SDK | en:Finalize SDK MV_CC_Finalize();

printf("Press a key to exit.\n"); WaitForKeyPress();

return 0; }

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

### Grab_Asynchronous.cpp

异步取图及处理

该示例程序说明如何异步取图并进行图像处理，规避了同步时可能存在的耗时长问题。

/* 异步取流示例demo

主要逻辑如下：
1. 调用SDK接口获取图像，并将图像数据放入对列中
2. 开启处理线程，从对列中获取图像，并处理;

注意要点：
1. 对列大小
2. 对列锁保护，读取/写入数据 */

#include <stdio.h> #include <Windows.h> #include <conio.h> #include <process.h>

#include <iostream> #include "MvCameraControl.h"

using namespace std;

class CMVMutex { public: CMVMutex() { InitializeCriticalSection(&m_Mutex);

} ~CMVMutex() { DeleteCriticalSection(&m_Mutex); } void _Lock() { EnterCriticalSection(&m_Mutex); } void _Unlock() { LeaveCriticalSection(&m_Mutex); }

private: CRITICAL_SECTION m_Mutex; };

bool g_bExit = false; //线程结束标记 #define Max_Count 5 //队列 缓冲区个数； 根据实际情况调节 int m_nImageSize = 0; // 图像大小 CMVMutex *g_mutex = NULL; //互斥锁

typedef struct _stImageNode_ { unsigned char* pData; unsigned int nFrameLen;

unsigned short nWidth; unsigned short nHeight; unsigned int nFrameNum;

}stImageNode;

class ArrayQueue { public: ArrayQueue() { this->size = 0; this->start = 0; this->end = 0; this->Queue = NULL; this->Qlen = 0; }

~ArrayQueue() { g_mutex->_Lock();

for (int i = 0; i< Qlen; i++) { Queue[i].nFrameNum = 0; Queue[i].nHeight = 0; Queue[i].nWidth = 0; Queue[i].nFrameLen = 0; if (Queue[i].pData) { free(Queue[i].pData); Queue[i].pData = NULL; }

printf(" free ArrayQueue [%d] !\r\n",i); }

delete []Queue; Queue = NULL;

size = 0; start = 0; end = 0;

g_mutex->_Unlock();

}

// 队列初始化 int Init(int nBufCount, int DefaultImagelen) { int nRet = 0 ;

this->Queue = new (std::nothrow)stImageNode[nBufCount]; if (this->Queue == NULL) { return MV_E_RESOURCE; } this->Qlen = nBufCount;

for (int i = 0; i< nBufCount; i++) { Queue[i].nFrameNum = 0; Queue[i].nHeight = 0; Queue[i].nWidth = 0; Queue[i].nFrameLen = 0; Queue[i].pData = (unsigned char*)malloc(DefaultImagelen); if(NULL == Queue[i].pData) { return MV_E_RESOURCE; } }

return 0;

}

// 数据放入队列 int push(int nFrameNum, int nWidth, int nHeight, unsigned char *pData, int nFrameLen) { g_mutex->_Lock();

if (size==Qlen) { g_mutex->_Unlock(); return MV_E_BUFOVER; }

size++;

Queue[end].nFrameNum = nFrameNum; Queue[end].nHeight = nHeight; Queue[end].nWidth = nWidth; Queue[end].nFrameLen = nFrameLen;

if (NULL != Queue[end].pData && NULL != pData) { memcpy(Queue[end].pData, pData, nFrameLen); }

end = end == Qlen - 1 ? 0 : end + 1; g_mutex->_Unlock();

return 0; }

// 数据从队列中取出 int poll(int &nFrameNum, int &nHeight, int &nWidth, unsigned char *pData, int &nFrameLen) {

g_mutex->_Lock();

if (size == 0) { g_mutex->_Unlock(); return MV_E_NODATA ; }

nFrameNum =Queue[start].nFrameNum; nHeight =Queue[start].nHeight; nWidth =Queue[start].nWidth; nFrameLen =Queue[start].nFrameLen;

if (NULL != pData && NULL != Queue[start].pData) { memcpy( pData,Queue[start].pData, nFrameLen); }

size--; start = start == Qlen - 1 ? 0 : start + 1;

g_mutex->_Unlock();

return 0; }

private: stImageNode *Queue; int size; int start; int end; int Qlen; };

ArrayQueue * m_queue = NULL; //线程通信队列

// ch:等待按键输入 | en:Wait for key press void WaitForKeyPress(void) { while(!_kbhit()) { Sleep(10); } _getch(); }

bool PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo) { if (NULL == pstMVDevInfo) { printf("The Pointer of pstMVDevInfo is NULL!\n"); return false; } if (pstMVDevInfo->nTLayerType == MV_GIGE_DEVICE) { int nIp1 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24); int nIp2 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16); int nIp3 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8); int nIp4 = (pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

// ch:打印当前相机ip和用户自定义名字 | en:print current ip and user defined name printf("CurrentIp: %d.%d.%d.%d\n" , nIp1, nIp2, nIp3, nIp4); printf("UserDefinedName: %s\n\n" , pstMVDevInfo- >SpecialInfo.stGigEInfo.chUserDefinedName); } else if (pstMVDevInfo->nTLayerType == MV_USB_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.chSerialNumber); printf("Device Number: %d\n\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.nDeviceNumber); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_GIGE_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_CAMERALINK_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_CXP_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_XOF_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chModelName); } else { printf("Not support.\n"); }

return true; }

void __stdcall ImageCallBackEx(unsigned char * pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser) { if (NULL == pFrameInfo || NULL == pData) { printf("ImageCallBackEx Input Param invalid.\n"); return; }

int nRet = MV_OK; nRet = m_queue->push(pFrameInfo->nFrameNum, pFrameInfo->nWidth, pFrameInfo->nHeight,pData, pFrameInfo->nFrameLen);//固定数组实现队列 故是10 9 8 7 6 5 4 3 2 1 if (MV_OK != nRet) { printf("Add Image [%d] to list failed. \r\n", pFrameInfo- >nFrameNum); } else { printf("Add Image [%d] to list success. \r\n", pFrameInfo- >nFrameNum); }

return; }

static unsigned int __stdcall WorkThread(void* pUser) { int nRet = MV_OK;

int nWidth = 0; int nHeight = 0; int nFrameLen = 0; int nFrameNum = 0; unsigned char *pOutData = (unsigned char *) malloc(m_nImageSize); if (NULL == pOutData) { printf("WorkThread malloc size [%d] failed. \r\n",m_nImageSize); return MV_E_RESOURCE; } printf("WorkThread Begin . \r\n");

while(true != g_bExit)

{ // 变量初始化 nWidth = 0; nHeight = 0; nFrameLen = 0; nFrameNum = 0;

nRet = m_queue- >poll(nFrameNum,nHeight,nWidth,pOutData,nFrameLen); if (MV_OK != nRet) { printf("Poll failed, maybe no data. \r\n"); Sleep(2); continue; } else { printf("Get nWidth [%d] nHeight [%d] nFrameNum [%d] \r\n",nWidth,nHeight,nFrameNum);

//根据实际场景需求，对图像进行 处理 #if 0 FILE* fp = NULL; char szFileName[256] = {0}; sprintf(szFileName, "Image_%d_width_%d_height_%d_Len_%d.raw",nFrameNum,nWidth,nHeight ,nFrameLen); fp = fopen(szFileName, "wb+"); if (fp == NULL) { return MV_E_RESOURCE; } fwrite(pOutData, 1, nFrameLen, fp); fclose(fp); fp = NULL; #endif } }

printf("WorkThread exit . \r\n");

if (pOutData) { free(pOutData); pOutData = NULL; }

return 0; } int main() {

int nRet = MV_OK; void* handle = NULL;

do { g_mutex = new (std::nothrow)CMVMutex(); //互斥锁 if (g_mutex == NULL) { printf("g_mutex is null! \n"); break; }

// ch:初始化SDK | en:Initialize SDK nRet = MV_CC_Initialize(); if (MV_OK != nRet) { printf("Initialize SDK fail! nRet [0x%x]\n", nRet); break; }

// ch:枚举设备 | Enum device MV_CC_DEVICE_INFO_LIST stDeviceList; memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST)); nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE | MV_GENTL_CAMERALINK_DEVICE | MV_GENTL_CXP_DEVICE | MV_GENTL_XOF_DEVICE, &stDeviceList); if (MV_OK != nRet) { printf("Enum Devices fail! nRet [0x%x]\n", nRet); break; }

if (stDeviceList.nDeviceNum > 0) { for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++) { printf("[device %d]:\n", i); MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i]; if (NULL == pDeviceInfo) { break; } PrintDeviceInfo(pDeviceInfo); } } else { printf("Find No Devices!\n"); break; }

printf("Please Input camera index(0-%d):", stDeviceList.nDeviceNum-1); unsigned int nIndex = 0; scanf_s("%d", &nIndex);

if (nIndex >= stDeviceList.nDeviceNum) { printf("Input error!\n"); break; }

// ch:选择设备并创建句柄 | Select device and create handle nRet = MV_CC_CreateHandle(&handle, stDeviceList.pDeviceInfo[nIndex]); if (MV_OK != nRet) { printf("Create Handle fail! nRet [0x%x]\n", nRet); break; }

// ch:打开设备 | Open device nRet = MV_CC_OpenDevice(handle); if (MV_OK != nRet) { printf("Open Device fail! nRet [0x%x]\n", nRet); break; }

int nWidth = 0; int nHeight = 0;

MVCC_INTVALUE_EX stIntEx = {0}; nRet = MV_CC_GetIntValueEx(handle, "Width", &stIntEx); if (MV_OK != nRet) { printf("Get IntValue fail! nRet [0x%x]\n", nRet); break; } nWidth = stIntEx.nCurValue;

memset(&stIntEx, 0, sizeof(MVCC_INTVALUE_EX)); nRet = MV_CC_GetIntValueEx(handle, "Height", &stIntEx); if (MV_OK != nRet) { printf("Get IntValue fail! nRet [0x%x]\n", nRet); break; }

nHeight = stIntEx.nCurValue;

nRet = MV_CC_GetIntValueEx(handle, "PayloadSize", &stIntEx); if (MV_OK != nRet) { printf("Get IntValue fail! nRet [0x%x], use [%d] replace.\n", nRet, nHeight*nWidth * 3); m_nImageSize = nHeight*nWidth*3; } else { m_nImageSize = stIntEx.nCurValue; }

//初始化队列 m_queue = new (std::nothrow)ArrayQueue(); if (m_queue==NULL) { printf("m_queue is null! \n"); break; } nRet = m_queue->Init(Max_Count,m_nImageSize); if (MV_OK != nRet) { printf("ArrayQueue init fail! nRet [0x%x]\n", nRet); break; }

// ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera) if (stDeviceList.pDeviceInfo[nIndex]->nTLayerType == MV_GIGE_DEVICE) { int nPacketSize = MV_CC_GetOptimalPacketSize(handle); if (nPacketSize > 0) { nRet = MV_CC_SetIntValueEx(handle,"GevSCPSPacketSize",nPacketSize); if(nRet != MV_OK) { printf("Warning: Set Packet Size fail nRet [0x%x]!", nRet); } } else { printf("Warning: Get Packet Size fail nRet [0x%x]!", nPacketSize); } }

// ch:设置触发模式为off | eb:Set trigger mode as off nRet = MV_CC_SetEnumValue(handle, "TriggerMode", MV_TRIGGER_MODE_OFF); if (MV_OK != nRet) { printf("Set Trigger Mode fail! nRet [0x%x]\n", nRet); break; }

// ch:注册抓图回调 | en:Register image callback nRet = MV_CC_RegisterImageCallBackEx(handle, ImageCallBackEx, handle); if (MV_OK != nRet) { printf("Register Image CallBack fail! nRet [0x%x]\n", nRet); break; }

unsigned int nThreadID = 0; void* hThreadHandle = (void*) _beginthreadex( NULL , 0 , WorkThread , handle, 0 , &nThreadID ); if (NULL == hThreadHandle) { printf("Start work thread fail! \n"); break; }

// ch:开始取流 | en:Start grab image nRet = MV_CC_StartGrabbing(handle); if (MV_OK != nRet) { printf("Start Grabbing fail! nRet [0x%x]\n", nRet); break; }

printf("Press a key to stop grabbing.\n"); WaitForKeyPress();

g_bExit = true; Sleep(1000);

// ch:停止取流 | en:Stop grab image nRet = MV_CC_StopGrabbing(handle); if (MV_OK != nRet) { printf("Stop Grabbing fail! nRet [0x%x]\n", nRet); break; }

// ch:注销抓图回调 | en:Unregister image callback nRet = MV_CC_RegisterImageCallBackEx(handle, NULL, NULL); if (MV_OK != nRet) { printf("Unregister Image CallBack fail! nRet [0x%x]\n", nRet); break; }

// ch:关闭设备 | en:Close device nRet = MV_CC_CloseDevice(handle); if (MV_OK != nRet) { printf("Close Device fail! nRet [0x%x]\n", nRet); break; }

// ch:销毁句柄 | en:Destroy handle nRet = MV_CC_DestroyHandle(handle); if (MV_OK != nRet) { printf("Destroy Handle fail! nRet [0x%x]\n", nRet); break; } handle = NULL;

} while (0);

if (m_queue) { delete m_queue; m_queue = NULL; }

printf("free buffer done\n");

if (handle != NULL) { MV_CC_DestroyHandle(handle); handle = NULL; }

// ch:反初始化SDK | en:Finalize SDK

MV_CC_Finalize();

if(g_mutex) { delete g_mutex; g_mutex = NULL; }

printf("Press a key to exit.\n"); WaitForKeyPress();

return 0; }

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

### Grab_Callback.cpp

通过回调获取相机图像

该示例程序说明如何通过回调的方式获取相机图像。创建句柄，打开相机，注册回调函数，开始取 流，在回调函数中可以获取到图像相关信息，停止取流，关闭相机，销毁句柄。

#include <stdio.h> #include <Windows.h>

#include <conio.h> #include "MvCameraControl.h"

// ch:等待按键输入 | en:Wait for key press void WaitForKeyPress(void) { while(!_kbhit()) { Sleep(10); } _getch(); }

bool PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo) { if (NULL == pstMVDevInfo) { printf("The Pointer of pstMVDevInfo is NULL!\n"); return false; } if (pstMVDevInfo->nTLayerType == MV_GIGE_DEVICE) { int nIp1 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24); int nIp2 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16); int nIp3 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8); int nIp4 = (pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

// ch:打印当前相机ip和用户自定义名字 | en:print current ip and user defined name printf("CurrentIp: %d.%d.%d.%d\n" , nIp1, nIp2, nIp3, nIp4); printf("UserDefinedName: %s\n\n" , pstMVDevInfo- >SpecialInfo.stGigEInfo.chUserDefinedName); } else if (pstMVDevInfo->nTLayerType == MV_USB_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.chSerialNumber); printf("Device Number: %d\n\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.nDeviceNumber); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_GIGE_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chModelName);

} else if (pstMVDevInfo->nTLayerType == MV_GENTL_CAMERALINK_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_CXP_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_XOF_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chModelName); } else { printf("Not support.\n"); }

return true; }

void __stdcall ImageCallBackEx(unsigned char * pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser) { if (pFrameInfo) { printf("Get One Frame: Width[%d], Height[%d], nFrameNum[%d]\n", pFrameInfo->nWidth, pFrameInfo->nHeight, pFrameInfo- >nFrameNum); } }

int main() { int nRet = MV_OK; void* handle = NULL;

do

{ // ch:初始化SDK | en:Initialize SDK nRet = MV_CC_Initialize(); if (MV_OK != nRet) { printf("Initialize SDK fail! nRet [0x%x]\n", nRet); break; }

// ch:枚举设备 | Enum device MV_CC_DEVICE_INFO_LIST stDeviceList; memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST)); nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE | MV_GENTL_CAMERALINK_DEVICE | MV_GENTL_CXP_DEVICE | MV_GENTL_XOF_DEVICE, &stDeviceList); if (MV_OK != nRet) { printf("Enum Devices fail! nRet [0x%x]\n", nRet); break; }

if (stDeviceList.nDeviceNum > 0) { for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++) { printf("[device %d]:\n", i); MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i]; if (NULL == pDeviceInfo) { break; } PrintDeviceInfo(pDeviceInfo); } } else { printf("Find No Devices!\n"); break; }

printf("Please Input camera index(0-%d):", stDeviceList.nDeviceNum-1); unsigned int nIndex = 0; scanf_s("%d", &nIndex);

if (nIndex >= stDeviceList.nDeviceNum) { printf("Input error!\n"); break; }

// ch:选择设备并创建句柄 | Select device and create handle nRet = MV_CC_CreateHandle(&handle, stDeviceList.pDeviceInfo[nIndex]); if (MV_OK != nRet) { printf("Create Handle fail! nRet [0x%x]\n", nRet); break; }

// ch:打开设备 | Open device nRet = MV_CC_OpenDevice(handle); if (MV_OK != nRet) { printf("Open Device fail! nRet [0x%x]\n", nRet); break; }

// ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera) if (stDeviceList.pDeviceInfo[nIndex]->nTLayerType == MV_GIGE_DEVICE) { int nPacketSize = MV_CC_GetOptimalPacketSize(handle); if (nPacketSize > 0) { nRet = MV_CC_SetIntValueEx(handle,"GevSCPSPacketSize",nPacketSize); if(nRet != MV_OK) { printf("Warning: Set Packet Size fail nRet [0x%x]!", nRet); } } else { printf("Warning: Get Packet Size fail nRet [0x%x]!", nPacketSize); } }

// ch:设置触发模式为off | eb:Set trigger mode as off nRet = MV_CC_SetEnumValue(handle, "TriggerMode", MV_TRIGGER_MODE_OFF); if (MV_OK != nRet) { printf("Set Trigger Mode fail! nRet [0x%x]\n", nRet); break; }

// ch:注册抓图回调 | en:Register image callback nRet = MV_CC_RegisterImageCallBackEx(handle, ImageCallBackEx, handle);

if (MV_OK != nRet) { printf("Register Image CallBack fail! nRet [0x%x]\n", nRet); break; }

// ch:开始取流 | en:Start grab image nRet = MV_CC_StartGrabbing(handle); if (MV_OK != nRet) { printf("Start Grabbing fail! nRet [0x%x]\n", nRet); break; }

printf("Press a key to stop grabbing.\n"); WaitForKeyPress();

// ch:停止取流 | en:Stop grab image nRet = MV_CC_StopGrabbing(handle); if (MV_OK != nRet) { printf("Stop Grabbing fail! nRet [0x%x]\n", nRet); break; }

// ch:注销抓图回调 | en:Unregister image callback nRet = MV_CC_RegisterImageCallBackEx(handle, NULL, NULL); if (MV_OK != nRet) { printf("Unregister Image CallBack fail! nRet [0x%x]\n", nRet); break; }

// ch:关闭设备 | en:Close device nRet = MV_CC_CloseDevice(handle); if (MV_OK != nRet) { printf("Close Device fail! nRet [0x%x]\n", nRet); break; }

// ch:销毁句柄 | en:Destroy handle nRet = MV_CC_DestroyHandle(handle); if (MV_OK != nRet) { printf("Destroy Handle fail! nRet [0x%x]\n", nRet); break; } handle = NULL;

} while (0);

if (handle != NULL) { MV_CC_DestroyHandle(handle); handle = NULL; }

// ch:反初始化SDK | en:Finalize SDK MV_CC_Finalize();

printf("Press a key to exit.\n"); WaitForKeyPress();

return 0; }

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

### GrabImage.cpp

获取相机图像

该示例程序说明如何通过主动取图的方式获取相机图像。创建句柄，打开相机，开始取流，创建取 流线程（用户分配采集buffer， MV_CC_GetOneFrameTimeout() 取图（拷贝）），停止取流，关 闭相机，销毁句柄。

#include <stdio.h> #include <Windows.h> #include <process.h> #include <conio.h> #include "MvCameraControl.h"

bool g_bExit = false;

// ch:等待按键输入 | en:Wait for key press void WaitForKeyPress(void) { while(!_kbhit()) { Sleep(10); } _getch(); }

bool PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo) { if (NULL == pstMVDevInfo) { printf("The Pointer of pstMVDevInfo is NULL!\n"); return false; } if (pstMVDevInfo->nTLayerType == MV_GIGE_DEVICE) { int nIp1 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24); int nIp2 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16); int nIp3 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8); int nIp4 = (pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

// ch:打印当前相机ip和用户自定义名字 | en:print current ip and user defined name printf("CurrentIp: %d.%d.%d.%d\n" , nIp1, nIp2, nIp3, nIp4); printf("UserDefinedName: %s\n\n" , pstMVDevInfo- >SpecialInfo.stGigEInfo.chUserDefinedName); } else if (pstMVDevInfo->nTLayerType == MV_USB_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.chUserDefinedName);

printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.chSerialNumber); printf("Device Number: %d\n\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.nDeviceNumber); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_GIGE_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_CAMERALINK_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_CXP_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_XOF_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chModelName); } else { printf("Not support.\n"); }

return true; }

static unsigned int __stdcall WorkThread(void* pUser) { int nRet = MV_OK; MV_FRAME_OUT stOutFrame = {0};

while(true) { nRet = MV_CC_GetImageBuffer(pUser, &stOutFrame, 1000); if (nRet == MV_OK) { printf("Get Image Buffer: Width[%d], Height[%d], FrameNum[%d]\n", stOutFrame.stFrameInfo.nWidth, stOutFrame.stFrameInfo.nHeight, stOutFrame.stFrameInfo.nFrameNum);

nRet = MV_CC_FreeImageBuffer(pUser, &stOutFrame); if(nRet != MV_OK) { printf("Free Image Buffer fail! nRet [0x%x]\n", nRet); } } else { printf("Get Image fail! nRet [0x%x]\n", nRet); } if(g_bExit) { break; } }

return 0; }

int main() { int nRet = MV_OK; void* handle = NULL;

do { // ch:初始化SDK | en:Initialize SDK nRet = MV_CC_Initialize(); if (MV_OK != nRet) { printf("Initialize SDK fail! nRet [0x%x]\n", nRet); break; }

// ch:枚举设备 | en:Enum device MV_CC_DEVICE_INFO_LIST stDeviceList; memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST)); nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE | MV_GENTL_CAMERALINK_DEVICE | MV_GENTL_CXP_DEVICE | MV_GENTL_XOF_DEVICE, &stDeviceList); if (MV_OK != nRet)

{ printf("Enum Devices fail! nRet [0x%x]\n", nRet); break; }

if (stDeviceList.nDeviceNum > 0) { for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++) { printf("[device %d]:\n", i); MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i]; if (NULL == pDeviceInfo) { break; } PrintDeviceInfo(pDeviceInfo); } } else { printf("Find No Devices!\n"); break; }

printf("Please Input camera index(0-%d):", stDeviceList.nDeviceNum-1); unsigned int nIndex = 0; scanf_s("%d", &nIndex);

if (nIndex >= stDeviceList.nDeviceNum) { printf("Input error!\n"); break; }

// ch:选择设备并创建句柄 | en:Select device and create handle nRet = MV_CC_CreateHandle(&handle, stDeviceList.pDeviceInfo[nIndex]); if (MV_OK != nRet) { printf("Create Handle fail! nRet [0x%x]\n", nRet); break; }

// ch:打开设备 | en:Open device nRet = MV_CC_OpenDevice(handle); if (MV_OK != nRet) { printf("Open Device fail! nRet [0x%x]\n", nRet); break; }

// ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera) if (stDeviceList.pDeviceInfo[nIndex]->nTLayerType == MV_GIGE_DEVICE) { int nPacketSize = MV_CC_GetOptimalPacketSize(handle); if (nPacketSize > 0) { nRet = MV_CC_SetIntValueEx(handle,"GevSCPSPacketSize",nPacketSize); if(nRet != MV_OK) { printf("Warning: Set Packet Size fail nRet [0x%x]!", nRet); } } else { printf("Warning: Get Packet Size fail nRet [0x%x]!", nPacketSize); } }

// ch:设置触发模式为off | en:Set trigger mode as off nRet = MV_CC_SetEnumValue(handle, "TriggerMode", 0); if (MV_OK != nRet) { printf("Set Trigger Mode fail! nRet [0x%x]\n", nRet); break; }

// ch:开始取流 | en:Start grab image nRet = MV_CC_StartGrabbing(handle); if (MV_OK != nRet) { printf("Start Grabbing fail! nRet [0x%x]\n", nRet); break; }

unsigned int nThreadID = 0; void* hThreadHandle = (void*) _beginthreadex( NULL , 0 , WorkThread , handle, 0 , &nThreadID ); if (NULL == hThreadHandle) { break; }

printf("Press a key to stop grabbing.\n"); WaitForKeyPress();

g_bExit = true; Sleep(1000);

// ch:停止取流 | en:Stop grab image nRet = MV_CC_StopGrabbing(handle); if (MV_OK != nRet) { printf("Stop Grabbing fail! nRet [0x%x]\n", nRet); break; }

// ch:关闭设备 | Close device nRet = MV_CC_CloseDevice(handle); if (MV_OK != nRet) { printf("ClosDevice fail! nRet [0x%x]\n", nRet); break; }

// ch:销毁句柄 | Destroy handle nRet = MV_CC_DestroyHandle(handle); if (MV_OK != nRet) { printf("Destroy Handle fail! nRet [0x%x]\n", nRet); break; } handle = NULL; } while (0);

if (handle != NULL) { MV_CC_DestroyHandle(handle); handle = NULL; }

// ch:反初始化SDK | en:Finalize SDK MV_CC_Finalize();

printf("Press a key to exit.\n"); WaitForKeyPress();

return 0; }

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

### GrabImage_Display.cpp

图像采集并显示

该示例程序演示如何取图并显示取到的每一帧图像。

#include <stdio.h> #include <process.h> #include <conio.h> #include "windows.h" #include "MvCameraControl.h"

HWND g_hwnd = NULL; bool g_bExit = false;

// ch:等待按键输入 | en:Wait for key press void WaitForKeyPress(void) { while(!_kbhit()) { Sleep(10); } _getch(); }

bool PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo) { if (NULL == pstMVDevInfo) { printf("The Pointer of pstMVDevInfo is NULL!\n"); return false; } if (pstMVDevInfo->nTLayerType == MV_GIGE_DEVICE) { int nIp1 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24); int nIp2 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16); int nIp3 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8); int nIp4 = (pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

// ch:打印当前相机ip和用户自定义名字 | en:print current ip and user defined name printf("CurrentIp: %d.%d.%d.%d\n" , nIp1, nIp2, nIp3, nIp4); printf("UserDefinedName: %s\n\n" , pstMVDevInfo- >SpecialInfo.stGigEInfo.chUserDefinedName); } else if (pstMVDevInfo->nTLayerType == MV_USB_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.chSerialNumber); printf("Device Number: %d\n\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.nDeviceNumber); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_GIGE_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_CAMERALINK_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_CXP_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chUserDefinedName);

printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_XOF_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chModelName); } else { printf("Not support.\n"); }

return true; }

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) { switch(msg) { case WM_DESTROY: PostQuitMessage(0); g_hwnd = NULL; break; }

return DefWindowProc(hWnd, msg, wParam, lParam); }

static unsigned int __stdcall CreateRenderWindow(void* pUser) { HINSTANCE hInstance = ::GetModuleHandle(NULL); //获取应 用程序的模块句柄 WNDCLASSEX wc; wc.cbSize = sizeof(wc); wc.style = CS_HREDRAW | CS_VREDRAW; //窗口的 风格 wc.cbClsExtra = 0; wc.cbWndExtra = 0; wc.hInstance = hInstance; wc.hIcon = ::LoadIcon(NULL, IDI_APPLICATION); //图标风 格 wc.hIconSm = ::LoadIcon( NULL, IDI_APPLICATION ); wc.hbrBackground = (HBRUSH)( COLOR_WINDOW + 1); //背景色

wc.hCursor = ::LoadCursor(NULL, IDC_ARROW); //鼠标风 格 wc.lpfnWndProc = WndProc; //自定义 消息处理函数 wc.lpszMenuName = NULL; wc.lpszClassName = "RenderWindow"; //该窗口 类的名称

if(!RegisterClassEx(&wc)) { return 0; }

DWORD style = WS_OVERLAPPEDWINDOW; DWORD styleEx = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE; RECT rect = {0, 0, 640, 480};

AdjustWindowRectEx(&rect, style, false, styleEx);

HWND hWnd = CreateWindowEx(styleEx, "RenderWindow", "Display", style, 0, 0, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, hInstance, NULL); if(hWnd == NULL) { return 0; }

::UpdateWindow(hWnd); ::ShowWindow(hWnd, SW_SHOW);

g_hwnd = hWnd;

MSG msg = {0}; while(msg.message != WM_QUIT) { if(PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) { TranslateMessage(&msg); DispatchMessage(&msg); } }

return 0; }

static unsigned int __stdcall WorkThread(void* pUser)

{ int nRet = MV_OK; MV_FRAME_OUT stImageInfo = {0}; MV_CC_IMAGE stImageData = { 0 };

while(1) { nRet = MV_CC_GetImageBuffer(pUser, &stImageInfo, 1000); if (nRet == MV_OK) { printf("Get Image Buffer: Width[%d], Height[%d], FrameNum[%d]\n", stImageInfo.stFrameInfo.nExtendWidth, stImageInfo.stFrameInfo.nExtendHeight, stImageInfo.stFrameInfo.nFrameNum);

if (g_hwnd) { stImageData.nWidth = stImageInfo.stFrameInfo.nExtendWidth; stImageData.nHeight = stImageInfo.stFrameInfo.nExtendHeight; stImageData.enPixelType = stImageInfo.stFrameInfo.enPixelType; stImageData.nImageBufLen = stImageInfo.stFrameInfo.nFrameLenEx; stImageData.pImageBuf = stImageInfo.pBufAddr; MV_CC_DisplayOneFrameEx2(pUser, g_hwnd, &stImageData,0); }

nRet = MV_CC_FreeImageBuffer(pUser, &stImageInfo); if(nRet != MV_OK) { printf("Free Image Buffer fail! nRet [0x%x]\n", nRet); } } else { printf("Get Image fail! nRet [0x%x]\n", nRet); } if(g_bExit) { break; } }

return 0; }

int main() { int nRet = MV_OK;

void* handle = NULL;

do { // ch:初始化SDK | en:Initialize SDK nRet = MV_CC_Initialize(); if (MV_OK != nRet) { printf("Initialize SDK fail! nRet [0x%x]\n", nRet); break; }

// ch:枚举设备 | en:Enum device MV_CC_DEVICE_INFO_LIST stDeviceList = {0}; nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE | MV_GENTL_CAMERALINK_DEVICE | MV_GENTL_CXP_DEVICE | MV_GENTL_XOF_DEVICE, &stDeviceList); if (MV_OK != nRet) { printf("Enum Devices fail! nRet [0x%x]\n", nRet); break; }

if (stDeviceList.nDeviceNum > 0) { for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++) { printf("[device %d]:\n", i); MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i]; if (NULL == pDeviceInfo) { break; } PrintDeviceInfo(pDeviceInfo); } } else { printf("Find No Devices!\n"); break; }

printf("Please Input camera index(0-%d):", stDeviceList.nDeviceNum-1); unsigned int nIndex = 0; scanf_s("%d", &nIndex);

if (nIndex >= stDeviceList.nDeviceNum) { printf("Input error!\n"); break;

}

// ch:选择设备并创建句柄 | en:Select device and create handle nRet = MV_CC_CreateHandle(&handle, stDeviceList.pDeviceInfo[nIndex]); if (MV_OK != nRet) { printf("Create Handle fail! nRet [0x%x]\n", nRet); break; }

// ch:打开设备 | en:Open device nRet = MV_CC_OpenDevice(handle); if (MV_OK != nRet) { printf("Open Device fail! nRet [0x%x]\n", nRet); break; }

// ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera) if (stDeviceList.pDeviceInfo[nIndex]->nTLayerType == MV_GIGE_DEVICE) { int nPacketSize = MV_CC_GetOptimalPacketSize(handle); if (nPacketSize > 0) { nRet = MV_CC_SetIntValueEx(handle,"GevSCPSPacketSize",nPacketSize); if(nRet != MV_OK) { printf("Warning: Set Packet Size fail nRet [0x%x]!", nRet); } } else { printf("Warning: Get Packet Size fail nRet [0x%x]!", nPacketSize); } }

// ch:设置触发模式为off | en:Set trigger mode as off nRet = MV_CC_SetEnumValue(handle, "TriggerMode", 0); if (MV_OK != nRet) { printf("Set Trigger Mode fail! nRet [0x%x]\n", nRet); break; }

unsigned int nThreadID = 0;

void* hCreateWindow = (void*) _beginthreadex( NULL , 0 , CreateRenderWindow , handle, 0 , &nThreadID); if (NULL == hCreateWindow) { break; }

// ch:开始取流 | en:Start grab image nRet = MV_CC_StartGrabbing(handle); if (MV_OK != nRet) { printf("Start Grabbing fail! nRet [0x%x]\n", nRet); break; }

nThreadID = 0; void* hThreadHandle = (void*) _beginthreadex( NULL , 0 , WorkThread , handle, 0 , &nThreadID); if (NULL == hThreadHandle) { break; }

printf("Press a key to stop grabbing.\n"); WaitForKeyPress();

g_bExit = true; WaitForSingleObject(hThreadHandle, INFINITE); CloseHandle(hThreadHandle);

// ch:停止取流 | en:Stop grab image nRet = MV_CC_StopGrabbing(handle); if (MV_OK != nRet) { printf("Stop Grabbing fail! nRet [0x%x]\n", nRet); break; }

// ch:关闭设备 | Close device nRet = MV_CC_CloseDevice(handle); if (MV_OK != nRet) { printf("ClosDevice fail! nRet [0x%x]\n", nRet); break; }

// ch:销毁句柄 | Destroy handle nRet = MV_CC_DestroyHandle(handle); if (MV_OK != nRet)

{ printf("Destroy Handle fail! nRet [0x%x]\n", nRet); break; } handle = NULL; } while (0);

if (handle != NULL) { MV_CC_DestroyHandle(handle); handle = NULL; }

// ch:反初始化SDK | en:Finalize SDK MV_CC_Finalize();

printf("Press a key to exit.\n"); WaitForKeyPress();

return 0; }

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

### GrabStrategies.cpp

取图策略

该示例程序说明如何使用不同取图策略从相机去取图。

#include <stdio.h> #include <Windows.h> #include <process.h> #include <conio.h> #include "MvCameraControl.h"

// ch:等待按键输入 | en:Wait for key press void WaitForKeyPress(void) { while(!_kbhit()) { Sleep(10); } _getch(); }

bool PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo) { if (NULL == pstMVDevInfo) { printf("The Pointer of pstMVDevInfo is NULL!\n"); return false; }

if (pstMVDevInfo->nTLayerType == MV_GIGE_DEVICE) { int nIp1 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24); int nIp2 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16); int nIp3 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8); int nIp4 = (pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

printf("CurrentIp: %d.%d.%d.%d\n" , nIp1, nIp2, nIp3, nIp4); printf("UserDefinedName: %s\n\n" , pstMVDevInfo- >SpecialInfo.stGigEInfo.chUserDefinedName); } else if (pstMVDevInfo->nTLayerType == MV_USB_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.chUserDefinedName);

printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.chSerialNumber); printf("Device Number: %d\n\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.nDeviceNumber); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_GIGE_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_CAMERALINK_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_CXP_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_XOF_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chModelName); } else { printf("Not support.\n"); }

return true; }

static unsigned int __stdcall UpcomingThread(void* pUser) { Sleep(3000);//为了等MV_CC_GetImageBuffer调用后再发送软触发命令

printf("Trigger Software Once for MV_GrabStrategy_UpcomingImage\n");

MV_CC_SetCommandValue(pUser, "TriggerSoftware");

return 0; }

int main() { int nRet = MV_OK; void* handle = NULL; unsigned char * pData = NULL;

do { // ch:初始化SDK | en:Initialize SDK nRet = MV_CC_Initialize(); if (MV_OK != nRet) { printf("Initialize SDK fail! nRet [0x%x]\n", nRet); break; }

// ch:枚举设备 | en:Enum device MV_CC_DEVICE_INFO_LIST stDeviceList = {0}; nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE | MV_GENTL_CAMERALINK_DEVICE | MV_GENTL_CXP_DEVICE | MV_GENTL_XOF_DEVICE, &stDeviceList); if (MV_OK != nRet) { printf("Enum Devices fail! nRet [0x%x]\n", nRet); break; }

if (stDeviceList.nDeviceNum > 0) { for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++) { printf("[device %d]:\n", i); MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i]; if (NULL == pDeviceInfo) { break; } PrintDeviceInfo(pDeviceInfo); } } else { printf("Find No Devices!\n"); break; }

printf("Please Input camera index(0-%d):", stDeviceList.nDeviceNum-1); unsigned int nIndex = 0; scanf_s("%d", &nIndex);

if (nIndex >= stDeviceList.nDeviceNum) { printf("Input error!\n"); break; }

// ch:选择设备并创建句柄 | en:Select device and create handle nRet = MV_CC_CreateHandle(&handle, stDeviceList.pDeviceInfo[nIndex]); if (MV_OK != nRet) { printf("Create Handle fail! nRet [0x%x]\n", nRet); break; }

// ch:打开设备 | en:Open device nRet = MV_CC_OpenDevice(handle); if (MV_OK != nRet) { printf("Open Device fail! nRet [0x%x]\n", nRet); break; }

// ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera) if (stDeviceList.pDeviceInfo[nIndex]->nTLayerType == MV_GIGE_DEVICE) { int nPacketSize = MV_CC_GetOptimalPacketSize(handle); if (nPacketSize > 0) {

if(MV_CC_SetIntValueEx(handle,"GevSCPSPacketSize",nPacketSize) != MV_OK) { printf("Warning: Set Packet Size fail nRet [0x%x]!", nRet); } } else { printf("Warning: Get Packet Size fail nRet [0x%x]!", nPacketSize); } }

// ch:设置软触发模式 | en:Set Trigger Mode and Set Trigger Source nRet = MV_CC_SetEnumValueByString(handle, "TriggerMode", "On"); if (MV_OK != nRet) { printf("Set Trigger Mode fail! nRet [0x%x]\n", nRet); break; } nRet = MV_CC_SetEnumValueByString(handle, "TriggerSource", "Software"); if (MV_OK != nRet) { printf("Set Trigger Source fail! nRet [0x%x]\n", nRet); break; }

unsigned int nImageNodeNum = 5; // ch:设置缓存节点个数 | en:Set number of image node nRet = MV_CC_SetImageNodeNum(handle, nImageNodeNum); if (MV_OK != nRet) { printf("Set number of image node fail! nRet [0x%x]\n", nRet); break; }

printf("\n******************************************************* *******************\n"); printf("* 0.MV_GrabStrategy_OneByOne; 1.MV_GrabStrategy_LatestImagesOnly; *\n"); printf("* 2.MV_GrabStrategy_LatestImages; 3.MV_GrabStrategy_UpcomingImage; *\n");

printf("********************************************************* *****************\n");

printf("Please Input Grab Strategy:"); unsigned int nGrabStrategy = 0; scanf_s("%d", &nGrabStrategy);

// ch:U3V相机不支持MV_GrabStrategy_UpcomingImage | en:U3V device not support UpcomingImage if (nGrabStrategy == MV_GrabStrategy_UpcomingImage && MV_USB_DEVICE == stDeviceList.pDeviceInfo[nIndex]->nTLayerType) { printf("U3V device not support UpcomingImage\n"); break; }

switch(nGrabStrategy)

{ case MV_GrabStrategy_OneByOne: { printf("Grab using the MV_GrabStrategy_OneByOne default strategy\n"); nRet = MV_CC_SetGrabStrategy(handle, MV_GrabStrategy_OneByOne); if (MV_OK != nRet) { printf("Set Grab Strategy fail! nRet [0x%x]\n", nRet); break; } } break; case MV_GrabStrategy_LatestImagesOnly: { printf("Grab using strategy MV_GrabStrategy_LatestImagesOnly\n"); nRet = MV_CC_SetGrabStrategy(handle, MV_GrabStrategy_LatestImagesOnly); if (MV_OK != nRet) { printf("Set Grab Strategy fail! nRet [0x%x]\n", nRet); break; } } break; case MV_GrabStrategy_LatestImages: { printf("Grab using strategy MV_GrabStrategy_LatestImages\n"); nRet = MV_CC_SetGrabStrategy(handle, MV_GrabStrategy_LatestImages); if (MV_OK != nRet) { printf("Set Grab Strategy fail! nRet [0x%x]\n", nRet); break; }

// ch:设置输出缓存个数 | en:Set Output Queue Size nRet = MV_CC_SetOutputQueueSize(handle, 2); if (MV_OK != nRet) { printf("Set Output Queue Size fail! nRet [0x%x]\n", nRet); break; } } break; case MV_GrabStrategy_UpcomingImage: { printf("Grab using strategy MV_GrabStrategy_UpcomingImage\n");

nRet = MV_CC_SetGrabStrategy(handle, MV_GrabStrategy_UpcomingImage); if (MV_OK != nRet) { printf("Set Grab Strategy fail! nRet [0x%x]\n", nRet); break; }

unsigned int nThreadID = 0; void* hThreadHandle = (void*) _beginthreadex( NULL , 0 , UpcomingThread , handle, 0 , &nThreadID ); if (NULL == hThreadHandle) { break; } } break; default: printf("Input error!Use default strategy:MV_GrabStrategy_OneByOne\n"); break; }

// ch:开始取流 | en:Start grab image nRet = MV_CC_StartGrabbing(handle); if (MV_OK != nRet) { printf("Start Grabbing fail! nRet [0x%x]\n", nRet); break; }

// ch:发送软触发命令 | en:Send Trigger Software command for (unsigned int i = 0;i < nImageNodeNum;i++) { nRet = MV_CC_SetCommandValue(handle, "TriggerSoftware"); if (MV_OK != nRet) { printf("Send Trigger Software command fail! nRet [0x%x]\n", nRet); break; } Sleep(500);//如果帧率过小或TriggerDelay很大，可能会出现软触发命令没 有全部起效而导致取不到数据的情况 }

MV_FRAME_OUT stOutFrame = {0}; if (nGrabStrategy != MV_GrabStrategy_UpcomingImage) { while(true) {

nRet = MV_CC_GetImageBuffer(handle, &stOutFrame, 0);//超 时时间设置成0，因为缓存列表中已经有数据存在 if (nRet == MV_OK) { printf("Get One Frame: Width[%d], Height[%d], FrameNum[%d]\n", stOutFrame.stFrameInfo.nWidth, stOutFrame.stFrameInfo.nHeight, stOutFrame.stFrameInfo.nFrameNum); } else { printf("Get Image fail! nRet [0x%x]\n", nRet); break; }

nRet = MV_CC_FreeImageBuffer(handle, &stOutFrame); if(nRet != MV_OK) { printf("Free Image Buffer fail! nRet [0x%x]\n", nRet); } } } else//仅用于upcoming { nRet = MV_CC_GetImageBuffer(handle, &stOutFrame, 5000);//需要 比较大的超时时间来获取即将到达的这帧数据 if (nRet == MV_OK) { printf("Get One Frame: Width[%d], Height[%d], FrameNum[%d]\n", stOutFrame.stFrameInfo.nWidth, stOutFrame.stFrameInfo.nHeight, stOutFrame.stFrameInfo.nFrameNum);

nRet = MV_CC_FreeImageBuffer(handle, &stOutFrame); if(nRet != MV_OK) { printf("Free Image Buffer fail! nRet [0x%x]\n", nRet); } } else { printf("Get Image fail! nRet [0x%x]\n", nRet); } }

// ch:停止取流 | en:Stop grab image nRet = MV_CC_StopGrabbing(handle); if (MV_OK != nRet) {

printf("Stop Grabbing fail! nRet [0x%x]\n", nRet); break; }

// ch:关闭设备 | en:Close device nRet = MV_CC_CloseDevice(handle); if (MV_OK != nRet) { printf("Close Device fail! nRet [0x%x]\n", nRet); break; }

// ch:销毁句柄 | en:Destroy handle nRet = MV_CC_DestroyHandle(handle); if (MV_OK != nRet) { printf("Destroy Handle fail! nRet [0x%x]\n", nRet); break; } handle = NULL; } while (0);

if (handle != NULL) { MV_CC_DestroyHandle(handle); handle = NULL; }

// ch:反初始化SDK | en:Finalize SDK MV_CC_Finalize();

printf("Press a key to exit.\n"); WaitForKeyPress();

return 0; }

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

### HighBandwidthDecode.cpp

高清无损压缩

该示例程序演示通过高清无损压缩算法将从相机取到的无损压缩码流解码成裸数据。

#include <stdio.h> #include <Windows.h> #include <conio.h> #include "MvCameraControl.h"

#define IMAGE_NAME_LEN 256 // ch:等待按键输入 | en:Wait for key press void WaitForKeyPress(void) { while(!_kbhit()) { Sleep(10); } _getch(); }

bool PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo) { if (NULL == pstMVDevInfo) { printf("The Pointer of pstMVDevInfo is NULL!\n"); return false; } if (pstMVDevInfo->nTLayerType == MV_GIGE_DEVICE) { int nIp1 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24);

int nIp2 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16); int nIp3 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8); int nIp4 = (pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

// ch:打印当前相机ip和用户自定义名字 | en:print current ip and user defined name printf("CurrentIp: %d.%d.%d.%d\n" , nIp1, nIp2, nIp3, nIp4); printf("UserDefinedName: %s\n\n" , pstMVDevInfo- >SpecialInfo.stGigEInfo.chUserDefinedName); } else if (pstMVDevInfo->nTLayerType == MV_USB_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.chSerialNumber); printf("Device Number: %d\n\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.nDeviceNumber); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_GIGE_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_CAMERALINK_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_CXP_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_XOF_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chSerialNumber);

printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chModelName); } else { printf("Not support.\n"); }

return true; }

int main() { int nRet = MV_OK; void* handle = NULL; unsigned char * pDstBuf = NULL;

do { // ch:初始化SDK | en:Initialize SDK nRet = MV_CC_Initialize(); if (MV_OK != nRet) { printf("Initialize SDK fail! nRet [0x%x]\n", nRet); break; }

// ch:枚举设备 | en:Enum device MV_CC_DEVICE_INFO_LIST stDeviceList; memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST)); nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE | MV_GENTL_CAMERALINK_DEVICE | MV_GENTL_CXP_DEVICE | MV_GENTL_XOF_DEVICE, &stDeviceList); if (MV_OK != nRet) { printf("Enum Devices fail! nRet [0x%x]\n", nRet); break; }

if (stDeviceList.nDeviceNum > 0) { for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++) { printf("[device %d]:\n", i); MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i]; if (NULL == pDeviceInfo) { break; } PrintDeviceInfo(pDeviceInfo); }

} else { printf("Find No Devices!\n"); break; }

printf("Please Input camera index(0-%d):", stDeviceList.nDeviceNum-1); unsigned int nIndex = 0; scanf_s("%d", &nIndex);

if (nIndex >= stDeviceList.nDeviceNum) { printf("Input error!\n"); break; }

// ch:选择设备并创建句柄 | en:Select device and create handle nRet = MV_CC_CreateHandle(&handle, stDeviceList.pDeviceInfo[nIndex]); if (MV_OK != nRet) { printf("Create Handle fail! nRet [0x%x]\n", nRet); break; }

// ch:打开设备 | en:Open device nRet = MV_CC_OpenDevice(handle); if (MV_OK != nRet) { printf("Open Device fail! nRet [0x%x]\n", nRet); break; }

// ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera) if (stDeviceList.pDeviceInfo[nIndex]->nTLayerType == MV_GIGE_DEVICE) { int nPacketSize = MV_CC_GetOptimalPacketSize(handle); if (nPacketSize > 0) { nRet = MV_CC_SetIntValueEx(handle,"GevSCPSPacketSize",nPacketSize); if(nRet != MV_OK) { printf("Warning: Set Packet Size fail nRet [0x%x]!", nRet); } }

else { printf("Warning: Get Packet Size fail nRet [0x%x]!", nPacketSize); } }

nRet = MV_CC_SetEnumValue(handle, "TriggerMode", MV_TRIGGER_MODE_OFF); if (MV_OK != nRet) { printf("Set Trigger Mode fail! nRet [0x%x]\n", nRet); break; }

// ch:获取数据包大小 | en:Get payload size MVCC_INTVALUE stParam; memset(&stParam, 0, sizeof(MVCC_INTVALUE)); nRet = MV_CC_GetIntValue(handle, "PayloadSize", &stParam); if (MV_OK != nRet) { printf("Get PayloadSize fail! nRet [0x%x]\n", nRet); break; } unsigned int nPayloadSize = stParam.nCurValue;

// ch:开始取流 | en:Start grab image nRet = MV_CC_StartGrabbing(handle); if (MV_OK != nRet) { printf("Start Grabbing fail! nRet [0x%x]\n", nRet); break; }

MV_FRAME_OUT stImageInfo = {0}; unsigned int nImageNum = 10; char chImageName[IMAGE_NAME_LEN] = {0}; MV_CC_HB_DECODE_PARAM stDecodeParam = {0};

for(unsigned int i = 0;i < nImageNum; i++) { nRet = MV_CC_GetImageBuffer(handle, &stImageInfo, 1000); if (nRet == MV_OK) { printf("Get One Frame: Width[%d], Height[%d], FrameNum[%d]，PixelFormat[0x%x]\n", stImageInfo.stFrameInfo.nWidth, stImageInfo.stFrameInfo.nHeight, stImageInfo.stFrameInfo.nFrameNum, stImageInfo.stFrameInfo.enPixelType);

//ch:无损压缩解码 | en:Lossless compression decoding stDecodeParam.pSrcBuf = stImageInfo.pBufAddr; stDecodeParam.nSrcLen = stImageInfo.stFrameInfo.nFrameLenEx;

if (pDstBuf == NULL) { pDstBuf = (unsigned char *)malloc(sizeof(unsigned char) * (nPayloadSize)); if (NULL == pDstBuf) { printf("malloc pDstData fail !\n"); nRet = MV_E_RESOURCE; break; } } stDecodeParam.pDstBuf = pDstBuf; stDecodeParam.nDstBufSize = nPayloadSize; nRet = MV_CC_HB_Decode(handle, &stDecodeParam); if (nRet != MV_OK) { printf("Decode fail![0x%x]\n", nRet); break; }

MV_CC_FreeImageBuffer(handle, &stImageInfo);

// ch: 解码后的图保存至本地 | en:Save decode image MV_CC_IMAGE stImg; MV_CC_SAVE_IMAGE_PARAM stSaveParams; memset(&stSaveParams, 0, sizeof(MV_CC_SAVE_IMAGE_PARAM)); memset(&stImg, 0, sizeof(MV_CC_SAVE_IMAGE_PARAM)); stImg.enPixelType = stDecodeParam.enDstPixelType; stImg.nHeight = stDecodeParam.nHeight; stImg.nWidth = stDecodeParam.nWidth; stImg.nImageBufLen = stDecodeParam.nDstBufLen; stImg.pImageBuf = stDecodeParam.pDstBuf;

stSaveParams.enImageType = MV_Image_Bmp; stSaveParams.iMethodValue = 1;

sprintf_s(chImageName, IMAGE_NAME_LEN, "Image_w%d_h%d_fn%03d.bmp", stDecodeParam.nWidth, stDecodeParam.nHeight, stImageInfo.stFrameInfo.nFrameNum);

nRet = MV_CC_SaveImageToFileEx2(handle, &stImg, &stSaveParams, chImageName); if (nRet!=MV_OK) {

printf("Save image fail! nRet [0x%x]\n", nRet); } printf("Decode succeed\n"); } else { printf("Get Image fail! nRet [0x%x]\n", nRet); } }

// ch:停止取流 | en:Stop grab image nRet = MV_CC_StopGrabbing(handle); if (MV_OK != nRet) { printf("Stop Grabbing fail! nRet [0x%x]\n", nRet); break; }

// ch:关闭设备 | en:Close device nRet = MV_CC_CloseDevice(handle); if (MV_OK != nRet) { printf("Close Device fail! nRet [0x%x]\n", nRet); break; }

// ch:销毁句柄 | en:Destroy handle nRet = MV_CC_DestroyHandle(handle); if (MV_OK != nRet) { printf("Destroy Handle fail! nRet [0x%x]\n", nRet); break; } handle = NULL; } while (0);

if(pDstBuf) { free(pDstBuf); pDstBuf = NULL; }

if (handle != NULL) { MV_CC_DestroyHandle(handle); handle = NULL; }

// ch:反初始化SDK | en:Finalize SDK MV_CC_Finalize();

printf("Press a key to exit.\n"); WaitForKeyPress();

return 0; }

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

### ImageContrast.cpp

图像对比度调节

该示例程序说明如何采集图像并进行图像对比度调节。

#include <stdio.h> #include <Windows.h> #include <conio.h> #include "MvCameraControl.h"

// ch:等待按键输入 | en:Wait for key press void WaitForKeyPress(void) { while(!_kbhit())

{ Sleep(10); } _getch(); }

bool PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo) { if (NULL == pstMVDevInfo) { printf("The Pointer of pstMVDevInfo is NULL!\n"); return false; } if (pstMVDevInfo->nTLayerType == MV_GIGE_DEVICE) { int nIp1 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24); int nIp2 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16); int nIp3 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8); int nIp4 = (pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

// ch:打印当前相机ip和用户自定义名字 | en:print current ip and user defined name printf("CurrentIp: %d.%d.%d.%d\n" , nIp1, nIp2, nIp3, nIp4); printf("UserDefinedName: %s\n\n" , pstMVDevInfo- >SpecialInfo.stGigEInfo.chUserDefinedName); } else if (pstMVDevInfo->nTLayerType == MV_USB_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.chSerialNumber); printf("Device Number: %d\n\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.nDeviceNumber); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_GIGE_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_CAMERALINK_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chSerialNumber);

printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_CXP_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_XOF_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chModelName); } else { printf("Not support.\n"); }

return true; }

int main() { int nRet = MV_OK; void* handle = NULL; unsigned char * pDstData = NULL;

do { // ch:初始化SDK | en:Initialize SDK nRet = MV_CC_Initialize(); if (MV_OK != nRet) { printf("Initialize SDK fail! nRet [0x%x]\n", nRet); break; }

// ch:枚举设备 | en:Enum device MV_CC_DEVICE_INFO_LIST stDeviceList; memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST)); nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE | MV_GENTL_CAMERALINK_DEVICE | MV_GENTL_CXP_DEVICE | MV_GENTL_XOF_DEVICE, &stDeviceList); if (MV_OK != nRet) {

printf("Enum Devices fail! nRet [0x%x]\n", nRet); break; }

if (stDeviceList.nDeviceNum > 0) { for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++) { printf("[device %d]:\n", i); MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i]; if (NULL == pDeviceInfo) { break; } PrintDeviceInfo(pDeviceInfo); } } else { printf("Find No Devices!\n"); break; }

printf("Please Input camera index(0-%d):", stDeviceList.nDeviceNum-1); unsigned int nIndex = 0; scanf_s("%d", &nIndex);

if (nIndex >= stDeviceList.nDeviceNum) { printf("Input error!\n"); break; }

// ch:选择设备并创建句柄 | en:Select device and create handle nRet = MV_CC_CreateHandle(&handle, stDeviceList.pDeviceInfo[nIndex]); if (MV_OK != nRet) { printf("Create Handle fail! nRet [0x%x]\n", nRet); break; }

// ch:打开设备 | en:Open device nRet = MV_CC_OpenDevice(handle); if (MV_OK != nRet) { printf("Open Device fail! nRet [0x%x]\n", nRet); break; }

// ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera) if (stDeviceList.pDeviceInfo[nIndex]->nTLayerType == MV_GIGE_DEVICE) { int nPacketSize = MV_CC_GetOptimalPacketSize(handle); if (nPacketSize > 0) { nRet = MV_CC_SetIntValueEx(handle,"GevSCPSPacketSize",nPacketSize); if(nRet != MV_OK) { printf("Warning: Set Packet Size fail nRet [0x%x]!", nRet); } } else { printf("Warning: Get Packet Size fail nRet [0x%x]!", nPacketSize); } }

nRet = MV_CC_SetEnumValue(handle, "TriggerMode", MV_TRIGGER_MODE_OFF); if (MV_OK != nRet) { printf("Set Trigger Mode fail! nRet [0x%x]\n", nRet); break; }

// ch:开始取流 | en:Start grab image nRet = MV_CC_StartGrabbing(handle); if (MV_OK != nRet) { printf("Start Grabbing fail! nRet [0x%x]\n", nRet); break; }

MV_FRAME_OUT stImageInfo = {0};

nRet = MV_CC_GetImageBuffer(handle, &stImageInfo, 1000); if (nRet == MV_OK) { printf("Get One Frame: Width[%d], Height[%d], nFrameNum[%d]\n", stImageInfo.stFrameInfo.nWidth, stImageInfo.stFrameInfo.nHeight, stImageInfo.stFrameInfo.nFrameNum);

pDstData = (unsigned char *)malloc(sizeof(unsigned char) * (stImageInfo.stFrameInfo.nFrameLen)); if (pDstData == NULL) { printf("malloc pData fail !\n"); break; }

MV_CC_CONTRAST_PARAM stContrastParam = {0};

stContrastParam.nWidth = stImageInfo.stFrameInfo.nWidth; stContrastParam.nHeight = stImageInfo.stFrameInfo.nHeight; stContrastParam.enPixelType = stImageInfo.stFrameInfo.enPixelType; stContrastParam.pSrcBuf = stImageInfo.pBufAddr; stContrastParam.nSrcBufLen = stImageInfo.stFrameInfo.nFrameLen;

stContrastParam.pDstBuf = pDstData; stContrastParam.nDstBufSize = stImageInfo.stFrameInfo.nFrameLen; stContrastParam.nContrastFactor = 1000; nRet = MV_CC_ImageContrast(handle, &stContrastParam); if (MV_OK != nRet) { printf("Adjust image contrast fail! nRet [0x%x]\n", nRet); break; }

FILE* fp = NULL; errno_t err = fopen_s(&fp, "BeforeContrast.raw", "wb"); if (0 != err || NULL == fp) { printf("Open file failed\n"); break; } fwrite(stImageInfo.pBufAddr, 1, stImageInfo.stFrameInfo.nFrameLen, fp); fclose(fp); fp = NULL;

MV_CC_FreeImageBuffer(handle, &stImageInfo);

err = fopen_s(&fp, "AfterContrast.raw", "wb"); if (0 != err || NULL == fp) { printf("Open file failed\n"); break; } fwrite(pDstData, 1, stImageInfo.stFrameInfo.nFrameLen, fp);

fclose(fp); printf("Image contrast succeed\n"); } else { printf("Get Image fail! nRet [0x%x]\n", nRet); }

// ch:停止取流 | en:Stop grab image nRet = MV_CC_StopGrabbing(handle); if (MV_OK != nRet) { printf("Stop Grabbing fail! nRet [0x%x]\n", nRet); break; }

// ch:关闭设备 | en:Close device nRet = MV_CC_CloseDevice(handle); if (MV_OK != nRet) { printf("Close Device fail! nRet [0x%x]\n", nRet); break; }

// ch:销毁句柄 | en:Destroy handle nRet = MV_CC_DestroyHandle(handle); if (MV_OK != nRet) { printf("Destroy Handle fail! nRet [0x%x]\n", nRet); break; } handle = NULL; } while (0);

if (pDstData) { free(pDstData); pDstData = NULL; }

if (handle != NULL) { MV_CC_DestroyHandle(handle); handle = NULL; }

// ch:反初始化SDK | en:Finalize SDK MV_CC_Finalize();

printf("Press a key to exit.\n"); WaitForKeyPress();

return 0; }

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

### ImageSave.cpp

保存相机设备图像

该示例程序演示了如何保存从相机设备拿到的图像，格式包含裸图(.raw)、jpeg、bmp、tiff、png。

#include <stdio.h> #include <Windows.h> #include <process.h> #include <conio.h> #include "MvCameraControl.h"

bool g_bExit = false; void* handle = NULL;

// ch:等待按键输入 | en:Wait for key press void WaitForKeyPress(void) {

while(!_kbhit()) { Sleep(10); } _getch(); }

bool PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo) { if (NULL == pstMVDevInfo) { printf("The Pointer of pstMVDevInfo is NULL!\n"); return false; } if (pstMVDevInfo->nTLayerType == MV_GIGE_DEVICE) { int nIp1 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24); int nIp2 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16); int nIp3 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8); int nIp4 = (pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

// ch:打印当前相机ip和用户自定义名字 | en:print current ip and user defined name printf("CurrentIp: %d.%d.%d.%d\n" , nIp1, nIp2, nIp3, nIp4); printf("UserDefinedName: %s\n\n" , pstMVDevInfo- >SpecialInfo.stGigEInfo.chUserDefinedName); } else if (pstMVDevInfo->nTLayerType == MV_USB_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.chSerialNumber); printf("Device Number: %d\n\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.nDeviceNumber); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_GIGE_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_CAMERALINK_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chSerialNumber);

printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_CXP_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_XOF_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chModelName); } else { printf("Not support.\n"); }

return true; }

// ch:保存图片 | en:Save Image int SaveImage(MV_SAVE_IAMGE_TYPE enSaveImageType,MV_FRAME_OUT* stImageInfo) { char chImageName[256] = {0}; MV_CC_IMAGE stImg; MV_CC_SAVE_IMAGE_PARAM stSaveParams; memset(&stSaveParams, 0, sizeof(MV_CC_SAVE_IMAGE_PARAM)); memset(&stImg, 0, sizeof(MV_CC_SAVE_IMAGE_PARAM));

stImg.enPixelType = stImageInfo->stFrameInfo.enPixelType; stImg.nHeight = stImageInfo->stFrameInfo.nHeight; stImg.nWidth = stImageInfo->stFrameInfo.nWidth; stImg.nImageBufLen = stImageInfo->stFrameInfo.nFrameLenEx; stImg.pImageBuf = stImageInfo->pBufAddr;

stSaveParams.enImageType = enSaveImageType; stSaveParams.iMethodValue = 1; stSaveParams.nQuality = 99;

if (MV_Image_Bmp == stSaveParams.enImageType)

{ sprintf_s(chImageName, 256, "Image_w%d_h%d_fn%03d.bmp", stImageInfo->stFrameInfo.nWidth,stImageInfo->stFrameInfo.nHeight, stImageInfo->stFrameInfo.nFrameNum); } else if (MV_Image_Jpeg == stSaveParams.enImageType) { sprintf_s(chImageName, 256, "Image_w%d_h%d_fn%03d.jpg", stImageInfo->stFrameInfo.nWidth,stImageInfo->stFrameInfo.nHeight, stImageInfo->stFrameInfo.nFrameNum); } else if (MV_Image_Tif == stSaveParams.enImageType) { sprintf_s(chImageName, 256, "Image_w%d_h%d_fn%03d.tif", stImageInfo->stFrameInfo.nWidth,stImageInfo->stFrameInfo.nHeight, stImageInfo->stFrameInfo.nFrameNum); } else if (MV_Image_Png == stSaveParams.enImageType) { sprintf_s(chImageName, 256, "Image_w%d_h%d_fn%03d.png", stImageInfo->stFrameInfo.nWidth,stImageInfo->stFrameInfo.nHeight, stImageInfo->stFrameInfo.nFrameNum); }

int nRet = MV_CC_SaveImageToFileEx2(handle,&stImg, &stSaveParams, (char*)chImageName);

return nRet; }

bool IsHBPixelFormat(MvGvspPixelType ePixelType) { switch (ePixelType) { case PixelType_Gvsp_HB_Mono8: case PixelType_Gvsp_HB_Mono10: case PixelType_Gvsp_HB_Mono10_Packed: case PixelType_Gvsp_HB_Mono12: case PixelType_Gvsp_HB_Mono12_Packed: case PixelType_Gvsp_HB_Mono16: case PixelType_Gvsp_HB_RGB8_Packed: case PixelType_Gvsp_HB_BGR8_Packed: case PixelType_Gvsp_HB_RGBA8_Packed: case PixelType_Gvsp_HB_BGRA8_Packed: case PixelType_Gvsp_HB_RGB16_Packed: case PixelType_Gvsp_HB_BGR16_Packed: case PixelType_Gvsp_HB_RGBA16_Packed: case PixelType_Gvsp_HB_BGRA16_Packed: case PixelType_Gvsp_HB_YUV422_Packed: case PixelType_Gvsp_HB_YUV422_YUYV_Packed: case PixelType_Gvsp_HB_BayerGR8: case PixelType_Gvsp_HB_BayerRG8: case PixelType_Gvsp_HB_BayerGB8: case PixelType_Gvsp_HB_BayerBG8:

case PixelType_Gvsp_HB_BayerRBGG8: case PixelType_Gvsp_HB_BayerGB10: case PixelType_Gvsp_HB_BayerGB10_Packed: case PixelType_Gvsp_HB_BayerBG10: case PixelType_Gvsp_HB_BayerBG10_Packed: case PixelType_Gvsp_HB_BayerRG10: case PixelType_Gvsp_HB_BayerRG10_Packed: case PixelType_Gvsp_HB_BayerGR10: case PixelType_Gvsp_HB_BayerGR10_Packed: case PixelType_Gvsp_HB_BayerGB12: case PixelType_Gvsp_HB_BayerGB12_Packed: case PixelType_Gvsp_HB_BayerBG12: case PixelType_Gvsp_HB_BayerBG12_Packed: case PixelType_Gvsp_HB_BayerRG12: case PixelType_Gvsp_HB_BayerRG12_Packed: case PixelType_Gvsp_HB_BayerGR12: case PixelType_Gvsp_HB_BayerGR12_Packed: return true; default: return false; } }

int main() { int nRet = MV_OK; unsigned char * pDstBuf = NULL; // 用于HB解码

do { // ch:初始化SDK | en:Initialize SDK nRet = MV_CC_Initialize(); if (MV_OK != nRet) { printf("Initialize SDK fail! nRet [0x%x]\n", nRet); break; }

// ch:枚举设备 | en:Enum device MV_CC_DEVICE_INFO_LIST stDeviceList; memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST)); nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE | MV_GENTL_CAMERALINK_DEVICE | MV_GENTL_CXP_DEVICE | MV_GENTL_XOF_DEVICE, &stDeviceList); if (MV_OK != nRet) { printf("Enum Devices fail! nRet [0x%x]\n", nRet); break; }

if (stDeviceList.nDeviceNum > 0) {

for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++) { printf("[device %d]:\n", i); MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i]; if (NULL == pDeviceInfo) { break; } PrintDeviceInfo(pDeviceInfo); } } else { printf("Find No Devices!\n"); break; }

printf("Please Input camera index(0-%d):", stDeviceList.nDeviceNum-1); unsigned int nIndex = 0; scanf_s("%d", &nIndex);

if (nIndex >= stDeviceList.nDeviceNum) { printf("Input error!\n"); break; }

// ch:选择设备并创建句柄 | en:Select device and create handle nRet = MV_CC_CreateHandle(&handle, stDeviceList.pDeviceInfo[nIndex]); if (MV_OK != nRet) { printf("Create Handle fail! nRet [0x%x]\n", nRet); break; }

// ch:打开设备 | en:Open device nRet = MV_CC_OpenDevice(handle); if (MV_OK != nRet) { printf("Open Device fail! nRet [0x%x]\n", nRet); break; }

// ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera) if (stDeviceList.pDeviceInfo[nIndex]->nTLayerType == MV_GIGE_DEVICE) {

int nPacketSize = MV_CC_GetOptimalPacketSize(handle); if (nPacketSize > 0) { nRet = MV_CC_SetIntValue(handle,"GevSCPSPacketSize",nPacketSize); if(nRet != MV_OK) { printf("Warning: Set Packet Size fail nRet [0x%x]!", nRet); } } else { printf("Warning: Get Packet Size fail nRet [0x%x]!", nPacketSize); } }

// ch:获取数据包大小 | en:Get payload size MVCC_INTVALUE stParam; memset(&stParam, 0, sizeof(MVCC_INTVALUE)); nRet = MV_CC_GetIntValue(handle, "PayloadSize", &stParam); if (MV_OK != nRet) { printf("Get PayloadSize fail! nRet [0x%x]\n", nRet); break; } unsigned int nPayloadSize = stParam.nCurValue;

// ch:设置触发模式为off | en:Set trigger mode as off nRet = MV_CC_SetEnumValue(handle, "TriggerMode", 0); if (MV_OK != nRet) { printf("Set Trigger Mode fail! nRet [0x%x]\n", nRet); break; }

// ch:开始取流 | en:Start grab image nRet = MV_CC_StartGrabbing(handle); if (MV_OK != nRet) { printf("Start Grabbing fail! nRet [0x%x]\n", nRet); break; }

MV_FRAME_OUT stImageInfo = {0}; MV_CC_HB_DECODE_PARAM stDecodeParam = {0}; nRet = MV_CC_GetImageBuffer(handle, &stImageInfo, 20000); //线阵 相机需要更大的图像获取间隔 if (nRet == MV_OK) {

printf("Get Image Buffer: Width[%d], Height[%d], FrameNum[%d]\n", stImageInfo.stFrameInfo.nWidth, stImageInfo.stFrameInfo.nHeight, stImageInfo.stFrameInfo.nFrameNum);

printf("Please Input Save Image Type(raw/Jpeg/bmp/tiff/png):"); while (true) { char chSaveImageType[128] ={0}; scanf("%s", chSaveImageType); if (0 == strcmp(chSaveImageType,"raw")) { // 判断是否为HB模式,若是进行解码 bool bflag = IsHBPixelFormat(stImageInfo.stFrameInfo.enPixelType); if(bflag) { stDecodeParam.pSrcBuf = stImageInfo.pBufAddr; stDecodeParam.nSrcLen = stImageInfo.stFrameInfo.nFrameLenEx;

if (pDstBuf == NULL) { pDstBuf = (unsigned char *)malloc(sizeof(unsigned char) * (nPayloadSize*3)); if (NULL == pDstBuf) { printf("malloc pDstData fail !\n"); nRet = MV_E_RESOURCE; break; } } stDecodeParam.pDstBuf = pDstBuf; stDecodeParam.nDstBufSize = nPayloadSize; nRet = MV_CC_HB_Decode(handle, &stDecodeParam); if (nRet != MV_OK) { printf("Decode fail![0x%x]\n", nRet); break; } else { printf("HB Decode success!\n"); }

// 保存裸图 char chRawPath[256] = {0}; sprintf_s(chRawPath, 256, "Image_w%d_h%d_fn%03d.raw", stDecodeParam.nWidth, stDecodeParam.nHeight, stImageInfo.stFrameInfo.nFrameNum); FILE* file = fopen(chRawPath, "wb+"); if (NULL != file)

{ // 写入数据到文件 size_t bytesWritten = fwrite(stDecodeParam.pDstBuf, 1, stDecodeParam.nDstBufLen, file); if (stDecodeParam.nDstBufLen == bytesWritten) { printf("Save image raw success.\n"); } else { printf("Save image raw failed.\n"); }

// 关闭文件 fclose(file); }

} else { // 保存裸图 char chRawPath[256] = {0}; sprintf_s(chRawPath, 256, "Image_w%d_h%d_fn%03d.raw", stImageInfo.stFrameInfo.nWidth, stImageInfo.stFrameInfo.nHeight, stImageInfo.stFrameInfo.nFrameNum); FILE* file = fopen(chRawPath, "wb+"); if (NULL != file) { // 写入数据到文件 size_t bytesWritten = fwrite(stImageInfo.pBufAddr, 1, stImageInfo.stFrameInfo.nFrameLenEx, file);

if (stImageInfo.stFrameInfo.nFrameLen == bytesWritten) { printf("Save image raw success.\n"); } else { printf("Save image raw failed.\n"); } // 关闭文件 fclose(file); } }

break; } else if(0 == strcmp(chSaveImageType,"Jpeg"))

{ nRet = SaveImage(MV_Image_Jpeg,&stImageInfo); if(MV_OK == nRet) { printf("Save Image Jpeg success.\n"); } else { printf("Save image Jpeg failed.\n"); } break; } else if(0 == strcmp(chSaveImageType,"bmp")) { nRet = SaveImage(MV_Image_Bmp,&stImageInfo); if(MV_OK == nRet) { printf("Save Image bmp success.\n"); } else { printf("Save image bmp failed.\n"); } break; } else if(0 == strcmp(chSaveImageType,"tiff")) { nRet = SaveImage(MV_Image_Tif,&stImageInfo); if(MV_OK == nRet) { printf("Save Image tif success.\n"); } else { printf("Save image tif failed.\n"); } break; } else if(0 == strcmp(chSaveImageType,"png")) { nRet = SaveImage(MV_Image_Png,&stImageInfo); if(MV_OK == nRet) { printf("Save Image png success.\n"); } else { printf("Save image png failed.\n"); } break; } else { printf("Input not supoort,Please Input Save Image Type(raw/Jpeg/bmp/tiff/png):"); continue; }

}

if (MV_OK != nRet) { if(NULL != pDstBuf) { free (pDstBuf); pDstBuf = NULL; } }

nRet = MV_CC_FreeImageBuffer(handle, &stImageInfo); if(nRet != MV_OK) { printf("Free Image Buffer fail! nRet [0x%x]\n", nRet); }

} else { printf("Get Image fail! nRet [0x%x]\n", nRet); }

// ch:停止取流 | en:Stop grab image nRet = MV_CC_StopGrabbing(handle); if (MV_OK != nRet) { printf("Stop Grabbing fail! nRet [0x%x]\n", nRet); break; }

// ch:关闭设备 | Close device nRet = MV_CC_CloseDevice(handle); if (MV_OK != nRet) { printf("ClosDevice fail! nRet [0x%x]\n", nRet); break; }

// ch:销毁句柄 | Destroy handle nRet = MV_CC_DestroyHandle(handle); if (MV_OK != nRet) { printf("Destroy Handle fail! nRet [0x%x]\n", nRet); break; } handle = NULL; } while (0);

if (handle != NULL) { MV_CC_DestroyHandle(handle); handle = NULL; }

// ch:反初始化SDK | en:Finalize SDK MV_CC_Finalize();

printf("Press a key to exit.\n"); WaitForKeyPress();

return 0; }

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

### InterfaceAndDeviceDemo.cpp

采集卡下的相机控制

该示例程序说明如何通过普通枚举接口枚举到自研采集卡下的相机，并实现打开相机、开始取流等 操作。

#include <stdio.h> #include <Windows.h> #include <conio.h> #include "MvCameraControl.h"

//ch：打印设备信息 | en: Print the information of devices bool PrintDeviceInfo(MV_CC_DEVICE_INFO_LIST& stDeviceList); //ch：打印采集卡信息 | en: Print the information of frame grabbers bool PrintInterfaceInfo(MV_INTERFACE_INFO_LIST& stInterfaceInfoList); //ch：取流回调函数 | en: Stream grabbing callback function void __stdcall ImageCallBackEx(unsigned char * pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser); // ch:等待按键输入 | en:Wait for key press void WaitForKeyPress(void) { while(!_kbhit()) { Sleep(10); } _getch(); }

int main() { int nRet = MV_OK; // ch:初始化SDK | en:Initialize SDK MV_CC_Initialize();

//ch: 采集卡接口类型 | en: The interface type of frame grabber int nInterfaceType = MV_GIGE_INTERFACE | MV_CAMERALINK_INTERFACE | MV_CXP_INTERFACE | MV_XOF_INTERFACE;

void* hInterface = NULL; //ch: 采集卡句柄 | en: Handle of frame grabber void* hDevHandle = NULL; //ch: 设备句柄 | en: Handle of device

do { // ch:初始化SDK | en:Initialize SDK nRet = MV_CC_Initialize(); if (MV_OK != nRet) { printf("Initialize SDK fail! nRet [0x%x]\n", nRet); break; }

//ch: 枚举采集卡 | en: Enumerate frame grabbers MV_INTERFACE_INFO_LIST stInterfaceInfoList={0}; nRet = MV_CC_EnumInterfaces(nInterfaceType, &stInterfaceInfoList); if (MV_OK != nRet) { printf("Enum interfaces fail! nRet [0x%x]\n", nRet); break; }

if (stInterfaceInfoList.nInterfaceNum > 0) { PrintInterfaceInfo(stInterfaceInfoList); printf("Enum interfaces success!\n\n"); } else { printf("Find no interface!\n"); break; }

printf("Please input interfaces index(0-%d):", stInterfaceInfoList.nInterfaceNum-1); unsigned int nIndex = 0; scanf_s("%d", &nIndex);

if (nIndex >= stInterfaceInfoList.nInterfaceNum) { printf("Input error!\n"); break; }

//ch:创建采集卡句柄 | en: Create the handle of frame grabber nRet = MV_CC_CreateInterface(&hInterface, stInterfaceInfoList.pInterfaceInfos[nIndex]); if (MV_OK == nRet) { printf("Create interface success!\n"); } else { printf("Create interface Handle fail! nRet [0x%x]\n", nRet); break; }

//ch: 打开采集卡 | en: Open the frame grabber nRet = MV_CC_OpenInterface(hInterface, NULL); if (MV_OK == nRet) { printf("Open interface success!\n");

} else { printf("Open interface fail! nRet [0x%x]\n", nRet); break; }

//ch: 枚举采集卡上的相机 | en: Enumerate the devices on the frame grabber MV_CC_DEVICE_INFO_LIST stDeviceList = {0}; nRet = MV_CC_EnumDevicesByInterface(hInterface, &stDeviceList); if (nRet != MV_OK) { printf("MV_CC_EnumDevicesByInterface fail! nRet [0x%x]\n", nRet); break; }

if (stDeviceList.nDeviceNum == 0) { printf("no device! fail!\n"); break; }

PrintDeviceInfo(stDeviceList);

printf("Please input camera index(0-%d):", stDeviceList.nDeviceNum-1); nIndex = 0; scanf_s("%d", &nIndex);

if (nIndex >= stDeviceList.nDeviceNum) { printf("Input error!\n"); break; }

// ch:选择设备并创建句柄 | en:Select device and create handle nRet = MV_CC_CreateHandle(&hDevHandle, stDeviceList.pDeviceInfo[nIndex]); if (MV_OK != nRet) { printf("Create handle fail! nRet [0x%x]\n", nRet); break; }

// ch:打开设备 | en:Open device nRet = MV_CC_OpenDevice(hDevHandle); if (MV_OK != nRet)

{ printf("Open device fail! nRet [0x%x]\n", nRet); break; }

printf("Open device success!\n");

// ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera) if (stDeviceList.pDeviceInfo[nIndex]->nTLayerType == MV_GIGE_DEVICE) { int nPacketSize = MV_CC_GetOptimalPacketSize(hDevHandle); if (nPacketSize > 0) { nRet = MV_CC_SetIntValueEx(hDevHandle,"GevSCPSPacketSize",nPacketSize); if(nRet != MV_OK) { printf("Warning: Set Packet Size fail nRet [0x%x]!", nRet); } } else { printf("Warning: Get Packet Size fail nRet [0x%x]!", nPacketSize); } }

// ch:注册抓图回调 | en:Register image callback nRet = MV_CC_RegisterImageCallBackEx(hDevHandle, ImageCallBackEx, NULL); if (MV_OK != nRet) { printf("Register Image CallBack fail! nRet [0x%x]\n", nRet); break; }

// ch:设置触发模式为off | en:Set trigger mode as off nRet = MV_CC_SetEnumValue(hDevHandle, "TriggerMode", 0); if (MV_OK != nRet) { printf("Set Trigger Mode fail! nRet [0x%x]\n", nRet); break; }

// ch:开始取流 | en:Start grab image nRet = MV_CC_StartGrabbing(hDevHandle); if (MV_OK != nRet) {

printf("Start Grabbing fail! nRet [0x%x]\n", nRet); break; }

printf("Start grabbing success!\n");

printf("Press a key to stop grabbing.\n"); WaitForKeyPress();

//ch：停止取流 | en: Stop grabbing MV_CC_StopGrabbing(hDevHandle); printf("Stop grabbing success!\n");

// ch:关闭设备 | Close device MV_CC_CloseDevice(hDevHandle); printf("Close device success!\n");

// ch:销毁设备句柄 | Destroy handle of device MV_CC_DestroyHandle(hDevHandle); printf("Destroy device handle success!\n");

hDevHandle = NULL;

//ch: 关闭采集卡 | en: Close the frame grabber MV_CC_CloseInterface(hInterface); printf("Close interface success!\n");

// ch:销毁采集卡句柄 | Destroy handle of frame grabber nRet = MV_CC_DestroyInterface(hInterface); printf("Destroy interface success!\n");

hInterface = NULL;

} while (0);

if (hDevHandle != NULL) { MV_CC_CloseDevice(hDevHandle); MV_CC_DestroyHandle(hDevHandle); hDevHandle = NULL; }

if (hInterface != NULL) {

MV_CC_CloseInterface(hInterface); MV_CC_DestroyInterface(hInterface); hInterface = NULL; }

// ch:反初始化SDK | en:Finalize SDK MV_CC_Finalize();

printf("Press a key to exit.\n"); WaitForKeyPress();

}

bool PrintDeviceInfo(MV_CC_DEVICE_INFO_LIST& stDeviceList) { for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++) { printf("[device %d]:\n", i); MV_CC_DEVICE_INFO* pstMVDevInfo = stDeviceList.pDeviceInfo[i];

if (pstMVDevInfo->nTLayerType == MV_GIGE_DEVICE) { int nIp1 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24); int nIp2 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16); int nIp3 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8); int nIp4 = (pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

// ch:打印当前相机ip和用户自定义名字 | en:print current ip and user defined name printf("CurrentIp: %d.%d.%d.%d\n" , nIp1, nIp2, nIp3, nIp4); printf("UserDefinedName: %s\n\n" , pstMVDevInfo- >SpecialInfo.stGigEInfo.chUserDefinedName); } else if (pstMVDevInfo->nTLayerType == MV_USB_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.chSerialNumber); printf("Device Number: %d\n\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.nDeviceNumber); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_GIGE_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chUserDefinedName);

printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chSerialNumber); printf("Model Name %s\n\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_CAMERALINK_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_CXP_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_XOF_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chModelName); } else { printf("Not support.\n"); } }

return true; }

bool PrintInterfaceInfo(MV_INTERFACE_INFO_LIST& stInterfaceInfoList) { for (unsigned int i = 0; i < stInterfaceInfoList.nInterfaceNum; i++) { printf("[Interface %d]:\n", i); MV_INTERFACE_INFO* pstInterfaceInfo = stInterfaceInfoList.pInterfaceInfos[i]; if (NULL == pstInterfaceInfo) { break; } printf("Display name: %s\n",pstInterfaceInfo->chDisplayName); printf("Serial number: %s\n",pstInterfaceInfo->chSerialNumber);

printf("model name: %s\n",pstInterfaceInfo->chModelName); printf("\n"); }

return true; }

void __stdcall ImageCallBackEx(unsigned char * pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser) { if (pFrameInfo) { printf("Get One Frame: Width[%d], Height[%d], nFrameNum[%d]\n", pFrameInfo->nWidth, pFrameInfo->nHeight, pFrameInfo- >nFrameNum); } }

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

### InterfaceDemo.cpp

采集卡的枚举及控制

该示例程序说明如何通过枚举接口枚举自研采集卡，并实现打开采集卡、设置属性等操作。

#include <stdio.h>

#include <Windows.h> #include <process.h> #include <conio.h> #include "MvCameraControl.h"

void* hInterface = NULL;

// ch:等待按键输入 | en:Wait for key press void WaitForKeyPress(void) { while(!_kbhit()) { Sleep(10); } _getch(); }

bool PrintInterfaceInfo(MV_INTERFACE_INFO* pstInterfaceInfo) { if (NULL == pstInterfaceInfo) { printf("The Pointer of pstInterfaceInfo is NULL!\n"); return false; } printf("Display name: %s\n",pstInterfaceInfo->chDisplayName); printf("Serial number: %s\n",pstInterfaceInfo->chSerialNumber); printf("model name: %s\n",pstInterfaceInfo->chModelName); printf("\n");

return true; }

void Set_Get_Enum(const char* str) { MVCC_ENUMVALUE stEnumValue = {0}; MVCC_ENUMENTRY stEnumentryInfo = {0};

int nRet = MV_CC_GetEnumValue(hInterface,str, &stEnumValue); if (MV_OK != nRet) { printf("Get %s Fail! nRet [0x%x]\n", str, nRet); return; }

stEnumentryInfo.nValue = stEnumValue.nCurValue; nRet = MV_CC_GetEnumEntrySymbolic(hInterface,str, &stEnumentryInfo); if (MV_OK != nRet) { printf("Get %s Fail! nRet [0x%x]\n", str,nRet); return;

} else { printf("Get %s = [%s] Success!\n",str,stEnumentryInfo.chSymbolic); }

MV_XML_AccessMode enAccessMode = AM_NI; nRet = MV_XML_GetNodeAccessMode(hInterface, str, &enAccessMode); if(MV_OK == nRet && AM_RW == enAccessMode) { nRet = MV_CC_SetEnumValue(hInterface,str,stEnumValue.nCurValue); if (MV_OK != nRet) { printf("Set %s Fail! nRet [0x%x]\n", str,nRet); return; } else { printf("Set %s = [%s] Success!\n",str,stEnumentryInfo.chSymbolic); } } }

void Set_Get_Bool(const char* str) { bool bValue = false; int nRet = MV_CC_GetBoolValue(hInterface,str, &bValue); if (MV_OK != nRet) { printf("Get %s Fail! nRet [0x%x]\n", str,nRet); return; } else { printf("Get %s = [%d] Success!\n",str,bValue); }

MV_XML_AccessMode enAccessMode = AM_NI; nRet = MV_XML_GetNodeAccessMode(hInterface, str, &enAccessMode); if(MV_OK == nRet && AM_RW == enAccessMode) { nRet = MV_CC_SetBoolValue(hInterface,str, bValue); if (MV_OK != nRet) { printf("Set %s Fail! nRet [0x%x]\n", str,nRet); return; } else { printf("Set %s = [%d] Success!\n",str,bValue); } }

}

void Set_Get_Int(const char* str) { MVCC_INTVALUE_EX stIntValue; int nRet = MV_CC_GetIntValueEx(hInterface,str,&stIntValue); if (MV_OK != nRet) { printf("Get %s Fail! nRet [0x%x]\n", str,nRet); return; } else { printf("Get %s = [%d] Success!\n",str,stIntValue.nCurValue); }

MV_XML_AccessMode enAccessMode = AM_NI; nRet = MV_XML_GetNodeAccessMode(hInterface, str, &enAccessMode); if(MV_OK == nRet && AM_RW == enAccessMode) { nRet = MV_CC_SetIntValueEx(hInterface,str,stIntValue.nCurValue); if (MV_OK != nRet) { printf("Set %s Fail! nRet [0x%x]\n", str,nRet); return; } else { printf("Set %s = [%d] Success!\n",str,stIntValue.nCurValue); } } }

void Set_Get_String(const char* str) { MVCC_STRINGVALUE StringValue; int nRet = MV_CC_GetStringValue(hInterface,str, &StringValue); if (MV_OK != nRet) { printf("Get %s Fail! nRet [0x%x]\n", str,nRet); return; } else { printf("Get %s = [%s] Success!\n",str,StringValue.chCurValue); }

MV_XML_AccessMode enAccessMode = AM_NI; nRet = MV_XML_GetNodeAccessMode(hInterface, str, &enAccessMode); if(MV_OK == nRet && AM_RW == enAccessMode) { nRet = MV_CC_SetStringValue(hInterface,str, StringValue.chCurValue);

if (MV_OK != nRet) { printf("Set %s Fail! nRet [0x%x]\n", str,nRet); return; } else { printf("Set %s = [%s] Success!\n",str,StringValue.chCurValue); } } }

void Set_Get_Float(const char* str) { MVCC_FLOATVALUE FloatValue; int nRet = MV_CC_GetFloatValue(hInterface,str, &FloatValue); if (MV_OK != nRet) { printf("Get %s Fail! nRet [0x%x]\n", str,nRet); return; } else { printf("Get %s = [%f] Success!\n",str,FloatValue.fCurValue); }

MV_XML_AccessMode enAccessMode = AM_NI; nRet = MV_XML_GetNodeAccessMode(hInterface, str, &enAccessMode); if(MV_OK == nRet && AM_RW == enAccessMode) { nRet = MV_CC_SetFloatValue(hInterface,str, FloatValue.fCurValue); if (MV_OK != nRet) { printf("Set %s Fail! nRet [0x%x]\n", str,nRet); return; } else { printf("Set %s = [%f] Success!\n",str,FloatValue.fCurValue); } } }

int main() { int nRet = MV_OK;

printf("[0]: GIGE Interface\n"); printf("[1]: CAMERALINK Interface\n"); printf("[2]: CXP Interface\n"); printf("[3]: XoF Interface\n\n");

do { // ch:初始化SDK | en:Initialize SDK nRet = MV_CC_Initialize(); if (MV_OK != nRet) { printf("Initialize SDK fail! nRet [0x%x]\n", nRet); break; }

MV_INTERFACE_INFO_LIST stInterfaceInfoList={0};

unsigned int nType = 0; printf("Please Input Enum Interfaces Type(0-%d):", 3); scanf_s("%d", &nType);

switch(nType) { case 0: { nRet = MV_CC_EnumInterfaces(MV_GIGE_INTERFACE, &stInterfaceInfoList); break; } case 1: { nRet = MV_CC_EnumInterfaces(MV_CAMERALINK_INTERFACE, &stInterfaceInfoList); break; } case 2: { nRet = MV_CC_EnumInterfaces(MV_CXP_INTERFACE, &stInterfaceInfoList); break; } case 3: { nRet = MV_CC_EnumInterfaces(MV_XOF_INTERFACE, &stInterfaceInfoList); break; } default: { printf("Input error!\n"); break; } }

//枚举采集卡 if (MV_OK != nRet)

{ printf("Enum Interfaces fail! nRet [0x%x]\n", nRet); break; }

if (stInterfaceInfoList.nInterfaceNum > 0) { for (unsigned int i = 0; i < stInterfaceInfoList.nInterfaceNum; i++) { printf("[Interface %d]:\n", i); MV_INTERFACE_INFO* pstInterfaceInfo = stInterfaceInfoList.pInterfaceInfos[i]; if (NULL == pstInterfaceInfo) { break; } PrintInterfaceInfo(pstInterfaceInfo); } printf("Enum Interfaces success!\n\n"); } else { printf("Find No Interface!\n"); break; }

printf("Please Input Interfaces index(0-%d):", stInterfaceInfoList.nInterfaceNum-1); unsigned int nIndex = 0; scanf_s("%d", &nIndex);

if (nIndex >= stInterfaceInfoList.nInterfaceNum) { printf("Input error!\n"); break; }

//创建采集卡句柄 nRet = MV_CC_CreateInterface(&hInterface, stInterfaceInfoList.pInterfaceInfos[nIndex]); if (MV_OK == nRet) { printf("Create Interface success!\n"); } else { printf("Create Interface Handle fail! nRet [0x%x]\n", nRet); break; }

//打开采集卡

nRet = MV_CC_OpenInterface(hInterface, NULL); if (MV_OK == nRet) { printf("Open Interface success!\n"); } else { printf("Open Interface fail! nRet [0x%x]\n", nRet); break; }

//采集卡属性操作,不同类型卡对各自常用属性进行获取和设置 switch(nType) { case 0: { //MV_GIGE_INTERFACE卡属性获取和设置操作 Set_Get_Enum("StreamSelector"); Set_Get_Enum("TimerSelector"); Set_Get_Enum("TimerTriggerSource"); Set_Get_Enum("TimerTriggerActivation"); Set_Get_Bool("HBDecompression"); Set_Get_Int("TimerDuration"); Set_Get_Int("TimerDelay"); Set_Get_Int("TimerFrequency"); break; } case 1: { //MV_CAMERALINK_INTERFACE卡属性操作 Set_Get_Enum("StreamSelector"); Set_Get_Enum("CameraType"); Set_Get_Enum("StreamPartialImageControl"); Set_Get_Int("ImageHeight"); Set_Get_Int("FrameTimeoutTime"); break; } case 2: { //MV_CXP_INTERFACE卡属性操作 Set_Get_Enum("StreamSelector"); Set_Get_String("CurrentStreamDevice"); Set_Get_Int("StreamEnableStatus"); Set_Get_Bool("BayerCFAEnable"); Set_Get_Bool("IspGammaEnable"); Set_Get_Float("IspGamma"); break; } case 3: { //MV_XOF_INTERFACE卡属性操作 Set_Get_Enum("StreamSelector"); Set_Get_String("CurrentStreamDevice"); Set_Get_Int("StreamEnableStatus"); Set_Get_Bool("BayerCFAEnable");

Set_Get_Bool("IspGammaEnable"); Set_Get_Float("IspGamma"); break; } default: { printf("Input error!\n"); break; } }

//关闭采集卡 nRet = MV_CC_CloseInterface(hInterface); if (MV_OK == nRet) { printf("Close Interface success!\n"); } else { printf("Close Interface Handle fail! nRet [0x%x]\n", nRet); break; }

//销毁采集卡句柄 nRet = MV_CC_DestroyInterface(hInterface); if (MV_OK == nRet) { printf("Destroy Interface success!\n"); } else { printf("Destroy Interface Handle fail! nRet [0x%x]\n", nRet); break; } hInterface = NULL; } while (0);

if (hInterface != NULL) { MV_CC_CloseInterface(hInterface); MV_CC_DestroyInterface(hInterface); hInterface = NULL; }

// ch:反初始化SDK | en:Finalize SDK MV_CC_Finalize();

printf("Press a key to exit.\n"); WaitForKeyPress(); }

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

### LensShadingCorrection.cpp

LSC

该示例程序说明如何进行LSC标定和校正。需要先通过LSC标定功能获取到标定表，然后才能对相 机图像进行LSC校正处理。

#include <stdio.h> #include <Windows.h> #include <conio.h> #include <io.h> #include "MvCameraControl.h"

// ch:等待按键输入 | en:Wait for key press void WaitForKeyPress(void) { while(!_kbhit()) { Sleep(10); } _getch(); }

bool PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo) { if (NULL == pstMVDevInfo) { printf("The Pointer of pstMVDevInfo is NULL!\n"); return false; } if (pstMVDevInfo->nTLayerType == MV_GIGE_DEVICE) { int nIp1 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24); int nIp2 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16); int nIp3 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8); int nIp4 = (pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

// ch:打印当前相机ip和用户自定义名字 | en:print current ip and user defined name printf("CurrentIp: %d.%d.%d.%d\n" , nIp1, nIp2, nIp3, nIp4); printf("UserDefinedName: %s\n\n" , pstMVDevInfo- >SpecialInfo.stGigEInfo.chUserDefinedName); } else if (pstMVDevInfo->nTLayerType == MV_USB_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.chSerialNumber); printf("Device Number: %d\n\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.nDeviceNumber); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_GIGE_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_CAMERALINK_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_CXP_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chUserDefinedName);

printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_XOF_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chModelName); } else { printf("Not support.\n"); }

return true; }

unsigned char * g_pDstData = NULL; unsigned int g_nDstDataSize = 0;

unsigned char * g_pCalibBuf = NULL; unsigned int g_nCalibBufSize = 0;

bool g_IsNeedCalib = true;

void __stdcall ImageCallBackEx(unsigned char * pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser) { printf("Get One Frame: Width[%d], Height[%d], nFrameNum[%d]\n", pFrameInfo->nWidth, pFrameInfo->nHeight, pFrameInfo->nFrameNum);

int nRet = MV_OK; //判断是否需要标定 if (true == g_IsNeedCalib) { // LSC标定 MV_CC_LSC_CALIB_PARAM stLSCCalib = {0}; stLSCCalib.nWidth = pFrameInfo->nWidth; stLSCCalib.nHeight = pFrameInfo->nHeight; stLSCCalib.enPixelType = pFrameInfo->enPixelType; stLSCCalib.pSrcBuf = pData; stLSCCalib.nSrcBufLen = pFrameInfo->nFrameLen;

if (g_pCalibBuf == NULL || g_nCalibBufSize < (pFrameInfo- >nWidth*pFrameInfo->nHeight*sizeof(unsigned short)))

{ if (g_pCalibBuf) { free(g_pCalibBuf); g_pCalibBuf = NULL; g_nCalibBufSize = 0; }

g_pCalibBuf = (unsigned char *)malloc(pFrameInfo- >nWidth*pFrameInfo->nHeight*sizeof(unsigned short)); if (g_pCalibBuf == NULL) { printf("malloc pCalibBuf fail !\n"); return; } g_nCalibBufSize = pFrameInfo->nWidth*pFrameInfo- >nHeight*sizeof(unsigned short); }

stLSCCalib.pCalibBuf = g_pCalibBuf; stLSCCalib.nCalibBufSize = g_nCalibBufSize;

stLSCCalib.nSecNumW = 689; stLSCCalib.nSecNumH = 249; stLSCCalib.nPadCoef = 2; stLSCCalib.nCalibMethod = 2; stLSCCalib.nTargetGray = 100;

//同一个相机，在场景、算法参数和图像参数都不变情况下，理论上只需要进行一次标 定，可将标定表保存下来。 //不同相机图片标定出来的标定表不可复用，当场景改变或算法参数改变或图像参数改 变时，需要重新标定。 nRet = MV_CC_LSCCalib(pUser, &stLSCCalib); if (MV_OK != nRet) { printf("LSC Calib fail! nRet [0x%x]\n", nRet); return; }

//保存标定表到本地 FILE* fp_out = fopen("./LSCCalib.bin", "wb"); if (NULL == fp_out) { printf("open LSCCalib.bin fail! \n"); return ; } fwrite(stLSCCalib.pCalibBuf, 1, stLSCCalib.nCalibBufLen, fp_out); fclose(fp_out); fp_out = NULL;

g_IsNeedCalib = false; }

// LSC校正 if (g_pDstData == NULL || g_nDstDataSize < pFrameInfo->nFrameLen) { if (g_pDstData) { free(g_pDstData); g_pDstData = NULL; g_nDstDataSize = 0; }

g_pDstData = (unsigned char *)malloc(pFrameInfo->nFrameLen); if (g_pDstData == NULL) { printf("malloc pDstData fail !\n"); return; } g_nDstDataSize = pFrameInfo->nFrameLen; }

MV_CC_LSC_CORRECT_PARAM stLSCCorrectParam = {0};

stLSCCorrectParam.nWidth = pFrameInfo->nWidth; stLSCCorrectParam.nHeight = pFrameInfo->nHeight; stLSCCorrectParam.enPixelType = pFrameInfo->enPixelType; stLSCCorrectParam.pSrcBuf = pData; stLSCCorrectParam.nSrcBufLen = pFrameInfo->nFrameLen;

stLSCCorrectParam.pDstBuf = g_pDstData; stLSCCorrectParam.nDstBufSize = g_nDstDataSize;

stLSCCorrectParam.pCalibBuf = g_pCalibBuf; stLSCCorrectParam.nCalibBufLen = g_nCalibBufSize; nRet = MV_CC_LSCCorrect(pUser, &stLSCCorrectParam); if (MV_OK != nRet) { printf("LSC Correct fail! nRet [0x%x]\n", nRet); return; }

if (pFrameInfo->nFrameNum < 10) { //保存图像到文件 MV_SAVE_IMG_TO_FILE_PARAM stSaveFileParam; memset(&stSaveFileParam, 0, sizeof(MV_SAVE_IMG_TO_FILE_PARAM));

stSaveFileParam.enImageType = MV_Image_Bmp; stSaveFileParam.enPixelType = pFrameInfo->enPixelType; stSaveFileParam.nWidth = pFrameInfo->nWidth; stSaveFileParam.nHeight = pFrameInfo->nHeight; stSaveFileParam.nDataLen = pFrameInfo->nFrameLen; stSaveFileParam.pData = pData; sprintf_s(stSaveFileParam.pImagePath, 256, "BeforeImage_w%d_h%d_fn%03d.bmp", stSaveFileParam.nWidth, stSaveFileParam.nHeight, pFrameInfo->nFrameNum); nRet = MV_CC_SaveImageToFile(pUser, &stSaveFileParam); if (MV_OK != nRet) { printf("SaveImageToFile failed[%x]!\n", nRet); return; }

stSaveFileParam.pData = g_pDstData; sprintf_s(stSaveFileParam.pImagePath, 256, "AfterImage_w%d_h%d_fn%03d.bmp", stSaveFileParam.nWidth, stSaveFileParam.nHeight, pFrameInfo->nFrameNum); nRet = MV_CC_SaveImageToFile(pUser, &stSaveFileParam); if (MV_OK != nRet) { printf("SaveImageToFile failed[%x]!\n", nRet); return; } } }

int main() { int nRet = MV_OK; void* handle = NULL;

do { // ch:初始化SDK | en:Initialize SDK nRet = MV_CC_Initialize(); if (MV_OK != nRet) { printf("Initialize SDK fail! nRet [0x%x]\n", nRet); break; }

// ch:枚举设备 | en:Enum device MV_CC_DEVICE_INFO_LIST stDeviceList; memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST)); nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE | MV_GENTL_CAMERALINK_DEVICE | MV_GENTL_CXP_DEVICE | MV_GENTL_XOF_DEVICE, &stDeviceList); if (MV_OK != nRet) { printf("Enum Devices fail! nRet [0x%x]\n", nRet);

break; }

if (stDeviceList.nDeviceNum > 0) { for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++) { printf("[device %d]:\n", i); MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i]; if (NULL == pDeviceInfo) { break; } PrintDeviceInfo(pDeviceInfo); } } else { printf("Find No Devices!\n"); break; }

printf("Please Input camera index(0-%d):", stDeviceList.nDeviceNum-1); unsigned int nIndex = 0; scanf_s("%d", &nIndex);

if (nIndex >= stDeviceList.nDeviceNum) { printf("Input error!\n"); break; }

// ch:选择设备并创建句柄 | en:Select device and create handle nRet = MV_CC_CreateHandle(&handle, stDeviceList.pDeviceInfo[nIndex]); if (MV_OK != nRet) { printf("Create Handle fail! nRet [0x%x]\n", nRet); break; }

// ch:打开设备 | en:Open device nRet = MV_CC_OpenDevice(handle); if (MV_OK != nRet) { printf("Open Device fail! nRet [0x%x]\n", nRet); break; }

// ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera) if (stDeviceList.pDeviceInfo[nIndex]->nTLayerType == MV_GIGE_DEVICE) { int nPacketSize = MV_CC_GetOptimalPacketSize(handle); if (nPacketSize > 0) { nRet = MV_CC_SetIntValueEx(handle,"GevSCPSPacketSize",nPacketSize); if(nRet != MV_OK) { printf("Warning: Set Packet Size fail nRet [0x%x]!", nRet); } } else { printf("Warning: Get Packet Size fail nRet [0x%x]!", nPacketSize); } }

// ch:设置触发模式为off | eb:Set trigger mode as off nRet = MV_CC_SetEnumValue(handle, "TriggerMode", MV_TRIGGER_MODE_OFF); if (MV_OK != nRet) { printf("Set Trigger Mode fail! nRet [0x%x]\n", nRet); break; }

//判断是否可以本地导入 FILE* fp = fopen("./LSCCalib.bin", "rb"); if (fp) { int nFileLen = filelength(fileno(fp)); if (g_pCalibBuf == NULL || g_nCalibBufSize < nFileLen) { if (g_pCalibBuf) { free(g_pCalibBuf); g_pCalibBuf = NULL; g_nCalibBufSize = 0; }

g_pCalibBuf = (unsigned char *)malloc(nFileLen); if (g_pCalibBuf == NULL) { printf("malloc pCalibBuf fail !\n"); break; } g_nCalibBufSize = nFileLen;

} fread(g_pCalibBuf, 1, g_nCalibBufSize, fp); fclose(fp);

g_IsNeedCalib = false; }

// ch:注册抓图回调 | en:Register image callback nRet = MV_CC_RegisterImageCallBackEx(handle, ImageCallBackEx, handle); if (MV_OK != nRet) { printf("Register Image CallBack fail! nRet [0x%x]\n", nRet); break; }

// ch:开始取流 | en:Start grab image nRet = MV_CC_StartGrabbing(handle); if (MV_OK != nRet) { printf("Start Grabbing fail! nRet [0x%x]\n", nRet); break; }

printf("Press a key to stop grabbing.\n"); WaitForKeyPress();

Sleep(1000);

// ch:停止取流 | en:Stop grab image nRet = MV_CC_StopGrabbing(handle); if (MV_OK != nRet) { printf("Stop Grabbing fail! nRet [0x%x]\n", nRet); break; }

// ch:关闭设备 | en:Close device nRet = MV_CC_CloseDevice(handle); if (MV_OK != nRet) { printf("Close Device fail! nRet [0x%x]\n", nRet); break; }

// ch:销毁句柄 | en:Destroy handle nRet = MV_CC_DestroyHandle(handle); if (MV_OK != nRet)

{ printf("Destroy Handle fail! nRet [0x%x]\n", nRet); break; } handle = NULL; } while (0);

if (g_pCalibBuf) { free(g_pCalibBuf); g_pCalibBuf = NULL; g_nCalibBufSize = 0; }

if (g_pDstData) { free(g_pDstData); g_pDstData = NULL; g_nDstDataSize = 0; }

if (handle != NULL) { MV_CC_DestroyHandle(handle); handle = NULL; }

// ch:反初始化SDK | en:Finalize SDK MV_CC_Finalize();

printf("Press a key to exit.\n"); WaitForKeyPress();

return 0; }

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

### MultiCast.cpp

设置组播模式

该示例程序说明如何设置传输模式为组播模式。通过询问用户启动多播控制应用程序或者多播监控 应用程序，然后根据用户输入，以对应的访问权限打开相机，最后通过 MV_GIGE_SetTransmissionType() 设置组播模式。

#include <stdio.h> #include <Windows.h> #include <process.h> #include <conio.h> #include "MvCameraControl.h"

bool g_bExit = false;

// ch:等待按键输入 | en:Wait for key press void WaitForKeyPress(void) { while(!_kbhit()) { Sleep(10); } _getch(); }

bool PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo) { if (NULL == pstMVDevInfo) { printf("The Pointer of pstMVDevInfo is NULL!\n"); return false; } if (pstMVDevInfo->nTLayerType == MV_GIGE_DEVICE) {

int nIp1 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24); int nIp2 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16); int nIp3 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8); int nIp4 = (pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

// ch:打印当前相机ip和用户自定义名字 | en:print current ip and user defined name printf("CurrentIp: %d.%d.%d.%d\n" , nIp1, nIp2, nIp3, nIp4); printf("UserDefinedName: %s\n\n" , pstMVDevInfo- >SpecialInfo.stGigEInfo.chUserDefinedName); } else { printf("Not support.\n"); }

return true; }

static unsigned int __stdcall WorkThread(void* pUser) { int nRet = MV_OK; MV_FRAME_OUT stImageInfo = {0};

while(1) { nRet = MV_CC_GetImageBuffer(pUser, &stImageInfo, 1000); if (nRet == MV_OK) { printf("Get Image Buffer: Width[%d], Height[%d], FrameNum[%d]\n", stImageInfo.stFrameInfo.nWidth, stImageInfo.stFrameInfo.nHeight, stImageInfo.stFrameInfo.nFrameNum);

nRet = MV_CC_FreeImageBuffer(pUser, &stImageInfo); if(nRet != MV_OK) { printf("Free Image Buffer fail! nRet [0x%x]\n", nRet); } } else { printf("Get Image fail! nRet [0x%x]\n", nRet); } if(g_bExit) { break;

} }

return 0; }

int main() { int nRet = MV_OK; void* handle = NULL;

do { // ch:初始化SDK | en:Initialize SDK nRet = MV_CC_Initialize(); if (MV_OK != nRet) { printf("Initialize SDK fail! nRet [0x%x]\n", nRet); break; }

// ch:枚举设备 |en:Enum device MV_CC_DEVICE_INFO_LIST stDeviceList; memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST)); nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE, &stDeviceList); if (MV_OK != nRet) { printf("Enum Devices fail! nRet [0x%x]\n", nRet); break; }

if (stDeviceList.nDeviceNum > 0) { for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++) { printf("[device %d]:\n", i); MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i]; if (NULL == pDeviceInfo) { break; } PrintDeviceInfo(pDeviceInfo); } } else { printf("Find No Devices!\n"); break; }

printf("Please Input camera index(0-%d):", stDeviceList.nDeviceNum-1); unsigned int nIndex = 0; scanf("%d", &nIndex);

if (nIndex >= stDeviceList.nDeviceNum) { printf("Input error!\n"); break; }

// ch:选择设备并创建句柄 | en:select device and create handle nRet = MV_CC_CreateHandle(&handle, stDeviceList.pDeviceInfo[nIndex]); if (MV_OK != nRet) { printf("Create Handle fail! nRet [0x%x]\n", nRet); break; }

// ch:询问用户启动多播控制应用程序或多播监控应用程序 // en:Ask the user to launch: the multicast controlling application or the multicast monitoring application. printf("Start multicast sample in (c)ontrol or in (m)onitor mode? (c/m)\n");

char key;

do { scanf("%c", &key); } while ( (key != 'c') && (key != 'm') && (key != 'C') && (key != 'M'));

// ch:查询用户使用的模式 | en:Query the user for the mode to use. bool monitorMode = (key == 'm') || (key == 'M');

// ch:打开设备 | Open device if (monitorMode) { nRet = MV_CC_OpenDevice(handle, MV_ACCESS_Monitor); } else { nRet = MV_CC_OpenDevice(handle, MV_ACCESS_Control); }

if (MV_OK != nRet) { printf("Open Device fail! nRet [0x%x]\n", nRet); break; }

// ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera) if (stDeviceList.pDeviceInfo[nIndex]->nTLayerType == MV_GIGE_DEVICE && false == monitorMode) { int nPacketSize = MV_CC_GetOptimalPacketSize(handle); if (nPacketSize > 0) { nRet = MV_CC_SetIntValueEx(handle,"GevSCPSPacketSize",nPacketSize); if(nRet != MV_OK) { printf("Warning: Set Packet Size fail nRet [0x%x]!\n", nRet); } } else { printf("Warning: Get Packet Size fail nRet [0x%x]!\n", nPacketSize); } }

// ch:指定组播ip | en:multicast IP char strIp[] = "239.192.1.23"; unsigned int nIp1, nIp2, nIp3, nIp4, nIp; sscanf_s(strIp, "%d.%d.%d.%d", &nIp1, &nIp2, &nIp3, &nIp4); nIp = (nIp1 << 24) | (nIp2 << 16) | (nIp3 << 8) | nIp4;

// ch:可指定端口号作为组播组端口 | en:multicast port MV_TRANSMISSION_TYPE stTransmissionType; memset(&stTransmissionType, 0, sizeof(MV_TRANSMISSION_TYPE));

stTransmissionType.enTransmissionType = MV_GIGE_TRANSTYPE_MULTICAST; stTransmissionType.nDestIp = nIp; stTransmissionType.nDestPort = 1042; nRet = MV_GIGE_SetTransmissionType(handle, &stTransmissionType); if (MV_OK != nRet) { printf("Set Transmission Type fail! nRet [0x%x]\n", nRet); break; }

// ch:开始取流 | en:Start grab image

nRet = MV_CC_StartGrabbing(handle); if (MV_OK != nRet) { printf("Start Grabbing fail! nRet [0x%x]\n", nRet); break; }

unsigned int nThreadID = 0; void* hThreadHandle = (void*) _beginthreadex( NULL , 0 , WorkThread , handle, 0 , &nThreadID ); if (NULL == hThreadHandle) { break; }

printf("Press a key to stop grabbing.\n"); WaitForKeyPress();

g_bExit = true; Sleep(1000);

// ch:停止取流 | en:Stop grab image nRet = MV_CC_StopGrabbing(handle); if (MV_OK != nRet) { printf("Stop Grabbing fail! nRet [0x%x]\n", nRet); break; }

// ch:关闭设备 | en:Close device nRet = MV_CC_CloseDevice(handle); if (MV_OK != nRet) { printf("ClosDevice fail! nRet [0x%x]\n", nRet); break; }

// ch:销毁句柄 | en:Destroy handle nRet = MV_CC_DestroyHandle(handle); if (MV_OK != nRet) { printf("Destroy Handle fail! nRet [0x%x]\n", nRet); break; } handle = NULL; } while (0);

if (handle != NULL) {

MV_CC_DestroyHandle(handle); handle = NULL; }

// ch:反初始化SDK | en:Finalize SDK MV_CC_Finalize();

printf("Press a key to exit.\n"); WaitForKeyPress();

return 0; }

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

### MultiLightCtrl_ImageStitching.cpp

线阵相机分时曝光并拆图拼图

该示例程序演示如何设置线阵相机采用2组分时曝光参数，先拆图再自上而下拼图，最后将拼好的 raw图以bmp格式保存至本地。

#include <stdio.h>

#include <Windows.h> #include <process.h> #include <conio.h> #include "MvCameraControl.h"

bool g_bExit = false;

#define IMAGE_NAME_LEN 256 // ch:等待按键输入 | en:Wait for key press void WaitForKeyPress(void) { while(!_kbhit()) { Sleep(10); } _getch(); }

bool IsHBPixelFormat(MvGvspPixelType ePixelType) { switch (ePixelType) { case PixelType_Gvsp_HB_Mono8: case PixelType_Gvsp_HB_Mono10: case PixelType_Gvsp_HB_Mono10_Packed: case PixelType_Gvsp_HB_Mono12: case PixelType_Gvsp_HB_Mono12_Packed: case PixelType_Gvsp_HB_Mono16: case PixelType_Gvsp_HB_RGB8_Packed: case PixelType_Gvsp_HB_BGR8_Packed: case PixelType_Gvsp_HB_RGBA8_Packed: case PixelType_Gvsp_HB_BGRA8_Packed: case PixelType_Gvsp_HB_RGB16_Packed: case PixelType_Gvsp_HB_BGR16_Packed: case PixelType_Gvsp_HB_RGBA16_Packed: case PixelType_Gvsp_HB_BGRA16_Packed: case PixelType_Gvsp_HB_YUV422_Packed: case PixelType_Gvsp_HB_YUV422_YUYV_Packed: case PixelType_Gvsp_HB_BayerGR8: case PixelType_Gvsp_HB_BayerRG8: case PixelType_Gvsp_HB_BayerGB8: case PixelType_Gvsp_HB_BayerBG8: case PixelType_Gvsp_HB_BayerRBGG8: case PixelType_Gvsp_HB_BayerGB10: case PixelType_Gvsp_HB_BayerGB10_Packed: case PixelType_Gvsp_HB_BayerBG10: case PixelType_Gvsp_HB_BayerBG10_Packed: case PixelType_Gvsp_HB_BayerRG10: case PixelType_Gvsp_HB_BayerRG10_Packed: case PixelType_Gvsp_HB_BayerGR10: case PixelType_Gvsp_HB_BayerGR10_Packed: case PixelType_Gvsp_HB_BayerGB12: case PixelType_Gvsp_HB_BayerGB12_Packed: case PixelType_Gvsp_HB_BayerBG12:

case PixelType_Gvsp_HB_BayerBG12_Packed: case PixelType_Gvsp_HB_BayerRG12: case PixelType_Gvsp_HB_BayerRG12_Packed: case PixelType_Gvsp_HB_BayerGR12: case PixelType_Gvsp_HB_BayerGR12_Packed: return true; default: return false; } }

bool PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo) { if (NULL == pstMVDevInfo) { printf("The Pointer of pstMVDevInfo is NULL!\n"); return false; } if (pstMVDevInfo->nTLayerType == MV_GIGE_DEVICE) { int nIp1 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24); int nIp2 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16); int nIp3 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8); int nIp4 = (pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

// ch:打印当前相机ip和用户自定义名字 | en:print current ip and user defined name printf("CurrentIp: %d.%d.%d.%d\n" , nIp1, nIp2, nIp3, nIp4); printf("UserDefinedName: %s\n\n" , pstMVDevInfo- >SpecialInfo.stGigEInfo.chUserDefinedName); } else if (pstMVDevInfo->nTLayerType == MV_USB_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.chSerialNumber); printf("Device Number: %d\n\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.nDeviceNumber); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_GIGE_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_CAMERALINK_DEVICE) {

printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_CXP_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_XOF_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chModelName); } else { printf("Not support.\n"); }

return true; }

int main() { int nRet = MV_OK; void* handle = NULL; unsigned char* pReconstructBuffer = NULL; unsigned char * pDstBuf = NULL;

unsigned int nExposureNum = 2; // 分时频闪的灯数, 默认曝光个数为2

do { // ch:初始化SDK | en:Initialize SDK nRet = MV_CC_Initialize(); if (MV_OK != nRet) { printf("Initialize SDK fail! nRet [0x%x]\n", nRet); break; }

// ch:枚举设备 | en:Enum device MV_CC_DEVICE_INFO_LIST stDeviceList; memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST)); nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE | MV_GENTL_CAMERALINK_DEVICE | MV_GENTL_CXP_DEVICE | MV_GENTL_XOF_DEVICE, &stDeviceList); if (MV_OK != nRet) { printf("Enum Devices fail! nRet [0x%x]\n", nRet); break; }

if (stDeviceList.nDeviceNum > 0) { for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++) { printf("[device %d]:\n", i); MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i]; if (NULL == pDeviceInfo) { break; } PrintDeviceInfo(pDeviceInfo); } } else { printf("Find No Devices!\n"); break; }

printf("Please Input camera index(0-%d):", stDeviceList.nDeviceNum-1); unsigned int nIndex = 0; scanf_s("%d", &nIndex);

if (nIndex >= stDeviceList.nDeviceNum) { printf("Input error!\n"); break; }

// ch:选择设备并创建句柄 | en:Select device and create handle nRet = MV_CC_CreateHandle(&handle, stDeviceList.pDeviceInfo[nIndex]); if (MV_OK != nRet) { printf("Create Handle fail! nRet [0x%x]\n", nRet); break; }

// ch:打开设备 | en:Open device nRet = MV_CC_OpenDevice(handle); if (MV_OK != nRet) { printf("Open Device fail! nRet [0x%x]\n", nRet); break; }

// ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera) if (stDeviceList.pDeviceInfo[nIndex]->nTLayerType == MV_GIGE_DEVICE) { int nPacketSize = MV_CC_GetOptimalPacketSize(handle); if (nPacketSize > 0) { nRet = MV_CC_SetIntValue(handle,"GevSCPSPacketSize",nPacketSize); if(nRet != MV_OK) { printf("Warning: Set Packet Size fail nRet [0x%x]!", nRet); } } else { printf("Warning: Get Packet Size fail nRet [0x%x]!", nPacketSize); } }

// ch:获取数据包大小 | en:Get payload size MVCC_INTVALUE stParam; memset(&stParam, 0, sizeof(MVCC_INTVALUE)); nRet = MV_CC_GetIntValue(handle, "PayloadSize", &stParam); if (MV_OK != nRet) { printf("Get PayloadSize fail! nRet [0x%x]\n", nRet); break; } unsigned int nPayloadSize = stParam.nCurValue;

// ch:设置相机曝光组数为"nExposureNum" | en:Set exposure num to "nExposureNum" int nRet = MV_CC_SetEnumValue(handle, "MultiLightControl", nExposureNum); if (MV_OK != nRet) { printf("Set MultiLightControl fail，nRet:[%#x]\n", nRet); } else { printf("Set MultiLightControl to [%d]\n", nExposureNum);

}

// ch:获取实际的曝光个数 | en:Get actual nExposureNum nExposureNum = nExposureNum & 0xF;

// ch:设置触发模式为off | en:Set trigger mode as off nRet = MV_CC_SetEnumValue(handle, "TriggerMode", 0); if (MV_OK != nRet) { printf("Set Trigger Mode fail! nRet [0x%x]\n", nRet); break; }

// ch:开始取流 | en:Start grab image nRet = MV_CC_StartGrabbing(handle); if (MV_OK != nRet) { printf("Start Grabbing fail! nRet [0x%x]\n", nRet); break; }

MV_FRAME_OUT stImageInfo = {0}; MV_RECONSTRUCT_IMAGE_PARAM stImgReconstructionParam = {0};

unsigned int nReconstructBufferSize = 0; MV_CC_HB_DECODE_PARAM stDecodeParam = {0};

nRet = MV_CC_GetImageBuffer(handle, &stImageInfo, 20000);//线阵相 机需要更大的图像获取间隔 if (nRet == MV_OK) { printf("Get Image Buffer: Width[%d], Height[%d], FrameNum[%d]\n", stImageInfo.stFrameInfo.nExtendWidth, stImageInfo.stFrameInfo.nExtendHeight, stImageInfo.stFrameInfo.nFrameNum);

bool bpixel = IsHBPixelFormat(stImageInfo.stFrameInfo.enPixelType); if (bpixel) { stDecodeParam.pSrcBuf = stImageInfo.pBufAddr; stDecodeParam.nSrcLen = stImageInfo.stFrameInfo.nFrameLenEx;

if (pDstBuf == NULL) {

pDstBuf = (unsigned char *)malloc(sizeof(unsigned char) * (nPayloadSize*3)); if (NULL == pDstBuf) { printf("malloc pDstData fail !\n"); nRet = MV_E_RESOURCE; break; } } stDecodeParam.pDstBuf = pDstBuf; stDecodeParam.nDstBufSize = nPayloadSize*3; nRet = MV_CC_HB_Decode(handle, &stDecodeParam); if (nRet != MV_OK) { printf("Decode fail![0x%x]\n", nRet); break; } else { printf("HB Decode success!\n"); }

stImgReconstructionParam.nWidth = stDecodeParam.nWidth; stImgReconstructionParam.nHeight = stDecodeParam.nHeight; stImgReconstructionParam.enPixelType = stDecodeParam.enDstPixelType; stImgReconstructionParam.pSrcData = stDecodeParam.pDstBuf; stImgReconstructionParam.nSrcDataLen = stDecodeParam.nDstBufLen; //申请总的拆分图像缓存，拆分后的图像保存在pReconstructBuffer中 ，后续可以进行存图处理 nReconstructBufferSize = stDecodeParam.nDstBufLen; pReconstructBuffer = (unsigned char*)malloc(sizeof(unsigned char) * nReconstructBufferSize); if(NULL == pReconstructBuffer) { printf("malloc pReconstructBuffer fail! nRet [0x%x]\n", nRet); break; }

//输出数据缓存赋值，频闪数为2 // 自上而下进行拼图 for (int i = 0; i < nExposureNum; ++i) { stImgReconstructionParam.stDstBufList[i].pBuf = pReconstructBuffer + i*(stDecodeParam.nDstBufLen / nExposureNum);//pReconstructBuffer偏移 stImgReconstructionParam.stDstBufList[i].nBufSize = stDecodeParam.nDstBufLen / nExposureNum; } }

else { //图像重构结构体赋值 stImgReconstructionParam.nWidth = stImageInfo.stFrameInfo.nExtendWidth; stImgReconstructionParam.nHeight = stImageInfo.stFrameInfo.nExtendHeight; stImgReconstructionParam.enPixelType = stImageInfo.stFrameInfo.enPixelType; stImgReconstructionParam.pSrcData = stImageInfo.pBufAddr; stImgReconstructionParam.nSrcDataLen = stImageInfo.stFrameInfo.nFrameLenEx; //申请总的拆分图像缓存，拆分后的图像保存在pReconstructBuffer中 ，后续可以进行存图处理 nReconstructBufferSize = stImageInfo.stFrameInfo.nFrameLenEx; pReconstructBuffer = (unsigned char*)malloc(sizeof(unsigned char) * nReconstructBufferSize); if(NULL == pReconstructBuffer) { printf("malloc pReconstructBuffer fail! nRet [0x%x]\n", nRet); break; }

//输出数据缓存赋值，频闪数为2 // 自上而下进行拼图 for (int i = 0; i < nExposureNum; ++i) { stImgReconstructionParam.stDstBufList[i].pBuf = pReconstructBuffer + i*(stImageInfo.stFrameInfo.nFrameLenEx / nExposureNum);//pReconstructBuffer偏移 stImgReconstructionParam.stDstBufList[i].nBufSize = stImageInfo.stFrameInfo.nFrameLenEx / nExposureNum; } } stImgReconstructionParam.nExposureNum = nExposureNum; stImgReconstructionParam.enReconstructMethod = MV_SPLIT_BY_LINE;

// Split Image And Reconstruct Image nRet = MV_CC_ReconstructImage(handle, &stImgReconstructionParam); if (MV_OK != nRet) { printf("Reconstruct Image fail! nRet [0x%x]\n", nRet); break; }

//存原始数据bmp图片 char chImageName[IMAGE_NAME_LEN] = { 0 }; MV_CC_IMAGE stImage;

memset(&stImage, 0, sizeof(MV_CC_IMAGE)); MV_CC_SAVE_IMAGE_PARAM stSaveImageParam; memset(&stSaveImageParam, 0, sizeof(MV_CC_SAVE_IMAGE_PARAM));

stImage.enPixelType = stImageInfo.stFrameInfo.enPixelType; stImage.nHeight = stImageInfo.stFrameInfo.nExtendHeight; stImage.nWidth = stImageInfo.stFrameInfo.nExtendWidth; stImage.nImageBufLen = stImageInfo.stFrameInfo.nFrameLenEx; stImage.pImageBuf = stImageInfo.pBufAddr;

stSaveImageParam.enImageType = MV_Image_Bmp; stSaveImageParam.iMethodValue = 1; stSaveImageParam.nQuality = 99;

sprintf_s(chImageName, IMAGE_NAME_LEN, "InPut_w%d_h%d_fn%03d.bmp", stImage.nWidth, stImage.nHeight, stImageInfo.stFrameInfo.nFrameNum);

nRet = MV_CC_SaveImageToFileEx2(handle, &stImage, &stSaveImageParam, chImageName); if (nRet != MV_OK) { printf("raw image save to File fail nRet[%x]\n", nRet); } else { printf("raw image save to File success,save to %s\n", chImageName); }

// 存拆分拼接后bmp图片 stImage.enPixelType = stImgReconstructionParam.enPixelType; stImage.nHeight = stImgReconstructionParam.nHeight; stImage.nWidth = stImgReconstructionParam.nWidth; stImage.nImageBufLen = nReconstructBufferSize; stImage.pImageBuf = pReconstructBuffer;

stSaveImageParam.enImageType = MV_Image_Bmp; stSaveImageParam.iMethodValue = 1; stSaveImageParam.nQuality = 99;

sprintf_s(chImageName, IMAGE_NAME_LEN, "OutPut_w%d_h%d.bmp", stImgReconstructionParam.nWidth, stImgReconstructionParam.nHeight);

nRet = MV_CC_SaveImageToFileEx2(handle, &stImage, &stSaveImageParam, chImageName); if (nRet != MV_OK) { printf("After reconstruction SaveImage To File fail nRet[%x]\n", nRet); } else { printf("After reconstruction SaveImage To File success, save to %s\n", chImageName); }

nRet = MV_CC_FreeImageBuffer(handle, &stImageInfo); if(nRet != MV_OK) { printf("Free Image Buffer fail! nRet [0x%x]\n", nRet); }

if(NULL != pReconstructBuffer) { free(pReconstructBuffer); pReconstructBuffer = NULL; }

if (NULL !=pDstBuf) { free(pDstBuf); pDstBuf = NULL; } } else { printf("Get Image fail! nRet [0x%x]\n", nRet); }

// ch:停止取流 | en:Stop grab image nRet = MV_CC_StopGrabbing(handle); if (MV_OK != nRet) { printf("Stop Grabbing fail! nRet [0x%x]\n", nRet); break; }

// ch:关闭设备 | Close device nRet = MV_CC_CloseDevice(handle); if (MV_OK != nRet) { printf("ClosDevice fail! nRet [0x%x]\n", nRet);

break; }

// ch:销毁句柄 | Destroy handle nRet = MV_CC_DestroyHandle(handle); if (MV_OK != nRet) { printf("Destroy Handle fail! nRet [0x%x]\n", nRet); break; } handle = NULL; } while (0);

if(MV_OK != nRet) { if(NULL != pDstBuf) { free (pDstBuf); pDstBuf = NULL; }

if(NULL != pReconstructBuffer) { free (pReconstructBuffer); pReconstructBuffer = NULL; } }

if (handle != NULL) { MV_CC_DestroyHandle(handle); handle = NULL; }

// ch:反初始化SDK | en:Finalize SDK MV_CC_Finalize();

printf("Press a key to exit.\n"); WaitForKeyPress();

return 0; }

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

### ParametrizeCamera_FileAccess.cpp

导入或导出文件到相机

该示例程序说明如何导入文件到相机以及如何导出相机文件。演示了通过 MV_CC_FileAccessRead() 从设备读取文件（例如UserSet1.bin）；通过 MV_CC_FileAccessWrite() 将文件（例如UserSet1.bin）写入相机；以及通过 MV_CC_GetFileAccessProgress() 获取文件的存取进度。

#include <stdio.h> #include <Windows.h> #include <process.h> #include <conio.h> #include "MvCameraControl.h"

unsigned int g_nMode = 0; int g_nRet = MV_OK; // ch:等待按键输入 | en:Wait for key press void WaitForKeyPress(void) { while(!_kbhit()) { Sleep(10); } _getch();

}

bool PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo) { if (NULL == pstMVDevInfo) { printf("The Pointer of pstMVDevInfo is NULL!\n"); return false; } if (pstMVDevInfo->nTLayerType == MV_GIGE_DEVICE) { int nIp1 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24); int nIp2 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16); int nIp3 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8); int nIp4 = (pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

// ch:打印当前相机ip和用户自定义名字 | en:print current ip and user defined name printf("CurrentIp: %d.%d.%d.%d\n" , nIp1, nIp2, nIp3, nIp4); printf("UserDefinedName: %s\n\n" , pstMVDevInfo- >SpecialInfo.stGigEInfo.chUserDefinedName); } else if (pstMVDevInfo->nTLayerType == MV_USB_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.chSerialNumber); printf("Device Number: %d\n\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.nDeviceNumber); } else { printf("Not support.\n"); }

return true; }

static unsigned int __stdcall ProgressThread(void* pUser) { int nRet = MV_OK; MV_CC_FILE_ACCESS_PROGRESS stFileAccessProgress = {0};

while(1) { //ch:获取文件存取进度 |en:Get progress of file access

nRet = MV_CC_GetFileAccessProgress(pUser, &stFileAccessProgress); printf("State = 0x%x,Completed = %I64d,Total = %I64d\r\n",nRet,stFileAccessProgress.nCompleted,stFileAccessProgr ess.nTotal); if (nRet != MV_OK || (stFileAccessProgress.nCompleted != 0 && stFileAccessProgress.nCompleted == stFileAccessProgress.nTotal)) { break; }

Sleep(50); }

return 0; }

static unsigned int __stdcall FileAccessThread(void* pUser) { MV_CC_FILE_ACCESS stFileAccess = {0};

stFileAccess.pUserFileName = "UserSet1.bin"; stFileAccess.pDevFileName = "UserSet1"; if (1 == g_nMode) { //ch:读模式 |en:Read mode g_nRet = MV_CC_FileAccessRead(pUser, &stFileAccess); if (MV_OK != g_nRet) { printf("File Access Read fail! nRet [0x%x]\n", g_nRet); } } else if (2 == g_nMode) { //ch:写模式 |en:Write mode g_nRet = MV_CC_FileAccessWrite(pUser, &stFileAccess); if (MV_OK != g_nRet) { printf("File Access Write fail! nRet [0x%x]\n", g_nRet); } }

return 0; }

int main() { int nRet = MV_OK; void* handle = NULL;

do { // ch:初始化SDK | en:Initialize SDK nRet = MV_CC_Initialize(); if (MV_OK != nRet) { printf("Initialize SDK fail! nRet [0x%x]\n", nRet); break; }

// ch:枚举设备 | en:Enum device MV_CC_DEVICE_INFO_LIST stDeviceList; memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST)); nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &stDeviceList); if (MV_OK != nRet) { printf("Enum Devices fail! nRet [0x%x]\n", nRet); break; }

if (stDeviceList.nDeviceNum > 0) { for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++) { printf("[device %d]:\n", i); MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i]; if (NULL == pDeviceInfo) { break; } PrintDeviceInfo(pDeviceInfo); } } else { printf("Find No Devices!\n"); break; }

printf("Please Input camera index(0-%d):", stDeviceList.nDeviceNum-1); unsigned int nIndex = 0; scanf_s("%d", &nIndex);

if (nIndex >= stDeviceList.nDeviceNum) { printf("Input error!\n"); break; }

// ch:选择设备并创建句柄 | en:Select device and create handle nRet = MV_CC_CreateHandle(&handle, stDeviceList.pDeviceInfo[nIndex]); if (MV_OK != nRet) { printf("Create Handle fail! nRet [0x%x]\n", nRet); break; }

// ch:打开设备 | en:Open device nRet = MV_CC_OpenDevice(handle); if (MV_OK != nRet) { printf("Open Device fail! nRet [0x%x]\n", nRet); break; }

//ch:读模式 |en:Read mode g_nMode = 1; printf("Read to file.\n");

unsigned int nThreadID = 0; void* hReadHandle = (void*) _beginthreadex( NULL , 0 , FileAccessThread , handle, 0 , &nThreadID ); if (NULL == hReadHandle) { printf("Start file access thread fail!\n"); break; }

Sleep(5);

nThreadID = 0; void* hReadProgressHandle = (void*) _beginthreadex( NULL , 0 , ProgressThread , handle, 0 , &nThreadID ); if (NULL == hReadProgressHandle) { printf("Start file progress thread fail!\n"); break; }

WaitForMultipleObjects(1, &hReadHandle, TRUE, INFINITE); WaitForMultipleObjects(1, &hReadProgressHandle, TRUE, INFINITE); if (MV_OK == g_nRet) { printf("File Access Read Success!\n"); } printf("\n");

//ch:写模式 |en:Write mode g_nMode = 2; printf("Write from file.\n");

nThreadID = 0; void* hWriteHandle = (void*) _beginthreadex( NULL , 0 , FileAccessThread , handle, 0 , &nThreadID ); if (NULL == hWriteHandle) { printf("Start file access thread fail!\n"); break; }

Sleep(5);

nThreadID = 0; void* hWriteProgressHandle = (void*) _beginthreadex( NULL , 0 , ProgressThread , handle, 0 , &nThreadID ); if (NULL == hWriteProgressHandle) { printf("Start file progress thread fail!\n"); break; }

WaitForMultipleObjects(1, &hWriteHandle, TRUE, INFINITE); WaitForMultipleObjects(1, &hWriteProgressHandle, TRUE, INFINITE); if (MV_OK == g_nRet) { printf("File Access Write Success!\n"); }

// ch:关闭设备 | Close device nRet = MV_CC_CloseDevice(handle); if (MV_OK != nRet) { printf("ClosDevice fail! nRet [0x%x]\n", nRet); break; }

// ch:销毁句柄 | Destroy handle nRet = MV_CC_DestroyHandle(handle); if (MV_OK != nRet) { printf("Destroy Handle fail! nRet [0x%x]\n", nRet); break; } handle = NULL; } while (0);

if (handle != NULL) { MV_CC_DestroyHandle(handle); handle = NULL; }

// ch:反初始化SDK | en:Finalize SDK MV_CC_Finalize();

printf("Press a key to exit.\n"); WaitForKeyPress();

return 0; }

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

### ParametrizeCamera_LineScanIOSettings.cpp

线阵相机IO设置

该示例程序演示如何通过 MV_CC_SetEnumValue() 配置线阵相机帧触发和行触发相关节点IO。

#include <stdio.h> #include <Windows.h>

#include <process.h> #include <conio.h> #include "MvCameraControl.h"

bool g_bExit = false;

// ch:等待按键输入 | en:Wait for key press void WaitForKeyPress(void) { while(!_kbhit()) { Sleep(10); } _getch(); }

bool PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo) { if (NULL == pstMVDevInfo) { printf("The Pointer of pstMVDevInfo is NULL!\n"); return false; } if (pstMVDevInfo->nTLayerType == MV_GIGE_DEVICE) { int nIp1 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24); int nIp2 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16); int nIp3 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8); int nIp4 = (pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

// ch:打印当前相机ip和用户自定义名字 | en:print current ip and user defined name printf("CurrentIp: %d.%d.%d.%d\n" , nIp1, nIp2, nIp3, nIp4); printf("UserDefinedName: %s\n\n" , pstMVDevInfo- >SpecialInfo.stGigEInfo.chUserDefinedName); } else if (pstMVDevInfo->nTLayerType == MV_USB_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.chSerialNumber); printf("Device Number: %d\n\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.nDeviceNumber); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_GIGE_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chUserDefinedName);

printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_CAMERALINK_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_CXP_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_XOF_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chModelName); } else { printf("Not support.\n"); }

return true; }

void __stdcall ImageCallBackEx(unsigned char * pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser) { if (pFrameInfo) { printf("Get One Frame: Width[%d], Height[%d], nFrameNum[%d]\n", pFrameInfo->nWidth, pFrameInfo->nHeight, pFrameInfo- >nFrameNum); } }

int main() { int nRet = MV_OK;

void* handle = NULL;

do { // ch:初始化SDK | en:Initialize SDK nRet = MV_CC_Initialize(); if (MV_OK != nRet) { printf("Initialize SDK fail! nRet [0x%x]\n", nRet); break; }

// ch:枚举设备 | en:Enum device MV_CC_DEVICE_INFO_LIST stDeviceList; memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST)); nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE | MV_GENTL_CAMERALINK_DEVICE | MV_GENTL_CXP_DEVICE | MV_GENTL_XOF_DEVICE, &stDeviceList); if (MV_OK != nRet) { printf("Enum Devices fail! nRet [0x%x]\n", nRet); break; }

if (stDeviceList.nDeviceNum > 0) { for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++) { printf("[device %d]:\n", i); MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i]; if (NULL == pDeviceInfo) { break; } PrintDeviceInfo(pDeviceInfo); } } else { printf("Find No Devices!\n"); break; }

printf("Please Input camera index(0-%d):", stDeviceList.nDeviceNum-1); unsigned int nIndex = 0; scanf_s("%d", &nIndex);

if (nIndex >= stDeviceList.nDeviceNum) { printf("Input error!\n");

break; }

// ch:选择设备并创建句柄 | en:Select device and create handle nRet = MV_CC_CreateHandle(&handle, stDeviceList.pDeviceInfo[nIndex]); if (MV_OK != nRet) { printf("Create Handle fail! nRet [0x%x]\n", nRet); break; }

// ch:打开设备 | en:Open device nRet = MV_CC_OpenDevice(handle); if (MV_OK != nRet) { printf("Open Device fail! nRet [0x%x]\n", nRet); break; }

// ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera) if (stDeviceList.pDeviceInfo[nIndex]->nTLayerType == MV_GIGE_DEVICE) { int nPacketSize = MV_CC_GetOptimalPacketSize(handle); if (nPacketSize > 0) { nRet = MV_CC_SetIntValueEx(handle,"GevSCPSPacketSize",nPacketSize); if(nRet != MV_OK) { printf("Warning: Set Packet Size fail nRet [0x%x]!\n", nRet); } } else { printf("Warning: Get Packet Size fail nRet [0x%x]!\n", nPacketSize); } }

printf("Please Input trigger selector index(0-%d): 0- FrameBurstStart, 1-LineStart\n"); unsigned int nTriggerSelector = 0; scanf_s("%d", &nTriggerSelector);

if (nTriggerSelector == 0) {

// ch:设置触发选项为FrameBurstStart | en:Set trigger selector as FrameBurstStart nRet = MV_CC_SetEnumValue(handle, "TriggerSelector", 6); if (MV_OK != nRet) { printf("Set Trigger Selector fail! nRet [0x%x]\n", nRet); break; }

// ch:设置触发模式为on | en:Set trigger mode as on nRet = MV_CC_SetEnumValue(handle, "TriggerMode", 1); if (MV_OK != nRet) { printf("Set Trigger Mode fail! nRet [0x%x]\n", nRet); break; }

// ch:设置触发源为Line0 | en:Set trigger source as Line0 nRet = MV_CC_SetEnumValue(handle, "TriggerSource", 0); if (MV_OK != nRet) { printf("Set Trigger source fail! nRet [0x%x]\n", nRet); break; } } else if (nTriggerSelector == 1) { // ch:设置触发选项为LineStart | en:Set trigger selector as LineStart nRet = MV_CC_SetEnumValue(handle, "TriggerSelector", 9); if (MV_OK != nRet) { printf("Set Trigger Selector fail! nRet [0x%x]\n", nRet); break; }

// ch:设置触发模式为on | en:Set trigger mode as on nRet = MV_CC_SetEnumValue(handle, "TriggerMode", 1); if (MV_OK != nRet) { printf("Set Trigger Mode fail! nRet [0x%x]\n", nRet); break; }

// ch:设置触发源为EncoderModuleOut | en:Set trigger source as EncoderModuleOut nRet = MV_CC_SetEnumValue(handle, "TriggerSource", 6); if (MV_OK != nRet) { printf("Set Trigger source fail! nRet [0x%x]\n", nRet);

break; }

// ch:设置编码器选项为Encoder0 | en:Set encoder selector as Encoder0 nRet = MV_CC_SetEnumValue(handle, "EncoderSelector", 0); if (MV_OK != nRet) { printf("Set encoder selector fail! nRet [0x%x]\n", nRet); break; }

// ch:设置编码器数据源A为Line1 | en:Set encoder source A as Line1 nRet = MV_CC_SetEnumValue(handle, "EncoderSourceA", 1); if (MV_OK != nRet) { printf("Set encoder sourceA fail! nRet [0x%x]\n", nRet); break; }

// ch:设置编码器数据源B为Line3 | en:Set encoder source B as Line3 nRet = MV_CC_SetEnumValue(handle, "EncoderSourceB", 3); if (MV_OK != nRet) { printf("Set encoder sourceB fail! nRet [0x%x]\n", nRet); break; } } else { printf("Input error!\n"); break; }

// ch:注册抓图回调 | en:Register image callback nRet = MV_CC_RegisterImageCallBackEx(handle, ImageCallBackEx, handle); if (MV_OK != nRet) { printf("Register Image CallBack fail! nRet [0x%x]\n", nRet); break; }

// ch:开始取流 | en:Start grab image nRet = MV_CC_StartGrabbing(handle); if (MV_OK != nRet) { printf("Start Grabbing fail! nRet [0x%x]\n", nRet);

break; }

printf("Press a key to stop grabbing.\n"); WaitForKeyPress();

// ch:停止取流 | en:Stop grab image nRet = MV_CC_StopGrabbing(handle); if (MV_OK != nRet) { printf("Stop Grabbing fail! nRet [0x%x]\n", nRet); break; }

// ch:注销抓图回调 | en:Unregister image callback nRet = MV_CC_RegisterImageCallBackEx(handle, NULL, NULL); if (MV_OK != nRet) { printf("Unregister Image CallBack fail! nRet [0x%x]\n", nRet); break; }

// ch:关闭设备 | Close device nRet = MV_CC_CloseDevice(handle); if (MV_OK != nRet) { printf("ClosDevice fail! nRet [0x%x]\n", nRet); break; }

// ch:销毁句柄 | Destroy handle nRet = MV_CC_DestroyHandle(handle); if (MV_OK != nRet) { printf("Destroy Handle fail! nRet [0x%x]\n", nRet); break; } handle = NULL; } while (0);

if (handle != NULL) { MV_CC_DestroyHandle(handle); handle = NULL; }

// ch:反初始化SDK | en:Finalize SDK MV_CC_Finalize();

printf("Press a key to exit.\n"); WaitForKeyPress();

return 0; }

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

### ParametrizeCamera_LoadAndSave.cpp

导入或导出相机属性

该示例程序说明如何导入和导出相机的属性树文件。演示了通过 MV_CC_FeatureSave() 将相机属 性导出到文件（例如FeatureFile.ini）中；通过 MV_CC_FeatureLoad() 从文件（例如 FeatureFile.ini）中导入相机的属性树。

#include <stdio.h> #include <Windows.h> #include <process.h> #include <conio.h> #include "MvCameraControl.h"

// ch:等待按键输入 | en:Wait for key press void WaitForKeyPress(void) { while(!_kbhit()) { Sleep(10); } _getch(); }

bool PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo) { if (NULL == pstMVDevInfo) { printf("The Pointer of pstMVDevInfo is NULL!\n"); return false; } if (pstMVDevInfo->nTLayerType == MV_GIGE_DEVICE) { int nIp1 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24); int nIp2 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16); int nIp3 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8); int nIp4 = (pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

// ch:打印当前相机ip和用户自定义名字 | en:print current ip and user defined name printf("CurrentIp: %d.%d.%d.%d\n" , nIp1, nIp2, nIp3, nIp4); printf("UserDefinedName: %s\n\n" , pstMVDevInfo- >SpecialInfo.stGigEInfo.chUserDefinedName); } else if (pstMVDevInfo->nTLayerType == MV_USB_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.chSerialNumber); printf("Device Number: %d\n\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.nDeviceNumber); } else { printf("Not support.\n"); }

return true; }

int main() {

int nRet = MV_OK; void* handle = NULL;

do { // ch:初始化SDK | en:Initialize SDK nRet = MV_CC_Initialize(); if (MV_OK != nRet) { printf("Initialize SDK fail! nRet [0x%x]\n", nRet); break; }

// ch:枚举设备 | en:Enum device MV_CC_DEVICE_INFO_LIST stDeviceList; memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST)); nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &stDeviceList); if (MV_OK != nRet) { printf("Enum Devices fail! nRet [0x%x]\n", nRet); break; }

if (stDeviceList.nDeviceNum > 0) { for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++) { printf("[device %d]:\n", i); MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i]; if (NULL == pDeviceInfo) { break; } PrintDeviceInfo(pDeviceInfo); } } else { printf("Find No Devices!\n"); break; }

printf("Please Input camera index(0-%d):", stDeviceList.nDeviceNum-1); unsigned int nIndex = 0; scanf_s("%d", &nIndex);

if (nIndex >= stDeviceList.nDeviceNum) { printf("Input error!\n");

break; }

// ch:选择设备并创建句柄 | en:Select device and create handle nRet = MV_CC_CreateHandle(&handle, stDeviceList.pDeviceInfo[nIndex]); if (MV_OK != nRet) { printf("Create Handle fail! nRet [0x%x]\n", nRet); break; }

// ch:打开设备 | en:Open device nRet = MV_CC_OpenDevice(handle); if (MV_OK != nRet) { printf("Open Device fail! nRet [0x%x]\n", nRet); break; }

printf("Start export the camera properties to the file\n"); printf("Wait......\n"); // ch:将相机属性导出到文件中 | en:Export the camera properties to the file

nRet = MV_CC_FeatureSave(handle, "FeatureFile.mfs"); if (MV_OK != nRet) { printf("Save Feature fail! nRet [0x%x]\n", nRet); break; } printf("Finish export the camera properties to the file\n\n");

printf("Start import the camera properties from the file\n"); printf("Wait......\n"); // ch:从文件中导入相机属性 | en:Import the camera properties from the file nRet = MV_CC_FeatureLoad(handle, "FeatureFile.mfs"); if (MV_OK != nRet) { printf("Load Feature fail! nRet [0x%x]\n", nRet); break; } printf("Finish import the camera properties from the file\n");

// ch:关闭设备 | Close device nRet = MV_CC_CloseDevice(handle); if (MV_OK != nRet) { printf("ClosDevice fail! nRet [0x%x]\n", nRet);

break; }

// ch:销毁句柄 | Destroy handle nRet = MV_CC_DestroyHandle(handle); if (MV_OK != nRet) { printf("Destroy Handle fail! nRet [0x%x]\n", nRet); break; } handle = NULL; } while (0);

if (handle != NULL) { MV_CC_DestroyHandle(handle); handle = NULL; }

// ch:反初始化SDK | en:Finalize SDK MV_CC_Finalize();

printf("Press a key to exit.\n"); WaitForKeyPress();

return 0; }

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

### QuickSoftwareTrigger.cpp

采集卡快速软触发

该示例程序说明如何使用采集卡实现快速软触发功能。快速软触发仅被部分采集卡支持，请以采集 卡实际情况为准。

#include <stdio.h> #include <Windows.h> #include <process.h> #include <conio.h> #include "MvCameraControl.h"

void* g_hInterface = NULL; bool g_bExit = false;

// ch:等待按键输入 | en:Wait for key press void WaitForKeyPress(void) { while(!_kbhit()) { Sleep(10); } _getch(); }

bool PrintInterfaceInfo(MV_INTERFACE_INFO* pstInterfaceInfo) { if (NULL == pstInterfaceInfo) { printf("The Pointer of pstInterfaceInfo is NULL!\n"); return false; } printf("Display name: %s\n",pstInterfaceInfo->chDisplayName); printf("Serial number: %s\n",pstInterfaceInfo->chSerialNumber); printf("model name: %s\n",pstInterfaceInfo->chModelName); printf("\n");

return true; }

bool PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo) { if (NULL == pstMVDevInfo) {

printf("The Pointer of pstMVDevInfo is NULL!\n"); return false; } if (pstMVDevInfo->nTLayerType == MV_GENTL_CXP_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_XOF_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chModelName); } else { printf("Not support.\n"); }

return true; }

static unsigned int __stdcall WorkThread(void* pUser) { int nRet = MV_OK; MV_FRAME_OUT stOutFrame = {0};

while(true) { // ch:通过采集卡软触发一次 | en:Software trigger once from interface nRet = MV_CC_SetCommandValue(g_hInterface, "QuickSoftwareTrigger0"); if (MV_OK != nRet) { printf("Quick Software Trigger once failed! %#x\n", nRet); } else { printf("Quick Software Trigger once success!\n"); } nRet = MV_CC_GetImageBuffer(pUser, &stOutFrame, 1000); if (nRet == MV_OK) { printf("Get Image Buffer: Width[%d], Height[%d], FrameNum[%d]\n",

stOutFrame.stFrameInfo.nWidth, stOutFrame.stFrameInfo.nHeight, stOutFrame.stFrameInfo.nFrameNum);

nRet = MV_CC_FreeImageBuffer(pUser, &stOutFrame); if(nRet != MV_OK) { printf("Free Image Buffer fail! nRet [0x%x]\n", nRet); } } else { printf("Get Image fail! nRet [0x%x]\n", nRet); } if(g_bExit) { break; } }

return 0; }

int main() { int nRet = MV_OK; void* hDevice = NULL;

do { // ch:初始化SDK | en:Initialize SDK nRet = MV_CC_Initialize(); if (MV_OK != nRet) { printf("Initialize SDK fail! nRet [0x%x]\n", nRet); break; }

MV_INTERFACE_INFO_LIST stInterfaceInfoList={0}; nRet = MV_CC_EnumInterfaces(MV_CXP_INTERFACE | MV_XOF_INTERFACE, &stInterfaceInfoList); //枚举采集卡 // enum frame grabber if (MV_OK != nRet) { printf("Enum Interfaces fail! nRet [0x%x]\n", nRet); break; }

if (stInterfaceInfoList.nInterfaceNum > 0) {

for (unsigned int i = 0; i < stInterfaceInfoList.nInterfaceNum; i++) { printf("[Interface %d]:\n", i); MV_INTERFACE_INFO* pstInterfaceInfo = stInterfaceInfoList.pInterfaceInfos[i]; if (NULL == pstInterfaceInfo) { break; } PrintInterfaceInfo(pstInterfaceInfo); } printf("Enum Interfaces success!\n\n"); } else { printf("Find No Interface!\n"); break; }

printf("Please Input Interfaces index(0-%d):", stInterfaceInfoList.nInterfaceNum-1); unsigned int nIndex = 0; scanf_s("%d", &nIndex);

if (nIndex >= stInterfaceInfoList.nInterfaceNum) { printf("Input error!\n"); break; }

//创建采集卡句柄 // create frame grabber handle nRet = MV_CC_CreateInterface(&g_hInterface, stInterfaceInfoList.pInterfaceInfos[nIndex]); if (MV_OK == nRet) { printf("Create Interface success!\n"); } else { printf("Create Interface Handle fail! nRet [0x%x]\n", nRet); break; }

//打开采集卡 // open frame grabber nRet = MV_CC_OpenInterface(g_hInterface, NULL); if (MV_OK == nRet) { printf("Open Interface success!\n"); } else

{ printf("Open Interface fail! nRet [0x%x]\n", nRet); break; }

// ch:设置数据流触发源为快速软触发0 | en:Set Stream Trigger Source to QuickSoftwareTrigger0 nRet = MV_CC_SetEnumValueByString(g_hInterface, "StreamTriggerSource", "QuickSoftwareTrigger0"); if (MV_OK != nRet) { printf("Set StreamTriggerSource fail! nRet [0x%x], maybe firmware not support Quick Software Trigger\n", nRet); break; } else { printf("Set StreamTriggerSource = QuickSoftwareTrigger0 Success!\n"); }

// ch:设置数据流触发方式为上升沿 | en:Set Stream Trigger Activation to RisingEdge nRet = MV_CC_SetEnumValueByString(g_hInterface, "StreamTriggerActivation", "RisingEdge"); if (MV_OK != nRet) { printf("Set StreamTriggerActivation Fail! nRet [0x%x]\n", nRet); break; } else { printf("Set StreamTriggerActivation = RisingEdge Success!\n"); }

MV_CC_DEVICE_INFO_LIST stDeviceList = { 0 }; nRet = MV_CC_EnumDevices(MV_GENTL_CXP_DEVICE | MV_GENTL_XOF_DEVICE, &stDeviceList); //枚举采集卡设备 // enum frame grabber device if (MV_OK != nRet) { printf("Enum Interfaces Devices fail! nRet [0x%x]\n", nRet); break; } int nDeviceNum = 0; if (stDeviceList.nDeviceNum > 0) { for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++) {

MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i]; if (NULL == pDeviceInfo) { break; } if (MV_CXP_INTERFACE == stInterfaceInfoList.pInterfaceInfos[nIndex]->nTLayerType) { if (0 == strcmp((char*)stInterfaceInfoList.pInterfaceInfos[nIndex]- >chInterfaceID, (char*)pDeviceInfo- >SpecialInfo.stCXPInfo.chInterfaceID)) { printf("[device %d]:\n", i); PrintDeviceInfo(pDeviceInfo); nDeviceNum++; } } else { if (0 == strcmp((char*)stInterfaceInfoList.pInterfaceInfos[nIndex]- >chInterfaceID, (char*)pDeviceInfo- >SpecialInfo.stXoFInfo.chInterfaceID)) { printf("[device %d]:\n", i); PrintDeviceInfo(pDeviceInfo); nDeviceNum++; } } } } else { printf("Find No Devices!\n"); break; }

if (0 == nDeviceNum) { printf("Find No Devices!\n"); break; }

printf("Please Input camera index(0-%d):", stDeviceList.nDeviceNum - 1); nIndex = 0; scanf_s("%d", &nIndex);

if (nIndex >= stDeviceList.nDeviceNum) {

printf("Input error!\n"); break; }

// ch:选择设备并创建句柄 | en:Select device and create handle nRet = MV_CC_CreateHandle(&hDevice, stDeviceList.pDeviceInfo[nIndex]); if (MV_OK != nRet) { printf("Create Handle fail! nRet [0x%x]\n", nRet); break; }

// ch:打开设备 | en:Open device nRet = MV_CC_OpenDevice(hDevice); if (MV_OK != nRet) { printf("Open Device fail! nRet [0x%x]\n", nRet); break; }

// ch:设置触发模式为off | en:Set trigger mode as off nRet = MV_CC_SetEnumValue(hDevice, "TriggerMode", 0); if (MV_OK != nRet) { printf("Set Trigger Mode fail! nRet [0x%x]\n", nRet); break; }

// ch:开始取流 | en:Start grab image nRet = MV_CC_StartGrabbing(hDevice); if (MV_OK != nRet) { printf("Start Grabbing fail! nRet [0x%x]\n", nRet); break; }

unsigned int nThreadID = 0; void* hThreadHandle = (void*) _beginthreadex( NULL , 0 , WorkThread , hDevice, 0 , &nThreadID ); if (NULL == hThreadHandle) { break; }

printf("Press a key to stop grabbing.\n"); WaitForKeyPress();

g_bExit = true;

Sleep(1000);

// ch:停止取流 | en:Stop grab image nRet = MV_CC_StopGrabbing(hDevice); if (MV_OK != nRet) { printf("Stop Grabbing fail! nRet [0x%x]\n", nRet); break; }

// ch:关闭设备 | Close device nRet = MV_CC_CloseDevice(hDevice); if (MV_OK != nRet) { printf("ClosDevice fail! nRet [0x%x]\n", nRet); break; }

// ch:销毁句柄 | Destroy handle nRet = MV_CC_DestroyHandle(hDevice); if (MV_OK != nRet) { printf("Destroy Handle fail! nRet [0x%x]\n", nRet); break; } hDevice = NULL;

//关闭采集卡 // close frame grabber nRet = MV_CC_CloseInterface(g_hInterface); if (MV_OK == nRet) { printf("Close Interface success!\n"); } else { printf("Close Interface Handle fail! nRet [0x%x]\n", nRet); break; }

//销毁采集卡句柄 // destory frame grabber handle nRet = MV_CC_DestroyInterface(g_hInterface); if (MV_OK == nRet) { printf("Destroy Interface success!\n"); } else { printf("Destroy Interface Handle fail! nRet [0x%x]\n", nRet);

break; } g_hInterface = NULL; } while (0);

if (hDevice != NULL) { MV_CC_CloseDevice(hDevice); MV_CC_DestroyHandle(hDevice); hDevice = NULL; }

if (g_hInterface != NULL) { MV_CC_CloseInterface(g_hInterface); MV_CC_DestroyInterface(g_hInterface); g_hInterface = NULL; }

// ch:反初始化SDK | en:Finalize SDK MV_CC_Finalize();

printf("Press a key to exit.\n"); WaitForKeyPress(); }

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

### Recording.cpp

视频录像存储

该示例程序演示将相机拍摄到的画面存储为视频录像。

#include <stdio.h> #include <Windows.h> #include <process.h> #include <conio.h> #include "MvCameraControl.h"

bool g_bExit = false;

// ch:等待按键输入 | en:Wait for key press void WaitForKeyPress(void) { while(!_kbhit()) { Sleep(10); } _getch(); }

bool PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo) { if (NULL == pstMVDevInfo) { printf("The Pointer of pstMVDevInfo is NULL!\n"); return false; } if (pstMVDevInfo->nTLayerType == MV_GIGE_DEVICE) { int nIp1 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24); int nIp2 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16); int nIp3 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8); int nIp4 = (pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

// ch:打印当前相机ip和用户自定义名字 | en:print current ip and user defined name printf("CurrentIp: %d.%d.%d.%d\n" , nIp1, nIp2, nIp3, nIp4); printf("UserDefinedName: %s\n\n" , pstMVDevInfo- >SpecialInfo.stGigEInfo.chUserDefinedName); } else if (pstMVDevInfo->nTLayerType == MV_USB_DEVICE)

{ printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.chSerialNumber); printf("Device Number: %d\n\n", pstMVDevInfo- >SpecialInfo.stUsb3VInfo.nDeviceNumber); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_GIGE_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stGigEInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_CAMERALINK_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stCMLInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_CXP_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stCXPInfo.chModelName); } else if (pstMVDevInfo->nTLayerType == MV_GENTL_XOF_DEVICE) { printf("UserDefinedName: %s\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chUserDefinedName); printf("Serial Number: %s\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chSerialNumber); printf("Model Name: %s\n\n", pstMVDevInfo- >SpecialInfo.stXoFInfo.chModelName); } else { printf("Not support.\n"); }

return true; }

static unsigned int __stdcall WorkThread(void* pUser) { int nRet = MV_OK;

MV_FRAME_OUT stImageInfo = {0}; MV_CC_INPUT_FRAME_INFO stInputFrameInfo = {0};

while(1) { nRet = MV_CC_GetImageBuffer(pUser, &stImageInfo, 1000); if (nRet == MV_OK) { printf("Get Image Buffer: Width[%d], Height[%d], FrameNum[%d]\n", stImageInfo.stFrameInfo.nWidth, stImageInfo.stFrameInfo.nHeight, stImageInfo.stFrameInfo.nFrameNum);

stInputFrameInfo.pData = stImageInfo.pBufAddr; stInputFrameInfo.nDataLen = stImageInfo.stFrameInfo.nFrameLen; nRet = MV_CC_InputOneFrame(pUser, &stInputFrameInfo); if (MV_OK != nRet) { printf("Input one frame fail! nRet [0x%x]\n", nRet); }

nRet = MV_CC_FreeImageBuffer(pUser, &stImageInfo); if(nRet != MV_OK) { printf("Free Image Buffer fail! nRet [0x%x]\n", nRet); } } else { printf("Get Image fail! nRet [0x%x]\n", nRet); } if(g_bExit) { break; } }

return 0; }

int main() { int nRet = MV_OK; void* handle = NULL;

do { // ch:初始化SDK | en:Initialize SDK nRet = MV_CC_Initialize();

if (MV_OK != nRet) { printf("Initialize SDK fail! nRet [0x%x]\n", nRet); break; }

// ch:枚举设备 | en:Enum device MV_CC_DEVICE_INFO_LIST stDeviceList; memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST)); nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE | MV_GENTL_CAMERALINK_DEVICE | MV_GENTL_CXP_DEVICE | MV_GENTL_XOF_DEVICE, &stDeviceList); if (MV_OK != nRet) { printf("Enum Devices fail! nRet [0x%x]\n", nRet); break; }

if (stDeviceList.nDeviceNum > 0) { for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++) { printf("[device %d]:\n", i); MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i]; if (NULL == pDeviceInfo) { break; } PrintDeviceInfo(pDeviceInfo); } } else { printf("Find No Devices!\n"); break; }

printf("Please Input camera index(0-%d):", stDeviceList.nDeviceNum-1); unsigned int nIndex = 0; scanf_s("%d", &nIndex);

if (nIndex >= stDeviceList.nDeviceNum) { printf("Input error!\n"); break; }

// ch:选择设备并创建句柄 | en:Select device and create handle nRet = MV_CC_CreateHandle(&handle, stDeviceList.pDeviceInfo[nIndex]);

if (MV_OK != nRet) { printf("Create Handle fail! nRet [0x%x]\n", nRet); break; }

// ch:打开设备 | en:Open device nRet = MV_CC_OpenDevice(handle); if (MV_OK != nRet) { printf("Open Device fail! nRet [0x%x]\n", nRet); break; }

// ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera) if (stDeviceList.pDeviceInfo[nIndex]->nTLayerType == MV_GIGE_DEVICE) { int nPacketSize = MV_CC_GetOptimalPacketSize(handle); if (nPacketSize > 0) { nRet = MV_CC_SetIntValueEx(handle,"GevSCPSPacketSize",nPacketSize); if(nRet != MV_OK) { printf("Warning: Set Packet Size fail nRet [0x%x]!", nRet); } } else { printf("Warning: Get Packet Size fail nRet [0x%x]!", nPacketSize); } }

// ch:设置触发模式为off | en:Set trigger mode as off nRet = MV_CC_SetEnumValue(handle, "TriggerMode", 0); if (MV_OK != nRet) { printf("Set Trigger Mode fail! nRet [0x%x]\n", nRet); break; }

// ch:设置HB模式为off | en:Set HB mode as off nRet = MV_CC_SetEnumValue(handle,"ImageCompressionMode", 0); if (MV_OK != nRet) { printf("Get ImageCompressionMode fail! nRet [0x%x]\n", nRet); }

MV_CC_RECORD_PARAM stRecordPar;

MVCC_INTVALUE stParam = {0}; nRet = MV_CC_GetIntValue(handle, "Width", &stParam); if (MV_OK != nRet) { printf("Get Width fail! nRet [0x%x]\n", nRet); break; } stRecordPar.nWidth = stParam.nCurValue;

nRet = MV_CC_GetIntValue(handle, "Height", &stParam); if (MV_OK != nRet) { printf("Get Height fail! nRet [0x%x]\n", nRet); break; } stRecordPar.nHeight = stParam.nCurValue;

MVCC_ENUMVALUE stEnumValue = {0}; nRet = MV_CC_GetEnumValue(handle, "PixelFormat", &stEnumValue); if (MV_OK != nRet) { printf("Get Width fail! nRet [0x%x]\n", nRet); break; } stRecordPar.enPixelType = MvGvspPixelType(stEnumValue.nCurValue);

MVCC_FLOATVALUE stFloatValue; nRet = MV_CC_GetFloatValue(handle, "ResultingFrameRate", &stFloatValue); if (MV_OK != nRet) { printf("Get Float value fail! nRet [0x%x]\n", nRet); break; } // ch:帧率(大于1/16)fps | en:Frame Rate (>1/16)fps stRecordPar.fFrameRate = stFloatValue.fCurValue; // ch:码率kbps(128kbps-16Mbps) | en:Bitrate kbps(128kbps-16Mbps) stRecordPar.nBitRate = 1000; // ch:录像格式(仅支持AVI) | en:Record Format(AVI is only supported) stRecordPar.enRecordFmtType = MV_FormatType_AVI; stRecordPar.strFilePath= "./Recording.avi"; nRet = MV_CC_StartRecord(handle,&stRecordPar); if (MV_OK != nRet) { printf("Start Record fail! nRet [0x%x]\n", nRet); break;

}

// ch:开始取流 | en:Start grab image nRet = MV_CC_StartGrabbing(handle); if (MV_OK != nRet) { printf("Start Grabbing fail! nRet [0x%x]\n", nRet); break; }

unsigned int nThreadID = 0; void* hThreadHandle = (void*) _beginthreadex( NULL , 0 , WorkThread , handle, 0 , &nThreadID ); if (NULL == hThreadHandle) { printf("Start work thread fail! \n"); break; }

printf("Press a key to stop grabbing.\n"); WaitForKeyPress();

g_bExit = true; Sleep(1000);

// ch:停止取流 | en:Stop grab image nRet = MV_CC_StopGrabbing(handle); if (MV_OK != nRet) { printf("Stop Grabbing fail! nRet [0x%x]\n", nRet); break; }

nRet = MV_CC_StopRecord(handle); if (MV_OK != nRet) { printf("Stop record fail! nRet [0x%x]\n", nRet); break; }

// ch:关闭设备 | Close device nRet = MV_CC_CloseDevice(handle); if (MV_OK != nRet) { printf("ClosDevice fail! nRet [0x%x]\n", nRet); break; }

// ch:销毁句柄 | Destroy handle nRet = MV_CC_DestroyHandle(handle); if (MV_OK != nRet) { printf("Destroy Handle fail! nRet [0x%x]\n", nRet); break; } handle = NULL; } while (0);

if (handle != NULL) { MV_CC_DestroyHandle(handle); handle = NULL; }

// ch:反初始化SDK | en:Finalize SDK MV_CC_Finalize();

printf("Press a key to exit.\n"); WaitForKeyPress();

return 0; }

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

### 常见问题

### 一、问题排查思路

1. 基于SDK开发的程序异常，建议先运行客户端，查看相应功能是否正常

2. 如果客户端正常，但基于SDK开发的程序异常，需要重点排查二次开发的程序问题

3. 如果客户端也异常，请参考以下典型问题的解决方法，看是否能匹配上
4. 若根据以上排查思路仍不能解决问题，请记录问题现象或图片、客户端版本号（在客户端 的帮助菜单查看）和dll库信息（名称和版本号），联系本公司技术同事获取支持

### 二、典型问题解决方法

1、GigE相机问题

问题 1：用客户端看图像有很多丢包。

问题原因：网络传输环境异常，导致数据传输丢包。

解决方法：1、保证网络有足够带宽。2、开启网卡巨帧。3、关闭系统防火墙。4、调节

SCPD，慢慢调大，直到不丢包为止，如下图:

问题 2：Demo编译无报错，但link时有错误。

问题原因：检查下demo运行目录是否有管理员权限，导致其无法写入exe文件。

解决方法：demo运行目录要改成有管理员权限的目录。

问题 3：DHCP下无法设置静态IP？

问题原因：相机端未发布版本对网关做了限制，0.0.0.0的会显示失败。

解决方法：重新升级固件。

问题 4：调用sdk执行软触发命令时，软触发命令执行失败。

问题原因：设置触发源不是软触发。

解决方法：调用sdk时，在执行软触发命令时，先保证相机处于软触发模式以及触发源为

软触发。

问题 5：相机跑一会出现断线情况。

问题原因：网卡休眠或者网口连接处松动。

解决方法：1、设置操作系统的电源选项，使计算机不要进入睡眠状态。2、确认下网口是

否松掉了。

问题 6：调用 MV_CC_OpenDevice() 返回无权限。

问题原因：1、该相机已经被其它程序占用了。2、可能是设置心跳时间过长，而程序没有

执行关闭设备或者销毁设备句柄接口，直接异常退出（在调试模式下常见），导致设备一直

被占用着。

解决方法：1、检查是否已经有别的程序占用了相机，或者在程序中已经被连接。2、需要

等心跳时间到或者插拔下相机电源。

问题 7：调试过程中返回错误码。

问题原因：调试时会引起心跳发送超时。

解决方法：可以把心跳时间设长（如30s，及设置值为30000），默认是3s，如下图：

问题 8：调用 MV_CC_GetOneFrameTimeout() 接口，发现

MV_CC_GetOneFrameTimeout() 返回无数据错误。

问题原因：对 MV_CC_GetOneFrameTimeout() 接口不理解，认为调用一次一定能取到

数据。

解决方法：需要强调 MV_CC_GetOneFrameTimeout() 是主动查询方式，不是每次都能

获取到数据的，可以增大超时时间。

问题 9：用 MV_CC_GetOneFrameTimeout() 始终无数据。

问题原因：同时调用了注册图像回调函数。

解决方法：注册回调和主动获取图像的方式，是不能同时并存的。

问题 10：调试状态异常结束后相机无法打开。

问题原因：在调试状态下，相机的心跳值会被默认设置成60000ms（60s），目的是避免

用户在Visual Studio集成开发环境调试状态下心跳发送超时的情况，但会出现调试状态异常

关闭后相机无法打开问题。

解决方法：建议客户在退出调试之前先将相机关闭。

2、U3V相机问题

问题 1：客户端取不到数据，或者帧率远低于实际帧率。

问题原因：相机接在2.0的口上，带宽不足。

解决方法：可以通过以下方式确认相机连接口是USB2.0口还是USB3.0口。若确认接在

USB2.0口上，则需改接USB3.0口，来确保相机正常使用。

1）设备列表的图标是否显示为2

2）设备属性中的USB Speed Mode属性值是HighSpeed还是SuperSpeed，HighSpeed表

示所使用的是USB2.0口，SuperSpeed表示所使用的的是USB3.0口。

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

### 法律声明

在法律允许的最大范围内，本文档是“按照现状”提供，可能存在瑕疵或错误。本公司不对本文档提 供任何形式的明示或默示保证，包括但不限于适销性、质量满意度、适合特定目的、不侵犯第三方 权利等保证；亦不对使用或是分发本文档导致的任何特殊、附带、偶然或间接的损害进行赔偿，包 括但不限于商业利润损失、系统故障、数据或文档丢失产生的损失。

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

samples

函数 | 变量

### ImageSave.cpp 文件参考

### 函数

void WaitForKeyPress (void)

bool PrintDeviceInfo (MV_CC_DEVICE_INFO *pstMVDevInfo)

int SaveImage (MV_SAVE_IAMGE_TYPE enSaveImageType, MV_FRAME_OUT *stImageInfo)

bool IsHBPixelFormat (MvGvspPixelType ePixelType)

int main ()

### 变量

bool g_bExit = false

void * handle = NULL

### 函数说明

### ￿ WaitForKeyPress()

void WaitForKeyPress ( void )

示例

ImageSave.cpp.

### ￿ PrintDeviceInfo()

bool PrintDeviceInfo ( MV_CC_DEVICE_INFO * pstMVDevInfo )

示例

ImageSave.cpp.

### ￿ SaveImage()

int SaveImage ( MV_SAVE_IAMGE_TYPE enSaveImageType,

MV_FRAME_OUT * stImageInfo

)

示例

ImageSave.cpp.

### ￿ IsHBPixelFormat()

bool IsHBPixelFormat ( MvGvspPixelType ePixelType )

示例

ImageSave.cpp.

### ￿ main()

int main ( )

示例

ImageSave.cpp.

### 变量说明

### ￿ g_bExit

bool g_bExit = false

示例

ImageSave.cpp.

### ￿ handle

void* handle = NULL

示例

ChunkData.cpp, ConnectSpecCamera.cpp, ConvertPixelType.cpp, DynamicallyLoadDLL.cpp, Events.cpp, ForceIPEx.cpp,

Grab_ActionCommand.cpp, Grab_Asynchronous.cpp, Grab_Callback.cpp, GrabImage.cpp, GrabImage_Display.cpp, GrabStrategies.cpp, HighBandwidthDecode.cpp, ImageContrast.cpp, ImageSave.cpp, LensShadingCorrection.cpp, MultiCast.cpp, MultiLightCtrl_ImageStitching.cpp, ParametrizeCamera_FileAccess.cpp, ParametrizeCamera_LineScanIOSettings.cpp, ParametrizeCamera_LoadAndSave.cpp , 以及 Recording.cpp.

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

samples

函数

### ChunkData.cpp 文件参考

### 函数

void WaitForKeyPress (void)

bool PrintDeviceInfo (MV_CC_DEVICE_INFO *pstMVDevInfo)

void __stdcall ImageCallBackEx (unsigned char *pData, MV_FRAME_OUT_INFO_EX *pFrameInfo, void *pUser)

int main ()

### 函数说明

### ￿ WaitForKeyPress()

void WaitForKeyPress ( void )

示例

ChunkData.cpp.

### ￿ PrintDeviceInfo()

bool PrintDeviceInfo ( MV_CC_DEVICE_INFO * pstMVDevInfo )

示例

ChunkData.cpp.

### ￿ ImageCallBackEx()

void __stdcall ImageCallBackEx ( unsigned char * pData,

MV_FRAME_OUT_INFO_EX * pFrameInfo,

void * pUser

)

示例

ChunkData.cpp.

### ￿ main()

int main ( )

示例

ChunkData.cpp.

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

samples

函数 | 变量

### ConnectSpecCamera.cpp 文件参考

### 函数

void WaitForKeyPress (void)

static unsigned int __stdcall WorkThread (void *pUser)

int main ()

### 变量

bool g_bExit = false

### 函数说明

### ￿ WaitForKeyPress()

void WaitForKeyPress ( void )

示例

ConnectSpecCamera.cpp.

### ￿ WorkThread()

pUsestati static unsigned int __stdcall WorkThread ( void *r )c

示例

ConnectSpecCamera.cpp.

### ￿ main()

int main ( )

示例

ConnectSpecCamera.cpp.

### 变量说明

### ￿ g_bExit

bool g_bExit = false

示例

ConnectSpecCamera.cpp.

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

samples

函数

### ConvertPixelType.cpp 文件参考

### 函数

void WaitForKeyPress (void)

bool PrintDeviceInfo (MV_CC_DEVICE_INFO *pstMVDevInfo)

bool IsColor (MvGvspPixelType enType)

bool IsMono (MvGvspPixelType enType)

int main ()

### 函数说明

### ￿ WaitForKeyPress()

void WaitForKeyPress ( void )

示例

ConvertPixelType.cpp.

### ￿ PrintDeviceInfo()

bool PrintDeviceInfo ( MV_CC_DEVICE_INFO * pstMVDevInfo )

示例

ConvertPixelType.cpp.

### ￿ IsColor()

bool IsColor ( MvGvspPixelType enType )

示例

ConvertPixelType.cpp.

### ￿ IsMono()

bool IsMono ( MvGvspPixelType enType )

示例

ConvertPixelType.cpp.

### ￿ main()

int main ( )

示例

ConvertPixelType.cpp.

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

samples

类型定义 | 函数 | 变量

### DynamicallyLoadDLL.cpp 文件参考

### 类型定义

typedef int(__stdcall * DLL_Initialize) ()

typedef int(__stdcall * DLL_Finalize) ()

typedef unsigned int(__stdcall * DLL_GetSDKVersion) ()

typedef int(__stdcall * DLL_EnumerateTls) ()

typedef int(__stdcall * DLL_EnumDevices) (IN unsigned int nTLayerType, IN OUT MV_CC_DEVICE_INFO_LIST *pstDevList)

typedef int(__stdcall * DLL_EnumDevicesEx) (IN unsigned int nTLayerType, IN OUT MV_CC_DEVICE_INFO_LIST *pstDevList, IN const char *pManufacturerName)

typedef bool(__stdcall * DLL_IsDeviceAccessible) (IN MV_CC_DEVICE_INFO *pstDevInfo, IN unsigned int nAccessMode)

typedef int(__stdcall * DLL_CreateHandle) (OUT void **handle, IN const MV_CC_DEVICE_INFO *pstDevInfo)

typedef int(__stdcall * DLL_CreateHandleWithoutLog) (OUT void **handle, IN const MV_CC_DEVICE_INFO *pstDevInfo)

typedef int(__stdcall * DLL_DestroyHandle) (IN void *handle)

typedef int(__stdcall * DLL_OpenDevice) (IN void *handle, IN unsigned int nAccessMode, IN unsigned short nSwitchoverKey)

typedef int(__stdcall * DLL_CloseDevice) (IN void *handle)

typedef int(__stdcall * DLL_RegisterImageCallBackEx) (void *handle, void(__stdcall *cbOutput) (unsigned char *pData, MV_FRAME_OUT_INFO_EX *pFrameInfo, void *pUser), void *pUser)

typedef int(__stdcall * DLL_RegisterImageCallBackForRGB) (void *handle, void(__stdcall *cbOutput) (unsigned char *pData, MV_FRAME_OUT_INFO_EX *pFrameInfo, void *pUser), void *pUser)

typedef int(__stdcall * DLL_RegisterImageCallBackForBGR) (void *handle, void(__stdcall *cbOutput) (unsigned char *pData, MV_FRAME_OUT_INFO_EX *pFrameInfo, void *pUser), void *pUser)

typedef int(__stdcall * DLL_StartGrabbing) (IN void *handle)

typedef int(__stdcall * DLL_StopGrabbing) (IN void *handle)

typedef int(__stdcall * DLL_GetImageForRGB) (IN void *handle, IN OUT unsigned char *pData, IN unsigned int nDataSize, IN OUT MV_FRAME_OUT_INFO_EX *pFrameInfo, int nMsec)

typedef int(__stdcall * DLL_GetImageForBGR) (IN void *handle, IN OUT unsigned char *pData, IN unsigned int nDataSize, IN OUT MV_FRAME_OUT_INFO_EX *pFrameInfo, int nMsec)

typedef int(__stdcall * DLL_GetImageBuffer) (IN void *handle, MV_FRAME_OUT *pFrameInfo, int nMsec)

typedef int(__stdcall * DLL_FreeImageBuffer) (IN void *handle, MV_FRAME_OUT *pFrameInfo)

typedef int(__stdcall * DLL_GetOneFrameTimeout) (IN void *handle, IN OUT unsigned char *pData, IN unsigned int nDataSize, IN OUT MV_FRAME_OUT_INFO_EX *pFrameInfo, int nMsec)

typedef int(__stdcall * DLL_Display) (IN void *handle, void *hWnd)

typedef int(__stdcall * DLL_DisplayOneFrame) (IN void *handle, IN MV_DISPLAY_FRAME_INFO *pDisplayInfo)

typedef int(__stdcall * DLL_SetImageNodeNum) (IN void *handle, unsigned int nNum)

typedef int(__stdcall * DLL_GetDeviceInfo) (IN void *handle, IN OUT MV_CC_DEVICE_INFO *pstDevInfo)

typedef int(__stdcall * DLL_GetAllMatchInfo) (IN void *handle, IN OUT MV_ALL_MATCH_INFO *pstInfo)

typedef int(__stdcall * DLL_GetIntValueEx) (IN void *handle, IN const char *strKey, OUT MVCC_INTVALUE_EX *pIntValue)

typedef int(__stdcall * DLL_SetIntValueEx) (IN void *handle, IN const char *strKey, IN int64_t nValue)

typedef int(__stdcall * DLL_GetEnumValue) (IN void *handle, IN const char *strValue, OUT MVCC_ENUMVALUE *pEnumValue)

typedef int(__stdcall * DLL_SetEnumValue) (IN void *handle, IN const char *strValue, IN unsigned int nValue)

typedef int(__stdcall * DLL_SetEnumValueByString) (IN void *handle, IN const char *strValue, IN const char *sValue)

typedef int(__stdcall * DLL_GetFloatValue) (IN void *handle, IN const char *strValue, OUT MVCC_FLOATVALUE *pFloatValue)

typedef int(__stdcall * DLL_SetFloatValue) (IN void *handle, IN const char *strValue, IN float fValue)

typedef int(__stdcall * DLL_GetBoolValue) (IN void *handle, IN const char *strValue, OUT bool *pBoolValue)

typedef int(__stdcall * DLL_SetBoolValue) (IN void *handle, IN const char *strValue, IN bool bValue)

typedef int(__stdcall * DLL_GetStringValue) (IN void *handle, IN const char *strKey, OUT MVCC_STRINGVALUE *pStringValue)

typedef int(__stdcall * DLL_SetStringValue) (IN void *handle, IN const char *strKey, IN const char *sValue)

typedef int(__stdcall * DLL_SetCommandValue) (IN void *handle, IN const char *strValue)

typedef int(__stdcall * DLL_LocalUpgrade) (IN void *handle, const void *pFilePathName)

typedef int(__stdcall * DLL_GetUpgradeProcess) (IN void *handle, unsigned int *pnProcess)

typedef int(__stdcall * DLL_GetOptimalPacketSize) (IN void *handle)

typedef int(__stdcall * DLL_ReadMemory) (IN void *handle, void *pBuffer, int64_t nAddress, int64_t nLength)

typedef int(__stdcall * DLL_WriteMemory) (IN void *handle, const void *pBuffer, int64_t nAddress, int64_t nLength)

typedef int(__stdcall * DLL_RegisterExceptionCallBack) (IN void *handle, void(__stdcall *cbException) (unsigned int nMsgType, void *pUser), void *pUser)

typedef int(__stdcall * DLL_RegisterEventCallBackEx) (void *handle, const char *pEventName, void(__stdcall *cbEvent) (MV_EVENT_OUT_INFO *pEventInfo, void *pUser), void *pUser)

typedef int(__stdcall * DLL_RegisterAllEventCallBack) (void *handle, void(__stdcall *cbEvent) (MV_EVENT_OUT_INFO *pEventInfo, void *pUser), void *pUser)

typedef int(__stdcall * DLL_ForceIpEx) (IN void *handle, unsigned int nIP, unsigned int nSubNetMask, unsigned int nDefaultGateWay)

typedef int(__stdcall * DLL_SetIpConfig) (IN void *handle, unsigned int nType)

typedef int(__stdcall * DLL_SetNetTransMode) (IN void *handle, unsigned int nType)

typedef int(__stdcall * DLL_GetNetTransInfo) (IN void *handle, MV_NETTRANS_INFO *pstInfo)

typedef int(__stdcall * DLL_SetGvcpTimeout) (IN void *handle, unsigned int nMillisec)

typedef int(__stdcall * DLL_SetResend) (IN void *handle, unsigned int bEnable, unsigned int nMaxResendPercent, unsigned int nResendTimeout)

typedef int(__stdcall * DLL_SetTransmissionType) (IN void *handle, MV_TRANSMISSION_TYPE *pstTransmissionType)

typedef int(__stdcall * DLL_IssueActionCommand) (IN MV_ACTION_CMD_INFO *pstActionCmdInfo, OUT MV_ACTION_CMD_RESULT_LIST *pstActionCmdResults)

typedef int(__stdcall * DLL_SetDeviceBauderate) (IN void *handle, unsigned int nBaudrate)

typedef int(__stdcall * DLL_GetDeviceBauderate) (IN void *handle, unsigned int *pnCurrentBaudrate)

typedef int(__stdcall * DLL_GetSupportBauderates) (IN void *handle, unsigned int *pnBaudrateAblity)

typedef int(__stdcall * DLL_SetGenCPTimeOut) (IN void *handle, unsigned int nMillisec)

typedef int(__stdcall * DLL_GetGenICamXML) (IN void *handle, IN OUT unsigned char *pData, IN unsigned int nDataSize, OUT unsigned int *pnDataLen)

typedef int(__stdcall * DLL_SaveImageEx2) (IN void *handle, IN OUT MV_SAVE_IMAGE_PARAM_EX *pSaveParam)

typedef int(__stdcall * DLL_ConvertPixelType) (IN void *handle, IN OUT MV_CC_PIXEL_CONVERT_PARAM *pstCvtParam)

typedef int(__stdcall * DLL_SetBayerCvtQuality) (IN void *handle, IN unsigned int BayerCvtQuality)

typedef int(__stdcall * DLL_FeatureSave) (IN void *handle, IN const char *pFileName)

typedef int(__stdcall * DLL_FeatureLoad) (IN void *handle, IN const char *pFileName)

typedef int(__stdcall * DLL_FileAccessRead) (IN void *handle, IN MV_CC_FILE_ACCESS *pstFileAccess)

typedef int(__stdcall * DLL_FileAccessWrite) (IN void *handle, IN MV_CC_FILE_ACCESS *pstFileAccess)

typedef int(__stdcall * DLL_GetFileAccessProgress) (IN void *handle, OUT MV_CC_FILE_ACCESS_PROGRESS *pstFileAccessProgress)

typedef int(__stdcall * DLL_StartRecord) (IN void *handle, IN MV_CC_RECORD_PARAM *pstRecordParam)

typedef int(__stdcall * DLL_InputOneFrame) (IN void *handle, IN MV_CC_INPUT_FRAME_INFO *pstInputFrameInfo)

typedef int(__stdcall * DLL_StopRecord) (IN void *handle)

### 函数

void WaitForKeyPress (void)

bool PrintDeviceInfo (MV_CC_DEVICE_INFO *pstMVDevInfo)

static unsigned int __stdcall WorkThread (void *stMuthreadPar)

int main ()

### 变量

bool g_bExit = false

### 类型定义说明

### ￿ DLL_Initialize

typedef int(__stdcall * DLL_Initialize) ()

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_Finalize

typedef int(__stdcall * DLL_Finalize) ()

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_GetSDKVersion

typedef unsigned int(__stdcall * DLL_GetSDKVersion) ()

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_EnumerateTls

typedef int(__stdcall * DLL_EnumerateTls) ()

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_EnumDevices

typedef int(__stdcall * DLL_EnumDevices) (IN unsigned int nTLayerType, IN OUT MV_CC_DEVICE_INFO_LIST *pstDevList)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_EnumDevicesEx

typedef int(__stdcall * DLL_EnumDevicesEx) (IN unsigned int nTLayerType, IN OUT MV_CC_DEVICE_INFO_LIST *pstDevList, IN const char *pManufacturerName)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_IsDeviceAccessible

typedef bool(__stdcall * DLL_IsDeviceAccessible) (IN MV_CC_DEVICE_INFO *pstDevInfo, IN unsigned int nAccessMode)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_CreateHandle

typedef int(__stdcall * DLL_CreateHandle) (OUT void **handle, IN const MV_CC_DEVICE_INFO *pstDevInfo)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_CreateHandleWithoutLog

typedef int(__stdcall * DLL_CreateHandleWithoutLog) (OUT void **handle, IN const MV_CC_DEVICE_INFO *pstDevInfo)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_DestroyHandle

typedef int(__stdcall * DLL_DestroyHandle) (IN void *handle)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_OpenDevice

typedef int(__stdcall * DLL_OpenDevice) (IN void *handle, IN unsigned int nAccessMode, IN unsigned short nSwitchoverKey)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_CloseDevice

typedef int(__stdcall * DLL_CloseDevice) (IN void *handle)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_RegisterImageCallBackEx

typedef int(__stdcall * DLL_RegisterImageCallBackEx) (void *handle, void(__stdcall *cbOutput)(unsigned char *pData, MV_FRAME_OUT_INFO_EX *pFrameInfo, void *pUser), void *pUser)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_RegisterImageCallBackForRGB

typedef int(__stdcall * DLL_RegisterImageCallBackForRGB) (void *handle, void(__stdcall *cbOutput)(unsigned char *pData, MV_FRAME_OUT_INFO_EX *pFrameInfo, void *pUser), void *pUser)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_RegisterImageCallBackForBGR

typedef int(__stdcall * DLL_RegisterImageCallBackForBGR) (void *handle, void(__stdcall *cbOutput)(unsigned char *pData, MV_FRAME_OUT_INFO_EX *pFrameInfo, void *pUser), void *pUser)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_StartGrabbing

typedef int(__stdcall * DLL_StartGrabbing) (IN void *handle)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_StopGrabbing

typedef int(__stdcall * DLL_StopGrabbing) (IN void *handle)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_GetImageForRGB

typedef int(__stdcall * DLL_GetImageForRGB) (IN void *handle, IN OUT unsigned char *pData, IN unsigned int nDataSize, IN OUT MV_FRAME_OUT_INFO_EX *pFrameInfo, int nMsec)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_GetImageForBGR

typedef int(__stdcall * DLL_GetImageForBGR) (IN void *handle, IN OUT unsigned char *pData, IN unsigned int nDataSize, IN OUT MV_FRAME_OUT_INFO_EX *pFrameInfo, int nMsec)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_GetImageBuffer

typedef int(__stdcall * DLL_GetImageBuffer) (IN void *handle, MV_FRAME_OUT *pFrameInfo, int nMsec)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_FreeImageBuffer

typedef int(__stdcall * DLL_FreeImageBuffer) (IN void *handle, MV_FRAME_OUT *pFrameInfo)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_GetOneFrameTimeout

typedef int(__stdcall * DLL_GetOneFrameTimeout) (IN void *handle, IN OUT unsigned char *pData, IN unsigned int nDataSize, IN OUT MV_FRAME_OUT_INFO_EX *pFrameInfo, int nMsec)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_Display

typedef int(__stdcall * DLL_Display) (IN void *handle, void *hWnd)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_DisplayOneFrame

typedef int(__stdcall * DLL_DisplayOneFrame) (IN void *handle, IN MV_DISPLAY_FRAME_INFO *pDisplayInfo)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_SetImageNodeNum

typedef int(__stdcall * DLL_SetImageNodeNum) (IN void *handle, unsigned int nNum)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_GetDeviceInfo

typedef int(__stdcall * DLL_GetDeviceInfo) (IN void *handle, IN OUT MV_CC_DEVICE_INFO *pstDevInfo)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_GetAllMatchInfo

typedef int(__stdcall * DLL_GetAllMatchInfo) (IN void *handle, IN OUT MV_ALL_MATCH_INFO *pstInfo)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_GetIntValueEx

typedef int(__stdcall * DLL_GetIntValueEx) (IN void *handle, IN const char *strKey, OUT MVCC_INTVALUE_EX *pIntValue)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_SetIntValueEx

typedef int(__stdcall * DLL_SetIntValueEx) (IN void *handle, IN const char *strKey, IN int64_t nValue)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_GetEnumValue

typedef int(__stdcall * DLL_GetEnumValue) (IN void *handle, IN const char *strValue, OUT MVCC_ENUMVALUE *pEnumValue)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_SetEnumValue

typedef int(__stdcall * DLL_SetEnumValue) (IN void *handle, IN const char *strValue, IN unsigned int nValue)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_SetEnumValueByString

typedef int(__stdcall * DLL_SetEnumValueByString) (IN void *handle, IN const char *strValue, IN const char *sValue)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_GetFloatValue

typedef int(__stdcall * DLL_GetFloatValue) (IN void *handle, IN const char *strValue, OUT MVCC_FLOATVALUE *pFloatValue)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_SetFloatValue

typedef int(__stdcall * DLL_SetFloatValue) (IN void *handle, IN const char *strValue, IN float fValue)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_GetBoolValue

typedef int(__stdcall * DLL_GetBoolValue) (IN void *handle, IN const char *strValue, OUT bool *pBoolValue)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_SetBoolValue

typedef int(__stdcall * DLL_SetBoolValue) (IN void *handle, IN const char *strValue, IN bool bValue)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_GetStringValue

typedef int(__stdcall * DLL_GetStringValue) (IN void *handle, IN const char *strKey, OUT MVCC_STRINGVALUE *pStringValue)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_SetStringValue

typedef int(__stdcall * DLL_SetStringValue) (IN void *handle, IN const char *strKey, IN const char *sValue)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_SetCommandValue

typedef int(__stdcall * DLL_SetCommandValue) (IN void *handle, IN const char *strValue)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_LocalUpgrade

typedef int(__stdcall * DLL_LocalUpgrade) (IN void *handle, const void *pFilePathName)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_GetUpgradeProcess

typedef int(__stdcall * DLL_GetUpgradeProcess) (IN void *handle, unsigned int *pnProcess)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_GetOptimalPacketSize

typedef int(__stdcall * DLL_GetOptimalPacketSize) (IN void *handle)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_ReadMemory

typedef int(__stdcall * DLL_ReadMemory) (IN void *handle, void *pBuffer, int64_t nAddress, int64_t nLength)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_WriteMemory

typedef int(__stdcall * DLL_WriteMemory) (IN void *handle, const void *pBuffer, int64_t nAddress, int64_t nLength)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_RegisterExceptionCallBack

typedef int(__stdcall * DLL_RegisterExceptionCallBack) (IN void *handle, void(__stdcall *cbException)(unsigned int nMsgType, void *pUser), void *pUser)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_RegisterEventCallBackEx

typedef int(__stdcall * DLL_RegisterEventCallBackEx) (void *handle, const char *pEventName, void(__stdcall *cbEvent)(MV_EVENT_OUT_INFO *pEventInfo, void *pUser), void *pUser)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_RegisterAllEventCallBack

typedef int(__stdcall * DLL_RegisterAllEventCallBack) (void *handle, void(__stdcall *cbEvent)(MV_EVENT_OUT_INFO *pEventInfo, void *pUser), void *pUser)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_ForceIpEx

typedef int(__stdcall * DLL_ForceIpEx) (IN void *handle, unsigned int nIP, unsigned int nSubNetMask, unsigned int nDefaultGateWay)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_SetIpConfig

typedef int(__stdcall * DLL_SetIpConfig) (IN void *handle, unsigned int nType)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_SetNetTransMode

typedef int(__stdcall * DLL_SetNetTransMode) (IN void *handle, unsigned int nType)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_GetNetTransInfo

typedef int(__stdcall * DLL_GetNetTransInfo) (IN void *handle, MV_NETTRANS_INFO *pstInfo)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_SetGvcpTimeout

typedef int(__stdcall * DLL_SetGvcpTimeout) (IN void *handle, unsigned int nMillisec)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_SetResend

typedef int(__stdcall * DLL_SetResend) (IN void *handle, unsigned int bEnable, unsigned int nMaxResendPercent, unsigned int nResendTimeout)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_SetTransmissionType

typedef int(__stdcall * DLL_SetTransmissionType) (IN void *handle, MV_TRANSMISSION_TYPE *pstTransmissionType)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_IssueActionCommand

typedef int(__stdcall * DLL_IssueActionCommand) (IN MV_ACTION_CMD_INFO *pstActionCmdInfo, OUT MV_ACTION_CMD_RESULT_LIST *pstActionCmdResults)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_SetDeviceBauderate

typedef int(__stdcall * DLL_SetDeviceBauderate) (IN void *handle, unsigned int nBaudrate)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_GetDeviceBauderate

typedef int(__stdcall * DLL_GetDeviceBauderate) (IN void *handle, unsigned int *pnCurrentBaudrate)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_GetSupportBauderates

typedef int(__stdcall * DLL_GetSupportBauderates) (IN void *handle, unsigned int *pnBaudrateAblity)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_SetGenCPTimeOut

typedef int(__stdcall * DLL_SetGenCPTimeOut) (IN void *handle, unsigned int nMillisec)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_GetGenICamXML

typedef int(__stdcall * DLL_GetGenICamXML) (IN void *handle, IN OUT unsigned char *pData, IN unsigned int nDataSize, OUT unsigned int *pnDataLen)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_SaveImageEx2

typedef int(__stdcall * DLL_SaveImageEx2) (IN void *handle, IN OUT MV_SAVE_IMAGE_PARAM_EX *pSaveParam)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_ConvertPixelType

typedef int(__stdcall * DLL_ConvertPixelType) (IN void *handle, IN OUT MV_CC_PIXEL_CONVERT_PARAM *pstCvtParam)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_SetBayerCvtQuality

typedef int(__stdcall * DLL_SetBayerCvtQuality) (IN void *handle, IN unsigned int BayerCvtQuality)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_FeatureSave

typedef int(__stdcall * DLL_FeatureSave) (IN void *handle, IN const char *pFileName)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_FeatureLoad

typedef int(__stdcall * DLL_FeatureLoad) (IN void *handle, IN const char *pFileName)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_FileAccessRead

typedef int(__stdcall * DLL_FileAccessRead) (IN void *handle, IN MV_CC_FILE_ACCESS *pstFileAccess)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_FileAccessWrite

typedef int(__stdcall * DLL_FileAccessWrite) (IN void *handle, IN MV_CC_FILE_ACCESS *pstFileAccess)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_GetFileAccessProgress

typedef int(__stdcall * DLL_GetFileAccessProgress) (IN void *handle, OUT MV_CC_FILE_ACCESS_PROGRESS *pstFileAccessProgress)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_StartRecord

typedef int(__stdcall * DLL_StartRecord) (IN void *handle, IN MV_CC_RECORD_PARAM *pstRecordParam)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_InputOneFrame

typedef int(__stdcall * DLL_InputOneFrame) (IN void *handle, IN MV_CC_INPUT_FRAME_INFO *pstInputFrameInfo)

示例

DynamicallyLoadDLL.cpp.

### ￿ DLL_StopRecord

typedef int(__stdcall * DLL_StopRecord) (IN void *handle)

示例

DynamicallyLoadDLL.cpp.

### 函数说明

### ￿ WaitForKeyPress()

void WaitForKeyPress ( void )

示例

DynamicallyLoadDLL.cpp.

### ￿ PrintDeviceInfo()

bool PrintDeviceInfo ( MV_CC_DEVICE_INFO * pstMVDevInfo )

示例

DynamicallyLoadDLL.cpp.

### ￿ WorkThread()

stMuthreadPastati static unsigned int __stdcall WorkThread ( void *r )c

示例

DynamicallyLoadDLL.cpp.

### ￿ main()

int main ( )

示例

DynamicallyLoadDLL.cpp.

### 变量说明

### ￿ g_bExit

bool g_bExit = false

示例

DynamicallyLoadDLL.cpp.

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

samples

函数

### Events.cpp 文件参考

### 函数

void WaitForKeyPress (void)

bool PrintDeviceInfo (MV_CC_DEVICE_INFO *pstMVDevInfo)

void __stdcall EventCallBack (MV_EVENT_OUT_INFO *pEventInfo, void *pUser)

int main ()

### 函数说明

### ￿ WaitForKeyPress()

void WaitForKeyPress ( void )

示例

Events.cpp.

### ￿ PrintDeviceInfo()

bool PrintDeviceInfo ( MV_CC_DEVICE_INFO * pstMVDevInfo )

示例

Events.cpp.

### ￿ EventCallBack()

void __stdcall EventCallBack ( MV_EVENT_OUT_INFO * pEventInfo,

void * pUser

)

示例

Events.cpp.

### ￿ main()

int main ( )

示例

Events.cpp.

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

samples

函数

### Events_Interface.cpp 文件参考

### 函数

bool PrintDeviceInfo (MV_CC_DEVICE_INFO_LIST &stDeviceList)

bool PrintInterfaceInfo (MV_INTERFACE_INFO_LIST &stInterfaceInfoList)

void __stdcall EventCallBack (MV_EVENT_OUT_INFO *pEventInfo, void *pUser)

void __stdcall ImageCallBackEx (unsigned char *pData, MV_FRAME_OUT_INFO_EX *pFrameInfo, void *pUser)

void WaitForKeyPress (void)

int main ()

### 函数说明

### ￿ PrintDeviceInfo()

bool PrintDeviceInfo ( MV_CC_DEVICE_INFO_LIST & stDeviceList )

示例

Events_Interface.cpp.

### ￿ PrintInterfaceInfo()

bool PrintInterfaceInfo ( MV_INTERFACE_INFO_LIST & stInterfaceInfoList )

示例

Events_Interface.cpp.

### ￿ EventCallBack()

void __stdcall EventCallBack ( MV_EVENT_OUT_INFO * pEventInfo,

void * pUser

)

示例

Events_Interface.cpp.

### ￿ ImageCallBackEx()

void __stdcall ImageCallBackEx ( unsigned char * pData,

MV_FRAME_OUT_INFO_EX * pFrameInfo,

void * pUser

)

示例

Events_Interface.cpp.

### ￿ WaitForKeyPress()

void WaitForKeyPress ( void )

示例

Events_Interface.cpp.

### ￿ main()

int main ( )

示例

Events_Interface.cpp.

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

samples

函数 | 变量

### InterfaceDemo.cpp 文件参考

### 函数

void WaitForKeyPress (void)

bool PrintInterfaceInfo (MV_INTERFACE_INFO *pstInterfaceInfo)

void Set_Get_Enum (const char *str)

void Set_Get_Bool (const char *str)

void Set_Get_Int (const char *str)

void Set_Get_String (const char *str)

void Set_Get_Float (const char *str)

int main ()

### 变量

void * hInterface = NULL

### 函数说明

### ￿ WaitForKeyPress()

void WaitForKeyPress ( void )

示例

InterfaceDemo.cpp.

### ￿ PrintInterfaceInfo()

bool PrintInterfaceInfo ( MV_INTERFACE_INFO * pstInterfaceInfo )

示例

InterfaceDemo.cpp.

### ￿ Set_Get_Enum()

void Set_Get_Enum ( const char * str )

示例

InterfaceDemo.cpp.

### ￿ Set_Get_Bool()

void Set_Get_Bool ( const char * str )

示例

InterfaceDemo.cpp.

### ￿ Set_Get_Int()

void Set_Get_Int ( const char * str )

示例

InterfaceDemo.cpp.

### ￿ Set_Get_String()

void Set_Get_String ( const char * str )

示例

InterfaceDemo.cpp.

### ￿ Set_Get_Float()

void Set_Get_Float ( const char * str )

示例

InterfaceDemo.cpp.

### ￿ main()

int main ( )

示例

InterfaceDemo.cpp.

### 变量说明

### ￿ hInterface

void* hInterface = NULL

示例 Events_Interface.cpp, InterfaceAndDeviceDemo.cpp , 以及 InterfaceDemo.cpp.

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

samples

宏定义 | 函数

### ForceIPEx.cpp 文件参考

### 宏定义

#define PERSISTENT_CONFIG_REG 0x0014

#define PERSISTENT_IPADDR_REG 0x064C

#define PERSISTENT_SUBNETMASK_REG 0x065C

#define PERSISTENT_DEFAULTGATEWAY_REG 0x066C

### 函数

void WaitForKeyPress (void)

bool PrintDeviceInfo (MV_CC_DEVICE_INFO *pstMVDevInfo)

bool ConvertToHexIp (unsigned int *nHexIP, unsigned int *nDecIP, char c)

int main ()

### 宏定义说明

### ￿ PERSISTENT_CONFIG_REG

#define PERSISTENT_CONFIG_REG 0x0014

示例

ForceIPEx.cpp.

### ￿ PERSISTENT_IPADDR_REG

#define PERSISTENT_IPADDR_REG 0x064C

示例

ForceIPEx.cpp.

### ￿ PERSISTENT_SUBNETMASK_REG

#define PERSISTENT_SUBNETMASK_REG 0x065C

示例

ForceIPEx.cpp.

### ￿ PERSISTENT_DEFAULTGATEWAY_REG

#define PERSISTENT_DEFAULTGATEWAY_REG 0x066C

示例

ForceIPEx.cpp.

### 函数说明

### ￿ WaitForKeyPress()

void WaitForKeyPress ( void )

示例

ForceIPEx.cpp.

### ￿ PrintDeviceInfo()

bool PrintDeviceInfo ( MV_CC_DEVICE_INFO * pstMVDevInfo )

示例

ForceIPEx.cpp.

### ￿ ConvertToHexIp()

bool ConvertToHexIp ( unsigned int * nHexIP,

unsigned int * nDecIP,

char c

)

示例

ForceIPEx.cpp.

### ￿ main()

int main ( )

示例

ForceIPEx.cpp.

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

samples

函数 | 变量

### Grab_ActionCommand.cpp 文件参考

### 函数

void WaitForKeyPress (void)

bool PrintDeviceInfo (MV_CC_DEVICE_INFO *pstMVDevInfo)

static unsigned int __stdcall ActionCommandWorkThread (void *pUser)

static unsigned int __stdcall ReceiveImageWorkThread (void *pUser)

int main ()

### 变量

bool g_bExit = false

unsigned int g_DeviceKey = 1

unsigned int g_GroupKey = 1

unsigned int g_GroupMask = 1

### 函数说明

### ￿ WaitForKeyPress()

void WaitForKeyPress ( void )

示例

Grab_ActionCommand.cpp.

### ￿ PrintDeviceInfo()

bool PrintDeviceInfo ( MV_CC_DEVICE_INFO * pstMVDevInfo )

示例

Grab_ActionCommand.cpp.

### ￿ ActionCommandWorkThread()

pUsestati static unsigned int __stdcall ActionCommandWorkThread ( void *r ) c

示例

Grab_ActionCommand.cpp.

### ￿ ReceiveImageWorkThread()

pUse stati static unsigned int __stdcall ReceiveImageWorkThread ( void *r )c

示例

Grab_ActionCommand.cpp.

### ￿ main()

int main ( )

示例

Grab_ActionCommand.cpp.

### 变量说明

### ￿ g_bExit

bool g_bExit = false

示例

Grab_ActionCommand.cpp.

### ￿ g_DeviceKey

unsigned int g_DeviceKey = 1

示例

Grab_ActionCommand.cpp.

### ￿ g_GroupKey

unsigned int g_GroupKey = 1

示例

Grab_ActionCommand.cpp.

### ￿ g_GroupMask

unsigned int g_GroupMask = 1

示例

Grab_ActionCommand.cpp.

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

samples

宏定义 | 函数 | 变量

### Grab_Asynchronous.cpp 文件参考

### 宏定义

#define Max_Count 5

### 函数

void WaitForKeyPress (void)

bool PrintDeviceInfo (MV_CC_DEVICE_INFO *pstMVDevInfo)

void __stdcall ImageCallBackEx (unsigned char *pData, MV_FRAME_OUT_INFO_EX *pFrameInfo, void *pUser)

static unsigned int __stdcall WorkThread (void *pUser)

int main ()

### 变量

bool g_bExit = false

int m_nImageSize = 0

CMVMutex * g_mutex = NULL

ArrayQueue * m_queue = NULL

### 宏定义说明

### ￿ Max_Count

#define Max_Count 5

示例

Grab_Asynchronous.cpp.

### 函数说明

### ￿ WaitForKeyPress()

void WaitForKeyPress ( void )

示例

Grab_Asynchronous.cpp.

### ￿ PrintDeviceInfo()

bool PrintDeviceInfo ( MV_CC_DEVICE_INFO * pstMVDevInfo )

示例

Grab_Asynchronous.cpp.

### ￿ ImageCallBackEx()

void __stdcall ImageCallBackEx ( unsigned char * pData,

MV_FRAME_OUT_INFO_EX * pFrameInfo,

void * pUser

)

示例

Grab_Asynchronous.cpp.

### ￿ WorkThread()

pUsestati static unsigned int __stdcall WorkThread ( void *r ) c

示例

Grab_Asynchronous.cpp.

### ￿ main()

int main ( )

示例

Grab_Asynchronous.cpp.

### 变量说明

### ￿ g_bExit

bool g_bExit = false

示例

Grab_Asynchronous.cpp.

### ￿ m_nImageSize

int m_nImageSize = 0

示例

Grab_Asynchronous.cpp.

### ￿ g_mutex

CMVMutex* g_mutex = NULL

示例

Grab_Asynchronous.cpp.

### ￿ m_queue

ArrayQueue* m_queue = NULL

示例

Grab_Asynchronous.cpp.

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

samples

函数

### Grab_Callback.cpp 文件参考

### 函数

void WaitForKeyPress (void)

bool PrintDeviceInfo (MV_CC_DEVICE_INFO *pstMVDevInfo)

void __stdcall ImageCallBackEx (unsigned char *pData, MV_FRAME_OUT_INFO_EX *pFrameInfo, void *pUser)

int main ()

### 函数说明

### ￿ WaitForKeyPress()

void WaitForKeyPress ( void )

示例

Grab_Callback.cpp.

### ￿ PrintDeviceInfo()

bool PrintDeviceInfo ( MV_CC_DEVICE_INFO * pstMVDevInfo )

示例

Grab_Callback.cpp.

### ￿ ImageCallBackEx()

void __stdcall ImageCallBackEx ( unsigned char * pData,

MV_FRAME_OUT_INFO_EX * pFrameInfo,

void * pUser

)

示例

Grab_Callback.cpp.

### ￿ main()

int main ( )

示例 Grab_Callback.cpp.

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

samples

函数 | 变量

### GrabImage.cpp 文件参考

### 函数

void WaitForKeyPress (void)

bool PrintDeviceInfo (MV_CC_DEVICE_INFO *pstMVDevInfo)

static unsigned int __stdcall WorkThread (void *pUser)

int main ()

### 变量

bool g_bExit = false

### 函数说明

### ￿ WaitForKeyPress()

void WaitForKeyPress ( void )

示例

GrabImage.cpp.

### ￿ PrintDeviceInfo()

bool PrintDeviceInfo ( MV_CC_DEVICE_INFO * pstMVDevInfo )

示例

GrabImage.cpp.

### ￿ WorkThread()

pUsestati static unsigned int __stdcall WorkThread ( void *r ) c

示例

GrabImage.cpp.

### ￿ main()

int main ( )

示例

GrabImage.cpp.

### 变量说明

### ￿ g_bExit

bool g_bExit = false

示例

GrabImage.cpp.

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

samples

函数 | 变量

### GrabImage_Display.cpp 文件参考

### 函数

void WaitForKeyPress (void)

bool PrintDeviceInfo (MV_CC_DEVICE_INFO *pstMVDevInfo)

LRESULT CALLBACK WndProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)

static unsigned int __stdcall CreateRenderWindow (void *pUser)

static unsigned int __stdcall WorkThread (void *pUser)

int main ()

### 变量

HWND g_hwnd = NULL

bool g_bExit = false

### 函数说明

### ￿ WaitForKeyPress()

void WaitForKeyPress ( void )

示例

GrabImage_Display.cpp.

### ￿ PrintDeviceInfo()

bool PrintDeviceInfo ( MV_CC_DEVICE_INFO * pstMVDevInfo )

示例

GrabImage_Display.cpp.

### ￿ WndProc()

LRESULT CALLBACK WndProc ( HWND hWnd,

UINT msg,

WPARAM wParam,

LPARAM lParam

)

示例

GrabImage_Display.cpp.

### ￿ CreateRenderWindow()

pUsestati static unsigned int __stdcall CreateRenderWindow ( void *r ) c

示例

GrabImage_Display.cpp.

### ￿ WorkThread()

pUsestati static unsigned int __stdcall WorkThread ( void *r )c

示例

GrabImage_Display.cpp.

### ￿ main()

int main ( )

示例

GrabImage_Display.cpp.

### 变量说明

### ￿ g_hwnd

HWND g_hwnd = NULL

示例

GrabImage_Display.cpp.

### ￿ g_bExit

bool g_bExit = false

示例

GrabImage_Display.cpp.

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

samples

函数

### GrabStrategies.cpp 文件参考

### 函数

void WaitForKeyPress (void)

bool PrintDeviceInfo (MV_CC_DEVICE_INFO *pstMVDevInfo)

static unsigned int __stdcall UpcomingThread (void *pUser)

int main ()

### 函数说明

### ￿ WaitForKeyPress()

void WaitForKeyPress ( void )

示例

GrabStrategies.cpp.

### ￿ PrintDeviceInfo()

bool PrintDeviceInfo ( MV_CC_DEVICE_INFO * pstMVDevInfo )

示例

GrabStrategies.cpp.

### ￿ UpcomingThread()

pUsestati static unsigned int __stdcall UpcomingThread ( void *r )c

示例

GrabStrategies.cpp.

### ￿ main()

int main ( )

示例

GrabStrategies.cpp.

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

samples

宏定义 | 函数

### HighBandwidthDecode.cpp 文件参考

### 宏定义

#define IMAGE_NAME_LEN 256

### 函数

void WaitForKeyPress (void)

bool PrintDeviceInfo (MV_CC_DEVICE_INFO *pstMVDevInfo)

int main ()

### 宏定义说明

### ￿ IMAGE_NAME_LEN

#define IMAGE_NAME_LEN 256

示例

HighBandwidthDecode.cpp.

### 函数说明

### ￿ WaitForKeyPress()

void WaitForKeyPress ( void )

示例

HighBandwidthDecode.cpp.

### ￿ PrintDeviceInfo()

bool PrintDeviceInfo ( MV_CC_DEVICE_INFO * pstMVDevInfo )

示例

HighBandwidthDecode.cpp.

### ￿ main()

int main ( )

示例

HighBandwidthDecode.cpp.

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

samples

函数

### ImageContrast.cpp 文件参考

### 函数

void WaitForKeyPress (void)

bool PrintDeviceInfo (MV_CC_DEVICE_INFO *pstMVDevInfo)

int main ()

### 函数说明

### ￿ WaitForKeyPress()

void WaitForKeyPress ( void )

示例

ImageContrast.cpp.

### ￿ PrintDeviceInfo()

bool PrintDeviceInfo ( MV_CC_DEVICE_INFO * pstMVDevInfo )

示例

ImageContrast.cpp.

### ￿ main()

int main ( )

示例

ImageContrast.cpp.

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

samples

函数

### InterfaceAndDeviceDemo.cpp 文件参考

### 函数

bool PrintDeviceInfo (MV_CC_DEVICE_INFO_LIST &stDeviceList)

bool PrintInterfaceInfo (MV_INTERFACE_INFO_LIST &stInterfaceInfoList)

void __stdcall ImageCallBackEx (unsigned char *pData, MV_FRAME_OUT_INFO_EX *pFrameInfo, void *pUser)

void WaitForKeyPress (void)

int main ()

### 函数说明

### ￿ PrintDeviceInfo()

bool PrintDeviceInfo ( MV_CC_DEVICE_INFO_LIST & stDeviceList )

示例

InterfaceAndDeviceDemo.cpp.

### ￿ PrintInterfaceInfo()

bool PrintInterfaceInfo ( MV_INTERFACE_INFO_LIST & stInterfaceInfoList )

示例

InterfaceAndDeviceDemo.cpp.

### ￿ ImageCallBackEx()

void __stdcall ImageCallBackEx ( unsigned char * pData,

MV_FRAME_OUT_INFO_EX * pFrameInfo,

void * pUser

)

示例

InterfaceAndDeviceDemo.cpp.

### ￿ WaitForKeyPress()

void WaitForKeyPress ( void )

示例

InterfaceAndDeviceDemo.cpp.

### ￿ main()

int main ( )

示例

InterfaceAndDeviceDemo.cpp.

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

samples

函数 | 变量

### LensShadingCorrection.cpp 文件参考

### 函数

void WaitForKeyPress (void)

bool PrintDeviceInfo (MV_CC_DEVICE_INFO *pstMVDevInfo)

void __stdcall ImageCallBackEx (unsigned char *pData, MV_FRAME_OUT_INFO_EX *pFrameInfo, void *pUser)

int main ()

### 变量

unsigned char * g_pDstData = NULL

unsigned int g_nDstDataSize = 0

unsigned char * g_pCalibBuf = NULL

unsigned int g_nCalibBufSize = 0

bool g_IsNeedCalib = true

### 函数说明

### ￿ WaitForKeyPress()

void WaitForKeyPress ( void )

示例

LensShadingCorrection.cpp.

### ￿ PrintDeviceInfo()

bool PrintDeviceInfo ( MV_CC_DEVICE_INFO * pstMVDevInfo )

示例

LensShadingCorrection.cpp.

### ￿ ImageCallBackEx()

void __stdcall ImageCallBackEx ( unsigned char * pData,

MV_FRAME_OUT_INFO_EX * pFrameInfo,

void * pUser

)

示例

LensShadingCorrection.cpp.

### ￿ main()

int main ( )

示例

LensShadingCorrection.cpp.

### 变量说明

### ￿ g_pDstData

unsigned char* g_pDstData = NULL

示例

LensShadingCorrection.cpp.

### ￿ g_nDstDataSize

unsigned int g_nDstDataSize = 0

示例

LensShadingCorrection.cpp.

### ￿ g_pCalibBuf

unsigned char* g_pCalibBuf = NULL

示例

LensShadingCorrection.cpp.

### ￿ g_nCalibBufSize

unsigned int g_nCalibBufSize = 0

示例

LensShadingCorrection.cpp.

### ￿ g_IsNeedCalib

bool g_IsNeedCalib = true

示例

LensShadingCorrection.cpp.

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

samples

函数 | 变量

### MultiCast.cpp 文件参考

### 函数

void WaitForKeyPress (void)

bool PrintDeviceInfo (MV_CC_DEVICE_INFO *pstMVDevInfo)

static unsigned int __stdcall WorkThread (void *pUser)

int main ()

### 变量

bool g_bExit = false

### 函数说明

### ￿ WaitForKeyPress()

void WaitForKeyPress ( void )

示例

MultiCast.cpp.

### ￿ PrintDeviceInfo()

bool PrintDeviceInfo ( MV_CC_DEVICE_INFO * pstMVDevInfo )

示例

MultiCast.cpp.

### ￿ WorkThread()

pUse stati static unsigned int __stdcall WorkThread ( void *r )c

示例

MultiCast.cpp.

### ￿ main()

int main ( )

示例

MultiCast.cpp.

### 变量说明

### ￿ g_bExit

bool g_bExit = false

示例

MultiCast.cpp.

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

samples

宏定义 | 函数 | 变量

### MultiLightCtrl_ImageStitching.cpp 文件参考

### 宏定义

#define IMAGE_NAME_LEN 256

### 函数

void WaitForKeyPress (void)

bool IsHBPixelFormat (MvGvspPixelType ePixelType)

bool PrintDeviceInfo (MV_CC_DEVICE_INFO *pstMVDevInfo)

int main ()

### 变量

bool g_bExit = false

### 宏定义说明

### ￿ IMAGE_NAME_LEN

#define IMAGE_NAME_LEN 256

示例

MultiLightCtrl_ImageStitching.cpp.

### 函数说明

### ￿ WaitForKeyPress()

void WaitForKeyPress ( void )

示例

MultiLightCtrl_ImageStitching.cpp.

### ￿ IsHBPixelFormat()

bool IsHBPixelFormat ( MvGvspPixelType ePixelType )

示例

MultiLightCtrl_ImageStitching.cpp.

### ￿ PrintDeviceInfo()

bool PrintDeviceInfo ( MV_CC_DEVICE_INFO * pstMVDevInfo )

示例

MultiLightCtrl_ImageStitching.cpp.

### ￿ main()

int main ( )

示例

MultiLightCtrl_ImageStitching.cpp.

### 变量说明

### ￿ g_bExit

bool g_bExit = false

示例

MultiLightCtrl_ImageStitching.cpp.

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

samples

函数 | 变量

### ParametrizeCamera_FileAccess.cpp 文件参考

### 函数

void WaitForKeyPress (void)

bool PrintDeviceInfo (MV_CC_DEVICE_INFO *pstMVDevInfo)

static unsigned int __stdcall ProgressThread (void *pUser)

static unsigned int __stdcall FileAccessThread (void *pUser)

int main ()

### 变量

unsigned int g_nMode = 0

int g_nRet = MV_OK

### 函数说明

### ￿ WaitForKeyPress()

void WaitForKeyPress ( void )

示例

ParametrizeCamera_FileAccess.cpp.

### ￿ PrintDeviceInfo()

bool PrintDeviceInfo ( MV_CC_DEVICE_INFO * pstMVDevInfo )

示例

ParametrizeCamera_FileAccess.cpp.

### ￿ ProgressThread()

pUsestati static unsigned int __stdcall ProgressThread ( void *r )c

示例

ParametrizeCamera_FileAccess.cpp.

### ￿ FileAccessThread()

pUsestati static unsigned int __stdcall FileAccessThread ( void *r )c

示例

ParametrizeCamera_FileAccess.cpp.

### ￿ main()

int main ( )

示例

ParametrizeCamera_FileAccess.cpp.

### 变量说明

### ￿ g_nMode

unsigned int g_nMode = 0

示例

ParametrizeCamera_FileAccess.cpp.

### ￿ g_nRet

int g_nRet = MV_OK

示例

ParametrizeCamera_FileAccess.cpp.

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

samples

函数 | 变量

### ParametrizeCamera_LineScanIOSettings.cpp 文件参考

### 函数

void WaitForKeyPress (void)

bool PrintDeviceInfo (MV_CC_DEVICE_INFO *pstMVDevInfo)

void __stdcall ImageCallBackEx (unsigned char *pData, MV_FRAME_OUT_INFO_EX *pFrameInfo, void *pUser)

int main ()

### 变量

bool g_bExit = false

### 函数说明

### ￿ WaitForKeyPress()

void WaitForKeyPress ( void )

示例

ParametrizeCamera_LineScanIOSettings.cpp.

### ￿ PrintDeviceInfo()

bool PrintDeviceInfo ( MV_CC_DEVICE_INFO * pstMVDevInfo )

示例

ParametrizeCamera_LineScanIOSettings.cpp.

### ￿ ImageCallBackEx()

void __stdcall ImageCallBackEx ( unsigned char * pData,

MV_FRAME_OUT_INFO_EX * pFrameInfo,

void * pUser

)

示例

ParametrizeCamera_LineScanIOSettings.cpp.

### ￿ main()

int main ( )

示例

ParametrizeCamera_LineScanIOSettings.cpp.

### 变量说明

### ￿ g_bExit

bool g_bExit = false

示例

ParametrizeCamera_LineScanIOSettings.cpp.

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

samples

函数

### ParametrizeCamera_LoadAndSave.cpp 文件参考

### 函数

void WaitForKeyPress (void)

bool PrintDeviceInfo (MV_CC_DEVICE_INFO *pstMVDevInfo)

int main ()

### 函数说明

### ￿ WaitForKeyPress()

void WaitForKeyPress ( void )

示例

ParametrizeCamera_LoadAndSave.cpp.

### ￿ PrintDeviceInfo()

bool PrintDeviceInfo ( MV_CC_DEVICE_INFO * pstMVDevInfo )

示例

ParametrizeCamera_LoadAndSave.cpp.

### ￿ main()

int main ( )

示例

ParametrizeCamera_LoadAndSave.cpp.

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

samples

函数 | 变量

### QuickSoftwareTrigger.cpp 文件参考

### 函数

void WaitForKeyPress (void)

bool PrintInterfaceInfo (MV_INTERFACE_INFO *pstInterfaceInfo)

bool PrintDeviceInfo (MV_CC_DEVICE_INFO *pstMVDevInfo)

static unsigned int __stdcall WorkThread (void *pUser)

int main ()

### 变量

void * g_hInterface = NULL

bool g_bExit = false

### 函数说明

### ￿ WaitForKeyPress()

void WaitForKeyPress ( void )

示例

QuickSoftwareTrigger.cpp.

### ￿ PrintInterfaceInfo()

bool PrintInterfaceInfo ( MV_INTERFACE_INFO * pstInterfaceInfo )

示例

QuickSoftwareTrigger.cpp.

### ￿ PrintDeviceInfo()

bool PrintDeviceInfo ( MV_CC_DEVICE_INFO * pstMVDevInfo )

示例

QuickSoftwareTrigger.cpp.

### ￿ WorkThread()

pUsestati static unsigned int __stdcall WorkThread ( void *r ) c

示例

QuickSoftwareTrigger.cpp.

### ￿ main()

int main ( )

示例

QuickSoftwareTrigger.cpp.

### 变量说明

### ￿ g_hInterface

void* g_hInterface = NULL

示例

QuickSoftwareTrigger.cpp.

### ￿ g_bExit

bool g_bExit = false

示例

QuickSoftwareTrigger.cpp.

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

samples

函数 | 变量

### Recording.cpp 文件参考

### 函数

void WaitForKeyPress (void)

bool PrintDeviceInfo (MV_CC_DEVICE_INFO *pstMVDevInfo)

static unsigned int __stdcall WorkThread (void *pUser)

int main ()

### 变量

bool g_bExit = false

### 函数说明

### ￿ WaitForKeyPress()

void WaitForKeyPress ( void )

示例

Recording.cpp.

### ￿ PrintDeviceInfo()

bool PrintDeviceInfo ( MV_CC_DEVICE_INFO * pstMVDevInfo )

示例 Recording.cpp.

### ￿ WorkThread()

pUsestati static unsigned int __stdcall WorkThread ( void *r )c

示例

Recording.cpp.

### ￿ main()

int main ( )

示例

Recording.cpp.

### 变量说明

### ￿ g_bExit

bool g_bExit = false

示例

Recording.cpp.

首页

发版说明

编程引导

API参考

结构体定义

相机参数节点表

示例程序

常见问题

法律声明

samples

### samples 目录参考

### 文件

文件 ChunkData.cpp

文件 ConnectSpecCamera.cpp

文件 ConvertPixelType.cpp

文件 DynamicallyLoadDLL.cpp

文件 Events.cpp

文件 Events_Interface.cpp

文件 ForceIPEx.cpp

文件 Grab_ActionCommand.cpp

文件 Grab_Asynchronous.cpp

文件 Grab_Callback.cpp

文件 GrabImage.cpp

文件 GrabImage_Display.cpp

文件 GrabStrategies.cpp

文件 HighBandwidthDecode.cpp

文件 ImageContrast.cpp

文件 ImageSave.cpp

文件 InterfaceAndDeviceDemo.cpp

文件 InterfaceDemo.cpp

文件 LensShadingCorrection.cpp

文件 MultiCast.cpp

文件 MultiLightCtrl_ImageStitching.cpp

文件 ParametrizeCamera_FileAccess.cpp

文件 ParametrizeCamera_LineScanIOSettings.cpp

文件 ParametrizeCamera_LoadAndSave.cpp

文件 QuickSoftwareTrigger.cpp

文件 Recording.cpp