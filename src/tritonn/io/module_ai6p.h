//=================================================================================================
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
#include "basechannel.h"
#include "rpmsg_connector.h"
#include "ai_channel.h"

class rIOManager;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
class rModuleAI6p : public rIOBaseModule
{
friend class rIOManager;

public:

	const UDINT CHANNEL_COUNT = 6;

	rModuleAI6p(UDINT id);
	rModuleAI6p(const rModuleAI6p* ai6);
	virtual ~rModuleAI6p();

	static std::string getRTTI() { return "ai6p"; }

	// Виртуальные функции от rBaseModule
public:
	virtual std::string getModuleType() override { return rModuleAI6p::getRTTI(); }
	virtual UDINT processing(USINT issim) override;
	virtual UDINT loadFromXML(tinyxml2::XMLElement* element, rError& err) override;
	virtual UDINT generateVars(const std::string& prefix, rVariableList& list, bool issimulate) override;
	virtual UDINT generateMarkDown(rGeneratorMD& md) override;
	virtual rIOBaseChannel* getChannel(USINT channel) override;
	virtual rIOBaseModule*  getModulePtr() override { return new rModuleAI6p(this); }

//	virtual UINT getADC(USINT id) const { return (id < CHANNEL_COUNT) ? m_data.Read.Adc[id] : 0; }
//	virtual REAL getCurrent(USINT id) const { return (id < CHANNEL_COUNT) ? m_data.Read.Data[id] : 0.0f; }
//	rIOAIChannel::Type getType(USINT id);
//	USINT getState(USINT id);

protected:
	K19_AI6p_ChType getHardwareModuleChType(UDINT index);

private:
	std::vector<rIOAIChannel*> m_channel;
	K19_AI6p_str m_data;
};



