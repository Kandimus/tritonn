/*
 *
 * module_fi4.h
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
#include "fi_channel.h"

class rIOManager;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
class rModuleFI4 : public rIOBaseModule
{
friend class rIOManager;

public:
	enum class OutType : USINT
	{
		NONE = 0,
		CHANNEL_1,
		CHANNEL_2,
		CHANNEL_3,
		CHANNEL_4,
	};


	const UDINT CHANNEL_COUNT = 4;

	rModuleFI4(UDINT id);
	rModuleFI4(const rModuleFI4* fi4);
	virtual ~rModuleFI4();

	static std::string getRTTI() { return "fi4"; }
	
	// Виртуальные функции от rBaseModule
public:
	virtual std::string getModuleType() override { return rModuleFI4::getRTTI(); }
	virtual UDINT processing(USINT issim) override;
	virtual UDINT loadFromXML(tinyxml2::XMLElement* element, rError& err) override;
	virtual UDINT generateVars(const std::string& prefix, rVariableList& list, bool issimulate) override;
	virtual UDINT generateMarkDown(rGeneratorMD& md) override;
	virtual rIOBaseChannel* getChannel(USINT channel, rIOBaseChannel::Type type) override;
	virtual rIOBaseModule*  getModulePtr() override { return new rModuleFI4(this); }

public:
	UDINT getCounter(USINT id);
	USINT getState(USINT id);

protected:
	K19_FIO_OutType getOutType();

private:
	std::vector<rIOFIChannel*> m_channel;

	OutType m_outtype = OutType::NONE;

	K19_FIO_str m_data;

	static rBitsArray m_flagsOutType;
};



