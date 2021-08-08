/*
 *
 * module_ao4.h
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
#include "rpmsg_connector.h"
#include "ao_channel.h"

class rIOManager;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
class rModuleAO4 : public rIOBaseModule
{
friend class rIOManager;

public:
	const UDINT CHANNEL_COUNT = 4;

	rModuleAO4(UDINT id);
	rModuleAO4(const rModuleAO4* ai6);
	virtual ~rModuleAO4();

	static std::string getRTTI() { return "ao4"; }
	
	// Виртуальные функции от rBaseModule
public:
	virtual std::string getModuleType() override { return rModuleAO4::getRTTI(); }
	virtual UDINT processing(USINT issim) override;
	virtual UDINT loadFromXML(tinyxml2::XMLElement* element, rError& err) override;
	virtual UDINT generateVars(const std::string& prefix, rVariableList& list, bool issimulate) override;
	virtual UDINT generateMarkDown(rGeneratorMD& md) override;
	virtual rIOBaseChannel* getChannel(USINT channel, rIOBaseChannel::Type type) override;
	virtual rIOBaseModule*  getModulePtr() override { return new rModuleAO4(this); }

protected:

public:
	void setCurrent(USINT id, UINT current);
	rIOAOChannel::Type getType(USINT id);

private:
	std::vector<rIOAOChannel*> m_channel;
	K19_AO4_str m_data;
};



