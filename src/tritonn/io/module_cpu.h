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

class rIOManager;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
class rModuleCPU : public rIOBaseModule
{
friend class rIOManager;

public:
	rModuleCPU(UDINT id);
	rModuleCPU(const rModuleCPU* cpu);
	virtual ~rModuleCPU() = default;

	static std::string getRTTI() { return "cpu"; }
	
	// Виртуальные функции от rBaseModule
public:
	virtual std::string getModuleType() override { return rModuleCPU::getRTTI(); }
	virtual UDINT processing(USINT issim) override;
	virtual UDINT loadFromXML(tinyxml2::XMLElement* element, rError& err) override;
	virtual UDINT generateVars(const std::string& prefix, rVariableList& list, bool issimulate) override;
	virtual UDINT generateMarkDown(rGeneratorMD& md) override;
	virtual rIOBaseChannel* getChannel(USINT channel, rIOBaseChannel::Type type) override { return nullptr; }
	virtual rIOBaseModule*  getModulePtr() override { return nullptr; }

public:

private:
};



