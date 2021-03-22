//=================================================================================================
//===
//=== average.h
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
class rAverage : public rSource
{
public:
	rAverage(const rStation* owner = nullptr);
	virtual ~rAverage() = default;

	enum {
		COUNT = 4,
	};
	
	// Виртуальные функции от rSource
public:
	virtual const char *RTTI() const override { return "average"; }

	virtual UDINT loadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix) override;
	virtual UDINT generateVars(rVariableList& list) override;
	virtual UDINT generateMarkDown(rGeneratorMD& md) override;
	virtual UDINT calculate() override;
	virtual UDINT check(rError& err) override;
protected:
	virtual UDINT initLimitEvent(rLink& link) override;

public:
	// Inputs, Inoutputs
	rLink m_inValue[COUNT];          // Текущее физическое значение

	// Outputs
	rLink m_outValue;                // Текущее физическое значение

private:
	UINT m_count = 0;
};



