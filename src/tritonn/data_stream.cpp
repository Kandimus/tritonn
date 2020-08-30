//=================================================================================================
//===
//=== data_stream.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс измерительной линии
//===
//=================================================================================================

#include <vector>
#include <limits>
#include <string.h>
#include "event_eid.h"
#include "text_id.h"
#include "event_manager.h"
#include "precision.h"
#include "data_manager.h"
#include "data_config.h"
#include "data_variable.h"
#include "data_station.h"
#include "data_stream.h"

//using std::vector;


const UDINT STREAM_LE_INPUTS     = 0x00000001;
const UDINT STREAM_LE_ACCOUNTING = 0x00000002;
const UDINT STREAM_LE_KEYPADKF   = 0x00000004;


///////////////////////////////////////////////////////////////////////////////////////////////////
//
rStream::rStream() : Setup(STR_SETUP_OFF)
{
//	Unit       = U_UNDEF;
	Maintenance   = true;
	Linearization = false;
//	rTotal   Total;

	InitLink(LINK_SETUP_INPUT   , Counter     , U_imp   , SID_IMPULSE          , CFGNAME_IMPULSE      , LINK_SHADOW_NONE);
	InitLink(LINK_SETUP_INPUT   , Freq        , U_Hz    , SID_FREQUENCY        , CFGNAME_FREQ         , CFGNAME_IMPULSE );
	InitLink(LINK_SETUP_INOUTPUT, Temp        , U_C     , SID_TEMPERATURE      , CFGNAME_TEMP         , LINK_SHADOW_NONE);
	InitLink(LINK_SETUP_INOUTPUT, Pres        , U_MPa   , SID_PRESSURE         , CFGNAME_PRES         , LINK_SHADOW_NONE);
	InitLink(LINK_SETUP_INOUTPUT, Dens        , U_kg_m3 , SID_DENSITY          , CFGNAME_DENSITY      , LINK_SHADOW_NONE);
	InitLink(LINK_SETUP_INOUTPUT, Dens15      , U_kg_m3 , SID_DENSITY15        , CFGNAME_DENSITY15    , CFGNAME_DENSITY );
	InitLink(LINK_SETUP_INOUTPUT, Dens20      , U_kg_m3 , SID_DENSITY20        , CFGNAME_DENSITY20    , CFGNAME_DENSITY );
	InitLink(LINK_SETUP_INOUTPUT, B15         , U_1_C   , SID_B15              , CFGNAME_B15          , CFGNAME_DENSITY );
	InitLink(LINK_SETUP_INOUTPUT, Y15         , U_1_MPa , SID_Y15              , CFGNAME_Y15          , CFGNAME_DENSITY );
	InitLink(LINK_SETUP_OUTPUT  , FlowMass    , U_t_h   , SID_FLOWRATE_MASS    , CFGNAME_FLOWRATEMASS , LINK_SHADOW_NONE);
	InitLink(LINK_SETUP_OUTPUT  , FlowVolume  , U_m3_h  , SID_FLOWRATE_VOLUME  , CFGNAME_FLOWRATEVOL  , LINK_SHADOW_NONE);
	InitLink(LINK_SETUP_OUTPUT  , FlowVolume15, U_m3_h  , SID_FLOWRATE_VOLUME15, CFGNAME_FLOWRATEVOL15, LINK_SHADOW_NONE);
	InitLink(LINK_SETUP_OUTPUT  , FlowVolume20, U_m3_h  , SID_FLOWRATE_VOLUME20, CFGNAME_FLOWRATEVOL20, LINK_SHADOW_NONE);
}


rStream::~rStream()
{
	;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rStream::InitLimitEvent(rLink &link)
{
	link.Limit.EventChangeAMin  = ReinitEvent(EID_STREAM_NEW_AMIN)  << link.Descr << link.Unit;
	link.Limit.EventChangeWMin  = ReinitEvent(EID_STREAM_NEW_WMIN)  << link.Descr << link.Unit;
	link.Limit.EventChangeWMax  = ReinitEvent(EID_STREAM_NEW_WMAX)  << link.Descr << link.Unit;
	link.Limit.EventChangeAMax  = ReinitEvent(EID_STREAM_NEW_AMAX)  << link.Descr << link.Unit;
	link.Limit.EventChangeHyst  = ReinitEvent(EID_STREAM_NEW_HYST)  << link.Descr << link.Unit;
	link.Limit.EventChangeSetup = ReinitEvent(EID_STREAM_NEW_SETUP) << link.Descr << link.Unit;
	link.Limit.EventAMin        = ReinitEvent(EID_STREAM_AMIN)      << link.Descr << link.Unit;
	link.Limit.EventWMin        = ReinitEvent(EID_STREAM_WMIN)      << link.Descr << link.Unit;
	link.Limit.EventWMax        = ReinitEvent(EID_STREAM_WMAX)      << link.Descr << link.Unit;
	link.Limit.EventAMax        = ReinitEvent(EID_STREAM_AMAX)      << link.Descr << link.Unit;
	link.Limit.EventNan         = ReinitEvent(EID_STREAM_NAN)       << link.Descr << link.Unit;
	link.Limit.EventNormal      = ReinitEvent(EID_STREAM_NORMAL)    << link.Descr << link.Unit;

	return 0;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rStream::GetFault(void)
{
	return 1;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rStream::Calculate()
{
	rEvent event_s;
	rEvent event_f;
//	UDINT  err = 0;

	if(rSource::Calculate()) return 0;

//	if(CheckExpr(err, STREAM_LE_INPUTS, event_f.Reinit(EID_STREAM_FAULT_INPUTS) << ID << Descr,
//													event_s.Reinit(EID_STREAM_GOOD_INPUTS ) << ID << Descr))
//	{
//		return Fault = 1;
//	}

	//-------------------------------------------------------------------------------------------
	//
	Factor.KeypadKF.Compare(ReinitEvent(EID_STREAM_KEYPADKF));
	Factor.KeypadMF.Compare(ReinitEvent(EID_STREAM_KEYPADMF));

	for(UDINT ii = 0; ii < MAX_FACTOR_POINT; ++ii)
	{
		Factor.Point[ii].Hz.Compare(ReinitEvent(EID_STREAM_POINT_HZ) << rStringID(U_Hz) << ii);
		Factor.Point[ii].Kf.Compare(ReinitEvent(EID_STREAM_POINT_KF) << GetUnitKF()     << ii);
	}

	// Вывод линии из учета
	CheckExpr(Maintenance, STREAM_LE_ACCOUNTING, event_f.Reinit(EID_STREAM_ACCOUNTING_ON ) << ID << Descr,
																event_s.Reinit(EID_STREAM_ACCOUNTING_OFF) << ID << Descr);

	// Использование ручного ввода KF
	CheckExpr(Linearization, STREAM_LE_KEYPADKF, event_f.Reinit(EID_STREAM_KEYPADKF_OFF) << ID << Descr,
																event_s.Reinit(EID_STREAM_KEYPADKF_ON ) << ID << Descr);

	// Ввод новых коэф-тов
	if(AcceptKF)
	{
		Factor   = SetFactor;
		AcceptKF = 0;
		rEventManager::Instance().Add(ReinitEvent(EID_STREAM_ACCEPT));
	}

	//-------------------------------------------------------------------------------------------
	// Расчет текущего коэф-та
	CurKF = (Linearization) ? CalcualateKF() : Factor.KeypadKF.Value;

	//-------------------------------------------------------------------------------------------
	// Нарастающие и расход
	// Увеличим глобальный счетчик
	UDINT check = CalcTotal();

	if(check & TOTAL_MAX_MASS    ) rEventManager::Instance().Add(ReinitEvent(EID_STREAM_TOTAL_MASS));
	if(check & TOTAL_MAX_VOLUME  ) rEventManager::Instance().Add(ReinitEvent(EID_STREAM_TOTAL_VOLUME));
	if(check & TOTAL_MAX_VOLUME15) rEventManager::Instance().Add(ReinitEvent(EID_STREAM_TOTAL_VOLUME15));
	if(check & TOTAL_MAX_VOLUME20) rEventManager::Instance().Add(ReinitEvent(EID_STREAM_TOTAL_VOLUME20));

	// Расчитаем расход исходя из частоты
	if(FlowMeter == STR_FLOWMETER_CARIOLIS)
	{
		FlowMass.Value   = Freq.Value     / CurKF      * 3600.0 * Factor.KeypadMF.Value;
		FlowVolume.Value = FlowMass.Value / Dens.Value * 1000.0 * Factor.KeypadMF.Value;
	}
	else
	{
		FlowVolume.Value = Freq.Value       / CurKF      * 3600.0 * Factor.KeypadMF.Value;
		FlowMass.Value   = FlowVolume.Value * Dens.Value / 1000.0 * Factor.KeypadMF.Value;
	}

	return 0;
}


//-------------------------------------------------------------------------------------------------
// Получение единиц измерения К-фактора (исходя из типа расходомера)
UDINT rStream::GetUnitKF()
{
	if(Station == nullptr) return U_UNDEF;

	if(FlowMeter == STR_FLOWMETER_CARIOLIS)
	{
		switch(Station->UnitMass)
		{
			case U_t : return U_imp_t;
			case U_kg: return U_imp_kg;
			default  : return U_UNDEF;
		}
	}
	else
	{
		switch(Station->UnitVolume)
		{
			case U_m3   : return U_imp_m3;
			case U_liter: return U_imp_ltr;
			default     : return U_UNDEF;
		}
	}
}



///////////////////////////////////////////////////////////////////////////////////////////////////
//
const rTotal *rStream::GetTotal(void)
{
	return &Total;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//



///////////////////////////////////////////////////////////////////////////////////////////////////
//


//-------------------------------------------------------------------------------------------------
//
UDINT rStream::GenerateVars(vector<rVariable *> &list)
{
	rSource::GenerateVars(list);

	// Внутренние переменные
	list.push_back(new rVariable(Alias + ".Setup"                 , TYPE_UINT , VARF_RS_L, &Setup.Value             , U_DIMLESS          , ACCESS_SA));
	list.push_back(new rVariable(Alias + ".FlowMeter"             , TYPE_USINT, VARF_R__L, &FlowMeter               , U_DIMLESS          , 0));
	list.push_back(new rVariable(Alias + ".Maintenance"           , TYPE_UDINT, VARF____L, &Maintenance             , U_DIMLESS          , ACCESS_MAINTENANCE));
	list.push_back(new rVariable(Alias + ".total.present.Volume"  , TYPE_LREAL, VARF_R___, &Total.Present.Volume    , Station->UnitVolume, 0));
	list.push_back(new rVariable(Alias + ".total.present.Volume15", TYPE_LREAL, VARF_R___, &Total.Present.Volume15  , Station->UnitVolume, 0));
	list.push_back(new rVariable(Alias + ".total.present.Volume20", TYPE_LREAL, VARF_R___, &Total.Present.Volume20  , Station->UnitVolume, 0));
	list.push_back(new rVariable(Alias + ".total.present.Mass"    , TYPE_LREAL, VARF_R___, &Total.Present.Mass      , Station->UnitMass  , 0));
	list.push_back(new rVariable(Alias + ".total.present.impulse" , TYPE_UDINT, VARF_R___, &Total.Present.Count     , U_imp              , 0));
	list.push_back(new rVariable(Alias + ".total.Inc.Volume"      , TYPE_LREAL, VARF_RSH_, &Total.Inc.Volume        , Station->UnitVolume, ACCESS_SA));
	list.push_back(new rVariable(Alias + ".total.Inc.Volume15"    , TYPE_LREAL, VARF_RSH_, &Total.Inc.Volume15      , Station->UnitVolume, ACCESS_SA));
	list.push_back(new rVariable(Alias + ".total.Inc.Volume20"    , TYPE_LREAL, VARF_RSH_, &Total.Inc.Volume20      , Station->UnitVolume, ACCESS_SA));
	list.push_back(new rVariable(Alias + ".total.Inc.Mass"        , TYPE_LREAL, VARF_RSH_, &Total.Inc.Mass          , Station->UnitMass  , ACCESS_SA));
	list.push_back(new rVariable(Alias + ".total.Inc.inpulse"     , TYPE_UDINT, VARF_RSH_, &Total.Inc.Count         , U_imp              , ACCESS_SA));
	list.push_back(new rVariable(Alias + ".total.raw.Volume"      , TYPE_LREAL, VARF_RSH_, &Total.Raw.Volume        , Station->UnitVolume, 0));
	list.push_back(new rVariable(Alias + ".total.raw.Volume15"    , TYPE_LREAL, VARF_RSH_, &Total.Raw.Volume15      , Station->UnitVolume, 0));
	list.push_back(new rVariable(Alias + ".total.raw.Volume20"    , TYPE_LREAL, VARF_RSH_, &Total.Raw.Volume20      , Station->UnitVolume, 0));
	list.push_back(new rVariable(Alias + ".total.raw.Mass"        , TYPE_LREAL, VARF_RSH_, &Total.Raw.Mass          , Station->UnitMass  , 0));
	list.push_back(new rVariable(Alias + ".total.raw.impulse"     , TYPE_UDINT, VARF_RSH_, &Total.Raw.Count         , U_imp              , 0));
	list.push_back(new rVariable(Alias + ".total.past.Volume"     , TYPE_LREAL, VARF_RSH_, &Total.Past.Volume       , Station->UnitVolume, 0));
	list.push_back(new rVariable(Alias + ".total.past.Volume15"   , TYPE_LREAL, VARF_RSH_, &Total.Past.Volume15     , Station->UnitVolume, 0));
	list.push_back(new rVariable(Alias + ".total.past.Volume20"   , TYPE_LREAL, VARF_RSH_, &Total.Past.Volume20     , Station->UnitVolume, 0));
	list.push_back(new rVariable(Alias + ".total.past.Mass"       , TYPE_LREAL, VARF_RSH_, &Total.Past.Mass         , Station->UnitMass  , 0));
	list.push_back(new rVariable(Alias + ".total.past.impulse"    , TYPE_UDINT, VARF_RSH_, &Total.Past.Count        , U_imp              , 0));
	list.push_back(new rVariable(Alias + ".acceptfactor"          , TYPE_UDINT, VARF____L, &AcceptKF                , U_DIMLESS          , ACCESS_FACTORS));
	list.push_back(new rVariable(Alias + ".presentkf"             , TYPE_LREAL, VARF_R___, &CurKF                   , GetUnitKF()        , 0));
	list.push_back(new rVariable(Alias + ".factor.kf"             , TYPE_LREAL, VARF_R___, &Factor.KeypadKF.Value   , GetUnitKF()        , 0));
	list.push_back(new rVariable(Alias + ".factor.mf"             , TYPE_LREAL, VARF_R__L, &Factor.KeypadMF.Value   , U_DIMLESS          , 0));
	list.push_back(new rVariable(Alias + ".linearization"         , TYPE_UDINT, VARF____L, &Linearization           , U_DIMLESS          , ACCESS_FACTORS));
	list.push_back(new rVariable(Alias + ".setfactor.kf"          , TYPE_LREAL, VARF____L, &SetFactor.KeypadKF.Value, GetUnitKF()        , ACCESS_FACTORS));
	list.push_back(new rVariable(Alias + ".setfactor.mf"          , TYPE_LREAL, VARF____L, &SetFactor.KeypadMF.Value, U_DIMLESS          , ACCESS_FACTORS));


	for(UDINT ii = 0; ii < MAX_FACTOR_POINT; ++ii)
	{
		string name_kf     = String_format("%s.factor.Point_%i.kf"   , Alias.c_str(), ii + 1);
		string name_hz     = String_format("%s.factor.Point_%i.hz"   , Alias.c_str(), ii + 1);
		string name_set_kf = String_format("%s.setfactor.point_%i.kf", Alias.c_str(), ii + 1);
		string name_set_hz = String_format("%s.setfactor.point_%i.hz", Alias.c_str(), ii + 1);

		list.push_back(new rVariable(name_kf    , TYPE_LREAL, VARF_R__L, &Factor.Point[ii].Kf   , GetUnitKF(), 0));
		list.push_back(new rVariable(name_set_kf, TYPE_LREAL, VARF_____, &SetFactor.Point[ii].Kf, GetUnitKF(), ACCESS_FACTORS));
		list.push_back(new rVariable(name_hz    , TYPE_LREAL, VARF_R__L, &Factor.Point[ii].Hz   , U_Hz       , 0));
		list.push_back(new rVariable(name_set_hz, TYPE_LREAL, VARF_____, &SetFactor.Point[ii].Hz, U_Hz       , ACCESS_FACTORS));
	}

	list.push_back(new rVariable(Alias + ".fault"             , TYPE_UDINT, VARF_R___, &Fault                , U_DIMLESS, 0));

	return 0;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rStream::LoadFromXML(tinyxml2::XMLElement *element, rDataConfig &cfg)
{
	string defFlowMeter = rDataConfig::GetFlagNameByValue(rDataConfig::STRFMeterFlags, STR_FLOWMETER_CARIOLIS);
	string strFlowMeter = (element->Attribute("flowmeter")) ? element->Attribute("flowmeter") : defFlowMeter;
	UDINT  err = 0;

	if(tinyxml2::XML_SUCCESS != rSource::LoadFromXML(element, cfg)) return DATACFGERR_STREAM;

	tinyxml2::XMLElement *impulse = element->FirstChildElement(CFGNAME_IMPULSE);
	tinyxml2::XMLElement *freq    = element->FirstChildElement(CFGNAME_FREQ);
	tinyxml2::XMLElement *temp    = element->FirstChildElement(CFGNAME_TEMP);
	tinyxml2::XMLElement *pres    = element->FirstChildElement(CFGNAME_PRES);
	tinyxml2::XMLElement *dens    = element->FirstChildElement(CFGNAME_DENSITY);
	tinyxml2::XMLElement *dens15  = element->FirstChildElement(CFGNAME_DENSITY15);
	tinyxml2::XMLElement *dens20  = element->FirstChildElement(CFGNAME_DENSITY20);
	tinyxml2::XMLElement *b15     = element->FirstChildElement(CFGNAME_B15);
	tinyxml2::XMLElement *y15     = element->FirstChildElement(CFGNAME_Y15);
	tinyxml2::XMLElement *factors = element->FirstChildElement(CFGNAME_FACTORS);

	Linearization = rDataConfig::GetAttributeUSINT(element, CFGNAME_LINEARIZATION, 0);
	Maintenance   = rDataConfig::GetAttributeUSINT(element, CFGNAME_MAINTENANCE  , 0);
	FlowMeter     = rDataConfig::GetFlagFromStr(rDataConfig::STRFMeterFlags, strFlowMeter, err);

	if(nullptr == impulse || nullptr == temp || nullptr == pres || nullptr == dens || nullptr == factors || err)
	{
		return DATACFGERR_STREAM;
	}

	if(tinyxml2::XML_SUCCESS != cfg.LoadLink(impulse->FirstChildElement(CFGNAME_LINK), Counter)) return cfg.ErrorID;
	if(tinyxml2::XML_SUCCESS != cfg.LoadLink(pres->FirstChildElement   (CFGNAME_LINK), Pres))    return cfg.ErrorID;
	if(tinyxml2::XML_SUCCESS != cfg.LoadLink(dens->FirstChildElement   (CFGNAME_LINK), Dens))    return cfg.ErrorID;
	if(tinyxml2::XML_SUCCESS != cfg.LoadLink(temp->FirstChildElement   (CFGNAME_LINK), Temp))    return cfg.ErrorID;

	if(tinyxml2::XML_SUCCESS != cfg.LoadShadowLink(freq  , Freq  , Counter, CFGNAME_FREQ)     ) return cfg.ErrorID;
	if(tinyxml2::XML_SUCCESS != cfg.LoadShadowLink(dens15, Dens15, Dens   , CFGNAME_DENSITY15)) return cfg.ErrorID;
	if(tinyxml2::XML_SUCCESS != cfg.LoadShadowLink(dens20, Dens20, Dens   , CFGNAME_DENSITY20)) return cfg.ErrorID;
	if(tinyxml2::XML_SUCCESS != cfg.LoadShadowLink(b15   , B15   , Dens   , CFGNAME_B15)      ) return cfg.ErrorID;
	if(tinyxml2::XML_SUCCESS != cfg.LoadShadowLink(y15   , Y15   , Dens   , CFGNAME_B15)      ) return cfg.ErrorID;

	//----------------------------------------------------------------------------------------------
	// Загрузка факторов
	tinyxml2::XMLElement *points = factors->FirstChildElement(CFGNAME_POINTS);

	SetFactor.KeypadKF.Init(rDataConfig::GetTextLREAL(factors->FirstChildElement(CFGNAME_KEYPAD_KF), 0.0, err));
	SetFactor.KeypadMF.Init(rDataConfig::GetTextLREAL(factors->FirstChildElement(CFGNAME_KEYPAD_MF), 0.0, err));

	if(nullptr != points)
	{
		for(UDINT ii = 0; ii < MAX_FACTOR_POINT; ++ii)
		{
			tinyxml2::XMLElement *point = factors->FirstChildElement(CFGNAME_POINT);

			if(nullptr == point) break;

			SetFactor.Point[ii].Hz.Init(rDataConfig::GetTextLREAL(point->FirstChildElement(CFGNAME_HZ), 0.0, err));
			SetFactor.Point[ii].Kf.Init(rDataConfig::GetTextLREAL(point->FirstChildElement(CFGNAME_KF), 0.0, err));
		}
	}

	if(err)
	{
		return DATACFGERR_STREAM;
	}

	//
	Factor = SetFactor;

	// Необходимо переустановить единицы измерения расхода
	FlowMass.Unit     = Station->UnitMass;
	FlowVolume.Unit   = Station->UnitVolume;
	FlowVolume15.Unit = Station->UnitVolume;
	FlowVolume20.Unit = Station->UnitVolume;

	ReinitLimitEvents();

	return tinyxml2::XML_SUCCESS;
}


UDINT rStream::SaveKernel(FILE *file, UDINT isio, const string &objname, const string &comment, UDINT isglobal)
{
	Counter.Limit.Setup.Init(0);
	Freq.Limit.Setup.Init(0);
	Temp.Limit.Setup.Init(0);
	Pres.Limit.Setup.Init(0);
	Dens.Limit.Setup.Init(0);
	Dens15.Limit.Setup.Init(0);
	Dens20.Limit.Setup.Init(0);
	B15.Limit.Setup.Init(0);
	Y15.Limit.Setup.Init(0);

	FlowMass.Limit.Setup.Init(0);
	FlowVolume.Limit.Setup.Init(0);
	FlowVolume15.Limit.Setup.Init(0);
	FlowVolume20.Limit.Setup.Init(0);

	return rSource::SaveKernel(file, isio, objname, comment, isglobal);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//
LREAL rStream::CalcualateKF()
{
	double minKF = 0;
	double maxKF = 0;
	double minHz = 0;
	double maxHz = 0;

	if(Freq.Value <= 0) return 0;

	if(!Linearization)
	{
		return Factor.KeypadKF.Value;
	}

	// Проверка на уход частоты за минимум
	if(Freq.Value < Factor.Point[0].Hz.Value)
	{
		return Factor.Point[0].Kf.Value;
	}

	for(UDINT ii = 1; ii < MAX_FACTOR_POINT; ++ii)
	{
		// Проверка ухода частоты за максимум
		if(Factor.Point[ii].Hz.Value <= 0)
		{
			return Factor.Point[ii - 1].Kf.Value;
		}

		if(Freq.Value < Factor.Point[ii].Hz.Value)
		{
			minKF = Factor.Point[ii - 1].Kf.Value;
			maxKF = Factor.Point[ii    ].Kf.Value;
			minHz = Factor.Point[ii - 1].Hz.Value;
			maxHz = Factor.Point[ii    ].Hz.Value;

			return minKF + ((maxKF - minKF) / (maxHz - minHz)) * (Freq.Value - minHz);
		}
	}

	// Ушли за максимум, при полностью заполенной таблице
	return Factor.Point[MAX_FACTOR_POINT - 1].Kf.Value;
}



UDINT rStream::CalcTotal()
{
	if(Maintenance)
	{
		Total.Inc.Count    = 0;
		Total.Inc.Mass     = 0;
		Total.Inc.Volume   = 0;
		Total.Inc.Volume15 = 0;
		Total.Inc.Volume20 = 0;
		return 0;
	}

	if(FlowMeter == STR_FLOWMETER_CARIOLIS)
	{
		Total.Inc.Mass     = Total.Inc.Count  / CurKF        * Factor.KeypadMF.Value;
		Total.Inc.Volume   = Total.Inc.Mass   / Dens.Value   * 1000.0;
		Total.Inc.Volume15 = Total.Inc.Mass   / Dens15.Value * 1000.0;
		Total.Inc.Volume20 = Total.Inc.Mass   / Dens20.Value * 1000.0;
	}
	else
	{
		Total.Inc.Volume   = Total.Inc.Count  / CurKF      * Factor.KeypadMF.Value;
		Total.Inc.Mass     = Total.Inc.Volume * Dens.Value                / 1000.0;
		Total.Inc.Volume15 = Total.Inc.Volume * Dens.Value / Dens15.Value / 1000.0;
		Total.Inc.Volume20 = Total.Inc.Volume * Dens.Value / Dens20.Value / 1000.0;
	}

	Total.Inc.Mass     = Round(Total.Inc.Mass    , 5);
	Total.Inc.Volume   = Round(Total.Inc.Volume  , 5);
	Total.Inc.Volume15 = Round(Total.Inc.Volume15, 5);
	Total.Inc.Volume20 = Round(Total.Inc.Volume20, 5);

	return Total.Calculate(Station->UnitMass, Station->UnitVolume);
}

