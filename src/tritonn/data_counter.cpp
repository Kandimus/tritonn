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
#include "data_config.h"
#include "data_counter.h"
#include "xml_util.h"


const UDINT FI_BAD_COUNT     = 0x10000000;
const LREAL FI_BAD_SPLINE    = -1.0;

const UDINT FI_LE_STATUSPATH = 0x00000001;
const UDINT FI_LE_SIM_AUTO   = 0x00000002;
const UDINT FI_LE_SIM_MANUAL = 0x00000004;
const UDINT FI_LE_SIM_OFF    = 0x00000008;
const UDINT FI_LE_SIM_LAST   = 0x00000010;
const UDINT FI_LE_CODE_FAULT = 0x00000020;

rBitsArray rCounter::m_flagsSetup;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
rCounter::rCounter() : Setup(FI_SETUP_OFF)
{
	if (m_flagsSetup.empty()) {
		m_flagsSetup
				.add("OFF"      , FI_SETUP_OFF)
				.add("NOBUFFER" , FI_SETUP_NOBUFFER);
	}

	LockErr     = 0;
	LastCount   = FI_BAD_COUNT;
	CountTail   = 0;
	SetCount    = 0;

	for(UDINT ii = 0; ii < MAX_FI_SPLINE; ++ii)
	{
		Spline[ii] = FI_BAD_SPLINE;
	}

	InitLink(LINK_SETUP_OUTPUT, Impulse, U_imp  , SID_IMPULSE  , XmlName::IMPULSE, LINK_SHADOW_NONE);
	InitLink(LINK_SETUP_OUTPUT, Freq   , U_Hz   , SID_FREQUENCY, XmlName::FREQ   , LINK_SHADOW_NONE);
	InitLink(LINK_SETUP_OUTPUT, Period , U_mksec, SID_PERIOD   , XmlName::PERIOD , LINK_SHADOW_NONE);
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
UDINT rCounter::GetFault(void)
{	
	// Нужно получить значения статуса модуля и канала
	return ChFault;
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

	// Если аналоговый сигнал выключен, то выходим
	if(Setup.Value & FI_SETUP_OFF)
	{
		//lStatusCh = OFAISTATUSCH_OK;		
		Count         = 0;
		LastCount     = FI_BAD_COUNT;
		Freq.Value    = 0.0;
		Period.Value  = 0.0;
		Impulse.Value = 0.0;

		for(UDINT ii = 0; ii < MAX_FI_SPLINE; ++ii)
		{
			Spline[ii] = FI_BAD_SPLINE;
		}

		PostCalculate();

		return 0;
	}

	curtick = Tick.Timer();

	if(SetCount)
	{
		ChFault = (SetCount & 0x00010000) > 0;
	}

	// Получаем значение счетчика
	// Count

	CheckExpr(ChFault, COUNTER_LE_CODE_FAULT, event_fault.Reinit(EID_COUNTER_CH_OK) << ID << Descr, event_success.Reinit(EID_COUNTER_CH_OK) << ID << Descr);

	if(ChFault)
	{
		Period.Value  = std::numeric_limits<LREAL>::quiet_NaN();
		Freq.Value    = std::numeric_limits<LREAL>::quiet_NaN();
		Impulse.Value = std::numeric_limits<LREAL>::quiet_NaN();
	}
	else
	{
		// Если это первый запуск, или произошло включение сигнала
		if(LastCount == FI_BAD_COUNT)
		{
			Tick.Timer();

			LastCount     = Count;
			Freq.Value    = 0.0;
			Period.Value  = 0.0;
			Impulse.Value = 0;
		}

		// Имитация, тестирование
		if(SetCount)
		{
			LREAL curimp = LREAL(SetCount & 0x0000FFFF) * (((LREAL)curtick) / 1000.0);

			Impulse.Value = UINT(curimp + CountTail);
			Freq.Value    = SetCount & 0x0000FFFF;
			CountTail     = (curimp + CountTail) - Impulse.Value;

//			printf("set: %u, tick: %u, imp: %.1f, freq: %.4f\n", SetCount, curtick, Impulse.Value, Freq.Value);
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

		if(!(Setup.Value & FI_SETUP_NOBUFFER))
		{
			LREAL spline_freq = Freq.Value;

			// Устредняем по последним "хорошим" значениям
			for(UDINT ii = 0; ii < MAX_FI_SPLINE; ++ii)
			{
				spline_freq += ((Spline[ii] <= -1) ? Freq.Value : Spline[ii]);
			}
			Freq.Value = spline_freq / (LREAL)(MAX_FI_SPLINE + 1);

			// Смещаем сплайн
			for(UDINT ii = 1; ii < MAX_FI_SPLINE; ++ii)
			{
				Spline[ii] = Spline[ii - 1];
			}
			Spline[0] = Freq.Value;
		}

		// Расчет периода
		Period.Value = (Freq.Value > 0.1) ? (1000000.0 / Freq.Value) : 0;
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
UDINT rCounter::LoadFromXML(tinyxml2::XMLElement *element, rDataConfig &cfg)
{
	std::string strSetup = XmlUtils::getAttributeString(element, XmlName::SETUP, m_flagsSetup.getNameByBits(FI_SETUP_OFF));
	UDINT  err      = 0;

	if(TRITONN_RESULT_OK != rSource::LoadFromXML(element, cfg)) return 1;

	Setup.Init(m_flagsSetup.getValue(strSetup, err));
	if(err) return 1;

	ReinitLimitEvents();

	return tinyxml2::XML_SUCCESS;
}


std::string rCounter::saveKernel(UDINT isio, const string &objname, const string &comment, UDINT isglobal)
{
	Impulse.Limit.m_setup.Init(rLimit::Setup::NONE);
	Freq.Limit.m_setup.Init(rLimit::Setup::NONE);
	Period.Limit.m_setup.Init(rLimit::Setup::NONE);

	return rSource::saveKernel(isio, objname, comment, isglobal);
}

