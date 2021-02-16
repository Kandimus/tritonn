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
	InitLink(rLink::Setup::INPUT , m_temp  , U_C       , SID::TEMPERATURE, XmlName::TEMP   , rLink::SHADOW_NONE);
	InitLink(rLink::Setup::INPUT , m_pres  , U_MPa     , SID::PRESSURE   , XmlName::PRES   , rLink::SHADOW_NONE);
	InitLink(rLink::Setup::INPUT , m_dens  , U_kg_m3   , SID::DENSITY    , XmlName::DENSITY, rLink::SHADOW_NONE);
	InitLink(rLink::Setup::INPUT , m_opened, U_discrete, SID::OPENED     , XmlName::OPENED , rLink::SHADOW_NONE);
	InitLink(rLink::Setup::INPUT , m_closed, U_discrete, SID::CLOSED     , XmlName::CLOSED , rLink::SHADOW_NONE);
	InitLink(rLink::Setup::OUTPUT, m_open  , U_discrete, SID::OPEN       , XmlName::OPEN   , rLink::SHADOW_NONE);
	InitLink(rLink::Setup::OUTPUT, m_close , U_discrete, SID::CLOSE      , XmlName::CLOSE  , rLink::SHADOW_NONE);
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

		m_moduleName      = module->getAlias();
		m_moduleFreq      = module->getFreq();
		m_moduleDetectors = module->getDetectors();
	}

	//TODO Сообщения от детекторов

	switch(m_command) {
		case Command::NONE:
		case Command::START:
		case Command::ABORT:
		case Command::RESET: break;

		default:
			rEventManager::instance().Add(ReinitEvent(EID_PROVE_BADCOMMAND) << static_cast<UINT>(m_command));
			break;
	}

	switch(m_state) {
		case State::IDLE:          onIdle();          break;
		case State::START:         onStart();         break;
		case State::STABILIZATION: onStabilization(); break;
		case State::VALVETOUP:     onValveToUp();     break;
		case State::WAITTOUP:      onWaitToUp();      break;
		case State::VALVETODOWN:   onValveToDown();   break;
		case State::WAITD1:        onWaitD1();        break;
		case State::WAITD2:        onWaitD2();        break;
		case State::CALCULATE:     onCalсulate();     break;
		case State::RETURNBALL:    onReturnBall();    break;

		case State::FINISH:
		case State::ABORT:         onFinish();        break;

		case State::ERRORFLOW:
		case State::ERRORSTAB:
		case State::ERRORTOUP:
		case State::ERRORTODOWN:
		case State::ERRORD1:
		case State::ERRORD2:
		case State::ERRORDETECTOR:
		case State::ERRORRETURN:
			onErrorState(); break;
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
			rEventManager::instance().Add(ReinitEvent(EID_PROVE_COMMANDSTART));
			break;

		default: break;
	}
}

void rProve::onStart()
{
	if (checkCommand()) {
		return;
	}

	if (!m_timer.isStarted()) {
		m_prvFreq = 0;
		m_prvDens = 0;
		m_prvPres = 0;
		m_prvTemp = 0;
		m_strDens = 0;
		m_strPres = 0;
		m_strTemp = 0;
		m_timer.start(m_timerStart);
		//TODO Переключить линию
		return;
	}

	if (m_timer.isFinished()) {
		m_timer.stop();

		if (m_moduleFreq < 0.001) {
			rEventManager::instance().Add(ReinitEvent(EID_PROVE_NOFLOW));
			m_state = State::ERRORFLOW;
			return;
		}

		if (m_setup.Value & Setup::STABILIZATION) {
			m_state = State::STABILIZATION;
			return;
		}

		m_state = State::VALVETOUP;
	}
}

void rProve::onStabilization()
{
	if (checkCommand()) {
		return;
	}

	if (!m_timer.isStarted()) {
		m_stabTemp = m_temp.Value;
		m_stabPres = m_pres.Value;
		m_stabDens = m_dens.Value;

		m_timer.start(m_timerStab);
		return;
	}

	if (m_timer.isFinished()) {
		m_timer.stop();
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
	if (checkCommand()) {
		return;
	}

	if (m_opened.Value > 0 && m_closed.Value == 0) {
		m_state = State::VALVETODOWN;
		return;
	}

	m_open.Value = 1;
	m_state = State::WAITTOUP;
}

void rProve::onWaitToUp()
{
	if (checkCommand()) {
		return;
	}

	if (!m_timer.isStarted()) {
		m_timer.start(2.0 * (m_timerD1 + m_timerD2 + m_timerVolume));
		rEventManager::instance().Add(ReinitEvent(EID_PROVE_WAITTOUP));
		return;
	}

	if (m_timer.isFinished()) {
		m_timer.stop();

		if (m_opened.Value > 0 && m_closed.Value == 0) {
			m_state = State::VALVETODOWN;
			return;
		}

		rEventManager::instance().Add(ReinitEvent(EID_PROVE_ERRORTOUP));
		m_state = State::ERRORTOUP;
	}
}

void rProve::onValveToDown()
{
	if (checkCommand()) {
		return;
	}

	if (!m_timer.isStarted()) {
		moduleStart();

		m_close.Value = 1;
		m_timer.start(m_timerValve);
		return;
	}

	if(m_timer.isFinished()) {
		m_timer.stop();
		moduleStop();

		rEventManager::instance().Add(ReinitEvent(EID_PROVE_ERRORTDOWN));
		m_state = State::ERRORTODOWN;
		return;
	}

	if (m_closed.Value > 0 && m_opened.Value == 0) {
		m_state = State::WAITD1;
		m_timer.stop();
		return;
	}
}

void rProve::onWaitD1()
{
	if (checkCommand()) {
		return;
	}

	if (!m_timer.isStarted()) {
		m_timer.start(m_timerD1);
		return;
	}

	if (m_timer.isFinished()) {
		moduleStop();
		m_timer.stop();
		m_state = State::ERRORD1;
		rEventManager::instance().Add(ReinitEvent(EID_PROVE_ERRORD1));
	}

	if (m_moduleDetectors & (rModuleCRM::Detector::Det3 | rModuleCRM::Detector::Det4)) {
		m_timer.stop();
		m_state = State::ERRORDETECTOR;
		rEventManager::instance().Add(ReinitEvent(EID_PROVE_ERRORDETECTORS) << m_moduleDetectors);
	}

	if (m_moduleDetectors & (rModuleCRM::Detector::Det1 | rModuleCRM::Detector::Det2)) {
		m_timer.stop();
		m_state = State::WAITD2;
		return;
	}
}

void rProve::onWaitD2()
{
	if (checkCommand()) {
		return;
	}

	if (!m_timer.isStarted()) {
		m_timer.start(m_timerVolume);
		return;
	}

	if (m_timer.isFinished()) {
		moduleStop();
		m_timer.stop();
		m_state = State::ERRORD2;
		rEventManager::instance().Add(ReinitEvent(EID_PROVE_ERRORD2));
	}

	if (m_moduleDetectors & (rModuleCRM::Detector::Det1 | rModuleCRM::Detector::Det2)) {
		m_timer.stop();
		m_state = State::ERRORDETECTOR;
		rEventManager::instance().Add(ReinitEvent(EID_PROVE_ERRORDETECTORS) << m_moduleDetectors);
	}

	if (m_moduleDetectors & (rModuleCRM::Detector::Det3 | rModuleCRM::Detector::Det4)) {
		m_timer.stop();
		m_state = State::CALCULATE;
		return;
	}
}

void rProve::onCalсulate()
{
	if (checkCommand()) {
		return;
	}

	if (!m_timer.isStarted()) {
		m_timer.start(m_timerD2);
		//TODO Получить данные с модуля
		return;
	}

	if (m_timer.isFinished()) {
		m_timer.stop();
		m_state = State::RETURNBALL;
		return;
	}
}

void rProve::onReturnBall()
{
	if (checkCommand()) {
		return;
	}

	if (!m_timer.isStarted()) {
		m_open.Value = 1;
		m_timer.start(m_timerValve);
//!		rEventManager::instance().Add(ReinitEvent(EID_PROVE_WAITTOUP));
		return;
	}

	if (m_timer.isFinished()) {
		m_timer.stop();
		m_state = State::ERRORRETURN;
		return;
	}

	if (m_opened.Value > 0 && m_closed.Value == 0) {
		m_timer.stop();
		m_state = State::FINISH;
		return;
	}
}

void rProve::onFinish()
{
	if (!m_timer.isStarted()) {
		m_timer.start(m_timerFinish);
		return;
	}

	if (m_timer.isFinished()) {
		m_timer.stop();
		m_state = State::IDLE;
		return;
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

bool rProve::checkCommand()
{
	switch(m_command) {
		case Command::ABORT:
			m_state = State::IDLE;
			rEventManager::instance().Add(ReinitEvent(EID_PROVE_COMMANDABORT));
			return true;

		default: break;
	}

	return false;
}


UDINT rProve::generateVars(rVariableList& list)
{
	rSource::generateVars(list);

	// Variables
	list.add(Alias + ".command"                  , TYPE_UINT , rVariable::Flags::____, &m_command       , U_DIMLESS, ACCESS_PROVE);
	list.add(Alias + ".setup"                    , TYPE_UINT , rVariable::Flags::____, &m_setup.Value   , U_DIMLESS, ACCESS_PROVE);
	list.add(Alias + ".state"                    , TYPE_UINT , rVariable::Flags::R___, &m_state         , U_DIMLESS, 0);
	list.add(Alias + ".timer.start"              , TYPE_UDINT, rVariable::Flags::___L, &m_timerStart    , U_msec   , ACCESS_PROVE);
	list.add(Alias + ".timer.stabilization"      , TYPE_UDINT, rVariable::Flags::___L, &m_timerStab     , U_msec   , ACCESS_PROVE);
	list.add(Alias + ".timer.detector1"          , TYPE_UDINT, rVariable::Flags::___L, &m_timerD1       , U_msec   , ACCESS_PROVE);
	list.add(Alias + ".timer.detector2"          , TYPE_UDINT, rVariable::Flags::___L, &m_timerD2       , U_msec   , ACCESS_PROVE);
	list.add(Alias + ".timer.volume"             , TYPE_UDINT, rVariable::Flags::___L, &m_timerVolume   , U_msec   , ACCESS_PROVE);
	list.add(Alias + ".timer.valve"              , TYPE_UDINT, rVariable::Flags::___L, &m_timerValve    , U_msec   , ACCESS_PROVE);
	list.add(Alias + ".result.volume1.count"     , TYPE_LREAL, rVariable::Flags::R___, &m_prvCount[0]   , U_imp    , 0);
	list.add(Alias + ".result.volume1.time"      , TYPE_LREAL, rVariable::Flags::R___, &m_prvTime[0]    , U_sec    , 0);
	list.add(Alias + ".result.volume2.count"     , TYPE_LREAL, rVariable::Flags::R___, &m_prvCount[1]   , U_imp    , 0);
	list.add(Alias + ".result.volume2.time"      , TYPE_LREAL, rVariable::Flags::R___, &m_prvTime[1]    , U_sec    , 0);
	list.add(Alias + ".result.prove.frequency"   , TYPE_LREAL, rVariable::Flags::R___, &m_prvFreq       , U_Hz     , 0);
	list.add(Alias + ".result.prove.temperature" , TYPE_LREAL, rVariable::Flags::R___, &m_prvTemp       , U_C      , 0);
	list.add(Alias + ".result.prove.pressure"    , TYPE_LREAL, rVariable::Flags::R___, &m_prvPres       , U_MPa    , 0);
	list.add(Alias + ".result.prove.density"     , TYPE_LREAL, rVariable::Flags::R___, &m_prvDens       , U_kg_m3  , 0);
	list.add(Alias + ".result.stream.temperature", TYPE_LREAL, rVariable::Flags::R___, &m_strTemp       , U_C      , 0);
	list.add(Alias + ".result.stream.pressure"   , TYPE_LREAL, rVariable::Flags::R___, &m_strPres       , U_MPa    , 0);
	list.add(Alias + ".result.stream.density"    , TYPE_LREAL, rVariable::Flags::R___, &m_strDens       , U_kg_m3  , 0);

	list.add(Alias + ".fault"     , TYPE_UDINT, rVariable::Flags::R___, &Fault            , U_DIMLESS     , 0);

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rProve::LoadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix)
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


std::string rProve::saveKernel(UDINT isio, const std::string& objname, const std::string& comment, UDINT isglobal)
{
	m_present.Limit.m_setup.Init(rLimit::Setup::NONE);
	PhValue.Limit.m_setup.Init(rLimit::Setup::NONE);
	Current.Limit.m_setup.Init(rLimit::Setup::NONE);

	return rSource::saveKernel(isio, objname, comment, isglobal);
}

void rProve::moduleStart()
{
	rSnapshot ss(rDataManager::instance().getVariableClass(), 0xFFFFFFFF);

	ss.add(m_moduleName + ".start", 1);
	ss.set();
}

void rProve::moduleStop()
{
	rSnapshot ss(rDataManager::instance().getVariableClass(), 0xFFFFFFFF);

	ss.add(m_moduleName + ".stop", 1);
	ss.set();
}



