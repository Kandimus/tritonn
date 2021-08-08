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
#include <vector>
#include "basechannel.h"
#include "rpmsg_connector.h"
#include "bits_array.h"

class rIOBaseInterface;
class rDataConfig;
class rVariableList;
class rError;
class rGeneratorMD;

namespace tinyxml2 {
class XMLElement;
}

struct rModuleStatus
{
	UINT m_CAN      = 0;
	UINT m_firmware = 0;
	UINT m_hardware = 0;
};

class rIOBaseModule
{
public:
	//NOTE при добавлении типа модуля, добавить описание в таблицу SID
	enum class Type : UINT
	{
		UNDEF  = 0,     //
		CPU    = 1,
		AI6a   = 2,     //
		AI6p   = 3,     //
		FI4    = 4,
		DI8DO8 = 5,
		DI16   = 6,
		DO16   = 7,
		CRM    = 8,
		AO4    = 9,
	};

	rIOBaseModule(UDINT id);
	rIOBaseModule(const rIOBaseModule* module);
	virtual ~rIOBaseModule();

	std::string getMarkDown();
	std::string getXmlChannels();

	virtual std::string getModuleType() = 0;
	virtual UDINT processing(USINT issim);
	virtual UDINT loadFromXML(tinyxml2::XMLElement* element, rError& err);
	virtual UDINT generateVars(const std::string& prefix, rVariableList& list, bool issimulate);
	virtual UDINT generateMarkDown(rGeneratorMD& md);
	virtual rIOBaseChannel* getChannel(USINT channel, rIOBaseChannel::Type type) = 0; //TODO delete
	virtual rIOBaseModule*  getModulePtr() = 0; // TODO delete
	virtual rIOBaseInterface* getModuleInterface() { return nullptr; } //TODO = 0;

	virtual std::string getAlias() const { return m_alias; }
	virtual std::string getName()  const { return m_name;  }
	virtual STRID       getDescr() const { return m_descr; }
	virtual UDINT       getID()    const { return m_ID;    }
	virtual Type        getType()  const { return m_type;  }

	virtual UINT  getModuleNodeID()     const { return m_module.NodeID; }
	virtual UDINT getModuleIDVendor()   const { return m_module.IDVendor; }
	virtual UDINT getModuleIDProdCode() const { return m_module.IDProdCode; }
	virtual UDINT getModuleIDRevision() const { return m_module.IDRevision; }
	virtual UDINT getModuleIDSerial()   const { return m_module.IDSerial; }
	virtual UDINT getCAN()              const { return m_status.m_CAN; }
	virtual UDINT getFirmware()         const { return m_status.m_firmware; }
	virtual UDINT getHardware()         const { return m_status.m_hardware; }

protected:
	void setModule(void* data, ModuleInfo_str* info, ModuleSysData_str* sysdata, void* status, UDINT readAll, UDINT exchange);
	void printModuleInfo();

public:
	static rBitsArray m_flagsType;
	static rBitsArray m_flagsShortType;

protected:
	Type m_type            = Type::UNDEF;
	USINT m_ID             = 0xFF;
	void* m_dataPtr        = nullptr;
	UDINT m_moduleReadAll  = 0;
	UDINT m_moduleExchange = 0;

	ModuleSysData_str  m_module;
	rModuleStatus      m_status;
	rModuleStatus*     m_moduleStatus = nullptr;
	ModuleInfo_str*    m_moduleInfo    = nullptr;
	ModuleSysData_str* m_moduleSysData = nullptr;

	std::string     m_name    = "";
	std::string     m_alias   = "";
	STRID           m_descr   = 0;
	std::string     m_comment = "";
	bool            m_isFault = false;

	std::vector<rIOBaseChannel*> m_listChannel;

	pthread_rwlock_t m_rwlock;
};



