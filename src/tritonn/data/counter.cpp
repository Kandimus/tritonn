/*
 *
 * data/counter.cpp
 *
 * Copyright (c) 2019-2021 by RangeSoft.
 * All rights reserved.
 *
 * Litvinov "VeduN" Vitaliy O.
 *
 */

#include <vector>
#include <limits>
#include <string.h>
#include "tickcount.h"
#include "xml_util.h"
#include "../error.h"
#include "../event/eid.h"
#include "../event/manager.h"
#include "../text_id.h"
#include "../data_manager.h"
#include "../variable_list.h"
#include "../io/manager.h"
#include "../io/fiinterface.h"
#include "../generator_md.h"
#include "../comment_defines.h"
#include "counter.h"

const UDINT FI_LE_CODE_FAULT = 0x00000001;

rBitsArray rCounter::m_flagsSetup;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
rCounter::rCounter(const rStation* owner) : rSource(owner), m_setup(Setup::OFF)
{
	if (m_flagsSetup.empty()) {
		m_flagsSetup
				.add("OFF"    , static_cast<UINT>(Setup::OFF)    , "Отключить обработку сигнала");
	}

	m_lockErr   = 0;
	m_countPrev = 0;
	m_tickPrev  = 0;

	initLink(rLink::Setup::OUTPUT | rLink::Setup::MUSTVIRT, m_impulse, U_imp  , SID::IMPULSE  , XmlName::IMPULSE, rLink::SHADOW_NONE);
	initLink(rLink::Setup::OUTPUT | rLink::Setup::MUSTVIRT, m_freq   , U_Hz   , SID::FREQUENCY, XmlName::FREQ   , rLink::SHADOW_NONE);
	initLink(rLink::Setup::OUTPUT | rLink::Setup::MUSTVIRT, m_period , U_mksec, SID::PERIOD   , XmlName::PERIOD , rLink::SHADOW_NONE);
}


//-------------------------------------------------------------------------------------------------
//
UDINT rCounter::initLimitEvent(rLink &link)
{
	link.m_limit.EventChangeAMin  = reinitEvent(EID_COUNTER_NEW_AMIN)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeWMin  = reinitEvent(EID_COUNTER_NEW_WMIN)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeWMax  = reinitEvent(EID_COUNTER_NEW_WMAX)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeAMax  = reinitEvent(EID_COUNTER_NEW_AMAX)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeHyst  = reinitEvent(EID_COUNTER_NEW_HYST)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeSetup = reinitEvent(EID_COUNTER_NEW_SETUP) << link.m_descr << link.m_unit;
	link.m_limit.EventAMin        = reinitEvent(EID_COUNTER_AMIN)      << link.m_descr << link.m_unit;
	link.m_limit.EventWMin        = reinitEvent(EID_COUNTER_WMIN)      << link.m_descr << link.m_unit;
	link.m_limit.EventWMax        = reinitEvent(EID_COUNTER_WMAX)      << link.m_descr << link.m_unit;
	link.m_limit.EventAMax        = reinitEvent(EID_COUNTER_AMAX)      << link.m_descr << link.m_unit;
	link.m_limit.EventNan         = reinitEvent(EID_COUNTER_NAN)       << link.m_descr << link.m_unit;
	link.m_limit.EventNormal      = reinitEvent(EID_COUNTER_NORMAL)    << link.m_descr << link.m_unit;

	return 0;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rCounter::calculate()
{
	rEvent event_f;
	rEvent event_s;
	
	if (rSource::calculate()) {
		return TRITONN_RESULT_OK;
	}

	// Если сигнал выключен, то выходим
	if (m_setup.Value & Setup::OFF) {
		//lStatusCh = OFAISTATUSCH_OK;		
		m_count           = 0;
		m_isInit          = false;
		m_freq.m_value    = 0.0;
		m_period.m_value  = 0.0;
		m_impulse.m_value = 0.0;

		postCalculate();

		return TRITONN_RESULT_OK;
	}

	m_fault = false;

	if(isSetModule()) {
		auto interface = dynamic_cast<rIOFIInterface*>(rIOManager::instance().getModuleInterface(m_module, rIOBaseModule::Type::UNDEF));

		if (!interface) {
			rEventManager::instance().add(reinitEvent(EID_COUNTER_MODULE) << m_module << m_channel);
			rDataManager::instance().DoHalt(HaltReason::RUNTIME, DATACFGERR_REALTIME_MODULELINK);
			return DATACFGERR_REALTIME_MODULELINK;
		}

		UDINT fault = 0;
		UDINT count = interface->getValue(m_channel, rIOBaseChannel::Type::FI, fault);
		LREAL freq  = interface->getFreq (m_channel, rIOBaseChannel::Type::FI, fault);
		UDINT tick  = rTickCount::SysTick();

		if (fault != TRITONN_RESULT_OK) {
			rEventManager::instance().add(reinitEvent(EID_PROVE_MODULE) << m_module << m_channel);
			rDataManager::instance().DoHalt(HaltReason::RUNTIME, fault);
			return fault;
		}

		if (!m_isInit) {
			m_impulse.m_value = 0;
			m_freq.m_value    = 0.0;
			m_period.m_value  = 0.0;
			m_countPrev       = count;
			m_tickPrev        = tick;
			m_isInit          = true;
		} else {
			UDINT curpulling = interface->getPulling(); //TODO это делать только в симуляторе

			if (m_pullingCount != curpulling) {
				m_impulse.m_value = count - m_countPrev;
				m_freq.m_value    = freq;
				m_period.m_value  = getPeriod();
				m_countPrev       = count;
				m_tickPrev        = tick;
				m_pullingCount    = curpulling;
			}
		}
	}

	postCalculate();
	
	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rCounter::generateVars(rVariableList& list)
{
	rSource::generateVars(list);

	// Variables
	list.add(m_alias + ".count", rVariable::Flags::R_H_, &m_count      , U_DIMLESS, 0        , "Счетчик импульсов");
	list.add(m_alias + ".setup", rVariable::Flags::RS_D, &m_setup.Value, U_DIMLESS, ACCESS_SA, COMMENT::SETUP + m_flagsSetup.getInfo());

	list.add(m_alias + ".fault", rVariable::Flags::R___, &m_fault      , U_DIMLESS, 0        , COMMENT::FAULT);

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rCounter::loadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix)
{
	std::string strSetup = XmlUtils::getAttributeString(element, XmlName::SETUP, m_flagsSetup.getNameByBits(Setup::OFF));

	if (TRITONN_RESULT_OK != rSource::loadFromXML(element, err, prefix)) {
		return err.getError();
	}

	tinyxml2::XMLElement* xml_module = element->FirstChildElement(XmlName::IOLINK);

	// Если аналоговый сигнал не привязан к каналу, то разрешаем менять его значение
	if (xml_module) {
		if (rDataModule::loadFromXML(xml_module, err) != TRITONN_RESULT_OK) {
			return err.getError();
		}
	} else {
		m_impulse.m_setup |= rLink::Setup::WRITABLE;
		m_freq.m_setup    |= rLink::Setup::WRITABLE;
		m_period.m_setup  |= rLink::Setup::WRITABLE;
	}

	UDINT fault = 0;
	m_setup.Init(m_flagsSetup.getValue(strSetup, fault));
	if (fault) {
		return err.set(DATACFGERR_FI, element->GetLineNum(), "setup fault");
	}

	reinitLimitEvents();

	return TRITONN_RESULT_OK;
}


UDINT rCounter::generateMarkDown(rGeneratorMD& md)
{
	m_impulse.m_limit.m_setup.Init(rLimit::Setup::HIHI | rLimit::Setup::HI | rLimit::Setup::LO | rLimit::Setup::LOLO);
	m_freq.m_limit.m_setup.Init(rLimit::Setup::HIHI | rLimit::Setup::HI | rLimit::Setup::LO | rLimit::Setup::LOLO);
	m_period.m_limit.m_setup.Init(rLimit::Setup::HIHI | rLimit::Setup::HI | rLimit::Setup::LO | rLimit::Setup::LOLO);

	md.add(this, true, rGeneratorMD::Type::IOCHANNEL_OPT)
			.addProperty(XmlName::SETUP, &m_flagsSetup);

	return TRITONN_RESULT_OK;
}

LREAL rCounter::getPeriod()
{
	return m_freq.m_value > 0.1 ? 1000000.0 / m_freq.m_value : 0.0;
}
