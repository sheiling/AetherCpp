#include "lct_server_proxy.h"
#include <common/utility/string_tool/string_format/string_format.hpp>
#include <common/utility/string_tool/time_format/time_format.hpp>
#include <common/core/comm/tcp/client/tcpclient.h>
#include <codecvt>

namespace common
{
namespace device
{

LCTServerProxy::LCTServerProxy()
{
	m_threadExit.store(false);
	std::thread th = std::thread(std::bind(&LCTServerProxy::notifyThreadFun, this));
	th.detach();
}

LCTServerProxy::~LCTServerProxy()
{
	m_threadExit.store(true);
	while (m_threadExitDone.load())
		Sleep(1);

	release();
}

LCTServerProxy& LCTServerProxy::getInstance()
{
	static LCTServerProxy serverProxy;
	return serverProxy;
}
bool LCTServerProxy::init(std::string& serverAddr, int serverPort)
{
	m_isOnline.store(false);
	m_msgSocketIsConnected.store(false);
	m_fileSocketIsConnected.store(false);
	m_serverAddr = serverAddr;
	m_serverPort = serverPort;
	m_dataBuf = "";
	m_strPortAddin = "";

	m_msgSocket_ = std::make_unique<common::utility::TcpClient>();
	if (!m_msgSocket_) {
		return false;
	}
	m_fileSocket_ = std::make_unique<common::utility::TcpClient>();
	if (!m_fileSocket_) {
		return false;
	}

	common::utility::CreateCliOpts createCliOptsMsg;
	createCliOptsMsg.serverIP_ = m_serverAddr;
	createCliOptsMsg.serverPort_ = m_serverPort;
	createCliOptsMsg.reconnSwitch_ = true;
	createCliOptsMsg.reconnMinDelay_ = 3000;
	createCliOptsMsg.reconnMaxDelay_ = 9000;
	createCliOptsMsg.reconnDelayPolicy_ = 1;
	createCliOptsMsg.userdata_ = nullptr;
	createCliOptsMsg.onConnectionCB_ = std::bind(&LCTServerProxy::onMsgConnection, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
	createCliOptsMsg.onMessageCB_ = std::bind(&LCTServerProxy::onMsgReceive, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
	if (0 != m_msgSocket_->Start(createCliOptsMsg)) {
		return false;
	}

	return true;
}

bool LCTServerProxy::release()
{
	if (m_msgSocket_ != nullptr)
		m_msgSocket_->Stop();
	if (m_fileSocket_ != nullptr)
		m_fileSocket_->Stop();

	m_isOnline.store(false);
	m_msgSocketIsConnected.store(false);
	m_fileSocketIsConnected.store(false);

	if (m_notifyList.size() > 0)
	{
		int count = m_notifyList.size();
		for (int index = 0; index < count; index++)
		{
			ReplyNotify* temInfo = m_notifyList.back();
			delete temInfo;
			temInfo = nullptr;
			m_notifyList.pop_back();
		}
	}


	return true;
}

bool LCTServerProxy::lctIsConnected()
{
	return m_msgSocketIsConnected.load() && (m_msgSocket_ != nullptr);
}
bool LCTServerProxy::lctIsOnline(int timeout)
{
    return common::utility::waitForCondition(
		[this]() { return lctIsConnected() && m_isOnline.load(); }, std::chrono::milliseconds(timeout));
}

bool LCTServerProxy::sendRequestData(std::shared_ptr<PackageOutCMD> pack, std::shared_ptr<CorrectionReplyPack> replyPack, bool async)
{
	// 组合请求数据
	pack->packSendRequest();
	std::shared_ptr<RequstNotify> requstNotify = std::make_shared<RequstNotify>();
	// 设置原始包数据指针
	requstNotify->setRequstSourcePacke(pack);
	// 设置回复包数据指针
	requstNotify->setCorrectionReplyPack(replyPack);

	//添加请求数据到map  使用包序列号作为key
	m_requstMapMutex.lock();
	m_requstMap[pack->getSerialNum()] = requstNotify;
	m_requstMapMutex.unlock();

	bool ret = false;
	if (pack->getPackCMDTypes() == PackCMDTypes::Pack_Common)
	{
		ret = msgSendData(pack->getDataProto(), pack->getEndTag());
	}
	else if (pack->getPackCMDTypes() == PackCMDTypes::Pack_File)
	{
		//外部命令-发送文件  发送文件时不启动超时判断
		ret = startTansFile(pack);
	}
	else
	{
		//异常数据
		ret = startTansFile(pack);
	}

	//同步等待结果返回
	if (!async)
	{
		uint64_t beginTm = utility::TimeFormat::GetCurrentMs();
		int timeout = pack->getTimeout();

		timeout = 60 * 1000;
#ifdef LOCAL_TEST
		timeout = 60 * 1000;
#endif
		if (timeout == 0)
		{
			replyPack->resultComm = CommResults::OK;
		}
		else
		{
			do
			{
				uint64_t currentTm = utility::TimeFormat::GetCurrentMs();
				if (timeout < (currentTm - beginTm))
				{
					replyPack->resultComm = CommResults::Timeout;
					hloge("[common][device][display_control]---同步等待超时,超时时长[%d],序列号[%lld]", timeout, pack->getSerialNum());
					break;
				}
				if (requstNotify->getCommReplySta() == enum_CommReplySta::RecvData)
					break;

				Sleep(1);
			} while (true);
		}
	}


	//通信完成或者超时  删除数据
	m_requstMapMutex.lock();
	auto it = m_requstMap.find(pack->getSerialNum());
	if (it != m_requstMap.end())
	{
		m_requstMap.erase(it);
	}
	m_requstMapMutex.unlock();

	return ret;
}


void LCTServerProxy::onMsgConnection(void* userData, int connfd, uint32_t connID, bool isConnected)
{
	if (isConnected) {
		hlogi("[common][device][display_control]---连接成功ip[%s],port[%d].", m_serverAddr.c_str(), m_serverPort);
		m_msgSocketIsConnected.store(true);

		//连接成功后发包
		std::shared_ptr<PackageInnerCMD> request(new PackageInnerCMD(RequestConnection_tag, "", 0, 0, PackCMDTypes::Pack_File));
		msgSendData(request->packSendRequest(), request->getEndTag());

	}
	else {
		hlogi("[common][device][display_control]---连接断开ip[%s],port[%d].", m_serverAddr.c_str(), m_serverPort);
		m_msgSocketIsConnected.store(false);
		m_isOnline.store(false);
	}
}
void LCTServerProxy::onMsgReceive(void* userData, int connfd, uint32_t connID, const std::string& payload)
{
    std::string hexStr = common::utility::StringFormat::Byte2HexStr((unsigned char*)payload.c_str(), payload.length());
    hlogd("[common][device][display_control]---接收到数据个数[%d]-Hex:[%s]", payload.length(), hexStr.c_str());

	m_dataBuf.append(payload);
	if (m_dataBuf.length() < 8)
	{
        hlogd("[common][device][display_control]---接收到的数据长度小于8!");
		return;
	}

	while (m_dataBuf.length() != 0)
	{
		uint32_t length =
			(uint32_t)((m_dataBuf[0] & 0x00FF) << 00) | (uint32_t)((m_dataBuf[1] & 0x00FF) << 8) |
			(uint32_t)((m_dataBuf[2] & 0x00FF) << 16) | (uint32_t)((m_dataBuf[3] & 0x00FF) << 24);

		hlogd("[common][device][display_control]---实数据个数[%d]", length);

		//结束符4个字符  每个字符占两个字节
		if (m_dataBuf.length() < length + 8)
		{
			//直接返回  循环接收
			return;
		}

		//截取数据，将结束符号一并截取   处理粘包
		m_dataBuf.erase(0, 4);
        std::string subStr = common::utility::StringFormat::leftString(m_dataBuf, length + 4);
		m_dataBuf.erase(0, length + 4);

		//数据解码【结束符不需要解】
		std::u16string u16str = u"";
		int maxLoop = static_cast<int>(subStr.size()) - 4;
		for (int i = 0; i < maxLoop; i += 2)
		{
			uint16_t data = static_cast<uint16_t>((static_cast<uint16_t>(subStr[i] & 0x00FF)) + ((static_cast<uint16_t>(subStr[i + 1] & 0x00FF) << 8)));
			u16str.append(1, static_cast<char16_t>(data));
		}
        std::string strEndTag = common::utility::StringFormat::rightString(subStr, 4);
        std::string u8str = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(u16str);
        hlogd("[common][device][display_control]---接收到数据解析后:[%s]", u8str.c_str());

		if (strEndTag == EndTag_OutCMD)		//外部协议
		{
			ReplyNotify* reply = new ReplyNotify();
			reply->setReplyData(u8str);
			reply->setReplyType(PackFromTypes::Pack_FromSocket);

			m_notifyListMutex.lock();
			m_notifyList.push_back(reply);
			m_notifyListMutex.unlock();
		}
		else if (strEndTag == EndTag_InnerCMD)	//内部协议
		{
                std::vector<std::string> vec = common::utility::StringFormat::splitString(u8str, SplitTag_InnerCMD);
			if (vec.size() == 3)
			{
				if (vec.at(0) == RequestConnectionReply_tag)
				{
					m_strPortAddin = vec.at(2);
					// 请求连接包回复成功
					m_isOnline.store(true);

					//hlogi("[common][device][display_control]---请求连接包回复成功");
					//fileSocketConnect();
				}
				else if (vec.at(0) == RequestConnectionFileReply_tag)
				{
					//emit signal_RequestFileConnected
				}
				else if (vec.at(0) == FileInfoOver_tag)
				{
					//emit signal_FileInfoOver
				}
				else if (vec.at(0) == FileOver_tag)
				{
					//emit signal_FileOver
				}
				else if (vec.at(0) == RequestConnectionAllReply_tag)
				{
					//emit signal_RequestFileConnectedFull
				}
			}
			else
			{
                hloge("[common][device][display_control]---分割后字符串个数不为3");
			}
		}
		else
		{
            hloge("[common][device][display_control]---结束符错误");
		}
		break;
	}
}

void LCTServerProxy::onFileConnection(void* userData, int connfd, uint32_t connID, bool isConnected)
{
	if (isConnected) {
        hlogi("[common][device][display_control]---Connfd=%d connID=%u, file connected.", connfd, connID);
		m_fileSocketIsConnected.store(true);
	}
	else {
        hlogi("[common][device][display_control]---Connfd=%d connID=%u, file disConnected.", connfd, connID);
		m_fileSocketIsConnected.store(false);
	}
}
void LCTServerProxy::onFileReceive(void* userData, int connfd, uint32_t connID, const std::string& payload)
{
    hlogi("[common][device][display_control]---接收到数据:[%s]", payload.c_str());
}


bool LCTServerProxy::msgSendData(const std::string& data, const std::string& endTag)
{
    hlogd("[common][device][display_control]---发送数据:[%s],结束符:[%s]", data.c_str(), endTag.c_str());
	if (!lctIsConnected())
	{
        hlogd("[common][device][display_control]---Not connected");
		return false;
	}

	//数据，一个字符占两个字节
	std::string strData = "";
	for (int i = 0;i < data.length();i++)
	{
		strData.append(1, (unsigned char)(data[i] & 0x00ff));
		strData.append(1, (unsigned char)((data[i] & 0xff00) >> 8));
	}

	//有结束符则加上结束符
	std::string strEndTag = "";
	if (!endTag.empty())
	{
		strEndTag = endTag;
	}

	//前四个字节为数据大小(数据大小不包括结束符)
	std::string strSize = "";
	int length = strData.length();
	strSize.append(1, (unsigned char)(length & 0x000000ff));
	strSize.append(1, (unsigned char)((length & 0x0000ff00) >> 8));
	strSize.append(1, (unsigned char)((length & 0x00ff0000) >> 16));
	strSize.append(1, (unsigned char)((length & 0xff000000) >> 24));

	//拼接数据
	std::string sendData = strSize + strData + strEndTag;

    std::string hexStr = common::utility::StringFormat::Byte2HexStr((unsigned char*)sendData.c_str(), sendData.length());
    hlogd("[common][device][display_control]---总长度:[%d],数据长度[%d],Hex[%s]", sendData.length(), strData.length(), hexStr.c_str());

	m_msgSocket_->AsyncSend(sendData);

	return true;
}

void LCTServerProxy::notifyThreadFun()
{
	m_threadExitDone.store(false);
	do
	{
		if (m_threadExit.load())
			break;
		Sleep(1);

		ReplyNotify* replyNotify = nullptr;
		m_notifyListMutex.lock();

		if (m_notifyList.size() > 0)
		{
			replyNotify = m_notifyList.at(0);
			m_notifyList.erase(m_notifyList.begin());
		}

		m_notifyListMutex.unlock();

		if (replyNotify == nullptr)
			continue;

		std::shared_ptr<PackageBase> requestRecved = parseCompleteReply(replyNotify);
		if (requestRecved == nullptr)
		{
                    hloge("[common][device][display_control]---notifyThreadFun--Parse reply is failed.");
			continue;
		}

		//判断是否有发送记录
		bool bMatched = false;
		std::shared_ptr<RequstNotify> requestMatched = nullptr;

		m_requstMapMutex.lock();
		auto item = m_requstMap.find(requestRecved->getSerialNum());
		if (item != m_requstMap.end())
		{
			//匹配到发送记录  设置请求数据的回复包
			requestMatched = item->second;
			requestMatched->setRequstReplyPacke(requestRecved);
			bMatched = true;
		}
		m_requstMapMutex.unlock();

		std::shared_ptr<CorrectionReplyPack> replyPack = nullptr;
		//没有匹配到数据时 一般为接收LCT的发送包
		if (!bMatched)
		{
			//没有匹配到发送记录时 生成一个回复包
			replyPack = std::make_shared<CorrectionReplyPack>();
            hloge("[common][device][display_control]---Received a sent packet without a request command---replyPack.requestCMD:%d,replyPack.requestDATA:%s", replyPack->requestCMD, replyPack->sData.c_str());
		}
		else
		{
			//匹配到发送记录时 获取到原始回复包
			replyPack = requestMatched->getCorrectionReplyPack();
			//设置命令字为原始请求命令字
			requestRecved->setOutCmd(requestMatched->getRequstSourcePacke()->getOutCmd());
		}
		replyPack->sData = requestRecved->getData();
		replyPack->requestCMD = (enum_LCTCommands)requestRecved->getOutCmd();	//取出命令字
		replyPack->resultComm = requestRecved->getCommResults();				//取出通信结果

		//有回调函数时同步调用回调函数去处理
		if (readLCTSendPackageFuncCB_ != nullptr)
			readLCTSendPackageFuncCB_(replyPack);

		parseCompleteReply(requestMatched);

		delete replyNotify;
	} while (true);

	m_threadExitDone.store(true);
}

bool LCTServerProxy::fileSocketConnect()
{
    common::tcp::CreateCliOpts createCliOptsFile;
	createCliOptsFile.serverIP_ = m_serverAddr;
	createCliOptsFile.serverPort_ = m_serverPort + 1;
	createCliOptsFile.reconnSwitch_ = true;
	createCliOptsFile.reconnMinDelay_ = 3000;
	createCliOptsFile.reconnMaxDelay_ = 9000;
	createCliOptsFile.reconnDelayPolicy_ = 1;
	createCliOptsFile.userdata_ = nullptr;
	createCliOptsFile.onConnectionCB_ = std::bind(&LCTServerProxy::onFileConnection, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
	createCliOptsFile.onMessageCB_ = std::bind(&LCTServerProxy::onFileReceive, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
	if (0 != m_fileSocket_->Start(createCliOptsFile)) {
		return false;
	}

	return true;
}

void LCTServerProxy::creatPackageFromInner(int number, PackageResults packgeResults)
{
	std::string strContent = "True";
	strContent.append(SplitTag_OutCMD);
	strContent.append(std::to_string(number));
	strContent.append(SplitTag_OutCMD);
	strContent.append("0");
	strContent.append(SplitTag_OutCMD);

	ReplyNotify* ns = new ReplyNotify();
	std::string strByte(strContent);
	ns->setReplyData(strByte);
	ns->setReplyType(PackFromTypes::Pack_FromInnerErr);
	ns->setPackageResults(packgeResults);

	std::shared_lock<std::shared_mutex> lock(m_notifyListMutex);
	m_notifyList.push_back(ns);
}


//处理回包
std::shared_ptr<PackageBase> LCTServerProxy::parseCompleteReply(ReplyNotify* replyNotify)
{
	if (!replyNotify)
	{
		return nullptr;
	}
	std::string data = replyNotify->getReplyData();
	PackFromTypes type = replyNotify->getReplyType();
	std::shared_ptr<PackageBase> pack = nullptr;

	switch (type)
	{
		case PackFromTypes::Pack_FromSocket:
		{
			std::shared_ptr<PackageOutCMD> prr = std::make_shared<PackageOutCMD>();
			prr->parsePackageReply(data);
			pack = prr;
		}
		break;
		case PackFromTypes::Pack_FromInnerErr:
		{
			std::shared_ptr<PackageOutCMD> prr = std::make_shared<PackageOutCMD>();
			prr->parsePackageReply(data, replyNotify->getPackageResults());
			pack = prr;
		}
		break;
		default:
		break;
	}

	return pack;
}

//统一处理接收数据
void LCTServerProxy::parseCompleteReply(std::shared_ptr<RequstNotify> requstNotify)
{
	if (requstNotify == nullptr || requstNotify->getCorrectionReplyPack() == nullptr)
		return;
	std::shared_ptr<CorrectionReplyPack> replyPack = requstNotify->getCorrectionReplyPack();

	hlogd("[common][device][display_control]---回复包,请求命令[%d],回复数据[%s]", replyPack->requestCMD, replyPack->sData.c_str());

	switch (replyPack->requestCMD)
	{
		case enum_LCTCommands::CP_GETDISPLAYINFO:
		{
			ST_DisplayScreenInfo info(CommResults::OK);
			m_dataCodec.DeserializeDisplayScreenInfo(replyPack->sData, info);

			requstNotify->setCommReplySta(enum_CommReplySta::RecvData);
			break;
		}


		//   case  enum_LCTCommands::CP_DEVICESTATUS:
		//   {
		//       qInfo() << qGenerateErrInfo(funcName, __LINE__, "getDevStatusInfo GetCmd");
		//       QList<ST_DeviceStatusInfo> devList;
		//       m_CorrectionCodec.DeserializeDeviceStatusInfo(replyPack.sData, devList);
		//       for (int i = 0; i < devList.size(); i++)
		//       {
		//           qInfo() << qGenerateErrInfo(funcName, __LINE__, "Invoke CP_DEVICESTATUS portName:" + devList[i].portName +"status:" + QString::number(devList[i].status));
		//           qInfo() << qGenerateErrInfo(funcName, __LINE__, "Invoke CP_DEVICESTATUS m_comport:" + m_comport);
		//           if (devList[i].portName == m_comport && devList[i].status == 1)
		//           {
		//               qInfo() << qGenerateErrInfo(funcName, __LINE__, "DisConnectLCT signal portName:" + devList[i].portName + "status:" + QString::number(devList[i].status));
		//               emit signals_DisconnectedLCT();
		//           }
		//       }
		//       emit signal_getDevStatus();
		//       break;
		//   }


		   //case enum_LCTCommands::CP_LOWGRAY_GETMODE:
		   //{
		   //	QJsonDocument doc = QJsonDocument::fromJson(replyPack.sData.toLocal8Bit().data());
		   //	qDebug() << "---------------------recive get low mode:---" << replyPack.sData.toLocal8Bit().data();
		   //	if (!doc.isNull()) {
		   //		QJsonObject obj = doc.object();
		   //		int lowMode = obj.value("LowModel").toInt();
		   //		int brightnessMode = obj.value("BrightnessMode").toInt();
		   //		qDebug() << "-----------------CP_LOWGRAY_GETMODE:" << lowMode;
		   //		emit signals_getLowMode((enumLowMode)lowMode, (enumBrightnessMode)brightnessMode);
		   //	}
		   //	break;
		   //}
		case enum_LCTCommands::CP_GETDISPLAYCOUNT:		//获取显示屏个数
		{
			int num = std::stoi(replyPack->sData);
			if (replyPack->parseData != nullptr) {
				((ST_DisplayScreenList*)replyPack->parseData)->m_singleScreenCount = num;
			}
			requstNotify->setCommReplySta(enum_CommReplySta::RecvData);
			break;
		}
		case enum_LCTCommands::CP_GETDISPLAY_COM:		//获取显示端口
		{
			if (replyPack->parseData != nullptr) {
				*((std::string*)replyPack->parseData) = replyPack->sData;
			}
			requstNotify->setCommReplySta(enum_CommReplySta::RecvData);
		    break;
		}
		case enum_LCTCommands::CP_LOWGRAY_SETMODE:		//低灰参数设置
		case enum_LCTCommands::CP_LOWASHUNIFORMITY:		//设置低灰均匀性
		case enum_LCTCommands::CP_LOWCORRECTIONSTATE:	//设置低灰开关
		case enum_LCTCommands::CP_LOWCORRECTIONMODE:	//设置低灰模式
		case enum_LCTCommands::CP_SETCURRENTGAMMAVALUE:	//设置指定灰阶gamma值
		case enum_LCTCommands::CP_COEFSOLID:			//系数固化
		case enum_LCTCommands::CP_SOLIDSCANNER:			//固化接收卡参数和配置文件
		case enum_LCTCommands::CP_COEFSTATE:			//校正系数开关
		case enum_LCTCommands::CP_MODULEIDSETINGS:		//设置模组编号
		{
			if (replyPack->resultComm == CommResults::OK)
			{
				requstNotify->setCommReplySta(enum_CommReplySta::RecvData);
			}
			else
			{
			}
			break;
		}

		case enum_LCTCommands::CP_SET_MULTILAYER_CORRECTION_STATE:
		case enum_LCTCommands::CP_SETUPLOADCOEFSMODE:
		case enum_LCTCommands::CP_SET_MRVID:
		case enum_LCTCommands::CP_SETCORRECTMODE_PIXEL:
		case enum_LCTCommands::CP_SET_EFFECT_SWITCH_STATE:
		case enum_LCTCommands::CP_SOLID_MULTILAYER_CORRECTION_COEF:
		{
			int num = 0;
			if (!replyPack->sData.empty())
				num = std::stoi(replyPack->sData);

			if (replyPack->resultComm == CommResults::OK)
			{
				requstNotify->setCommReplySta(enum_CommReplySta::RecvData);
			}
			else
			{
			}
			break;
		}


		//   case enum_LCTCommands::CP_GETDISPLAYCOUNTNEW:
		//   {
		//       qDebug() << QThread::currentThreadId();
		//       QStringList  disDisplayList;
		//       m_CorrectionCodec.DeserializeDisplayCountNew(disDisplayList, replyPack.sData);
		//       emit signals_GetDisplayCountNew(disDisplayList);
		//       break;
		//   }
		//   case enum_LCTCommands::CP_GETDISPLAYINFONEW:
		//   {
		//       DisplayCombScreenInfo info;
		//       m_CorrectionCodec.DeserializeComboScreenInfo(info, replyPack.sData);
		//       if (info.m_displayScreenInfos.size() > 0)
		//       {
		//           m_comport = info.m_displayScreenInfos.first().m_DisplayScreenInfo.m_CommPort;
		//       }
		//       else
		//       {
		//           m_comport = "";
		//       }
		//       qInfo() << qGenerateErrInfo(funcName, __LINE__, "Invoke CP_GETDISPLAYINFONEW m_comport:" + m_comport);

		//       emit signals_GetDisplayInfoNew(info);
		//       break;
		//   }

		//   case  enum_LCTCommands::CP_DELAYTIME:
		//   {
		//       if (replyPack.resultComm == CommResults::ok)
		//       {
		//           emit signal_GetModeDelayTime(true, replyPack.sData.toInt());
		//       }
		//       else
		//       {
		//           emit signal_GetModeDelayTime(false, 0);
		//       }
		//       break;
		//   }
		//   case  enum_LCTCommands::CP_GET_UPLOADLAYERS:
		//   {
		//       // 双层校正：获取上传系数层数返回结果 错误：-1： 接收卡忙碌；0：获取屏体信息异常
		//       if (replyPack.resultComm == CommResults::ok)
		//       {
		//           qDebug() << "CP_GET_UPLOADLAYERS  get reply, res is succ layers = " + replyPack.sData;

		//           emit signal_GetUploadLayersResult(true, replyPack.sData.toInt());
		//       }
		//       else
		//       {
		//           qDebug() << "CP_GET_UPLOADLAYERS  get reply, res is fail err = " + QString::number((int)replyPack.resultComm);

		//           // 返回为空，异常，为保证异常导致灰阶层数改变，默认
		//           emit signal_GetUploadLayersResult(false, 4);
		//       }
		//       break;
		//   }
		//   case  enum_LCTCommands::CP_REPLAYSCRRECT_AVERCOEFS:
		//   {
		//       ST_NotifyRectAverCoefs notifyRectAverCoefs;
		//       if (replyPack.resultComm == CommResults::ok)
		//       {
		//           if (m_CorrectionCodec.DeserializeNotifyRectAverCoefs(replyPack.sData, notifyRectAverCoefs.m_AvgCoef))
		//           {
		//               notifyRectAverCoefs.m_NetFreamResults = enum_NetFreamResults::NetFreamResultsok; 
		//           }
		//           else
		//           {
		//               notifyRectAverCoefs.m_NetFreamResults = enum_NetFreamResults::NetFreamResultsrecvErrResp;
		//           }
		//       }
		//       else
		//       {
		//           notifyRectAverCoefs.m_NetFreamResults = enum_NetFreamResults::NetFreamResultsrecvErrResp;
		//       }
		//       emit signals_NotifyRectAverCoefs(notifyRectAverCoefs);
		//       break;
		//   }
		case enum_LCTCommands::CP_GET_DVICONNECTRES:
		{
#ifndef LOCAL_TEST
			if (replyPack->sData == "")
			{
				//LCT第一次回复的确认包，确认已发送
				requstNotify->setCommReplySta(enum_CommReplySta::RecvACK);
				return;
			}
#endif // !LOCAL_TEST
			//根据第二次发送的发送包反馈是否连接DVI
			if (replyPack->sData == "OK")
			{
				if (replyPack->parseData != nullptr)
					*((bool*)replyPack->parseData) = true;
				hlogd("[common][device][display_control]---is exist DVI true");
			}
			else
			{
				if (replyPack->parseData != nullptr)
					*((bool*)replyPack->parseData) = false;
				hlogd("[common][device][display_control]---is exist DVI false");
			}
			requstNotify->setCommReplySta(enum_CommReplySta::RecvData);
			break;
		}
		//   case enum_LCTCommands::CP_SET_CURRENTSCREENUSEMODE:
		//   {
		//       emit signals_SetScreenModeFinished(replyPack.resultComm);
		//       break;
		//   }
		case enum_LCTCommands::CP_GET_HWDISPLAYSCREEN:
		{
#ifndef LOCAL_TEST
			if (replyPack->sData == "")
			{
				//LCT第一次回复的确认包，确认已发送
				requstNotify->setCommReplySta(enum_CommReplySta::RecvACK);
				return;
			}
#endif // !LOCAL_TEST

			//根据第二次发送的发送包反馈是否支持硬件打屏的结果
			if (replyPack->sData == "True")
			{
				if (replyPack->parseData != nullptr)
					*((bool*)replyPack->parseData) = true;
				hlogd("[common][device][display_control]---is support hw display true");
			}
			else
			{
				if (replyPack->parseData != nullptr)
					*((bool*)replyPack->parseData) = false;
				hlogd("[common][device][display_control]---is support hw display false");
			}
			requstNotify->setCommReplySta(enum_CommReplySta::RecvData);
			break;
		}
		//case enum_LCTCommands::CP_GETSCANNERFUNCTIONINFOS:
		//{
		//	//根据第二次发送的发送包反馈是否支持硬件打屏的结果
		//	bool isSupport = false;
		//	std::map<enum_LCTSupportFeatureList, bool> featureList;
		//	m_CorrectionCodec.DeserializeFeaturesSupport(featureList, replyPack.sData);
		//	emit signals_GetFeatureList(featureList);
		//	break;
		//}

	 //   case  enum_LCTCommands::CP_MODULEFLASH_WRITECOEF_RES:
	 //   {
	 //       ST_ModuleFlashNotifyParamsEx moduleFlashNotifyParamsEx;
	 //       if (replyPack.sData != "0")//附加信息为字符0时代表指定区域写入灯板Falsh成功
	 //       {
	 //           moduleFlashNotifyParamsEx.m_isAllOK = false;
	 //           emit signals_NotifyModuleFlashRes(moduleFlashNotifyParamsEx);
	 //       }
	 //       break;
	 //   }
	 //   case enum_LCTCommands::CP_NOTIFY_MODULEFLASH_RES:
	 //   {
	 //       ST_ModuleFlashNotifyParamsEx moduleFlashNotifyParamsEx;
	 //       if (replyPack.sData == "0") //附加信息为字符0时代表指定区域写入灯板Falsh成功
	 //       {
	 //           moduleFlashNotifyParamsEx.m_isAllOK = true;
	 //           emit signals_NotifyModuleFlashRes(moduleFlashNotifyParamsEx);
	 //       }
	 //       else
	 //       {
	 //           moduleFlashNotifyParamsEx.m_isAllOK = false;
	 //           emit signals_NotifyModuleFlashRes(moduleFlashNotifyParamsEx);
	 //       }
	 //       break;
	 //   }
	 //   case  enum_LCTCommands::CP_MODULEFLASH_OPEARTION_RES_RECT:
	 //   {
	 //       ST_ModuleFlashNotifyParamsEx moduleFlashNotifyParamsEx;
	 //       if (replyPack.sData == "0")//附加信息为字符0时代表指定区域写入灯板Falsh成功
	 //       {
	 //           moduleFlashNotifyParamsEx.m_isAllOK = true;
	 //           emit signals_NotifyModuleFlashRes(moduleFlashNotifyParamsEx);
	 //       }
	 //       else
	 //       {
	 //           moduleFlashNotifyParamsEx.m_isAllOK = false;
	 //           emit signals_NotifyModuleFlashRes(moduleFlashNotifyParamsEx);
	 //       }
	 //       break;
	 //   }
	 //   case  enum_LCTCommands::CP_MODULEFLASH_OPEARTION_RES_RECT_EX:
	 //   {
	 //       ST_ModuleFlashNotifyParamsEx moduleFlashNotifyParamsEx;
	 //       m_CorrectionCodec.DeserializeModuleFlashNotifyEx(replyPack.sData, moduleFlashNotifyParamsEx);
	 //       emit signals_NotifyModuleFlashRes(moduleFlashNotifyParamsEx);
	 //       break;
	 //   }
	 //   case  enum_LCTCommands::CP_GETCONTROLLERVERSION:
	 //   {
	 //       emit signals_GetControllerVersion(replyPack.sData);
	 //       break;
	 //   }
	 //   case  enum_LCTCommands::CP_GETSCRRECT_AVERCOEFS:
	 //   {
	 //       ST_LocationInfo info;
	 //       m_CorrectionCodec.DeserialGetRectAverCoefs(replyPack.sData, info);
	 //       break;
	 //   }
	 //   case  enum_LCTCommands::CP_WEITECOEFBYPIXEL:
	 //   {
	 //       qInfo() << qGenerateErrInfo(funcName, __LINE__, "CP_WEITECOEFBYPIXEL write Coef back!");
		//	//写入区域信息 不是真正的系数上传完成 无需关注
	 //       //系数上传完成结果：INVALID_ID:
	 //       ST_WriteCoefficients info;
	 //       info.m_results = enum_NetFreamResults::NetFreamResultsok;
	 //       info.m_FilePath = "";
	 //       emit signals_WriteCoefficients(info);
	 //       break;
	 //   }
	 //   case  enum_LCTCommands::CP_COEFUPLOAD:
	 //   {
	 //       if (replyPack.resultComm == CommResults::ok)
	 //       {
	 //           qInfo() << qGenerateErrInfo(funcName, __LINE__, "write Coef successed!");
	 //           emit signals_LCTUploadCoefficientsResult(enum_NetFreamResults::NetFreamResultsok);
	 //       }
	 //       else
	 //       {
	 //           qCritical() << qGenerateErrInfo(funcName, __LINE__, "write Coef error!");
	 //           emit signals_LCTUploadCoefficientsResult(enum_NetFreamResults::NetFreamResultsrecvErrResp);
	 //       }
	 //       break;
	 //   }
	 //   

		//case  enum_LCTCommands::CP_BRIGHT_DARK_LINE_FIX_COEF_UPLOAD:
		//{
		//	// modify by zlb @20210823 for 亮暗线校正上传系数回复结果
		//	if (replyPack.resultComm == CommResults::ok)
		//	{
		//		//ok
		//		emit signals_brightDarkLineFixCoefUpload(enum_NetFreamResults::NetFreamResultsok);
		//	}
		//	else
		//	{
		//		//fail
		//		emit signals_brightDarkLineFixCoefUpload(enum_NetFreamResults::NetFreamResultsnetErr);
		//	}

		//	break;
		//}

	 //   case  enum_LCTCommands::CP_CAMERASEAMREPAIR:
	 //   {
	 //       //相机亮暗线校正上传系数回复结果
	 //       if (replyPack.resultComm == CommResults::ok)
	 //       {
	 //           //ok
	 //           emit signals_brightDarkLineFixCoefUpload(enum_NetFreamResults::NetFreamResultsok);
	 //       }
	 //       else
	 //       {
	 //           //fail
	 //           emit signals_brightDarkLineFixCoefUpload(enum_NetFreamResults::NetFreamResultsnetErr);
	 //       }

	 //       break;
	 //   }

		//case  enum_LCTCommands::CP_SETCURRENTDISPLAYINDEX:
		//{
		//	if (replyPack.resultComm == CommResults::ok)
		//	{
		//		//ok
		//		emit signals_setCurrentDisplay(enum_NetFreamResults::NetFreamResultsok);
		//	}
		//	else
		//	{
		//		//fail
		//		emit signals_setCurrentDisplay(enum_NetFreamResults::NetFreamResultsnetErr);
		//	}

		//	break;
		//}
		//case  enum_LCTCommands::CP_SETCURRENTDISPLAYNAME:
		//{
		//	if (replyPack.resultComm == CommResults::ok)
		//	{
		//		//ok
		//		emit signals_setCurrentDisplay(enum_NetFreamResults::NetFreamResultsok);
		//	}
		//	else
		//	{
		//		//fail
		//		emit signals_setCurrentDisplay(enum_NetFreamResults::NetFreamResultsnetErr);
		//	}

		//	break;
		//}
		//case  enum_LCTCommands::CP_GET_SCANBOARDINFO:
		//{
		//	if (replyPack.resultComm == CommResults::ok)
		//	{
		//		//ok
		//		emit signals_getScanBoardInfo(replyPack.sData);
		//	}
		//	else
		//	{
		//		//fail
		//		emit signals_getScanBoardInfo("");
		//	}

		//	break;
		//}
		//case  enum_LCTCommands::CP_SET_BRIGHT_DARK_LINE_FIX_STATE:
		//{
		//	if (replyPack.resultComm == CommResults::ok)
		//	{
		//		//ok
		//		emit signals_setBrightDarkLineFixState(enum_NetFreamResults::NetFreamResultsok);
		//	}
		//	else
		//	{
		//		//fail
		//		emit signals_setBrightDarkLineFixState(enum_NetFreamResults::NetFreamResultsnetErr);
		//	}
		//	break;
		//}
		//case  enum_LCTCommands::CP_SOLID_BRIGHT_DARK_LINE_FIX_COEF:
		//{
		//	if (replyPack.resultComm == CommResults::ok)
		//	{
		//		//ok
		//		emit signals_solidBrightDarkLineCoef(enum_NetFreamResults::NetFreamResultsok);
		//	}
		//	else
		//	{
		//		//fail
		//		emit signals_solidBrightDarkLineCoef(enum_NetFreamResults::NetFreamResultsnetErr);
		//	}
		//	break;
		//}
		//case  enum_LCTCommands::CP_MULTILAYER_CORRECTION_COEF_UPLOAD:
		//{
	 //       //上传完成
	 //       qInfo() << qGenerateErrInfo(funcName, __LINE__, "CP_MULTILAYER_CORRECTION_COEF_UPLOAD write Coef!");
		//	if (replyPack.resultComm == CommResults::ok)
		//	{
		//		qInfo() << qGenerateErrInfo(funcName, __LINE__, "write Coef successed!");
		//		emit signals_LCTUploadCoefficientsResult(enum_NetFreamResults::NetFreamResultsok);
		//	}
		//	else
		//	{
		//		qCritical() << qGenerateErrInfo(funcName, __LINE__, "write Coef error!");
		//		emit signals_LCTUploadCoefficientsResult(enum_NetFreamResults::NetFreamResultsrecvErrResp);
		//	}
		//	break;
		//}

	 //   case  enum_LCTCommands::CP_LOWGRAYWEITECOEF:
	 //   {
	 //       // 上传完成
	 //       qInfo() << qGenerateErrInfo(funcName, __LINE__, "CP_LOWGRAYWEITECOEF write Coef!");
	 //       if (replyPack.resultComm == CommResults::ok)
	 //       {
	 //           qInfo() << qGenerateErrInfo(funcName, __LINE__, "write Coef successed!");
	 //           emit signals_LCTUploadCoefficientsResult(enum_NetFreamResults::NetFreamResultsok);
	 //       }
	 //       else
	 //       {
	 //           qCritical() << qGenerateErrInfo(funcName, __LINE__, "write Coef error!");
	 //           emit signals_LCTUploadCoefficientsResult(enum_NetFreamResults::NetFreamResultsrecvErrResp);
	 //       }
	 //       break;
	 //   }

		case enum_LCTCommands::CP_GET_CURRENTSCREENUSEMODE:
		{
			if (replyPack->parseData != nullptr)
			{
				if (replyPack->sData == "UnKnow")
				{
					(*((ST_ScreenOperationMode*)replyPack->parseData)).screenOperationMode = enum_ScreenOperationMode::UnKnow;
				}
				else if (replyPack->sData == "OneScreen" || replyPack->sData == "1")
				{
					(*((ST_ScreenOperationMode*)replyPack->parseData)).screenOperationMode = enum_ScreenOperationMode::OneScreen;
				}
				else if (replyPack->sData == "CombScreen" || replyPack->sData == "2")
				{
					(*((ST_ScreenOperationMode*)replyPack->parseData)).screenOperationMode = enum_ScreenOperationMode::CombScreen;
				}
			}
			requstNotify->setCommReplySta(enum_CommReplySta::RecvData);
			break;
		}
		//   case  enum_LCTCommands::CP_SETDISPLAYSECTIONBYPIXEL:
		//   {
		//       if (replyPack.resultComm == CommResults::ok)
		//       {
		//           qInfo() << qGenerateErrInfo(funcName, __LINE__, QString("CP_SETDISPLAYSECTIONBYPIXEL Cmd exec result:") + "Successed!");
		//           emit signals_NotifyCustomBmpSetted(enum_NetFreamResults::NetFreamResultsok);
		//       }
		//       else
		//       {
		//           qCritical() << qGenerateErrInfo(funcName, __LINE__, QString("CP_SETDISPLAYSECTIONBYPIXEL Cmd exec result:") + "Failed!");
		//           emit signals_NotifyCustomBmpSetted(enum_NetFreamResults::NetFreamResultsnetErr);
		//       }
		//       break;
		//   }
		//   case enum_LCTCommands::CP_NOTIFYCUSTOMBITMAP_SETTED:
		//   {
		//       qInfo() << qGenerateErrInfo(funcName, __LINE__, "notify custom bitmap result:" + replyPack.sData);

		//       emit signals_NotifyCustomBmpSetted(enum_NetFreamResults(replyPack.sData.toInt()));
		//       break;
		//   }

		//   case enum_LCTCommands::CP_SET_HWDISPLAYSCREEN:
		//   {
		//       if (replyPack.resultComm == CommResults::ok)
		//       {
		//           emit signals_NotifyCustomBmpSetted(enum_NetFreamResults::NetFreamResultsok);
		//       }
		//       else
		//       {
		//           emit signals_NotifyCustomBmpSetted(enum_NetFreamResults::NetFreamResultsnetErr);
		//       }
		//       break;
		//   }
		//   case enum_LCTCommands::INVALID_ID:
		//   {
		//       //亮色度系数 真正完成上传
		//       qInfo() << qGenerateErrInfo(funcName, __LINE__, "INVALID_ID write Coef!");
		//       if (replyPack.sData.length())
		//       {
		//           //当上传校正系数完成时LCT会发送cmd为0的发送包，内容为ok代表写入成功
		//           if (replyPack.sData == "ok")
		//           {
		//               qInfo() << qGenerateErrInfo(funcName, __LINE__,"write Coef successed!");
		//               emit signals_LCTUploadCoefficientsResult(enum_NetFreamResults::NetFreamResultsok);
		//           }
		//           else
		//           {
		//               qCritical() << qGenerateErrInfo(funcName, __LINE__, "write Coef error!");
		//               emit signals_LCTUploadCoefficientsResult(enum_NetFreamResults::NetFreamResultsrecvErrResp);
		//           }
		//       }
		//       break;
		//   }
		//   case   enum_LCTCommands::CP_PASSTHROUGH:
		//   {
		//       if (replyPack.sData == "")
		//       {
		//           qCritical() << qGenerateErrInfo(funcName, __LINE__, "data is empty!");
		//           return;
		//       }
		//       qInfo() << qGenerateErrInfo(funcName, __LINE__, replyPack.sData);

		//       ST_PassthroughPackEvent pack;
		//       m_CorrectionCodec.DeserialPassthroughPack(replyPack.sData, pack);
		//       if (m_passthroughDataType == enum_PassthroughDataType::PassthroughData_Send)
		//       {
		//           emit signals_SendPassthroughData(pack);
		//       }
		//       else if (m_passthroughDataType == enum_PassthroughDataType::PassthroughData_Read)
		//       {
		//           emit signals_ReadPassthroughData(pack);
		//       }
		//       break;
		//   }
		case enum_LCTCommands::CP_GET_GAMMA_TABLE:
		{
			std::map<enum_LedColorType, std::vector<unsigned int>> gammaList;

			if (replyPack->resultComm != CommResults::OK)
			{
				replyPack->resultComm = CommResults::ParaErr;
			}
			else
			{
				if (replyPack->sData == "")
				{
					hlogd("[common][device][display_control]---get gamma table data is empty!");
					// 使用旧端口获取gamma表返回为空时 表示LCT不支持 认为正常
					// if (isNewPort) port += 1000; LCT V5.4.3之后 连接9080端口不下发gamma，校正软件通过接口获取gamma
					/*if (!m_bIsUseNewPort)
					{
						emit signals_getGammaListFinished(enum_NetFreamResults::NetFreamResultCustomNotSupportGetGamma, gammaList);
					}
					else
					{
						emit signals_getGammaListFinished(enum_NetFreamResults::NetFreamResultsparaErr, gammaList);
					}*/
				}
				else
				{
					m_dataCodec.DeserializeGammaTable(gammaList, replyPack->resultComm, replyPack->sData);
					if (replyPack->parseData != nullptr)
					{
						*((std::map<enum_LedColorType, std::vector<unsigned int>>*)replyPack->parseData) = gammaList;
					}
				}
			}
			requstNotify->setCommReplySta(enum_CommReplySta::RecvData);
			break;
		}

		//   //信号源棋盘格打屏回复
		//   case  enum_LCTCommands::CP_SETCUSTOMBITMAP:
		//   {
		//       //棋盘格打屏
		//       if (replyPack.resultComm == CommResults::ok)
		//       {
		//           qInfo() << qGenerateErrInfo(funcName, __LINE__, "checker board successed!");
		//           emit signals_checkerBoardDisplay(enum_NetFreamResults::NetFreamResultsok);
		//           emit signals_KBDisplay(enum_NetFreamResults::NetFreamResultsok);
		//       }
		//       else
		//       {
		//           qCritical() << qGenerateErrInfo(funcName, __LINE__, "checker board error!");
		//           emit signals_checkerBoardDisplay(enum_NetFreamResults::NetFreamResultsrecvErrResp);
		//           emit signals_KBDisplay(enum_NetFreamResults::NetFreamResultsrecvErrResp);
		//       }
		//       break;
		//   }


		//   // add by zlb for 棋盘格打屏回复
		//   case  enum_LCTCommands::CP_SET_CHECKERBOARD_DISPLAY:
		//   {
		//       //棋盘格打屏
		//       if (replyPack.resultComm == CommResults::ok)
		//       {
		//           qInfo() << qGenerateErrInfo(funcName, __LINE__, "checker board successed!");
		//           emit signals_checkerBoardDisplay(enum_NetFreamResults::NetFreamResultsok);
		//       }
		//       else
		//       {
		//           qCritical() << qGenerateErrInfo(funcName, __LINE__, "checker board error!");
		//           emit signals_checkerBoardDisplay(enum_NetFreamResults::NetFreamResultsrecvErrResp);
		//       }
		//       break;
		//   }
		//   // add by zlb for 是否支持棋盘格打屏
		//   case enum_LCTCommands::CP_GET_CHESSBOARDSCREEN_SUPPORT:
		//   {
		//       qInfo() << qGenerateErrInfo(funcName, __LINE__, "get get checker board is support reply:" + replyPack.sData);
		//       if (replyPack.sData == "")
		//       {
		//           //LCT第一次回复的确认包，确认已发送
		//           return;
		//       }
		//       if (replyPack.sData.size())
		//       {
		   //		if (replyPack.sData == "True")
		   //		{
		//               m_bIsSupportCheckerBoard = true;
		   //			emit signals_isSupportCheckerBoard(true);
		   //		}
		   //		else
		   //		{
		//               m_bIsSupportCheckerBoard = false;
		   //			emit signals_isSupportCheckerBoard(false);
		   //		}
		//       }
		//       break;
		//   }
		//   // add by zlb for 逐点系数上传取消 and 全灰阶系数上传取消
		//   case enum_LCTCommands::CP_CANCELCOMMONCORRECTIONCOEFUPLOAD:
		//   case enum_LCTCommands::CP_CANCELMULTILAYERBRIGHTCOEFUPLOAD:
		//   {
		//       qInfo() << qGenerateErrInfo(funcName, __LINE__, "cancel coef upload:" + replyPack.sData);
		//       if (replyPack.resultComm == CommResults::ok)
		//       {
		//           qInfo() << qGenerateErrInfo(funcName, __LINE__, "cancel coef upload successed!");
		//           emit signals_cancelUploadCoeffResult(enum_NetFreamResults::NetFreamResultsok);
		//       }
		//       else
		//       {
		//           qInfo() << qGenerateErrInfo(funcName, __LINE__, "cancel coef upload error!");
		//           emit signals_cancelUploadCoeffResult(enum_NetFreamResults::NetFreamResultsrecvErrResp);
		//       }
		//       break;
		//   }
		//   case enum_LCTCommands(2):
		//   {
		//       qInfo() << qGenerateErrInfo(funcName, __LINE__, "send mrvfile reply:" + replyPack.sData);
		//       if (replyPack.resultComm == CommResults::ok)
		//       {
		//           emit signals_GetQuickViewFileReplay(true);
		//       }
		//       else
		//       {
		//           emit signals_GetQuickViewFileReplay(false);
		//       }
		//       break;
		//   }
		//   case enum_LCTCommands::CP_SCANBDPROPRESULT:
		//   {
		//       if (replyPack.resultComm == CommResults::ok)
		//       {
		//           if (replyPack.sData.toInt() == 0)
		//           {
		//               qInfo() << qGenerateErrInfo(funcName, __LINE__, "send file successed!");
		//               emit signals_sendMrvFileResult(enum_NetFreamResults::NetFreamResultsok);
		//           }
		//           else
		//           {
		//               qCritical() << qGenerateErrInfo(funcName, __LINE__, "send file error!");
		//               emit signals_sendMrvFileResult(enum_NetFreamResults::NetFreamResultsrecvErrResp);
		//           }
		//       }
		//       else
		//       {
		//           qCritical() << qGenerateErrInfo(funcName, __LINE__, "send file error!");
		//           emit signals_sendMrvFileResult(enum_NetFreamResults::NetFreamResultsrecvErrResp);
		//       }
		//       break;
		//   }
		//   case enum_LCTCommands::CP_GETSWITCHSTATE:
		//   {
		//       
		//       if (replyPack.resultComm == CommResults::ok)
		//       {
		//           int num = replyPack.sData.toInt();
		//           std::cout << "get correctstate successed" << num;
		//           qInfo() << qGenerateErrInfo(funcName, __LINE__, "get correctstate successed!");
		//           emit signals_getCorrectState(enum_NetFreamResults::NetFreamResultsok,num);
		//       }
		//       else
		//       {
		//           qCritical() << qGenerateErrInfo(funcName, __LINE__, "get correctstate error!");
		//           emit signals_getCorrectState(enum_NetFreamResults::NetFreamResultsrecvErrResp, 0);
		//       }
		//       break;
		//   }


		case enum_LCTCommands::CP_OBTAIN_SUBPIXELSCREEN_INFORMATION:
		{
			ST_SubPixInfo subPixInfo;

			if (replyPack->resultComm != CommResults::OK || replyPack->sData == "")
			{
				replyPack->resultComm = CommResults::RecvErrResp;
			}
			else
			{
				if (!m_dataCodec.DeserializeSubPixInfo(replyPack->sData, subPixInfo))
				{
					replyPack->resultComm = CommResults::RecvErrResp;
				}
				else
				{
					if (replyPack->parseData != nullptr)
					{
						*((ST_SubPixInfo*)replyPack->parseData) = subPixInfo;
					}
				}
			}
			requstNotify->setCommReplySta(enum_CommReplySta::RecvData);
			break;
		}
		//case enum_LCTCommands::CP_SUBPIXELPOINT_BYPOINT_DISPLAY:
		//case enum_LCTCommands::CP_SUBPIXELCHECKERBOARD_SCREEN:
		//{
		//	if (replyPack.resultComm == CommResults::ok) {
		//		emit signals_NotifyCustomBmpSetted(enum_NetFreamResults::NetFreamResultsok);
		//	} else {
		//		emit signals_NotifyCustomBmpSetted(enum_NetFreamResults::NetFreamResultsnetErr);
		//	}
		//	break;
		//}


		case  enum_LCTCommands::CP_GET_ISSUPPORT_MODULE_FLASH:
		{
			int num = std::stoi(replyPack->sData);
			if (replyPack->parseData != nullptr)
			{
				*((bool*)replyPack->parseData) = (num == 0 ? false : true);
			}
			requstNotify->setCommReplySta(enum_CommReplySta::RecvData);
			break;
		}

		//   case  enum_LCTCommands::CP_COEFSOLIDTOFLASH:
		//   {
		//       int num = replyPack.sData.toInt();
		//       if (num == 0)
		//       {
		//           qInfo() << qGenerateErrInfo(funcName, __LINE__, "write moduleflash successed!");
		//           emit signal_writeModuleFlash(true);
		//       }
		//       else
		//       {
		//           qInfo() << qGenerateErrInfo(funcName, __LINE__, "write moduleflash failed!");
		//           emit signal_writeModuleFlash(false);
		//       }
		//       break;
		//   }


		//   case  enum_LCTCommands::CP_SETLOWLUMCORRECTMODE_PIXEL:
		//   {
		//       //芯片低灰系数应用
		//       if (replyPack.resultComm == CommResults::ok)
		//       {
		//           qInfo() << qGenerateErrInfo(funcName, __LINE__, "write Coef successed!");
		//           emit signals_useLCCoefficients(true);
		//       }
		//       else
		//       {
		//           qCritical() << qGenerateErrInfo(funcName, __LINE__, "write Coef error!");
		//           emit signals_useLCCoefficients(false);
		//       }
		//       break;
		//   }
		//   case  enum_LCTCommands::CP_HWDISPLAYSCREENRETURN:
		//   {
		//       qInfo() << qGenerateErrInfo(funcName, __LINE__, "displayKB! GetCmd");
		//       if (replyPack.resultComm == CommResults::ok)
		//       {
		//           qInfo() << qGenerateErrInfo(funcName, __LINE__, "hw kbDisplay successed!");
		//           emit signals_KBDisplay(0);

		//       } 
		//       else
		//       {
		//           qInfo() << qGenerateErrInfo(funcName, __LINE__, "hw kbDisplay fail!");
		//           emit signals_KBDisplay((int)replyPack.resultComm);
		//       }        
		//       break;
		//   }
		//   case  enum_LCTCommands::CP_SET_GLOBALBRIGHT:
		//   {
		//       if (replyPack.resultComm == CommResults::ok)
		//       {
		//           qInfo() << qGenerateErrInfo(funcName, __LINE__, "set screenBrightRatio successed!");
		//           emit signals_screenBrightnessRatio(enum_NetFreamResults::NetFreamResultsok);

		//       }
		//       else
		//       {
		//           qInfo() << qGenerateErrInfo(funcName, __LINE__, "set screenBrightRatio fail!");
		//           emit signals_screenBrightnessRatio(enum_NetFreamResults::NetFreamResultsnetErr);
		//       }
		//       break;
		//   }

		//   case  enum_LCTCommands::CP_ISVALID_NCP:
		//   {
		//       if (replyPack.resultComm == CommResults::ok)
		//       {
		//           int num = replyPack.sData.toInt();
		//           if (0 == num)//ncp不可用
		//           {
		//               qInfo() << qGenerateErrInfo(funcName, __LINE__, "not exist NCP!");
		//               emit signals_getNCPUsingState(enum_NetFreamResults::NetFreamResultsok,false);
		//           }
		//           else //ncp可用
		//           {
		//               qInfo() << qGenerateErrInfo(funcName, __LINE__, "exist NCP!!");
		//               emit signals_getNCPUsingState(enum_NetFreamResults::NetFreamResultsok,true);
		//           }
		//       }
		//       else
		//       {
		//           qInfo() << qGenerateErrInfo(funcName, __LINE__, "getNCPUsingState fail!");
		//           emit signals_getNCPUsingState(enum_NetFreamResults::NetFreamResultsnetErr,false);
		//       }
		//       break;
		//   }

		//   case  enum_LCTCommands::CP_CHECKNCP_IS_SAME:
		//   {
		//       if (replyPack.resultComm == CommResults::ok)
		//       {
		//           int num = replyPack.sData.toInt();
		//           if (0 == num)//ncp不一致
		//           {
		//               qInfo() << qGenerateErrInfo(funcName, __LINE__, "check NCP not same!");
		//               emit signals_getNCPCheckResult(enum_NetFreamResults::NetFreamResultsok, false);
		//           }
		//           else //ncp一致
		//           {
		//               qInfo() << qGenerateErrInfo(funcName, __LINE__, "check NCP same");
		//               emit signals_getNCPCheckResult(enum_NetFreamResults::NetFreamResultsok, true);
		//           }
		//       }
		//       else
		//       {
		//           qInfo() << qGenerateErrInfo(funcName, __LINE__, "check NCP fail!");
		//           emit signals_getNCPCheckResult(enum_NetFreamResults::NetFreamResultsnetErr, false);
		//       }
		//       break;
		//   }
		//   //不关心系数擦除结果
		//   //case  enum_LCTCommands::CP_ERASECOFE_SPIFLASH:
		//   //{
		//   //    if (replyPack.resultComm == CommResults::ok)
		//   //    {
		//   //        qInfo() << qGenerateErrInfo(funcName, __LINE__, "CleanCoeff success!");
		//   //    }
		//   //    else
		//   //    {
		//   //        qInfo() << qGenerateErrInfo(funcName, __LINE__, "CleanCoeff fail!");

		//   //    }
		//   //    break;
		//   //}

		//   //设置自适应阈值
		//   case  enum_LCTCommands::CP_SETTHRESHOLD_WHITE:
		//   {
		//       if (replyPack.resultComm == CommResults::ok)
		//       {
		//           qInfo() << qGenerateErrInfo(funcName, __LINE__, "set adapthreshold success!");
		//           emit signals_setAdaptThreshold(enum_NetFreamResults::NetFreamResultsok);
		//       }
		//       else
		//       {
		//           qInfo() << qGenerateErrInfo(funcName, __LINE__, "set adapthreshold fail!");
		//           emit signals_setAdaptThreshold(enum_NetFreamResults::NetFreamResultsrecvErrResp);

		//       }
		//       break;
		//   }

		case  enum_LCTCommands::CP_IMAGEENGINE_ISDONE:
		{
			if (replyPack->resultComm == CommResults::OK)
			{
				bool state = false;
				m_dataCodec.DeserializeImageEngineExistInfo(replyPack->sData, state);
				if (state)	//当前屏体有画质引擎
				{
					hlogd("[common][device][display_control]---imageengine exist!");
				}
				else		//当前屏体没有画质引擎
				{
					hlogd("[common][device][display_control]---imageengine not exist!");
				}
				if (replyPack->parseData != nullptr)
				{
					*((bool*)replyPack->parseData) = state;
				}
			}
			else
			{
				hlogd("[common][device][display_control]---get imageengine exist fail!");
			}

			requstNotify->setCommReplySta(enum_CommReplySta::RecvData);
			break;
		}
		case  enum_LCTCommands::CP_IMAGEENGINE_READ_SWITCH:
		{
			if (replyPack->resultComm == CommResults::OK)
			{
				bool state = false;
				m_dataCodec.DeserializeImageEngineOpenInfo(replyPack->sData, state);
				if (state)	//当前屏体开启画质引擎
				{
					hlogd("[common][device][display_control]---imageengine open!");
				}
				else		//当前屏体关闭画质引擎
				{
					hlogd("[common][device][display_control]---imageengine not open");
				}
				if (replyPack->parseData != nullptr)
				{
					*((bool*)replyPack->parseData) = state;
				}
			}
			else
			{
				hlogd("[common][device][display_control]---get imageengine open fail!");
			}

			requstNotify->setCommReplySta(enum_CommReplySta::RecvData);
			break;
		}
		case  enum_LCTCommands::CP_READ_CABINETID:
		{
			std::vector<std::string> cabIdList;
			if (replyPack->resultComm == CommResults::OK)
			{
				m_dataCodec.DeserializeCabinetId(replyPack->sData, cabIdList);
				if (replyPack->parseData != nullptr)
					*((std::vector<std::string>*)replyPack->parseData) = cabIdList;
			}
			else
			{
				hlogd("[common][device][display_control]---get cabinet id failed!");
			}

			requstNotify->setCommReplySta(enum_CommReplySta::RecvData);
			break;
		}
		case  enum_LCTCommands::CP_GETMODEL_TOPOLOGY:
		{
			if (replyPack->parseData != nullptr)
			{
				std::vector<ST_ModuleTopologyInfo>* moduleTopologyInfo = (std::vector<ST_ModuleTopologyInfo>*)replyPack->parseData;
				m_dataCodec.DeserializeModuleTopologyInfo(replyPack->sData, *moduleTopologyInfo);
			}

			requstNotify->setCommReplySta(enum_CommReplySta::RecvData);
			break;
		}
		default:
		break;
	}

}


bool LCTServerProxy::startTansFile(std::shared_ptr<PackageOutCMD> sendFilePackage)
{

	return true;
}


}

}






