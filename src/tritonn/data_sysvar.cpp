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
#include "data_variable.h"


//-------------------------------------------------------------------------------------------------
//
UDINT rSystemVariable::InitVariables(vector<rVariable *> &list)
{
	list.push_back(new rVariable("system.version.major"         , TYPE_USINT, VARF_RS__, &Ver.Major           , U_DIMLESS, ACCESS_SA));
	list.push_back(new rVariable("system.version.minor"         , TYPE_USINT, VARF_RS__, &Ver.Minor           , U_DIMLESS, ACCESS_SA));
	list.push_back(new rVariable("system.version.patch"         , TYPE_USINT, VARF_RS__, &Ver.Patch           , U_DIMLESS, ACCESS_SA));
	list.push_back(new rVariable("system.version.status"        , TYPE_USINT, VARF_RS__, &Ver.Status          , U_DIMLESS, ACCESS_SA));
	list.push_back(new rVariable("system.version.build"         , TYPE_UDINT, VARF_RS__, &Ver.Build           , U_DIMLESS, ACCESS_SA));
	list.push_back(new rVariable("system.version.crc"           , TYPE_UDINT, VARF_RS__, &Ver.CRC             , U_DIMLESS, ACCESS_SA));

	list.push_back(new rVariable("system.state.alarm"           , TYPE_UDINT, VARF_RS__, &State.EventAlarm    , U_DIMLESS, ACCESS_SA));
	list.push_back(new rVariable("system.state.live"            , TYPE_USINT, VARF_RS__, &State.Live          , U_DIMLESS, ACCESS_SA));
	list.push_back(new rVariable("system.state.rebootreason"    , TYPE_USINT, VARF_RS__, &State.StartReason   , U_DIMLESS, ACCESS_SA));

	list.push_back(new rVariable("system.datetime.present.sec"  , TYPE_INT  , VARF_R___, &DateTime.tm_sec     , U_DIMLESS, 0));
	list.push_back(new rVariable("system.datetime.present.min"  , TYPE_INT  , VARF_R___, &DateTime.tm_min     , U_DIMLESS, 0));
	list.push_back(new rVariable("system.datetime.present.hour" , TYPE_INT  , VARF_R___, &DateTime.tm_hour    , U_DIMLESS, 0));
	list.push_back(new rVariable("system.datetime.present.day"  , TYPE_INT  , VARF_R___, &DateTime.tm_mday    , U_DIMLESS, 0));
	list.push_back(new rVariable("system.datetime.present.month", TYPE_INT  , VARF_R___, &DateTime.tm_mon     , U_DIMLESS, 0));
	list.push_back(new rVariable("system.datetime.present.year" , TYPE_INT  , VARF_R___, &DateTime.tm_year    , U_DIMLESS, 0));

	list.push_back(new rVariable("system.datetime.set.sec"      , TYPE_INT  , VARF_____, &SetDateTime.tm_sec  , U_DIMLESS, ACCESS_SYSTEM));
	list.push_back(new rVariable("system.datetime.set.min"      , TYPE_INT  , VARF_____, &SetDateTime.tm_min  , U_DIMLESS, ACCESS_SYSTEM));
	list.push_back(new rVariable("system.datetime.set.hour"     , TYPE_INT  , VARF_____, &SetDateTime.tm_hour , U_DIMLESS, ACCESS_SYSTEM));
	list.push_back(new rVariable("system.datetime.set.day"      , TYPE_INT  , VARF_____, &SetDateTime.tm_mday , U_DIMLESS, ACCESS_SYSTEM));
	list.push_back(new rVariable("system.datetime.set.month"    , TYPE_INT  , VARF_____, &SetDateTime.tm_mon  , U_DIMLESS, ACCESS_SYSTEM));
	list.push_back(new rVariable("system.datetime.set.year"     , TYPE_INT  , VARF_____, &SetDateTime.tm_year , U_DIMLESS, ACCESS_SYSTEM));
	list.push_back(new rVariable("system.datetime.set.accept"   , TYPE_INT  , VARF_____, &SetDateTimeAccept   , U_DIMLESS, ACCESS_SYSTEM));

	return TRITONN_RESULT_OK;
}


UDINT rSystemVariable::SaveKernel(FILE *file)
{
	vector<rVariable *> list;

	InitVariables(list);

	fprintf(file, "<!--\n\tСистемные переменные\n-->\n");

	fprintf(file, "<sysvars>\n");

	fprintf(file, "\t<values>\n");

	for(UDINT ii = 0; ii < list.size(); ++ii)
	{
		rVariable *v = list[ii];

		if(v->Flags & VARF___H_) continue;

		fprintf(file, "\t\t<value name=\"%s\" type=\"%s\" readonly=\"%i\" loadable=\"%i\" unit=\"%i\" access=\"0x%08X\"/>\n",
				  v->Name.c_str(), NAME_TYPE[v->Type].c_str(), (v->Flags & VARF_R___) ? 1 : 0, (v->Flags & VARF____L) ? 1 : 0, (UDINT)v->Unit, v->Access);
	}
	fprintf(file, "\t</values>\n");
	fprintf(file, "</sysvars>\n\n");

	for(UDINT ii = 0; ii < list.size(); ++ii)
	{
		delete list[ii];
	}
	list.clear();

	return TRITONN_RESULT_OK;
}

