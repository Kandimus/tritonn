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

std::string rVariable::saveToCSV()
{
	if (isHide()) {
		return "";
	}

	return String_format("%s;%s;%#06x;%08X;%#010x;\n", m_name.c_str(), NAME_TYPE[m_type].c_str(), m_flags, m_access, m_hash);
}

std::string rVariable::saveKernel(UDINT offset, const std::string prefix) const
{
	return String_format("%s<value name=\"%s\" type=\"%s\" readonly=\"%i\" loadable=\"%i\" unit=\"%i\" access=\"0x%08X\"/>\n",
					prefix.c_str(),
					getName().c_str() + offset,
					NAME_TYPE[getType()].c_str(),
					(isReadonly()) ? 1 : 0,
					(isLodable()) ? 1 : 0,
					static_cast<UDINT>(getUnit()),
					getAccess());
}
