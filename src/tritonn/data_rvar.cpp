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
#include "error.h"
#include "variable_item.h"
#include "variable_list.h"
#include "data_rvar.h"
#include "xml_util.h"



///////////////////////////////////////////////////////////////////////////////////////////////////
//
rRVar::rRVar() : rSource(), Setup(0)
{
	if (m_flagSetup.empty()) {
		m_flagSetup
				.add("CONST", VAR_SETUP_CONST);
	}

	InitLink(rLink::Setup::INOUTPUT | rLink::Setup::NONAME | rLink::Setup::WRITABLE, Value, U_any, SID::VALUE, XmlName::VALUE, rLink::SHADOW_NONE);
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
UDINT rRVar::generateVars(rVariableList& list)
{
	rSource::generateVars(list);

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rRVar::LoadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix)
{
	std::string strSetup = XmlUtils::getAttributeString(element, XmlName::SETUP, "");

	if(TRITONN_RESULT_OK != rSource::LoadFromXML(element, err, prefix)) {
		return err.getError();
	}

	tinyxml2::XMLElement* xml_value = element->FirstChildElement(XmlName::VALUE);
	tinyxml2::XMLElement* xml_unit  = element->FirstChildElement(XmlName::UNIT);

	if(!xml_value || !xml_unit) {
		return err.set(DATACFGERR_VAR, element->GetLineNum(), "fault value or unit");
	}

	UDINT fault = 0;
	Setup = m_flagSetup.getValue(strSetup, fault);

	Value.Value = XmlUtils::getTextLREAL(xml_value, 0.0  , fault);
	Value.Unit  = XmlUtils::getTextUDINT(xml_unit , U_any, fault);

	if (fault) {
		return err.set(DATACFGERR_VAR, element->GetLineNum(), "");
	}

	// Если переменная константа, то снимаем флаг записи
	if(Setup & VAR_SETUP_CONST) {
		Value.m_setup &= ~rLink::Setup::WRITABLE;
	}

	ReinitLimitEvents();

	return TRITONN_RESULT_OK;
}


std::string rRVar::saveKernel(UDINT isio, const std::string &objname, const std::string &comment, UDINT isglobal)
{
	Value.Limit.m_setup.Init(rLimit::Setup::NONE);

	return rSource::saveKernel(isio, objname, comment, isglobal);
}






