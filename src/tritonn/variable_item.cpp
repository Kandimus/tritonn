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
	switch(getType()) {
		case TYPE_USINT: return String_format("%hhu", *(USINT *)m_pointer); break;
		case TYPE_SINT : return String_format("%hhi", *(SINT  *)m_pointer); break;
		case TYPE_UINT : return String_format("%hu" , *(UINT  *)m_pointer); break;
		case TYPE_INT  : return String_format("%hi" , *(INT   *)m_pointer); break;
		case TYPE_UDINT: return String_format("%u"  , *(UDINT *)m_pointer); break;
		case TYPE_DINT : return String_format("%i"  , *(DINT  *)m_pointer); break;
		case TYPE_REAL : return String_format("%#g" , *(REAL  *)m_pointer); break;
		case TYPE_LREAL: return String_format("%#g" , *(LREAL *)m_pointer); break;
		case TYPE_STRID: return String_format("%u"  , *(UDINT *)m_pointer); break;
		default: return "";
	}
}

void rVariable::stringToValue(const std::string& strvalue) const
{
	switch(getType()) {
		case TYPE_USINT: { USINT value = atoi(strvalue.c_str()); memcpy(m_pointer, &value, sizeof(value)); return; }
		case TYPE_SINT : { SINT  value = atoi(strvalue.c_str()); memcpy(m_pointer, &value, sizeof(value)); return; }
		case TYPE_UINT : { UINT  value = atoi(strvalue.c_str()); memcpy(m_pointer, &value, sizeof(value)); return; }
		case TYPE_INT  : { INT   value = atoi(strvalue.c_str()); memcpy(m_pointer, &value, sizeof(value)); return; }
		case TYPE_UDINT: { UDINT value = atoi(strvalue.c_str()); memcpy(m_pointer, &value, sizeof(value)); return; }
		case TYPE_DINT : { DINT  value = atoi(strvalue.c_str()); memcpy(m_pointer, &value, sizeof(value)); return; }
		case TYPE_REAL : { REAL  value = atof(strvalue.c_str()); memcpy(m_pointer, &value, sizeof(value)); return; }
		case TYPE_LREAL: { LREAL value = atof(strvalue.c_str()); memcpy(m_pointer, &value, sizeof(value)); return; }
		case TYPE_STRID: { UDINT value = atoi(strvalue.c_str()); memcpy(m_pointer, &value, sizeof(value)); return; }
		default: return;
	}
}
