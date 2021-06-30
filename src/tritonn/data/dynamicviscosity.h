//=================================================================================================
//===
//=== dynamicviscosity.h
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
class rDynamicViscosity : public rSource
{
public:
	rDynamicViscosity(const rStation* owner = nullptr);
	virtual ~rDynamicViscosity() = default;
	
	// Виртуальные функции от rSource
public:
	virtual const char* getRTTI() const override { return "dynviscosity"; }

	virtual UDINT loadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix) override;
	virtual UDINT generateVars(rVariableList& list) override;
	virtual UDINT generateMarkDown(rGeneratorMD& md) override;
	virtual UDINT calculate() override;
protected:
	virtual UDINT initLimitEvent(rLink& link) override;

public:
	// Inputs, Inoutputs
	rLink m_kinVisc;
	rLink m_density;

	// Outputs
	rLink m_dynVisc;         // Текущее физическое значение
};
