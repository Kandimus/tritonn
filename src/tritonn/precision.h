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
#include "singlenton.h"

namespace tinyxml2 {
	class XMLElement;
}

class rError;

class rPrecision
{
	SINGLETON(rPrecision)

public:
	enum
	{
		DEFAUILT = 5,
	};

public:
	USINT get(UDINT unit);
	void  reset(USINT def);

	UDINT load(tinyxml2::XMLElement* element, rError& err);

protected:
	USINT m_unitPrec[MAX_UNITS_COUNT];
	USINT m_defPrec;
};



