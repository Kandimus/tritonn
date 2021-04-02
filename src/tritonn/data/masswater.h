//=================================================================================================
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

#include "../data_source.h"
#include "../data_link.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
class rMassWater : public rSource
{
public:
	rMassWater(const rStation* owner = nullptr);
	virtual ~rMassWater() = default;
	
	// Виртуальные функции от rSource
public:
	virtual const char *RTTI() const override { return "masswater"; }

	virtual UDINT loadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix) override;
	virtual UDINT generateVars(rVariableList& list) override;
	virtual UDINT generateMarkDown(rGeneratorMD& md) override;
	virtual UDINT calculate() override;
	virtual UDINT check(rError& err) override;
protected:
	virtual UDINT initLimitEvent(rLink& link) override;

public:
	// Inputs, Inoutputs
	rLink m_volWater;          // Текущее физическое значение
	rLink m_density;

	// Outputs
	rLink m_massWater;         // Текущее физическое значение

private:
	LREAL m_waterDensity;
};



