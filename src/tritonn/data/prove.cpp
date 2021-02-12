//=================================================================================================
//===
//=== prove.cpp
//===
//=== Copyright (c) 2021 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс поверки
//===
//=================================================================================================

#include <vector>
#include <limits>
#include <cmath>
#include <string.h>
#include "tinyxml2.h"
#include "event_eid.h"
#include "text_id.h"
#include "../event_manager.h"
#include "../data_manager.h"
#include "../data_ai.h"
#include "../error.h"
#include "../variable_item.h"
#include "../variable_list.h"
#include "../io/manager.h"
#include "../io/module_crm.h"
#include "xml_util.h"
#include "prove.h"

rBitsArray rProve::m_flagsSetup;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
rProve::rProve(const rStation* owner)
	: rSource(owner), rDataModule(true), m_setup(static_cast<UINT>(Setup::NONE))
{
	if (m_flagsSetup.empty()) {
		m_flagsSetup
				.add("NONE"         , static_cast<UINT>(Setup::NONE))
				.add("4WAY"         , static_cast<UINT>(Setup::VALVE_4WAY))
				.add("STABILIZATION", static_cast<UINT>(Setup::STABILIZATION))
				.add("NOVALVE"      , static_cast<UINT>(Setup::NOVALVE));
	}


	//NOTE Единицы измерения добавим после загрузки сигнала
	InitLink(rLink::Setup::INPUT, m_temp  , U_C       , SID::TEMPERATURE, XmlName::TEMP   , rLink::SHADOW_NONE);
	InitLink(rLink::Setup::INPUT, m_pres  , U_MPa     , SID::PRESSURE   , XmlName::PRES   , rLink::SHADOW_NONE);
	InitLink(rLink::Setup::INPUT, m_dens  , U_kg_m3   , SID::DENSITY    , XmlName::DENSITY, rLink::SHADOW_NONE);
	InitLink(rLink::Setup::INPUT, m_open  , U_discrete, SID::OPEN       , XmlName::OPEN   , rLink::SHADOW_NONE);
	InitLink(rLink::Setup::INPUT, m_close , U_discrete, SID::CLOSE      , XmlName::CLOSE  , rLink::SHADOW_NONE);
	InitLink(rLink::Setup::INPUT, m_opened, U_discrete, SID::OPENED     , XmlName::OPENED , rLink::SHADOW_NONE);
	InitLink(rLink::Setup::INPUT, m_closed, U_discrete, SID::CLOSED     , XmlName::CLOSED , rLink::SHADOW_NONE);
}


//-------------------------------------------------------------------------------------------------
//
UDINT rProve::InitLimitEvent(rLink &link)
{
	link.Limit.EventChangeAMin  = ReinitEvent(EID_AI_NEW_AMIN)  << link.Descr << link.Unit;
	link.Limit.EventChangeWMin  = ReinitEvent(EID_AI_NEW_WMIN)  << link.Descr << link.Unit;
	link.Limit.EventChangeWMax  = ReinitEvent(EID_AI_NEW_WMAX)  << link.Descr << link.Unit;
	link.Limit.EventChangeAMax  = ReinitEvent(EID_AI_NEW_AMAX)  << link.Descr << link.Unit;
	link.Limit.EventChangeHyst  = ReinitEvent(EID_AI_NEW_HYST)  << link.Descr << link.Unit;
	link.Limit.EventChangeSetup = ReinitEvent(EID_AI_NEW_SETUP) << link.Descr << link.Unit;
	link.Limit.EventAMin        = ReinitEvent(EID_AI_AMIN)      << link.Descr << link.Unit;
	link.Limit.EventWMin        = ReinitEvent(EID_AI_WMIN)      << link.Descr << link.Unit;
	link.Limit.EventWMax        = ReinitEvent(EID_AI_WMAX)      << link.Descr << link.Unit;
	link.Limit.EventAMax        = ReinitEvent(EID_AI_AMAX)      << link.Descr << link.Unit;
	link.Limit.EventNan         = ReinitEvent(EID_AI_NAN)       << link.Descr << link.Unit;
	link.Limit.EventNormal      = ReinitEvent(EID_AI_NORMAL)    << link.Descr << link.Unit;

	return TRITONN_RESULT_OK;
}

//-------------------------------------------------------------------------------------------------
//
UDINT rProve::Calculate()
{
	rEvent event_success;
	rEvent event_fault;
	
	if(rSource::Calculate()) return TRITONN_RESULT_OK;

	if (isSetModule()) {
		auto module_ptr = rIOManager::instance().getModule(m_module);
		auto module     = dynamic_cast<rModuleCRM*>(module_ptr.get());

		if (!module) {
			rEventManager::instance().Add(ReinitEvent(EID_PROVE_MODULE) << m_module);
			rDataManager::instance().DoHalt(HALT_REASON_RUNTIME | DATACFGERR_REALTIME_MODULELINK);
			return DATACFGERR_REALTIME_MODULELINK;
		}

		m_curFreq      = module->getFreq();
		m_curDetectors = module->getDetectors();
	}

	switch(m_command) {
		case Command::NONE:
		case Command::START:
		case Command::STOP:
		case Command::ABORT:
		case Command::RESET: break;

		default:
			rEventManager::instance().Add(ReinitEvent(EID_PROVE_BADCOMMAND) << static_cast<UINT>(m_command));
			break;
	}

	switch(m_state) {
		case State::IDLE: onIdle(); break;
		case State::START: onStart(); break;
		case State::STABILIZATION: onStabilization(); break;
		case State::VALVETOUP: onValveToUp(); break;
		case State::WAITTOUP: onWaitToUp(); break;

		case State::NOFLOW:
		case State::ERRORSTAB:
		case State::ERRORTOUP: onErrorState(); break;
	};

	m_command = Command::NONE;

	PostCalculate();
		
	return TRITONN_RESULT_OK;
}

void rProve::onIdle()
{
	switch(m_command) {
		case Command::START:
			m_state = State::START;
			break;

		default: break;
	}
}

void rProve::onStart()
{
	switch(m_command) {
		case Command::STOP:
		case Command::ABORT:
			m_state = State::IDLE;
			break;

		default: break;
	}

	if (!m_timerStart.isStarted()) {
		m_inDens  = 0;
		m_inPres  = 0;
		m_inTemp  = 0;
		m_strDens = 0;
		m_strPres = 0;
		m_strTemp = 0;
		m_timerStart.start(m_tsStart);
		//TODO Переключить линию
		return;
	}

	if (!m_timerStart.isFinished()) {
		return;
	}

	m_timerStart.stop();

	if (m_curFreq) {
		rEventManager::instance().Add(ReinitEvent(EID_PROVE_NOFLOW));
		m_state = State::NOFLOW;
		return;
	}

	if (m_setup.Value & Setup::STABILIZATION) {
		m_state = State::STABILIZATION;
		return;
	}

	m_state = State::VALVETOUP;
}

void rProve::onStabilization()
{
	if (!m_timerStab.isStarted()) {
		m_stabTemp = m_temp.Value;
		m_stabPres = m_pres.Value;
		m_stabDens = m_dens.Value;

		m_timerStab.start(m_tsStab);
		return;
	}

	if (m_timerStab.isFinished()) {
		m_timerStab.stop();
		m_state = State::VALVETOUP;
	}

	LREAL delta = 0;

	delta = std::fabs(m_temp.Value - m_stabTemp);
	if (delta > m_maxStabTemp) {
		rEventManager::instance().Add(ReinitEvent(EID_PROVE_STABTEMP) << delta << m_maxStabTemp);
		m_state = State::ERRORSTAB;
	}

	delta = std::fabs(m_pres.Value - m_stabPres);
	if (delta > m_maxStabPres) {
		rEventManager::instance().Add(ReinitEvent(EID_PROVE_STABPRES) << delta << m_maxStabPres);
		m_state = State::ERRORSTAB;
	}

	delta = std::fabs(m_dens.Value - m_stabDens);
	if (delta > m_maxStabDens) {
		rEventManager::instance().Add(ReinitEvent(EID_PROVE_STABDENS) << delta << m_maxStabDens);
		m_state = State::ERRORSTAB;
	}
}

void rProve::onValveToUp()
{
	if (m_opened.Value > 0 && m_closed.Value == 0) {
		m_state = State::VALVETODOWN;
		return;
	}

	m_open.Value = 1;
	m_state = State::WAITTOUP;
	rEventManager::instance().Add(ReinitEvent(EID_PROVE_WAITTOUP));
}

void rProve::onWaitToUp()
{
	if (!m_timerWaitUp.isStarted()) {
		m_timerWaitUp.start(2.0 * (m_tsD1 + m_tsD2 + m_tsV));
		return;
	}

	if (m_timerWaitUp.isFinished()) {
		m_timerWaitUp.stop();

		if (m_opened.Value > 0 && m_closed.Value == 0) {
			m_state = State::VALVETODOWN;
			return;
		}

		rEventManager::instance().Add(ReinitEvent(EID_PROVE_ERRORTOUP));
		m_state = State::ERRORTOUP;
	}
}


void rProve::onErrorState()
{
	switch(m_command) {
		case Command::RESET:
			m_state = State::IDLE;
			break;

		default: break;
	}
}


UDINT rProve::generateVars(rVariableList& list)
{
	rSource::generateVars(list);

	// Variables
	list.add(Alias + ".command"                   , TYPE_UINT , rVariable::Flags::____, &m_command    , U_DIMLESS, ACCESS_PROVE);
	list.add(Alias + ".setup"                     , TYPE_UINT , rVariable::Flags::____, &m_setup.Value, U_DIMLESS, ACCESS_PROVE);
	list.add(Alias + ".state"                     , TYPE_UINT , rVariable::Flags::R___, &m_state      , U_DIMLESS, 0);
	list.add(Alias + ".average.prove.temperature" , TYPE_LREAL, rVariable::Flags::R__L, &m_inTemp     , U_C      , 0);
	list.add(Alias + ".average.prove.pressure"    , TYPE_LREAL, rVariable::Flags::R__L, &m_inPres     , U_MPa    , 0);
	list.add(Alias + ".average.prove.density"     , TYPE_LREAL, rVariable::Flags::R__L, &m_inDens     , U_kg_m3  , 0);
	list.add(Alias + ".average.stream.temperature", TYPE_LREAL, rVariable::Flags::R__L, &m_stnTemp    , U_C      , 0);
	list.add(Alias + ".average.stream.pressure"   , TYPE_LREAL, rVariable::Flags::R__L, &m_stnPres    , U_MPa    , 0);
	list.add(Alias + ".average.stream.density"    , TYPE_LREAL, rVariable::Flags::R__L, &m_stnDens    , U_kg_m3  , 0);
	list.add(Alias + ".timer.stabilization", TYPE_UDINT, rVariable::Flags::___L, &m_timerStab  , U_sec    , ACCESS_PROVE);

	list.add(Alias + ".fault"     , TYPE_UDINT, rVariable::Flags::R___, &Fault            , U_DIMLESS     , 0);

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rAI::LoadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix)
{
	std::string strMode  = XmlUtils::getAttributeString(element, XmlName::MODE , m_flagsMode.getNameByBits (static_cast<UINT>(Mode::PHIS)));
	std::string strSetup = XmlUtils::getAttributeString(element, XmlName::SETUP, m_flagsSetup.getNameByBits(Setup::OFF));

	if (rSource::LoadFromXML(element, err, prefix) != TRITONN_RESULT_OK) {
		return err.getError();
	}

	tinyxml2::XMLElement* xml_module = element->FirstChildElement(XmlName::IOLINK);
	tinyxml2::XMLElement* xml_limits = element->FirstChildElement(XmlName::LIMITS); // Limits считываем только для проверки
	tinyxml2::XMLElement* xml_unit   = element->FirstChildElement(XmlName::UNIT);
	tinyxml2::XMLElement* xml_scale  = element->FirstChildElement(XmlName::SCALE);

	// Если аналоговый сигнал не привязан к каналу, то разрешаем менять его значение
	if (xml_module) {
		if (rDataModule::loadFromXML(xml_module, err) != TRITONN_RESULT_OK) {
			return err.getError();
		}
	} else {
		m_present.m_setup |= rLink::Setup::WRITABLE;
	}

	if (!xml_limits || !xml_unit || !xml_scale) {
		return err.set(DATACFGERR_AI, element->GetLineNum(), "cant found limits or unit or scale");
	}

	UDINT fault = 0;
	m_mode = static_cast<Mode>(m_flagsMode.getValue(strMode, fault));

	m_setup.Init(m_flagsSetup.getValue(strSetup, fault));

	KeypadValue.Init(XmlUtils::getTextLREAL(element->FirstChildElement(XmlName::KEYPAD) , 0.0, fault));
	m_scale.Min.Init(XmlUtils::getTextLREAL(xml_scale->FirstChildElement  (XmlName::MIN), 0.0, fault));
	m_scale.Max.Init(XmlUtils::getTextLREAL(xml_scale->FirstChildElement  (XmlName::MAX), 0.0, fault));

	STRID Unit = XmlUtils::getTextUDINT(element->FirstChildElement(XmlName::UNIT), U_any, fault);

	if (fault) {
		return err.set(DATACFGERR_AI, element->GetLineNum(), "");
	}

	// Подправляем единицы измерения, исходя из конфигурации AI
	m_present.Unit = Unit;
	PhValue.Unit   = Unit;

	ReinitLimitEvents();

	return TRITONN_RESULT_OK;
}


std::string rAI::saveKernel(UDINT isio, const std::string& objname, const std::string& comment, UDINT isglobal)
{
	m_present.Limit.m_setup.Init(rLimit::Setup::NONE);
	PhValue.Limit.m_setup.Init(rLimit::Setup::NONE);
	Current.Limit.m_setup.Init(rLimit::Setup::NONE);

	return rSource::saveKernel(isio, objname, comment, isglobal);
}

bool rProve::checkStab(const rAI& ai, LREAL val)
{
	return
}





