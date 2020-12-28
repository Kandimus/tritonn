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
	struct rBitFlag
	{
		std::string m_name;
		UDINT       m_value;
		std::string m_descr;

		rBitFlag(const std::string& name, UDINT value, const std::string& descr)
			: m_name(name), m_value(value), m_descr(descr) {}

	};

	rBitsArray();
	virtual ~rBitsArray();

public:
	rBitsArray& add(const std::string &name, UDINT value);
	rBitsArray& add(const std::string &name, UINT  value);
	rBitsArray& add(const std::string &name, USINT value);
	std::string getNameByBits (UDINT value);
	std::string getNameByValue(UDINT value);
	UDINT       getBit  (const std::string &str, UDINT &err);
	UDINT       getValue(const std::string &name, UDINT &err);
	std::string getBitDescr(UDINT value);

	bool   empty() const { return m_list.empty(); }
	size_t size() const  { return m_list.size(); }
	const rBitFlag* getAt(UDINT id) const { return id < size() ? &m_list[id] : nullptr; }
	std::vector<const rBitFlag*>::iterator begin() { return m_list.begin(); }
	std::vector<const rBitFlag*>::iterator end()   { return m_list.end();   }

private:
	std::vector<rBitFlag> m_list;
};

