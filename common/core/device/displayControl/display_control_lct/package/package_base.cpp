#include "package_base.h"
#include <random>

namespace common
{
namespace device
{
uint64_t  PackageBase::g_serialNum = 0;

PackageBase::PackageBase() : m_packgeResults(PackageResults::ok)
{
}

PackageBase::PackageBase(const PackageBase& packageBase)
{
	this->fromOtherObj(packageBase);
}

PackageBase::PackageBase(int timeout, std::string endTag, CommResults result, const std::string& data, PackTypes packageType, PackCMDTypes  packageCMDtype)
{
	if (g_serialNum >= 5000)
	{
		g_serialNum = 0;
	}

	g_serialNum++;
	m_Timeout = timeout;
	m_commResult = result;

	m_data = data;
	m_dataLength = data.size();

	m_packageType = packageType;
	m_packCMDTypes = packageCMDtype;
	m_strEndTag = endTag;
	m_packgeResults = PackageResults::ok;

	std::mt19937 rng;
	std::random_device rd;
	// 如果硬件随机源不可用，回退到时间种子
	if (rd.entropy() == 0) {
    	rng.seed(std::time(nullptr));
	}
	else{
		rng.seed(rd());     // 设置种子（使用硬件随机设备）
	}
	//std::uniform_int_distribution<uint64_t> dist(1, UINT64_MAX);  // 选择分布类型 均匀整数分布
	std::uniform_int_distribution<uint64_t> dist(1, 10000000);		// 限制序列号的位数, 数据过长LCT会解析失败
	m_serialNum = dist(rng);

}

PackageBase::~PackageBase()
{

}

PackageBase& PackageBase::operator=(const PackageBase& packageBase)
{
	if (this == &packageBase)
	{
		return *this;
	}

	this->fromOtherObj(packageBase);
	return *this;
}

std::string PackageBase::getEndTag()
{
	return m_strEndTag;
}
void PackageBase::setEndTag(const std::string& tag)
{
	m_strEndTag = tag;
}

uint32_t PackageBase::getSerialNum() {

	return m_serialNum;
}
void PackageBase::setSerialNum(uint32_t serialNum)
{
	m_serialNum = serialNum;
}



void PackageBase::setData(const std::string& data)
{
	m_data = data;
	m_dataLength = data.size();
}
std::string PackageBase::getData()
{
	return m_data;
}
std::string PackageBase::getDataProto()
{
	return m_dataProto;
}
uint32_t PackageBase::getDataLength() const
{
	return m_dataLength;
}

void PackageBase::setCommResults(const CommResults& result)
{
	m_commResult = result;
}
CommResults PackageBase::getCommResults()
{
	return m_commResult;
}

void PackageBase::setPackTypes(const PackTypes& type)
{
	m_packageType = type;
}
PackTypes PackageBase::getPackTypes()
{
	return m_packageType;
}

void PackageBase::setPackCMDTypes(const PackCMDTypes& type)
{
	m_packCMDTypes = type;
}
PackCMDTypes PackageBase::getPackCMDTypes()
{
	return m_packCMDTypes;
}

int PackageBase::getTimeout()
{
	return m_Timeout;
}
void PackageBase::setTimeout(int timeout)
{
	m_Timeout = timeout;
}

void PackageBase::setPackageResults(const PackageResults& result)
{
	m_packgeResults = result;
}
PackageResults PackageBase::getPackageResults()
{
	return m_packgeResults;
}

void PackageBase::setOutCmd(uint32_t cmdCode)
{
	m_cmdCode = cmdCode;
}
uint32_t PackageBase::getOutCmd()
{
	return m_cmdCode;
}

bool PackageBase::isPackageSuccess()
{
	if (m_commResult != CommResults::OK
		|| m_packgeResults != PackageResults::ok)
	{
		return false;
	}
	return true;
}

void PackageBase::fromOtherObj(const PackageBase& packageBase)
{
	//此处表示由对方返回的协议携带的错误类型
	m_commResult = CommResults(packageBase.m_commResult);

	//此处表示本地处理逻辑等一些列的错误类型,诸如发送文件时该文件已经打开失败了
	m_packgeResults = PackageResults(packageBase.m_packgeResults);
	m_packageType = PackTypes(packageBase.m_packageType);
	m_packCMDTypes = PackCMDTypes(packageBase.m_packCMDTypes);

	m_strEndTag = packageBase.m_strEndTag;
	m_serialNum = packageBase.m_serialNum;
	m_Timeout = packageBase.m_Timeout;
	m_cmdCode = packageBase.m_cmdCode;
}

}
}