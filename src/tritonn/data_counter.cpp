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
				.add("OFF"      , Setup::OFF);
	}

	LockErr     = 0;
	m_countLast = 0;

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
	UDINT  cnt     = 0; // Расширенный текущий счетчик с модуля
	UDINT  curtick = 0;
	rEvent event_fault;
	rEvent event_success;
	
	if(rSource::Calculate()) return 0;

	// Если сигнал выключен, то выходим
	if(Setup.Value & FI_SETUP_OFF)
	{
		//lStatusCh = OFAISTATUSCH_OK;		
		Count         = 0;
		LastCount     = FI_BAD_COUNT;
		Freq.Value    = 0.0;
		Period.Value  = 0.0;
		Impulse.Value = 0.0;

		PostCalculate();

		return TRITONN_RESULT_OK;
	}

	Fault = false;

	if(isSetModule()) {
		auto channel_ptr = rIOManager::instance().getChannel(m_module, m_channel);
		auto channel     = static_cast<rIOFIChannel*>(channel_ptr.get());

		if (channel == nullptr)
		{
			rEventManager::instance().Add(ReinitEvent(EID_AI_MODULE) << m_module << m_channel);
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

			}

			m_impulse.Value = count - m_countLast;
			m_countLast     = count;
			m_freq.Value    = m_impulse.Value * 1000.0 / ((LREAL)curtick);
			m_period.Value  = m_freq.Value > 0.1 ? 1000000.0 / m_freq.Value : 0.0;
		}
	}

	// Получаем значение счетчика
	// Count
	else
	{
		// Имитация, тестирование
		if(SetCount)
		{
			LREAL curimp = LREAL(SetCount & 0x0000FFFF) * (((LREAL)curtick) / 1000.0);

			Impulse.Value = UINT(curimp + CountTail);
			Freq.Value    = SetCount & 0x0000FFFF;
			CountTail     = (curimp + CountTail) - Impulse.Value;
		}
		// Данные с модуля
		else
		{
			// Расчитываем значение счетчика за период, с учетом переполнения
			//TODO Зачем это? если модуль будет выдавать в UINT, то вычитание пройдет нормально!!!!
			//TODO Если частота будет прыгать даже со сглаживаением, то сделать расчет частоты не чаще раз в секунду, через таймер и собственный накопитель импульсов
			cnt = Count;
			if(cnt < LastCount)
			{
				cnt += 0x00010000;
			}

			// Основные вычисления
//			if(cnt - LastCount) //TODO Так делать нельзя, нужен таймер 1 сек, на проверку отсутствия импульсов.
			{
				Impulse.Value = cnt - LastCount;
				LastCount     = Count;
				Freq.Value    = Impulse.Value * 1000.0 / ((LREAL)curtick);
			}
		}

		// Расчет периода

	}

	/*if(Alias == "sikn_123.bik.io.period1")
	{
		int a = 0;
		a = 1;

		printf("tick: %u, setcount: %u, count: %u, lastcount: %u, freq: %f, setup: %i\n", curtick, SetCount, Count, LastCount, Freq.Value, Setup.Value);
	}*/

	PostCalculate();
	
	return 1;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//




///////////////////////////////////////////////////////////////////////////////////////////////////
//



///////////////////////////////////////////////////////////////////////////////////////////////////
//

//-------------------------------------------------------------------------------------------------
//
UDINT rCounter::generateVars(rVariableList& list)
{
	rSource::generateVars(list);

	// Variables
	list.add(Alias + ".setcount", TYPE_UDINT, rVariable::Flags::RSH_, &SetCount   , U_DIMLESS, ACCESS_SA);
	list.add(Alias + ".Count"   , TYPE_UINT , rVariable::Flags::R_H_, &Count      , U_DIMLESS, 0);
	list.add(Alias + ".Setup"   , TYPE_UINT , rVariable::Flags::RS_L, &Setup.Value, U_DIMLESS, ACCESS_SA);

	list.add(Alias + ".fault"   , TYPE_UDINT, rVariable::Flags::R___, &Fault      , U_DIMLESS, 0);

	return 0;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rCounter::LoadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix)
{
	std::string strSetup = XmlUtils::getAttributeString(element, XmlName::SETUP, m_flagsSetup.getNameByBits(FI_SETUP_OFF));

	if (TRITONN_RESULT_OK != rSource::LoadFromXML(element, err, prefix)) {
		return err.getError();
	}

	UDINT fault = 0;
	Setup.Init(m_flagsSetup.getValue(strSetup, fault));
	if (fault) {
		return err.set(DATACFGERR_FI, element->GetLineNum(), "setup fault");
	}

	ReinitLimitEvents();

	return TRITONN_RESULT_OK;
}


std::string rCounter::saveKernel(UDINT isio, const string &objname, const string &comment, UDINT isglobal)
{
	Impulse.Limit.m_setup.Init(rLimit::Setup::NONE);
	Freq.Limit.m_setup.Init(rLimit::Setup::NONE);
	Period.Limit.m_setup.Init(rLimit::Setup::NONE);

	return rSource::saveKernel(isio, objname, comment, isglobal);
}

