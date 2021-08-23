/*
 *
 * io/module_crm.h
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
#include "bits_array.h"
#include "basemodule.h"
#include "basechannel.h"
#include "crminterface.h"
#include "fi_channel.h"
#include "di_channel.h"

class rModuleCRM : public rIOBaseModule, public rIOCRMInterface
{
friend class rIOManager;

public:

	enum Detector : UINT
	{
		Det1 = 0x0001,
		Det2 = 0x0002,
		Det3 = 0x0004,
		Det4 = 0x0008,
	};

	const UDINT CHANNEL_DI_COUNT = 4;

	rModuleCRM(UDINT id);
	rModuleCRM(const rModuleCRM* crm);
	virtual ~rModuleCRM();

	static std::string getRTTI() { return "crm"; }
	
// rBaseModule
public:
	virtual std::string getModuleType() override { return rModuleCRM::getRTTI(); }
	virtual UDINT processing(USINT issim) override;
	virtual UDINT loadFromXML(tinyxml2::XMLElement* element, rError& err) override;
	virtual UDINT generateVars(const std::string& prefix, rVariableList& list, bool issimulate) override;
	virtual UDINT generateMarkDown(rGeneratorMD& md) override;
	virtual rIOBaseInterface* getModuleInterface() override { return dynamic_cast<rIOCRMInterface*>(this); }

// rIOFIInterface
public:
	virtual UDINT getPulling() override;
	virtual bool  isFault() const override { return m_isFault; }
	virtual UDINT getValue(USINT num, rIOBaseChannel::Type type, UDINT& fault) override;
	virtual UDINT setValue(USINT num, rIOBaseChannel::Type type, UDINT  value) override;
	virtual LREAL getFreq() override;
	virtual UINT  getDetectors() override;
	virtual State getState(USINT idx) override;
	virtual LREAL getTime(USINT idx) override;
	virtual LREAL getImp(USINT idx) override;
	virtual bool  start() override;
	virtual bool  abort() override;

private:
	rIOCRMInterface::State convertState(USINT state);

private:
	std::vector<rIODIChannel*> m_channelDI;
	rIOFIChannel* m_channelFI;

	K19_CRM_str m_data;

	static rBitsArray m_flagsSetup;
};



