//=================================================================================================
//===
//=== module_crm.h
//===
//=== Copyright (c) 2021 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс модуля поверочной установки (CRM)
//===
//=================================================================================================

#pragma once

#include <vector>
#include "def.h"
#include "bits_array.h"
#include "basemodule.h"
#include "basechannel.h"
#include "fi_channel.h"
#include "di_channel.h"

class rIOManager;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
class rModuleCRM : public rIOBaseModule
{
friend class rIOManager;

public:

	const UDINT CHANNEL_DI_COUNT = 4;

	rModuleCRM();
	virtual ~rModuleCRM() = default;

	static std::string getRTTI() { return "crm"; }
	
	// Виртуальные функции от rBaseModule
public:
	virtual std::string getModuleType() { return rModuleCRM::getRTTI(); }
	virtual UDINT processing(USINT issim);
	virtual std::unique_ptr<rIOBaseChannel> getChannel(USINT channel);
	virtual UDINT loadFromXML(tinyxml2::XMLElement* element, rError& err);
	virtual UDINT generateVars(const std::string& prefix, rVariableList& list, bool issimulate);
	virtual std::unique_ptr<rIOBaseModule> getModulePtr() { return std::make_unique<rModuleCRM>(*this); }

public:
	UDINT start();
	USINT abort();
	LREAL getFreq() const;
	UINT  getDetectors() const;

private:
	std::vector<rIODIChannel> m_channelDI;
	rIOFIChannel m_channelFI;

	static rBitsArray m_flagsSetup;
};



