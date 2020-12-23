//=================================================================================================
//===
//=== data_counter.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
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


const UDINT FI_BAD_COUNT     = 0x10000000;
const LREAL FI_BAD_SPLINE    = -1.0;

//const UDINT FI_LE_SIM_AUTO   = 0x00000002;
//const UDINT FI_LE_SIM_MANUAL = 0x00000004;
//const UDINT FI_LE_SIM_OFF    = 0x00000008;
//const UDINT FI_LE_SIM_LAST   = 0x00000010;
const UDINT FI_LE_CODE_FAULT = 0x00000001;

rBitsArray rCounter::m_flagsSetup;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
rCounter::rCounter() : m_setup(Setup::OFF)
{
	if (m_flagsSetup.empty()) {
		m_flagsSetup
				.add("OFF"      , static_cast<UDINT>(Setup::OFF));
	}

	LockErr     = 0;
	m_countPrev = 0;
	m_tickPrev  = 0;

	InitLink(rLink::Setup::OUTPUT, m_impulse, U_imp  , SID::IMPULSE  , XmlName::IMPULSE, rLink::SHADOW_NONE);
	InitLink(rLink::Setup::OUTPUT, m_freq   , U_Hz   , SID::FREQUENCY, XmlName::FREQ   , rLink::SHADOW_NONE);
	InitLink(rLink::Setup::OUTPUT, m_period , U_mksec, SID::PERIOD   , XmlName::PERIOD , rLink::SHADOW_NONE);
}


rCounter::~rCounter()
{
	;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rCounter::InitLimitEvent(rLink &link)
{
	link.Limit.EventChangeAMin  = ReinitEvent(EID_COUNTER_NEW_AMIN)  << link.Descr << link.Unit;
	link.Limit.EventChangeWMin  = ReinitEvent(EID_COUNTER_NEW_WMIN)  << link.Descr << link.Unit;
	link.Limit.EventChangeWMax  = ReinitEvent(EID_COUNTER_NEW_WMAX)  << link.Descr << link.Unit;
	link.Limit.EventChangeAMax  = ReinitEvent(EID_COUNTER_NEW_AMAX)  << link.Descr << link.Unit;
	link.Limit.EventChangeHyst  = ReinitEvent(EID_COUNTER_NEW_HYST)  << link.Descr << link.Unit;
	link.Limit.EventChangeSetup = ReinitEvent(EID_COUNTER_NEW_SETUP) << link.Descr << link.Unit;
	link.Limit.EventAMin        = ReinitEvent(EID_COUNTER_AMIN)      << link.Descr << link.Unit;
	link.Limit.EventWMin        = ReinitEvent(EID_COUNTER_WMIN)      << link.Descr << link.Unit;
	link.Limit.EventWMax        = ReinitEvent(EID_COUNTER_WMAX)      << link.Descr << link.Unit;
	link.Limit.EventAMax        = ReinitEvent(EID_COUNTER_AMAX)      << link.Descr << link.Unit;
	link.Limit.EventNan         = ReinitEvent(EID_COUNTER_NAN)       << link.Descr << link.Unit;
	link.Limit.EventNormal      = ReinitEvent(EID_COUNTER_NORMAL)    << link.Descr << link.Unit;

	return 0;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rCounter::Calculate()
{
	rEvent event_fault;
	rEvent event_success;
	
	if(rSource::Calculate()) return 0;

	// Если сигнал выключен, то выходим
	if(m_setup.Value & Setup::OFF)
	{
		//lStatusCh = OFAISTATUSCH_OK;		
		m_count         = 0;
		m_isInit        = false;
		m_freq.Value    = 0.0;
		m_period.Value  = 0.0;
		m_impulse.Value = 0.0;

		PostCalculate();

		return TRITONN_RESULT_OK;
	}

	Fault = false;

	if(isSetModule()) {
		auto channel_ptr = rIOManager::instance().getChannel(m_module, m_channel);
		auto channel     = static_cast<rIOFIChannel*>(channel_ptr.get());

		if (channel == nullptr)
		{
			rEventManager::instance().Add(ReinitEvent(EID_COUNTER_MODULE) << m_module << m_channel);
			rDataManager::instance().DoHalt(HALT_REASON_RUNTIME | DATACFGERR_REALTIME_MODULELINK);
			return DATACFGERR_REALTIME_MODULELINK;
		}

		CheckExpr(channel->m_state, FI_LE_CODE_FAULT, event_fault.Reinit(EID_COUNTER_CH_FAULT) << ID << Descr, event_success.Reinit(EID_COUNTER_CH_OK) << ID << Descr);

		Fault = channel->m_state;

		if (channel->m_state) {
			m_period.Value  = std::numeric_limits<LREAL>::quiet_NaN();
			m_freq.Value    = std::numeric_limits<LREAL>::quiet_NaN();
			m_impulse.Value = std::numeric_limits<LREAL>::quiet_NaN();

		} else {
			UDINT count = channel->getValue();
			UDINT tick  = rTickCount::SysTick();

			if (!m_isInit) {
				m_impulse.Value = 0;
				m_freq.Value    = 0.0;
				m_period.Value  = 0.0;
				m_countPrev = count;
				m_tickPrev  = tick;
				m_isInit    = true;
			} else {
				m_impulse.Value = count - m_countPrev;
				m_freq.Value    = m_impulse.Value * 1000.0 / (static_cast<LREAL>(tick - m_tickPrev));
				m_period.Value  = m_freq.Value > 0.1 ? 1000000.0 / m_freq.Value : 0.0;
				m_countPrev     = count;
				m_tickPrev      = tick;
			}

			//TODO В теории тут нужно еще усреднить частоту и возможно период
		}
	}

	PostCalculate();
	
	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rCounter::generateVars(rVariableList& list)
{
	rSource::generateVars(list);

	// Variables
	list.add(Alias + ".count"   , TYPE_UINT , rVariable::Flags::R_H_, &m_count      , U_DIMLESS, 0);
	list.add(Alias + ".setup"   , TYPE_UINT , rVariable::Flags::RS_L, &m_setup.Value, U_DIMLESS, ACCESS_SA);

	list.add(Alias + ".fault"   , TYPE_UDINT, rVariable::Flags::R___, &Fault        , U_DIMLESS, 0);

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rCounter::LoadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix)
{
	std::string strSetup = XmlUtils::getAttributeString(element, XmlName::SETUP, m_flagsSetup.getNameByBits(Setup::OFF));

	if (TRITONN_RESULT_OK != rSource::LoadFromXML(element, err, prefix)) {
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

	ReinitLimitEvents();

	return TRITONN_RESULT_OK;
}


std::string rCounter::saveKernel(UDINT isio, const string &objname, const string &comment, UDINT isglobal)
{
	m_impulse.Limit.m_setup.Init(rLimit::Setup::NONE);
	m_freq.Limit.m_setup.Init(rLimit::Setup::NONE);
	m_period.Limit.m_setup.Init(rLimit::Setup::NONE);

	return rSource::saveKernel(isio, objname, comment, isglobal);
}

