//=================================================================================================
//===
//=== data_sampler.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс пробоотборника
//===
//=================================================================================================

#include "data_sampler.h"
#include <vector>
#include <string.h>
#include "event_manager.h"
#include "error.h"
#include "event_eid.h"
#include "data_config.h"
#include "variable_list.h"
#include "xml_util.h"
#include "text_id.h"

rBitsArray rSampler::m_flagsMode;
rBitsArray rSampler::m_flagsSetup;


//-------------------------------------------------------------------------------------------------
//
rSampler::rSampler()
{
	if (m_flagsMode.empty()) {
		m_flagsMode
				.add("PERIOD", static_cast<UINT>(Mode::PERIOD))
				.add("MASS"  , static_cast<UINT>(Mode::MASS))
				.add("VOLUME", static_cast<UINT>(Mode::VOLUME));
	}

	if (m_flagsSetup.empty()) {
		m_flagsSetup
				.add("OFF"         , static_cast<UINT>(Setup::OFF))
				.add("ERR2RESERVE" , static_cast<UINT>(Setup::ERR_RESERV))
				.add("FILL2RESERVE", static_cast<UINT>(Setup::FILL_RESERV))
				.add("SINGLECAN"   , static_cast<UINT>(Setup::SINGLE_CAN))
				.add("DUALCAN"     , static_cast<UINT>(Setup::DUAL_CAN))
				.add("AUTOSWITCH"  , static_cast<UINT>(Setup::AUTOSWITCH));
	}

	for (UDINT ii = 0; ii < CAN_MAX; ++ii) {
		std::string name = String_format("can_%c.", 'a' + ii);
		InitLink(rLink::Setup::INPUT , m_can[ii].m_overflow, U_discrete, SID::CANFILLED, name + XmlName::CANFILLED, rLink::SHADOW_NONE);
		InitLink(rLink::Setup::INPUT , m_can[ii].m_fault   , U_discrete, SID::FAULT    , name + XmlName::FAULT    , rLink::SHADOW_NONE);
		InitLink(rLink::Setup::INPUT , m_can[ii].m_weight  , U_g  , SID::CANMASS  , name + XmlName::MASS     , rLink::SHADOW_NONE);
	}
	InitLink(rLink::Setup::INPUT , m_ioStart , U_discrete, SID::CANIOSTART, XmlName::IOSTART, rLink::SHADOW_NONE);
	InitLink(rLink::Setup::INPUT , m_ioStop  , U_discrete, SID::CANIOSTOP , XmlName::IOSTOP , rLink::SHADOW_NONE);

	InitLink(rLink::Setup::OUTPUT, m_grab    , U_discrete, SID::GRAB     , XmlName::GRAB     , rLink::SHADOW_NONE);
	InitLink(rLink::Setup::OUTPUT, m_selected, U_discrete, SID::CANSELECT, XmlName::SELECTED , rLink::SHADOW_NONE);
}


//
rSampler::~rSampler()
{
	;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rSampler::InitLimitEvent(rLink &/*link*/)
{
	return 0;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rSampler::Calculate()
{
	if(rSource::Calculate()) return TRITONN_RESULT_OK;

	switch (m_state) {
		case State::IDLE:
			switch (m_command) {
				case Command::NONE : break;
				case Command::START: onStart(); break;
				case Command::STOP : break;
				case Command::TEST : break;
			}
			break;

		case State::TEST:
			break;

		case State::WORKTIME:
			break;

		case State::WORKVOLUME:
			break;

		case State::WORKMASS:
			break;
	}

	m_command = Command::NONE;

	PostCalculate();

	return TRITONN_RESULT_OK;
}

//-------------------------------------------------------------------------------------------------
//
UDINT rSampler::generateVars(rVariableList& list)
{
	rSource::generateVars(list);

	// Variables
	list.add(Alias + ".mode"        , TYPE_UINT , rVariable::Flags::___L, &m_mode       , U_DIMLESS, ACCESS_SAMPLERS | ACCESS_SETSAMPLERS);
	list.add(Alias + ".setup"       , TYPE_UINT , rVariable::Flags::___L, &m_setup.Value, U_DIMLESS, ACCESS_SAMPLERS | ACCESS_SETSAMPLERS);
	list.add(Alias + ".select"      , TYPE_UINT , rVariable::Flags::____, &m_select     , U_DIMLESS, ACCESS_SAMPLERS | ACCESS_SETSAMPLERS);
	list.add(Alias + ".command"     , TYPE_UINT , rVariable::Flags::___L, &m_command    , U_DIMLESS, ACCESS_SAMPLERS | ACCESS_SETSAMPLERS);
	list.add(Alias + ".probe.period", TYPE_UDINT, rVariable::Flags::___L, &m_probePeriod, U_DIMLESS, ACCESS_SAMPLERS | ACCESS_SETSAMPLERS);
	list.add(Alias + ".probe.volume", TYPE_LREAL, rVariable::Flags::___L, &m_probeVolume, U_DIMLESS, ACCESS_SAMPLERS | ACCESS_SETSAMPLERS);
	list.add(Alias + ".probe.mass"  , TYPE_LREAL, rVariable::Flags::___L, &m_probeMass  , U_DIMLESS, ACCESS_SAMPLERS | ACCESS_SETSAMPLERS);
	list.add(Alias + ".grabtest"    , TYPE_UDINT, rVariable::Flags::___L, &m_grabTest   , U_DIMLESS, ACCESS_SAMPLERS | ACCESS_SETSAMPLERS);
	list.add(Alias + ".grabvol"     , TYPE_LREAL, rVariable::Flags::R___, &m_grabVol    , U_ml     , 0);
	list.add(Alias + ".volume"      , TYPE_LREAL, rVariable::Flags::____, &m_volume     , U_ml     , 0);
	list.add(Alias + ".volRemain"   , TYPE_LREAL, rVariable::Flags::____, &m_volRemain  , U_ml     , 0);
	list.add(Alias + ".grabcount"   , TYPE_UDINT, rVariable::Flags::R___, &m_grabCount  , U_DIMLESS, 0);
	list.add(Alias + ".grabremain"  , TYPE_UDINT, rVariable::Flags::R___, &m_grabRemain , U_DIMLESS, 0);
	list.add(Alias + ".interval"    , TYPE_LREAL, rVariable::Flags::R___, &m_interval   , U_DIMLESS, 0);
	list.add(Alias + ".state"       , TYPE_UINT , rVariable::Flags::R___, &m_state      , U_DIMLESS, 0);

	for (UDINT ii = 0; ii < CAN_MAX; ++ii) {
		std::string prefix = Alias + String_format(".can_%c", 'a' + ii);
		list.add(prefix + ".volume", TYPE_LREAL, rVariable::Flags::___L, &m_can[ii].m_volume, U_ml, ACCESS_SAMPLERS | ACCESS_SETSAMPLERS);
	}

	return TRITONN_RESULT_OK;
}

UDINT rSampler::Can::loadFromXML(tinyxml2::XMLElement *element, rError &err)
{
	tinyxml2::XMLElement* xml_overflow = element->FirstChildElement(XmlName::OVERFLOW_);
	tinyxml2::XMLElement* xml_fault    = element->FirstChildElement(XmlName::FAULT);
	tinyxml2::XMLElement* xml_weight   = element->FirstChildElement(XmlName::WEIGHT);

	if (rDataConfig::instance().LoadLink(xml_overflow, m_overflow, false) != TRITONN_RESULT_OK ||
		rDataConfig::instance().LoadLink(xml_fault   , m_fault   , false) != TRITONN_RESULT_OK ||
		rDataConfig::instance().LoadLink(xml_weight  , m_weight  , false) != TRITONN_RESULT_OK) {
		return err.set(DATACFGERR_SAMPLER_CAN, element->GetLineNum(), "link fault");
	}

	UDINT fault = 0;
	m_volume = XmlUtils::getTextUDINT(element->FirstChildElement(XmlName::VOLUME), 0, fault);

	if (fault) {
		return err.set(DATACFGERR_SAMPLER_CAN, element->GetLineNum(), "fault volume");
	}

	return TRITONN_RESULT_OK;
}


UDINT rSampler::LoadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix)
{
	std::string strMode  = XmlUtils::getAttributeString(element, XmlName::MODE , m_flagsMode.getNameByBits (static_cast<UINT>(Mode::PERIOD)));
	std::string strSetup = XmlUtils::getAttributeString(element, XmlName::SETUP, m_flagsSetup.getNameByBits(Setup::OFF));

	if (rSource::LoadFromXML(element, err, prefix) != TRITONN_RESULT_OK) {
		return err.getError();
	}

	tinyxml2::XMLElement* xml_totals   = element->FirstChildElement(XmlName::TOTALS);
	tinyxml2::XMLElement* xml_iostart  = element->FirstChildElement(XmlName::IOSTART);
	tinyxml2::XMLElement* xml_iostop   = element->FirstChildElement(XmlName::IOSTOP);
	tinyxml2::XMLElement* xml_reserve  = element->FirstChildElement(XmlName::RESERVE);
	tinyxml2::XMLElement* xml_grabvol  = element->FirstChildElement(XmlName::GRABVOL);
	tinyxml2::XMLElement* xml_period   = element->FirstChildElement(XmlName::PERIOD);
	tinyxml2::XMLElement* xml_grabtest = element->FirstChildElement(XmlName::GRABTEST);
	tinyxml2::XMLElement* xml_can_a    = element->FirstChildElement(XmlName::CAN1);
	tinyxml2::XMLElement* xml_can_b    = element->FirstChildElement(XmlName::CAN2);

	UDINT fault = 0;

	if (!xml_totals) {
		return err.set(DATACFGERR_SAMPLER_TOTALS, element->GetLineNum(), "");
	}
	if (!xml_can_a) {
		return err.set(DATACFGERR_SAMPLER_CAN, element->GetLineNum(), "can A");
	}

	m_mode = static_cast<Mode>(m_flagsMode.getValue(strMode, fault));
	if (fault) {
		return err.set(DATACFGERR_SAMPLER_MODE, element->GetLineNum(), "");
	}

	m_setup.Init(m_flagsSetup.getValue(strSetup, fault));
	if (fault) {
		return err.set(DATACFGERR_SAMPLER_SETUP, element->GetLineNum(), "");
	}

	if (!xml_reserve && m_setup.Value & (Setup::ERR_RESERV | Setup::FILL_RESERV)) {
		return err.set(DATACFGERR_SAMPLER_RESERVE_NF, element->GetLineNum(), "");
	}

	if (!xml_can_b && m_setup.Value & (Setup::DUAL_CAN | Setup::AUTOSWITCH)) {
		return err.set(DATACFGERR_SAMPLER_CAN, element->GetLineNum(), "can B");
	}

	m_totalsAlias = XmlUtils::getTextString(xml_totals, "", fault);
	if (fault) {
		return err.set(DATACFGERR_SAMPLER_TOTALS, element->GetLineNum(), "empty alias");
	}

	if (xml_reserve) {
		m_reserveAlias = XmlUtils::getTextString(xml_reserve, "", fault);
		if (fault) {
			return err.set(DATACFGERR_SAMPLER_RESERVE, xml_reserve->GetLineNum(), "");
		}
	}

	if (rDataConfig::instance().LoadLink(xml_iostart, m_ioStart, false) != TRITONN_RESULT_OK) {
		return err.getError();
	}

	if (rDataConfig::instance().LoadLink(xml_iostop, m_ioStop, false) != TRITONN_RESULT_OK) {
		return err.getError();
	}

	if (m_can[0].loadFromXML(xml_can_a, err) != TRITONN_RESULT_OK) {
		return err.getError();
	}

	if (xml_can_b) {
		if (m_can[1].loadFromXML(xml_can_b, err) != TRITONN_RESULT_OK) {
			return err.getError();
		}
	}

	m_grabVol     = XmlUtils::getTextLREAL(xml_grabvol , 1.0  , fault);
	m_grabTest    = XmlUtils::getTextUINT (xml_grabtest, 100  , fault);
	m_probePeriod = XmlUtils::getTextUDINT(xml_period  , 43200, fault);

	return TRITONN_RESULT_OK;
}


UDINT rSampler::check(rError& err)
{
	const rSource* src = rDataConfig::instance().getSource(m_totalsAlias);

	if (!src) {
		return err.set(DATACFGERR_SAMPLER_TOTALS, m_lineNum, "can't fount source '" + m_totalsAlias + "'");
	}

	m_totals = src->getTotal();

	if (!m_totals) {
		return err.set(DATACFGERR_SAMPLER_TOTALS, m_lineNum, "source '" + m_totalsAlias + "' does not contain totals");
	}


	if (m_reserveAlias.empty()) {
		return TRITONN_RESULT_OK;
	}

	src = rDataConfig::instance().getSource(m_reserveAlias);

	if (!src) {
		return err.set(DATACFGERR_SAMPLER_TOTALS, m_lineNum, "can't fount source '" + m_totalsAlias + "'");
	}

	if (std::string(src->RTTI()) != RTTI()) {
		return err.set(DATACFGERR_SAMPLER_RESERVE, m_lineNum, "source '" + m_totalsAlias + "' is not sampler");
	}

	m_reserve = dynamic_cast<const rSampler*>(src);

	return TRITONN_RESULT_OK;
}

std::string rSampler::saveKernel(UDINT isio, const std::string& objname, const std::string& comment, UDINT isglobal)
{
	UNUSED(isio);
	UNUSED(isglobal);

	m_ioStart.Limit.m_setup.Init(rLimit::Setup::OFF);
	m_ioStop.Limit.m_setup.Init(rLimit::Setup::OFF);
	m_grab.Limit.m_setup.Init(rLimit::Setup::OFF);
	m_selected.Limit.m_setup.Init(rLimit::Setup::OFF);

	for (UDINT ii = 0; ii < CAN_MAX; ++ii) {
		m_can[ii].m_overflow.Limit.m_setup.Init(rLimit::Setup::OFF);
		m_can[ii].m_fault.Limit.m_setup.Init(rLimit::Setup::OFF);
		m_can[ii].m_weight.Limit.m_setup.Init(rLimit::Setup::OFF);
	}

	return rSource::saveKernel(false, objname, comment, false);
}


void rSampler::onStart()
{
	m_interval   = 0;
	m_grabCount  = 0;
	m_grabRemain = 0;
	m_volRemain  = 0;

	switch (m_mode) {
		case Mode::PERIOD:
			m_grabRemain = static_cast<UDINT>(m_volume / m_grabVol + 0.5);
			m_interval   = m_probePeriod / m_grabRemain;
			m_state      = State::WORKTIME;
			break;

		case Mode::VOLUME:
			m_grabRemain = static_cast<UDINT>(m_volume / m_grabVol + 0.5);
			m_interval   = m_probeVolume / m_grabRemain;
			m_state      = State::WORKVOLUME;
			break;

		case Mode::MASS:
			m_grabRemain = static_cast<UDINT>(m_volume / m_grabVol + 0.5);
			m_interval   = m_probeMass / m_grabRemain;
			m_state      = State::WORKMASS;
			break;

		default:
			rEventManager::instance().Add(ReinitEvent(EID_SAMPLER_MODE_FAULT) << static_cast<UINT>(m_mode));
			m_mode = Mode::PERIOD;
			break;
	}
}
