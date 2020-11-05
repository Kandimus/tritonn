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

#include "io_manager.h"
#include "locker.h"
#include "io_basechannel.h"
#include "data_config.h"
#include "units.h"
#include "simpleargs.h"
#include "def_arguments.h"
#include "variable_item.h"
#include "threadmaster.h"
#include "tinyxml2.h"
#include "xml_util.h"
#include "io_ai6.h"


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
		module->generateVars("hardware.", m_varList);
	}

	if (parent) {
		rVariableClass::linkToExternal(parent);
	}

	return TRITONN_RESULT_OK;
}


UDINT rIOManager::LoadFromXML(tinyxml2::XMLElement* element, rDataConfig &cfg)
{
	XML_FOR(module_xml, element, XmlName::MODULE) {
		rIOBaseModule* module = nullptr;
		string type = XmlUtils::getAttributeString(module_xml, XmlName::TYPE, "");

		if (type == "") {
			cfg.ErrorID   = DATACFGERR_UNKNOWN_MODULE;
			cfg.ErrorLine = module_xml->GetLineNum();
			return cfg.ErrorID;
		}

		type = String_tolower(type);
		if (type == rIOAI6::m_rtti) {
			module = new rIOAI6();
		} else {
			cfg.ErrorID   = DATACFGERR_UNKNOWN_MODULE;
			cfg.ErrorLine = module_xml->GetLineNum();
			return cfg.ErrorID;
		}

		cfg.ErrorID = module->loadFromXML(module_xml, cfg);

		if (cfg.ErrorID != TRITONN_RESULT_OK) {
			return cfg.ErrorID;
		}

		m_modules.push_back(module);
	}

	return TRITONN_RESULT_OK;
}

std::string rIOManager::saveKernel()
{
	std::string   result = "";
	rVariableList list;
	rIOAI6        ai6;

	ai6.generateVars("hardware.", list);

	result += "\n<!--\n\tHardware io modules\n-->\n<hardware>\n";

	result += ai6.saveKernel("Module 6 current/voltage channels");

	result += "</hardware>\n";

	return result;
}
