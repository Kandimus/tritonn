//=================================================================================================
//===
//=== variable_list.cpp
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

#include "variable_list.h"
#include "variable_item.h"
#include "simplefile.h"
#include <algorithm>
#include "text_manager.h"


rVariableList::rVariableList()
{
}

rVariableList::~rVariableList()
{
	for (auto var : m_list) {
		if (var) {
			delete var;
		}
	}
	m_list.clear();
}

void rVariableList::add(const std::string& name, TYPE type, rVariable::Flags flags, void* pointer, STRID unit, UDINT access, const std::string& comment)
{
	const rVariable* var = find(name);

	if (var) {
		//TODO Выдать сообщение
		return;
	}

	m_list.push_back(new rVariable(name, type, flags, pointer, unit, access, comment));
}

void rVariableList::add(const std::string& name, rVariable::Flags flags,  SINT* pointer, STRID unit, UDINT access, const std::string& comment)
{
	add(name, TYPE::SINT, flags, pointer, unit, access, comment);
}

void rVariableList::add(const std::string& name, rVariable::Flags flags, USINT* pointer, STRID unit, UDINT access, const std::string& comment)
{
	add(name, TYPE::USINT, flags, pointer, unit, access, comment);
}

void rVariableList::add(const std::string& name, rVariable::Flags flags,   INT* pointer, STRID unit, UDINT access, const std::string& comment)
{
	add(name, TYPE::INT, flags, pointer, unit, access, comment);
}

void rVariableList::add(const std::string& name, rVariable::Flags flags,  UINT* pointer, STRID unit, UDINT access, const std::string& comment)
{
	add(name, TYPE::UINT, flags, pointer, unit, access, comment);
}

void rVariableList::add(const std::string& name, rVariable::Flags flags,  DINT* pointer, STRID unit, UDINT access, const std::string& comment)
{
	add(name, TYPE::DINT, flags, pointer, unit, access, comment);
}

void rVariableList::add(const std::string& name, rVariable::Flags flags, UDINT* pointer, STRID unit, UDINT access, const std::string& comment)
{
	add(name, TYPE::UDINT, flags, pointer, unit, access, comment);
}

void rVariableList::add(const std::string& name, rVariable::Flags flags,  REAL* pointer, STRID unit, UDINT access, const std::string& comment)
{
	add(name, TYPE::REAL, flags, pointer, unit, access, comment);
}

void rVariableList::add(const std::string& name, rVariable::Flags flags, LREAL* pointer, STRID unit, UDINT access, const std::string& comment)
{
	add(name, TYPE::LREAL, flags, pointer, unit, access, comment);
}

void rVariableList::add(const std::string& name, rVariable::Flags flags, STRID* pointer, STRID unit, UDINT access, const std::string& comment)
{
	add(name, TYPE::STRID, flags, pointer, unit, access, comment);
}

//
rVariable* rVariableList::find(const string &name)
{
	std::string namelower = String_tolower(name);
	UDINT  hash = std::hash<std::string>{}(namelower);

	for(auto var : m_list) {
		if(hash == var->getHash())
		{
			if(var->getName() == namelower)
			{
				return var;
			}
		}
	}

	return nullptr;
}


//-------------------------------------------------------------------------------------------------
// Сортируем список по hash, для более быстрого поиска в будущем
void rVariableList::sort()
{
	std::sort(m_list.begin(), m_list.end(), [](rVariable *a, rVariable *b){ return a < b; });
}


//
void rVariableList::deleteAll()
{
	for(auto var : m_list)
	{
		if (var) {
			delete var;
		}
	}
	m_list.clear();
}


UDINT rVariableList::saveToCSV(const std::string& path)
{
	std::string text = ":alias;type;flags;access;alias hash;\n";

	for (auto var : m_list) {
		text += var->saveToCSV();
	}

	return SimpleFileSave(path + ".variable.csv", text);
}


std::string rVariableList::getMarkDown() const
{
	bool flagMutable = false;
	std::string result = "";

	if (m_list.empty()) {
		return result;
	}

	result += "\n## Variable\n";
	result += "Variable | Type | Unit | Unit ID | Readonly | Access | Comment\n";
	result += ":-- |:--:|:--:|:--:|:--:|:-- |:--\n";
	for (auto item : m_list) {
		std::string strunit = "";
		std::string name    = (item->getName().size() && item->getName()[0] == '.') ? item->getName().substr(1) : item->getName();

		rTextManager::instance().get(item->getUnit(), strunit);

		result += name + (item->isMutable() ? "[^mutable]" : "") + " | ";
		result += getTypeName(item->getType()) + " | ";
		result += strunit + " | " + String_format("%u", static_cast<UDINT>(item->getUnit())) + " | ";
		result += std::string(item->isReadonly() ? "Yes" : "") + " | ";
		result += (item->getAccess() ? String_format("0x%08x", item->getAccess()) : std::string(" ")) + " | ";
		result += item->getComment() + "\n";

		flagMutable |= item->isMutable();
	}

	result += "\n";

	return result;
}
