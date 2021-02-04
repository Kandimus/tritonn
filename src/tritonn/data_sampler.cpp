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
//#include <string.h>
#include "tickcount.h"
#include "event_manager.h"
#include "error.h"
#include "event_eid.h"
#include "data_config.h"
#include "variable_list.h"
#include "xml_util.h"
#include "text_id.h"
#include "generator_md.h"

rBitsArray rSampler::m_flagsMethod;
rBitsArray rSampler::m_flagsSetup;

const UDINT LE_IO_START = 0x00000001;
const UDINT LE_IO_STOP  = 0x00000002;


//-------------------------------------------------------------------------------------------------
//
rSampler::rSampler(const rStation* owner) : rSource(owner)
{
	if (m_flagsMethod.empty()) {
		m_flagsMethod
				.add("PERIOD", static_cast<UINT>(Method::PERIOD), "Отбор проб по времени")
				.add("MASS"  , static_cast<UINT>(Method::MASS)  , "Отбор проб по данным массового расхода")
				.add("VOLUME", static_cast<UINT>(Method::VOLUME), "Отбор проб по данным объемного расхода");
	}

	if (m_flagsSetup.empty()) {
		m_flagsSetup
				.add("OFF"         , static_cast<UINT>(Setup::OFF)        , "Отключить объект")
				.add("ERR2RESERVE" , static_cast<UINT>(Setup::ERR_RESERV) , "При аварии перейти на резервный пробоотборник")
				.add("FILL2RESERVE", static_cast<UINT>(Setup::FILL_RESERV), "При заполнении переходить на резервный пробоотборник")
				.add("SINGLECAN"   , static_cast<UINT>(Setup::SINGLE_CAN) , "Используется только один бак пробоотборника")
				.add("DUALCAN"     , static_cast<UINT>(Setup::DUAL_CAN)   , "Использовать два бака пробоотборника")
				.add("AUTOSWITCH"  , static_cast<UINT>(Setup::AUTOSWITCH) , "При заполнении переходить на другой бак пробоотборника");
	}

	for (UDINT ii = 0; ii < CAN_MAX; ++ii) {
		std::string name = String_format("can_%c.", 'a' + ii);
		InitLink(rLink::Setup::INPUT , m_can[ii].m_overflow, U_discrete, SID::CANFILLED, name + XmlName::OVERFLOW_, rLink::SHADOW_NONE);
		InitLink(rLink::Setup::INPUT , m_can[ii].m_fault   , U_discrete, SID::FAULT    , name + XmlName::FAULT    , rLink::SHADOW_NONE);
		InitLink(rLink::Setup::INPUT , m_can[ii].m_weight  , U_g       , SID::CANMASS  , name + XmlName::MASS     , rLink::SHADOW_NONE);
	}
	InitLink(rLink::Setup::INPUT , m_ioStart , U_discrete, SID::CANIOSTART, XmlName::IOSTART, rLink::SHADOW_NONE);
	InitLink(rLink::Setup::INPUT , m_ioStop  , U_discrete, SID::CANIOSTOP , XmlName::IOSTOP , rLink::SHADOW_NONE);

	InitLink(rLink::Setup::OUTPUT, m_grab    , U_discrete, SID::GRAB     , XmlName::GRAB     , rLink::SHADOW_NONE);
	InitLink(rLink::Setup::OUTPUT, m_selected, U_discrete, SID::CANSELECT, XmlName::SELECTED , rLink::SHADOW_NONE);

	// Нарастающие подцепим в chack()
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
	m_grab.Value = false;

	if (rSource::Calculate()) {
		return TRITONN_RESULT_OK;
	}

	checkIO();

	switch (m_state) {
		case State::IDLE:
		case State::FINISH:     onIdle();            break;
		case State::TEST:       onWorkTimer(false);  break;
		case State::WORKTIME:   onWorkTimer(true);   break;
		case State::WORKVOLUME: onWorkVolume(false); break;
		case State::WORKMASS:   onWorkVolume(true);  break;
		case State::PAUSE:                           break;
		case State::ERROR:      onWorkError();       break;
	}

	m_command = Command::NONE;

	PostCalculate();

	return TRITONN_RESULT_OK;
}


void rSampler::onIdle(void)
{
	switch (m_command) {
		case Command::NONE   : break;
		case Command::START  : onStart(); break;
		case Command::STOP   : onStop(); break;
		case Command::TEST   : onStartTest(); break;
		case Command::CONFIRM: break;
		case Command::PAUSE  : break;
		case Command::RESUME : break;

		default:
			rEventManager::instance().Add(ReinitEvent(EID_SAMPLER_COMMAND_FAULT) << static_cast<UINT>(m_command));
			break;
	}
}


void rSampler::onStart()
{
	m_interval      = 0;
	m_grabCount     = 0;
	m_grabPresent   = 0;
	m_grabRemain    = 0;
	m_timeRemain    = 0;
	m_canRemain     = 0;
	m_canPresent    = 0;
	m_noflow        = false;
	m_timeStart     = 0;
	m_timerInterval = 0;

	if (m_select >= CAN_MAX) {
		m_state = State::ERROR;
		rEventManager::instance().Add(ReinitEvent(EID_SAMPLER_SELECT_FAULT));
		return;
	}

	switch (m_method) {
		case Method::PERIOD:
			m_grabCount = static_cast<UDINT>(m_canVolume / m_grabVol + 0.5);
			m_interval  = m_probePeriod / m_grabRemain;

			if (checkInterval()) {
				m_state  = State::WORKTIME;
				rEventManager::instance().Add(ReinitEvent(EID_SAMPLER_START_PERIOD));
			}
			break;

		case Method::VOLUME:
			m_grabCount = static_cast<UDINT>(m_canVolume / m_grabVol + 0.5);
			m_interval  = m_probeVolume / m_grabRemain;
			m_state     = State::WORKVOLUME;

			rEventManager::instance().Add(ReinitEvent(EID_SAMPLER_START_VOLUME));
			break;

		case Method::MASS:
			m_grabCount = static_cast<UDINT>(m_canVolume / m_grabVol + 0.5);
			m_interval  = m_probeMass / m_grabRemain;
			m_state     = State::WORKMASS;

			rEventManager::instance().Add(ReinitEvent(EID_SAMPLER_START_MASS));
			break;

		default:
			rEventManager::instance().Add(ReinitEvent(EID_SAMPLER_MODE_FAULT) << static_cast<UINT>(m_method));
			m_method = Method::PERIOD;
			m_state  = State::ERROR;
			return;
	}

	m_lastRawTotal   = m_totals->Raw;
	m_grabRemain    = m_grabCount;
	m_canRemain     = m_can[m_select].m_volume;
	m_timeStart     = rTickCount::UnixTime();
	m_timerInterval = rTickCount::SysTick();
}


void rSampler::onStop(void)
{
	m_state = State::IDLE;

	rEventManager::instance().Add(ReinitEvent(EID_SAMPLER_STOP));
}


void rSampler::onStartTest(void)
{
	m_noflow        = false;
	m_interval      = 2000 * m_probeTest;
	m_state         = State::TEST;
	m_lastRawTotal  = m_totals->Raw;
	m_grabCount     = m_probeTest;
	m_grabRemain    = m_grabCount;
	m_grabPresent   = 0;
	m_canRemain     = m_can[m_select].m_volume;
	m_timeRemain    = 0;
	m_timeStart     = rTickCount::UnixTime();
	m_timerInterval = rTickCount::SysTick();

	rEventManager::instance().Add(ReinitEvent(EID_SAMPLER_START_TEST));
}


void rSampler::onPause(void)
{
	m_resumeState = m_state;
	m_state       = State::PAUSE;
}


void rSampler::onResume(void)
{
	//TODO нужно пересчитать значения
	m_state       = m_resumeState;
	m_resumeState = State::IDLE;
}


void rSampler::onWorkTimer(bool checkflow)
{
	switch(m_command) {
		case Command::NONE: break;
		case Command::STOP:
			m_state = State::FINISH;
			rEventManager::instance().Add(ReinitEvent(EID_SAMPLER_STOP));
			return;

		case Command::PAUSE:
			onPause();
			return;

		default:
			rEventManager::instance().Add(ReinitEvent(EID_SAMPLER_DONT_STOP));
			break;
	}

	//TODO как правильно уменьшать m_timeRemain в случае если нет расхода,
	//     нужно ли уменьшать постоянно, или допустимо скачками

	auto tick = rTickCount::SysTick();

	if (checkflow) {
		//if (m_totals->Raw.Volume - m_lastRawTotal.Volume < 0.00001) {
		if (m_totals->Inc.Volume < 0.00001) {
			m_noflow = true;
		} else {
			if (m_noflow) {
				recalcInterval();
				m_noflow = false;
			}
		}
	}

	// Нет расхода - выходим
	if (checkflow && m_noflow) {
		return;
	}

	if (tick >= m_timerInterval + m_interval) {
		m_grab.Value     = true;
		m_timeRemain    -= static_cast<UDINT>(m_interval);
		m_canPresent    += m_grabVol;
		m_canRemain     -= m_grabVol;
		m_timerInterval += static_cast<UDINT>(m_interval); // учитываем то время, что прое*али
		m_lastRawTotal   = m_totals->Raw;

		++m_grabPresent;
		--m_grabRemain;
	}

	if (rTickCount::UnixTime() > m_timeStart + m_probePeriod || m_grabRemain < 0.001 || isCanOverflow()) {
		rEventManager::instance().Add(ReinitEvent(EID_SAMPLER_FINISH));
		m_state = State::FINISH;
	}
}


void rSampler::onWorkVolume(bool isMass)
{
	switch(m_command) {
		case Command::NONE: break;
		case Command::STOP:
			m_state = State::FINISH;
			rEventManager::instance().Add(ReinitEvent(EID_SAMPLER_STOP));
			return;

		case Command::PAUSE: onPause(); return;

		default:
			rEventManager::instance().Add(ReinitEvent(EID_SAMPLER_DONT_STOP));
			break;
	}

	LREAL currvol = isMass ? m_totals->Raw.Mass  : m_totals->Raw.Volume;
	LREAL lastvol = isMass ? m_lastRawTotal.Mass : m_lastRawTotal.Volume;

	if (currvol > lastvol + m_interval) {
		m_grab.Value    = true;
		m_canPresent   += m_grabVol;
		m_canRemain    -= m_grabVol;
		m_lastRawTotal  = m_totals->Raw;

		++m_grabPresent;
		--m_grabRemain;
	}

	if (m_grabRemain < 0.001 || isCanOverflow()) {
		rEventManager::instance().Add(ReinitEvent(EID_SAMPLER_FINISH));
		m_state = State::FINISH;
	}
}


void rSampler::onWorkError()
{
	switch (m_command) {
		case Command::NONE:
			break;

		case Command::CONFIRM:
			rEventManager::instance().Add(ReinitEvent(EID_SAMPLER_CONFIRM));
			m_state = State::IDLE;
			break;

		default:
			rEventManager::instance().Add(ReinitEvent(EID_SAMPLER_COMMAND_FAULT) << static_cast<UINT>(m_command));
			break;
	}
}


//-------------------------------------------------------------------------------------------------
//
UDINT rSampler::generateVars(rVariableList& list)
{
	rSource::generateVars(list);

	// Variables
	list.add(Alias + ".method"      , TYPE_UINT , rVariable::Flags::___L, &m_method     , U_DIMLESS, ACCESS_SAMPLERS | ACCESS_SETSAMPLERS);
	list.add(Alias + ".setup"       , TYPE_UINT , rVariable::Flags::___L, &m_setup.Value, U_DIMLESS, ACCESS_SAMPLERS | ACCESS_SETSAMPLERS);
	list.add(Alias + ".select"      , TYPE_UINT , rVariable::Flags::____, &m_select     , U_DIMLESS, ACCESS_SAMPLERS | ACCESS_SETSAMPLERS);
	list.add(Alias + ".command"     , TYPE_UINT , rVariable::Flags::___L, &m_command    , U_DIMLESS, ACCESS_SAMPLERS | ACCESS_SETSAMPLERS);
	list.add(Alias + ".state"       , TYPE_UINT , rVariable::Flags::R___, &m_state      , U_DIMLESS, 0);
	list.add(Alias + ".noflow"      , TYPE_UINT , rVariable::Flags::R___, &m_noflow     , U_DIMLESS, 0);
	list.add(Alias + ".probe.period", TYPE_UDINT, rVariable::Flags::___L, &m_probePeriod, U_DIMLESS, ACCESS_SAMPLERS | ACCESS_SETSAMPLERS);
	list.add(Alias + ".probe.volume", TYPE_LREAL, rVariable::Flags::___L, &m_probeVolume, U_DIMLESS, ACCESS_SAMPLERS | ACCESS_SETSAMPLERS);
	list.add(Alias + ".probe.mass"  , TYPE_LREAL, rVariable::Flags::___L, &m_probeMass  , U_DIMLESS, ACCESS_SAMPLERS | ACCESS_SETSAMPLERS);
	list.add(Alias + ".probe.test"  , TYPE_UDINT, rVariable::Flags::___L, &m_probeTest  , U_DIMLESS, ACCESS_SAMPLERS | ACCESS_SETSAMPLERS);
	list.add(Alias + ".grab.volume" , TYPE_LREAL, rVariable::Flags::R___, &m_grabVol    , U_ml     , 0);
	list.add(Alias + ".grab.count"  , TYPE_UDINT, rVariable::Flags::R___, &m_grabCount  , U_DIMLESS, 0);
	list.add(Alias + ".grab.present", TYPE_UDINT, rVariable::Flags::R___, &m_grabPresent, U_DIMLESS, 0);
	list.add(Alias + ".grab.remain" , TYPE_UDINT, rVariable::Flags::R___, &m_grabRemain , U_DIMLESS, 0);
	list.add(Alias + ".can.volume"  , TYPE_LREAL, rVariable::Flags::____, &m_canVolume  , U_ml     , 0);
	list.add(Alias + ".can.present" , TYPE_LREAL, rVariable::Flags::R___, &m_canPresent , U_ml     , 0);
	list.add(Alias + ".can.remain"  , TYPE_LREAL, rVariable::Flags::R___, &m_canRemain  , U_ml     , 0);
	list.add(Alias + ".interval"    , TYPE_LREAL, rVariable::Flags::R___, &m_interval   , U_DIMLESS, 0);
	list.add(Alias + ".time.remain" , TYPE_UDINT, rVariable::Flags::R___, &m_timeRemain , U_msec   , 0);
	list.add(Alias + ".time.start"  , TYPE_UDINT, rVariable::Flags::R___, &m_timeStart  , U_sec    , 0);

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
	std::string strMethod = XmlUtils::getAttributeString(element, XmlName::METHOD, m_flagsMethod.getNameByBits (static_cast<UINT>(Method::PERIOD)));
	std::string strSetup  = XmlUtils::getAttributeString(element, XmlName::SETUP , m_flagsSetup.getNameByBits(Setup::OFF));

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

	m_method = static_cast<Method>(m_flagsMethod.getValue(strMethod, fault));
	if (fault) {
		return err.set(DATACFGERR_SAMPLER_METHOD, element->GetLineNum(), "");
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

	m_grabVol     = XmlUtils::getTextLREAL(xml_grabvol , m_grabVol    , fault);
	m_probeTest   = XmlUtils::getTextUINT (xml_grabtest, m_probeTest  , fault);
	m_probePeriod = XmlUtils::getTextUDINT(xml_period  , m_probePeriod, fault);

	m_ioStart.Limit.m_setup.Init(rLimit::Setup::OFF);
	m_ioStop.Limit.m_setup.Init(rLimit::Setup::OFF);
	m_grab.Limit.m_setup.Init(rLimit::Setup::OFF);
	m_selected.Limit.m_setup.Init(rLimit::Setup::OFF);

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


UDINT rSampler::generateMarkDown(rGeneratorMD& md)
{
	m_ioStart.Limit.m_setup.Init(rLimit::Setup::OFF);
	m_ioStop.Limit.m_setup.Init(rLimit::Setup::OFF);
	m_grab.Limit.m_setup.Init(rLimit::Setup::OFF);
	m_selected.Limit.m_setup.Init(rLimit::Setup::OFF);

	for (UDINT ii = 0; ii < CAN_MAX; ++ii) {
		m_can[ii].m_overflow.Limit.m_setup.Init(rLimit::Setup::OFF);
		m_can[ii].m_fault.Limit.m_setup.Init(rLimit::Setup::OFF);
		m_can[ii].m_weight.Limit.m_setup.Init(rLimit::Setup::OFF);
	}

	md.add(this, false)
			.addProperty(XmlName::METHOD, &m_flagsMethod)
			.addProperty(XmlName::SETUP, &m_flagsSetup)
			.addXml(XmlName::TOTALS   , "object containing totals")
			.addXml(XmlName::RESERVE  , "sampler object", true)
			.addXml(XmlName::IOSTART  , "<link alias=\"object's output\"/>", true)
			.addXml(XmlName::IOSTOP   , "<link alias=\"object's output\"/>", true)
			.addXml(XmlName::GRABVOL  , m_grabVol, true)
			.addXml(XmlName::PERIOD   , m_probePeriod, true)
			.addXml(XmlName::GRABTEST , m_probeTest, true)
			.addXml(String_format("<%s>", XmlName::CAN1))
			.addXml(XmlName::OVERFLOW_, "<link alias=\"object's output\"/>", true, "\t")
			.addXml(XmlName::FAULT    , "<link alias=\"object's output\"/>", true, "\t")
			.addXml(XmlName::WEIGHT   , "<link alias=\"object's output\"/>", true, "\t")
			.addXml(XmlName::VOLUME   , m_can[0].m_volume, true, "\t")
			.addXml(String_format("</%s>", XmlName::CAN1))
			.addXml(String_format("<%s>", XmlName::CAN2))
			.addXml(XmlName::OVERFLOW_, "<link alias=\"object's output\"/>", true, "\t")
			.addXml(XmlName::FAULT    , "<link alias=\"object's output\"/>", true, "\t")
			.addXml(XmlName::WEIGHT   , "<link alias=\"object's output\"/>", true, "\t")
			.addXml(XmlName::VOLUME   , m_can[1].m_volume, true, "\t")
			.addXml(String_format("</%s>", XmlName::CAN2));

	return TRITONN_RESULT_OK;
}


bool rSampler::checkInterval(void)
{
	if (m_interval > 1000) {
		return true;
	}

	rEventManager::instance().Add(ReinitEvent(EID_SAMPLER_START_FAULT));
	return false;
}


void rSampler::recalcInterval(void)
{
	m_interval = m_timeRemain / m_grabRemain;

	if (!checkInterval()) {
		m_state = State::IDLE;
	}
}


bool rSampler::isCanOverflow(void)
{
	return m_select < CAN_MAX && m_can[m_select].m_overflow.isValid() && m_can[m_select].m_overflow.Value > 0;
}


bool rSampler::isCanFault(void)
{
	return m_select < CAN_MAX && m_can[m_select].m_fault.isValid() && m_can[m_select].m_fault.Value > 0;
}


bool rSampler::checkIO(void)
{
	if (m_ioStop.isValid()){
		if (m_ioStop.Value > 0 && !(LockErr & LE_IO_STOP)) {
			LockErr |= LE_IO_STOP;

			if (!(LockErr & LE_IO_START)) {
				onStop();
				return true;
			}
		}

		if (static_cast<DINT>(m_ioStop.Value) == 0 && (LockErr & LE_IO_STOP))
		{
			LockErr &= ~LE_IO_STOP;
		}

	}

	if (m_ioStart.isValid()) {
		if (m_ioStart.Value > 0 && !(LockErr & LE_IO_START)) {
			LockErr |= LE_IO_START;

			if (!(LockErr & LE_IO_STOP)) {
				onStart();
				return true;
			}
		}

		if (static_cast<DINT>(m_ioStart.Value) == 0 && (LockErr & LE_IO_START)) {
			LockErr &= ~LE_IO_START;
		}
	}

	return false;
}
