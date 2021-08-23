/*
 *
 * io/module_ai6a.h
 *
 * Copyright (c) 2019-2021 by RangeSoft.
 * All rights reserved.
 *
 * Litvinov "VeduN" Vitaliy O.
 *
 */

#pragma once

#include <vector>
#include "correctpoint.h"
#include "basemodule.h"
#include "aiinterface.h"
#include "basechannel.h"
#include "rpmsg_connector.h"
#include "ai_channel.h"

class rIOManager;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
class rModuleAI6a : public rIOBaseModule, public rIOAIInterface
{
friend class rIOManager;

public:
	const UDINT CHANNEL_COUNT = 6;

	rModuleAI6a(UDINT id);
	rModuleAI6a(const rModuleAI6a* ai6);
	virtual ~rModuleAI6a();

	static std::string getRTTI() { return "ai6a"; }
	
// rBaseModule
public:
	virtual std::string getModuleType() override { return getRTTI(); }
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

private:
	K19_AI6a_ChType getHardwareModuleChType(UDINT index);
	UDINT checkChannelAccess(USINT num, rIOBaseChannel::Type type);
	void  checkChannelCorrectPoint(USINT idx);
	void  checkCorrectPoint();

private:
	std::vector<rIOAIChannel*> m_channel;
	K19_AI6a_str m_data;

	ModuleCorrectPoint m_correct = ModuleCorrectPoint::NONE;

	static rBitsArray m_flagsCorrect;
};



