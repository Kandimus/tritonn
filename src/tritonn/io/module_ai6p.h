﻿//=================================================================================================
//===
//=== module_ai6p.h
//===
//=== Copyright (c) 2019-2021 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================

#pragma once

#include <vector>
#include "def.h"
#include "basemodule.h"
#include "aiinterface.h"
#include "basechannel.h"
#include "rpmsg_connector.h"
#include "ai_channel.h"

class rIOManager;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
class rModuleAI6p : public rIOBaseModule, public rIOAIInterface
{
friend class rIOManager;

public:
	const UDINT CHANNEL_COUNT = 6;

	rModuleAI6p(UDINT id);
	rModuleAI6p(const rModuleAI6p* ai6);
	virtual ~rModuleAI6p();

	static std::string getRTTI() { return "ai6p"; }

// rBaseModule
public:
	virtual std::string getModuleType() override { return rModuleAI6p::getRTTI(); }
	virtual UDINT processing(USINT issim) override;
	virtual UDINT loadFromXML(tinyxml2::XMLElement* element, rError& err) override;
	virtual UDINT generateVars(const std::string& prefix, rVariableList& list, bool issimulate) override;
	virtual UDINT generateMarkDown(rGeneratorMD& md) override;
	virtual rIOBaseInterface* getModuleInterface() override { return dynamic_cast<rIOAIInterface*>(this); }

// IOAIInterface
public:
	virtual UDINT getPulling() override;
	virtual bool  isFault() const override { return m_isFault; }
	virtual USINT getState(USINT num, rIOBaseChannel::Type type, UDINT& fault) override;
	virtual UDINT getValue(USINT num, rIOBaseChannel::Type type, UDINT& fault) override;
	virtual UDINT setValue(USINT num, rIOBaseChannel::Type type, UDINT  value) override;
	virtual REAL  getCurrent(USINT num, rIOBaseChannel::Type type, UDINT& fault) override;
	virtual UINT  getMinValue(USINT num, rIOBaseChannel::Type type, UDINT& fault) override;
	virtual UINT  getMaxValue(USINT num, rIOBaseChannel::Type type, UDINT& fault) override;
	virtual UINT  getRange(USINT num, rIOBaseChannel::Type type, UDINT& fault) override;

protected:
	K19_AI6p_ChType getHardwareModuleChType(UDINT index);

private:
	UDINT checkChannelAccess(USINT num, rIOBaseChannel::Type type);

private:
	std::vector<rIOAIChannel*> m_channel;
	K19_AI6p_str m_data;
};



