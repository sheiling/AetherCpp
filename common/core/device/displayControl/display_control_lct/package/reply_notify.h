#ifndef _LCT_REPLY_NOTIFY_HH
#define _LCT_REPLY_NOTIFY_HH

#include "../../display_control_data.h"
#include "package_base.h"

namespace common
{
namespace device
{

class ReplyNotify
{
public:
	ReplyNotify();
	~ReplyNotify();

	std::string getId() const;
	void setId(const std::string& value);

	void* getReply() const;
	void setReply(void* info);

	PackFromTypes getReplyType() const;
	void setReplyType(PackFromTypes replyType);

	std::string getReplyData() const;
	void setReplyData(const std::string& replyData);

	PackageResults getPackageResults() const;
	void setPackageResults(PackageResults packgeResults);

private:
	std::string				m_id = "";
	void*					m_pReply;
	PackFromTypes			m_replyType;
	std::string				m_replyData;
	PackageResults			m_packgeResults;
};

}

}

#endif
