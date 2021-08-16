﻿//=================================================================================================
//===
//=== basechannel.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Базовый класс канала
//===
//=================================================================================================

#pragma once

#include "def.h"
#include <string>

class rVariableList;
class rError;
class rBitsArray;

namespace tinyxml2 {
class XMLElement;
}

class rIOBaseChannel
{
public:
	enum Type : USINT
	{
		UNDEF = 0,
		AI,
		FI,
		DI,
		DO,
		AO,
	};

	rIOBaseChannel(Type type, USINT index, const std::string& comment = "")
		: m_index(index), m_comment(comment), m_type(type) {}
	virtual ~rIOBaseChannel() = default;

	std::string getStrType() const;
	Type        getType() const;

	virtual UDINT loadFromXML(tinyxml2::XMLElement* element, rError& err) = 0;
	virtual UDINT generateVars(const std::string& name, rVariableList& list, bool issimulate);
	virtual UDINT processing();
	virtual UDINT simulate() = 0;
	virtual rBitsArray& getFlagsSetup() = 0;

public:
	USINT       m_simType = 0;
	USINT       m_index   = 0xFF;
	USINT       m_canIdx  = 0xFF;
	std::string m_comment = "";

protected:
	Type  m_type = Type::UNDEF;
};



