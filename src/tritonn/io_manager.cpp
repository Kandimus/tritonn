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

#include "io_basechannel.h"
#include "data_config.h"
#include "io_manager.h"
#include "tinyxml2.h"
#include "xml_util.h"
#include "io_ai6.h"


rIOManager::rIOManager()
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


rIOBaseChannel* rIOManager::getChannel(USINT module, USINT channel)
{
	if (module >= m_modules.size()) {
		return nullptr;
	}

	return m_modules[module]->getChannel(channel);
}


rThreadStatus rIOManager::Proccesing()
{
	rThreadStatus thread_status = rThreadStatus::UNDEF;
	UDINT ii = 0;

	while (true) {
		// Обработка команд нити
		thread_status = rThreadClass::Proccesing();
		if (!THREAD_IS_WORK(thread_status)) {
			return thread_status;
		}

//		if (m_isSimulate) {

//		}
	}
}


UDINT rIOManager::LoadFromXML(tinyxml2::XMLElement* element, rDataConfig &cfg)
{
	for (tinyxml2::XMLElement *module_xml = element->FirstChildElement(XmlName::MODULE); module_xml != nullptr; module_xml = module_xml->NextSiblingElement(XmlName::MODULE)) {
		string type = XmlUtils::getAttributeString(module_xml, XmlName::TYPE, "");

		if (type == "") {
			cfg.ErrorID   = DATACFGERR_UNKNOWN_MODULE;
			cfg.ErrorLine = module_xml->GetLineNum();
			return cfg.ErrorID;
		}

		type = String_tolower(type);
		if (type == rIOAI6::m_name) {
			rIOAI6 *ioai6 = new rIOAI6();

			cfg.ErrorID = ioai6->LoadFromXML(module_xml, cfg);

			if (cfg.ErrorID != TRITONN_RESULT_OK) {
				return cfg.ErrorID;
			}
		} else {
			cfg.ErrorID   = DATACFGERR_UNKNOWN_MODULE;
			cfg.ErrorLine = module_xml->GetLineNum();
			return cfg.ErrorID;
		}


	}

	return TRITONN_RESULT_OK;
}
