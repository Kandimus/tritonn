//=================================================================================================
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

namespace tinyxml2 {
class XMLElement;
}

class rIOBaseChannel
{
public:
	rIOBaseChannel(USINT index) : m_index(index) {}
	virtual ~rIOBaseChannel() = default;

	virtual UDINT loadFromXML(tinyxml2::XMLElement* element, rError& err) = 0;
	virtual UDINT generateVars(const std::string& name, rVariableList& list, bool issimulate);
	virtual UDINT processing();
	virtual UDINT simulate() = 0;
	virtual UDINT getPullingCount();

public:
	USINT m_simType = 0;
	USINT m_index   = 0xFF;

protected:
	UDINT m_pullingCount = 0;
};



