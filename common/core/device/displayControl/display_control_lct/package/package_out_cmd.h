#ifndef _LCT_PACKAGE_OUT_CMD_HH
#define _LCT_PACKAGE_OUT_CMD_HH

#include "package_base.h"

namespace common
{
namespace device
{

#define SplitTag_OutCMD "&^^&"
#define EndTag_OutCMD "|%%|"

class PackageOutCMD : public PackageBase
{
public:
	PackageOutCMD();
	PackageOutCMD(bool bReply, uint32_t unionCode, uint32_t cmdCode, uint32_t opeCode, bool bReplyFile, const std::string& path,
		int timeout, const std::string& data, const PackCMDTypes& packageCMDtype);

	virtual ~PackageOutCMD() {}

	std::string getFilePath();
	void setFilePath(std::string& path);

	std::string getExtFileInfo();
	int getSerialExtFileInfo();

public:
	std::string packSendRequest();									    //组建发包-下行

	bool parsePackageReply(const std::string& data);					//解析回包-上行
	bool parsePackageReply(const std::string& data, PackageResults packgeResults);	//解析回包(内部逻辑产生)-上行


private:
	std::string m_strSplitTag;                      //分隔符
	std::string m_path = "";
	std::string m_extFileInfo = "";                 //文件的附加信息

	uint32_t m_unionCode;
	uint32_t m_opeCode;
	bool m_bReply;
	bool m_bReplyFile;
};
}
}

#endif


