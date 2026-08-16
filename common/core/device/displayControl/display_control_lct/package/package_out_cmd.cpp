#include "package_out_cmd.h"
#include <common/utility/string_tool/string_format/string_format.hpp>

namespace common
{
namespace device
{
PackageOutCMD::PackageOutCMD()
{

}

PackageOutCMD::PackageOutCMD(bool bReply, uint32_t unionCode, uint32_t cmdCode, uint32_t opeCode, bool bReplyFile, const std::string& path,
	int timeout, const std::string& data, const PackCMDTypes& packageCMDtype)
	:PackageBase(timeout, EndTag_OutCMD, CommResults::OK, data, PackTypes::Pack_OutCMD, packageCMDtype)
{
	m_strSplitTag = SplitTag_OutCMD;
	m_unionCode = unionCode;
	m_cmdCode = cmdCode;
	m_opeCode = opeCode;
	m_bReply = bReply;
	m_bReplyFile = bReplyFile;
	m_path = path;
}

std::string PackageOutCMD::getFilePath()
{
	return m_path;
}
void PackageOutCMD::setFilePath(std::string& path)
{
	m_path = path;
}

std::string PackageOutCMD::getExtFileInfo()
{
	return m_extFileInfo;
}
int PackageOutCMD::getSerialExtFileInfo()
{
	return getSerialNum();
}

std::string PackageOutCMD::packSendRequest()
{
	std::string strData;

	strData.clear();

	strData.append(m_bReply ? "True" : "False");
	strData.append(m_strSplitTag);

	strData.append(std::to_string(m_serialNum));
	strData.append(m_strSplitTag);

	strData.append(std::to_string(m_unionCode));
	strData.append(m_strSplitTag);

	strData.append(std::to_string(m_cmdCode));
	strData.append(m_strSplitTag);

	strData.append(std::to_string(m_opeCode));
	strData.append(m_strSplitTag);

	strData.append(m_bReplyFile ? "True" : "False");
	strData.append(m_strSplitTag);

	strData.append(m_data);
	strData.append(m_strSplitTag);

	strData.append(std::to_string(this->getTimeout()));

	m_dataProto = strData;

	hlogd("[common][device][display_control]---外部命令---组包发送:%s", strData.c_str());

	return m_dataProto;
}

bool PackageOutCMD::parsePackageReply(const std::string& data)
{
	std::vector<std::string> vec = common::utility::StringFormat::splitString(data, SplitTag_OutCMD);

	// 判断分隔符后面是否有数据，如果没有时添加一个空数据
	bool endEmpty = common::utility::StringFormat::rightString(data, std::string(SplitTag_OutCMD).length()) == SplitTag_OutCMD ? true : false;
	if (endEmpty)
		vec.push_back("");

	int splitCount = vec.size();

	// 分割字符串为4代表LCT的回复包，分割字符串为8代表LCT发送包
	if (splitCount > 4 && vec.at(0) != "True") {

		//添加接收LCT发送包功能
		//LCT发送包序列号，对应发送回复包时使用
		m_serialNum = stoull(vec.at(1));

		//获取LCT发送包命令字
		m_cmdCode = stoul(vec.at(3));

		//获取LCT发送包数据内容
		if (splitCount >= 7)
			setData(vec.at(6));
	}
	else
	{
		//LCT回复包序列号
		if (splitCount >= 2)
			setSerialNum(stoull(vec.at(1)));

		//发送包的回复执行结果成功/失败定义
		if (splitCount >= 3)
			setCommResults((CommResults)stoi(vec.at(2)));

		//发送包的回复执行结果数据
		if (splitCount >= 4)
			setData(vec.at(3));
	}


	return true;
}

bool PackageOutCMD::parsePackageReply(const std::string& data, PackageResults packgeResults)
{
	std::vector<std::string> vec = common::utility::StringFormat::splitString(data, SplitTag_OutCMD);
	if (vec.size() == 4)
	{
		//内容个数匹配上		
		if (vec.at(0) != "True")	//是否是回复包
		{
			//发送包的回复执行结果成功/失败定义
			setPackageResults(PackageResults::request_replyErr);
		}
		else
		{
			setPackageResults(packgeResults);

			//发送包序列号
			setSerialNum(stoi(vec.at(1)));

			//发送包的回复执行结果成功/失败定义
			setCommResults((CommResults)stoi(vec.at(2)));

			//发送包的回复执行结果数据
			setData(vec.at(3));
		}
	}
	return true;
}



}

}