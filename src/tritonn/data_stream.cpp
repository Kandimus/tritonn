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
#include "xml_util.h"
#include "event_manager.h"
#include "precision.h"
#include "data_manager.h"
#include "data_config.h"
#include "variable_item.h"
#include "variable_list.h"
#include "data_station.h"
#include "data_stream.h"


const UDINT STREAM_LE_INPUTS     = 0x00000001;
const UDINT STREAM_LE_ACCOUNTING = 0x00000002;
const UDINT STREAM_LE_KEYPADKF   = 0x00000004;


rBitsArray rStream::m_flagsFlowmeter;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
rStream::rStream() : Setup(STR_SETUP_OFF)
{
	if (m_flagsFlowmeter.empty()) {
		m_flagsFlowmeter
				.add("TURBINE"   , static_cast<UINT>(Type::TURBINE))
				.add("CARIOLIS"  , static_cast<UINT>(Type::CARIOLIS))
				.add("ULTRASONIC", static_cast<UINT>(Type::ULTRASONIC));
	}

//	Unit       = U_UNDEF;
	Maintenance   = true;
	Linearization = false;
//	rTotal   Total;

	InitLink(rLink::Setup::INPUT   , Counter     , U_imp   , SID_IMPULSE          , XmlName::IMPULSE      , rLink::SHADOW_NONE);
	InitLink(rLink::Setup::INPUT   , Freq        , U_Hz    , SID_FREQUENCY        , XmlName::FREQ         , XmlName::IMPULSE);
	InitLink(rLink::Setup::INOUTPUT, Temp        , U_C     , SID_TEMPERATURE      , XmlName::TEMP         , rLink::SHADOW_NONE);
	InitLink(rLink::Setup::INOUTPUT, Pres        , U_MPa   , SID_PRESSURE         , XmlName::PRES         , rLink::SHADOW_NONE);
	InitLink(rLink::Setup::INOUTPUT, Dens        , U_kg_m3 , SID_DENSITY          , XmlName::DENSITY      , rLink::SHADOW_NONE);
	InitLink(rLink::Setup::INOUTPUT, Dens15      , U_kg_m3 , SID_DENSITY15        , XmlName::DENSITY15    , XmlName::DENSITY);
	InitLink(rLink::Setup::INOUTPUT, Dens20      , U_kg_m3 , SID_DENSITY20        , XmlName::DENSITY20    , XmlName::DENSITY);
	InitLink(rLink::Setup::INOUTPUT, B15         , U_1_C   , SID_B15              , XmlName::B15          , XmlName::DENSITY);
	InitLink(rLink::Setup::INOUTPUT, Y15         , U_1_MPa , SID_Y15              , XmlName::Y15          , XmlName::DENSITY);
	InitLink(rLink::Setup::OUTPUT  , FlowMass    , U_t_h   , SID_FLOWRATE_MASS    , XmlName::FLOWRATEMASS , rLink::SHADOW_NONE);
	InitLink(rLink::Setup::OUTPUT  , FlowVolume  , U_m3_h  , SID_FLOWRATE_VOLUME  , XmlName::FLOWRATEVOL  , rLink::SHADOW_NONE);
	InitLink(rLink::Setup::OUTPUT  , FlowVolume15, U_m3_h  , SID_FLOWRATE_VOLUME15, XmlName::FLOWRATEVOL15, rLink::SHADOW_NONE);
	InitLink(rLink::Setup::OUTPUT  , FlowVolume20, U_m3_h  , SID_FLOWRATE_VOLUME20, XmlName::FLOWRATEVOL20, rLink::SHADOW_NONE);
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
		rEventManager::instance().Add(ReinitEvent(EID_STREAM_ACCEPT));
	}

	//-------------------------------------------------------------------------------------------
	// Расчет текущего коэф-та
	CurKF = (Linearization) ? CalcualateKF() : Factor.KeypadKF.Value;

	//-------------------------------------------------------------------------------------------
	// Нарастающие и расход
	// Увеличим глобальный счетчик
	UDINT check = CalcTotal();

	if(check & TOTAL_MAX_MASS    ) rEventManager::instance().Add(ReinitEvent(EID_STREAM_TOTAL_MASS));
	if(check & TOTAL_MAX_VOLUME  ) rEventManager::instance().Add(ReinitEvent(EID_STREAM_TOTAL_VOLUME));
	if(check & TOTAL_MAX_VOLUME15) rEventManager::instance().Add(ReinitEvent(EID_STREAM_TOTAL_VOLUME15));
	if(check & TOTAL_MAX_VOLUME20) rEventManager::instance().Add(ReinitEvent(EID_STREAM_TOTAL_VOLUME20));

	// Расчитаем расход исходя из частоты
	if (m_flowmeter == Type::CARIOLIS) {
		FlowMass.Value   = Freq.Value     / CurKF      * 3600.0 * Factor.KeypadMF.Value;
		FlowVolume.Value = FlowMass.Value / Dens.Value * 1000.0 * Factor.KeypadMF.Value;
	} else {
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

	if (m_flowmeter == Type::CARIOLIS) {
		switch (Station->UnitMass) {
			case U_t : return U_imp_t;
			case U_kg: return U_imp_kg;
			default  : return U_UNDEF;
		}
	} else {
		switch (Station->UnitVolume) {
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


//-------------------------------------------------------------------------------------------------
//
UDINT rStream::generateVars(rVariableList& list)
{
	rSource::generateVars(list);

	// Внутренние переменные
	list.add(Alias + ".setup"                 , TYPE_UINT , rVariable::Flags::RS_L, &Setup.Value             , U_DIMLESS          , ACCESS_SA);
	list.add(Alias + ".flowmeter"             , TYPE_USINT, rVariable::Flags::R__L, &m_flowmeter             , U_DIMLESS          , 0);
	list.add(Alias + ".maintenance"           , TYPE_UDINT, rVariable::Flags::___L, &Maintenance             , U_DIMLESS          , ACCESS_MAINTENANCE);
	list.add(Alias + ".linearization"         , TYPE_UDINT, rVariable::Flags::___L, &Linearization           , U_DIMLESS          , ACCESS_FACTORS);
	list.add(Alias + ".total.present.Volume"  , TYPE_LREAL, rVariable::Flags::R___, &Total.Present.Volume    , Station->UnitVolume, 0);
	list.add(Alias + ".total.present.Volume15", TYPE_LREAL, rVariable::Flags::R___, &Total.Present.Volume15  , Station->UnitVolume, 0);
	list.add(Alias + ".total.present.Volume20", TYPE_LREAL, rVariable::Flags::R___, &Total.Present.Volume20  , Station->UnitVolume, 0);
	list.add(Alias + ".total.present.Mass"    , TYPE_LREAL, rVariable::Flags::R___, &Total.Present.Mass      , Station->UnitMass  , 0);
	list.add(Alias + ".total.present.impulse" , TYPE_UDINT, rVariable::Flags::R___, &Total.Present.Count     , U_imp              , 0);
	list.add(Alias + ".total.Inc.Volume"      , TYPE_LREAL, rVariable::Flags::RSH_, &Total.Inc.Volume        , Station->UnitVolume, ACCESS_SA);
	list.add(Alias + ".total.Inc.Volume15"    , TYPE_LREAL, rVariable::Flags::RSH_, &Total.Inc.Volume15      , Station->UnitVolume, ACCESS_SA);
	list.add(Alias + ".total.Inc.Volume20"    , TYPE_LREAL, rVariable::Flags::RSH_, &Total.Inc.Volume20      , Station->UnitVolume, ACCESS_SA);
	list.add(Alias + ".total.Inc.Mass"        , TYPE_LREAL, rVariable::Flags::RSH_, &Total.Inc.Mass          , Station->UnitMass  , ACCESS_SA);
	list.add(Alias + ".total.Inc.inpulse"     , TYPE_UDINT, rVariable::Flags::RSH_, &Total.Inc.Count         , U_imp              , ACCESS_SA);
	list.add(Alias + ".total.raw.Volume"      , TYPE_LREAL, rVariable::Flags::RSH_, &Total.Raw.Volume        , Station->UnitVolume, 0);
	list.add(Alias + ".total.raw.Volume15"    , TYPE_LREAL, rVariable::Flags::RSH_, &Total.Raw.Volume15      , Station->UnitVolume, 0);
	list.add(Alias + ".total.raw.Volume20"    , TYPE_LREAL, rVariable::Flags::RSH_, &Total.Raw.Volume20      , Station->UnitVolume, 0);
	list.add(Alias + ".total.raw.Mass"        , TYPE_LREAL, rVariable::Flags::RSH_, &Total.Raw.Mass          , Station->UnitMass  , 0);
	list.add(Alias + ".total.raw.impulse"     , TYPE_UDINT, rVariable::Flags::RSH_, &Total.Raw.Count         , U_imp              , 0);
	list.add(Alias + ".total.past.Volume"     , TYPE_LREAL, rVariable::Flags::RSH_, &Total.Past.Volume       , Station->UnitVolume, 0);
	list.add(Alias + ".total.past.Volume15"   , TYPE_LREAL, rVariable::Flags::RSH_, &Total.Past.Volume15     , Station->UnitVolume, 0);
	list.add(Alias + ".total.past.Volume20"   , TYPE_LREAL, rVariable::Flags::RSH_, &Total.Past.Volume20     , Station->UnitVolume, 0);
	list.add(Alias + ".total.past.Mass"       , TYPE_LREAL, rVariable::Flags::RSH_, &Total.Past.Mass         , Station->UnitMass  , 0);
	list.add(Alias + ".total.past.impulse"    , TYPE_UDINT, rVariable::Flags::RSH_, &Total.Past.Count        , U_imp              , 0);
	list.add(Alias + ".presentkf"             , TYPE_LREAL, rVariable::Flags::R___, &CurKF                   , GetUnitKF()        , 0);
	list.add(Alias + ".factor.kf"             , TYPE_LREAL, rVariable::Flags::R___, &Factor.KeypadKF.Value   , GetUnitKF()        , 0);
	list.add(Alias + ".factor.mf"             , TYPE_LREAL, rVariable::Flags::R__L, &Factor.KeypadMF.Value   , U_DIMLESS          , 0);
	list.add(Alias + ".factor.set.kf"         , TYPE_LREAL, rVariable::Flags::___L, &SetFactor.KeypadKF.Value, GetUnitKF()        , ACCESS_FACTORS);
	list.add(Alias + ".factor.set.mf"         , TYPE_LREAL, rVariable::Flags::___L, &SetFactor.KeypadMF.Value, U_DIMLESS          , ACCESS_FACTORS);
	list.add(Alias + ".factor.set.accept"     , TYPE_UDINT, rVariable::Flags::___L, &AcceptKF                , U_DIMLESS          , ACCESS_FACTORS);


	for(UDINT ii = 0; ii < MAX_FACTOR_POINT; ++ii)
	{
		string name_kf     = String_format("%s.factor.point_%i.kf"    , Alias.c_str(), ii + 1);
		string name_hz     = String_format("%s.factor.point_%i.hz"    , Alias.c_str(), ii + 1);
		string name_set_kf = String_format("%s.factor.set.point_%i.kf", Alias.c_str(), ii + 1);
		string name_set_hz = String_format("%s.factor.set.point_%i.hz", Alias.c_str(), ii + 1);

		list.add(name_kf    , TYPE_LREAL, rVariable::Flags::R__L, &Factor.Point[ii].Kf   , GetUnitKF(), 0);
		list.add(name_set_kf, TYPE_LREAL, rVariable::Flags::____, &SetFactor.Point[ii].Kf, GetUnitKF(), ACCESS_FACTORS);
		list.add(name_hz    , TYPE_LREAL, rVariable::Flags::R__L, &Factor.Point[ii].Hz   , U_Hz       , 0);
		list.add(name_set_hz, TYPE_LREAL, rVariable::Flags::____, &SetFactor.Point[ii].Hz, U_Hz       , ACCESS_FACTORS);
	}

	list.add(Alias + ".fault", TYPE_UDINT, rVariable::Flags::R___, &Fault, U_DIMLESS, 0);

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rStream::LoadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix)
{
	std::string strFlowMeter = XmlUtils::getAttributeString(element, XmlName::FLOWMETER, m_flagsFlowmeter.getNameByBits(static_cast<USINT>(Type::CARIOLIS)));

	if (TRITONN_RESULT_OK != rSource::LoadFromXML(element, err, prefix)) {
		return err.getError();
	}

	tinyxml2::XMLElement* xml_impulse = element->FirstChildElement(XmlName::IMPULSE);
	tinyxml2::XMLElement* xml_freq    = element->FirstChildElement(XmlName::FREQ);
	tinyxml2::XMLElement* xml_temp    = element->FirstChildElement(XmlName::TEMP);
	tinyxml2::XMLElement* xml_pres    = element->FirstChildElement(XmlName::PRES);
	tinyxml2::XMLElement* xml_dens    = element->FirstChildElement(XmlName::DENSITY);
	tinyxml2::XMLElement* xml_dens15  = element->FirstChildElement(XmlName::DENSITY15);
	tinyxml2::XMLElement* xml_dens20  = element->FirstChildElement(XmlName::DENSITY20);
	tinyxml2::XMLElement* xml_b15     = element->FirstChildElement(XmlName::B15);
	tinyxml2::XMLElement* xml_y15     = element->FirstChildElement(XmlName::Y15);
	tinyxml2::XMLElement* xml_factors = element->FirstChildElement(XmlName::FACTORS);

	UDINT fault = 0;
	Linearization = XmlUtils::getAttributeUSINT(element, XmlName::LINEARIZATION, 0);
	Maintenance   = XmlUtils::getAttributeUSINT(element, XmlName::MAINTENANCE  , 0);
	m_flowmeter   = static_cast<Type>(m_flagsFlowmeter.getValue(strFlowMeter, fault));

	if (!xml_impulse || !xml_temp || !xml_pres || !xml_dens || !xml_factors || fault) {
		return err.set(DATACFGERR_STREAM, element->GetLineNum(), "undefened links");
	}

	if (TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(xml_impulse->FirstChildElement(XmlName::LINK), Counter)) return err.getError();
	if (TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(xml_pres->FirstChildElement   (XmlName::LINK), Pres))    return err.getError();
	if (TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(xml_dens->FirstChildElement   (XmlName::LINK), Dens))    return err.getError();
	if (TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(xml_temp->FirstChildElement   (XmlName::LINK), Temp))    return err.getError();

	if (TRITONN_RESULT_OK != rDataConfig::instance().LoadShadowLink(xml_freq  , Freq  , Counter, XmlName::FREQ)     ) return err.getError();
	if (TRITONN_RESULT_OK != rDataConfig::instance().LoadShadowLink(xml_dens15, Dens15, Dens   , XmlName::DENSITY15)) return err.getError();
	if (TRITONN_RESULT_OK != rDataConfig::instance().LoadShadowLink(xml_dens20, Dens20, Dens   , XmlName::DENSITY20)) return err.getError();
	if (TRITONN_RESULT_OK != rDataConfig::instance().LoadShadowLink(xml_b15   , B15   , Dens   , XmlName::B15)      ) return err.getError();
	if (TRITONN_RESULT_OK != rDataConfig::instance().LoadShadowLink(xml_y15   , Y15   , Dens   , XmlName::B15)      ) return err.getError();

	//----------------------------------------------------------------------------------------------
	// Загрузка факторов
	tinyxml2::XMLElement* points = xml_factors->FirstChildElement(XmlName::POINTS);

	SetFactor.KeypadKF.Init(XmlUtils::getTextLREAL(xml_factors->FirstChildElement(XmlName::KEYPAD_KF), 0.0, fault));
	SetFactor.KeypadMF.Init(XmlUtils::getTextLREAL(xml_factors->FirstChildElement(XmlName::KEYPAD_MF), 0.0, fault));

	if (points) {
		for (UDINT ii = 0; ii < MAX_FACTOR_POINT; ++ii) {
			tinyxml2::XMLElement* point = xml_factors->FirstChildElement(XmlName::POINT);

			if(!point) break;

			SetFactor.Point[ii].Hz.Init(XmlUtils::getTextLREAL(point->FirstChildElement(XmlName::HERTZ), 0.0, fault));
			SetFactor.Point[ii].Kf.Init(XmlUtils::getTextLREAL(point->FirstChildElement(XmlName::KF   ), 0.0, fault));
		}
	}

	if (fault) {
		return err.set(DATACFGERR_STREAM, xml_factors->GetLineNum(), "fault load factors");
	}

	//
	Factor = SetFactor;

	// Необходимо переустановить единицы измерения расхода
	FlowMass.Unit     = Station->UnitMass;
	FlowVolume.Unit   = Station->UnitVolume;
	FlowVolume15.Unit = Station->UnitVolume;
	FlowVolume20.Unit = Station->UnitVolume;

	ReinitLimitEvents();

	return TRITONN_RESULT_OK;
}


std::string rStream::saveKernel(UDINT isio, const string &objname, const string &comment, UDINT isglobal)
{
	Counter.Limit.m_setup.Init(rLimit::Setup::NONE);
	Freq.Limit.m_setup.Init(rLimit::Setup::NONE);
	Temp.Limit.m_setup.Init(rLimit::Setup::NONE);
	Pres.Limit.m_setup.Init(rLimit::Setup::NONE);
	Dens.Limit.m_setup.Init(rLimit::Setup::NONE);
	Dens15.Limit.m_setup.Init(rLimit::Setup::NONE);
	Dens20.Limit.m_setup.Init(rLimit::Setup::NONE);
	B15.Limit.m_setup.Init(rLimit::Setup::NONE);
	Y15.Limit.m_setup.Init(rLimit::Setup::NONE);

	FlowMass.Limit.m_setup.Init(rLimit::Setup::NONE);
	FlowVolume.Limit.m_setup.Init(rLimit::Setup::NONE);
	FlowVolume15.Limit.m_setup.Init(rLimit::Setup::NONE);
	FlowVolume20.Limit.m_setup.Init(rLimit::Setup::NONE);

	return rSource::saveKernel(isio, objname, comment, isglobal);
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

	if(m_flowmeter == Type::CARIOLIS) {
		Total.Inc.Mass     = Total.Inc.Count  / CurKF        * Factor.KeypadMF.Value;
		Total.Inc.Volume   = Total.Inc.Mass   / Dens.Value   * 1000.0;
		Total.Inc.Volume15 = Total.Inc.Mass   / Dens15.Value * 1000.0;
		Total.Inc.Volume20 = Total.Inc.Mass   / Dens20.Value * 1000.0;
	} else {
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

