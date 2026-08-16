#ifndef _LCT_PACKAGE_INNER_CMD_HH
#define _LCT_PACKAGE_INNER_CMD_HH

#include "package_base.h"

namespace common
{
namespace device
{
#define SplitTag_InnerCMD "%*$%"
#define EndTag_InnerCMD "!##!"

#define RequestConnection_tag "Request activeInfo ID"
#define RequestConnectionReply_tag "Reply activeInfo ID"

#define RequestConnectionFile_tag "Regist file Connected"
#define RequestConnectionFileReply_tag "Client connected success"
#define RequestConnectionAllReply_tag "Client connected full"

#define FileInfoOver_tag "file info over"
#define FileOver_tag "file over"

class PackageInnerCMD : public PackageBase
{
public:
	PackageInnerCMD();
	PackageInnerCMD(const PackageInnerCMD &obj);
    PackageInnerCMD(std::string cmdTag, std::string extInfo, int num, int timeout, PackCMDTypes  packageCMDtype);
	PackageInnerCMD & operator=(const PackageInnerCMD &obj);
	virtual ~PackageInnerCMD() {}

public:	
	std::string packSendRequest();

	bool parsePackageReply(void *data, int typeMsg);

private:
	std::string m_strSplitTag;                      //分隔符
	std::string m_strCMDTag;						//命令符

	int m_serialNumCMD;								//序列号
    std::string m_extInfo;							//附加信息
};
}
}
#endif


