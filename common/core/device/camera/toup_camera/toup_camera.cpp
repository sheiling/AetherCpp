#include "toup_camera.h"

#include <d3d9types.h>
#include <Windows.h>
#include <cmath>
#include <cstring>
#include <thread>
#include <vector>

#include "../camera_error_codes.h"

namespace common
{
namespace device
{
using namespace common::errors::camera_module;

static uint8_t CheckSumFun(uint8_t* data, uint16_t len)
{
	uint8_t nCheckSum = 0;
	for (uint16_t i = 0; i < len; i++)
	{
		nCheckSum += data[i];
	}
	return nCheckSum;
}

static std::string RawFormatTrans(unsigned nFourCC)
{
	if (nFourCC == MAKEFOURCC('G', 'B', 'R', 'G'))
	{
		return "GBGR";
	}
	else if (nFourCC == MAKEFOURCC('R', 'G', 'G', 'B'))
	{
		return "RGGB";
	}
	else if (nFourCC == MAKEFOURCC('B', 'G', 'G', 'R'))
	{
		return "BGGR";
	}
	else if (nFourCC == MAKEFOURCC('G', 'R', 'B', 'G'))
	{
		return "GRBG";
	}
	else if (nFourCC == MAKEFOURCC('Y', 'Y', 'Y', 'Y'))
	{
		return "YYYY";
	}
	else if (nFourCC == MAKEFOURCC('Y', '4', '1', '1'))
	{
		return "YUV411";
	}
	else if (nFourCC == MAKEFOURCC('V', 'U', 'Y', 'Y'))
	{
		return "YUV422";
	}
	else if (nFourCC == MAKEFOURCC('U', 'Y', 'V', 'Y'))
	{
		return "YUV422";
	}
	else if (nFourCC == MAKEFOURCC('Y', '4', '4', '4'))
	{
		return "YUV444";
	}
	else if (nFourCC == MAKEFOURCC('R', 'G', 'B', '8'))
	{
		return "RGB888";
	}
	else
	{
		return "UNKNOWN";
	}
}

static common::imgproc::ImageFormat ToImageFormat(const std::string& pixelFormat, int bitDepth)
{
	using common::imgproc::ImageFormat;

	if (pixelFormat == "RGGB") {
		if (bitDepth <= 8) return ImageFormat::BAYER_RG8;
		if (bitDepth <= 10) return ImageFormat::BAYER_RG10;
		if (bitDepth <= 12) return ImageFormat::BAYER_RG12;
		if (bitDepth <= 14) return ImageFormat::BAYER_RG14;
		return ImageFormat::BAYER_RG16;
	}
	if (pixelFormat == "BGGR") {
		if (bitDepth <= 8) return ImageFormat::BAYER_BG8;
		if (bitDepth <= 10) return ImageFormat::BAYER_BG10;
		if (bitDepth <= 12) return ImageFormat::BAYER_BG12;
		if (bitDepth <= 14) return ImageFormat::BAYER_BG14;
		return ImageFormat::BAYER_BG16;
	}
	if (pixelFormat == "GBGR") {
		if (bitDepth <= 8) return ImageFormat::BAYER_GB8;
		if (bitDepth <= 10) return ImageFormat::BAYER_GB10;
		if (bitDepth <= 12) return ImageFormat::BAYER_GB12;
		if (bitDepth <= 14) return ImageFormat::BAYER_GB14;
		return ImageFormat::BAYER_GB16;
	}
	if (pixelFormat == "GRBG") {
		if (bitDepth <= 8) return ImageFormat::BAYER_GR8;
		if (bitDepth <= 10) return ImageFormat::BAYER_GR10;
		if (bitDepth <= 12) return ImageFormat::BAYER_GR12;
		if (bitDepth <= 14) return ImageFormat::BAYER_GR14;
		return ImageFormat::BAYER_GR16;
	}
	if (pixelFormat == "YYYY") {
		if (bitDepth <= 8) return ImageFormat::MONO8;
		if (bitDepth <= 10) return ImageFormat::MONO10;
		if (bitDepth <= 12) return ImageFormat::MONO12;
		if (bitDepth <= 14) return ImageFormat::MONO14;
		return ImageFormat::MONO16;
	}
	if (pixelFormat == "RGB888") {
		return ImageFormat::RGB24;
	}
	return ImageFormat::UNKNOWN;
}

void __stdcall ToupCamEventCallBackHandler(unsigned nEvent, void* ctxEvent)
{
	ToupCamera* toupCamera = reinterpret_cast<ToupCamera*>(ctxEvent);
	switch (nEvent)
	{
	case TOUPCAM_EVENT_IMAGE:
	case TOUPCAM_EVENT_STILLIMAGE:
		toupCamera->imageGrabProcess();
		break;
	case TOUPCAM_EVENT_ERROR: {
		// SDK 回调线程禁止直接 Close（会死锁）；投递独立线程执行
		ToupCamera* cam = toupCamera;
		std::thread([cam]() { cam->Close(); }).detach();
		break;
	}
	case TOUPCAM_EVENT_DISCONNECTED:
		// 掉线仅改状态+通知，不调 Toupcam_Stop/Close（SDK 文档要求）
		toupCamera->offlineProcess();
		break;
	}
}


ToupCamera::~ToupCamera()
{
    (void)Close();
    delete[] imageBuffer_;
    imageBuffer_ = nullptr;
}

ErrorPtr ToupCamera::Open()
{
    if (m_cameraDevInfo_.connType != CameraConnType::CONN_BY_INDEX) {
        HandleError("打开图谱相机失败，仅支持按索引连接", kInvalidConfig, "unsupported conn type");
        return Err("unsupported conn type", kInvalidConfig);
    }
    return OpenByIndex(m_cameraDevInfo_.index);
}

ErrorPtr ToupCamera::Close()
{
    std::lock_guard<std::mutex> locker(mtx_);

	if (hToupcam_ == nullptr)
	{
		return Ok();
	}

	if (isStart_)
	{
		Toupcam_Stop(hToupcam_);
		isStart_ = false;
	}

	if (isGrabbing_) {
		if (auto err = stopGrabImpl(true); err) {
			return err;
		}
	}

	if (isConnected_)
	{
		Toupcam_Close(hToupcam_);
		SetConnectedState(false);
	}

	hToupcam_ = nullptr;
	SetCaptureState(CaptureState::Idle);

	return Ok();
}

bool ToupCamera::IsConnected()
{
	return isConnected_;
}

ErrorPtr ToupCamera::ReConnect()
{
	std::lock_guard<std::mutex> locker(mtx_);
	auto err = resetCamera();
	if (err) {
		return err;
	}

	hToupcam_ = Toupcam_OpenByIndex(cameraIndex_);
	if (!hToupcam_) {
		HandleError("图谱相机重连失败", kOpenFailed, "Toupcam_OpenByIndex failed");
		return Err("Toupcam_OpenByIndex failed", kOpenFailed);
	}
    SetConnectedState(true);
	isGrabbing_ = false;
	syncGrabInProgress_.store(false);
	SetCaptureState(CaptureState::Idle);
	return initCamera();
}


ErrorPtr ToupCamera::SyncGrab(int& height, int& width, unsigned char*& dataBuffer, int& dataLen)
{
	struct SyncGrabScope {
		std::atomic<bool>& flag;
		explicit SyncGrabScope(std::atomic<bool>& f) : flag(f) { flag.store(true); }
		~SyncGrabScope() { flag.store(false); }
	} syncScope(syncGrabInProgress_);

	std::unique_lock<std::mutex> lock(mtx_);
	if (!isConnected_)
	{
		HandleError("单张同步采集，相机未连接", kNotOpened);
		return Err("camera not connected", kNotOpened);
	}

	if (isGrabbing_ && asyncFlag_.load(std::memory_order_acquire)) {
		if (auto err = stopGrabImpl(true); err) {
			return err;
		}
	} else if (isGrabbing_) {
		HandleError("单张同步采集，处于采集状态中", kAlreadyStarted);
		return Err("camera is grabbing", kAlreadyStarted);
	}

	if (auto err = ensureFreeRunMode(); err) {
		return err;
	}

	isGrabbing_ = true;
	asyncFlag_.store(false, std::memory_order_release);
	SetCaptureState(CaptureState::Capturing);

	unsigned int nExpoTime = 0;
	if (FAILED(Toupcam_get_ExpoTime(hToupcam_, &nExpoTime))) {
		isGrabbing_ = false;
		SetCaptureState(CaptureState::Idle);
		HandleError("单张同步采集，获取曝光时间失败", kSdkError, "Toupcam_get_ExpoTime failed");
		return Err("Toupcam_get_ExpoTime failed", kSdkError);
	}
	const long nDelayTimeMicroSec = static_cast<long>(nExpoTime * 1.05f + (10.f) * 1000.f * 1000.f);

	if (FAILED(Toupcam_Trigger(hToupcam_, 1))) {
		isGrabbing_ = false;
		SetCaptureState(CaptureState::Idle);
		HandleError("单张同步采集，触发失败", kStartGrabbingFailed, "Toupcam_Trigger failed");
		return Err("Toupcam_Trigger failed", kStartGrabbingFailed);
	}

	// 释放 mtx_ 等待 SDK 回调 notify；asyncFlag_==false 路由到 sync 分支
	lock.unlock();
	std::unique_lock<std::mutex> lck(condmtx_);
	const auto waitStatus = cond_.wait_for(lck, std::chrono::microseconds(nDelayTimeMicroSec));
	lock.lock();

	if (!isConnected_) {
		isGrabbing_ = false;
		SetCaptureState(CaptureState::Idle);
		return Err("camera disconnected during SyncGrab", kNotOpened);
	}

	if (waitStatus == std::cv_status::timeout)
	{
		isGrabbing_ = false;
		SetCaptureState(CaptureState::Idle);
		HandleError("单张同步采集，采集超时", kFrameTimeout);
		return Err("frame timeout", kFrameTimeout);
	}

	if (imageBuffer_ == nullptr)
	{
		isGrabbing_ = false;
		SetCaptureState(CaptureState::Idle);
		HandleError("单张同步采集，buffer为空", kUnknown);
		return Err("image buffer is null", kUnknown);
	}

	dataBuffer = imageBuffer_;  // 借用内部 buffer，调用方须立即拷贝
	width = imgWidth_;
	height = imgHeight_;
	const int bitDepthByte = static_cast<int>(std::ceil(m_cameraInfo_.bitDepth / 8.f));
	dataLen = width * height * bitDepthByte;

	isGrabbing_ = false;
	SetCaptureState(CaptureState::Idle);

	return Ok();
}

ErrorPtr ToupCamera::AsyncGrab()
{
	std::lock_guard<std::mutex> locker(mtx_);
	if (!isConnected_)
	{
		HandleError("连续异步采集，未连接相机不能采集", kNotOpened);
		return Err("camera not connected", kNotOpened);
	}

	if (isGrabbing_) {
		if (auto err = stopGrabImpl(true); err) {
			return err;
		}
	}

	if (auto err = ensureFreeRunMode(); err) {
		return err;
	}

	asyncFlag_.store(true, std::memory_order_release);

	if (FAILED(Toupcam_Trigger(hToupcam_, 0xffff))) {
		HandleError("连续异步采集，触发失败", kStartGrabbingFailed, "Toupcam_Trigger failed");
		return Err("Toupcam_Trigger failed", kStartGrabbingFailed);
	}
	isGrabbing_ = true;
	SetCaptureState(CaptureState::Preview);

	return Ok();
}

ErrorPtr ToupCamera::TriggerCapture(CameraTriggerSourceType source)
{
	std::lock_guard<std::mutex> locker(mtx_);
	if (!isConnected_)
	{
		HandleError("单张异步采集，未连接相机不能采集", kNotOpened);
		return Err("camera not connected", kNotOpened);
	}

	if (isGrabbing_ && asyncFlag_.load(std::memory_order_acquire) && !m_triggerSwitch_) {
		if (auto err = stopGrabImpl(); err) {
			return err;
		}
	}

	if (isGrabbing_ && asyncFlag_.load(std::memory_order_acquire) && m_triggerSwitch_) {
		if (auto err = ensureTriggerMode(source); err) {
			return err;
		}
		if (FAILED(Toupcam_Trigger(hToupcam_, 1))) {
			HandleError("单张异步采集，触发失败", kStartGrabbingFailed, "Toupcam_Trigger failed");
			return Err("Toupcam_Trigger failed", kStartGrabbingFailed);
		}
		return Ok();
	}

	if (isGrabbing_ && !asyncFlag_.load(std::memory_order_acquire)) {
		HandleError("单张异步采集，处于同步采集状态中", kAlreadyStarted);
		return Err("sync grab in progress", kAlreadyStarted);
	}

	if (auto err = ensureTriggerMode(source); err) {
		return err;
	}

	asyncFlag_.store(true, std::memory_order_release);

	if (FAILED(Toupcam_Trigger(hToupcam_, 1))) {
		HandleError("单张异步采集，触发失败", kStartGrabbingFailed, "Toupcam_Trigger failed");
		return Err("Toupcam_Trigger failed", kStartGrabbingFailed);
	}
	isGrabbing_ = true;
	SetCaptureState(CaptureState::Capturing);

	return Ok();
}

ErrorPtr ToupCamera::StopGrab()
{
	std::lock_guard<std::mutex> locker(mtx_);
	return stopGrabImpl();
}


void ToupCamera::imageGrabProcess()
{
	// SDK 回调线程；asyncFlag_ atomic 区分 Sync/Async 路径
	if (asyncFlag_.load(std::memory_order_acquire)) {
		// 连续/触发异步：pull 到内部 buffer 再拷贝到 asyncCopyBuffer_ 后回调
		if (imageBuffer_ == nullptr)
		{
			imageBuffer_ = new unsigned char[imageBufSize_] {0x00};
		}
		int height = 0;
		int width = 0;
		pullImage(height, width, imageBuffer_);

		if (imageGrabFuncCB_ && imageBuffer_ && imageBufSize_ > 0) {
			if (asyncCopyBuffer_.size() < static_cast<size_t>(imageBufSize_)) {
				asyncCopyBuffer_.resize(static_cast<size_t>(imageBufSize_));
			}
			memcpy(asyncCopyBuffer_.data(), imageBuffer_, static_cast<size_t>(imageBufSize_));
			imageGrabFuncCB_(height, width, asyncCopyBuffer_.data(), static_cast<int>(imageBufSize_),
				m_cameraInfo_.imageFormat);
		}

	}
	else {
		// SyncGrab：pull 后 notify 主线程（condmtx_ 与 SyncGrab wait 配对）
		std::unique_lock<std::mutex> lck(condmtx_);
		if (imageBuffer_ == nullptr)
		{
			imageBuffer_ = new unsigned char[imageBufSize_] {0x00};
		}

		pullImage(imgHeight_, imgWidth_, imageBuffer_);
		cond_.notify_one();
	}
}

void ToupCamera::offlineProcess()
{
	std::lock_guard<std::mutex> locker(mtx_);
	isGrabbing_ = false;
	SetConnectedState(false);
	isStart_ = false;
	asyncFlag_.store(false, std::memory_order_release);
	syncGrabInProgress_.store(false);
	SetCaptureState(CaptureState::Idle);
	if (cameraOfflineFuncCB_) {
		cameraOfflineFuncCB_(m_cameraInfo_.cameraID + "相机掉线了!!!");
	}
}


void ToupCamera::refreshCameraInfo()
{
	if (isConnected_) {
		setCameraInfo();
	}
}

ErrorPtr ToupCamera::setCameraFeatureImpl(CameraFeatureType featureType, const CameraFeatureArg &arg)
{
	std::lock_guard<std::mutex> locker(mtx_);
	ErrorPtr triggerErr;
	if (trySetTriggerFeature(featureType, arg, triggerErr)) {
		return triggerErr;
	}

	switch (featureType) {
	case CameraFeatureType::CAMERA_FILTER_WHEEL:
		if (arg.kind == CameraFeatureArg::Kind::FilterWheel) {
			return setWheelPosition(CameraFilterWheelType2Int(arg.filterWheel));
		}
		if (arg.kind == CameraFeatureArg::Kind::Int) {
			return setWheelPosition(CameraFilterWheelType2Int(static_cast<CameraFilterWheelType>(arg.integer)));
		}
		return Err("invalid filter wheel arg", kInvalidConfig);
	case CameraFeatureType::CAMERA_ND_FILTER_WHEEL:
		if (arg.kind != CameraFeatureArg::Kind::Int) {
			return Err("invalid ND wheel arg", kInvalidConfig);
		}
		return setNDWheelPosition(arg.integer);
	case CameraFeatureType::CAMERA_EXPOSURE_TIME:
		if (arg.kind != CameraFeatureArg::Kind::Number) {
			return Err("invalid exposure arg", kInvalidConfig);
		}
		if (!isConnected_) {
			HandleError("设置曝光失败，相机未连接", kNotOpened);
			return Err("camera not connected", kNotOpened);
		}
		if (FAILED(Toupcam_put_ExpoTime(hToupcam_, static_cast<unsigned>(arg.number)))) {
			HandleError("设置曝光失败", kSdkError, "Toupcam_put_ExpoTime failed");
			return Err("Toupcam_put_ExpoTime failed", kSdkError);
		}
		return Ok();
	default:
		return Err("feature not supported by toup camera", kInvalidConfig);
	}
}

ErrorPtr ToupCamera::getCameraFeatureImpl(CameraFeatureType featureType, CameraFeatureArg &arg)
{
	std::lock_guard<std::mutex> locker(mtx_);
	if (tryGetTriggerFeature(featureType, arg)) {
		return Ok();
	}
	return Err("feature get not supported by toup camera", kInvalidConfig);
}

int ToupCamera::CameraFilterWheelType2Int(CameraFilterWheelType type) const
{
	switch (type)
	{
	case CameraFilterWheelType::FILTER_UNDEFINED:
		return 1;
	case CameraFilterWheelType::FILTER_X:
		return 2;
	case CameraFilterWheelType::FILTER_Y:
		return 3;
	case CameraFilterWheelType::FILTER_Z:
		return 4;
	default:
		return 0;
	}
}


ErrorPtr ToupCamera::initCamera()
{
	// 关闭自动曝光
	if (FAILED(Toupcam_put_AutoExpoEnable(hToupcam_, false)))
	{
		return Err("Toupcam_put_AutoExpoEnable failed", kSdkError);
	}

	// 数据格式采用RAW格式
	if (FAILED(Toupcam_put_Option(hToupcam_, TOUPCAM_OPTION_RAW, 1)))
	{
		return Err("Toupcam_put_Option RAW failed", kSdkError);
	}

	// 设置相机为最大采集分辨率
	if (FAILED(Toupcam_put_eSize(hToupcam_, 0)))
	{
		return Err("Toupcam_put_eSize failed", kSdkError);
	}

	// 相机触发模式为软件触发模式
	if (FAILED(Toupcam_put_Option(hToupcam_, TOUPCAM_OPTION_TRIGGER, 1)))
	{
		return Err("Toupcam_put_Option TRIGGER failed", kSdkError);
	}

	// 设置相机为最高位深
	if (FAILED(Toupcam_put_Option(hToupcam_, TOUPCAM_OPTION_BITDEPTH, 1)))
	{
		return Err("Toupcam_put_Option BITDEPTH failed", kSdkError);
	}

	// 设置相机增益100%
	if (FAILED(Toupcam_put_ExpoAGain(hToupcam_, 100)))
	{
		return Err("Toupcam_put_ExpoAGain failed", kSdkError);
	}

	// 相机串口UART设置
	int iValue = 1;
	if (FAILED(Toupcam_IoControl(hToupcam_, 0, TOUPCAM_IOCONTROLTYPE_SET_UART_ENABLE, iValue, &iValue)))
	{
		return Err("Toupcam_IoControl UART_ENABLE failed", kSdkError);
	}
	iValue = 4;
	if (FAILED(Toupcam_IoControl(hToupcam_, 0, TOUPCAM_IOCONTROLTYPE_SET_UART_BAUDRATE, iValue, &iValue)))
	{
		return Err("Toupcam_IoControl UART_BAUDRATE failed", kSdkError);
	}
	iValue = 0;
	if (FAILED(Toupcam_IoControl(hToupcam_, 0, TOUPCAM_IOCONTROLTYPE_SET_UART_LINEMODE, iValue, &iValue)))
	{
		return Err("Toupcam_IoControl UART_LINEMODE failed", kSdkError);
	}

	// 开启制冷装置
	if (FAILED(Toupcam_put_Option(hToupcam_, TOUPCAM_OPTION_TEC, true)))
	{
		return Err("Toupcam_put_Option TEC failed", kSdkError);
	}

	// 设置制冷目标温度，单位0.1℃.
	if (FAILED(Toupcam_put_Option(hToupcam_, TOUPCAM_OPTION_TECTARGET, 150)))
	{
		return Err("Toupcam_put_Option TECTARGET failed", kSdkError);
	}

	// 设置风扇速率
	if (FAILED(Toupcam_put_Option(hToupcam_, TOUPCAM_OPTION_FAN, 2)))
	{
		return Err("Toupcam_put_Option FAN failed", kSdkError);
	}

	// 关闭加热去雾
	if (FAILED(Toupcam_put_Option(hToupcam_, TOUPCAM_OPTION_HEAT, 0)))
	{
		return Err("Toupcam_put_Option HEAT failed", kSdkError);
	}

	if (auto err = setWheelPosition(1); err) {
		return err;
	}

	if (auto err = setNDWheelPosition(1); err) {
		return err;
	}

	return Ok();
}

ErrorPtr ToupCamera::resetCamera()
{
	if (FAILED(Toupcam_put_Option(hToupcam_, TOUPCAM_OPTION_DEVICE_RESET, 1)))
	{
		return Err("Toupcam device reset failed", kOpenFailed);
	}
	hToupcam_ = nullptr;
	return Ok();
}

void ToupCamera::setCameraInfo()
{
	// 获取相机最大采集分辨率
	int imgWidth = 0;
	int imgHeight = 0;
	Toupcam_get_FinalSize(hToupcam_, &imgWidth, &imgHeight);
	m_cameraInfo_.imageWidth = imgWidth;
	m_cameraInfo_.imageHeight = imgHeight;


	// 获取相机最大采集位深
	m_cameraInfo_.bitDepth = Toupcam_get_MaxBitDepth(hToupcam_);

	// 获取相机序列号
	char camSN[33] = { 0 };
	Toupcam_get_SerialNumber(hToupcam_, camSN);
	m_cameraInfo_.cameraID = camSN;

	// 获取RAW数据格式
	unsigned nFourCC = 0;
	unsigned bitsperpixel = 0;
	Toupcam_get_RawFormat(hToupcam_, &nFourCC, &bitsperpixel);
	m_cameraInfo_.cameraModelName = "C6100";
	m_cameraInfo_.pixelFormat = RawFormatTrans(nFourCC);
	m_cameraInfo_.imageFormat = ToImageFormat(m_cameraInfo_.pixelFormat, m_cameraInfo_.bitDepth);
	m_cameraInfo_.pixelFormatType = PixelFormatType::Filter;
	m_cameraInfo_.isSupportColor  = false;

	// 设置图像属性
	imgWidth_ = imgWidth;
	imgHeight_ = imgHeight;
	int bitDepthByte = static_cast<int>(std::ceil(m_cameraInfo_.bitDepth / 8.f));
	imageBufSize_ = imgWidth * imgHeight * bitDepthByte;
}

ErrorPtr ToupCamera::setWheelPosition(int pos)
{
	unsigned char chData[8] = { 0 };
	chData[0] = 'P';
	chData[1] = 'C';
	chData[2] = 'G';
	chData[3] = 'D';
	chData[4] = 0;      //xyz滤镜
	chData[5] = 1;      //设置
	chData[6] = (unsigned char)pos;    //指定滤镜
	chData[7] = CheckSumFun(chData, 7);

	int len = sizeof(chData);
	if (!writeUART(chData, len))
	{
		HandleError("设置滤轮失败，写 UART 失败", kSdkError);
		return Err("write UART failed", kSdkError);
	}

	memset(chData, 0, sizeof(chData));
	if (!readUART(chData, len))
	{
		HandleError("设置滤轮失败，读 UART 失败", kSdkError);
		return Err("read UART failed", kSdkError);
	}

	if (chData[7] != CheckSumFun(chData, 7) || chData[0] != 'G' || chData[1] != 'D' || chData[2] != 'P' || chData[3] != 'C')
	{
		return Err("invalid UART response", kSdkError);
	}

	if (chData[4] != 0 || chData[5] != 2)
	{
		return Err("invalid UART response payload", kSdkError);
	}
	return Ok();
}

ErrorPtr ToupCamera::setNDWheelPosition(int pos)
{
	unsigned char chData[8] = { 0 };
	chData[0] = 'P';
	chData[1] = 'C';
	chData[2] = 'G';
	chData[3] = 'D';
	chData[4] = 1;      //ND镜
	chData[5] = 1;      //设置
	chData[6] = (unsigned char)pos;    //指定滤镜
	chData[7] = CheckSumFun(chData, 7);

	int len = sizeof(chData);
	if (!writeUART(chData, len))
	{
		return Err("write UART failed", kSdkError);
	}

	memset(chData, 0, sizeof(chData));
	if (!readUART(chData, len))
	{
		return Err("read UART failed", kSdkError);
	}

	if (chData[7] != CheckSumFun(chData, 7) || chData[0] != 'G' || chData[1] != 'D' || chData[2] != 'P' || chData[3] != 'C')
	{
		return Err("invalid UART response", kSdkError);
	}

	if (chData[4] != 1 || chData[5] != 2)
	{
		return Err("invalid UART response payload", kSdkError);
	}
	return Ok();
}

void ToupCamera::pullImage(int& height, int& width, unsigned char* dataBuffer)
{
	// 拷贝图像
	ToupcamFrameInfoV3 info = { 0 };
	Toupcam_PullImageV3(hToupcam_, dataBuffer, 0, 24, 0, &info);
	width = static_cast<int>(info.width);
	height = static_cast<int>(info.height);
}

bool ToupCamera::writeUART(const unsigned char* data, int len)
{
	if (!hToupcam_)
	{
		return false;
	}

	int sendLen = 0;
	while (sendLen < len)
	{
		int tmpLen = Toupcam_write_UART(hToupcam_, data + sendLen, len - sendLen);
		if (tmpLen <= 0)
		{
			break;
		}

		sendLen += tmpLen;
	}
	if (len != sendLen)
	{
		return false;
	}

	return true;
}

bool ToupCamera::readUART(unsigned char* data, int len)
{
	if (!hToupcam_)
	{
		return false;
	}

	unsigned char chTmpData[2] = { 0 };
	int count = 0;
	int maxCount = 500; // 5s准备时间
	while (count++ < maxCount)
	{
		Sleep(10);
		memset(chTmpData, 0, sizeof(chTmpData));
		if (!hToupcam_ || 2 != Toupcam_read_UART(hToupcam_, chTmpData, 2))
		{
			return false;
		}
		if ((chTmpData[0] == 0xF0 && chTmpData[1] == 0xF0)) // F0F0表示数据发送begin
		{
			break;
		}
	}
	if (count >= maxCount)
	{
		return false;
	}

	// 图谱接口只能读取两字节,制定特殊格式
	unsigned char chData[8] = { 0 };
	chData[0] = 'P';
	chData[1] = 'C';
	chData[2] = 'G';
	chData[3] = 'D';
	chData[5] = 0xFF;
	for (int offset = 0; offset < len; offset += 2)
	{
		chData[6] = offset;
		chData[7] = CheckSumFun(chData, 7);
		if (!writeUART(chData, sizeof(chData)))
		{
			return false;
		}

		Sleep(10);
		memset(chTmpData, 0, sizeof(chTmpData));
		if (2 != Toupcam_read_UART(hToupcam_, chTmpData, 2)
			|| (chTmpData[0] == 0xFE && chTmpData[1] == 0xFE))  //FEFE表示数据发送end
		{
			return false;
		}

		*(data + offset) = chTmpData[1];
		*(data + offset + 1) = chTmpData[0];
	}

	chData[6] = 0xFF;
	chData[7] = CheckSumFun(chData, 7);
	if (!writeUART(chData, sizeof(chData)))
	{
		return false;
	}

	Sleep(10);
	memset(chTmpData, 0, sizeof(chTmpData));
	if (2 != Toupcam_read_UART(hToupcam_, chTmpData, 2)
		|| (chTmpData[0] != 0xFE && chTmpData[1] != 0xFE))  //FEFE表示数据发送end
	{
		return false;
	}
	return true;
}


ErrorPtr ToupCamera::OpenByIndex(int index)
{
	std::lock_guard<std::mutex> locker(mtx_);
	if (isConnected_)
	{
		return Ok();
	}

	hToupcam_ = Toupcam_OpenByIndex(index);
	if (!hToupcam_) {
        HandleError("打开图谱相机失败，未找到设备", kDeviceNotFound, "Toupcam_OpenByIndex failed");
		return Err("Toupcam_OpenByIndex failed", kDeviceNotFound);
	}

	auto err = initCamera();
	if (err) {
        Toupcam_Close(hToupcam_);
        hToupcam_ = nullptr;
		return err;
	}

	SetConnectedState(true);
	isGrabbing_ = false;
	SetCaptureState(CaptureState::Idle);
	cameraIndex_ = index;

	// 设置相机信息
	setCameraInfo();

	// 注册回调函数
	if (!isStart_) {
		if (FAILED(Toupcam_StartPullModeWithCallback(hToupcam_, ToupCamEventCallBackHandler, this))) {
            HandleError("启动图谱相机回调模式失败", kStartGrabbingFailed, "Toupcam_StartPullModeWithCallback failed");
            Toupcam_Close(hToupcam_);
            hToupcam_ = nullptr;
            SetCaptureState(CaptureState::Idle);
			return Err("Toupcam_StartPullModeWithCallback failed", kStartGrabbingFailed);
		}
		isStart_ = true;
	}

	return Ok();
}

ErrorPtr ToupCamera::stopGrabImpl(bool forceStop)
{
	// 同步采集中禁止外部 StopGrab；Close 用 forceStop 打断
	if (!forceStop && syncGrabInProgress_.load(std::memory_order_acquire)) {
		HandleError("停止采集失败，同步采集中", kAlreadyStarted, "sync grab in progress");
		return Err("sync grab in progress", kAlreadyStarted);
	}
	if (!isConnected_)
	{
		return Ok();
	}

	if (!isGrabbing_) {
		return Ok();
	}

	if (FAILED(Toupcam_Trigger(hToupcam_, 0))) {
		HandleError("停止采集失败，取消触发失败", kStopGrabbingFailed, "Toupcam_Trigger failed");
		return Err("Toupcam_Trigger failed", kStopGrabbingFailed);
	}

	asyncFlag_.store(false, std::memory_order_release);
	isGrabbing_ = false;
	SetCaptureState(CaptureState::Idle);

	if (forceStop && syncGrabInProgress_.load(std::memory_order_acquire)) {
		cond_.notify_all();
	}

	return Ok();
}


} // end of namespace device

}