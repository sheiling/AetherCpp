#include "requst_notify.h"

namespace common
{
namespace device
{

RequstNotify::RequstNotify()
{

}

RequstNotify::~RequstNotify()
{

}

std::shared_ptr<PackageBase> RequstNotify::getRequstSourcePacke()
{
	return m_requstSourcePack;
}

void RequstNotify::setRequstSourcePacke(std::shared_ptr<PackageBase> packe)
{
	m_requstSourcePack = packe;
}

std::shared_ptr<PackageBase> RequstNotify::getRequstReplyPacke()
{
	return m_requstReplyPack;
}

void RequstNotify::setRequstReplyPacke(std::shared_ptr<PackageBase> packe)
{
	m_requstReplyPack = packe;
}

std::shared_ptr<CorrectionReplyPack> RequstNotify::getCorrectionReplyPack()
{
	return m_correctionReplyPack;
}

void RequstNotify::setCorrectionReplyPack(std::shared_ptr<CorrectionReplyPack> info)
{
	m_correctionReplyPack = info;
}

enum_CommReplySta RequstNotify::getCommReplySta()
{
	std::shared_lock<std::shared_mutex> lock(m_commReplyStaMutex);
	return m_commReplySta;
}

void RequstNotify::setCommReplySta(enum_CommReplySta sta)
{
	std::shared_lock<std::shared_mutex> lock(m_commReplyStaMutex);
	m_commReplySta = sta;
}

}

}