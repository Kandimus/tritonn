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
rStation::rStation() :
	Setup(0),
	m_total(ReinitEvent(EID_STATION_TOTAL_MASS),
			ReinitEvent(EID_STATION_TOTAL_VOLUME),
			ReinitEvent(EID_STATION_TOTAL_VOLUME15),
			ReinitEvent(EID_STATION_TOTAL_VOLUME20))
{
	if (m_flagsProduct.empty()) {
		m_flagsProduct
				.add("PETROLEUM"   , static_cast<USINT>(rDensity::Product::PETROLEUM)   , "Нефть")
				.add("GAZOLENE"    , static_cast<USINT>(rDensity::Product::GAZOLENE)    , "Бензины")
				.add("TRANSITION"  , static_cast<USINT>(rDensity::Product::TRANSITION)  , "Топливо, между бензинами и керосинами")
				.add("JETFUEL"     , static_cast<USINT>(rDensity::Product::JETFUEL)     , "Керосины")
				.add("FUELOIL"     , static_cast<USINT>(rDensity::Product::FUELOIL)     , "Дизельное топливо")
				.add("SMARTBENZENE", static_cast<USINT>(rDensity::Product::SMARTBENZENE), "Автоматичский расчет группы нефтепродуктов")
				.add("LUBRICANT"   , static_cast<USINT>(rDensity::Product::LUBRICANT)   , "Смазочные масла");
	}

	m_product  = rDensity::Product::PETROLEUM;

	Stream.clear();

	InitLink(rLink::Setup::INOUTPUT, m_temp        , m_unit.getTemperature(), SID::TEMPERATURE      , XmlName::TEMP         , rLink::SHADOW_NONE);
	InitLink(rLink::Setup::INOUTPUT, m_pres        , m_unit.getPressure()   , SID::PRESSURE         , XmlName::PRES         , rLink::SHADOW_NONE);
	InitLink(rLink::Setup::INOUTPUT, m_dens        , m_unit.getDensity()    , SID::DENSITY          , XmlName::DENSITY      , rLink::SHADOW_NONE);
	InitLink(rLink::Setup::OUTPUT  , m_flowMass    , m_unit.getFlowMass()   , SID::FLOWRATE_MASS    , XmlName::FLOWRATEMASS , rLink::SHADOW_NONE);
	InitLink(rLink::Setup::OUTPUT  , m_flowVolume  , m_unit.getFlowVolume() , SID::FLOWRATE_VOLUME  , XmlName::FLOWRATEVOL  , rLink::SHADOW_NONE);
	InitLink(rLink::Setup::OUTPUT  , m_flowVolume15, m_unit.getFlowVolume() , SID::FLOWRATE_VOLUME15, XmlName::FLOWRATEVOL15, rLink::SHADOW_NONE);
	InitLink(rLink::Setup::OUTPUT  , m_flowVolume20, m_unit.getFlowVolume() , SID::FLOWRATE_VOLUME20, XmlName::FLOWRATEVOL20, rLink::SHADOW_NONE);
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
		m_flowMass.Value     += str->m_flowMass.Value;
		m_flowVolume.Value   += str->m_flowVolume.Value;
		m_flowVolume15.Value += str->m_flowVolume15.Value;
		m_flowVolume20.Value += str->m_flowVolume20.Value;

		m_total.Inc.Count     = 0;
		m_total.Inc.Mass     += str->m_total.Inc.Mass;
		m_total.Inc.Volume   += str->m_total.Inc.Volume;
		m_total.Inc.Volume15 += str->m_total.Inc.Volume15;
		m_total.Inc.Volume20 += str->m_total.Inc.Volume20;

		m_total.Calculate(m_unit);
	}

	// Расчет параметров станции
	err = 0;

	for (auto str : Stream) {
		if (str->Maintenance) {
			continue;
		}

		if(!m_temp.isValid()) m_temp.Value += (m_total.Inc.Mass > 0.0) ? str->GetValue(XmlName::TEMP   , m_temp.Unit, err) * (str->m_total.Inc.Mass / m_total.Inc.Mass) : 0.0;
		if(!m_pres.isValid()) m_pres.Value += (m_total.Inc.Mass > 0.0) ? str->GetValue(XmlName::PRES   , m_pres.Unit, err) * (str->m_total.Inc.Mass / m_total.Inc.Mass) : 0.0;
		if(!m_dens.isValid()) m_dens.Value += (m_total.Inc.Mass > 0.0) ? str->GetValue(XmlName::DENSITY, m_dens.Unit, err) * (str->m_total.Inc.Mass / m_total.Inc.Mass) : 0.0;
	}

	return TRITONN_RESULT_OK;
}





///////////////////////////////////////////////////////////////////////////////////////////////////
//
const rTotal *rStation::getTotal(void) const
{
	return &m_total;
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
	list.add(Alias + ".Product"               , TYPE_USINT, rVariable::Flags::RS_L, &m_product               , U_DIMLESS         , ACCESS_SA);
	list.add(Alias + ".Setup"                 , TYPE_UINT , rVariable::Flags::RS_L, &Setup.Value             , U_DIMLESS         , ACCESS_SA);
	list.add(Alias + ".total.present.volume"  , TYPE_LREAL, rVariable::Flags::R___, &m_total.Present.Volume  , m_unit.getVolume(), 0);
	list.add(Alias + ".total.present.volume15", TYPE_LREAL, rVariable::Flags::R___, &m_total.Present.Volume15, m_unit.getVolume(), 0);
	list.add(Alias + ".total.present.volume20", TYPE_LREAL, rVariable::Flags::R___, &m_total.Present.Volume20, m_unit.getVolume(), 0);
	list.add(Alias + ".total.present.mass"    , TYPE_LREAL, rVariable::Flags::R___, &m_total.Present.Mass    , m_unit.getMass()  , 0);
	list.add(Alias + ".total.inc.volume"      , TYPE_LREAL, rVariable::Flags::RSH_, &m_total.Inc.Volume      , m_unit.getVolume(), ACCESS_SA);
	list.add(Alias + ".total.inc.volume15"    , TYPE_LREAL, rVariable::Flags::RSH_, &m_total.Inc.Volume15    , m_unit.getVolume(), ACCESS_SA);
	list.add(Alias + ".total.inc.volume20"    , TYPE_LREAL, rVariable::Flags::RSH_, &m_total.Inc.Volume20    , m_unit.getVolume(), ACCESS_SA);
	list.add(Alias + ".total.inc.mass"        , TYPE_LREAL, rVariable::Flags::RSH_, &m_total.Inc.Mass        , m_unit.getMass()  , ACCESS_SA);
	list.add(Alias + ".total.raw.volume"      , TYPE_LREAL, rVariable::Flags::RSH_, &m_total.Raw.Volume      , m_unit.getVolume(), 0);
	list.add(Alias + ".total.raw.volume15"    , TYPE_LREAL, rVariable::Flags::RSH_, &m_total.Raw.Volume15    , m_unit.getVolume(), 0);
	list.add(Alias + ".total.raw.volume20"    , TYPE_LREAL, rVariable::Flags::RSH_, &m_total.Raw.Volume20    , m_unit.getVolume(), 0);
	list.add(Alias + ".total.raw.mass"        , TYPE_LREAL, rVariable::Flags::RSH_, &m_total.Raw.Mass        , m_unit.getMass()  , 0);
	list.add(Alias + ".total.past.volume"     , TYPE_LREAL, rVariable::Flags::RSH_, &m_total.Past.Volume     , m_unit.getVolume(), 0);
	list.add(Alias + ".total.past.volume15"   , TYPE_LREAL, rVariable::Flags::RSH_, &m_total.Past.Volume15   , m_unit.getVolume(), 0);
	list.add(Alias + ".total.past.volume20"   , TYPE_LREAL, rVariable::Flags::RSH_, &m_total.Past.Volume20   , m_unit.getVolume(), 0);
	list.add(Alias + ".total.past.mass"       , TYPE_LREAL, rVariable::Flags::RSH_, &m_total.Past.Mass       , m_unit.getMass()  , 0);

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

	if (TRITONN_RESULT_OK != m_unit.LoadFromXML(element, err)) {
		return err.getError();
	}

	m_flowMass.Unit     = m_unit.getFlowMass();
	m_flowVolume.Unit   = m_unit.getFlowVolume();
	m_flowVolume15.Unit = m_unit.getFlowVolume();
	m_flowVolume20.Unit = m_unit.getFlowVolume();
	m_temp.Unit         = m_unit.getTemperature();
	m_pres.Unit         = m_unit.getPressure();
	m_dens.Unit         = m_unit.getDensity();

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

rDensity::Product rStation::getProduct() const
{
	return m_product;
}

const rObjUnit& rStation::getUnit() const
{
	return m_unit;
}


