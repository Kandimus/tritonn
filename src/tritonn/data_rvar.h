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
	virtual UDINT GenerateVars(vector<rVariable *> &list);
	virtual UDINT SaveKernel(FILE *file, UDINT isio, const string &objname, const string &comment, UDINT isglobal);
	virtual UDINT Calculate();
protected:
	virtual UDINT InitLimitEvent(rLink &link);

public:


public:
	// Inputs, Inoutputs

	// Outputs
	rLink     Value;                   // Результирующие значение

	UINT      Setup;                   // Настройка сигнала
};



