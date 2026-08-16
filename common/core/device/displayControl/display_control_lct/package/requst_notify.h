#ifndef _LCT_REQUST_NOTIFY_HH
#define _LCT_REQUST_NOTIFY_HH

#include "../../display_control_data.h"
#include "package_base.h"
#include <shared_mutex>

namespace common
{
namespace device
{

class RequstNotify
{
public:
	RequstNotify();
	~RequstNotify();

	std::shared_ptr<PackageBase> getRequstSourcePacke();
	void setRequstSourcePacke(std::shared_ptr<PackageBase> packe);

	std::shared_ptr<PackageBase> getRequstReplyPacke();
	void setRequstReplyPacke(std::shared_ptr<PackageBase> packe);

	std::shared_ptr<CorrectionReplyPack> getCorrectionReplyPack();
	void setCorrectionReplyPack(std::shared_ptr<CorrectionReplyPack> info);

	enum_CommReplySta getCommReplySta();
	void setCommReplySta(enum_CommReplySta sta);


private:
	std::shared_ptr<PackageBase>			m_requstSourcePack = nullptr;					//请求原始数据包
	std::shared_ptr<PackageBase>			m_requstReplyPack = nullptr;				 	//请求回复数据包
	std::shared_ptr<CorrectionReplyPack>	m_correctionReplyPack = nullptr;;				//校正数据回复包
	enum_CommReplySta						m_commReplySta = enum_CommReplySta::Wait;	 	//通信接收回包状态

	std::shared_mutex     					m_commReplyStaMutex;

};

}

}

#endif
