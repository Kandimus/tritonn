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
#include "data_config.h"
#include "data_manager.h"
#include "variable_item.h"
#include "variable_list.h"
#include "data_stream.h"
#include "data_station.h"
#include "xml_util.h"
#include "error.h"

rBitsArray rStation::m_flagsProduct;


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
	if (m_flagsProduct.empty()) {
		m_flagsProduct
				.add("PETROLEUM"   , static_cast<USINT>(rDensity::Product::PETROLEUM), "")
				.add("GAZOLENE"    , static_cast<USINT>(rDensity::Product::GAZOLENE))
				.add("TRANSITION"  , static_cast<USINT>(rDensity::Product::TRANSITION))
				.add("JETFUEL"     , static_cast<USINT>(rDensity::Product::JETFUEL))
				.add("FUELOIL"     , static_cast<USINT>(rDensity::Product::FUELOIL))
				.add("SMARTBENZENE", static_cast<USINT>(rDensity::Product::SMARTBENZENE))
				.add("LUBRICANT"   , static_cast<USINT>(rDensity::Product::LUBRICANT));
	}

	m_product  = rDensity::Product::PETROLEUM;
	m_unitVolume = U_m3;
	m_unitMass   = U_t;

	Stream.clear();

	InitLink(rLink::Setup::INOUTPUT, m_temp        , U_C                , SID::TEMPERATURE      , XmlName::TEMP         , rLink::SHADOW_NONE);
	InitLink(rLink::Setup::INOUTPUT, m_pres        , U_MPa              , SID::PRESSURE         , XmlName::PRES         , rLink::SHADOW_NONE);
	InitLink(rLink::Setup::INOUTPUT, m_dens        , U_kg_m3            , SID::DENSITY          , XmlName::DENSITY      , rLink::SHADOW_NONE);
	InitLink(rLink::Setup::OUTPUT  , m_flowMass    , getUnitFlowMass()  , SID::FLOWRATE_MASS    , XmlName::FLOWRATEMASS , rLink::SHADOW_NONE);
	InitLink(rLink::Setup::OUTPUT  , m_flowVolume  , getUnitFlowVolume(), SID::FLOWRATE_VOLUME  , XmlName::FLOWRATEVOL  , rLink::SHADOW_NONE);
	InitLink(rLink::Setup::OUTPUT  , m_flowVolume15, getUnitFlowVolume(), SID::FLOWRATE_VOLUME15, XmlName::FLOWRATEVOL15, rLink::SHADOW_NONE);
	InitLink(rLink::Setup::OUTPUT  , m_flowVolume20, getUnitFlowVolume(), SID::FLOWRATE_VOLUME20, XmlName::FLOWRATEVOL20, rLink::SHADOW_NONE);
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
	UDINT err = 0;

	if(rSource::Calculate()) return 0;

	// Сброс значений расхода
	m_flowMass.Value     = 0.0;
	m_flowVolume.Value   = 0.0;
	m_flowVolume15.Value = 0.0;
	m_flowVolume20.Value = 0.0;

	// Расчет весов по линиям и нарастающих
	for (auto str : Stream) {
		str->Calculate();

		// Линия в ремонте (не в учете)
		if(str->Maintenance) {
			continue;
		}

		// Расход
		m_flowMass.Value     += str->FlowMass.Value;
		m_flowVolume.Value   += str->FlowVolume.Value;
		m_flowVolume15.Value += str->FlowVolume15.Value;
		m_flowVolume20.Value += str->FlowVolume20.Value;

		Total.Inc.Count     = 0;
		Total.Inc.Mass     += str->Total.Inc.Mass;
		Total.Inc.Volume   += str->Total.Inc.Volume;
		Total.Inc.Volume15 += str->Total.Inc.Volume15;
		Total.Inc.Volume20 += str->Total.Inc.Volume20;

		UDINT check = Total.Calculate(m_unitMass, m_unitVolume);
		if(check & TOTAL_MAX_MASS    ) rEventManager::instance().Add(ReinitEvent(EID_STATION_TOTAL_MASS)    );
		if(check & TOTAL_MAX_VOLUME  ) rEventManager::instance().Add(ReinitEvent(EID_STATION_TOTAL_VOLUME)  );
		if(check & TOTAL_MAX_VOLUME15) rEventManager::instance().Add(ReinitEvent(EID_STATION_TOTAL_VOLUME15));
		if(check & TOTAL_MAX_VOLUME20) rEventManager::instance().Add(ReinitEvent(EID_STATION_TOTAL_VOLUME20));
	}

	// Расчет параметров станции
	err = 0;

	for (auto str : Stream) {
		if (str->Maintenance) {
			continue;
		}

		if(!m_temp.isValid()) m_temp.Value += (Total.Inc.Mass > 0.0) ? str->GetValue(XmlName::TEMP   , m_temp.Unit, err) * (str->Total.Inc.Mass / Total.Inc.Mass) : 0.0;
		if(!m_pres.isValid()) m_pres.Value += (Total.Inc.Mass > 0.0) ? str->GetValue(XmlName::PRES   , m_pres.Unit, err) * (str->Total.Inc.Mass / Total.Inc.Mass) : 0.0;
		if(!m_dens.isValid()) m_dens.Value += (Total.Inc.Mass > 0.0) ? str->GetValue(XmlName::DENSITY, m_dens.Unit, err) * (str->Total.Inc.Mass / Total.Inc.Mass) : 0.0;
	}

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rStation::getUnitFlowVolume()
{
	switch (m_unitVolume) {
		case U_m3   : return U_m3_h;
		case U_liter: return U_ltr_h;
		default     : return U_UNDEF;
	}
}

UDINT rStation::getUnitFlowMass()
{
	switch (m_unitMass) {
		case U_t : return U_t_h;
		case U_kg: return U_kg_h;
		default  : return U_UNDEF;
	}
}



///////////////////////////////////////////////////////////////////////////////////////////////////
//
const rTotal *rStation::getTotal(void) const
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
	list.add(Alias + ".Product"               , TYPE_USINT, rVariable::Flags::RS_L, &m_product             , U_DIMLESS   , ACCESS_SA);
	list.add(Alias + ".Setup"                 , TYPE_UINT , rVariable::Flags::RS_L, &Setup.Value           , U_DIMLESS   , ACCESS_SA);
	list.add(Alias + ".total.present.volume"  , TYPE_LREAL, rVariable::Flags::R___, &Total.Present.Volume  , m_unitVolume, 0);
	list.add(Alias + ".total.present.volume15", TYPE_LREAL, rVariable::Flags::R___, &Total.Present.Volume15, m_unitVolume, 0);
	list.add(Alias + ".total.present.volume20", TYPE_LREAL, rVariable::Flags::R___, &Total.Present.Volume20, m_unitVolume, 0);
	list.add(Alias + ".total.present.mass"    , TYPE_LREAL, rVariable::Flags::R___, &Total.Present.Mass    , m_unitMass  , 0);
	list.add(Alias + ".total.inc.volume"      , TYPE_LREAL, rVariable::Flags::RSH_, &Total.Inc.Volume      , m_unitVolume, ACCESS_SA);
	list.add(Alias + ".total.inc.volume15"    , TYPE_LREAL, rVariable::Flags::RSH_, &Total.Inc.Volume15    , m_unitVolume, ACCESS_SA);
	list.add(Alias + ".total.inc.volume20"    , TYPE_LREAL, rVariable::Flags::RSH_, &Total.Inc.Volume20    , m_unitVolume, ACCESS_SA);
	list.add(Alias + ".total.inc.mass"        , TYPE_LREAL, rVariable::Flags::RSH_, &Total.Inc.Mass        , m_unitMass  , ACCESS_SA);
	list.add(Alias + ".total.raw.volume"      , TYPE_LREAL, rVariable::Flags::RSH_, &Total.Raw.Volume      , m_unitVolume, 0);
	list.add(Alias + ".total.raw.volume15"    , TYPE_LREAL, rVariable::Flags::RSH_, &Total.Raw.Volume15    , m_unitVolume, 0);
	list.add(Alias + ".total.raw.volume20"    , TYPE_LREAL, rVariable::Flags::RSH_, &Total.Raw.Volume20    , m_unitVolume, 0);
	list.add(Alias + ".total.raw.mass"        , TYPE_LREAL, rVariable::Flags::RSH_, &Total.Raw.Mass        , m_unitMass  , 0);
	list.add(Alias + ".total.past.volume"     , TYPE_LREAL, rVariable::Flags::RSH_, &Total.Past.Volume     , m_unitVolume, 0);
	list.add(Alias + ".total.past.volume15"   , TYPE_LREAL, rVariable::Flags::RSH_, &Total.Past.Volume15   , m_unitVolume, 0);
	list.add(Alias + ".total.past.volume20"   , TYPE_LREAL, rVariable::Flags::RSH_, &Total.Past.Volume20   , m_unitVolume, 0);
	list.add(Alias + ".total.past.mass"       , TYPE_LREAL, rVariable::Flags::RSH_, &Total.Past.Mass       , m_unitMass  , 0);

	list.add(Alias + ".fault"                 , TYPE_UDINT, rVariable::Flags::R___, &Fault                 , U_DIMLESS , 0);

	return 0;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rStation::LoadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix)
{
	std::string strProduct = XmlUtils::getAttributeString(element, XmlName::PRODUCT, m_flagsProduct.getNameByBits(static_cast<USINT>(rDensity::Product::PETROLEUM)));

	if (TRITONN_RESULT_OK != rSource::LoadFromXML(element, err, prefix)) {
		return err.getError();
	}

	tinyxml2::XMLElement* xml_temp  = element->FirstChildElement(XmlName::TEMP);
	tinyxml2::XMLElement* xml_pres  = element->FirstChildElement(XmlName::PRES);
	tinyxml2::XMLElement* xml_dens  = element->FirstChildElement(XmlName::DENSITY);
	tinyxml2::XMLElement* xml_units = element->FirstChildElement(XmlName::UNITS);

	UDINT fault = 0;
	m_product = static_cast<rDensity::Product>(m_flagsProduct.getValue(strProduct, fault));
	if (fault) {
		return err.set(DATACFGERR_STATION, element->GetLineNum(), strProduct);
	}

	Setup.Init(0);

	// Параметры ниже могут отсутствовать в конфигурации, в этом случае они будут вычисляться как средневзвешанные
	if (xml_temp) if (TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(xml_temp->FirstChildElement(XmlName::LINK), m_temp)) return err.getError();
	if (xml_pres) if (TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(xml_pres->FirstChildElement(XmlName::LINK), m_pres)) return err.getError();
	if (xml_dens) if (TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(xml_dens->FirstChildElement(XmlName::LINK), m_dens)) return err.getError();

	if (xml_units) {
		m_unitMass   = XmlUtils::getTextUDINT(xml_units->FirstChildElement(XmlName::MASS), U_t, fault);
		m_unitVolume = XmlUtils::getTextUDINT(xml_units->FirstChildElement(XmlName::VOLUME), U_m3, fault);

		m_flowMass.Unit     = getUnitFlowMass();
		m_flowVolume.Unit   = getUnitFlowVolume();
		m_flowVolume15.Unit = getUnitFlowVolume();
		m_flowVolume20.Unit = getUnitFlowVolume();
		m_temp.Unit         = XmlUtils::getTextUDINT(xml_units->FirstChildElement(XmlName::TEMP)   , m_temp.Unit, fault);
		m_pres.Unit         = XmlUtils::getTextUDINT(xml_units->FirstChildElement(XmlName::PRES)   , m_pres.Unit, fault);
		m_dens.Unit         = XmlUtils::getTextUDINT(xml_units->FirstChildElement(XmlName::DENSITY), m_dens.Unit, fault);

		if (fault) {
			return err.set(DATACFGERR_STATION_UNITS, xml_units->GetLineNum(), "");
		}
	}

	ReinitLimitEvents();

	return tinyxml2::XML_SUCCESS;
}



std::string rStation::saveKernel(UDINT isio, const string &objname, const string &comment, UDINT isglobal)
{
	m_temp.Limit.m_setup.Init(rLimit::Setup::NONE);
	m_pres.Limit.m_setup.Init(rLimit::Setup::NONE);
	m_dens.Limit.m_setup.Init(rLimit::Setup::NONE);
	m_flowMass.Limit.m_setup.Init(rLimit::Setup::NONE);
	m_flowVolume.Limit.m_setup.Init(rLimit::Setup::NONE);
	m_flowVolume15.Limit.m_setup.Init(rLimit::Setup::NONE);
	m_flowVolume20.Limit.m_setup.Init(rLimit::Setup::NONE);

	return rSource::saveKernel(isio, objname, comment, isglobal);
}





