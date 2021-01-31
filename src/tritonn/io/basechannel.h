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

class rIOBaseChannel
{
public:
	rIOBaseChannel() = default;
	virtual ~rIOBaseChannel() = default;

	virtual UDINT generateVars(const std::string& name, rVariableList& list, bool issimulate);
	virtual UDINT processing();
	virtual UDINT simulate() = 0;
	virtual UDINT getPullingCount();

public:
	USINT m_simType = 0;

protected:
	UDINT m_pullingCount = 0;
};



