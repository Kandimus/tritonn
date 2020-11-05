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
#include "variable_item.h"
#include "variable_list.h"
#include "data_interface.h"
#include "text_manager.h"



rInterface::rInterface(pthread_mutex_t& mutex)
	: rVariableClass(mutex)
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
std::string rInterface::saveKernel(const std::string& objname, const std::string& comment)
{
	const std::string Tag[2] = {"", "io"};
	std::string result = "";

	generateVars(nullptr);

	result += String_format("<!--\n\t%s\n-->\n"
							"<interface name=\"%s\">\n", comment.c_str(), objname.c_str());

	result += "\t<values>\n";
	for (auto var : m_varList) {
		if (var->isHide()) {
			continue;
		}

		result += var->saveKernel(Alias.size() + 1, "\t\t");
	}
	result += "\t</values>\n</interface>\n";

	return result;
}




