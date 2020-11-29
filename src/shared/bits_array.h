//=================================================================================================
//===
//=== bits_array.h
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

#pragma once

#include <vector>
#include <string>
#include "def.h"

class rBitsArray
{
public:
	rBitsArray();
	virtual ~rBitsArray();

public:
	rBitsArray& add(const std::string &name, UDINT value);
	rBitsArray& add(const std::string &name, UINT  value);
	rBitsArray& add(const std::string &name, USINT value);
	bool        empty() const;
	std::string getNameByBits (UDINT value);
	std::string getNameByValue(UDINT value);
	UDINT       getBit  (const std::string &str, UDINT &err);
	UDINT       getValue(const std::string &name, UDINT &err);


private:
	struct rBitFlag
	{
		std::string m_name;
		UDINT       m_value;

		rBitFlag(const std::string &name, UDINT value) : m_name(name), m_value(value) {}

	};

	std::vector<rBitFlag> m_list;
};

