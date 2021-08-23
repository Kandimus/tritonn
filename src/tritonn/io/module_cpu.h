//=================================================================================================
//===
//=== module_cpu.h
//===
//=== Copyright (c) 2021 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================

#pragma once

#include <vector>
#include "def.h"
#include "basemodule.h"
#include "baseinterface.h"

class rIOManager;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
class rModuleCPU : public rIOBaseModule, public rIOBaseInterface
{
friend class rIOManager;

public:
	rModuleCPU(UDINT id);
	rModuleCPU(const rModuleCPU* cpu);
	virtual ~rModuleCPU() = default;

	static std::string getRTTI() { return "cpu"; }
	
// rBaseModule
public:
	virtual std::string getModuleType() override { return rModuleCPU::getRTTI(); }
	virtual UDINT processing(USINT issim) override;
	virtual UDINT loadFromXML(tinyxml2::XMLElement* element, rError& err) override;
	virtual UDINT generateVars(const std::string& prefix, rVariableList& list, bool issimulate) override;
	virtual UDINT generateMarkDown(rGeneratorMD& md) override;
	virtual rIOBaseInterface* getModuleInterface() override { return dynamic_cast<rIOBaseInterface*>(this); }

// rIOBaseInterface
public:
	virtual UDINT getPulling() override;
	virtual bool  isFault() const override { return m_isFault; }
	virtual UDINT getValue(USINT num, rIOBaseChannel::Type type, UDINT& fault) override;
	virtual UDINT setValue(USINT num, rIOBaseChannel::Type type, UDINT  value) override;
};



