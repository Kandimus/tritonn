//=================================================================================================
//===
//=== data_rvar.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс простой вещественной переменной
//===
//=================================================================================================

#include <vector>
#include "tinyxml2.h"
#include "text_id.h"
#include "data_manager.h"
#include "data_config.h"
#include "data_variable.h"
#include "data_rvar.h"



///////////////////////////////////////////////////////////////////////////////////////////////////
//
rRVar::rRVar() : rSource(), Setup(0)
{
	InitLink(LINK_SETUP_INOUTPUT | LINK_SETUP_NONAME | LINK_SETUP_WRITEBLE, Value, U_any, SID_VALUE, CFGNAME_VALUE, LINK_SHADOW_NONE);
}


rRVar::~rRVar()
{
	;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rRVar::InitLimitEvent(rLink &link)
{
	link.Limit.EventChangeAMin  = ReinitEvent(EID_VAR_NEW_AMIN)  << link.Descr << link.Unit;
	link.Limit.EventChangeWMin  = ReinitEvent(EID_VAR_NEW_WMIN)  << link.Descr << link.Unit;
	link.Limit.EventChangeWMax  = ReinitEvent(EID_VAR_NEW_WMAX)  << link.Descr << link.Unit;
	link.Limit.EventChangeAMax  = ReinitEvent(EID_VAR_NEW_AMAX)  << link.Descr << link.Unit;
	link.Limit.EventChangeHyst  = ReinitEvent(EID_VAR_NEW_HYST)  << link.Descr << link.Unit;
	link.Limit.EventChangeSetup = ReinitEvent(EID_VAR_NEW_SETUP) << link.Descr << link.Unit;
	link.Limit.EventAMin        = ReinitEvent(EID_VAR_AMIN)      << link.Descr << link.Unit;
	link.Limit.EventWMin        = ReinitEvent(EID_VAR_WMIN)      << link.Descr << link.Unit;
	link.Limit.EventWMax        = ReinitEvent(EID_VAR_WMAX)      << link.Descr << link.Unit;
	link.Limit.EventAMax        = ReinitEvent(EID_VAR_AMAX)      << link.Descr << link.Unit;
	link.Limit.EventNan         = ReinitEvent(EID_VAR_NAN)       << link.Descr << link.Unit;
	link.Limit.EventNormal      = ReinitEvent(EID_VAR_NORMAL)    << link.Descr << link.Unit;

	return 0;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rRVar::Calculate()
{
	if(rSource::Calculate()) return 0;

	//----------------------------------------------------------------------------------------------
	// Обрабатываем Limits для выходных значений
	PostCalculate();
		
	return 0;
}





///////////////////////////////////////////////////////////////////////////////////////////////////
//



///////////////////////////////////////////////////////////////////////////////////////////////////
//

//-------------------------------------------------------------------------------------------------
//
UDINT rRVar::GenerateVars(vector<rVariable *> &list)
{
	rSource::GenerateVars(list);

	return 0;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rRVar::LoadFromXML(tinyxml2::XMLElement *element, rDataConfig &cfg)
{
	string strSetup = (element->Attribute(CFGNAME_SETUP)) ? element->Attribute(CFGNAME_SETUP) : "";
	UDINT  err = 0;

	if(tinyxml2::XML_SUCCESS != rSource::LoadFromXML(element, cfg)) return DATACFGERR_VAR;

	tinyxml2::XMLElement *xml_value = element->FirstChildElement(CFGNAME_VALUE);
	tinyxml2::XMLElement *xml_unit  = element->FirstChildElement(CFGNAME_UNIT);

	if(nullptr == xml_value || nullptr == xml_unit)
	{
		return DATACFGERR_VAR;
	}

	Setup = rDataConfig::GetFlagFromStr(rDataConfig::VarSetupFlags, strSetup, err);

	Value.Value = rDataConfig::GetTextLREAL(xml_value, 0.0  , err);
	Value.Unit  = rDataConfig::GetTextUDINT(xml_unit , U_any, err);

	if(err)
	{
		return DATACFGERR_VAR;
	}

	// Если переменная константа, то снимаем флаг записи
	if(Setup & VAR_SETUP_CONST)
	{
		Value.Setup &= ~LINK_SETUP_WRITEBLE;
	}

	ReinitLimitEvents();

	return tinyxml2::XML_SUCCESS;
}


UDINT rRVar::SaveKernel(FILE *file, UDINT isio, const string &objname, const string &comment, UDINT isglobal)
{
	Value.Limit.Setup.Init(0);

	return rSource::SaveKernel(file, isio, objname, comment, isglobal);
}






