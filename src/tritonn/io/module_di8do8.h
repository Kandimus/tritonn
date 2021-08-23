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
#include "baseinterface.h"
#include "di_channel.h"
#include "do_channel.h"

class rIOManager;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
class rModuleDI8DO8 : public rIOBaseModule, public rIOBaseInterface
{
friend class rIOManager;

public:

	const UDINT CHANNEL_DI_COUNT = 8;
	const UDINT CHANNEL_DO_COUNT = 8;

	rModuleDI8DO8(UDINT id);
	rModuleDI8DO8(const rModuleDI8DO8* di8do8);
	virtual ~rModuleDI8DO8();

	static std::string getRTTI() { return "di8do8"; }
	
// rBaseModule
public:
	virtual std::string getModuleType() override { return rModuleDI8DO8::getRTTI(); }
	virtual UDINT processing(USINT issim) override;
	virtual UDINT loadFromXML(tinyxml2::XMLElement* element, rError& err) override;
	virtual UDINT generateVars(const std::string& prefix, rVariableList& list, bool issimulate) override;
	virtual UDINT generateMarkDown(rGeneratorMD& md) override;
	virtual rIOBaseInterface* getModuleInterface() override { return dynamic_cast<rIOBaseInterface*>(this); }

// IOBaseInterface
public:
	virtual UDINT getPulling() override;
	virtual bool  isFault() const override { return m_isFault; }
	virtual UDINT getValue(USINT num, rIOBaseChannel::Type type, UDINT& fault) override;
	virtual UDINT setValue(USINT num, rIOBaseChannel::Type type, UDINT  value) override;

private:
	static rBitsArray m_flagsDOSetup;
	std::vector<rIODIChannel*> m_channelDI;
	std::vector<rIODOChannel*> m_channelDO;

	K19_DIDO8_str m_data;
};



