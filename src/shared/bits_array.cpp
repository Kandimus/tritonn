//=================================================================================================
//===
//=== bits_array.cpp
//===
//=== Copyright (c) 2020 by VeduN, RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс для перевода строк в биты
//===
//=================================================================================================


#include "bits_array.h"

rBitsArray::rBitsArray()
{

}

rBitsArray::~rBitsArray()
{
	m_list.clear();
}


rBitsArray& rBitsArray::add(const std::string &name, UDINT value, const std::string& comment)
{
	m_list.push_back(rBitsArray::rBitFlag(name, value, comment));
	return *this;
}

rBitsArray& rBitsArray::add(const std::string &name, UINT value, const std::string& comment)
{
	return add(name, static_cast<UDINT>(value), comment);
}

rBitsArray& rBitsArray::add(const std::string &name, USINT value, const std::string& comment)
{
	return add(name, static_cast<UDINT>(value), comment);
}

//-------------------------------------------------------------------------------------------------
// Возвращает строковое название флага, если выставлены несколько бит, то вернется срока с
// несколькими флагами
std::string rBitsArray::getNameByBits(UDINT value) const
{
	std::string result = "";

	for (auto& item : m_list) {
		if(item.m_value & value)
		{
			result += ((result.size()) ? "|" : "") + item.m_name;
		}
	}

	return result;
}


//-------------------------------------------------------------------------------------------------
// Возвращает строковое значение числа, проверка идет на равенство
std::string rBitsArray::getNameByValue(UDINT value) const
{
	for(auto& item : m_list) {
		if (item.m_value == value) {
			return item.m_name;
		}
	}

	return "";
}


//-------------------------------------------------------------------------------------------------
//
UDINT rBitsArray::getBit(const std::string &name, UDINT &err) const
{
	if(err) return 0;
	if(name.empty()) return 0;

	for(auto& item : m_list) {
		if(item.m_name == name) {
			return item.m_value;
		}
	}

	err = 1;
	return 0;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rBitsArray::getValue(const std::string &str, UDINT &err) const
{
	std::string name   = "";
	UDINT       result = 0;

	if(err) return result;
	if(str.empty()) return result;

	for(auto ch : str)
	{
		if(' ' == ch || '|' == ch)
		{
			if(name.size())
			{
				result |= getBit(name, err);
				name    = "";

				if(err) break;
			}
		}
		else
		{
			name += ch;
		}
	}
	if(name.size())
	{
		result |= getBit(name, err);
	}

	return result;
}


std::string rBitsArray::getMarkDown(const std::string& name) const
{
	std::string result = "\n#### " + name + "\n";

	for (auto& item : m_list) {
		result += "* _" + item.m_name + "_ " + (item.m_comment.size() ? " - " : "") + item.m_comment + "\n";
	}

	return result;
}
