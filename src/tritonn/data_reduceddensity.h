//=================================================================================================
//===
//=== data_reduceddensity.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс приведения плотности к требуемым условиям
//===
//=================================================================================================

#pragma once

#include "data_link.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
class rReducedDens : public rSource
{
public:
	rReducedDens();
	virtual ~rReducedDens();
	
	// Виртуальные функции от rSource
public:
	virtual const char *RTTI() const { return "rdcdens"; }

	virtual UDINT LoadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix);
	virtual UDINT generateVars(rVariableList& list);
	virtual std::string saveKernel(UDINT isio, const string &objname, const string &comment, UDINT isglobal);
	virtual UDINT Calculate();
protected:
	virtual UDINT InitLimitEvent(rLink &link);

public:
	// Inputs
	rLink Dens15;
	rLink B15;
	rLink Temp;
	rLink Pres;

	// Outputs
	rLink Dens;
	rLink Dens20;
	rLink CTL;
	rLink CPL;
	rLink B;
	rLink Y;

protected:
	UDINT SetFault();

};



