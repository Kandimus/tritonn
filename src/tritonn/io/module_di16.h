/*
 *
 * module_di16.h
 *
 * Copyright (c) 2021 by RangeSoft.
 * All rights reserved.
 *
 * Litvinov "VeduN" Vitaliy O.
 *
 */

#pragma once

#include <vector>
#include "def.h"
#include "basemodule.h"
#include "basechannel.h"
#include "di_channel.h"

class rIOManager;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
class rModuleDI16 : public rIOBaseModule
{
friend class rIOManager;

public:

	const UDINT CHANNEL_COUNT = 16;

	rModuleDI16(UDINT id);
	rModuleDI16(const rModuleDI16* di16);
	virtual ~rModuleDI16();

	static std::string getRTTI() { return "di16"; }
	
	// Виртуальные функции от rBaseModule
public:
	virtual std::string getModuleType() override { return rModuleDI16::getRTTI(); }
	virtual UDINT processing(USINT issim) override;
	virtual UDINT loadFromXML(tinyxml2::XMLElement* element, rError& err) override;
	virtual UDINT generateVars(const std::string& prefix, rVariableList& list, bool issimulate) override;
	virtual UDINT generateMarkDown(rGeneratorMD& md) override;
	virtual rIOBaseChannel* getChannel(USINT channel) override;
	virtual rIOBaseModule*  getModulePtr() override { return new rModuleDI16(this); }

public:
	USINT getValue(USINT id);
	USINT getState(USINT id);

private:
	std::vector<rIODIChannel*> m_channel;
};



