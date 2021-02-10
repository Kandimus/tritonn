//=================================================================================================
//===
//=== io/manager.h
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

#include "manager.h"
#include "locker.h"
#include "tinyxml2.h"
#include "basechannel.h"
#include "../data_config.h"
#include "../units.h"
#include "simpleargs.h"
#include "../def_arguments.h"
#include "../variable_item.h"
#include "../threadmaster.h"
#include "../xml_util.h"
#include "../error.h"
#include "module_ai6.h"
#include "module_di8do8.h"
#include "module_fi4.h"
#include "module_crm.h"


rIOManager::rIOManager() : rVariableClass(Mutex)
{
	RTTI = "rIOManager";
}


rIOManager::~rIOManager()
{
	for(auto item : m_modules) {
		if (item) {
			delete item;
		}
	}
	m_modules.clear();
}


std::unique_ptr<rIOBaseChannel> rIOManager::getChannel(USINT module, USINT channel)
{
	if (module >= m_modules.size()) {
		return nullptr;
	}

	return m_modules[module]->getChannel(channel);
}

rThreadStatus rIOManager::Proccesing()
{
	rThreadStatus thread_status = rThreadStatus::UNDEF;

	while (true) {
		// Обработка команд нити
		thread_status = rThreadClass::Proccesing();
		if (!THREAD_IS_WORK(thread_status)) {
			return thread_status;
		}

		{
			rLocker lock(Mutex); UNUSED(lock);

			for(auto& item : m_modules) {
				item->processing(rSimpleArgs::instance().isSet(rArg::Simulate));
			}

			rVariableClass::processing();
			rThreadClass::EndProccesing();
		}
	}
}


UDINT rIOManager::generateVars(rVariableClass* parent)
{
	for (auto module : m_modules) {
		module->generateVars("hardware.", m_varList, rSimpleArgs::instance().isSet(rArg::Simulate));
	}

	if (parent) {
		rVariableClass::linkToExternal(parent);
	}

	return TRITONN_RESULT_OK;
}


UDINT rIOManager::LoadFromXML(tinyxml2::XMLElement* element, rError& err)
{
	XML_FOR(module_xml, element, XmlName::MODULE) {
		rIOBaseModule* module = nullptr;
		std::string    type   = XmlUtils::getAttributeString(module_xml, XmlName::TYPE, "");

		if (type == "") {
			return err.set(DATACFGERR_UNKNOWN_MODULE, module_xml->GetLineNum());
		}

		type = String_tolower(type);
		if (type == rModuleAI6::getRTTI()) {
			module = dynamic_cast<rIOBaseModule*>(new rModuleAI6());

		} else if (type == rModuleDI8DO8::getRTTI()) {
			module = dynamic_cast<rIOBaseModule*>(new rModuleDI8DO8());

		} else if (type == rModuleFI4::getRTTI()) {
			module = dynamic_cast<rIOBaseModule*>(new rModuleFI4());

		} else if (type == rModuleCRM::getRTTI()) {
			module = dynamic_cast<rIOBaseModule*>(new rModuleCRM());

		} else {
			return err.set(DATACFGERR_UNKNOWN_MODULE, module_xml->GetLineNum());
		}

		if (module->loadFromXML(module_xml, err) != TRITONN_RESULT_OK) {
			return err.getError();
		}

		m_modules.push_back(module);
	}

	return TRITONN_RESULT_OK;
}

std::string rIOManager::saveKernel()
{
	std::string   result = "";
	rVariableList list;
	rModuleAI6    ai6;
	rModuleDI8DO8 di8do8;
	rModuleFI4    fi4;
	rModuleCRM    crm;

	ai6.generateVars("hardware.", list, true);
	di8do8.generateVars("hardware.", list, true);
	fi4.generateVars("hardware.", list, true);
	crm.generateVars("hardware.", list, true);

	result += "\n<!--\n\tHardware io modules\n-->\n<hardware>\n";

	result += ai6.saveKernel("Module 6 current/voltage channels");
	result += di8do8.saveKernel("Module 8 discrete input and 8 discrete output");
	result += fi4.saveKernel("Module 4 frequency input");
	result += crm.saveKernel("Module prove");

	result += "</hardware>\n";

	return result;
}
