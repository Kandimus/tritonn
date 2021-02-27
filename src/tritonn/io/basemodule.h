//=================================================================================================
//===
//=== io_basemodule.h
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

#include <memory>
#include "def.h"

class rIOBaseChannel;
class rDataConfig;
class rVariableList;
class rError;
class rGeneratorMD;

namespace tinyxml2 {
class XMLElement;
}

class rIOBaseModule
{
public:
	enum class Type : UINT
	{
		UNDEF  = 0,     //
		AI6    = 1,     //
		DI8DO8 = 2,
		FI4    = 3,
		CRM    = 4,
	};

	rIOBaseModule();
	virtual ~rIOBaseModule();

	std::string rIOBaseModule::getMarkDown();

	virtual std::string getModuleType() = 0;
	virtual UDINT processing(USINT issim);
	virtual UDINT loadFromXML(tinyxml2::XMLElement* element, rError& err);
	virtual UDINT generateVars(const std::string& prefix, rVariableList& list, bool issimulate);
	virtual UDINT generateMarkDown(rGeneratorMD& md) const;
	virtual std::unique_ptr<rIOBaseChannel> getChannel(USINT channel) = 0;
	virtual std::unique_ptr<rIOBaseModule>  getModulePtr() = 0;
	virtual std::string getAlias() const;
	virtual std::string getName() const;
	virtual STRID       getDescr() const;

/*
	Type  getType()         { return m_type; }
	UINT  getNodeID()       { return m_nodeID; }
	UDINT getVendorID()     { return m_vendorID; }
	UDINT getProductCode()  { return m_productCode; }
	UDINT getRevision()     { return m_revision; }
	UDINT getSerialNumber() { return m_serialNumber; }

	REAL  getTemperature()  { return m_temperature; }
	UINT  getCAN()          { return m_CAN; }
	UINT  getFirmware()     { return m_firmware; }
	UINT  getHardware()     { return m_hardware; }
*/
public:
	Type  m_type;
	UINT  m_nodeID;
	UDINT m_vendorID;
	UDINT m_productCode;
	UDINT m_revision;
	UDINT m_serialNumber;

	REAL  m_temperature;
	UINT  m_CAN;
	UINT  m_firmware;
	UINT  m_hardware;

protected:
	pthread_mutex_t m_mutex;
	std::string     m_name  = "";
	std::string     m_alias = "";
	STRID           m_descr = 0;
};



