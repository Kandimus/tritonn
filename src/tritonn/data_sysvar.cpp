//=================================================================================================
//===
//=== data_sysvar.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс для системных переменных rDataManager
//===
//=================================================================================================

#include "units.h"
#include "data_sysvar.h"
#include "variable_item.h"
#include "variable_list.h"


//-------------------------------------------------------------------------------------------------
//
UDINT rSystemVariable::initVariables(rVariableList& list)
{
	list.add("system.version.major"         , TYPE_USINT, rVariable::Flags::RS__, &Ver.Major           , U_DIMLESS, ACCESS_SA);
	list.add("system.version.minor"         , TYPE_USINT, rVariable::Flags::RS__, &Ver.Minor           , U_DIMLESS, ACCESS_SA);
	list.add("system.version.patch"         , TYPE_USINT, rVariable::Flags::RS__, &Ver.Patch           , U_DIMLESS, ACCESS_SA);
	list.add("system.version.status"        , TYPE_USINT, rVariable::Flags::RS__, &Ver.Status          , U_DIMLESS, ACCESS_SA);
	list.add("system.version.build"         , TYPE_UDINT, rVariable::Flags::RS__, &Ver.Build           , U_DIMLESS, ACCESS_SA);
	list.add("system.version.crc"           , TYPE_UDINT, rVariable::Flags::RS__, &Ver.CRC             , U_DIMLESS, ACCESS_SA);

	list.add("system.state.alarm"           , TYPE_UDINT, rVariable::Flags::RS__, &m_state.EventAlarm  , U_DIMLESS, ACCESS_SA);
	list.add("system.state.live"            , TYPE_USINT, rVariable::Flags::RS__, &m_state.Live        , U_DIMLESS, ACCESS_SA);
	list.add("system.state.rebootreason"    , TYPE_USINT, rVariable::Flags::RS__, &m_state.StartReason , U_DIMLESS, ACCESS_SA);
	list.add("system.state.simulate"        , TYPE_USINT, rVariable::Flags::R___, &m_state.m_isSimulate, U_DIMLESS, 0);

	list.add("system.datetime.present.sec"  , TYPE_INT  , rVariable::Flags::R___, &DateTime.tm_sec     , U_DIMLESS, 0);
	list.add("system.datetime.present.min"  , TYPE_INT  , rVariable::Flags::R___, &DateTime.tm_min     , U_DIMLESS, 0);
	list.add("system.datetime.present.hour" , TYPE_INT  , rVariable::Flags::R___, &DateTime.tm_hour    , U_DIMLESS, 0);
	list.add("system.datetime.present.day"  , TYPE_INT  , rVariable::Flags::R___, &DateTime.tm_mday    , U_DIMLESS, 0);
	list.add("system.datetime.present.month", TYPE_INT  , rVariable::Flags::R___, &DateTime.tm_mon     , U_DIMLESS, 0);
	list.add("system.datetime.present.year" , TYPE_INT  , rVariable::Flags::R___, &DateTime.tm_year    , U_DIMLESS, 0);

	list.add("system.datetime.set.sec"      , TYPE_INT  , rVariable::Flags::____, &SetDateTime.tm_sec  , U_DIMLESS, ACCESS_SYSTEM);
	list.add("system.datetime.set.min"      , TYPE_INT  , rVariable::Flags::____, &SetDateTime.tm_min  , U_DIMLESS, ACCESS_SYSTEM);
	list.add("system.datetime.set.hour"     , TYPE_INT  , rVariable::Flags::____, &SetDateTime.tm_hour , U_DIMLESS, ACCESS_SYSTEM);
	list.add("system.datetime.set.day"      , TYPE_INT  , rVariable::Flags::____, &SetDateTime.tm_mday , U_DIMLESS, ACCESS_SYSTEM);
	list.add("system.datetime.set.month"    , TYPE_INT  , rVariable::Flags::____, &SetDateTime.tm_mon  , U_DIMLESS, ACCESS_SYSTEM);
	list.add("system.datetime.set.year"     , TYPE_INT  , rVariable::Flags::____, &SetDateTime.tm_year , U_DIMLESS, ACCESS_SYSTEM);
	list.add("system.datetime.set.accept"   , TYPE_INT  , rVariable::Flags::____, &SetDateTimeAccept   , U_DIMLESS, ACCESS_SYSTEM);

	return TRITONN_RESULT_OK;
}


std::string rSystemVariable::saveKernel()
{
	std::string   result = "";
	rVariableList list;

	initVariables(list);

	result += "<!--\n\tSystem variables\n-->\n"
			  "<sysvars>\n"
			  "\t<values>\n";

	for (auto var : list) {
		if (var->getFlags() & rVariable::Flags::HIDE) {
			continue;
		}

		result += var->saveKernel(0, "\t\t");
	}
	result += "\t</values>\n"
			  "</sysvars>\n\n";

	return result;
}

