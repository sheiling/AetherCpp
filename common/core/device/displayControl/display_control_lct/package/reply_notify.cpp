#include "reply_notify.h"

namespace common
{
namespace device
{

ReplyNotify::ReplyNotify()
{

}

ReplyNotify::~ReplyNotify()
{

}


std::string ReplyNotify::getId() const
{
	return m_id;
}

void ReplyNotify::setId(const std::string& value)
{
	m_id = value;
}

void* ReplyNotify::getReply() const
{
	return m_pReply;
}

void ReplyNotify::setReply(void* info)
{
	m_pReply = info;
}

PackFromTypes ReplyNotify::getReplyType() const
{
	return m_replyType;
}

void ReplyNotify::setReplyType(PackFromTypes replyType)
{
	m_replyType = replyType;
}

std::string ReplyNotify::getReplyData() const
{
	return m_replyData;
}

void ReplyNotify::setReplyData(const std::string& replyData)
{
	m_replyData = replyData;
}

PackageResults ReplyNotify::getPackageResults() const
{
	return m_packgeResults;
}

void ReplyNotify::setPackageResults(PackageResults packgeResults)
{
	m_packgeResults = packgeResults;
}

}

}