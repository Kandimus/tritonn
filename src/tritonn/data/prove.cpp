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
#include "../data_config.h"
#include "../data_manager.h"
#include "../data_station.h"
#include "../data_ai.h"
#include "../error.h"
#include "../variable_item.h"
#include "../variable_list.h"
#include "../data_snapshot.h"
#include "../io/manager.h"
#include "../io/module_crm.h"
#include "../xml_util.h"
#include "../generator_md.h"
#include "prove.h"

rBitsArray rProve::m_flagsSetup;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
rProve::rProve(const rStation* owner)
	: rSource(owner), rDataModule(true), m_setup(static_cast<UINT>(Setup::NONE))
{
	if (m_flagsSetup.empty()) {
		m_flagsSetup
				.add("NONE"         , static_cast<UINT>(Setup::NONE), "Не использовать флаги настройки")
				.add("4WAY"         , static_cast<UINT>(Setup::VALVE_4WAY), "ПУ использует четырех ходовой кран")
				.add("STABILIZATION", static_cast<UINT>(Setup::STABILIZATION), "Перед измерением проверять параметры на стабильность")
				.add("NOVALVE"      , static_cast<UINT>(Setup::NOVALVE), "ПУ ручным краном")
				.add("ONEDETECTOR"  , static_cast<UINT>(Setup::ONEDETECTOR), "Используется один детектор")
				.add("BOUNCE"       , static_cast<UINT>(Setup::BOUNCE), "Фильтрация дребезга детекторов")
				.add("SIMULATE"     , static_cast<UINT>(Setup::SIMULATE), "Симуляция крана");
	}

	//NOTE Единицы измерения добавим после загрузки сигнала
	initLink(rLink::Setup::INPUT , m_temp  , U_C       , SID::TEMPERATURE, XmlName::TEMP   , rLink::SHADOW_NONE);
	initLink(rLink::Setup::INPUT , m_pres  , U_MPa     , SID::PRESSURE   , XmlName::PRES   , rLink::SHADOW_NONE);
	initLink(rLink::Setup::INPUT , m_dens  , U_kg_m3   , SID::DENSITY    , XmlName::DENSITY, rLink::SHADOW_NONE);
	initLink(rLink::Setup::INPUT , m_opened, U_discrete, SID::OPENED     , XmlName::OPENED , rLink::SHADOW_NONE);
	initLink(rLink::Setup::INPUT , m_closed, U_discrete, SID::CLOSED     , XmlName::CLOSED , rLink::SHADOW_NONE);
	initLink(rLink::Setup::OUTPUT, m_open  , U_discrete, SID::OPEN       , XmlName::OPEN   , rLink::SHADOW_NONE);
	initLink(rLink::Setup::OUTPUT, m_close , U_discrete, SID::CLOSE      , XmlName::CLOSE  , rLink::SHADOW_NONE);
}


//-------------------------------------------------------------------------------------------------
//
UDINT rProve::initLimitEvent(rLink &link)
{
	link.m_limit.EventChangeAMin  = reinitEvent(EID_AI_NEW_AMIN)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeWMin  = reinitEvent(EID_AI_NEW_WMIN)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeWMax  = reinitEvent(EID_AI_NEW_WMAX)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeAMax  = reinitEvent(EID_AI_NEW_AMAX)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeHyst  = reinitEvent(EID_AI_NEW_HYST)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeSetup = reinitEvent(EID_AI_NEW_SETUP) << link.m_descr << link.m_unit;
	link.m_limit.EventAMin        = reinitEvent(EID_AI_AMIN)      << link.m_descr << link.m_unit;
	link.m_limit.EventWMin        = reinitEvent(EID_AI_WMIN)      << link.m_descr << link.m_unit;
	link.m_limit.EventWMax        = reinitEvent(EID_AI_WMAX)      << link.m_descr << link.m_unit;
	link.m_limit.EventAMax        = reinitEvent(EID_AI_AMAX)      << link.m_descr << link.m_unit;
	link.m_limit.EventNan         = reinitEvent(EID_AI_NAN)       << link.m_descr << link.m_unit;
	link.m_limit.EventNormal      = reinitEvent(EID_AI_NORMAL)    << link.m_descr << link.m_unit;

	return TRITONN_RESULT_OK;
}

//-------------------------------------------------------------------------------------------------
//
UDINT rProve::calculate()
{
	rEvent event_s;
	rEvent event_f;
	
	if (rSource::calculate()) {
		return TRITONN_RESULT_OK;
	}

	m_setup.Compare(reinitEvent(EID_AI_NEW_SETUP));

	if (isSetModule()) {
		auto module_ptr = rIOManager::instance().getModule(m_module);
		auto module     = dynamic_cast<rModuleCRM*>(module_ptr.get());

		if (!module) {
			rEventManager::instance().Add(reinitEvent(EID_PROVE_MODULE) << m_module);
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
			rEventManager::instance().Add(reinitEvent(EID_PROVE_BADCOMMAND) << static_cast<UINT>(m_command));
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
		case State::ERRORRETURN:
		case State::ERRORSTREAMID:
			onErrorState();    break;
	};

	m_command = Command::NONE;

	postCalculate();
		
	return TRITONN_RESULT_OK;
}

void rProve::onIdle()
{
	switch(m_command) {
		case Command::START:
			rEventManager::instance().Add(reinitEvent(EID_PROVE_COMMANDSTART));
			setState(State::START);
			break;

		case Command::ABORT:
			rEventManager::instance().Add(reinitEvent(EID_PROVE_NOTSTARTED));
			break;

		case Command::RESET:
			rEventManager::instance().Add(reinitEvent(EID_PROVE_COMMANDRESET));
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
		if (!connectToLine())
		{
			return;
		}
		m_timer.start(m_tStart);
		return;
	}

	if (m_timer.isFinished()) {
		m_timer.stop();

		if (m_moduleFreq < 0.001) {
			setState(State::ERRORFLOW);
			return;
		}

		if (m_setup.Value & Setup::STABILIZATION) {
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
		m_stabTemp = m_temp.m_value;
		m_stabPres = m_pres.m_value;
		m_stabDens = m_dens.m_value;
		m_stabFreq = m_moduleFreq;

		m_timer.start(m_tStab);
		return;
	}

	if (m_timer.isFinished()) {
		m_timer.stop();
		setState(State::VALVETOUP);
		return;
	}

	if (checkStab(m_stabTemp, m_temp.m_value, m_maxStabTemp, EID_PROVE_ERRORSTABTEMP)) {
		return;
	}

	if (checkStab(m_stabPres, m_pres.m_value, m_maxStabPres, EID_PROVE_ERRORSTABPRES)) {
		return;
	}

	if (checkStab(m_stabDens, m_dens.m_value, m_maxStabDens, EID_PROVE_ERRORSTABDENS)) {
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

	if (m_opened.m_value > 0 && m_closed.m_value == 0) {
		setState(State::VALVETODOWN);
		return;
	}

	m_close.m_value = 0;
	m_open.m_value  = 1;
	setState(State::WAITTOUP);
}


void rProve::onWaitToUp()
{
	if (checkCommand()) {
		return;
	}

	if (!m_timer.isStarted()) {
		m_timer.start(2 * (m_tD1 + m_tD2 + m_tVolume));
		return;
	}

	if (m_timer.isFinished()) {
		m_timer.stop();

		if (m_opened.m_value > 0 && m_closed.m_value == 0) {
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
		m_close.m_value = 1;
		m_open.m_value  = 0;
		m_timer.start(m_tValve);
		return;
	}

	if(m_timer.isFinished()) {
		m_timer.stop();

		setState(State::ERRORTODOWN);
		return;
	}

	if (m_closed.m_value > 0 && m_opened.m_value == 0) {
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
		m_close.m_value = 0;
		m_open.m_value  = 1;
		m_timer.start(m_tValve);
		return;
	}

	if (m_timer.isFinished()) {
		m_timer.stop();
		setState(State::ERRORRETURN);
		return;
	}

	if (m_opened.m_value > 0 && m_closed.m_value == 0) {
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
			rEventManager::instance().Add(reinitEvent(EID_PROVE_ISERROR));
			break;

		case Command::ABORT:
			setState(State::IDLE);
			break;

		case Command::RESET:
			m_state = State::IDLE;
			rEventManager::instance().Add(reinitEvent(EID_PROVE_COMMANDRESET));
			break;

		default: break;
	}
}

bool rProve::checkCommand()
{
	switch(m_command) {
		case Command::START:
			rEventManager::instance().Add(reinitEvent(EID_PROVE_ALREADYSTARTED));
			break;

		case Command::ABORT:
			rEventManager::instance().Add(reinitEvent(EID_PROVE_COMMANDABORT));
			setState(State::IDLE);
			return true;

		case Command::RESET:
			rEventManager::instance().Add(reinitEvent(EID_PROVE_NOTERROR));
			break;

		default: break;
	}

	return false;
}


DINT rProve::checkDetectors(bool first)
{
	if (m_setup.Value & Setup::ONEDETECTOR) {
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
		rEventManager::instance().Add(reinitEvent(eid) << delta << maxstab);
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

			if (m_setup.Value & Setup::BOUNCE) {
				m_timerBounce.start(m_tBounce);
			}
		}
		return;
	}

	if (m_timerBounce.isFinished()) {
		m_timerBounce.stop();
	}
}

bool rProve::connectToLine()
{
	if (m_strIdx.Value >= m_station->getStreamCount()) {
		rEventManager::instance().Add(reinitEvent(EID_PROVE_BADSTREAMNUMBER) << m_strIdx.Value);
		setState(State::ERRORSTREAMID);
		return false;
	}

	m_station->setStreamFreqOut(m_strIdx.Value);
	return false;
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
		case State::ERRORSTREAMID: event.Reinit(EID_PROVE_STATE_ERRORRSTRID); break;
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
		m_prvFreq = (m_moduleFreq   * dCount) + (m_prvFreq * m_averageCount) / static_cast<LREAL>(m_moduleCount);
		m_prvDens = (m_dens.m_value * dCount) + (m_prvDens * m_averageCount) / static_cast<LREAL>(m_moduleCount);
		m_prvTemp = (m_temp.m_value * dCount) + (m_prvTemp * m_averageCount) / static_cast<LREAL>(m_moduleCount);
		m_prvPres = (m_pres.m_value * dCount) + (m_prvPres * m_averageCount) / static_cast<LREAL>(m_moduleCount);
		m_strDens = (m_curStrDens   * dCount) + (m_strDens * m_averageCount) / static_cast<LREAL>(m_moduleCount);
		m_strTemp = (m_curStrTemp   * dCount) + (m_strTemp * m_averageCount) / static_cast<LREAL>(m_moduleCount);
		m_strPres = (m_curStrPres   * dCount) + (m_strPres * m_averageCount) / static_cast<LREAL>(m_moduleCount);
	}
}

UDINT rProve::generateVars(rVariableList& list)
{
	rSource::generateVars(list);

	// Variables
	list.add(m_alias + ".command"                  , TYPE_UINT , rVariable::Flags::____, &m_command    , U_DIMLESS, ACCESS_PROVE);
	list.add(m_alias + ".setup"                    , TYPE_UINT , rVariable::Flags::____, &m_setup.Value, U_DIMLESS, ACCESS_PROVE);
	list.add(m_alias + ".state"                    , TYPE_UINT , rVariable::Flags::R___, &m_state      , U_DIMLESS, 0);
	list.add(m_alias + ".timer.start"              , TYPE_UDINT, rVariable::Flags::____, &m_tStart     , U_msec   , ACCESS_PROVE);
	list.add(m_alias + ".timer.stabilization"      , TYPE_UDINT, rVariable::Flags::____, &m_tStab      , U_msec   , ACCESS_PROVE);
	list.add(m_alias + ".timer.detector1"          , TYPE_UDINT, rVariable::Flags::____, &m_tD1        , U_msec   , ACCESS_PROVE);
	list.add(m_alias + ".timer.detector2"          , TYPE_UDINT, rVariable::Flags::____, &m_tD2        , U_msec   , ACCESS_PROVE);
	list.add(m_alias + ".timer.volume"             , TYPE_UDINT, rVariable::Flags::____, &m_tVolume    , U_msec   , ACCESS_PROVE);
	list.add(m_alias + ".timer.valve"              , TYPE_UDINT, rVariable::Flags::____, &m_tValve     , U_msec   , ACCESS_PROVE);
	list.add(m_alias + ".timer.bounce"             , TYPE_UDINT, rVariable::Flags::____, &m_tBounce    , U_msec   , ACCESS_PROVE);
	list.add(m_alias + ".result.volume1.count"     , TYPE_LREAL, rVariable::Flags::R___, &m_prvCount[0], U_imp    , 0);
	list.add(m_alias + ".result.volume1.time"      , TYPE_LREAL, rVariable::Flags::R___, &m_prvTime[0] , U_sec    , 0);
	list.add(m_alias + ".result.volume2.count"     , TYPE_LREAL, rVariable::Flags::R___, &m_prvCount[1], U_imp    , 0);
	list.add(m_alias + ".result.volume2.time"      , TYPE_LREAL, rVariable::Flags::R___, &m_prvTime[1] , U_sec    , 0);
	list.add(m_alias + ".result.prove.frequency"   , TYPE_LREAL, rVariable::Flags::R___, &m_prvFreq    , U_Hz     , 0);
	list.add(m_alias + ".result.prove.temperature" , TYPE_LREAL, rVariable::Flags::R___, &m_prvTemp    , U_C      , 0);
	list.add(m_alias + ".result.prove.pressure"    , TYPE_LREAL, rVariable::Flags::R___, &m_prvPres    , U_MPa    , 0);
	list.add(m_alias + ".result.prove.density"     , TYPE_LREAL, rVariable::Flags::R___, &m_prvDens    , U_kg_m3  , 0);
	list.add(m_alias + ".result.stream.temperature", TYPE_LREAL, rVariable::Flags::R___, &m_strTemp    , U_C      , 0);
	list.add(m_alias + ".result.stream.pressure"   , TYPE_LREAL, rVariable::Flags::R___, &m_strPres    , U_MPa    , 0);
	list.add(m_alias + ".result.stream.density"    , TYPE_LREAL, rVariable::Flags::R___, &m_strDens    , U_kg_m3  , 0);
	list.add(m_alias + ".detectors.present"        , TYPE_UINT , rVariable::Flags::R___, &m_curDet     , U_DIMLESS, 0);
	list.add(m_alias + ".detectors.fixed"          , TYPE_UINT , rVariable::Flags::R___, &m_fixDet     , U_DIMLESS, 0);
	list.add(m_alias + ".stabilization.temperature", TYPE_LREAL, rVariable::Flags::____, &m_maxStabTemp, U_C      , ACCESS_PROVE);
	list.add(m_alias + ".stabilization.pressure"   , TYPE_LREAL, rVariable::Flags::____, &m_maxStabPres, U_MPa    , ACCESS_PROVE);
	list.add(m_alias + ".stabilization.density"    , TYPE_LREAL, rVariable::Flags::____, &m_maxStabDens, U_kg_m3  , ACCESS_PROVE);
	list.add(m_alias + ".stabilization.frequency"  , TYPE_LREAL, rVariable::Flags::____, &m_maxStabFreq, U_Hz     , ACCESS_PROVE);

	list.add(m_alias + ".fault"                    , TYPE_UDINT, rVariable::Flags::R___, &m_fault      , U_DIMLESS, 0);

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rProve::loadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix)
{
	std::string strSetup = XmlUtils::getAttributeString(element, XmlName::SETUP, m_flagsSetup.getNameByBits(Setup::STABILIZATION | Setup::BOUNCE));

	if (rSource::loadFromXML(element, err, prefix) != TRITONN_RESULT_OK) {
		return err.getError();
	}

	tinyxml2::XMLElement* xml_module = element->FirstChildElement(XmlName::IOLINK);
	if (xml_module) {
		if (rDataModule::loadFromXML(xml_module, err) != TRITONN_RESULT_OK) {
			return err.getError();
		}
	} else {
		return err.set(DATACFGERR_PORVE_MISSINGMODULE, element->GetLineNum());
	}

	auto xml_temp  = element->FirstChildElement(XmlName::TEMP);
	auto xml_pres  = element->FirstChildElement(XmlName::PRES);
	auto xml_dens  = element->FirstChildElement(XmlName::DENSITY);
	auto xml_valve = element->FirstChildElement(XmlName::VALVE);

	if (xml_temp) if (TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(xml_temp->FirstChildElement(XmlName::LINK), m_temp)) return err.getError();
	if (xml_pres) if (TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(xml_temp->FirstChildElement(XmlName::LINK), m_pres)) return err.getError();
	if (xml_dens) if (TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(xml_temp->FirstChildElement(XmlName::LINK), m_dens)) return err.getError();

	if (xml_valve) {
		auto xml_opened = xml_valve->FirstChildElement(XmlName::OPENED);
		auto xml_closed = xml_valve->FirstChildElement(XmlName::CLOSED);

		if (xml_opened) if (TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(xml_opened->FirstChildElement(XmlName::LINK), m_opened)) return err.getError();
		if (xml_closed) if (TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(xml_closed->FirstChildElement(XmlName::LINK), m_closed)) return err.getError();
	}

	m_open.m_limit.m_setup.Init(rLimit::Setup::OFF);
	m_close.m_limit.m_setup.Init(rLimit::Setup::OFF);
	m_opened.m_limit.m_setup.Init(rLimit::Setup::OFF);
	m_closed.m_limit.m_setup.Init(rLimit::Setup::OFF);

	reinitLimitEvents();

	return TRITONN_RESULT_OK;
}


UDINT rProve::generateMarkDown(rGeneratorMD& md)
{
	m_temp.m_limit.m_setup.Init(rLimit::Setup::HIHI | rLimit::Setup::HI | rLimit::Setup::LO | rLimit::Setup::LOLO);
	m_pres.m_limit.m_setup.Init(rLimit::Setup::HIHI | rLimit::Setup::HI | rLimit::Setup::LO | rLimit::Setup::LOLO);
	m_dens.m_limit.m_setup.Init(rLimit::Setup::HIHI | rLimit::Setup::HI | rLimit::Setup::LO | rLimit::Setup::LOLO);
	m_open.m_limit.m_setup.Init(rLimit::Setup::OFF);
	m_close.m_limit.m_setup.Init(rLimit::Setup::OFF);
	m_opened.m_limit.m_setup.Init(rLimit::Setup::OFF);
	m_closed.m_limit.m_setup.Init(rLimit::Setup::OFF);

	md.add(this, false)
			.addProperty(XmlName::SETUP, &m_flagsSetup)
			.addXml("<io_link module=\"module index\"/>" + std::string(rGeneratorMD::rItem::XML_OPTIONAL))
			.addLink(XmlName::TEMP, true)
			.addLink(XmlName::PRES, true)
			.addLink(XmlName::DENSITY, true)
			.addXml(String_format("<%s>", XmlName::VALVE))
			.addLink(XmlName::OPENED, false, "\t")
			.addLink(XmlName::CLOSED, false, "\t")
			.addXml(String_format("</%s>", XmlName::VALVE));

	return TRITONN_RESULT_OK;
}

void rProve::moduleStart()
{
	rSnapshot ss(rDataManager::instance().getVariableClass(), 0xFFFFFFFF);

	ss.add(m_moduleName + ".start", 1);
	ss.set();

	m_enableAverage = true;
	m_averageCount  = m_moduleCount;
}

void rProve::moduleStop()
{
	rSnapshot ss(rDataManager::instance().getVariableClass(), 0xFFFFFFFF);

	ss.add(m_moduleName + ".stop", 1);
	ss.set();

	m_enableAverage = false;
}



