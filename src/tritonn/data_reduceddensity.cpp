//=================================================================================================
//===
//=== data_reduceddensity.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс приведения плотности к требуемым условиям
//===
//=================================================================================================

#include <math.h>
#include <vector>
#include <limits>
#include "tinyxml2.h"
#include "density.h"
#include "event_eid.h"
#include "text_id.h"
#include "event_manager.h"
#include "data_manager.h"
#include "data_config.h"
#include "variable_item.h"
#include "variable_list.h"
#include "data_station.h"
#include "data_reduceddensity.h"
#include "xml_util.h"
#include "generator_md.h"


const UDINT REDUCEDDENS_LE_DENSITY = 0x00000002;


///////////////////////////////////////////////////////////////////////////////////////////////////
//
rReducedDens::rReducedDens(const rStation* owner) : rSource(owner)
{
	initLink(rLink::Setup::INOUTPUT, m_temp  , U_C      , SID::TEMPERATURE, XmlName::TEMP     , rLink::SHADOW_NONE );
	initLink(rLink::Setup::INOUTPUT, m_pres  , U_MPa    , SID::PRESSURE   , XmlName::PRES     , rLink::SHADOW_NONE );
	initLink(rLink::Setup::INOUTPUT, m_dens15, U_kg_m3  , SID::DENSITY15  , XmlName::DENSITY15, rLink::SHADOW_NONE );
	initLink(rLink::Setup::INOUTPUT, m_b15   , U_1_C    , SID::B15        , XmlName::B15      , XmlName::DENSITY15);
	initLink(rLink::Setup::OUTPUT  , m_dens  , U_kg_m3  , SID::DENSITY    , XmlName::DENSITY  , rLink::SHADOW_NONE );
	initLink(rLink::Setup::OUTPUT  , m_dens20, U_kg_m3  , SID::DENSITY20  , XmlName::DENSITY20, rLink::SHADOW_NONE );
	initLink(rLink::Setup::OUTPUT  , m_ctl   , U_DIMLESS, SID::CTL        , XmlName::CTL      , rLink::SHADOW_NONE );
	initLink(rLink::Setup::OUTPUT  , m_cpl   , U_DIMLESS, SID::CPL        , XmlName::CPL      , rLink::SHADOW_NONE );
	initLink(rLink::Setup::OUTPUT  , m_b     , U_1_C    , SID::B          , XmlName::B        , rLink::SHADOW_NONE );
	initLink(rLink::Setup::OUTPUT  , m_y     , U_1_MPa  , SID::Y          , XmlName::Y        , rLink::SHADOW_NONE );
	initLink(rLink::Setup::OUTPUT  , m_y15   , U_1_MPa  , SID::Y15        , XmlName::Y15      , rLink::SHADOW_NONE );
}


//-------------------------------------------------------------------------------------------------
//
UDINT rReducedDens::initLimitEvent(rLink &link)
{
	link.m_limit.EventChangeAMin  = reinitEvent(EID_RDCDDENS_NEW_AMIN)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeWMin  = reinitEvent(EID_RDCDDENS_NEW_WMIN)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeWMax  = reinitEvent(EID_RDCDDENS_NEW_WMAX)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeAMax  = reinitEvent(EID_RDCDDENS_NEW_AMAX)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeHyst  = reinitEvent(EID_RDCDDENS_NEW_HYST)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeSetup = reinitEvent(EID_RDCDDENS_NEW_SETUP) << link.m_descr << link.m_unit;
	link.m_limit.EventAMin        = reinitEvent(EID_RDCDDENS_AMIN)      << link.m_descr << link.m_unit;
	link.m_limit.EventWMin        = reinitEvent(EID_RDCDDENS_WMIN)      << link.m_descr << link.m_unit;
	link.m_limit.EventWMax        = reinitEvent(EID_RDCDDENS_WMAX)      << link.m_descr << link.m_unit;
	link.m_limit.EventAMax        = reinitEvent(EID_RDCDDENS_AMAX)      << link.m_descr << link.m_unit;
	link.m_limit.EventNan         = reinitEvent(EID_RDCDDENS_NAN)       << link.m_descr << link.m_unit;
	link.m_limit.EventNormal      = reinitEvent(EID_RDCDDENS_NORMAL)    << link.m_descr << link.m_unit;

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rReducedDens::calculate()
{
	rEvent event_f;
	rEvent event_s;

	if(rSource::calculate()) {
		return TRITONN_RESULT_OK;
	}

//	if(CheckExpr(err, REDUCEDDENS_LE_INPUTS, event_f.Reinit(EID_RDCDDENS_FAULT_INPUTS) << ID << Descr,
//														  event_s.Reinit(EID_RDCDDENS_GOOD_INPUTS ) << ID << Descr))
//	{
//		return SetFault();
//	}

	// Проверка на корректность плотности
	rDensity::Product product_id = m_station->getProduct();
	bool limit = rDensity::getMinLimit(product_id) <= m_dens15.m_value && m_dens15.m_value < rDensity::getMaxLimit(product_id);

	if(checkExpr(!limit, REDUCEDDENS_LE_DENSITY,
				 reinitEvent(event_f, EID_RDCDDENS_FAULT_DENSITY) << m_dens15.m_value,
				 reinitEvent(event_s, EID_RDCDDENS_GOOD_DENSITY ) << m_dens15.m_value)) {
		m_fault = 1;
	}

	LREAL dTemp = m_temp.m_value - 15.0;

	m_b.m_value      = m_b15.m_value + 1.6 * m_b15.m_value * m_b15.m_value * dTemp;
	m_y.m_value      = 0.001 * exp(-1.62080 + 0.00021592 * m_temp.m_value + (870960 + 4209.2 * m_temp.m_value) / (m_dens15.m_value * m_dens15.m_value));
	m_cpl.m_value    = 1.0 / (1.0 - m_y.m_value * m_pres.m_value);
	m_ctl.m_value    = exp(-m_b15.m_value * dTemp * (1 + 0.8 * m_b15.m_value * dTemp));
	m_dens.m_value   = m_dens15.m_value * m_ctl.m_value * m_cpl.m_value;
	m_dens20.m_value = rDensity::getDens20(m_dens15.m_value, m_b15.m_value);
	m_y15.m_value    = rDensity::getY15(m_dens15.m_value);

	postCalculate();
	
	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rReducedDens::setFault()
{
	m_dens.m_value   = std::numeric_limits<LREAL>::quiet_NaN();
	m_dens20.m_value = std::numeric_limits<LREAL>::quiet_NaN();
	m_ctl.m_value    = std::numeric_limits<LREAL>::quiet_NaN();
	m_cpl.m_value    = std::numeric_limits<LREAL>::quiet_NaN();
	m_b.m_value      = std::numeric_limits<LREAL>::quiet_NaN();
	m_y.m_value      = std::numeric_limits<LREAL>::quiet_NaN();
	m_fault          = 1;

	return m_fault;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//
UDINT rReducedDens::generateVars(rVariableList& list)
{
	rSource::generateVars(list);

	// Внутренние переменные
	list.add(m_alias + ".fault", TYPE_UDINT, rVariable::Flags::R___, &m_fault, U_DIMLESS, 0);

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rReducedDens::loadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix)
{
	if (TRITONN_RESULT_OK != rSource::loadFromXML(element, err, prefix)) {
		return err.getError();
	}

	auto xml_dens15 = element->FirstChildElement(XmlName::DENSITY15);
	auto xml_b15    = element->FirstChildElement(XmlName::B15);
	auto xml_temp   = element->FirstChildElement(XmlName::TEMP);
	auto xml_pres   = element->FirstChildElement(XmlName::PRES);

	if(!xml_temp || !xml_pres || !xml_dens15) {
		return err.set(DATACFGERR_REDUCEDDENS, element->GetLineNum(), "fault input");
	}

	// Обязательные линки и параметры, без которых работа не возможна
	if (TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(xml_temp->FirstChildElement  (XmlName::LINK), m_temp)  ) return err.getError();
	if (TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(xml_pres->FirstChildElement  (XmlName::LINK), m_pres)  ) return err.getError();
	if (TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(xml_dens15->FirstChildElement(XmlName::LINK), m_dens15)) return err.getError();

	// Теневые линки
	if (TRITONN_RESULT_OK != rDataConfig::instance().LoadShadowLink(xml_b15, m_b15, m_dens15, XmlName::B15)) {
		return err.getError();
	}

	reinitLimitEvents();

	return TRITONN_RESULT_OK;
}


UDINT rReducedDens::generateMarkDown(rGeneratorMD& md)
{
	m_temp.m_limit.m_setup.Init(rLimit::Setup::HIHI | rLimit::Setup::HI | rLimit::Setup::LO | rLimit::Setup::LOLO);
	m_pres.m_limit.m_setup.Init(rLimit::Setup::HIHI | rLimit::Setup::HI | rLimit::Setup::LO | rLimit::Setup::LOLO);
	m_dens15.m_limit.m_setup.Init(rLimit::Setup::HIHI | rLimit::Setup::HI | rLimit::Setup::LO | rLimit::Setup::LOLO);
	m_b15.m_limit.m_setup.Init(rLimit::Setup::HIHI | rLimit::Setup::HI | rLimit::Setup::LO | rLimit::Setup::LOLO);
	m_dens.m_limit.m_setup.Init(rLimit::Setup::HIHI | rLimit::Setup::HI | rLimit::Setup::LO | rLimit::Setup::LOLO);
	m_dens20.m_limit.m_setup.Init(rLimit::Setup::HIHI | rLimit::Setup::HI | rLimit::Setup::LO | rLimit::Setup::LOLO);
	m_ctl.m_limit.m_setup.Init(rLimit::Setup::HIHI | rLimit::Setup::HI | rLimit::Setup::LO | rLimit::Setup::LOLO);
	m_cpl.m_limit.m_setup.Init(rLimit::Setup::HIHI | rLimit::Setup::HI | rLimit::Setup::LO | rLimit::Setup::LOLO);
	m_b.m_limit.m_setup.Init(rLimit::Setup::HIHI | rLimit::Setup::HI | rLimit::Setup::LO | rLimit::Setup::LOLO);
	m_y.m_limit.m_setup.Init(rLimit::Setup::HIHI | rLimit::Setup::HI | rLimit::Setup::LO | rLimit::Setup::LOLO);
	m_y15.m_limit.m_setup.Init(rLimit::Setup::HIHI | rLimit::Setup::HI | rLimit::Setup::LO | rLimit::Setup::LOLO);

	md.add(this, true);

	return TRITONN_RESULT_OK;
}

//-------------------------------------------------------------------------------------------------
//
std::string rReducedDens::saveKernel(UDINT isio, const string &objname, const string &comment, UDINT isglobal)
{
	m_temp.m_limit.m_setup.Init(rLimit::Setup::NONE);
	m_pres.m_limit.m_setup.Init(rLimit::Setup::NONE);
	m_dens15.m_limit.m_setup.Init(rLimit::Setup::NONE);
	m_b15.m_limit.m_setup.Init(rLimit::Setup::NONE);
	m_dens.m_limit.m_setup.Init(rLimit::Setup::NONE);
	m_dens20.m_limit.m_setup.Init(rLimit::Setup::NONE);
	m_ctl.m_limit.m_setup.Init(rLimit::Setup::NONE);
	m_cpl.m_limit.m_setup.Init(rLimit::Setup::NONE);
	m_b.m_limit.m_setup.Init(rLimit::Setup::NONE);
	m_y.m_limit.m_setup.Init(rLimit::Setup::NONE);
	m_y15.m_limit.m_setup.Init(rLimit::Setup::NONE);

	return rSource::saveKernel(isio, objname, comment, isglobal);
}

