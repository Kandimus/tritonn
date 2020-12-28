﻿//=================================================================================================
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
		std::string m_comment;

		rBitFlag(const std::string& name, UDINT value, const std::string& comment)
			: m_name(name), m_value(value), m_comment(comment) {}

	};

	rBitsArray();
	virtual ~rBitsArray();

public:
	rBitsArray& add(const std::string &name, UDINT value, const std::string& comment = std::string());
	rBitsArray& add(const std::string &name, UINT  value, const std::string& comment = std::string());
	rBitsArray& add(const std::string &name, USINT value, const std::string& comment = std::string());
	std::string getNameByBits (UDINT value) const;
	std::string getNameByValue(UDINT value) const;
	UDINT       getBit  (const std::string &str, UDINT &err) const;
	UDINT       getValue(const std::string &name, UDINT &err) const;
	std::string getMarkDown(const std::string& name) const;

	bool   empty() const { return m_list.empty(); }
	size_t size() const  { return m_list.size(); }


private:
	std::vector<rBitFlag> m_list;
};

