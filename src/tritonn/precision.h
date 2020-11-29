//=================================================================================================
//===
//=== precision.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс хранения точности отображения, по единицам измерения
//===
//=================================================================================================

#pragma once

#include "def.h"

namespace tinyxml2 {
	class XMLElement;
}

class rError;

class rPrecision
{
public:
	virtual ~rPrecision();

// Singleton
private:
	rPrecision();
	rPrecision(const rPrecision &);
	rPrecision& operator=(rPrecision &);

public:
	static rPrecision &Instance();


public:
	USINT Get(UDINT unit);
	UDINT Reset(USINT def);

	UDINT Load(tinyxml2::XMLElement* element, rError& err);

protected:
	USINT UnitPrec[MAX_UNITS_COUNT];
	USINT DefPrec;
};



