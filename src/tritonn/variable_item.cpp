//=================================================================================================
//===
//=== variable_item.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс доступа к объектам и переменным из внешних источников. Массив данных структур
//=== заполняется на старте приложения и остается не изменным.
//===
//=================================================================================================

#include "variable_item.h"
#include <algorithm>
#include <string.h>
#include "xml_util.h"


rVariable::rVariable(const std::string& name, TT_TYPE type, UINT flags, void* pointer, STRID unit, UDINT access, const std::string& comment)
{
	m_name    = String_tolower(name);
	m_type    = type;
	m_flags   = flags;
	m_pointer = pointer;
	m_hash    = std::hash<std::string>{}(m_name);
	m_unit    = unit;
	m_access  = access;
	m_comment = comment;
}

rVariable::rVariable(rVariable *var)
{
	if (!var) {
		return;
	}

	*this = *var;

	var->m_external   = new rExternal;
	var->m_external->m_var = this;
	m_external        = new rExternal;
	m_external->m_var = var;
	m_pointer         = nullptr;
	m_flags          |= Flags::EXTERNAL;
}

//-------------------------------------------------------------------------------------------------
// Конструктор удаляет все дерево переменных, включая дочерние и соседние узлы
rVariable::~rVariable()
{
	if (m_external) {
		delete m_external;
	}
	m_external = nullptr;
}

std::string rVariable::saveToCSV() const
{
	if (isHide()) {
		return "";
	}

	return String_format("%s;%s;%#06x;%08X;%#010x;\n", m_name.c_str(), NAME_TYPE[m_type].c_str(), m_flags, m_access, m_hash);
}

std::string rVariable::valueToXml() const
{
	return "<" + m_name + ">" + valueToString() + "</" + m_name + ">";
}


std::string rVariable::valueToString() const
{
	void *ptr = (isExternal()) ? m_external->m_read : m_pointer;

	switch(getType()) {
		case TYPE_USINT: return String_format("%hhu", *(USINT *)ptr); break;
		case TYPE_SINT : return String_format("%hhi", *(SINT  *)ptr); break;
		case TYPE_UINT : return String_format("%hu" , *(UINT  *)ptr); break;
		case TYPE_INT  : return String_format("%hi" , *(INT   *)ptr); break;
		case TYPE_UDINT: return String_format("%u"  , *(UDINT *)ptr); break;
		case TYPE_DINT : return String_format("%i"  , *(DINT  *)ptr); break;
		case TYPE_REAL : return String_format("%#g" , *(REAL  *)ptr); break;
		case TYPE_LREAL: return String_format("%#g" , *(LREAL *)ptr); break;
		case TYPE_STRID: return String_format("%u"  , *(UDINT *)ptr); break;
		default: return "";
	}
}

void rVariable::stringToValue(const std::string& strvalue) const
{
	switch(getType()) {
		case TYPE_USINT: { USINT value = std::stoul(strvalue); memcpy(m_pointer, &value, sizeof(value)); return; }
		case TYPE_SINT : { SINT  value = std::stoi (strvalue); memcpy(m_pointer, &value, sizeof(value)); return; }
		case TYPE_UINT : { UINT  value = std::stoul(strvalue); memcpy(m_pointer, &value, sizeof(value)); return; }
		case TYPE_INT  : { INT   value = std::stoi (strvalue); memcpy(m_pointer, &value, sizeof(value)); return; }
		case TYPE_UDINT: { UDINT value = std::stoul(strvalue); memcpy(m_pointer, &value, sizeof(value)); return; }
		case TYPE_DINT : { DINT  value = std::stoi (strvalue); memcpy(m_pointer, &value, sizeof(value)); return; }
		case TYPE_REAL : { REAL  value = std::stof (strvalue); memcpy(m_pointer, &value, sizeof(value)); return; }
		case TYPE_LREAL: { LREAL value = std::stod (strvalue); memcpy(m_pointer, &value, sizeof(value)); return; }
		case TYPE_STRID: { UDINT value = std::stoul(strvalue); memcpy(m_pointer, &value, sizeof(value)); return; }
		default: return;
	}
}

void rVariable::valueFromXml(tinyxml2::XMLElement* root)
{
	if (!root) {
		return;
	}

	auto xml_item = root->FirstChildElement(m_name.c_str());

	if (!xml_item) {
		return;
	}

	UDINT       err       = 0;
	std::string textvalue = XmlUtils::getTextString(xml_item, "", err);

	if (err) {
		return;
	}

	stringToValue(textvalue);
}
