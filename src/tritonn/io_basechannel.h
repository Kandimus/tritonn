//=================================================================================================
//===
//=== io_basechannel.h
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
	rIOBaseChannel();
	virtual ~rIOBaseChannel();

	virtual UDINT generateVars(const std::string& name, rVariableList& list);
	virtual UDINT processing() = 0;
	virtual UDINT simulate() = 0;

public:
	USINT m_simType = 0;
};



