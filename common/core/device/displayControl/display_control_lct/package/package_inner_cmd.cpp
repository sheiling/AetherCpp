#include "package_inner_cmd.h"

namespace common
{
namespace device
{
PackageInnerCMD::PackageInnerCMD()
{

}

PackageInnerCMD::PackageInnerCMD(std::string cmdTag, std::string extInfo, int num, int timeout, PackCMDTypes  packageCMDtype)
	:PackageBase(timeout, EndTag_InnerCMD, CommResults::OK, "", PackTypes::Pack_InnerCMD, packageCMDtype)
{
	m_strSplitTag = SplitTag_InnerCMD;
	m_strCMDTag = cmdTag;			//命令符
	m_extInfo = extInfo;			//附加信息
	m_serialNumCMD = num;			//序列号
}
PackageInnerCMD& PackageInnerCMD::operator=(const PackageInnerCMD& obj)
{
	return *this;
}


std::string PackageInnerCMD::packSendRequest()
{
	std::string strData;
	//组包
	strData.append(m_strCMDTag);
	strData.append(m_strSplitTag);
	strData.append(std::to_string(m_serialNumCMD));
	strData.append(m_strSplitTag);
	strData.append(m_extInfo);

	m_dataProto = strData;

	hlogd("[common][device][display_control]---内部命令---组包发送:%s", m_dataProto.c_str());

	return m_dataProto;
}

bool PackageInnerCMD::parsePackageReply(void* data, int typeMsg)
{

	return true;
}


}

}