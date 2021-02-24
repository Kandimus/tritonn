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
	rReducedDens(const rStation* owner = nullptr);
	virtual ~rReducedDens() = default;
	
	// Виртуальные функции от rSource
public:
	virtual const char* RTTI() const { return "rdcdens"; }

	virtual UDINT       loadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix);
	virtual UDINT       generateVars(rVariableList& list);
	virtual std::string saveKernel(UDINT isio, const std::string& objname, const std::string& comment, UDINT isglobal);
	virtual UDINT       generateMarkDown(rGeneratorMD& md);
	virtual UDINT       calculate();
protected:
	virtual UDINT       initLimitEvent(rLink &link);

public:
	// Inputs
	rLink m_dens15;
	rLink m_b15;
	rLink m_temp;
	rLink m_pres;

	// Outputs
	rLink m_dens;
	rLink m_dens20;
	rLink m_ctl;
	rLink m_cpl;
	rLink m_b;
	rLink m_y;
	rLink m_y15;

protected:
	UDINT setFault();

};



