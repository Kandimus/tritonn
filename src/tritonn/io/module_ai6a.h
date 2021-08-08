//=================================================================================================
//===
//=== module_ai6a.h
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
#include "basechannel.h"
#include "rpmsg_connector.h"
#include "ai_channel.h"

class rIOManager;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
class rModuleAI6a : public rIOBaseModule
{
friend class rIOManager;

public:

	const UDINT CHANNEL_COUNT = 6;

	rModuleAI6a(UDINT id);
	rModuleAI6a(const rModuleAI6a* ai6);
	virtual ~rModuleAI6a();

	static std::string getRTTI() { return "ai6a"; }
	
	// Виртуальные функции от rBaseModule
public:
	virtual std::string getModuleType() override { return rModuleAI6a::getRTTI(); }
	virtual UDINT processing(USINT issim) override;
	virtual UDINT loadFromXML(tinyxml2::XMLElement* element, rError& err) override;
	virtual UDINT generateVars(const std::string& prefix, rVariableList& list, bool issimulate) override;
	virtual UDINT generateMarkDown(rGeneratorMD& md) override;
	virtual rIOBaseChannel* getChannel(USINT channel, rIOBaseChannel::Type type) override;
	virtual rIOBaseModule*  getModulePtr() override { return new rModuleAI6a(this); }

protected:
	K19_AI6a_ChType getHardwareModuleChType(UDINT index);

public:
	UINT getADC(USINT id);
	REAL getCurrent(USINT id);
	rIOAIChannel::Type getType(USINT id);
	USINT getState(USINT id);

private:
	std::vector<rIOAIChannel*> m_channel;
	K19_AI6a_str m_data;
};



