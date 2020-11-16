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
#include "tinyxml2.h"
#include "event_eid.h"
#include "text_id.h"
#include "event_manager.h"
#include "precision.h"
#include "data_manager.h"
#include "data_config.h"
#include "variable_item.h"
#include "variable_list.h"
#include "data_stream.h"
#include "data_station.h"
#include "xml_util.h"

using std::vector;

/*
const UDINT FI_BAD_COUNT     = 0x10000000;
const LREAL FI_BAD_SPLINE    = -1.0;

const UDINT FI_LE_STATUSPATH = 0x00000001;
const UDINT FI_LE_SIM_AUTO   = 0x00000002;
const UDINT FI_LE_SIM_MANUAL = 0x00000004;
const UDINT FI_LE_SIM_OFF    = 0x00000008;
const UDINT FI_LE_SIM_LAST   = 0x00000010;
const UDINT FI_LE_CODE_FAULT = 0x00000020;
*/

///////////////////////////////////////////////////////////////////////////////////////////////////
//
rStation::rStation() : Setup(0)
{
	LockErr    = 0;
	Product    = PRODUCT_PETROLEUM;
	UnitVolume = U_m3;
	UnitMass   = U_t;

	Stream.clear();

	InitLink(LINK_SETUP_INOUTPUT, Temp        , U_C       , SID_TEMPERATURE      , XmlName::TEMP         , LINK_SHADOW_NONE);
	InitLink(LINK_SETUP_INOUTPUT, Pres        , U_MPa     , SID_PRESSURE         , XmlName::PRES         , LINK_SHADOW_NONE);
	InitLink(LINK_SETUP_INOUTPUT, Dens        , U_kg_m3   , SID_DENSITY          , XmlName::DENSITY      , LINK_SHADOW_NONE);
	InitLink(LINK_SETUP_OUTPUT  , FlowMass    , UnitMass  , SID_FLOWRATE_MASS    , XmlName::FLOWRATEMASS , LINK_SHADOW_NONE);
	InitLink(LINK_SETUP_OUTPUT  , FlowVolume  , UnitVolume, SID_FLOWRATE_VOLUME  , XmlName::FLOWRATEVOL  , LINK_SHADOW_NONE);
	InitLink(LINK_SETUP_OUTPUT  , FlowVolume15, UnitVolume, SID_FLOWRATE_VOLUME15, XmlName::FLOWRATEVOL15, LINK_SHADOW_NONE);
	InitLink(LINK_SETUP_OUTPUT  , FlowVolume20, UnitVolume, SID_FLOWRATE_VOLUME20, XmlName::FLOWRATEVOL20, LINK_SHADOW_NONE);
}


rStation::~rStation()
{
	Stream.clear();
}


//-------------------------------------------------------------------------------------------------
//
UDINT rStation::InitLimitEvent(rLink &link)
{
	link.Limit.EventChangeAMin  = ReinitEvent(EID_STATION_NEW_AMIN)  << link.Descr << link.Unit;
	link.Limit.EventChangeWMin  = ReinitEvent(EID_STATION_NEW_WMIN)  << link.Descr << link.Unit;
	link.Limit.EventChangeWMax  = ReinitEvent(EID_STATION_NEW_WMAX)  << link.Descr << link.Unit;
	link.Limit.EventChangeAMax  = ReinitEvent(EID_STATION_NEW_AMAX)  << link.Descr << link.Unit;
	link.Limit.EventChangeHyst  = ReinitEvent(EID_STATION_NEW_HYST)  << link.Descr << link.Unit;
	link.Limit.EventChangeSetup = ReinitEvent(EID_STATION_NEW_SETUP) << link.Descr << link.Unit;
	link.Limit.EventAMin        = ReinitEvent(EID_STATION_AMIN)      << link.Descr << link.Unit;
	link.Limit.EventWMin        = ReinitEvent(EID_STATION_WMIN)      << link.Descr << link.Unit;
	link.Limit.EventWMax        = ReinitEvent(EID_STATION_WMAX)      << link.Descr << link.Unit;
	link.Limit.EventAMax        = ReinitEvent(EID_STATION_AMAX)      << link.Descr << link.Unit;
	link.Limit.EventNan         = ReinitEvent(EID_STATION_NAN)       << link.Descr << link.Unit;
	link.Limit.EventNormal      = ReinitEvent(EID_STATION_NORMAL)    << link.Descr << link.Unit;

	return 0;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rStation::GetFault(void)
{
	return 1;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rStation::Calculate()
{
	rStream      *str = nullptr;
	UDINT         err = 0;

	if(rSource::Calculate()) return 0;

	// Сброс значений расхода
	FlowMass.Value     = 0.0;
	FlowVolume.Value   = 0.0;
	FlowVolume15.Value = 0.0;
	FlowVolume20.Value = 0.0;

	// Расчет весов по линиям и нарастающих
	for(UDINT ii = 0; ii < Stream.size(); ++ii)
	{
		str = Stream[ii];

		str->Calculate();

		if(str->Maintenance) continue; // Линия в ремонте (не в учете)

		// Расход
		FlowMass.Value     += str->FlowMass.Value;
		FlowVolume.Value   += str->FlowVolume.Value;
		FlowVolume15.Value += str->FlowVolume15.Value;
		FlowVolume20.Value += str->FlowVolume20.Value;

		Total.Inc.Count     = 0;
		Total.Inc.Mass     += str->Total.Inc.Mass;
		Total.Inc.Volume   += str->Total.Inc.Volume;
		Total.Inc.Volume15 += str->Total.Inc.Volume15;
		Total.Inc.Volume20 += str->Total.Inc.Volume20;

		UDINT check = Total.Calculate(UnitMass, UnitVolume);
		if(check & TOTAL_MAX_MASS    ) rEventManager::instance().Add(ReinitEvent(EID_STATION_TOTAL_MASS)    );
		if(check & TOTAL_MAX_VOLUME  ) rEventManager::instance().Add(ReinitEvent(EID_STATION_TOTAL_VOLUME)  );
		if(check & TOTAL_MAX_VOLUME15) rEventManager::instance().Add(ReinitEvent(EID_STATION_TOTAL_VOLUME15));
		if(check & TOTAL_MAX_VOLUME20) rEventManager::instance().Add(ReinitEvent(EID_STATION_TOTAL_VOLUME20));
	}

	// Расчет параметров станции
	err = 0;

	for(UDINT ii = 0; ii < Stream.size(); ++ii)
	{
		str = Stream[ii];

		if(str->Maintenance) continue;

		if(nullptr == Temp.Source) Temp.Value += (Total.Inc.Mass > 0.0) ? str->GetValue(XmlName::TEMP   , Temp.Unit, err) * (str->Total.Inc.Mass / Total.Inc.Mass) : 0.0;
		if(nullptr == Pres.Source) Pres.Value += (Total.Inc.Mass > 0.0) ? str->GetValue(XmlName::PRES   , Pres.Unit, err) * (str->Total.Inc.Mass / Total.Inc.Mass) : 0.0;
		if(nullptr == Dens.Source) Dens.Value += (Total.Inc.Mass > 0.0) ? str->GetValue(XmlName::DENSITY, Dens.Unit, err) * (str->Total.Inc.Mass / Total.Inc.Mass) : 0.0;
	}

	return 0;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rStation::GetUnitFlowVolume()
{
	switch(UnitVolume)
	{
		case U_m3   : return U_m3_h;
		case U_liter: return U_ltr_h;
		default     : return U_UNDEF;
	}
}

UDINT rStation::GetUnitFlowMass()
{
	switch(UnitMass)
	{
		case U_t : return U_t_h;
		case U_kg: return U_kg_h;
		default  : return U_UNDEF;
	}
}



///////////////////////////////////////////////////////////////////////////////////////////////////
//
const rTotal *rStation::GetTotal(void)
{
	return &Total;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//

///////////////////////////////////////////////////////////////////////////////////////////////////
//


//-------------------------------------------------------------------------------------------------
//
UDINT rStation::generateVars(rVariableList& list)
{
	rSource::generateVars(list);

	// Внутренние переменные
	list.add(Alias + ".Product"               , TYPE_UDINT, rVariable::Flags::RS_L, &Product               , U_DIMLESS , ACCESS_SA);
	list.add(Alias + ".Setup"                 , TYPE_UINT , rVariable::Flags::RS_L, &Setup.Value           , U_DIMLESS , ACCESS_SA);
	list.add(Alias + ".total.present.volume"  , TYPE_LREAL, rVariable::Flags::R___, &Total.Present.Volume  , UnitVolume, 0);
	list.add(Alias + ".total.present.volume15", TYPE_LREAL, rVariable::Flags::R___, &Total.Present.Volume15, UnitVolume, 0);
	list.add(Alias + ".total.present.volume20", TYPE_LREAL, rVariable::Flags::R___, &Total.Present.Volume20, UnitVolume, 0);
	list.add(Alias + ".total.present.mass"    , TYPE_LREAL, rVariable::Flags::R___, &Total.Present.Mass    , UnitMass  , 0);
	list.add(Alias + ".total.inc.volume"      , TYPE_LREAL, rVariable::Flags::RSH_, &Total.Inc.Volume      , UnitVolume, ACCESS_SA);
	list.add(Alias + ".total.inc.volume15"    , TYPE_LREAL, rVariable::Flags::RSH_, &Total.Inc.Volume15    , UnitVolume, ACCESS_SA);
	list.add(Alias + ".total.inc.volume20"    , TYPE_LREAL, rVariable::Flags::RSH_, &Total.Inc.Volume20    , UnitVolume, ACCESS_SA);
	list.add(Alias + ".total.inc.mass"        , TYPE_LREAL, rVariable::Flags::RSH_, &Total.Inc.Mass        , UnitMass  , ACCESS_SA);
	list.add(Alias + ".total.raw.volume"      , TYPE_LREAL, rVariable::Flags::RSH_, &Total.Raw.Volume      , UnitVolume, 0);
	list.add(Alias + ".total.raw.volume15"    , TYPE_LREAL, rVariable::Flags::RSH_, &Total.Raw.Volume15    , UnitVolume, 0);
	list.add(Alias + ".total.raw.volume20"    , TYPE_LREAL, rVariable::Flags::RSH_, &Total.Raw.Volume20    , UnitVolume, 0);
	list.add(Alias + ".total.raw.mass"        , TYPE_LREAL, rVariable::Flags::RSH_, &Total.Raw.Mass        , UnitMass  , 0);
	list.add(Alias + ".total.past.volume"     , TYPE_LREAL, rVariable::Flags::RSH_, &Total.Past.Volume     , UnitVolume, 0);
	list.add(Alias + ".total.past.volume15"   , TYPE_LREAL, rVariable::Flags::RSH_, &Total.Past.Volume15   , UnitVolume, 0);
	list.add(Alias + ".total.past.volume20"   , TYPE_LREAL, rVariable::Flags::RSH_, &Total.Past.Volume20   , UnitVolume, 0);
	list.add(Alias + ".total.past.mass"       , TYPE_LREAL, rVariable::Flags::RSH_, &Total.Past.Mass       , UnitMass  , 0);

	list.add(Alias + ".fault"                 , TYPE_UDINT, rVariable::Flags::R___, &Fault                 , U_DIMLESS , 0);

	return 0;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rStation::LoadFromXML(tinyxml2::XMLElement *element, rDataConfig &cfg)
{
	string defProduct = rDataConfig::GetFlagNameByValue(rDataConfig::STNProductValues, PRODUCT_PETROLEUM);
	string strProduct = XmlUtils::getAttributeString(element, XmlName::PRODUCT, defProduct);
	UDINT  err = 0;

	if(tinyxml2::XML_SUCCESS != rSource::LoadFromXML(element, cfg)) return 1;

	tinyxml2::XMLElement *temp = element->FirstChildElement(XmlName::TEMP);
	tinyxml2::XMLElement *pres = element->FirstChildElement(XmlName::PRES);
	tinyxml2::XMLElement *dens = element->FirstChildElement(XmlName::DENSITY);

	Product   = (TYPE_PRODUCT)rDataConfig::GetFlagFromStr(rDataConfig::STNProductValues , strProduct , err);
	if(err) return DATACFGERR_STATION;

	Setup.Init(0);

	// Параметры ниже могут отсутствовать в конфигурации, в этом случае они будут вычисляться как средневзвешанные
	if(temp) if(tinyxml2::XML_SUCCESS != cfg.LoadLink(temp->FirstChildElement(XmlName::LINK), Temp)) return cfg.ErrorID;
	if(pres) if(tinyxml2::XML_SUCCESS != cfg.LoadLink(pres->FirstChildElement(XmlName::LINK), Pres)) return cfg.ErrorID;
	if(dens) if(tinyxml2::XML_SUCCESS != cfg.LoadLink(dens->FirstChildElement(XmlName::LINK), Dens)) return cfg.ErrorID;

	ReinitLimitEvents();

	return tinyxml2::XML_SUCCESS;
}



std::string rStation::saveKernel(UDINT isio, const string &objname, const string &comment, UDINT isglobal)
{
	Temp.Limit.m_setup.Init(rLimit::Setup::NONE);
	Pres.Limit.m_setup.Init(rLimit::Setup::NONE);
	Dens.Limit.m_setup.Init(rLimit::Setup::NONE);
	FlowMass.Limit.m_setup.Init(rLimit::Setup::NONE);
	FlowVolume.Limit.m_setup.Init(rLimit::Setup::NONE);
	FlowVolume15.Limit.m_setup.Init(rLimit::Setup::NONE);
	FlowVolume20.Limit.m_setup.Init(rLimit::Setup::NONE);

	return rSource::saveKernel(isio, objname, comment, isglobal);
}





