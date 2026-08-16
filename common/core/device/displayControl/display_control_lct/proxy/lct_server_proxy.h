#ifndef _LCT_SERVER_PROXY_HH
#define _LCT_SERVER_PROXY_HH

#include <shared_mutex>
#include <atomic>
#include <vector>
#include <common/core/comm/tcp/client/tcpclient.h>
#include "../package/package_base.h"
#include "../package/package_out_cmd.h"
#include "../package/package_inner_cmd.h"
#include "../package/reply_notify.h"
#include "../package/requst_notify.h"
#include "lct_data_codec.h"

namespace common
{
namespace device
{
class LCTServerProxy
{
protected:
	LCTServerProxy();
	~LCTServerProxy();
public:
	static LCTServerProxy& getInstance();
	bool init(std::string& serverAddr, int serverPort);
	bool release();

	bool lctIsConnected();						//socket是否连接成功
	bool lctIsOnline(int timeout = 5000);		//请求连接时的回复包是否收到

	bool sendRequestData(std::shared_ptr<PackageOutCMD> pack, std::shared_ptr<CorrectionReplyPack> replyPack, bool async = false); 	//外部包的发送

	void registerReadLCTSendPackageCB(ReadLCTSendPackageFuncCB readLCTSendPackageFuncCB) { readLCTSendPackageFuncCB_ = readLCTSendPackageFuncCB; }
private:
	void onMsgConnection(void* userData, int connfd, uint32_t connID, bool isConnected);
	void onMsgReceive(void* userData, int connfd, uint32_t connID, const std::string& payload);

	void onFileConnection(void* userData, int connfd, uint32_t connID, bool isConnected);
	void onFileReceive(void* userData, int connfd, uint32_t connID, const std::string& payload);

	bool msgSendData(const std::string& data, const std::string& endTag);

private:
	bool fileSocketConnect();													//文件传输socket连接，当端口连上且请求连接包回复OK时调用
	void creatPackageFromInner(int number, PackageResults packgeResults);       //创建内部业务逻辑(非从socket传回)的场景的外部回包,如:打开文件失败、socket未连接等

	void notifyThreadFun();														//上行---消息处理线程

	std::shared_ptr<PackageBase> parseCompleteReply(ReplyNotify* replyNotify);	//上行---处理回包, 封装请求数据
	void parseCompleteReply(std::shared_ptr<RequstNotify> requstNotify);		//上行---处理回包, 处理数据

	bool startTansFile(std::shared_ptr<PackageOutCMD> sendFilePackage);			//发送文件
private:
	std::atomic_bool					m_threadExit = false;					//线程退出标志
	std::atomic_bool					m_threadExitDone = false;				//线程退出完成标志
	std::atomic_bool					m_isOnline = false;						//端口连上且请求连接包回复OK时为true
	std::atomic_bool 				 	m_msgSocketIsConnected = false;			//消息发送socket的连接状态
	std::atomic_bool 				 	m_fileSocketIsConnected = false;		//文件传输socket的连接状态

	std::shared_mutex                	m_requstMapMutex;
	std::map<uint64_t, std::shared_ptr<RequstNotify>>	m_requstMap;			//发送队列（匹配上下行数据的唯一性）

	std::shared_mutex 					m_notifyListMutex;
	std::vector<ReplyNotify*>         	m_notifyList;                        	//上行---接收队列

	std::string							m_recvFileName;							//接收文件的路径
	std::string							m_dataBuf;								//接收数据缓存buffer
	std::string							m_strPortAddin;							//端口附加信息(在连接回包中携带)
	std::string							m_serverAddr;							//端口地址
	int									m_serverPort;							//端口号
	std::unique_ptr<common::tcp::TcpClient> m_msgSocket_ = nullptr;          	//消息发送对象   8080端口
	std::unique_ptr<common::tcp::TcpClient> m_fileSocket_ = nullptr;         	//文件发送对象   8081端口
						
	LCTDataCodec						m_dataCodec;							//数据编解码

private:
	ReadLCTSendPackageFuncCB readLCTSendPackageFuncCB_ = nullptr;				//收到LCT回复时调用


};
}
}

#endif
