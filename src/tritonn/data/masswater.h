﻿//=================================================================================================
//===
//=== masswater.h
//===
//=== Copyright (c) 2021 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================

#pragma once

#include "link.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
class rMassWater : public rSource
{
public:

	enum Setup : UINT
	{
		NONE    = 0x0000,
		NOWATER = 0x0001,
	};

	rMassWater(const rStation* owner = nullptr);
	virtual ~rMassWater() = default;
	
	// Виртуальные функции от rSource
public:
	virtual const char* getRTTI() const override { return "masswater"; }

	virtual UDINT loadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix) override;
	virtual UDINT generateVars(rVariableList& list) override;
	virtual UDINT generateMarkDown(rGeneratorMD& md) override;
	virtual UDINT calculate() override;
protected:
	virtual UDINT initLimitEvent(rLink& link) override;

public:
	// Inputs, Inoutputs
	rLink m_volWater;          // Текущее физическое значение
	rLink m_density;
	rLink m_temperature;

	// Outputs
	rLink m_massWater;         // Текущее физическое значение

private:
	LREAL m_waterDensity;
	UINT  m_setup = Setup::NONE;

	static rBitsArray m_flagsSetup;
};



