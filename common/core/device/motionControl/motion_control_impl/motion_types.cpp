#include "motion_types.h"

void common::device::axis::Axis::reflectFromJson(const json& j)
{
	axisDafaultParams.reflectFromJson(j);
	axisMoveParams.reflectFromJson(j);
}

void common::device::axis::Axis::reflectToJson(json& j) const
{
	axisDafaultParams.reflectToJson(j);
	axisMoveParams.reflectToJson(j);
}

void common::device::axis::Axis::AxisData::reflectFromJson(const json& j)
{
	getFromJson(j, "enable", m_enable);
	getFromJson(j, "axisNO", m_axisNO);
	getFromJson(j, "slaveID", m_slaveID);
	getFromJson(j, "axisType", m_axisType);
	getFromJson(j, "unitType", m_unitType);
	getFromJson(j, "encoderType", m_encoderType);
	getFromJson(j, "driverType", m_driverType);
	getFromJson(j, "cardName", m_cardName);
	getFromJson(j, "moduleName", m_moduleName);
	getFromJson(j, "homeDir", m_homeDir);
	getFromJson(j, "homeEdgeType", m_homeEdgeType);
	getFromJson(j, "homeVelMode", m_homeVelMode);
	getFromJson(j, "homePinLogic", m_homePinLogic);
	getFromJson(j, "homeSourceSel", m_homeSourceSel);
	getFromJson(j, "homeMode", m_homeMode);
	getFromJson(j, "homeOffset", m_homeOffset);
	getFromJson(j, "homeHighVel", m_homeHighVel);
	getFromJson(j, "homeLowVel", m_homeLowVel);
	getFromJson(j, "homeTacc", m_homeTacc);
	getFromJson(j, "equiv", m_equiv);
	getFromJson(j, "pulseUnit", m_pulseUnit);
	getFromJson(j, "sourceSelLimit", m_sourceSelLimit);
	getFromJson(j, "softwareLimitAction", m_softwareLimitAction);
	getFromJson(j, "negativeLimit", m_negativeLimit);
	getFromJson(j, "positiveLimit", m_positiveLimit);
	getFromJson(j, "errorBandPluse", m_errorBandPluse);
}

void common::device::axis::Axis::MoveParameters::reflectFromJson(const json& j)
{
	getFromJson(j, "startVel", m_startVel);
	getFromJson(j, "stopVel", m_stopVel);
	getFromJson(j, "maxVel", m_maxVel);
	getFromJson(j, "tacc", m_tacc);
	getFromJson(j, "tdec", m_tdec);
	getFromJson(j, "sPara", m_sPara);
}

void common::device::axis::Axis::AxisData::reflectToJson(json& j) const
{
	j["enable"] = m_enable;
	j["axisNO"] = m_axisNO;
	j["slaveID"] = m_slaveID;
	j["axisType"] = m_axisType;
	j["unitType"] = m_unitType;
	j["encoderType"] = m_encoderType;
	j["driverType"] = m_driverType;
	j["cardName"] = m_cardName;
	j["moduleName"] = m_moduleName;
	j["homeDir"] = m_homeDir;
	j["homeEdgeType"] = m_homeEdgeType;
	j["homeVelMode"] = m_homeVelMode;
	j["homePinLogic"] = m_homePinLogic;
	j["homeSourceSel"] = m_homeSourceSel;
	j["homeMode"] = m_homeMode;
	j["homeOffset"] = m_homeOffset;
	j["homeHighVel"] = m_homeHighVel;
	j["homeLowVel"] = m_homeLowVel;
	j["homeTacc"] = m_homeTacc;
	j["equiv"] = m_equiv;
	j["pulseUnit"] = m_pulseUnit;
	j["sourceSelLimit"] = m_sourceSelLimit;
	j["softwareLimitAction"] = m_softwareLimitAction;
	j["negativeLimit"] = m_negativeLimit;
	j["positiveLimit"] = m_positiveLimit;
	j["errorBandPluse"] = m_errorBandPluse;
}

void common::device::axis::Axis::MoveParameters::reflectToJson(json& j) const
{
	j["startVel"] = m_startVel;
	j["stopVel"] = m_stopVel;
	j["maxVel"] = m_maxVel;
	j["tacc"] = m_tacc;
	j["tdec"] = m_tdec;
	j["sPara"] = m_sPara;
}
