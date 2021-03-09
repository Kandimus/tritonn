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
#include "xml_util.h"
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


//-------------------------------------------------------------------------------------------------
//
UDINT rInterface::loadFromXML(tinyxml2::XMLElement *element, rError& err)
{
	const char* strAlias = element->Attribute(XmlName::NAME);

	if (!strAlias) {
		return err.set(DATACFGERR_INTERFACES_BADNAME, element->GetLineNum(), "fault name"); //TODO Можно еще алиас проверить на валидность имени
	}

	m_alias = String_tolower(strAlias);

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
	result += "\t</values>\n</interface>\n";

	return result;
}




