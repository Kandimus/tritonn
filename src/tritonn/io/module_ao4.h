/*
 *
 * module_ao4.h
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
#include "aointerface.h"
#include "rpmsg_connector.h"
#include "ao_channel.h"

class rIOManager;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
class rModuleAO4 : public rIOBaseModule, public rIOAOInterface
{
friend class rIOManager;

public:
	const UDINT CHANNEL_COUNT = 4;

	rModuleAO4(UDINT id);
	rModuleAO4(const rModuleAO4* ai6);
	virtual ~rModuleAO4();

	static std::string getRTTI() { return "ao4"; }
	
// rBaseModule
public:
	virtual std::string getModuleType() override { return rModuleAO4::getRTTI(); }
	virtual UDINT processing(USINT issim) override;
	virtual UDINT loadFromXML(tinyxml2::XMLElement* element, rError& err) override;
	virtual UDINT generateVars(const std::string& prefix, rVariableList& list, bool issimulate) override;
	virtual UDINT generateMarkDown(rGeneratorMD& md) override;
	virtual rIOBaseInterface* getModuleInterface() override { return dynamic_cast<rIOAOInterface*>(this); }

// rIOAOInterface
public:
	virtual UDINT getPulling() override;
	virtual UDINT getValue(USINT num, rIOBaseChannel::Type type, UDINT& fault) override;
	virtual UDINT setValue(USINT num, rIOBaseChannel::Type type, UDINT  value) override;
	virtual UINT  getMinValue(USINT num, rIOBaseChannel::Type type, UDINT& fault) override;
	virtual UINT  getMaxValue(USINT num, rIOBaseChannel::Type type, UDINT& fault) override;
	virtual UINT  getRange(USINT num, rIOBaseChannel::Type type, UDINT& fault) override;

private:
	UDINT checkChannelAccess(USINT num, rIOBaseChannel::Type type);

private:
	std::vector<rIOAOChannel*> m_channel;
	K19_AO4_str m_data;
};



