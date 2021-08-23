/*
 *
 * module_do16.h
 *
 * Copyright (c) 2021 by RangeSoft.
 * All rights reserved.
 *
 * Litvinov "VeduN" Vitaliy O.
 *
 */

#pragma once

#include <vector>
#include "def.h"
#include "basemodule.h"
#include "basechannel.h"
#include "baseinterface.h"
#include "do_channel.h"

class rIOManager;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
class rModuleDO16 : public rIOBaseModule, public rIOBaseInterface
{
friend class rIOManager;

public:

	const UDINT CHANNEL_COUNT = 16;

	rModuleDO16(UDINT id);
	rModuleDO16(const rModuleDO16* di16);
	virtual ~rModuleDO16();

	static std::string getRTTI() { return "do16"; }
	
// rBaseModule
public:
	virtual std::string getModuleType() override { return rModuleDO16::getRTTI(); }
	virtual UDINT processing(USINT issim) override;
	virtual UDINT loadFromXML(tinyxml2::XMLElement* element, rError& err) override;
	virtual UDINT generateVars(const std::string& prefix, rVariableList& list, bool issimulate) override;
	virtual UDINT generateMarkDown(rGeneratorMD& md) override;
	virtual rIOBaseInterface* getModuleInterface() override { return dynamic_cast<rIOBaseInterface*>(this); }

// IOBaseInterface
public:
	virtual UDINT getPulling() override;
	virtual bool  isFault() const override { return m_isFault; }
	virtual UDINT getValue(USINT num, rIOBaseChannel::Type type, UDINT& fault) override;
	virtual UDINT setValue(USINT num, rIOBaseChannel::Type type, UDINT  value) override;

private:
	std::vector<rIODOChannel*> m_channel;
	K19_DO16_str m_data;
};



