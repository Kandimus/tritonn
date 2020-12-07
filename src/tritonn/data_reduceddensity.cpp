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


const UDINT REDUCEDDENS_LE_INPUTS  = 0x00000001;
const UDINT REDUCEDDENS_LE_DENSITY = 0x00000002;


///////////////////////////////////////////////////////////////////////////////////////////////////
//
rReducedDens::rReducedDens() : rSource()
{
	InitLink(rLink::Setup::INOUTPUT, Temp  , U_C      , SID::TEMPERATURE, XmlName::TEMP     , rLink::SHADOW_NONE );
	InitLink(rLink::Setup::INOUTPUT, Pres  , U_MPa    , SID::PRESSURE   , XmlName::PRES     , rLink::SHADOW_NONE );
	InitLink(rLink::Setup::INOUTPUT, Dens15, U_kg_m3  , SID::DENSITY15  , XmlName::DENSITY15, rLink::SHADOW_NONE );
	InitLink(rLink::Setup::INOUTPUT, B15   , U_1_C    , SID::B15        , XmlName::B15      , XmlName::DENSITY15);
	InitLink(rLink::Setup::OUTPUT  , Dens  , U_kg_m3  , SID::DENSITY    , XmlName::DENSITY  , rLink::SHADOW_NONE );
	InitLink(rLink::Setup::OUTPUT  , Dens20, U_kg_m3  , SID::DENSITY20  , XmlName::DENSITY20, rLink::SHADOW_NONE );
	InitLink(rLink::Setup::OUTPUT  , CTL   , U_DIMLESS, SID::CTL        , XmlName::CTL      , rLink::SHADOW_NONE );
	InitLink(rLink::Setup::OUTPUT  , CPL   , U_DIMLESS, SID::CPL        , XmlName::CPL      , rLink::SHADOW_NONE );
	InitLink(rLink::Setup::OUTPUT  , B     , U_1_C    , SID::B          , XmlName::B        , rLink::SHADOW_NONE );
	InitLink(rLink::Setup::OUTPUT  , Y     , U_1_MPa  , SID::Y          , XmlName::Y        , rLink::SHADOW_NONE );
}


rReducedDens::~rReducedDens()
{
	;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rReducedDens::InitLimitEvent(rLink &link)
{
	link.Limit.EventChangeAMin  = ReinitEvent(EID_RDCDDENS_NEW_AMIN)  << link.Descr << link.Unit;
	link.Limit.EventChangeWMin  = ReinitEvent(EID_RDCDDENS_NEW_WMIN)  << link.Descr << link.Unit;
	link.Limit.EventChangeWMax  = ReinitEvent(EID_RDCDDENS_NEW_WMAX)  << link.Descr << link.Unit;
	link.Limit.EventChangeAMax  = ReinitEvent(EID_RDCDDENS_NEW_AMAX)  << link.Descr << link.Unit;
	link.Limit.EventChangeHyst  = ReinitEvent(EID_RDCDDENS_NEW_HYST)  << link.Descr << link.Unit;
	link.Limit.EventChangeSetup = ReinitEvent(EID_RDCDDENS_NEW_SETUP) << link.Descr << link.Unit;
	link.Limit.EventAMin        = ReinitEvent(EID_RDCDDENS_AMIN)      << link.Descr << link.Unit;
	link.Limit.EventWMin        = ReinitEvent(EID_RDCDDENS_WMIN)      << link.Descr << link.Unit;
	link.Limit.EventWMax        = ReinitEvent(EID_RDCDDENS_WMAX)      << link.Descr << link.Unit;
	link.Limit.EventAMax        = ReinitEvent(EID_RDCDDENS_AMAX)      << link.Descr << link.Unit;
	link.Limit.EventNan         = ReinitEvent(EID_RDCDDENS_NAN)       << link.Descr << link.Unit;
	link.Limit.EventNormal      = ReinitEvent(EID_RDCDDENS_NORMAL)    << link.Descr << link.Unit;

	return 0;
}

//-------------------------------------------------------------------------------------------------
//





//-------------------------------------------------------------------------------------------------
//
UDINT rReducedDens::Calculate()
{
	rEvent event_f;
	rEvent event_s;
	UDINT  limit = 0;

	if(rSource::Calculate()) return 0;

//	if(CheckExpr(err, REDUCEDDENS_LE_INPUTS, event_f.Reinit(EID_RDCDDENS_FAULT_INPUTS) << ID << Descr,
//														  event_s.Reinit(EID_RDCDDENS_GOOD_INPUTS ) << ID << Descr))
//	{
//		return SetFault();
//	}

	// Проверка на корректность плотности
	USINT product_id = static_cast<USINT>(Station->m_product);
	limit = rDensity::Limit[0][product_id] <= Dens15.Value && Dens15.Value < rDensity::Limit[1][product_id];

	if(CheckExpr(!limit, REDUCEDDENS_LE_DENSITY, ReinitEvent(event_f, EID_RDCDDENS_FAULT_DENSITY)<< Dens15.Value, ReinitEvent(event_s, EID_RDCDDENS_GOOD_DENSITY ) << Dens15.Value))
	{
		Fault = 1;
	}

	LREAL dTemp = Temp.Value - 15.0;

	B.Value      = B15.Value + 1.6 * B15.Value * B15.Value * dTemp;
	Y.Value      = 0.001 * exp(-1.62080 + 0.00021592 * Temp.Value + (870960 + 4209.2 * Temp.Value) / (Dens15.Value * Dens15.Value));
	CPL.Value    = 1.0 / (1.0 - Y.Value * Pres.Value);
	CTL.Value    = exp(-B15.Value * dTemp * (1 + 0.8 * B15.Value * dTemp));
	Dens.Value   = Dens15.Value * CTL.Value * CPL.Value;
	Dens20.Value = rDensity::getDens20(Dens15.Value, B15.Value);

	PostCalculate();
	
	return 0;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rReducedDens::SetFault()
{
	Dens.Value   = std::numeric_limits<LREAL>::quiet_NaN();
	Dens20.Value = std::numeric_limits<LREAL>::quiet_NaN();
	CTL.Value    = std::numeric_limits<LREAL>::quiet_NaN();
	CPL.Value    = std::numeric_limits<LREAL>::quiet_NaN();
	B.Value      = std::numeric_limits<LREAL>::quiet_NaN();
	Y.Value      = std::numeric_limits<LREAL>::quiet_NaN();
	Fault        = 1;

	return Fault;
}



///////////////////////////////////////////////////////////////////////////////////////////////////
//



///////////////////////////////////////////////////////////////////////////////////////////////////
//

///////////////////////////////////////////////////////////////////////////////////////////////////
//
UDINT rReducedDens::generateVars(rVariableList& list)
{
	rSource::generateVars(list);

	// Внутренние переменные
	list.add(Alias + ".fault", TYPE_UDINT, rVariable::Flags::R___, &Fault, U_DIMLESS, 0);

	return 0;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rReducedDens::LoadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix)
{
	if (TRITONN_RESULT_OK != rSource::LoadFromXML(element, err, prefix)) {
		return err.getError();
	}

	tinyxml2::XMLElement* xml_dens15 = element->FirstChildElement(XmlName::DENSITY15);
	tinyxml2::XMLElement* xml_b15    = element->FirstChildElement(XmlName::B15);
	tinyxml2::XMLElement* xml_temp   = element->FirstChildElement(XmlName::TEMP);
	tinyxml2::XMLElement* xml_pres   = element->FirstChildElement(XmlName::PRES);

	if(!xml_temp || !xml_pres || !xml_dens15) {
		return err.set(DATACFGERR_REDUCEDDENS, element->GetLineNum(), "fault input");
	}

	// Обязательные линки и параметры, без которых работа не возможна
	if (TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(xml_temp->FirstChildElement  (XmlName::LINK), Temp)  ) return err.getError();
	if (TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(xml_pres->FirstChildElement  (XmlName::LINK), Pres)  ) return err.getError();
	if (TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(xml_dens15->FirstChildElement(XmlName::LINK), Dens15)) return err.getError();

	// Теневые линки
	if (TRITONN_RESULT_OK != rDataConfig::instance().LoadShadowLink(xml_b15, B15, Dens15, XmlName::B15)) {
		return err.getError();
	}

	ReinitLimitEvents();

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
std::string rReducedDens::saveKernel(UDINT isio, const string &objname, const string &comment, UDINT isglobal)
{
	Temp.Limit.m_setup.Init(rLimit::Setup::NONE);
	Pres.Limit.m_setup.Init(rLimit::Setup::NONE);
	Dens15.Limit.m_setup.Init(rLimit::Setup::NONE);
	B15.Limit.m_setup.Init(rLimit::Setup::NONE);
	Dens.Limit.m_setup.Init(rLimit::Setup::NONE);
	Dens20.Limit.m_setup.Init(rLimit::Setup::NONE);
	CTL.Limit.m_setup.Init(rLimit::Setup::NONE);
	CPL.Limit.m_setup.Init(rLimit::Setup::NONE);
	B.Limit.m_setup.Init(rLimit::Setup::NONE);
	Y.Limit.m_setup.Init(rLimit::Setup::NONE);

	return rSource::saveKernel(isio, objname, comment, isglobal);
}






