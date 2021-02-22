//=================================================================================================
//===
//=== data_rvar.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс простой вещественной переменной
//===
//=================================================================================================

#pragma once

#include "data_source.h"
#include "data_link.h"
#include "bits_array.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
class rRVar : public rSource
{
public:
	rRVar();
	virtual ~rRVar() = default;
	
	// Виртуальные функции от rSource
public:
	virtual const char* RTTI() const { return "rvar"; }

	virtual UDINT       loadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix);
	virtual UDINT       generateVars(rVariableList& list);
	virtual std::string saveKernel(UDINT isio, const std::string& objname, const std::string& comment, UDINT isglobal);
	virtual UDINT       calculate();
protected:
	virtual UDINT       initLimitEvent(rLink &link);

public:
	// Inputs, Inoutputs

	// Outputs
	rLink m_value;                   // Результирующие значение

	UINT  m_setup;                   // Настройка сигнала

private:
	rBitsArray m_flagSetup;
};



