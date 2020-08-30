//=================================================================================================
//===
//=== data_interface.cpp
//===
//=== Copyright (c) 2020 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс для всех интерфейсов
//===
//=================================================================================================

#include <limits>
#include "def.h"
#include "tinyxml2.h"
#include "data_config.h"
#include "event_manager.h"
#include "data_variable.h"
#include "data_interface.h"
#include "text_manager.h"



rInterface::rInterface()
{
}

rInterface::~rInterface()
{
printf("~rInterface()");
}


//-------------------------------------------------------------------------------------------------
//


//-------------------------------------------------------------------------------------------------
//
UDINT rInterface::LoadFromXML(tinyxml2::XMLElement *element, rDataConfig &/*cfg*/)
{
	const char *strAlias = element->Attribute("name");

	if(!strAlias) return 1; //TODO Можно еще алиас проверить на валидность имени

	Alias  = String_tolower(strAlias);

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rInterface::SaveKernel(FILE *file, const string &objname, const string &comment)
{
	const string Tag[2] = {"", "io"};
	vector<rVariable *> list;

	GenerateVars(list);

	fprintf(file, "<!--\n\t%s\n-->\n", comment.c_str());

	fprintf(file, "<interface name=\"%s\">\n", objname.c_str());

	fprintf(file, "\t<values>\n");
	for(UDINT ii = 0; ii < list.size(); ++ii)
	{
		rVariable *v = list[ii];

		if(v->Flags & VARF___H_) continue;

		fprintf(file, "\t\t<value name=\"%s\" type=\"%s\" readonly=\"%i\" loadable=\"%i\" unit=\"%i\" access=\"0x%08X\"/>\n",
				  v->Name.c_str() + Alias.size() + 1, NAME_TYPE[v->Type].c_str(), (v->Flags & VARF_R___) ? 1 : 0, (v->Flags & VARF____L) ? 1 : 0, (UDINT)v->Unit, v->Access);
	}
	fprintf(file, "\t</values>\n");


	fprintf(file, "</interface>\n");

	for(UDINT ii = 0; ii < list.size(); ++ii)
	{
		delete list[ii];
	}
	list.clear();

	return 0;
}




