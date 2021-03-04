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
	enum Setup : UINT
	{
		CONST = 0x0001,
	};

	rRVar();
	virtual ~rRVar() = default;
	
	// Виртуальные функции от rSource
public:
	virtual const char* RTTI() const override { return "rvar"; }

	virtual UDINT loadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix) override;
	virtual UDINT generateVars(rVariableList& list) override;
	virtual UDINT generateMarkDown(rGeneratorMD& md) override;
	virtual UDINT calculate() override;
protected:
	virtual UDINT initLimitEvent(rLink &link) override;

public:
	// Inputs, Inoutputs

	// Outputs
	rLink m_value;                   // Результирующие значение

	UINT  m_setup;                   // Настройка сигнала

private:
	static rBitsArray m_flagsSetup;
};



