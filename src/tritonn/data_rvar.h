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
	virtual ~rRVar();
	
	// Виртуальные функции от rSource
public:
	virtual const char *RTTI() { return "rvar"; }

	virtual UDINT LoadFromXML(tinyxml2::XMLElement *element, rDataConfig &cfg);
	virtual UDINT generateVars(rVariableList& list);
	virtual std::string saveKernel(UDINT isio, const string &objname, const string &comment, UDINT isglobal);
	virtual UDINT Calculate();
protected:
	virtual UDINT InitLimitEvent(rLink &link);

public:


// Inputs, Inoutputs
public:

// Outputs
public:
	rLink     Value;                   // Результирующие значение

public:
	UINT      Setup;                   // Настройка сигнала

private:
	rBitsArray m_flagSetup;
};



