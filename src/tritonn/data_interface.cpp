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
}


//-------------------------------------------------------------------------------------------------
//


//-------------------------------------------------------------------------------------------------
//
UDINT rInterface::loadFromXML(tinyxml2::XMLElement *element, rDataConfig &/*cfg*/)
{
	const char *strAlias = element->Attribute("name");

	if(!strAlias) return 1; //TODO Можно еще алиас проверить на валидность имени

	Alias  = String_tolower(strAlias);

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rInterface::saveKernel(FILE *file, const string &objname, const string &comment)
{
	const string Tag[2] = {"", "io"};
	rVariableList list;

	generateVars(list);

	fprintf(file, "<!--\n\t%s\n-->\n", comment.c_str());

	fprintf(file, "<interface name=\"%s\">\n", objname.c_str());

	fprintf(file, "\t<values>\n");
	for (auto var : list) {
		if (var->isHide()) {
			continue;
		}

		fprintf(file, "\t\t<value name=\"%s\" type=\"%s\" readonly=\"%i\" loadable=\"%i\" unit=\"%i\" access=\"0x%08X\"/>\n",
				var->getName().c_str() + Alias.size() + 1, NAME_TYPE[var->getType()].c_str(),
				(var->isReadonly()) ? 1 : 0,
				(var->isLodable()) ? 1 : 0,
				(UDINT)var->getUnit(), var->getAccess());
	}
	fprintf(file, "\t</values>\n");


	fprintf(file, "</interface>\n");

	return TRITONN_RESULT_OK;
}




