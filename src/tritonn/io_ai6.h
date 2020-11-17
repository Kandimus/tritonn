//=================================================================================================
//===
//=== io_ai.h
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
#include "bits_array.h"
#include "io_basemodule.h"
#include "io_basechannel.h"
#include "io_ai_channel.h"

class rIOManager;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
class rIOAI6 : public rIOBaseModule
{
friend class rIOManager;

public:

	const UDINT CHANNEL_COUNT = 6;

	rIOAI6();
	virtual ~rIOAI6();

public:
	static std::string m_rtti;
	
	// Виртуальные функции от rBaseModule
public:
	virtual std::string getModuleType() { return "ai6"; }
	virtual UDINT processing(USINT issim);
	virtual std::unique_ptr<rIOBaseChannel> getChannel(USINT channel);
	virtual UDINT loadFromXML(tinyxml2::XMLElement* element, rDataConfig &cfg);
	virtual UDINT generateVars(const std::string& prefix, rVariableList& list);

public:
	UINT getADC(USINT id);
	REAL getCurrent(USINT id);
	rIOAIChannel::Type getType(USINT id);
	USINT getState(USINT id);

private:
	static rBitsArray m_flagsSetup;
	std::vector<rIOAIChannel> m_channel;
};



