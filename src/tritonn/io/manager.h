//=================================================================================================
//===
//=== io_manager.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Основной класс-нить для получения данных с модулей ввода-вывода
//===
//=================================================================================================

#pragma once

#include <vector>
#include <map>
#include <memory>
#include "safity.h"
#include "tinyxml2.h"
#include "../variable_class.h"
#include "thread_class.h"
#include "basemodule.h"
#include "basechannel.h"

class rDataConfig;
class rIOBaseChannel;
class rError;
class rGeneratorMD;


//-------------------------------------------------------------------------------------------------
//
class rIOManager : public rThreadClass, public rVariableClass
{
	friend rDataConfig;

	SINGLETON(rIOManager)

public:

// Методы
public:
	UDINT LoadFromXML(tinyxml2::XMLElement* element, rError& err);
	UDINT generateVars(rVariableClass* parent);

	rIOBaseChannel* getChannel(USINT module, rIOBaseChannel::Type type, USINT channel); //TODO delete
	rIOBaseModule*  getModule(USINT module, rIOBaseModule::Type type); //TODO delete
	rIOBaseInterface* getModuleInterface(USINT module, rIOBaseModule::Type type);
	std::string getModuleAlias(USINT module) const;

	bool checkListOfModules() const;

protected:
	virtual rThreadStatus Proccesing();

	rIOBaseModule* addModule(const std::string& type, rError& err, UDINT lineno, std::map<std::string, UDINT>& maxmap);

private:
	std::vector<rIOBaseModule*> m_modules; //
	USINT m_moduleCount;
};



