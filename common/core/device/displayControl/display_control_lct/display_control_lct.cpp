#include "display_control_lct.h"
#include "package/package_out_cmd.h"
#include "proxy/lct_server_proxy.h"
#include <common/utility/ipc_tool/tcp/client/tcp_client_opts.h>
#include <common/utility/file_tool/logger/hlog.h>

namespace common
{
namespace device
{

DisplayControlLct::DisplayControlLct()
{

}

bool DisplayControlLct::init()
{
	if (!LCTServerProxy::getInstance().init(m_displayControlDevInfo_.ip, m_displayControlDevInfo_.port))
		return false;

	if (LCTServerProxy::getInstance().lctIsOnline()) {
		if (initData().resultCode_ == 0)
			return true;
	}

	return false;
}

bool DisplayControlLct::release()
{

	return true;
}

ark::utility::ResultInfo DisplayControlLct::controlDisplay(const ST_HWDisplayParameterBase& hwDisplayParameter)
{
	std::shared_ptr<CorrectionReplyPack> replyPack = std::make_shared<CorrectionReplyPack>();
	
	if (m_subPixInfo.m_isSubPix == enum_SubPixSup::enSubDevSup)
	{
		//ST_HWDisplayParameterPatternExtend* pParam = (ST_HWDisplayParameterPatternExtend*)(hwDisplayParameter);
		//if (pParam->m_PatternType == enum_DisplayPatternType::DotScreen) {

		//	//设置屏体模式为亚像素屏 目前只支持四灯亚像素 先固定数值 应该从外面传
		//	pParam->m_SubPixInfo.m_isSubPix = m_subPixInfo.m_isSubPix;
		//	pParam->m_SubPixInfo.m_subPixLTNum = m_subPixInfo.m_subPixLTNum;
		//	pParam->m_SubPixInfo.m_LPLayout = m_subPixInfo.m_LPLayout;
		//	pParam->m_SubPixInfo.m_LTLayout = m_subPixInfo.m_LTLayout;

		//	std::string sData = m_dataCodec.SerialSubPixHWDisplayParameterPattern(*pParam);
		//	hloge("[ark][device][display_control_lct] subPixHWDisplayScreen DotScreen param is %s", sData);

		//	DCU_CHECK_BOOL_RTN_INFO(sendCorrectionData(sData, replyPack, enum_LCTCommands::CP_SUBPIXELPOINT_BYPOINT_DISPLAY),
		//		replyPack->resultComm, "controlDisplay");

		//}
		//else {
		//	hloge("[ark][device][display_control_lct] subPixHWDisplayScreen DotScreen failed");
		//}
	}
	else
	{
		//ST_HWDisplayParameterPatternExtend* pParam = (ST_HWDisplayParameterPatternExtend*)(hwDisplayParameter);

		// 如果是组合屏模式的话，默认使用硬件打屏。如果是单屏的话，可选硬件/软件打屏
		if ((enum_ScreenOperationMode::OneScreen == m_curScreenMode.screenOperationMode || enum_ScreenOperationMode::UnKnow == m_curScreenMode.screenOperationMode) && (m_curScreenMode.displayMode == enum_DisplayMode::VisualInterface)) {
			if (m_currentCorrectType == enum_ScreenCorrectType::MultiLayer || m_bIsSupportHWDisplay)
			{
				//if (!m_bIsSupportCheckerBoard)
				//{
					//NQRect rect = NQRect(QPoint(displayInfo.getBegPixCol(), displayInfo.getBegPixRow()), QPoint(displayInfo.getEndPixCol(), displayInfo.getEndPixRow()));
				//	ark::utility::Rect rect;
				//	rect.x = m_DisplayInfo.getBegPixCol();
				//	rect.y = m_DisplayInfo.getBegPixRow();
				//	rect.width = m_DisplayInfo.getEndPixCol() - rect.x;
				//	rect.height = m_DisplayInfo.getEndPixRow() - rect.y;

				//	ST_HWDisplayParameterPattern* dispnew = new ST_HWDisplayParameterPattern();

				//	dispnew->m_BlueBrightValue = 0;
				//	dispnew->m_GreenBrightValue = 0;
				//	dispnew->m_RedBrightValue = 0;
				//	switch (m_DisplayInfo.m_color)
				//	{
				//	case enum_LedColorType::LedColor_Red:
				//		dispnew->m_RedBrightValue = m_DisplayInfo.m_intensity;
				//		break;
				//	case enum_LedColorType::LedColor_Green:
				//		dispnew->m_GreenBrightValue = m_DisplayInfo.m_intensity;
				//		break;
				//	case enum_LedColorType::LedColor_Blue:
				//		dispnew->m_BlueBrightValue = m_DisplayInfo.m_intensity;
				//		break;
				//	case enum_LedColorType::LedColor_White:
				//		dispnew->m_BlueBrightValue = m_DisplayInfo.m_intensity;
				//		dispnew->m_GreenBrightValue = m_DisplayInfo.m_intensity;
				//		dispnew->m_RedBrightValue = m_DisplayInfo.m_intensity;
				//		break;
				//	default:
				//		break;
				//	}
				//	dispnew->m_Rect = rect;

				//	if (m_UinitInfo.m_UintCol != 1)
				//	{
				//		dispnew->m_IntervalPixelsX = m_UinitInfo.m_UintCol;
				//		dispnew->m_IntervalPixelsY = m_UinitInfo.m_UintRow;
				//	}
				//	else
				//	{
				//		dispnew->m_IntervalPixelsX = 1;
				//		dispnew->m_IntervalPixelsY = 1;
				//	}

				//	dispnew->m_Position = ark::utility::Point(m_UinitInfo.m_CurrentCol, m_UinitInfo.m_CurrentRow);

				//	return hWDisplayScreen(dispnew, replyPack);
				//}
				//else
				//{
				//	ark::utility::Rect rect;
				//	rect.x = m_DisplayInfo.getBegPixCol();
				//	rect.y = m_DisplayInfo.getBegPixRow();
				//	rect.width = m_DisplayInfo.getEndPixCol() - rect.x;
				//	rect.height = m_DisplayInfo.getEndPixRow() - rect.y;


				//	// add by zlb for 校正提速隔块打屏
				//	//NQRect rect = NQRect(QPoint(displayInfo.getBegPixCol(), displayInfo.getBegPixRow()), QPoint(displayInfo.getEndPixCol(), displayInfo.getEndPixRow()));
				//	ST_HWDisplayParameterBlock* dispnew = new ST_HWDisplayParameterBlock();
				//	dispnew->m_BlueBrightValue = 0;
				//	dispnew->m_GreenBrightValue = 0;
				//	dispnew->m_RedBrightValue = 0;
				//	switch (m_DisplayInfo.m_color)
				//	{
				//	case enum_LedColorType::LedColor_Red:
				//		dispnew->m_RedBrightValue = m_DisplayInfo.m_intensity;
				//		break;
				//	case enum_LedColorType::LedColor_Green:
				//		dispnew->m_GreenBrightValue = m_DisplayInfo.m_intensity;
				//		break;
				//	case enum_LedColorType::LedColor_Blue:
				//		dispnew->m_BlueBrightValue = m_DisplayInfo.m_intensity;
				//		break;
				//	case enum_LedColorType::LedColor_White:
				//		dispnew->m_BlueBrightValue = m_DisplayInfo.m_intensity;
				//		dispnew->m_GreenBrightValue = m_DisplayInfo.m_intensity;
				//		dispnew->m_RedBrightValue = m_DisplayInfo.m_intensity;
				//		break;
				//	default:
				//		break;
				//	}
				//	dispnew->m_Rect = rect;

				//	dispnew->m_IntervalPixelsX = 2;
				//	dispnew->m_IntervalPixelsY = 2;

				//	dispnew->m_Position = ark::utility::Point(m_UinitInfo.m_CurrentCol, m_UinitInfo.m_CurrentRow);
				//	dispnew->m_BlackSize = ark::utility::Size(m_UinitInfo.m_UintCol, m_UinitInfo.m_UintRow);

				//	return hWDisplayScreen(dispnew, replyPack);
				//	
				//}
			}
			else
			{
				//if (m_UinitInfo.m_UintCol == 1 && m_UinitInfo.m_UintRow == 1)
				//{
				//	if (m_currentCorrectType == enum_ScreenCorrectType::LowGray)
				//	{
				//		return setDisplayCustomBitmap(m_DisplayInfo, CreateBitmap(m_DisplayInfo, m_UinitInfo), enum_BmpShowModeType::Stretch);
				//	}
				//	else
				//	{
				//		DCU_CHECK_BOOL_RTN_INFO(sendCorrectionData(m_dataCodec.SerializeDisplaySection(displayInfo), replyPack, enum_LCTCommands::CP_SETDISPLAYSECTIONBYPIXEL),
				//			replyPack->resultComm, "controlDisplay");

				//		/*return sendCorrectionData(m_CorrectionCodec.SerializeDisplaySection(displayInfo),
				//			enum_LCTCommands::CP_SETDISPLAYSECTIONBYPIXEL);*/
				//	}
				//}
				//else
				//{
				//	return setDisplayCustomBitmap(m_DisplayInfo, CreateBitmap(m_DisplayInfo, uinitInfo), enum_BmpShowModeType::Stretch);
				//}
			}
		}
		else {
			/*enum_DisplayPatternType dpt = pParam->getData().patternType;
			if (enum_DisplayPatternType::DotScreen == dpt) {
				HWDisplayParameterPattern& hwdpPattern = pParam->getData().hwdpPattern;
				ret = _servicePtr->hWDisplayScreen(&hwdpPattern, out);
			}
			else if (enum_DisplayPatternType::BlockScreen == dpt) {
				HWDisplayParameterBlock& hwdpBlock = pParam->getData().hwdpBlock;
				ret = _servicePtr->hWDisplayScreen(&hwdpBlock, out);
			}
			else if (enum_DisplayPatternType::CloseScreen == dpt) {
				HWDisplayParameterClose& hwdpClose = pParam->getData().hwdpClose;
				ret = _servicePtr->hWDisplayScreen(&hwdpClose, out);
			}*/
		}
	}


	return ark::utility::ResultInfo();
}

std::string DisplayControlLct::CreateBitmap(const ST_DisplayLoactionInfo& displayInfo, const ST_DisplayUnitInfo& uinitInfo)
{
	/*std::string saveDir = "./TempImage";
	QDir dir(saveDir);
	if (!dir.exists())
	{
		dir.mkdir(saveDir);
	}

	QString filePath = saveDir + "/dot.bmp";

	int gray = displayInfo.m_intensity;
	int width = displayInfo.getEndPixCol() - displayInfo.getBegPixCol() + 1;
	int height = displayInfo.getEndPixRow() - displayInfo.getBegPixRow() + 1;
	int exCols = width / uinitInfo.m_UintCol;
	int exRows = height / uinitInfo.m_UintRow;

	NQRgb&& rgb = ledColorTypeToRgb(displayInfo.m_color, displayInfo.m_intensity);

	if (exCols > extendCols && exRows > extendRows)
	{
		QImage image(uinitInfo.m_UintCol * extendCols, uinitInfo.m_UintRow * extendRows, QImage::Format_RGB32);
		image.fill(Qt::black);

		for (int i = 0; i < extendCols; i++)
		{
			for (int j = 0; j < extendRows; j++)
			{
				image.setPixel(uinitInfo.m_CurrentCol + uinitInfo.m_UintCol * i, uinitInfo.m_CurrentRow + uinitInfo.m_UintRow * j, qRgb(rgb.red, rgb.green, rgb.blue));
			}
		}

		image.save(filePath);
	}
	else
	{
		QImage image(uinitInfo.m_UintCol, uinitInfo.m_UintRow, QImage::Format_RGB32);
		image.fill(Qt::black);
		image.setPixel(uinitInfo.m_CurrentCol, uinitInfo.m_CurrentRow, qRgb(rgb.red, rgb.green, rgb.blue));
		image.save(filePath);
	}*/

	return "";
}

ark::utility::ResultInfo DisplayControlLct::setDisplayCustomBitmap(const ST_LocationInfo& locaitonInfo, std::string bmpFileName, enum_BmpShowModeType bmpShowMode, std::shared_ptr<CorrectionReplyPack> replyPack)
{
	/*QString funcName = __FUNCTION__;
	FuncRunningState funcState(funcName);*/

	std::string sData;
	sData = m_dataCodec.SerializeCustomBitmapAddInfo(locaitonInfo, bmpShowMode);

	DCU_CHECK_BOOL_RTN_INFO(sendMixtureData(sData, replyPack, bmpFileName, enum_LCTCommands::CP_SETCUSTOMBITMAP),
		replyPack->resultComm, "setDisplayCustomBitmap");


	return ark::utility::ResultInfo();
}
ark::utility::ResultInfo DisplayControlLct::hWDisplayScreen(ST_HWDisplayParameterBase* hwDisplayParameter, std::shared_ptr<CorrectionReplyPack> replyPack)
{
	std::string sData;
	if (hwDisplayParameter->m_PatternType == enum_DisplayPatternType::DotScreen)
	{
		ST_HWDisplayParameterBase* blockbase = static_cast<ST_HWDisplayParameterBase*>(hwDisplayParameter);
		ST_HWDisplayParameterPattern* pattern = static_cast<ST_HWDisplayParameterPattern*>(blockbase);
		sData = m_dataCodec.SerialHWDisplayParameterPattern(*pattern);


	}
	else if (hwDisplayParameter->m_PatternType == enum_DisplayPatternType::BlockScreen)
	{

		ST_HWDisplayParameterBlock* block = static_cast<ST_HWDisplayParameterBlock*>(hwDisplayParameter);
		sData = m_dataCodec.SerialHWDisplayParameterBlock(*block);
	}
	else
	{
		ST_HWDisplayParameterBase* blockbase = static_cast<ST_HWDisplayParameterBase*>(hwDisplayParameter);
		ST_HWDisplayParameterClose* block = static_cast<ST_HWDisplayParameterClose*>(blockbase);
		sData = m_dataCodec.SerialHWDisplayParameterClose(*block);
	}
	DCU_CHECK_BOOL_RTN_INFO(sendCorrectionData(sData, replyPack, enum_LCTCommands::CP_SET_HWDISPLAYSCREEN),
		replyPack->resultComm, "hWDisplayScreen");

	return ark::utility::ResultInfo();
}


ark::utility::ResultInfo DisplayControlLct::eraseCoefficients()
{


	return ark::utility::ResultInfo();
}
ark::utility::ResultInfo DisplayControlLct::correctionEffect(bool isClose)
{
	return ark::utility::ResultInfo();
}
ark::utility::ResultInfo DisplayControlLct::uploadMultiLayerCoefficients()
{


	return ark::utility::ResultInfo();
}
ark::utility::ResultInfo DisplayControlLct::writeFlash(/*enum_DataBaseCoefType coefType*/)
{
	//std::shared_ptr<CorrectionReplyPack> replyPack = std::make_shared<CorrectionReplyPack>();
	//replyPack->parseData = &m_screenInfo;

	////int coefTypeLCT = 1;
	////if (coefType == enum_DataBaseCoefType::Coef_Normal)
	////{
	////	coefTypeLCT = (int)LCTCoefType::Common;
	////}
	////else if (coefType == enum_DataBaseCoefType::Coef_Adaptive)
	////{
	////	coefTypeLCT = (int)LCTCoefType::SelfAdaption;
	////}
	////else if (coefType == enum_DataBaseCoefType::Coef_LowGray)
	////{
	////	coefTypeLCT = (int)LCTCoefType::Lowlum;
	////}

	//DCU_CHECK_BOOL_RTN_INFO(sendCorrectionData("", replyPack, enum_LCTCommands::CP_COEFSOLID),
	//	replyPack->resultComm, "writeFlash");

	return ark::utility::ResultInfo();
}
ark::utility::ResultInfo DisplayControlLct::toggleCorrectionSwitch(const ST_ToggleCorrectionSwitch& toggleCorrectionSwitch)
{
	enum_CorrectMode correctModeTemp = enum_CorrectMode::CorrectModeLum;
	long tempMode = 0;
	bool isEnableCorrect = toggleCorrectionSwitch.isEnableCorrect;

	if (isEnableCorrect)
	{
		if (toggleCorrectionSwitch.correctMode == enum_CorrectMode::CorrectModeChroma)
		{
			correctModeTemp = enum_CorrectMode::CorrectModeChroma;
		}
		else if (toggleCorrectionSwitch.correctMode == enum_CorrectMode::CorrectModeLum)
		{
			correctModeTemp = enum_CorrectMode::CorrectModeLum;
		}
		else if (toggleCorrectionSwitch.correctMode == enum_CorrectMode::CorrectMultiLayer)
		{
			tempMode |= (int)enum_MultiLayerSwitchType::Bright;
			isEnableCorrect = false;
		}
		else 
		{
			isEnableCorrect = false;
		}
	}

	std::shared_ptr<CorrectionReplyPack> replyPack = std::make_shared<CorrectionReplyPack>();
	std::string sData = m_dataCodec.SerialCorrectMode(isEnableCorrect, correctModeTemp, toggleCorrectionSwitch.isChromaCorrentionLowGray);
	DCU_CHECK_BOOL_RTN_INFO(sendCorrectionData(sData, replyPack, enum_LCTCommands::CP_SETCORRECTMODE_PIXEL),
		replyPack->resultComm, "CP_SETCORRECTMODE_PIXEL");


	if (toggleCorrectionSwitch.correctMode == enum_CorrectMode::CorrectMultiLayer || isEnableCorrect == false)
	{
		// 设置多灰阶校正效果
		replyPack = std::make_shared<CorrectionReplyPack>();
		DCU_CHECK_BOOL_RTN_INFO(sendCorrectionData(std::to_string(tempMode), replyPack, enum_LCTCommands::CP_SET_MULTILAYER_CORRECTION_STATE),
			replyPack->resultComm, "SET_MULTILAYER_CORRECTION_STATE");

	}

	return ark::utility::ResultInfo();
}
ark::utility::ResultInfo DisplayControlLct::displayInfoForIndex(int index, ST_DisplayScreenInfo& displayScreenInfo)
{
	std::shared_ptr<CorrectionReplyPack> replyPack = std::make_shared<CorrectionReplyPack>();
	replyPack->parseData = &displayScreenInfo;

	DCU_CHECK_BOOL_RTN_INFO(sendCorrectionData(std::to_string(index), replyPack, enum_LCTCommands::CP_GETDISPLAYINFO),
		replyPack->resultComm, "displayInfoForIndex");

	return ark::utility::ResultInfo();
}
ark::utility::ResultInfo DisplayControlLct::displayInfoForName(std::string screenName, ST_DisplayCombScreenInfo& displayCombScreenInfo)
{
	std::shared_ptr<CorrectionReplyPack> replyPack = std::make_shared<CorrectionReplyPack>();
	replyPack->parseData = &displayCombScreenInfo;

	DCU_CHECK_BOOL_RTN_INFO(sendCorrectionData(screenName, replyPack, enum_LCTCommands::CP_GETDISPLAYINFONEW),
		replyPack->resultComm, "displayInfoForName");

	return ark::utility::ResultInfo();
}
ark::utility::ResultInfo DisplayControlLct::getFile()
{


	return ark::utility::ResultInfo();
}
ark::utility::ResultInfo DisplayControlLct::sendFile()
{


	return ark::utility::ResultInfo();
}
ark::utility::ResultInfo DisplayControlLct::checkDVISignal(const ark::utility::Rect& rect, bool& isExistDVISignal)
{
	std::shared_ptr<CorrectionReplyPack> replyPack = std::make_shared<CorrectionReplyPack>();
	replyPack->parseData = &isExistDVISignal;

	std::string sData = "\"" + m_dataCodec.SerialRect(rect) + "\"";

	DCU_CHECK_BOOL_RTN_INFO(sendCorrectionData(sData, replyPack, enum_LCTCommands::CP_GET_DVICONNECTRES),
		replyPack->resultComm, "checkDVISignal");

	return ark::utility::ResultInfo();
}
ark::utility::ResultInfo DisplayControlLct::getGammaList(std::map<enum_LedColorType, std::vector<unsigned int>>& gammaList)
{
	std::shared_ptr<CorrectionReplyPack> replyPack = std::make_shared<CorrectionReplyPack>();
	replyPack->parseData = &gammaList;

	DCU_CHECK_BOOL_RTN_INFO(sendCorrectionData("", replyPack, enum_LCTCommands::CP_GET_GAMMA_TABLE, 500),
		replyPack->resultComm, "getGammaList");

	return ark::utility::ResultInfo();
}


ark::utility::ResultInfo DisplayControlLct::setUploadMode(enum_UploadCoefModeType mode, int timeout)
{
	std::shared_ptr<CorrectionReplyPack> replyPack = std::make_shared<CorrectionReplyPack>();

    // 注： 切换系数上传模式（稳定/快速）指令LCT没有回复，默认为成功；
	sendCorrectionData(std::to_string((int)mode), replyPack, enum_LCTCommands::CP_SETUPLOADCOEFSMODE, timeout);

	return ark::utility::ResultInfo();
}
ark::utility::ResultInfo DisplayControlLct::uploadCoefficients(const ST_UploadCoefficientsParam& uploadCoefficientsParam)
{
 	if (uploadCoefficientsParam.coefType == enum_DataBaseCoefType::Coef_Normal)				//常规系数
    {
        return uploadNormalCoef(uploadCoefficientsParam.locaitonInfo, uploadCoefficientsParam.coefArray);
    } 
    else if (uploadCoefficientsParam.coefType == enum_DataBaseCoefType::Coef_MultiGray)		//全灰阶系数
    {
        return uploadMultiLayerCoef(uploadCoefficientsParam.locaitonInfo, uploadCoefficientsParam.grayLayerList, uploadCoefficientsParam.multiLayerZipCoefs);
    }
	else if (uploadCoefficientsParam.coefType == enum_DataBaseCoefType::Coef_LowGray) 		//全屏低灰系数
    {		
		return uploadLowGrayCoef(uploadCoefficientsParam.locaitonInfo, uploadCoefficientsParam.coefArray);
	}
	else if (uploadCoefficientsParam.coefType == enum_DataBaseCoefType::Coef_LowGray_9005_9937) 		//9005/9937芯片低灰阶校正系数
    {		
		return uploadLowGrayCoef_9005_9937(uploadCoefficientsParam.locaitonInfo, uploadCoefficientsParam.coefArray);
	}
	else if (uploadCoefficientsParam.coefType == enum_DataBaseCoefType::Coef_Adaptive) 		//自适应系数
    {		
		return uploadAdaptiveCoeff(uploadCoefficientsParam.locaitonInfo, uploadCoefficientsParam.grayLayerList, uploadCoefficientsParam.adaptGrayLayerList, uploadCoefficientsParam.multiCoefArray, uploadCoefficientsParam.adaptCoefArray);
	}
  
	return ark::utility::ResultInfo();
}


ark::utility::ResultInfo DisplayControlLct::readImageEngine(bool& isExistState, bool& isOpenState)
{
	CHECK_RTN_INFO(getImageEngineExistState(isExistState));
	CHECK_RTN_INFO(getImageEngineOpenState(isOpenState));

	return ark::utility::ResultInfo();
}
ark::utility::ResultInfo DisplayControlLct::writeImageEngine(enum_ImageEngineMode mode)
{
	std::shared_ptr<CorrectionReplyPack> replyPack = std::make_shared<CorrectionReplyPack>();

	DCU_CHECK_BOOL_RTN_INFO(sendCorrectionData(std::to_string((int)mode), replyPack, enum_LCTCommands::CP_SET_EFFECT_SWITCH_STATE),
		replyPack->resultComm, "writeImageEngine");

	return ark::utility::ResultInfo();
}
ark::utility::ResultInfo DisplayControlLct::writeConfigFile()
{
	std::shared_ptr<CorrectionReplyPack> replyPack = std::make_shared<CorrectionReplyPack>();

	//0表示固化全部
	std::string sData = std::to_string(0);

	DCU_CHECK_BOOL_RTN_INFO(sendCorrectionData(sData, replyPack, enum_LCTCommands::CP_SOLIDSCANNER),
		replyPack->resultComm, "writeConfigFile");

	return ark::utility::ResultInfo();
}
ark::utility::ResultInfo DisplayControlLct::checkFlashStatus()
{


	return ark::utility::ResultInfo();
}


ark::utility::ResultInfo DisplayControlLct::supportWriteModuleID(bool& isSupportWriteModuleID)
{
	std::shared_ptr<CorrectionReplyPack> replyPack = std::make_shared<CorrectionReplyPack>();
	replyPack->parseData = &isSupportWriteModuleID;

	DCU_CHECK_BOOL_RTN_INFO(sendCorrectionData("", replyPack, enum_LCTCommands::CP_GET_ISSUPPORT_MODULE_FLASH),
		replyPack->resultComm, "supportWriteModuleID");

	return ark::utility::ResultInfo();
}
ark::utility::ResultInfo DisplayControlLct::writeModuleID(ST_LocationInfo cabLocationinfo, int moduleWidth, int moduleHeight, const std::vector<std::string>& idList) 
{
	std::shared_ptr<CorrectionReplyPack> replyPack = std::make_shared<CorrectionReplyPack>();

	std::string sData = m_dataCodec.SerializeWriteModuleId(cabLocationinfo, moduleWidth, moduleHeight, idList);
	DCU_CHECK_BOOL_RTN_INFO(sendCorrectionData(sData, replyPack, enum_LCTCommands::CP_MODULEIDSETINGS),
		replyPack->resultComm, "writeModuleID");

	return ark::utility::ResultInfo();
}
ark::utility::ResultInfo DisplayControlLct::readModuleID()
{
	return ark::utility::ResultInfo();
}
ark::utility::ResultInfo DisplayControlLct::writeCabinetID(const std::map<std::string, std::string> cabinetIDMap)
{
	std::shared_ptr<CorrectionReplyPack> replyPack = std::make_shared<CorrectionReplyPack>();
	ST_DisplayScreenInfo scInfo = m_deviceConfigInfo.getScreenInfo();
	scInfo.m_MrvInfoList = m_deviceConfigInfo.getMrvInfoList();

	std::string sData = m_dataCodec.SerializeWriteCabinetId(scInfo, cabinetIDMap);
	DCU_CHECK_BOOL_RTN_INFO(sendCorrectionData(sData, replyPack, enum_LCTCommands::CP_SET_MRVID),
		replyPack->resultComm, "writeCabinetID");


	return ark::utility::ResultInfo();
}
ark::utility::ResultInfo DisplayControlLct::readCabinetID(std::vector<std::string>& cabIdList)
{
	std::shared_ptr<CorrectionReplyPack> replyPack = std::make_shared<CorrectionReplyPack>();
	replyPack->parseData = &cabIdList;

	DCU_CHECK_BOOL_RTN_INFO(sendCorrectionData("", replyPack, enum_LCTCommands::CP_READ_CABINETID),
		replyPack->resultComm, "readCabinetID");

	return ark::utility::ResultInfo();
}
ark::utility::ResultInfo DisplayControlLct::supportHWDisplayScreen(bool& isSupportHWDisplay)
{
	std::shared_ptr<CorrectionReplyPack> replyPack = std::make_shared<CorrectionReplyPack>();
	replyPack->parseData = &isSupportHWDisplay;

	DCU_CHECK_BOOL_RTN_INFO(sendCorrectionData("", replyPack, enum_LCTCommands::CP_GET_HWDISPLAYSCREEN),
		replyPack->resultComm, "supportHWDisplayScreen");

	return ark::utility::ResultInfo();
}
ark::utility::ResultInfo DisplayControlLct::getCurrentScreenMode(ST_ScreenOperationMode& screenOperationMode)
{
	std::shared_ptr<CorrectionReplyPack> replyPack = std::make_shared<CorrectionReplyPack>();
	replyPack->parseData = &screenOperationMode;

	DCU_CHECK_BOOL_RTN_INFO(sendCorrectionData("", replyPack, enum_LCTCommands::CP_GET_CURRENTSCREENUSEMODE),
		replyPack->resultComm, "getCurrentScreenMode");

	return ark::utility::ResultInfo();
}
ark::utility::ResultInfo DisplayControlLct::getCurrentSubPixInfo(ST_SubPixInfo& subPixInfo)
{
	std::shared_ptr<CorrectionReplyPack> replyPack = std::make_shared<CorrectionReplyPack>();
	replyPack->parseData = &subPixInfo;

	DCU_CHECK_BOOL_RTN_INFO(sendCorrectionData("", replyPack, enum_LCTCommands::CP_OBTAIN_SUBPIXELSCREEN_INFORMATION),
		replyPack->resultComm, "getCurrentSubPixInfo");

	return ark::utility::ResultInfo();
}
ark::utility::ResultInfo DisplayControlLct::getScreenList(enum_ScreenOperationMode screenMode, ST_DisplayScreenList& displayScreenList)
{
	std::shared_ptr<CorrectionReplyPack> replyPack = std::make_shared<CorrectionReplyPack>();
	replyPack->parseData = &displayScreenList;

	if (screenMode == enum_ScreenOperationMode::CombScreen)
	{
		DCU_CHECK_BOOL_RTN_INFO(sendCorrectionData("", replyPack, enum_LCTCommands::CP_GETDISPLAYCOUNTNEW),
			replyPack->resultComm, "getScreenList---CombScreen");
	}
	else
	{
		DCU_CHECK_BOOL_RTN_INFO(sendCorrectionData("", replyPack, enum_LCTCommands::CP_GETDISPLAYCOUNT),
			replyPack->resultComm, "getScreenList---OneScreen");
	}

	return ark::utility::ResultInfo();
}
ark::utility::ResultInfo DisplayControlLct::getComPort(int index, std::string& comPort)
{
	std::shared_ptr<CorrectionReplyPack> replyPack = std::make_shared<CorrectionReplyPack>();
	replyPack->parseData = &comPort;

	DCU_CHECK_BOOL_RTN_INFO(sendCorrectionData(std::to_string(index), replyPack, enum_LCTCommands::CP_GETDISPLAY_COM),
		replyPack->resultComm, "getComPort");


	return ark::utility::ResultInfo();
}


ark::utility::ResultInfo DisplayControlLct::setLowAshuniformity(int rValue, int gValue, int bValue)
{
	json j;
	j["RVal"] = rValue;
	j["GVal"] = gValue;
	j["BVal"] = bValue;
	std::string sData = j.dump(4);

	std::shared_ptr<CorrectionReplyPack> replyPack = std::make_shared<CorrectionReplyPack>();

	DCU_CHECK_BOOL_RTN_INFO(sendCorrectionData(sData, replyPack, enum_LCTCommands::CP_LOWASHUNIFORMITY),
		replyPack->resultComm, "setLowAshuniformity");

	return ark::utility::ResultInfo();
}
ark::utility::ResultInfo DisplayControlLct::setLowCorrectionState(bool isOpen)
{
	std::shared_ptr<CorrectionReplyPack> replyPack = std::make_shared<CorrectionReplyPack>();

	DCU_CHECK_BOOL_RTN_INFO(sendCorrectionData(isOpen ? "true" : "false", replyPack, enum_LCTCommands::CP_LOWCORRECTIONSTATE),
		replyPack->resultComm, "setLowCorrectionState");

	return ark::utility::ResultInfo();
}
ark::utility::ResultInfo DisplayControlLct::setLowCorrectionMode(int num)
{
	std::shared_ptr<CorrectionReplyPack> replyPack = std::make_shared<CorrectionReplyPack>();

	DCU_CHECK_BOOL_RTN_INFO(sendCorrectionData(std::to_string(num), replyPack, enum_LCTCommands::CP_LOWCORRECTIONMODE),
		replyPack->resultComm, "setLowCorrectionMode");

	return ark::utility::ResultInfo();
}
ark::utility::ResultInfo DisplayControlLct::setCurrentGammaValue(int layer, int value)
{
	json j;
	j["Index"] = layer;
	j["Value"] = value;
	std::string sData = j.dump(4);

	std::shared_ptr<CorrectionReplyPack> replyPack = std::make_shared<CorrectionReplyPack>();

	DCU_CHECK_BOOL_RTN_INFO(sendCorrectionData(sData, replyPack, enum_LCTCommands::CP_SETCURRENTGAMMAVALUE),
		replyPack->resultComm, "setCurrentGammaValue");

	return ark::utility::ResultInfo();
}
ark::utility::ResultInfo DisplayControlLct::getModuleTopologyInfo(int& totalRow, int& totalCol, std::vector<ST_ModuleTopologyInfo>& moduleTopologyInfo)
{
	totalRow = 0;
	totalCol = 0;
	moduleTopologyInfo.clear();
	std::shared_ptr<CorrectionReplyPack> replyPack = std::make_shared<CorrectionReplyPack>();
	replyPack->parseData = &moduleTopologyInfo;

	DCU_CHECK_BOOL_RTN_INFO(sendCorrectionData("", replyPack, enum_LCTCommands::CP_GETMODEL_TOPOLOGY),
		replyPack->resultComm, "getModuleTopologyInfo");

	int curx = -1;
    int countCols = 0;
	for (const auto& moudleInfo : moduleTopologyInfo) 
	{
    	if (curx == -1)
    	{
    	    curx = moudleInfo.moduley;
    	    countCols++;
    	}
    	else
    	{
    	    if (curx == moudleInfo.moduley)
    	        countCols++;
		}
    }
	totalCol = countCols;
	if (countCols != 0)
    {
        totalRow = moduleTopologyInfo.size() / countCols;
    }

	return ark::utility::ResultInfo();
}
ark::utility::ResultInfo DisplayControlLct::getImageEngineExistState(bool& state)
{
	std::shared_ptr<CorrectionReplyPack> replyPack = std::make_shared<CorrectionReplyPack>();
	replyPack->parseData = &state;

	DCU_CHECK_BOOL_RTN_INFO(sendCorrectionData("", replyPack, enum_LCTCommands::CP_IMAGEENGINE_ISDONE),
		replyPack->resultComm, "getImageEngineExistState");

	return ark::utility::ResultInfo();
}
ark::utility::ResultInfo DisplayControlLct::getImageEngineOpenState(bool& state)
{
	std::shared_ptr<CorrectionReplyPack> replyPack = std::make_shared<CorrectionReplyPack>();
	replyPack->parseData = &state;

	DCU_CHECK_BOOL_RTN_INFO(sendCorrectionData("", replyPack, enum_LCTCommands::CP_IMAGEENGINE_READ_SWITCH),
		replyPack->resultComm, "getImageEngineOpenState");

	return ark::utility::ResultInfo();
}


ark::utility::ResultInfo DisplayControlLct::uploadNormalCoef(const ST_LocationInfo& locaitonInfo, const std::vector<uint16_t>& coefArray)
{
	int timeOut = getUploadlCoefTimeout(locaitonInfo);

	std::shared_ptr<CorrectionReplyPack> replyPack = std::make_shared<CorrectionReplyPack>();

	std::string filePath = saveByteCofeFile(locaitonInfo, coefArray);

	DCU_CHECK_BOOL_RTN_INFO(sendMixtureData("", replyPack, filePath, enum_LCTCommands::CP_WEITECOEFBYPIXEL, timeOut),
		replyPack->resultComm, "uploadNormalCoef");

	return ark::utility::ResultInfo();
}
ark::utility::ResultInfo DisplayControlLct::uploadMultiLayerCoef(const ST_LocationInfo& locaitonInfo, const std::vector<unsigned int>& uploadLayerList, const std::vector<unsigned char>& layerCoefs)
{
    int timeOut = getUploadlCoefTimeout(locaitonInfo);

	std::shared_ptr<CorrectionReplyPack> replyPack = std::make_shared<CorrectionReplyPack>();

	std::string filePath = saveMultiLayerCoefFile(locaitonInfo, uploadLayerList, layerCoefs);

	DCU_CHECK_BOOL_RTN_INFO(sendMixtureData("", replyPack, filePath, enum_LCTCommands::CP_MULTILAYER_CORRECTION_COEF_UPLOAD, timeOut),
		replyPack->resultComm, "uploadMultiLayerCoef");

	return ark::utility::ResultInfo();
}
ark::utility::ResultInfo DisplayControlLct::uploadLowGrayCoef(const ST_LocationInfo& locaitonInfo, const std::vector<uint16_t>& coefArray)
{
	int timeOut = getUploadlCoefTimeout(locaitonInfo);

	std::shared_ptr<CorrectionReplyPack> replyPack = std::make_shared<CorrectionReplyPack>();

	std::string filePath = saveByteCofeFile(locaitonInfo, coefArray);

	DCU_CHECK_BOOL_RTN_INFO(sendMixtureData("", replyPack, filePath, enum_LCTCommands::CP_LOWGRAYWEITECOEF, timeOut),
		replyPack->resultComm, "uploadLowGrayCoef");

	return ark::utility::ResultInfo();
}
ark::utility::ResultInfo DisplayControlLct::uploadLowGrayCoef_9005_9937(const ST_LocationInfo& locaitonInfo, const std::vector<uint16_t>& coefArray)
{
	int timeOut = getUploadlCoefTimeout(locaitonInfo);

	std::shared_ptr<CorrectionReplyPack> replyPack = std::make_shared<CorrectionReplyPack>();

	std::string filePath = saveByteCofeFile(locaitonInfo, coefArray);

	DCU_CHECK_BOOL_RTN_INFO(sendMixtureData(std::to_string((int)enum_LCTCoefType::ChipLowlum), replyPack, filePath, enum_LCTCommands::CP_COEFUPLOAD, timeOut),
		replyPack->resultComm, "uploadLowGrayCoef_9005_9937");

	return ark::utility::ResultInfo();
}
ark::utility::ResultInfo DisplayControlLct::uploadAdaptiveCoeff(const ST_LocationInfo& locaitonInfo, const std::vector<unsigned int>& grayLayerList, const std::vector<unsigned int>& adaptGrayLayerList, const std::vector<std::vector<uint16_t>>& multiCoefArray, const std::vector<std::vector<uint16_t>>& adaptCoefArray)	//上传自适应系数
{
	int timeOut = getUploadlCoefTimeout(locaitonInfo);

	std::shared_ptr<CorrectionReplyPack> replyPack = std::make_shared<CorrectionReplyPack>();

	std::string filePath = savedAdaptCoefFile(locaitonInfo, grayLayerList, adaptGrayLayerList, multiCoefArray, adaptCoefArray);

	DCU_CHECK_BOOL_RTN_INFO(sendMixtureData(std::to_string((int)enum_LCTCoefType::SelfAdaption), replyPack, filePath, enum_LCTCommands::CP_COEFUPLOAD, timeOut),
		replyPack->resultComm, "uploadAdaptiveCoeff");

	return ark::utility::ResultInfo();
}


bool DisplayControlLct::sendFile(const std::string& strFilePath, std::shared_ptr<CorrectionReplyPack> replyPack)
{
	std::shared_ptr<PackageOutCMD> readDataPack = std::make_shared<PackageOutCMD>(false,
		(uint32_t)enum_LCTUnionCommon::Union_Default, (uint32_t)enum_LCTCommands::CP_SENDFILE,
		(uint32_t)enum_LCTOperations::OPeration_Default,
		false, strFilePath, CommandTimeOut::SENDER_SAVEMORE_TIMEOUT, "", PackCMDTypes::Pack_File);

	if (LCTServerProxy::getInstance().sendRequestData(readDataPack, replyPack))
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool DisplayControlLct::sendCorrectionData(const std::string& data, std::shared_ptr<CorrectionReplyPack> replyPack, enum_LCTCommands correctionCMD, int timeout)
{

	std::shared_ptr<PackageOutCMD> readDataPack = std::make_shared<PackageOutCMD>(false, (uint32_t)enum_LCTUnionCommon::Union_Default, (uint32_t)correctionCMD,
		(uint32_t)enum_LCTOperations::OPeration_Default, false, "", timeout, data, PackCMDTypes::Pack_Common);
	replyPack->requestCMD = correctionCMD;

	if (LCTServerProxy::getInstance().sendRequestData(readDataPack, replyPack))
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool DisplayControlLct::sendMixtureData(const std::string& data, std::shared_ptr<CorrectionReplyPack> replyPack, const std::string& filePath, enum_LCTCommands correctionCMD, int timeout)
{
	if (timeout < CommandTimeOut::SENDER_UPLOADCOEFF_TIMEOUT) {
		timeout = CommandTimeOut::SENDER_UPLOADCOEFF_TIMEOUT;
	}

	std::shared_ptr<PackageOutCMD> readDataPack = std::make_shared<PackageOutCMD>(false, (uint32_t)enum_LCTUnionCommon::Union_Default, (uint32_t)correctionCMD,
		(uint32_t)enum_LCTOperations::OPeration_Default, false, filePath, timeout, data, PackCMDTypes::Pack_File);
	replyPack->requestCMD = correctionCMD;

	if (LCTServerProxy::getInstance().sendRequestData(readDataPack, replyPack))
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool DisplayControlLct::passthroughData(const std::string& data, std::shared_ptr<CorrectionReplyPack> replyPack, enum_PassthroughDataType passType)
{
	std::shared_ptr<PackageOutCMD> readDataPack = std::make_shared<PackageOutCMD>(false, (uint32_t)enum_LCTUnionCommon::Union_Default, (uint32_t)enum_LCTCommands::CP_PASSTHROUGH,
		(uint32_t)passType, false, "", CommandTimeOut::SENDER_SAVEMORE_TIMEOUT, data, PackCMDTypes::Pack_Common);

	if (LCTServerProxy::getInstance().sendRequestData(readDataPack, replyPack))
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool DisplayControlLct::sendFilePack(ST_OperFileInfo& sendFileInfo, std::shared_ptr<CorrectionReplyPack> replyPack)
{
	std::shared_ptr<PackageOutCMD> readDataPack = nullptr;

	if (sendFileInfo.m_addInfo == "ncp")
	{
		sendFileInfo.m_addInfo = "";
		readDataPack = std::make_shared<PackageOutCMD>(false, sendFileInfo.m_group, sendFileInfo.m_cmd, (uint32_t)enum_LCTCommands::CP_SEND_CHECKFIlE_NCP,
			false, sendFileInfo.m_filePath, CommandTimeOut::SENDER_SAVEMORE_TIMEOUT, sendFileInfo.m_addInfo, PackCMDTypes::Pack_File);
	}
	else
	{
		readDataPack = std::make_shared<PackageOutCMD>(false, sendFileInfo.m_group, sendFileInfo.m_cmd, (uint32_t)enum_LCTCommands::CP_SEND_CONPLETE_RCFGX,
			false, sendFileInfo.m_filePath, CommandTimeOut::SENDER_SAVEMORE_TIMEOUT, sendFileInfo.m_addInfo, PackCMDTypes::Pack_File);
	}


	if (LCTServerProxy::getInstance().sendRequestData(readDataPack, replyPack))
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool DisplayControlLct::getFilePack(const ST_OperFileInfo& sendFileInfo, std::shared_ptr<CorrectionReplyPack> replyPack)
{
	std::shared_ptr<PackageOutCMD> readDataPack = std::make_shared<PackageOutCMD>(false, sendFileInfo.m_group, sendFileInfo.m_cmd, (uint32_t)enum_LCTCommands::CP_GETFILE,
		true, "", CommandTimeOut::SENDER_SAVEMORE_TIMEOUT * 2, sendFileInfo.m_addInfo, PackCMDTypes::Pack_Common);

	// LCTServerProxy::instance()->setReceiveFilePath(sendFileInfo.m_filePath);
	if (LCTServerProxy::getInstance().sendRequestData(readDataPack, replyPack))
	{
		return true;
	}
	else
	{
		return false;
	}
}


std::string DisplayControlLct::saveByteCofeFile(const ST_LocationInfo& locaitonInfo, const std::vector<uint16_t>& coefArray, bool bBrightDarkLineFix)
{
	std::string filePath;

	return filePath;
}
std::string DisplayControlLct::saveByteCofeFile(std::string filePath, const ST_LocationInfo& locaitonInfo, const std::vector<uint16_t>& coefArray, bool bBrightDarkLineFix)
{

	return filePath;
}
std::string DisplayControlLct::saveMultiLayerCoefFile(const ST_LocationInfo& locationInfo, const std::vector<unsigned int>& uploadLayerList, const std::vector<unsigned char>& layerCoefs)
{
	std::string filePath;
	////获取生成系数存储路径
	//QString filePath = QApplication::applicationDirPath() + "/panelCoefs";
	//QDir dir(filePath);
	//if (!dir.exists())
	//{
	//	dir.mkpath(filePath);
	//}
	//deleteCoefFile(dir);
	//qsrand(QTime(0, 0, 0).secsTo(QTime::currentTime()));
	//int random = qrand() % 10;
	//filePath = filePath + "/" + QString::number(random) + ".bin";


	//QFile coefFile(filePath);
	//if (coefFile.exists(filePath))
	//{
	//	coefFile.remove(filePath);
	//}

	//if (!coefFile.open(QIODevice::WriteOnly))
	//{
	//	qCritical() << qGenerateErrInfo(funcName, __LINE__, "File creation or opening failed.");
	//	return "";
	//}
	////将上传灰阶列表转换成byte;
	//int layerSize = updateLayerList.size();
	//if (0 == layerSize)
	//{
	//	return "";
	//}

 //   int layerConstSize = updateLayerList.size();

	//std::vector<unsigned int> tempLayerList;
	//tempLayerList.resize(MaxLayerSize, 0);

	//if (layerSize < MaxLayerSize)
	//{
	//	for (int i = 0; i < MaxLayerSize; i++)
	//	{
	//		if (i < layerConstSize)
	//		{
	//			if (i < layerSize)
	//			{
	//				tempLayerList[i] = updateLayerList[i];
	//			}
	//			else
	//			{
	//				tempLayerList[i] = tempLayerList[i - 1];
	//			}				
	//		}			
	//	}
	//}

	//QByteArray info;
	//for (int i = 0; i < MaxLayerSize; i++)
	//{
	//	info.append((byte)(tempLayerList[i]));
	//	info.append((byte)(tempLayerList[i] >> 8));
	//	info.append((byte)(tempLayerList[i] >> 16));
	//	info.append((byte)(tempLayerList[i] >> 24));
	//}
	////计算出灰度间隔
	//for (int index = 0; index < MaxLayerSize; index++ )
	//{
	//	uint8_t inVldBitLayer = 0;
	//	if (index != 0)
	//	{
	//		if (index < layerSize)
	//		{
	//			inVldBitLayer = log2((float)(updateLayerList[index] - updateLayerList[index - 1]));
	//		}
	//		else
	//		{
	//			inVldBitLayer = 0;
	//		}
	//	}
	//	info.append((byte)inVldBitLayer);
	//}

	////将分区信息转化成uchar数组
	//info.append((byte)(locationInfo.getBegPixCol()));
	//info.append((byte)(locationInfo.getBegPixCol() >> 8));
	//info.append((byte)(locationInfo.getBegPixCol() >> 16));
	//info.append((byte)(locationInfo.getBegPixCol() >> 24));

	//info.append((byte)(locationInfo.getBegPixRow()));
	//info.append((byte)(locationInfo.getBegPixRow() >> 8));
	//info.append((byte)(locationInfo.getBegPixRow() >> 16));
	//info.append((byte)(locationInfo.getBegPixRow() >> 24));

	//info.append((byte)(locationInfo.getEndPixCol()));
	//info.append((byte)(locationInfo.getEndPixCol() >> 8));
	//info.append((byte)(locationInfo.getEndPixCol() >> 16));
	//info.append((byte)(locationInfo.getEndPixCol() >> 24));

	//info.append((byte)(locationInfo.getEndPixRow()));
	//info.append((byte)(locationInfo.getEndPixRow() >> 8));
	//info.append((byte)(locationInfo.getEndPixRow() >> 16));
	//info.append((byte)(locationInfo.getEndPixRow() >> 24));
	//info.append((char*)(layerCoefs.data()), layerCoefs.size());
	//coefFile.write(info);
	//coefFile.close();
	return filePath;
}
std::string DisplayControlLct::savedAdaptCoefFile(const ST_LocationInfo& locationInfo, const std::vector<unsigned int>& updateLayerList, const std::vector<unsigned int>& updateWhiteLayerList, const std::vector<std::vector<uint16_t>>& multiLayerCoefs, const std::vector<std::vector<uint16_t>>& layerCoefs)
{
	std::string filePath;

	return filePath;
}


ark::utility::ResultInfo DisplayControlLct::initData()
{
	hlogd("[ark][device][display_control_lct]======================================================================================================");
	hlogd("[ark][device][display_control_lct]==============================             LCT数据初始化开始           ================================");
	hlogd("[ark][device][display_control_lct]======================================================================================================");


	//是否支持硬件打屏
	//////////////////////////////////////////////////////////////////////////
	hlogd("[ark][device][display_control_lct]============================== 是否支持硬件打屏命令执行 start =========================================");

	//CHECK_RTN_INFO(supportHWDisplayScreen(m_bIsSupportHWDisplay));

	hlogd("[ark][device][display_control_lct]============================== 是否支持硬件打屏命令执行 end ===========================================");

	//获取当前屏体模式
	//////////////////////////////////////////////////////////////////////////
	hlogd("[ark][device][display_control_lct]============================== 获取当前屏体模式 start =================================================");

	CHECK_RTN_INFO(getCurrentScreenMode(m_curScreenMode));

	// 获取屏体模式异常处理
	if (m_curScreenMode.screenOperationMode == enum_ScreenOperationMode::UnKnow)
	{
		m_curScreenMode.screenOperationMode = enum_ScreenOperationMode::OneScreen;
	}
	hlogd("[ark][device][display_control_lct]============================== 获取当前屏体模式 end =================================================");

	//获取亚像素屏体信息
	//////////////////////////////////////////////////////////////////////////
	hlogd("[ark][device][display_control_lct]============================== 获取亚像素屏体信息 start =============================================");
#ifndef LOCAL_TEST
	//CHECK_RTN_INFO(getCurrentSubPixInfo(m_subPixInfo));

	// 亚像素屏信息
	if (m_subPixInfo.m_isSubPix == enum_SubPixSup::enSubDevSup) //亚像素屏且硬件支持
	{
		hlogi("[ark][device][display_control_lct]--当前屏体为亚像素屏 灯点模式：%d", (int)m_subPixInfo.m_LPLayout);
	}
	else if (m_subPixInfo.m_isSubPix == enum_SubPixSup::enSubDevNotSup) //亚像素屏但硬件不支持
	{
		hlogi("[ark][device][display_control_lct]--当前屏体为亚像素屏但设备存在问题");
	}
	else//非亚像素屏
	{
		hlogi("[ark][device][display_control_lct]--当前屏体为常规屏");
	}
	hlogd("[ark][device][display_control_lct]============================== 获取亚像素屏体信息 start =============================================");
#endif

	//获取屏体个数
	//////////////////////////////////////////////////////////////////////////
	hlogd("[ark][device][display_control_lct]============================== 获取屏体个数 start ==================================================");
	CHECK_RTN_INFO(getScreenList(m_curScreenMode.screenOperationMode, m_curScreenList));

	hlogd("[ark][device][display_control_lct]============================== 获取屏体个数 end ====================================================");


	//获取屏体信息(循环获取所有屏体)
	//////////////////////////////////////////////////////////////////////////
	hlogd("[ark][device][display_control_lct]============================== 获取屏体信息(循环获取所有屏体) start =================================");
	m_vecScreenInfo.clear();
	m_vecCombScreenInfo.clear();
	if (m_curScreenMode.screenOperationMode == enum_ScreenOperationMode::OneScreen)
	{
		hlogd("[ark][device][display_control_lct]--获取屏体信息OneScreen Count:%d", m_curScreenList.m_singleScreenCount);
		for (int i = 0;i < m_curScreenList.m_singleScreenCount;i++)
		{
			// 获取端口号 拿到的是当前操作通信口
			std::string comPort;
			CHECK_RTN_INFO(getComPort(i, comPort));
			m_deviceConfigInfo.setComPort(comPort);

			// 获取单屏信息
			ST_DisplayScreenInfo displayScreenInfo;
			CHECK_RTN_INFO(displayInfoForIndex(i, displayScreenInfo));

			// 保存第一个屏体信息到配置信息里 箱体校正下使用 通常箱体下只会有一个屏
			if (i == 0) 
			{
				//一箱多卡场景下需要映射接收卡关系，实际读回来的mrvID可能是空，给个有意义的值
				for (size_t i = 0; i < displayScreenInfo.m_MrvInfoList.size(); i++)
				{
					hlogd("[ark][device][display_control_lct]--mrvid is :%s", displayScreenInfo.m_MrvInfoList[i].m_MrvID.c_str());
					if (displayScreenInfo.m_MrvInfoList[i].m_MrvID.empty())
					{
						displayScreenInfo.m_MrvInfoList[i].m_MrvID = std::to_string(i);
						hlogd("[ark][device][display_control_lct]--set mrvid :%s", displayScreenInfo.m_MrvInfoList[i].m_MrvID.c_str());
					}
				}
				// 保存当前屏体参数
				m_deviceConfigInfo.setScreenInfo(displayScreenInfo);
				m_deviceConfigInfo.setMrvInfo(displayScreenInfo.m_MrvInfoList);
				m_deviceConfigInfo.setComPort(comPort);
			}
			m_vecScreenInfo.push_back(displayScreenInfo);

			hlogd("[ark][device][display_control_lct]============================= 屏体索引: %d 屏体名称：%s =============================",
				displayScreenInfo.m_Index, displayScreenInfo.m_ScreenName.c_str());
		}
	}
	else if (m_curScreenMode.screenOperationMode == enum_ScreenOperationMode::CombScreen)
	{
		hlogd("[ark][device][display_control_lct]--获取屏体信息CombScreen Count:%d", m_curScreenList.m_combScreenList.size());
		for (size_t i = 0; i < m_curScreenList.m_combScreenList.size(); ++i)
		{
			// 获取组合屏信息
			ST_DisplayCombScreenInfo displayCombScreenInfo;
			CHECK_RTN_INFO(displayInfoForName(m_curScreenList.m_combScreenList.at(i), displayCombScreenInfo));

			m_vecCombScreenInfo.push_back(displayCombScreenInfo);
		}
	}
	else
	{
		hloge("[ark][device][display_control_lct]============================== 获取屏体当前模式未知 UnKnow  ==================================");
	}

	hlogd("[ark][device][display_control_lct]============================== 获取屏体信息 end ==================================");


	hlogd("[ark][device][display_control_lct]======================================================================================================");
	hlogd("[ark][device][display_control_lct]==============================             LCT数据初始化结束           ================================");
	hlogd("[ark][device][display_control_lct]======================================================================================================");

	return ark::utility::ResultInfo();
}

int DisplayControlLct::getUploadlCoefTimeout(const ST_LocationInfo& locaitonInfo)
{
	// 自定义命令执行超时时间 
    unsigned int cols = locaitonInfo.getEndPixCol() - locaitonInfo.getBegPixCol() + 1;
    unsigned int rows = locaitonInfo.getEndPixRow() - locaitonInfo.getBegPixRow() + 1;
    int baseTime = 84;
    int timeOut = CommandTimeOut::SENDER_UPLOADCOEFF_TIMEOUT + (9 * cols * rows) / (128 * 128 * 9) * baseTime;

	return timeOut;
}

bool DisplayControlLct::makeError(long rtn, const std::string& strFun, std::string& rtnStr)
{
	if (rtn == (int)CommResults::OK)
		return true;

	rtnStr = "unknown error";

	hloge("[ark][device][display_control_lct]--%s failed, ret=%d, errInfo=%s", strFun.c_str(), rtn, rtnStr.c_str());

	return false;
}

}


}
