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

#include "data_station.h"
#include <limits>
#include <string.h>
//#include "event/eid.h"
//#include "event/manager.h"
#include "text_id.h"
#include "precision.h"
#include "data_config.h"
#include "data_manager.h"
#include "variable_list.h"
#include "data_stream.h"
#include "xml_util.h"
#include "error.h"
#include "generator_md.h"
#include "comment_defines.h"

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
	m_setup(0),
	m_total(reinitEvent(EID_STATION_TOTAL_MASS),
			reinitEvent(EID_STATION_TOTAL_VOLUME),
			reinitEvent(EID_STATION_TOTAL_VOLUME15),
			reinitEvent(EID_STATION_TOTAL_VOLUME20))
{
	if (m_flagsProduct.empty()) {
		m_flagsProduct
				.add("PETROLEUM"   , static_cast<USINT>(rDensity::Product::PETROLEUM)   , "Нефть")
				.add("GAZOLENE"    , static_cast<USINT>(rDensity::Product::GAZOLENE)    , "Нефтепродукты. Бензины")
				.add("TRANSITION"  , static_cast<USINT>(rDensity::Product::TRANSITION)  , "Нефтепродукты. Топливо, между бензинами и керосинами")
				.add("JETFUEL"     , static_cast<USINT>(rDensity::Product::JETFUEL)     , "Нефтепродукты. Керосины")
				.add("FUELOIL"     , static_cast<USINT>(rDensity::Product::FUELOIL)     , "Нефтепродукты. Дизельное топливо")
				.add("SMARTBENZENE", static_cast<USINT>(rDensity::Product::SMARTBENZENE), "Автоматичский расчет группы нефтепродуктов")
				.add("LUBRICANT"   , static_cast<USINT>(rDensity::Product::LUBRICANT)   , "Смазочные масла");
	}

	m_product  = rDensity::Product::PETROLEUM;

	m_stream.clear();

	initLink(rLink::Setup::INOUTPUT, m_temp        , m_unit.getTemperature(), SID::TEMPERATURE      , XmlName::TEMP         , rLink::SHADOW_NONE);
	initLink(rLink::Setup::INOUTPUT, m_pres        , m_unit.getPressure()   , SID::PRESSURE         , XmlName::PRES         , rLink::SHADOW_NONE);
	initLink(rLink::Setup::INOUTPUT, m_dens        , m_unit.getDensity()    , SID::DENSITY          , XmlName::DENSITY      , rLink::SHADOW_NONE);
	initLink(rLink::Setup::OUTPUT  , m_flowMass    , m_unit.getFlowMass()   , SID::FLOWRATE_MASS    , XmlName::FLOWRATEMASS , rLink::SHADOW_NONE);
	initLink(rLink::Setup::OUTPUT  , m_flowVolume  , m_unit.getFlowVolume() , SID::FLOWRATE_VOLUME  , XmlName::FLOWRATEVOL  , rLink::SHADOW_NONE);
	initLink(rLink::Setup::OUTPUT  , m_flowVolume15, m_unit.getFlowVolume() , SID::FLOWRATE_VOLUME15, XmlName::FLOWRATEVOL15, rLink::SHADOW_NONE);
	initLink(rLink::Setup::OUTPUT  , m_flowVolume20, m_unit.getFlowVolume() , SID::FLOWRATE_VOLUME20, XmlName::FLOWRATEVOL20, rLink::SHADOW_NONE);
}


//-------------------------------------------------------------------------------------------------
//
UDINT rStation::initLimitEvent(rLink& link)
{
	link.m_limit.EventChangeAMin  = reinitEvent(EID_STATION_NEW_AMIN)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeWMin  = reinitEvent(EID_STATION_NEW_WMIN)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeWMax  = reinitEvent(EID_STATION_NEW_WMAX)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeAMax  = reinitEvent(EID_STATION_NEW_AMAX)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeHyst  = reinitEvent(EID_STATION_NEW_HYST)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeSetup = reinitEvent(EID_STATION_NEW_SETUP) << link.m_descr << link.m_unit;
	link.m_limit.EventAMin        = reinitEvent(EID_STATION_AMIN)      << link.m_descr << link.m_unit;
	link.m_limit.EventWMin        = reinitEvent(EID_STATION_WMIN)      << link.m_descr << link.m_unit;
	link.m_limit.EventWMax        = reinitEvent(EID_STATION_WMAX)      << link.m_descr << link.m_unit;
	link.m_limit.EventAMax        = reinitEvent(EID_STATION_AMAX)      << link.m_descr << link.m_unit;
	link.m_limit.EventNan         = reinitEvent(EID_STATION_NAN)       << link.m_descr << link.m_unit;
	link.m_limit.EventNormal      = reinitEvent(EID_STATION_NORMAL)    << link.m_descr << link.m_unit;

	return 0;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rStation::calculate()
{
	UDINT err = 0;

	if (rSource::calculate()) {
		return TRITONN_RESULT_OK;
	}

	// Сброс значений расхода
	m_flowMass.m_value     = 0.0;
	m_flowVolume.m_value   = 0.0;
	m_flowVolume15.m_value = 0.0;
	m_flowVolume20.m_value = 0.0;

	// Расчет весов по линиям и нарастающих
	for (auto str : m_stream) {
		str->calculate();

		// Линия в ремонте (не в учете)
		if(str->m_maintenance) {
			continue;
		}

		// Расход
		m_flowMass.m_value     += str->m_flowMass.m_value;
		m_flowVolume.m_value   += str->m_flowVolume.m_value;
		m_flowVolume15.m_value += str->m_flowVolume15.m_value;
		m_flowVolume20.m_value += str->m_flowVolume20.m_value;

		m_total.m_inc.Count     = 0;
		m_total.m_inc.Mass     += str->m_total.m_inc.Mass;
		m_total.m_inc.Volume   += str->m_total.m_inc.Volume;
		m_total.m_inc.Volume15 += str->m_total.m_inc.Volume15;
		m_total.m_inc.Volume20 += str->m_total.m_inc.Volume20;

		m_total.Calculate(m_unit);
	}

	// Расчет параметров станции
	err = 0;

	for (auto str : m_stream) {
		if (str->m_maintenance) {
			continue;
		}

		if(!m_temp.isValid()) m_temp.m_value += (m_total.m_inc.Mass > 0.0) ? str->getValue(XmlName::TEMP   , m_temp.m_unit, err) * (str->m_total.m_inc.Mass / m_total.m_inc.Mass) : 0.0;
		if(!m_pres.isValid()) m_pres.m_value += (m_total.m_inc.Mass > 0.0) ? str->getValue(XmlName::PRES   , m_pres.m_unit, err) * (str->m_total.m_inc.Mass / m_total.m_inc.Mass) : 0.0;
		if(!m_dens.isValid()) m_dens.m_value += (m_total.m_inc.Mass > 0.0) ? str->getValue(XmlName::DENSITY, m_dens.m_unit, err) * (str->m_total.m_inc.Mass / m_total.m_inc.Mass) : 0.0;
	}

	return TRITONN_RESULT_OK;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//
UDINT rStation::addStream(rStream* str)
{
	m_stream.push_back(str);
	return m_stream.size() - 1;
}

UDINT rStation::getStreamCount() const
{
	return m_stream.size();
}

UDINT rStation::setStreamFreqOut(UDINT strid) const
{
	if (strid >= m_stream.size()) {
		return DATACFGERR_STATION_WRONGSTREAM;
	}

	for (auto item : m_stream) {
		item->disableFreqOut();
	}

	return m_stream[strid]->enableFreqOut();
}

rStream* rStation::getStream(UDINT strid) const
{
	return strid < m_stream.size() ? m_stream[strid] : nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//


//-------------------------------------------------------------------------------------------------
//
UDINT rStation::generateVars(rVariableList& list)
{
	std::string prefix = m_alias + ".total.";

	rSource::generateVars(list);

	// Внутренние переменные
	list.add(m_alias + ".Product"       , TYPE_USINT, rVariable::Flags::RS__, &m_product               , U_DIMLESS         , ACCESS_SA, "Тип продукта:<br/>" + m_flagsProduct.getInfo(true));
//	list.add(m_alias + ".Setup"         , TYPE_UINT , rVariable::Flags::RS_, &m_setup.Value           , U_DIMLESS         , ACCESS_SA, );
	list.add(prefix + "present.volume"  , TYPE_LREAL, rVariable::Flags::R___, &m_total.m_present.Volume  , m_unit.getVolume(), 0        , COMMENT::TOTAL_PRESENT + COMMENT::VOLUME);
	list.add(prefix + "present.volume15", TYPE_LREAL, rVariable::Flags::R___, &m_total.m_present.Volume15, m_unit.getVolume(), 0        , COMMENT::TOTAL_PRESENT + COMMENT::VOLUME15);
	list.add(prefix + "present.volume20", TYPE_LREAL, rVariable::Flags::R___, &m_total.m_present.Volume20, m_unit.getVolume(), 0        , COMMENT::TOTAL_PRESENT + COMMENT::VOLUME20);
	list.add(prefix + "present.mass"    , TYPE_LREAL, rVariable::Flags::R___, &m_total.m_present.Mass    , m_unit.getMass()  , 0        , COMMENT::TOTAL_PRESENT + COMMENT::MASS);
	list.add(prefix + "inc.volume"      , TYPE_LREAL, rVariable::Flags::RSH_, &m_total.m_inc.Volume      , m_unit.getVolume(), ACCESS_SA, COMMENT::TOTAL_INC     + COMMENT::VOLUME);
	list.add(prefix + "inc.volume15"    , TYPE_LREAL, rVariable::Flags::RSH_, &m_total.m_inc.Volume15    , m_unit.getVolume(), ACCESS_SA, COMMENT::TOTAL_INC     + COMMENT::VOLUME15);
	list.add(prefix + "inc.volume20"    , TYPE_LREAL, rVariable::Flags::RSH_, &m_total.m_inc.Volume20    , m_unit.getVolume(), ACCESS_SA, COMMENT::TOTAL_INC     + COMMENT::VOLUME20);
	list.add(prefix + "inc.mass"        , TYPE_LREAL, rVariable::Flags::RSH_, &m_total.m_inc.Mass        , m_unit.getMass()  , ACCESS_SA, COMMENT::TOTAL_INC     + COMMENT::MASS);
	list.add(prefix + "raw.volume"      , TYPE_LREAL, rVariable::Flags::RSH_, &m_total.m_raw.Volume      , m_unit.getVolume(), 0        , COMMENT::TOTAL_RAW     + COMMENT::VOLUME);
	list.add(prefix + "raw.volume15"    , TYPE_LREAL, rVariable::Flags::RSH_, &m_total.m_raw.Volume15    , m_unit.getVolume(), 0        , COMMENT::TOTAL_RAW     + COMMENT::VOLUME15);
	list.add(prefix + "raw.volume20"    , TYPE_LREAL, rVariable::Flags::RSH_, &m_total.m_raw.Volume20    , m_unit.getVolume(), 0        , COMMENT::TOTAL_RAW     + COMMENT::VOLUME20);
	list.add(prefix + "raw.mass"        , TYPE_LREAL, rVariable::Flags::RSH_, &m_total.m_raw.Mass        , m_unit.getMass()  , 0        , COMMENT::TOTAL_RAW     + COMMENT::MASS);
	list.add(prefix + "past.volume"     , TYPE_LREAL, rVariable::Flags::RSH_, &m_total.m_past.Volume     , m_unit.getVolume(), 0        , COMMENT::TOTAL_PAST    + COMMENT::VOLUME);
	list.add(prefix + "past.volume15"   , TYPE_LREAL, rVariable::Flags::RSH_, &m_total.m_past.Volume15   , m_unit.getVolume(), 0        , COMMENT::TOTAL_PAST    + COMMENT::VOLUME15);
	list.add(prefix + "past.volume20"   , TYPE_LREAL, rVariable::Flags::RSH_, &m_total.m_past.Volume20   , m_unit.getVolume(), 0        , COMMENT::TOTAL_PAST    + COMMENT::VOLUME20);
	list.add(prefix + "past.mass"       , TYPE_LREAL, rVariable::Flags::RSH_, &m_total.m_past.Mass       , m_unit.getMass()  , 0        , COMMENT::TOTAL_PAST    + COMMENT::MASS);

	list.add(m_alias + ".fault"         , TYPE_UDINT, rVariable::Flags::R___, &m_fault                 , U_DIMLESS         , 0        , COMMENT::FAULT);

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rStation::loadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix)
{
	std::string strProduct = XmlUtils::getAttributeString(element, XmlName::PRODUCT, m_flagsProduct.getNameByBits(static_cast<USINT>(rDensity::Product::PETROLEUM)));

	if (TRITONN_RESULT_OK != rSource::loadFromXML(element, err, prefix)) {
		return err.getError();
	}

	auto xml_temp = element->FirstChildElement(XmlName::TEMP);
	auto xml_pres = element->FirstChildElement(XmlName::PRES);
	auto xml_dens = element->FirstChildElement(XmlName::DENSITY);

	UDINT fault = 0;
	m_product = static_cast<rDensity::Product>(m_flagsProduct.getValue(strProduct, fault));
	if (fault) {
		return err.set(DATACFGERR_STATION, element->GetLineNum(), strProduct);
	}

	m_setup.Init(0);

	// Параметры ниже могут отсутствовать в конфигурации, в этом случае они будут вычисляться как средневзвешанные
	if (xml_temp) if (TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(xml_temp->FirstChildElement(XmlName::LINK), m_temp)) return err.getError();
	if (xml_pres) if (TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(xml_pres->FirstChildElement(XmlName::LINK), m_pres)) return err.getError();
	if (xml_dens) if (TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(xml_dens->FirstChildElement(XmlName::LINK), m_dens)) return err.getError();

	if (TRITONN_RESULT_OK != m_unit.loadFromXML(element, err)) {
		return err.getError();
	}

	m_flowMass.m_unit     = m_unit.getFlowMass();
	m_flowVolume.m_unit   = m_unit.getFlowVolume();
	m_flowVolume15.m_unit = m_unit.getFlowVolume();
	m_flowVolume20.m_unit = m_unit.getFlowVolume();
	m_temp.m_unit         = m_unit.getTemperature();
	m_pres.m_unit         = m_unit.getPressure();
	m_dens.m_unit         = m_unit.getDensity();

	reinitLimitEvents();

	return TRITONN_RESULT_OK;
}

rDensity::Product rStation::getProduct() const
{
	return m_product;
}

const rObjUnit& rStation::getUnit() const
{
	return m_unit;
}


UDINT rStation::generateMarkDown(rGeneratorMD& md)
{
	m_temp.m_limit.m_setup.Init        (LIMIT_SETUP_ALL | rLimit::Setup::OPTIONAL);
	m_pres.m_limit.m_setup.Init        (LIMIT_SETUP_ALL | rLimit::Setup::OPTIONAL);
	m_dens.m_limit.m_setup.Init        (LIMIT_SETUP_ALL | rLimit::Setup::OPTIONAL);
	m_flowMass.m_limit.m_setup.Init    (LIMIT_SETUP_ALL);
	m_flowVolume.m_limit.m_setup.Init  (LIMIT_SETUP_ALL);
	m_flowVolume15.m_limit.m_setup.Init(LIMIT_SETUP_ALL);
	m_flowVolume20.m_limit.m_setup.Init(LIMIT_SETUP_ALL);

	md.add(this, true, rGeneratorMD::Type::CALCULATE)
//			.addProperty(XmlName::SETUP  , &m_flagsSetup, false)
			.addProperty(XmlName::PRODUCT, &m_flagsProduct, true)
			.addXml("<" + std::string(XmlName::UNITS) + "> " + rGeneratorMD::rItem::XML_OPTIONAL)
			.addXml(XmlName::VOLUME , m_unit.getVolume().toUDINT()     , true, "\t")
			.addXml(XmlName::MASS   , m_unit.getMass().toUDINT()       , true, "\t")
			.addXml(XmlName::TEMP   , m_unit.getTemperature().toUDINT(), true, "\t")
			.addXml(XmlName::PRES   , m_unit.getPressure().toUDINT()   , true, "\t")
			.addXml(XmlName::DENSITY, m_unit.getDensity().toUDINT()    , true, "\t")
			.addXml("</" + std::string(XmlName::UNITS) + ">")
			.addXml("<" + std::string(XmlName::STREAMS) + ">")
			.addXml("\t<!-- list of streams -->")
			.addXml("</" + std::string(XmlName::STREAMS) + ">")
			.addXml("<" + std::string(XmlName::IO) + "> " + rGeneratorMD::rItem::XML_OPTIONAL)
			.addXml("\t<!-- list of io objects -->")
			.addXml("</" + std::string(XmlName::IO) + ">")
			.addXml("<" + std::string(XmlName::CALC) + "> " + rGeneratorMD::rItem::XML_OPTIONAL)
			.addXml("\t<!-- list of calculate objects -->")
			.addXml("</" + std::string(XmlName::CALC) + ">")
			.addRemark("> Если для станции не указаны источники температуры, давления или плотности, то данные параметры будут расчитываться как средневзвешанное по линиям.\n");

	return TRITONN_RESULT_OK;
}

