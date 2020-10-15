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
	rIOAI6();
	virtual ~rIOAI6();

	const USINT m_channelMax = 6;
	
	// Виртуальные функции от rBaseModule
public:
	virtual UDINT processing(USINT issim);

public:
	UINT getADC(USINT id);
	REAL getCurrent(USINT id);
	rIOAIChannel::Type getType(USINT id);
	USINT getState(USINT id);

private:
	rIOAIChannel m_ai[m_channelMax];
};



