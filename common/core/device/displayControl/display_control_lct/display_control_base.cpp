#include "display_control_base.h"

namespace common
{
namespace device
{
enum_ScreenCorrectType DisplayControlBase::getCurrentCorrectType() const
{	
	return m_currentCorrectType;
}
void DisplayControlBase::setCurrentCorrectType(const enum_ScreenCorrectType& val)
{
	m_currentCorrectType = val;
}

common::utility::ResultInfo DisplayControlBase::checkError(long rtn, const std::string& strFun)
{
	common::utility::ResultInfo resultInfo;
	std::string rtnStr;
	if (makeError(rtn, strFun, rtnStr))
		return resultInfo;

	resultInfo.resultCode_ = rtn;
	resultInfo.resultMsg_ = rtnStr;
	return resultInfo;
}

}

}

