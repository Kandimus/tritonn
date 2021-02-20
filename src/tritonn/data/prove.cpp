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
#include "../text_id.h"
#include "../event_manager.h"
#include "../data_manager.h"
#include "../data_ai.h"
#include "../error.h"
#include "../variable_item.h"
#include "../variable_list.h"
#include "../io/manager.h"
#include "../io/module_crm.h"
#include "../xml_util.h"
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
				.add("NOVALVE"      , static_cast<UINT>(Setup::NOVALVE))
				.add("ONEDETECTOR"  , static_cast<UINT>(Setup::ONEDETECTOR))
				.add("BOUNCE"       , static_cast<UINT>(Setup::BOUNCE))
				.add("SIMULATE"     , static_cast<UINT>(Setup::SIMULATE));
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

		m_moduleName  = module->getAlias();
		m_moduleFreq  = module->getFreq();
		m_moduleDet   = module->getDetectors();
		m_moduleCount = module->getCounter();
	}

	detectorsProcessing();
	calcAverage();

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
		case State::IDLE:
		case State::FINISH:        onIdle();          break;

		case State::START:         onStart();         break;
		case State::STABILIZATION: onStabilization(); break;
		case State::VALVETOUP:     onValveToUp();     break;
		case State::WAITTOUP:      onWaitToUp();      break;
		case State::VALVETODOWN:   onValveToDown();   break;
		case State::WAITD1:        onWaitD1();        break;
		case State::WAITD2:        onWaitD2();        break;
		case State::CALCULATE:     onCalculate();     break;
		case State::RETURNBALL:    onReturnBall();    break;

		case State::ABORT:         onAbort();         break;

		case State::ERRORFLOW:
		case State::ERRORSTAB:
		case State::ERRORTOUP:
		case State::ERRORTODOWN:
		case State::ERRORD1:
		case State::ERRORD2:
		case State::ERRORDETECTOR:
		case State::ERRORRETURN:   onErrorState();    break;
	};

	m_command = Command::NONE;

	PostCalculate();
		
	return TRITONN_RESULT_OK;
}

void rProve::onIdle()
{
	switch(m_command) {
		case Command::START:
			rEventManager::instance().Add(ReinitEvent(EID_PROVE_COMMANDSTART));
			setState(State::START);
			break;

		case Command::ABORT:
			rEventManager::instance().Add(ReinitEvent(EID_PROVE_NOTSTARTED));
			break;

		case Command::RESET:
			rEventManager::instance().Add(ReinitEvent(EID_PROVE_COMMANDRESET));
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
		clearAverage();
		connectToLine();
		m_timer.start(m_tStart);
		return;
	}

	if (m_timer.isFinished()) {
		m_timer.stop();

		if (m_moduleFreq < 0.001) {
			setState(State::ERRORFLOW);
			return;
		}

		if (m_setup & Setup::STABILIZATION) {
			setState(State::STABILIZATION);
			return;
		}

		setState(State::VALVETOUP);
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
		m_stabFreq = m_moduleFreq;

		m_timer.start(m_tStab);
		return;
	}

	if (m_timer.isFinished()) {
		m_timer.stop();
		setState(State::VALVETOUP);
		return;
	}

	if (checkStab(m_stabTemp, m_temp.Value, m_maxStabTemp, EID_PROVE_ERRORSTABTEMP)) {
		return;
	}

	if (checkStab(m_stabPres, m_pres.Value, m_maxStabPres, EID_PROVE_ERRORSTABPRES)) {
		return;
	}

	if (checkStab(m_stabDens, m_dens.Value, m_maxStabDens, EID_PROVE_ERRORSTABDENS)) {
		return;
	}

	if (checkStab(m_stabFreq, m_moduleFreq, m_maxStabFreq, EID_PROVE_ERRORSTABFREQ)) {
		return;
	}
}


void rProve::onValveToUp()
{
	if (checkCommand()) {
		return;
	}

	if (m_opened.Value > 0 && m_closed.Value == 0) {
		setState(State::VALVETODOWN);
		return;
	}

	m_close.Value = 0;
	m_open.Value  = 1;
	setState(State::WAITTOUP);
}


void rProve::onWaitToUp()
{
	if (checkCommand()) {
		return;
	}

	if (!m_timer.isStarted()) {
		m_timer.start(2.0 * (m_tD1 + m_tD2 + m_tVolume));
		return;
	}

	if (m_timer.isFinished()) {
		m_timer.stop();

		if (m_opened.Value > 0 && m_closed.Value == 0) {
			setState(State::VALVETODOWN);
			return;
		}

		setState(State::ERRORTOUP);
		return;
	}
}


void rProve::onValveToDown()
{
	if (checkCommand()) {
		return;
	}

	if (!m_timer.isStarted()) {
		m_close.Value = 1;
		m_open.Value  = 0;
		m_timer.start(m_tValve);
		return;
	}

	if(m_timer.isFinished()) {
		m_timer.stop();

		setState(State::ERRORTODOWN);
		return;
	}

	if (m_closed.Value > 0 && m_opened.Value == 0) {
		m_timer.stop();
		setState(State::WAITD1);
		return;
	}
}


void rProve::onWaitD1()
{
	if (checkCommand()) {
		return;
	}

	if (!m_timer.isStarted()) {
		m_timer.start(m_tD1);
		moduleStart();
		return;
	}

	if (m_timer.isFinished()) {
		moduleStop();
		m_timer.stop();
		setState(State::ERRORD1);
		return;
	}

	DINT result = checkDetectors(true);
	if (-1 == result) {
		m_timer.stop();
		setState(State::ERRORDETECTOR);
		return;
	}

	if (result) {
		m_timer.stop();
		setState(State::WAITD2);
		return;
	}
}

void rProve::onWaitD2()
{
	if (checkCommand()) {
		return;
	}

	if (!m_timer.isStarted()) {
		m_timer.start(m_tVolume);
		return;
	}

	if (m_timer.isFinished()) {
		moduleStop();
		m_timer.stop();
		setState(State::ERRORD2);
		return;
	}

	DINT result = checkDetectors(false);
	if (-1 == result) {
		m_timer.stop();
		setState(State::ERRORDETECTOR);
		return;
	}

	if (result) {
		m_timer.stop();
		setState(State::CALCULATE);
		return;
	}
}


void rProve::onCalculate()
{
	if (checkCommand()) {
		return;
	}

	if (!m_timer.isStarted()) {
		m_timer.start(m_tD2);
		//TODO Получить данные с модуля
		return;
	}

	if (m_timer.isFinished()) {
		m_timer.stop();
		setState(State::RETURNBALL);
		return;
	}
}


void rProve::onReturnBall()
{
	if (checkCommand()) {
		return;
	}

	if (!m_timer.isStarted()) {
		m_close.Value = 0;
		m_open.Value  = 1;
		m_timer.start(m_tValve);
		return;
	}

	if (m_timer.isFinished()) {
		m_timer.stop();
		setState(State::ERRORRETURN);
		return;
	}

	if (m_opened.Value > 0 && m_closed.Value == 0) {
		m_timer.stop();
		setState(State::FINISH);
		return;
	}
}

void rProve::onAbort()
{
	if (!m_timer.isStarted()) {
		m_timer.start(m_tAbort);
		return;
	}

	if (m_timer.isFinished()) {
		m_timer.stop();
		setState(State::IDLE);
		return;
	}
}


void rProve::onErrorState()
{
	switch(m_command) {
		case Command::START:
			rEventManager::instance().Add(ReinitEvent(EID_PROVE_ISERROR));
			break;

		case Command::ABORT:
			setState(State::IDLE);
			break;

		case Command::RESET:
			m_state = State::IDLE;
			rEventManager::instance().Add(ReinitEvent(EID_PROVE_COMMANDRESET));
			break;

		default: break;
	}
}

bool rProve::checkCommand()
{
	switch(m_command) {
		case Command::START:
			rEventManager::instance().Add(ReinitEvent(EID_PROVE_ALREADYSTARTED));
			break;

		case Command::ABORT:
			rEventManager::instance().Add(ReinitEvent(EID_PROVE_COMMANDABORT));
			setState(State::IDLE);
			return true;

		case Command::RESET:
			rEventManager::instance().Add(ReinitEvent(EID_PROVE_NOTERROR));
			break;

		default: break;
	}

	return false;
}


DINT rProve::checkDetectors(bool first)
{
	if (m_setup & Setup::ONEDETECTOR) {
		return m_fixDet & (rModuleCRM::Detector::Det1 | rModuleCRM::Detector::Det2 | rModuleCRM::Detector::Det3 | rModuleCRM::Detector::Det4);
	}

	if (first) {
		if (m_fixDet & (rModuleCRM::Detector::Det3 | rModuleCRM::Detector::Det4)) {
			return -1;
		}

		return m_fixDet & (rModuleCRM::Detector::Det1 | rModuleCRM::Detector::Det2);
	}

	if (m_fixDet & (rModuleCRM::Detector::Det1 | rModuleCRM::Detector::Det2)) {
		return -1;
	}

	return m_fixDet & (rModuleCRM::Detector::Det3 | rModuleCRM::Detector::Det4);
}


bool rProve::checkStab(LREAL start, LREAL present, LREAL maxstab, UDINT eid)
{
	LREAL delta = std::fabs(present - start);

	if (delta > maxstab) {
		rEventManager::instance().Add(ReinitEvent(eid) << delta << maxstab);
		setState(State::ERRORSTAB);
		return true;
	}

	return false;
}


void rProve::clearAverage()
{
	m_fixDet  = 0;

	m_prvFreq     = 0;
	m_prvTemp     = 0;
	m_prvPres     = 0;
	m_prvDens     = 0;
	m_prvCount[0] = 0;
	m_prvCount[1] = 0;
	m_prvTime[0]  = 0;
	m_prvTime[1]  = 0;
	m_strTemp = 0;
	m_strPres = 0;
	m_strDens = 0;
}

void rProve::detectorsProcessing()
{
	if (!m_timerBounce.isStarted()) {
		if ((m_fixDet & m_moduleDet) != m_moduleDet) {
			m_fixDet |= m_moduleDet;
			m_curDet  = m_moduleDet;

			if (m_setup & Setup::BOUNCE) {
				m_timerBounce.start(m_tBounce);
			}
		}
		return;
	}

	if (m_timerBounce.isFinished()) {
		m_timerBounce.stop();
	}
}

void rProve::setState(State state)
{
	rEvent event;

	m_state = state;

	switch(m_state)
	{
		case State::IDLE:          event.Reinit(EID_PROVE_STATE_IDLE);        break;
		case State::START:         event.Reinit(EID_PROVE_STATE_START);       break;
		case State::STABILIZATION: event.Reinit(EID_PROVE_STATE_STAB);        break;
		case State::VALVETOUP:     event.Reinit(EID_PROVE_STATE_VALVETOUP);   break;
		case State::WAITTOUP:      event.Reinit(EID_PROVE_STATE_WAITTOUP);    break;
		case State::VALVETODOWN:   event.Reinit(EID_PROVE_STATE_VALVETODOWN); break;
		case State::WAITD1:        event.Reinit(EID_PROVE_STATE_WAITD1);      break;
		case State::WAITD2:        event.Reinit(EID_PROVE_STATE_WAITD2);      break;
		case State::CALCULATE:     event.Reinit(EID_PROVE_STATE_CALCULATE);   break;
		case State::RETURNBALL:    event.Reinit(EID_PROVE_STATE_RETURNBALL);  break;
		case State::FINISH:        event.Reinit(EID_PROVE_STATE_FINISH);      break;
		case State::ABORT:         event.Reinit(EID_PROVE_STATE_ABORT);       break;
		case State::ERRORFLOW:     event.Reinit(EID_PROVE_STATE_ERRORFLOW);   break;
		case State::ERRORSTAB:     event.Reinit(EID_PROVE_STATE_ERRORSTAB);   break;
		case State::ERRORTOUP:     event.Reinit(EID_PROVE_STATE_ERRORTOUP);   break;
		case State::ERRORTODOWN:   event.Reinit(EID_PROVE_STATE_ERRORTDOWN);  break;
		case State::ERRORD1:       event.Reinit(EID_PROVE_STATE_ERRORD1);     break;
		case State::ERRORD2:       event.Reinit(EID_PROVE_STATE_ERRORD2);     break;
		case State::ERRORDETECTOR: event.Reinit(EID_PROVE_STATE_ERRORDETS);   break;
		case State::ERRORRETURN:   event.Reinit(EID_PROVE_STATE_ERRORRETURN); break;
	}

	rEventManager::instance().Add(event);
}

void rProve::calcAverage()
{
	if (!m_enableAverage) {
		return;
	}

	UDINT dCount = m_moduleCount - m_averageCount;
	if (dCount) {
		m_prvFreq = (m_moduleFreq * dCount) + (m_prvFreq * m_averageCount) / static_cast<LREAL>(m_moduleCount);
		m_prvDens = (m_dens.Value * dCount) + (m_prvDens * m_averageCount) / static_cast<LREAL>(m_moduleCount);
		m_prvTemp = (m_temp.Value * dCount) + (m_prvTemp * m_averageCount) / static_cast<LREAL>(m_moduleCount);
		m_prvPres = (m_pres.Value * dCount) + (m_prvPres * m_averageCount) / static_cast<LREAL>(m_moduleCount);
		m_strDens = (m_curStrDens * dCount) + (m_strDens * m_averageCount) / static_cast<LREAL>(m_moduleCount);
		m_strTemp = (m_curStrTemp * dCount) + (m_strTemp * m_averageCount) / static_cast<LREAL>(m_moduleCount);
		m_strPres = (m_curStrPres * dCount) + (m_strPres * m_averageCount) / static_cast<LREAL>(m_moduleCount);
	}
}

UDINT rProve::generateVars(rVariableList& list)
{
	rSource::generateVars(list);

	// Variables
	list.add(Alias + ".command"                  , TYPE_UINT , rVariable::Flags::____, &m_command    , U_DIMLESS, ACCESS_PROVE);
	list.add(Alias + ".setup"                    , TYPE_UINT , rVariable::Flags::____, &m_setup      , U_DIMLESS, ACCESS_PROVE);
	list.add(Alias + ".state"                    , TYPE_UINT , rVariable::Flags::R___, &m_state      , U_DIMLESS, 0);
	list.add(Alias + ".timer.start"              , TYPE_UDINT, rVariable::Flags::____, &m_tStart     , U_msec   , ACCESS_PROVE);
	list.add(Alias + ".timer.stabilization"      , TYPE_UDINT, rVariable::Flags::____, &m_tStab      , U_msec   , ACCESS_PROVE);
	list.add(Alias + ".timer.detector1"          , TYPE_UDINT, rVariable::Flags::____, &m_tD1        , U_msec   , ACCESS_PROVE);
	list.add(Alias + ".timer.detector2"          , TYPE_UDINT, rVariable::Flags::____, &m_tD2        , U_msec   , ACCESS_PROVE);
	list.add(Alias + ".timer.volume"             , TYPE_UDINT, rVariable::Flags::____, &m_tVolume    , U_msec   , ACCESS_PROVE);
	list.add(Alias + ".timer.valve"              , TYPE_UDINT, rVariable::Flags::____, &m_tValve     , U_msec   , ACCESS_PROVE);
	list.add(Alias + ".timer.bounce"             , TYPE_UDINT, rVariable::Flags::____, &m_tBounce    , U_msec   , ACCESS_PROVE);
	list.add(Alias + ".result.volume1.count"     , TYPE_LREAL, rVariable::Flags::R___, &m_prvCount[0], U_imp    , 0);
	list.add(Alias + ".result.volume1.time"      , TYPE_LREAL, rVariable::Flags::R___, &m_prvTime[0] , U_sec    , 0);
	list.add(Alias + ".result.volume2.count"     , TYPE_LREAL, rVariable::Flags::R___, &m_prvCount[1], U_imp    , 0);
	list.add(Alias + ".result.volume2.time"      , TYPE_LREAL, rVariable::Flags::R___, &m_prvTime[1] , U_sec    , 0);
	list.add(Alias + ".result.prove.frequency"   , TYPE_LREAL, rVariable::Flags::R___, &m_prvFreq    , U_Hz     , 0);
	list.add(Alias + ".result.prove.temperature" , TYPE_LREAL, rVariable::Flags::R___, &m_prvTemp    , U_C      , 0);
	list.add(Alias + ".result.prove.pressure"    , TYPE_LREAL, rVariable::Flags::R___, &m_prvPres    , U_MPa    , 0);
	list.add(Alias + ".result.prove.density"     , TYPE_LREAL, rVariable::Flags::R___, &m_prvDens    , U_kg_m3  , 0);
	list.add(Alias + ".result.stream.temperature", TYPE_LREAL, rVariable::Flags::R___, &m_strTemp    , U_C      , 0);
	list.add(Alias + ".result.stream.pressure"   , TYPE_LREAL, rVariable::Flags::R___, &m_strPres    , U_MPa    , 0);
	list.add(Alias + ".result.stream.density"    , TYPE_LREAL, rVariable::Flags::R___, &m_strDens    , U_kg_m3  , 0);
	list.add(Alias + ".detectors.present"        , TYPE_UINT , rVariable::Flags::R___, &m_curDet     , U_DIMLESS, 0);
	list.add(Alias + ".detectors.fixed"          , TYPE_UINT , rVariable::Flags::R___, &m_fixDet     , U_DIMLESS, 0);
	list.add(Alias + ".stabilization.temperature", TYPE_LREAL, rVariable::Flags::____, &m_maxStabTemp, U_C      , ACCESS_PROVE);
	list.add(Alias + ".stabilization.pressure"   , TYPE_LREAL, rVariable::Flags::____, &m_maxStabPres, U_MPa    , ACCESS_PROVE);
	list.add(Alias + ".stabilization.density"    , TYPE_LREAL, rVariable::Flags::____, &m_maxStabDens, U_kg_m3  , ACCESS_PROVE);
	list.add(Alias + ".stabilization.frequency"  , TYPE_LREAL, rVariable::Flags::____, &m_maxStabFreq, U_Hz     , ACCESS_PROVE);

	list.add(Alias + ".fault"                    , TYPE_UDINT, rVariable::Flags::R___, &Fault        , U_DIMLESS, 0);

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rProve::LoadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix)
{
	std::string strSetup = XmlUtils::getAttributeString(element, XmlName::SETUP, m_flagsSetup.getNameByBits(Setup::STABILIZATION | Setup::BOUNCE);

	if (rSource::LoadFromXML(element, err, prefix) != TRITONN_RESULT_OK) {
		return err.getError();
	}

	tinyxml2::XMLElement* xml_module = element->FirstChildElement(XmlName::IOLINK);
	if (xml_module) {
		if (rDataModule::loadFromXML(xml_module, err) != TRITONN_RESULT_OK) {
			return err.getError();
		}
	} else {
		return err.set(DATACFGERR_PORVE_MISSINGMODULE, "");
	}

	tinyxml2::XMLElement* xml_temp = element->FirstChildElement(XmlName::TEMP);
	tinyxml2::XMLElement* xml_pres = element->FirstChildElement(XmlName::PRES);
	tinyxml2::XMLElement* xml_dens = element->FirstChildElement(XmlName::DENS);

	if (xml_temp) if (TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(xml_temp->FirstChildElement(XmlName::LINK), m_temp)) return err.getError();
	if (xml_pres) if (TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(xml_temp->FirstChildElement(XmlName::LINK), m_temp)) return err.getError();
	if (xml_dens) if (TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(xml_temp->FirstChildElement(XmlName::LINK), m_temp)) return err.getError();

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

	m_enableAverage = true;
}

void rProve::moduleStop()
{
	rSnapshot ss(rDataManager::instance().getVariableClass(), 0xFFFFFFFF);

	ss.add(m_moduleName + ".stop", 1);
	ss.set();

	m_enableAverage = false;
}



