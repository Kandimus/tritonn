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
#include "basemodule.h"
#include "basechannel.h"
#include "di_channel.h"
#include "do_channel.h"

class rIOManager;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
class rModuleDI8DO8 : public rIOBaseModule
{
friend class rIOManager;

public:

	const UDINT CHANNEL_DI_COUNT = 8;
	const UDINT CHANNEL_DO_COUNT = 8;

	rModuleDI8DO8();
	virtual ~rModuleDI8DO8() = default;

	static std::string getRTTI() { return "di8do8"; }
	
	// Виртуальные функции от rBaseModule
public:
	virtual std::string getModuleType() { return rModuleDI8DO8::getRTTI(); }
	virtual UDINT processing(USINT issim);
	virtual std::unique_ptr<rIOBaseChannel> getChannel(USINT channel);
	virtual UDINT loadFromXML(tinyxml2::XMLElement* element, rError& err);
	virtual UDINT generateVars(const std::string& prefix, rVariableList& list, bool issimulate);

public:
	USINT getValue(USINT id);
	USINT getState(USINT id);

private:
	static rBitsArray m_flagsDOSetup;
	std::vector<rIODIChannel> m_channelDI;
	std::vector<rIODOChannel> m_channelDO;
};



