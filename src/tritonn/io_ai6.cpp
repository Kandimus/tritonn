//=================================================================================================
//===
//=== io_ai.cpp
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

#include "io_ai6.h"
#include "data_config.h"
#include "tinyxml2.h"

std::string rIOAI6::m_name = "ai6";

rIOAI6::rIOAI6()
{

}


rIOAI6::~rIOAI6()
{

}


UDINT rIOAI6::processing(USINT issim)
{
	if (issim) {
		for(auto ii = 0; ii < CHANNEL_COUNT; ++ii) {
			m_ai[ii].simulate();
		}

		return TRITONN_RESULT_OK;
	}

	return TRITONN_RESULT_OK;
}


rIOBaseChannel* rIOAI6::getChannel(USINT channel)
{
	if (channel < CHANNEL_COUNT) {
		return nullptr;
	}

	return &m_ai[channel];
}


UDINT rIOAI6::LoadFromXML(tinyxml2::XMLElement* element, rDataConfig &cfg)
{
/*	for (tinyxml2::XMLElement *channel_xml = element->FirstChildElement(CFGNAME_CHANNEL); channel_xml != nullptr; channel_xml = channel_xml->NextSiblingElement(CFGNAME_MODULE)) {
		string type = rDataConfig::GetAttributeString(module_xml, CFGNAME_TYPE, "");

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
*/
	return TRITONN_RESULT_OK;
}
