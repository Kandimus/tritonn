//=================================================================================================
//===
//=== data_counter.cpp
//===
//=== Copyright (c) 2019-2021 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс частотного входного сигнала (FI)
//===
//=================================================================================================

#include "data_counter.h"
#include <vector>
#include <limits>
#include <string.h>
#include "tinyxml2.h"
#include "error.h"
#include "data_selector.h"
#include "event_eid.h"
#include "text_id.h"
#include "event_manager.h"
#include "data_manager.h"
#include "variable_item.h"
#include "variable_list.h"
#include "io/manager.h"
#include "io/fi_channel.h"
#include "tickcount.h"
#include "xml_util.h"
#include "generator_md.h"
#include "comment_defines.h"

const UDINT FI_LE_CODE_FAULT = 0x00000001;

rBitsArray rCounter::m_flagsSetup;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
rCounter::rCounter(const rStation* owner) : rSource(owner), m_setup(Setup::OFF)
{
	if (m_flagsSetup.empty()) {
		m_flagsSetup
				.add("OFF"    , static_cast<UINT>(Setup::OFF)    , "Отключить обработку сигнала")
				.add("AVERAGE", static_cast<UINT>(Setup::AVERAGE), "Включить устреднение частоты");
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

		m_averageFreq.clear();

		postCalculate();

		return TRITONN_RESULT_OK;
	}

	m_fault = false;

	if(isSetModule()) {
		auto channel_ptr = rIOManager::instance().getChannel(m_module, m_channel);
		auto channel     = static_cast<rIOFIChannel*>(channel_ptr.get());

		if (channel == nullptr) {
			rEventManager::instance().Add(reinitEvent(EID_COUNTER_MODULE) << m_module << m_channel);
			rDataManager::instance().DoHalt(HALT_REASON_RUNTIME | DATACFGERR_REALTIME_MODULELINK);
			return DATACFGERR_REALTIME_MODULELINK;
		}

		checkExpr(channel->m_state, FI_LE_CODE_FAULT,
				  event_f.Reinit(EID_COUNTER_CH_FAULT) << m_ID << m_descr,
				  event_s.Reinit(EID_COUNTER_CH_OK)    << m_ID << m_descr);

		m_fault = channel->m_state;

		if (channel->m_state) {
			m_period.m_value  = std::numeric_limits<LREAL>::quiet_NaN();
			m_freq.m_value    = std::numeric_limits<LREAL>::quiet_NaN();
			m_impulse.m_value = std::numeric_limits<LREAL>::quiet_NaN();

		} else {
			UDINT count = channel->getValue();
			LREAL freq  = channel->getFreq();
			UDINT tick  = rTickCount::SysTick();

			if (!m_isInit) {
				m_impulse.m_value = 0;
				m_freq.m_value    = 0.0;
				m_period.m_value  = 0.0;
				m_countPrev       = count;
				m_tickPrev        = tick;
				m_isInit          = true;
			} else {
				if (m_pullingCount != channel->getPullingCount()) {
					m_impulse.m_value = count - m_countPrev;
					m_freq.m_value    = freq;
					m_period.m_value  = getPeriod();
					m_countPrev       = count;
					m_tickPrev        = tick;
					m_pullingCount    = channel->getPullingCount();

					if (m_setup.Value & Setup::AVERAGE) {
						m_averageFreq.push_back(m_freq.m_value);

						while (m_averageFreq.size() > AVERAGE_MAX) {
							m_averageFreq.pop_front();
						}

						LREAL average = 0.0;
						for (auto value : m_averageFreq) {
							average += value;
						}
						m_freq.m_value   = average / AVERAGE_MAX;
						m_period.m_value = getPeriod();
					}
				}
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
	list.add(m_alias + ".count", TYPE_UINT , rVariable::Flags::R_H, &m_count      , U_DIMLESS, 0        , "Счетчик импульсов");
	list.add(m_alias + ".setup", TYPE_UINT , rVariable::Flags::RS_, &m_setup.Value, U_DIMLESS, ACCESS_SA, COMMENT::SETUP + m_flagsSetup.getInfo());

	list.add(m_alias + ".fault", TYPE_UDINT, rVariable::Flags::R__, &m_fault      , U_DIMLESS, 0        , COMMENT::FAULT);

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

	md.add(this, true)
			.addProperty(XmlName::SETUP, &m_flagsSetup);

	return TRITONN_RESULT_OK;
}

LREAL rCounter::getPeriod()
{
	return m_freq.m_value > 0.1 ? 1000000.0 / m_freq.m_value : 0.0;
}
