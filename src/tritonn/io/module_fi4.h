//=================================================================================================
//===
//=== module_fi4.h
//===
//=== Copyright (c) 2020 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс частотного модуля CAN (FI)
//===
//=================================================================================================

#pragma once

#include <vector>
#include "def.h"
#include "bits_array.h"
#include "basemodule.h"
#include "basechannel.h"
#include "fi_channel.h"

class rIOManager;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
class rModuleFI4 : public rIOBaseModule
{
friend class rIOManager;

public:

	const UDINT CHANNEL_COUNT = 4;

	rModuleFI4();
	virtual ~rModuleFI4();

	static std::string getRTTI() { return "fi4"; }
	
	// Виртуальные функции от rBaseModule
public:
	virtual std::string getModuleType() { return rModuleFI4::getRTTI(); }
	virtual UDINT processing(USINT issim);
	virtual std::unique_ptr<rIOBaseChannel> getChannel(USINT channel);
	virtual UDINT loadFromXML(tinyxml2::XMLElement* element, rError& err);
	virtual UDINT generateVars(const std::string& prefix, rVariableList& list, bool issimulate);

public:
	UDINT getCounter(USINT id);
	USINT getState(USINT id);

private:
	static rBitsArray m_flagsSetup;
	std::vector<rIOFIChannel> m_channel;
};



