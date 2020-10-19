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

#include "def.h"
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

	enum {
		CHANNEL_COUNT = 6
	};

	rIOAI6();
	virtual ~rIOAI6();

public:
	static std::string m_name;
	
	// Виртуальные функции от rBaseModule
public:
	virtual UDINT processing(USINT issim);
	virtual rIOBaseChannel* getChannel(USINT channel);
	virtual UDINT LoadFromXML(tinyxml2::XMLElement* element, rDataConfig &cfg);

public:
	UINT getADC(USINT id);
	REAL getCurrent(USINT id);
	rIOAIChannel::Type getType(USINT id);
	USINT getState(USINT id);

private:
	rIOAIChannel m_ai[CHANNEL_COUNT];
};



