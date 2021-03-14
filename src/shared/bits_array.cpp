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
	m_list.push_back(rBitsArray::rBitFlag(name, value, comment));
	return *this;
}

rBitsArray& rBitsArray::add(const std::string &name, USINT value, const std::string& comment)
{
	m_list.push_back(rBitsArray::rBitFlag(name, value, comment));
	return *this;
}

//-------------------------------------------------------------------------------------------------
// Возвращает строковое название флага, если выставлены несколько бит, то вернется срока с
// несколькими флагами
std::string rBitsArray::getNameByBits(UDINT value, const std::string& delim) const
{
	std::string result = "";

	for (auto& item : m_list) {
		if(item.m_value & value && item.m_name.size())
		{
			result += ((result.size()) ? delim : "") + item.m_name;
		}
	}

	return result;
}


//-------------------------------------------------------------------------------------------------
// Возвращает строковое значение числа, проверка идет на равенство
std::string rBitsArray::getNameByValue(UDINT value) const
{
	for(auto& item : m_list) {
		if (item.m_value == value && item.m_name.size()) {
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
		if(item.m_name == name && item.m_name.size()) {
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
		if (item.m_name.size()) {
			result += "* _" + item.m_name + "_ " + (item.m_comment.size() ? " - " : "") + item.m_comment + "\n";
		}
	}

	return result;
}

std::string rBitsArray::getInfo(bool isnumbers) const
{
	std::string result = "";

	for (auto& item : m_list) {
		if (isnumbers) {
			result += String_format("%u", item.m_value);

		} else {
			switch(item.m_type) {
				case TYPE_UDINT: result += String_format("0x%08x", item.m_value); break;
				case TYPE_UINT : result += String_format("0x%04x", item.m_value); break;
				case TYPE_USINT: result += String_format("0x%02x", item.m_value); break;

				default: result += String_format("unknow type %i", item.m_value); break;
			}
		}

		result += ": " + item.m_comment+ "<br/>";
	}

	return result;
}
