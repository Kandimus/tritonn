//=================================================================================================
//===
//=== data_sampler.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс пробоотборника
//===
//=================================================================================================

#pragma once

//#include "io.h"
#include "data_source.h"
#include "compared_values.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
//
//

class rSampler : public rSource
{
public:
	rSampler();
	virtual ~rSampler();
	
	// Виртуальные функции от rSource
public:
	virtual const char *RTTI() { return "sampler"; }

	virtual UDINT GetFault();
	virtual UDINT LoadFromXML(tinyxml2::XMLElement *element, rDataConfig &cfg);
	virtual UDINT generateVars(rVariableList& list);
	virtual UDINT Calculate();
protected:
	virtual UDINT InitLimitEvent(rLink &link);
		
};




