//=================================================================================================
//===
//=== module_ai6.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс аналового входног сигнала модуля CAN (AI)
//===
//=================================================================================================

#pragma once

#include <vector>
#include "def.h"
#include "basemodule.h"
#include "basechannel.h"
#include "ai_channel.h"

class rIOManager;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
class rModuleAI6 : public rIOBaseModule
{
friend class rIOManager;

public:

	const UDINT CHANNEL_COUNT = 6;

	rModuleAI6(UDINT id);
	rModuleAI6(const rModuleAI6* ai6);
	virtual ~rModuleAI6();

	static std::string getRTTI() { return "ai6"; }
	
	// Виртуальные функции от rBaseModule
public:
	virtual std::string getModuleType() override { return rModuleAI6::getRTTI(); }
	virtual UDINT processing(USINT issim) override;
	virtual UDINT loadFromXML(tinyxml2::XMLElement* element, rError& err) override;
	virtual UDINT generateVars(const std::string& prefix, rVariableList& list, bool issimulate) override;
	virtual UDINT generateMarkDown(rGeneratorMD& md) override;
	virtual rIOBaseChannel* getChannel(USINT channel) override;
	virtual rIOBaseModule*  getModulePtr() override { return new rModuleAI6(this); }

public:
	UINT getADC(USINT id);
	REAL getCurrent(USINT id);
	rIOAIChannel::Type getType(USINT id);
	USINT getState(USINT id);

private:
	std::vector<rIOAIChannel*> m_channel;
};



