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
#include "data_variable.h"
#include "data_station.h"
#include "data_reduceddensity.h"




const UDINT REDUCEDDENS_LE_INPUTS  = 0x00000001;
const UDINT REDUCEDDENS_LE_DENSITY = 0x00000002;


///////////////////////////////////////////////////////////////////////////////////////////////////
//
rReducedDens::rReducedDens() : rSource()
{
	InitLink(LINK_SETUP_INOUTPUT, Temp  , U_C      , SID_TEMPERATURE, CFGNAME_TEMP     , LINK_SHADOW_NONE );
	InitLink(LINK_SETUP_INOUTPUT, Pres  , U_MPa    , SID_PRESSURE   , CFGNAME_PRES     , LINK_SHADOW_NONE );
	InitLink(LINK_SETUP_INOUTPUT, Dens15, U_kg_m3  , SID_DENSITY15  , CFGNAME_DENSITY15, LINK_SHADOW_NONE );
	InitLink(LINK_SETUP_INOUTPUT, B15   , U_1_C    , SID_B15        , CFGNAME_B15      , CFGNAME_DENSITY15);
	InitLink(LINK_SETUP_OUTPUT  , Dens  , U_kg_m3  , SID_DENSITY    , CFGNAME_DENSITY  , LINK_SHADOW_NONE );
	InitLink(LINK_SETUP_OUTPUT  , Dens20, U_kg_m3  , SID_DENSITY20  , CFGNAME_DENSITY20, LINK_SHADOW_NONE );
	InitLink(LINK_SETUP_OUTPUT  , CTL   , U_DIMLESS, SID_CTL        , CFGNAME_CTL      , LINK_SHADOW_NONE );
	InitLink(LINK_SETUP_OUTPUT  , CPL   , U_DIMLESS, SID_CPL        , CFGNAME_CPL      , LINK_SHADOW_NONE );
	InitLink(LINK_SETUP_OUTPUT  , B     , U_1_C    , SID_B          , CFGNAME_B        , LINK_SHADOW_NONE );
	InitLink(LINK_SETUP_OUTPUT  , Y     , U_1_MPa  , SID_Y          , CFGNAME_Y        , LINK_SHADOW_NONE );
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
	rEvent       event_f;
	rEvent       event_s;
	UDINT        limit   = 0;
	TYPE_PRODUCT product = Station->Product;

	if(rSource::Calculate()) return 0;

//	if(CheckExpr(err, REDUCEDDENS_LE_INPUTS, event_f.Reinit(EID_RDCDDENS_FAULT_INPUTS) << ID << Descr,
//														  event_s.Reinit(EID_RDCDDENS_GOOD_INPUTS ) << ID << Descr))
//	{
//		return SetFault();
//	}

	// Проверка на корректность плотности
	limit = Dens_Limit[0][product] <= Dens15.Value && Dens15.Value < Dens_Limit[1][product];

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
	Dens20.Value = GetDens20(Dens15.Value, B15.Value);

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
UDINT rReducedDens::GenerateVars(vector<rVariable *> &list)
{
	rSource::GenerateVars(list);

	// Внутренние переменные
	list.push_back(new rVariable(Alias + ".fault"    , TYPE_UDINT, VARF_R___, &Fault       , U_DIMLESS, 0));

	return 0;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rReducedDens::LoadFromXML(tinyxml2::XMLElement *element, rDataConfig &cfg)
{
	if(tinyxml2::XML_SUCCESS != rSource::LoadFromXML(element, cfg)) return DATACFGERR_REDUCEDDENS;

	tinyxml2::XMLElement *dens15 = element->FirstChildElement(CFGNAME_DENSITY15);
	tinyxml2::XMLElement *b15    = element->FirstChildElement(CFGNAME_B15);
//	tinyxml2::XMLElement *y15    = element->FirstChildElement(CFGNAME_Y15);
	tinyxml2::XMLElement *temp   = element->FirstChildElement(CFGNAME_TEMP);
	tinyxml2::XMLElement *pres   = element->FirstChildElement(CFGNAME_PRES);

	if(nullptr == temp || nullptr == pres || nullptr == dens15)
	{
		return DATACFGERR_REDUCEDDENS;
	}

	// Обязательные линки и параметры, без которых работа не возможна
	if(tinyxml2::XML_SUCCESS != cfg.LoadLink(temp->FirstChildElement  (CFGNAME_LINK), Temp)  ) return cfg.ErrorID;
	if(tinyxml2::XML_SUCCESS != cfg.LoadLink(pres->FirstChildElement  (CFGNAME_LINK), Pres)  ) return cfg.ErrorID;
	if(tinyxml2::XML_SUCCESS != cfg.LoadLink(dens15->FirstChildElement(CFGNAME_LINK), Dens15)) return cfg.ErrorID;

	// Теневые линки
	if(tinyxml2::XML_SUCCESS != cfg.LoadShadowLink(b15 ? b15->FirstChildElement(CFGNAME_LINK) : nullptr, B15, Dens15, CFGNAME_B15)) return cfg.ErrorID;
//	if(tinyxml2::XML_SUCCESS != cfg.LoadShadowLink(y15 ? y15->FirstChildElement(CFGNAME_LINK) : nullptr, Y15, Dens15, CFGNAME_Y15)) return cfg.ErrorID;

	ReinitLimitEvents();

	return tinyxml2::XML_SUCCESS;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rReducedDens::SaveKernel(FILE *file, UDINT isio, const string &objname, const string &comment, UDINT isglobal)
{
	Temp.Limit.Setup.Init(0);
	Pres.Limit.Setup.Init(0);
	Dens15.Limit.Setup.Init(0);
	B15.Limit.Setup.Init(0);
	Dens.Limit.Setup.Init(0);
	Dens20.Limit.Setup.Init(0);
	CTL.Limit.Setup.Init(0);
	CPL.Limit.Setup.Init(0);
	B.Limit.Setup.Init(0);
	Y.Limit.Setup.Init(0);

	return rSource::SaveKernel(file, isio, objname, comment, isglobal);
}






