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

#include "prove.h"
#include <vector>
#include <limits>
#include <cmath>
#include <string.h>
#include "tinyxml2.h"
#include "../event/eid.h"
#include "../event/manager.h"
#include "../text_id.h"
#include "../data_config.h"
#include "../data_manager.h"
#include "../data_station.h"
#include "../data_stream.h"
#include "../data_ai.h"
#include "../error.h"
#include "../variable_item.h"
#include "../variable_list.h"
#include "../data_snapshot.h"
#include "../io/manager.h"
#include "../io/module_crm.h"
#include "../xml_util.h"
#include "../generator_md.h"
#include "../comment_defines.h"


rBitsArray rProve::m_flagsSetup;
rBitsArray rProve::m_flagsCommand;
rBitsArray rProve::m_flagsState;
rBitsArray rProve::m_flagsWay;
rBitsArray rProve::m_flagsDetectors;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
rProve::rProve(const rStation* owner)
	: rSource(owner), rDataModule(true), m_setup(static_cast<UINT>(Setup::NONE))
{
	if (m_flagsSetup.empty()) {
		m_flagsSetup
				.add("NONE"         , static_cast<UINT>(Setup::NONE)         , "Не использовать флаги настройки.")
				.add("4WAY"         , static_cast<UINT>(Setup::VALVE_4WAY)   , "ПУ использует четырех ходовой кран (двухпроходная ПУ). Если флаг не указан, то используется однопроходная ПУ.")
				.add("STABILIZATION", static_cast<UINT>(Setup::STABILIZATION), "Перед измерением проверять параметры на стабильность.")
				.add("NOVALVE"      , static_cast<UINT>(Setup::NOVALVE)      , "ПУ с ручным краном.")
				.add("ONEDETECTOR"  , static_cast<UINT>(Setup::ONEDETECTOR)  , "Используется один детектор.")
				.add("BOUNCE"       , static_cast<UINT>(Setup::BOUNCE)       , "Фильтрация дребезга детекторов.")
				.add("NOSELECTSTR"  , static_cast<UINT>(Setup::NOSELECTSTR)  , "Не переключать частоту ПР.")
				.add("SIMULATE"     , static_cast<UINT>(Setup::SIMULATE)     , "Симуляция крана без выдачи управляющих воздействий.");
	}

	if (m_flagsCommand.empty()) {
		m_flagsCommand
				.add("", static_cast<UINT>(Command::NONE) , COMMENT::COMMAND_NONE)
				.add("", static_cast<UINT>(Command::START), COMMENT::COMMAND_START + COMMENT::PROVE)
				.add("", static_cast<UINT>(Command::ABORT), "Прервать" + COMMENT::PROVE)
				.add("", static_cast<UINT>(Command::RESET), "Сбросить ошибку");
	}

	if (m_flagsWay.empty())
	{
		m_flagsWay
				.add("", static_cast<UINT>(Way::FORWARD), "Прямой проход шара")
				.add("", static_cast<UINT>(Way::REVERSE), "Обратных проход шара");
	}

	if (m_flagsState.empty()) {
		m_flagsState
				.add("", static_cast<UINT>(State::IDLE)           , "Процедура не запущена")
				.add("", static_cast<UINT>(State::START)          , "Запуск процедуры поверки")
				.add("", static_cast<UINT>(State::STABILIZATION)  , "Стабилизация")
				.add("", static_cast<UINT>(State::RUNBALL)        , "Запуск шара")
				.add("", static_cast<UINT>(State::VALVETOUP)      , "Поворот крана в верхнюю позицию")
				.add("", static_cast<UINT>(State::WAITTOUP)       , "Ожидание поворота крана в верхнюю позицию")
				.add("", static_cast<UINT>(State::VALVETODOWN)    , "Поворот крана в нижнюю позицию")
				.add("", static_cast<UINT>(State::WAITD1)         , "Ожидание первого детектора")
				.add("", static_cast<UINT>(State::WAITD2)         , "Ожидание второго детектора")
				.add("", static_cast<UINT>(State::CALCULATE)      , "Вычисления")
				.add("", static_cast<UINT>(State::RETURNBALL)     , "Ожидание возврата шара")
				.add("", static_cast<UINT>(State::WAITD1_REVERSE) , "Обратный ход шара. Ожидание первого детектора")
				.add("", static_cast<UINT>(State::WAITD2_REVERSE) , "Обратный ход шара. Ожидание второго детектора")
				.add("", static_cast<UINT>(State::WAYCOMPLITED)   , "Проход шара завершен")
				.add("", static_cast<UINT>(State::FINISH)         , "Процедура завершена")
				.add("", static_cast<UINT>(State::ABORT)          , "Прерывание процедуры")
				.add("", static_cast<UINT>(State::ERRORFLOW)      , "Нет расхода")
				.add("", static_cast<UINT>(State::ERRORSTAB)      , "Ошибка стабилизации")
				.add("", static_cast<UINT>(State::ERRORTOUP)      , "Ошибка поворота крана в верхнюю позицию")
				.add("", static_cast<UINT>(State::ERRORTODOWN)    , "Ошибка поворота крана в нижнюю позицию")
				.add("", static_cast<UINT>(State::ERRORD1)        , "Ошибка первого детектора")
				.add("", static_cast<UINT>(State::ERRORD2)        , "Ошибка второго детектора")
				.add("", static_cast<UINT>(State::ERRORDETECTOR)  , "Ошибка детекторов")
				.add("", static_cast<UINT>(State::ERRORRETURN)    , "Ошибка возврата шара")
				.add("", static_cast<UINT>(State::ERRORSTREAMID)  , "Выбрана не корректный ПР")
				.add("", static_cast<UINT>(State::ERRORD1_REVERSE), "Обратных ход шара. Ошибка первого детектора")
				.add("", static_cast<UINT>(State::ERRORD2_REVERSE), "Обратных ход шара. Ошибка второго детектора");
	}

	if (m_flagsDetectors.empty()) {
		m_flagsDetectors
				.add("", static_cast<UINT>(Detector::DET_1), "Детектор 1")
				.add("", static_cast<UINT>(Detector::DET_2), "Детектор 2")
				.add("", static_cast<UINT>(Detector::DET_3), "Детектор 3")
				.add("", static_cast<UINT>(Detector::DET_4), "Детектор 4");
	}

	//NOTE Единицы измерения добавим после загрузки сигнала
	initLink(rLink::Setup::INPUT , m_temp      , U_C       , SID::TEMPERATURE    , XmlName::TEMP      , rLink::SHADOW_NONE);
	initLink(rLink::Setup::INPUT , m_pres      , U_MPa     , SID::PRESSURE       , XmlName::PRES      , rLink::SHADOW_NONE);
	initLink(rLink::Setup::INPUT , m_dens      , U_kg_m3   , SID::DENSITY        , XmlName::DENSITY   , rLink::SHADOW_NONE);
	initLink(rLink::Setup::INPUT , m_opened    , U_discrete, SID::PROVER_OPENED  , XmlName::OPENED    , rLink::SHADOW_NONE);
	initLink(rLink::Setup::INPUT , m_closed    , U_discrete, SID::PROVER_CLOSED  , XmlName::CLOSED    , rLink::SHADOW_NONE);
	initLink(rLink::Setup::OUTPUT, m_open      , U_discrete, SID::PROVER_OPEN    , XmlName::OPEN      , rLink::SHADOW_NONE);
	initLink(rLink::Setup::OUTPUT, m_close     , U_discrete, SID::PROVER_CLOSE   , XmlName::CLOSE     , rLink::SHADOW_NONE);
	initLink(rLink::Setup::OUTPUT, m_inProgress, U_discrete, SID::PROVER_PROGRESS, XmlName::INPROGRESS, rLink::SHADOW_NONE);
}


//-------------------------------------------------------------------------------------------------
//
UDINT rProve::initLimitEvent(rLink &link)
{
	link.m_limit.EventChangeAMin  = reinitEvent(EID_PROVE_NEW_AMIN)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeWMin  = reinitEvent(EID_PROVE_NEW_WMIN)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeWMax  = reinitEvent(EID_PROVE_NEW_WMAX)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeAMax  = reinitEvent(EID_PROVE_NEW_AMAX)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeHyst  = reinitEvent(EID_PROVE_NEW_HYST)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeSetup = reinitEvent(EID_PROVE_NEW_SETUP) << link.m_descr << link.m_unit;
	link.m_limit.EventAMin        = reinitEvent(EID_PROVE_AMIN)      << link.m_descr << link.m_unit;
	link.m_limit.EventWMin        = reinitEvent(EID_PROVE_WMIN)      << link.m_descr << link.m_unit;
	link.m_limit.EventWMax        = reinitEvent(EID_PROVE_WMAX)      << link.m_descr << link.m_unit;
	link.m_limit.EventAMax        = reinitEvent(EID_PROVE_AMAX)      << link.m_descr << link.m_unit;
	link.m_limit.EventNan         = reinitEvent(EID_PROVE_NAN)       << link.m_descr << link.m_unit;
	link.m_limit.EventNormal      = reinitEvent(EID_PROVE_NORMAL)    << link.m_descr << link.m_unit;

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

	if (m_state == State::IDLE) {
		m_setup.Compare(reinitEvent(EID_AI_NEW_SETUP));
	} else if (m_setup.isDiff()) {
		rEventManager::instance().add(reinitEvent(EID_PROVE_CANNOTMODIFYSETUP));
		m_setup.Value = m_setup.Saved;
	}

	if (isSetModule()) {
		auto module_ptr = rIOManager::instance().getModule(m_module);
		auto module     = dynamic_cast<rModuleCRM*>(module_ptr);

		if (!module) {
			rEventManager::instance().add(reinitEvent(EID_PROVE_MODULE) << m_module);
			rDataManager::instance().DoHalt(HALT_REASON_RUNTIME | DATACFGERR_REALTIME_MODULELINK);
			return DATACFGERR_REALTIME_MODULELINK;
		}

		m_moduleName  = module->getAlias();
		m_moduleFreq  = module->getFreq();
		m_moduleDet   = module->getDetectors();
		m_moduleCount = module->getCounter();

		if (module_ptr) {
			delete module_ptr;
		}
	}

	detectorsProcessing();
	calcAverage();

	switch(m_command) {
		case Command::NONE:
		case Command::START:
		case Command::ABORT:
		case Command::RESET: break;

		default:
			rEventManager::instance().add(reinitEvent(EID_PROVE_BADCOMMAND) << static_cast<UINT>(m_command));
			break;
	}

	switch(m_state) {
		case State::IDLE:
		case State::FINISH:         onIdle();          break;

		case State::START:          onStart();         break;
		case State::RUNBALL:        onRunBall();       break;
		case State::STABILIZATION:  onStabilization(); break;
		case State::VALVETOUP:      onValveToUp();     break;
		case State::WAITTOUP:       onWaitToUp();      break;
		case State::VALVETODOWN:    onValveToDown();   break;
		case State::WAITD1:         onWaitD1();        break;
		case State::WAITD2:         onWaitD2();        break;
		case State::CALCULATE:      onCalculate();     break;
		case State::RETURNBALL:     onReturnBall();    break;
		case State::ABORT:          onAbort();         break;
		case State::WAITD1_REVERSE: onWaitD1();        break;
		case State::WAITD2_REVERSE: onWaitD2();        break;
		case State::WAYCOMPLITED:   onWayComplited();  break;

		case State::ERRORFLOW:
		case State::ERRORSTAB:
		case State::ERRORTOUP:
		case State::ERRORTODOWN:
		case State::ERRORD1:
		case State::ERRORD2:
		case State::ERRORDETECTOR:
		case State::ERRORRETURN:
		case State::ERRORSTREAMID:
		case State::ERRORD1_REVERSE:
		case State::ERRORD2_REVERSE:
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
			rEventManager::instance().add(reinitEvent(EID_PROVE_COMMANDSTART));
			setState(State::START);
			break;

		case Command::ABORT:
			rEventManager::instance().add(reinitEvent(EID_PROVE_NOTSTARTED));
			break;

		case Command::RESET:
			rEventManager::instance().add(reinitEvent(EID_PROVE_COMMANDRESET));
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
			rEventManager::instance().add(reinitEvent(EID_PROVE_BADSTREAMNUMBER) << m_strIdx.Value);
			setState(State::ERRORSTREAMID);
			return;
		}
		m_timer.start(m_tStart);
		m_way = Way::FORWARD;
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

		setState(State::RUNBALL);
	}
}

void rProve::onRunBall()
{
	if (checkCommand()) {
		return;
	}

	if (!m_timer.isStarted()) {
		if (m_setup.Value & Setup::NOVALVE) {
			m_timer.start(m_tValve);
		} else {
			setState(State::VALVETOUP);
			return;
		}
	}

	if (m_timer.isFinished()) {
		m_timer.stop();

		setState(m_way == Way::FORWARD ? State::WAITD1 : State::WAITD1_REVERSE);
		return;
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
		setState(State::RUNBALL);
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

	if (isValveOpened() || isSimulate()) {
		setState(State::VALVETODOWN);
		return;
	}

	if (!isSimulate()) {
		m_close.m_value = 0;
		m_open.m_value  = 1;
	}
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

		if (isValveOpened() || isSimulate()) {
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
		if (!isSimulate()) {
			m_close.m_value = 1;
			m_open.m_value  = 0;
		}
		m_timer.start(m_tValve);
		return;
	}

	if(m_timer.isFinished()) {
		m_timer.stop();

		setState(State::ERRORTODOWN);
		return;
	}

	if (isValveClosed() || isSimulate()) {
		m_timer.stop();
		setState(m_way == Way::FORWARD ? State::WAITD1 : State::WAITD1_REVERSE);
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
		setState(m_way == Way::FORWARD ? State::ERRORD1 : State::ERRORD2_REVERSE);
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
		setState(m_way == Way::FORWARD ? State::WAITD2 : State::WAITD2_REVERSE);
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
		setState(m_way == Way::FORWARD ? State::ERRORD2 : State::ERRORD1_REVERSE);
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


void rProve::onReturnBall_NoValve()
{
	if (!m_timer.isStarted()) {
		m_timer.start(m_tValve);
		return;
	}

	if (m_timer.isFinished()) {
		m_timer.stop();
		setState(State::WAYCOMPLITED);
	}
}

void rProve::onReturnBall()
{
	if (checkCommand()) {
		return;
	}

	if (m_setup.Value & Setup::NOVALVE) {
		onReturnBall_NoValve();
		return;
	}

	if (!m_timer.isStarted()) {
		if (!isSimulate()) {
			m_close.m_value = 0;
			m_open.m_value  = 1;
		}
		m_timer.start(m_tValve);
		return;
	}

	if (m_timer.isFinished()) {
		m_timer.stop();
		setState(State::ERRORRETURN);
		return;
	}

	if (isValveOpened() || isSimulate()) {
		m_timer.stop();
		setState(State::WAYCOMPLITED);
		return;
	}
}

void rProve::onWayComplited()
{
	if (m_setup.Value & Setup::VALVE_4WAY) {
		if (m_way == Way::FORWARD) {
			m_way = Way::REVERSE;
			setState(State::RUNBALL);
			return;
		}
	}

	setState(State::FINISH);
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
			rEventManager::instance().add(reinitEvent(EID_PROVE_ISERROR));
			break;

		case Command::ABORT:
			setState(State::IDLE);
			break;

		case Command::RESET:
			m_state = State::IDLE;
			rEventManager::instance().add(reinitEvent(EID_PROVE_COMMANDRESET));
			break;

		default: break;
	}
}

bool rProve::checkCommand()
{
	switch(m_command) {
		case Command::START:
			rEventManager::instance().add(reinitEvent(EID_PROVE_ALREADYSTARTED));
			break;

		case Command::ABORT:
			rEventManager::instance().add(reinitEvent(EID_PROVE_COMMANDABORT));
			setState(State::IDLE);
			return true;

		case Command::RESET:
			rEventManager::instance().add(reinitEvent(EID_PROVE_NOTERROR));
			break;

		default: break;
	}

	return false;
}


DINT rProve::checkDetectors(bool first)
{
	bool det1 = m_fixDet & (rModuleCRM::Detector::Det1 | rModuleCRM::Detector::Det2);
	bool det2 = m_fixDet & (rModuleCRM::Detector::Det3 | rModuleCRM::Detector::Det4);

	if (m_setup.Value & Setup::ONEDETECTOR) {
		return det1 || det2;
	}

	if (m_way == Way::REVERSE) {
		bool temp = det1;
		det1 = det2;
		det2 = temp;
	}

	if (first) {
		if (det2) {
			return -1;
		}
		return det1;
	}

	if (det1) {
		return -1;
	}

	return det2;
}


bool rProve::checkStab(LREAL start, LREAL present, LREAL maxstab, UDINT eid)
{
	LREAL delta = std::fabs(present - start);

	if (delta > maxstab) {
		rEventManager::instance().add(reinitEvent(eid) << delta << maxstab);
		setState(State::ERRORSTAB);
		return true;
	}

	return false;
}


void rProve::clearAverage()
{
	m_fixDet  = 0;

	m_prvFreq = 0;
	m_prvTemp = 0;
	m_prvPres = 0;
	m_prvDens = 0;
	m_strTemp = 0;
	m_strPres = 0;
	m_strDens = 0;
	m_strKf   = 0;

	m_volume[D131].clear();
	m_volume[D242].clear();
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
		return false;
	}

	if (m_setup.Value & Setup::NOSELECTSTR) {
		return true;
	}

	m_station->setStreamFreqOut(m_strIdx.Value);
	return true;
}

void rProve::setState(State state)
{
	rEvent event;

	m_state = state;

	switch(m_state)
	{
		case State::IDLE:            m_inProgress.m_value = 0; event.reinit(EID_PROVE_STATE_IDLE);        break;
		case State::START:           m_inProgress.m_value = 0; event.reinit(EID_PROVE_STATE_START);       break;
		case State::STABILIZATION:   m_inProgress.m_value = 0; event.reinit(EID_PROVE_STATE_STAB);        break;
		case State::RUNBALL:         m_inProgress.m_value = 1; event.reinit(EID_PROVE_STATE_RUNBALL);     break;
		case State::VALVETOUP:       m_inProgress.m_value = 1; event.reinit(EID_PROVE_STATE_VALVETOUP);   break;
		case State::WAITTOUP:        m_inProgress.m_value = 1; event.reinit(EID_PROVE_STATE_WAITTOUP);    break;
		case State::VALVETODOWN:     m_inProgress.m_value = 1; event.reinit(EID_PROVE_STATE_VALVETODOWN); break;
		case State::WAITD1:          m_inProgress.m_value = 1; event.reinit(EID_PROVE_STATE_WAITD1);      break;
		case State::WAITD2:          m_inProgress.m_value = 1; event.reinit(EID_PROVE_STATE_WAITD2);      break;
		case State::CALCULATE:       m_inProgress.m_value = 1; event.reinit(EID_PROVE_STATE_CALCULATE);   break;
		case State::RETURNBALL:      m_inProgress.m_value = 1; event.reinit(EID_PROVE_STATE_RETURNBALL);  break;
		case State::WAITD1_REVERSE:  m_inProgress.m_value = 1; event.reinit(EID_PROVE_STATE_WAITED1_RVS); break;
		case State::WAITD2_REVERSE:  m_inProgress.m_value = 1; event.reinit(EID_PROVE_STATE_WAITED2_RVS); break;
		case State::WAYCOMPLITED:    m_inProgress.m_value = 1; event.reinit(EID_PROVE_STATE_WAYCOMPLTED); break;
		case State::FINISH:          m_inProgress.m_value = 0; event.reinit(EID_PROVE_STATE_FINISH);      break;
		case State::ABORT:           m_inProgress.m_value = 0; event.reinit(EID_PROVE_STATE_ABORT);       break;
		case State::ERRORFLOW:       m_inProgress.m_value = 0; event.reinit(EID_PROVE_STATE_ERRORFLOW);   break;
		case State::ERRORSTAB:       m_inProgress.m_value = 0; event.reinit(EID_PROVE_STATE_ERRORSTAB);   break;
		case State::ERRORTOUP:       m_inProgress.m_value = 0; event.reinit(EID_PROVE_STATE_ERRORTOUP);   break;
		case State::ERRORTODOWN:     m_inProgress.m_value = 0; event.reinit(EID_PROVE_STATE_ERRORTDOWN);  break;
		case State::ERRORD1:         m_inProgress.m_value = 0; event.reinit(EID_PROVE_STATE_ERRORD1);     break;
		case State::ERRORD2:         m_inProgress.m_value = 0; event.reinit(EID_PROVE_STATE_ERRORD2);     break;
		case State::ERRORDETECTOR:   m_inProgress.m_value = 0; event.reinit(EID_PROVE_STATE_ERRORDETS);   break;
		case State::ERRORRETURN:     m_inProgress.m_value = 0; event.reinit(EID_PROVE_STATE_ERRORRETURN); break;
		case State::ERRORSTREAMID:   m_inProgress.m_value = 0; event.reinit(EID_PROVE_STATE_ERRORRSTRID); break;
		case State::ERRORD1_REVERSE: m_inProgress.m_value = 0; event.reinit(EID_PROVE_STATE_ERRORD1_RVS); break;
		case State::ERRORD2_REVERSE: m_inProgress.m_value = 0; event.reinit(EID_PROVE_STATE_ERRORD2_RVS); break;
	}

	rEventManager::instance().add(event);
}

void rProve::calcAverage()
{
	if (!m_inProgress.m_value) {
		return;
	}

	UDINT dCount = m_moduleCount - m_averageCount;
	auto str = m_station->getStream(m_strIdx.Value);

	if (!str) {
		setState(State::ERRORSTREAMID);
		return;
	}

	LREAL str_temp = str->m_temp.m_value;
	LREAL str_pres = str->m_pres.m_value;
	LREAL str_dens = str->m_dens.m_value;
	LREAL str_kf   = str->m_curKF;

	if (dCount) {
		m_prvFreq = (m_moduleFreq   * dCount) + (m_prvFreq * m_averageCount) / static_cast<LREAL>(m_moduleCount);
		m_prvDens = (m_dens.m_value * dCount) + (m_prvDens * m_averageCount) / static_cast<LREAL>(m_moduleCount);
		m_prvTemp = (m_temp.m_value * dCount) + (m_prvTemp * m_averageCount) / static_cast<LREAL>(m_moduleCount);
		m_prvPres = (m_pres.m_value * dCount) + (m_prvPres * m_averageCount) / static_cast<LREAL>(m_moduleCount);
		m_strDens = (str_dens       * dCount) + (m_strDens * m_averageCount) / static_cast<LREAL>(m_moduleCount);
		m_strTemp = (str_temp       * dCount) + (m_strTemp * m_averageCount) / static_cast<LREAL>(m_moduleCount);
		m_strPres = (str_pres       * dCount) + (m_strPres * m_averageCount) / static_cast<LREAL>(m_moduleCount);
		m_strKf   = (str_kf         * dCount) + (m_strPres * m_averageCount) / static_cast<LREAL>(m_moduleCount);
	}
}

UDINT rProve::generateVars(rVariableList& list)
{
	rSource::generateVars(list);

	list.add(m_alias + ".command"                  , TYPE_UINT , rVariable::Flags::____, &m_command       , U_DIMLESS, ACCESS_PROVE, COMMENT::COMMAND + m_flagsCommand.getInfo(true));
	list.add(m_alias + ".setup"                    , TYPE_UINT , rVariable::Flags::___D, &m_setup.Value   , U_DIMLESS, ACCESS_PROVE, COMMENT::SETUP + m_flagsSetup.getInfo());
	list.add(m_alias + ".state"                    , TYPE_UINT , rVariable::Flags::R___, &m_state         , U_DIMLESS, 0           , COMMENT::STATUS + m_flagsState.getInfo(true));
	list.add(m_alias + ".stream_no"                , TYPE_UINT , rVariable::Flags::___D, &m_lineNum       , U_DIMLESS, 0           , "Номер ПР для поверки");
	list.add(m_alias + ".direction"                , TYPE_UINT , rVariable::Flags::R___, &m_way           , U_DIMLESS, 0           , "Проход шара:<br/>" + m_flagsWay.getInfo(true));
	list.add(m_alias + ".timer.start"              ,             rVariable::Flags::___D, &m_tStart        , U_msec   , ACCESS_PROVE, "Значение таймера выбора требуемого ПР");
	list.add(m_alias + ".timer.stabilization"      ,             rVariable::Flags::___D, &m_tStab         , U_msec   , ACCESS_PROVE, "Значение таймера стабилизации");
	list.add(m_alias + ".timer.detector1"          ,             rVariable::Flags::___D, &m_tD1           , U_msec   , ACCESS_PROVE, "Максимальное время прохода шара от корзины до первого детектора");
	list.add(m_alias + ".timer.detector2"          ,             rVariable::Flags::___D, &m_tD2           , U_msec   , ACCESS_PROVE, "Максимальное время прохода шара от второго детектора до корзины");
	list.add(m_alias + ".timer.volume"             ,             rVariable::Flags::___D, &m_tVolume       , U_msec   , ACCESS_PROVE, "Максимальное время прохода шара от первого детектора до второго");
	list.add(m_alias + ".timer.valve"              ,             rVariable::Flags::___D, &m_tValve        , U_msec   , ACCESS_PROVE, "Значение таймера поворота корзины");
	list.add(m_alias + ".timer.bounce"             ,             rVariable::Flags::___D, &m_tBounce       , U_msec   , ACCESS_PROVE, "Значение таймера анти-дребезга");
	list.add(m_alias + ".result.prove.frequency"   ,             rVariable::Flags::R___, &m_prvFreq       , U_Hz     , 0           , "Средневзвешанное значение частоты во время процедуры");
	list.add(m_alias + ".result.prove.temperature" ,             rVariable::Flags::R___, &m_prvTemp       , U_C      , 0           , "Средневзвешанное значение температуры ПУ во время процедуры");
	list.add(m_alias + ".result.prove.pressure"    ,             rVariable::Flags::R___, &m_prvPres       , U_MPa    , 0           , "Средневзвешанное значение давления ПУ во время процедуры");
	list.add(m_alias + ".result.prove.density"     ,             rVariable::Flags::R___, &m_prvDens       , U_kg_m3  , 0           , "Средневзвешанное значение плотности ПУ во время процедуры");
	list.add(m_alias + ".result.stream.temperature",             rVariable::Flags::R___, &m_strTemp       , U_C      , 0           , "Средневзвешанное значение температуры ПР во время процедуры");
	list.add(m_alias + ".result.stream.pressure"   ,             rVariable::Flags::R___, &m_strPres       , U_MPa    , 0           , "Средневзвешанное значение давления ПР во время процедуры");
	list.add(m_alias + ".result.stream.density"    ,             rVariable::Flags::R___, &m_strDens       , U_kg_m3  , 0           , "Средневзвешанное значение плотности ПР во время процедуры");
	list.add(m_alias + ".result.stream.kf"         ,             rVariable::Flags::R___, &m_strKf         , U_DIMLESS, 0           , "Средневзвешанное значение К-фактора ПР во время процедуры");
	list.add(m_alias + ".detectors.present"        ,             rVariable::Flags::R___, &m_curDet        , U_DIMLESS, 0           , "Мгновенное значение флагов детекторов:<br/>" + m_flagsDetectors.getInfo());
	list.add(m_alias + ".detectors.fixed"          ,             rVariable::Flags::R___, &m_fixDet        , U_DIMLESS, 0           , "Зафиксированное значение флагов детекторов:<br/>" + m_flagsDetectors.getInfo());
	list.add(m_alias + ".stabilization.temperature",             rVariable::Flags::___D, &m_maxStabTemp   , U_C      , ACCESS_PROVE, "Предельное значение разности температур во время стабилизации");
	list.add(m_alias + ".stabilization.pressure"   ,             rVariable::Flags::___D, &m_maxStabPres   , U_MPa    , ACCESS_PROVE, "Предельное значение разности давления во время стабилизации");
	list.add(m_alias + ".stabilization.density"    ,             rVariable::Flags::___D, &m_maxStabDens   , U_kg_m3  , ACCESS_PROVE, "Предельное значение разности плотности во время стабилизации");
	list.add(m_alias + ".stabilization.frequency"  ,             rVariable::Flags::___D, &m_maxStabFreq   , U_Hz     , ACCESS_PROVE, "Предельное значение разности частот ПР во время стабилизации");

	list.add(m_alias + ".result.volume1.count" , rVariable::Flags::R___, &m_volume[D131].m_summ.m_count   , U_imp, 0, "Количество импульсов для объема 1-3-1");
	list.add(m_alias + ".result.volume1.time"  , rVariable::Flags::R___, &m_volume[D131].m_summ.m_time    , U_sec, 0, "Время прохода шара 1-3-1");
	list.add(m_alias + ".result.volume1.fcount", rVariable::Flags::R___, &m_volume[D131].m_forward.m_count, U_imp, 0, "Количество импульсов для объема 1-3");
	list.add(m_alias + ".result.volume1.ftime" , rVariable::Flags::R___, &m_volume[D131].m_forward.m_time , U_sec, 0, "Время прохода шара 1-3");
	list.add(m_alias + ".result.volume1.rcount", rVariable::Flags::R___, &m_volume[D131].m_reverse.m_count, U_imp, 0, "Количество импульсов для объема 3-1");
	list.add(m_alias + ".result.volume1.rtime" , rVariable::Flags::R___, &m_volume[D131].m_reverse.m_time , U_sec, 0, "Время прохода шара 3-1");
	list.add(m_alias + ".result.volume2.count" , rVariable::Flags::R___, &m_volume[D242].m_summ.m_count   , U_imp, 0, "Количество импульсов для объема 2-4-2");
	list.add(m_alias + ".result.volume2.time"  , rVariable::Flags::R___, &m_volume[D242].m_summ.m_time    , U_sec, 0, "Время прохода шара 2-4-2");
	list.add(m_alias + ".result.volume2.fcount", rVariable::Flags::R___, &m_volume[D242].m_forward.m_count, U_imp, 0, "Количество импульсов для объема 2-4");
	list.add(m_alias + ".result.volume2.ftime" , rVariable::Flags::R___, &m_volume[D242].m_forward.m_time , U_sec, 0, "Время прохода шара 2-4");
	list.add(m_alias + ".result.volume2.rcount", rVariable::Flags::R___, &m_volume[D242].m_reverse.m_count, U_imp, 0, "Количество импульсов для объема 4-2");
	list.add(m_alias + ".result.volume2.rtime" , rVariable::Flags::R___, &m_volume[D242].m_reverse.m_time , U_sec, 0, "Время прохода шара 4-2");

	list.add(m_alias + ".fault", rVariable::Flags::R___, &m_fault, U_DIMLESS, 0, COMMENT::FAULT);

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

	auto xml_temp = element->FirstChildElement(XmlName::TEMP);
	auto xml_pres = element->FirstChildElement(XmlName::PRES);
	auto xml_dens = element->FirstChildElement(XmlName::DENSITY);

	if (xml_temp) if (TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(xml_temp->FirstChildElement(XmlName::LINK), m_temp)) return err.getError();
	if (xml_pres) if (TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(xml_temp->FirstChildElement(XmlName::LINK), m_pres)) return err.getError();
	if (xml_dens) if (TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(xml_temp->FirstChildElement(XmlName::LINK), m_dens)) return err.getError();

	auto xml_valve = element->FirstChildElement(XmlName::VALVE);

	if (xml_valve) {
		auto xml_opened = xml_valve->FirstChildElement(XmlName::OPENED);
		auto xml_closed = xml_valve->FirstChildElement(XmlName::CLOSED);

		if (xml_opened) if (TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(xml_opened->FirstChildElement(XmlName::LINK), m_opened)) return err.getError();
		if (xml_closed) if (TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(xml_closed->FirstChildElement(XmlName::LINK), m_closed)) return err.getError();
	}

	auto xml_timers = element->FirstChildElement(XmlName::TIMERS);

	if (xml_timers) {
		UDINT fault;

		m_tStart  = XmlUtils::getTextUDINT(xml_timers->FirstChildElement(XmlName::START)     , m_tStart , fault);
		m_tStab   = XmlUtils::getTextUDINT(xml_timers->FirstChildElement(XmlName::STABILISE) , m_tStab  , fault);
		m_tD1     = XmlUtils::getTextUDINT(xml_timers->FirstChildElement(XmlName::DETECTOR_1), m_tD1    , fault);
		m_tD2     = XmlUtils::getTextUDINT(xml_timers->FirstChildElement(XmlName::DETECTOR_2), m_tD2    , fault);
		m_tVolume = XmlUtils::getTextUDINT(xml_timers->FirstChildElement(XmlName::VOLUME)    , m_tVolume, fault);
		m_tValve  = XmlUtils::getTextUDINT(xml_timers->FirstChildElement(XmlName::VALVE)     , m_tValve , fault);
		m_tAbort  = XmlUtils::getTextUDINT(xml_timers->FirstChildElement(XmlName::ABORT)     , m_tAbort , fault);
		m_tBounce = XmlUtils::getTextUDINT(xml_timers->FirstChildElement(XmlName::BOUNCE)    , m_tBounce, fault);
	}

	auto xml_stab = element->FirstChildElement(XmlName::STABILISE);

	if (xml_stab) {
		UDINT fault;

		m_maxStabTemp = XmlUtils::getTextLREAL(xml_timers->FirstChildElement(XmlName::TEMP)   , m_maxStabTemp, fault);
		m_maxStabPres = XmlUtils::getTextLREAL(xml_timers->FirstChildElement(XmlName::PRES)   , m_maxStabPres, fault);
		m_maxStabDens = XmlUtils::getTextLREAL(xml_timers->FirstChildElement(XmlName::DENSITY), m_maxStabDens, fault);
		m_maxStabFreq = XmlUtils::getTextLREAL(xml_timers->FirstChildElement(XmlName::FREQ)   , m_maxStabFreq, fault);
	}

	m_open.m_limit.m_setup.Init(rLimit::Setup::OFF);
	m_close.m_limit.m_setup.Init(rLimit::Setup::OFF);
	m_opened.m_limit.m_setup.Init(rLimit::Setup::OFF);
	m_closed.m_limit.m_setup.Init(rLimit::Setup::OFF);
	m_inProgress.m_limit.m_setup.Init(rLimit::Setup::OFF);

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
	m_inProgress.m_limit.m_setup.Init(rLimit::Setup::OFF);

	md.add(this, false, rGeneratorMD::Type::IOMDULE)
			.addProperty(XmlName::SETUP, &m_flagsSetup)
			.addLink(XmlName::TEMP, true)
			.addLink(XmlName::PRES, true)
			.addLink(XmlName::DENSITY, true)
			.addXml(String_format("<%s>", XmlName::VALVE))
			.addLink(XmlName::OPENED, false, "\t")
			.addLink(XmlName::CLOSED, false, "\t")
			.addXml(String_format("</%s>", XmlName::VALVE))
			.addXml(String_format("<%s>", XmlName::TIMERS), true)
			.addXml(XmlName::START     , m_tStart , false, "\t")
			.addXml(XmlName::STABILISE , m_tStab  , false, "\t")
			.addXml(XmlName::DETECTOR_1, m_tD1    , false, "\t")
			.addXml(XmlName::DETECTOR_2, m_tD2    , false, "\t")
			.addXml(XmlName::VOLUME    , m_tVolume, false, "\t")
			.addXml(XmlName::VALVE     , m_tValve , false, "\t")
			.addXml(XmlName::ABORT     , m_tAbort , false, "\t")
			.addXml(XmlName::BOUNCE    , m_tBounce, false, "\t")
			.addXml(String_format("</%s>", XmlName::TIMERS), false)
			.addXml(String_format("<%s>", XmlName::STABILISE), true)
			.addXml(XmlName::TEMP   , m_maxStabTemp, false, "\t")
			.addXml(XmlName::PRES   , m_maxStabPres, false, "\t")
			.addXml(XmlName::DENSITY, m_maxStabDens, false, "\t")
			.addXml(XmlName::FREQ   , m_maxStabFreq, false, "\t")
			.addXml(String_format("</%s>", XmlName::STABILISE), false)
			;

	return TRITONN_RESULT_OK;
}

void rProve::moduleStart()
{
	rSnapshot ss(rDataManager::instance().getVariableClass(), 0xFFFFFFFF);

	ss.add(m_moduleName + ".start", 1);
	ss.set();

	m_averageCount  = m_moduleCount;
}

void rProve::moduleStop()
{
	rSnapshot ss(rDataManager::instance().getVariableClass(), 0xFFFFFFFF);

	ss.add(m_moduleName + ".stop", 1);
	ss.set();
}



