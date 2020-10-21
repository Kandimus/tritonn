//=================================================================================================
//===
//=== io_basemodule.cpp
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

#include "xml_util.h"
#include "data_config.h"
#include "io_basemodule.h"

std::string rIOBaseModule::m_rtti = "BaseModule";

rIOBaseModule::rIOBaseModule()
{
	pthread_mutex_init(&m_mutex, nullptr);
}

rIOBaseModule::~rIOBaseModule()
{
	pthread_mutex_destroy(&m_mutex);
}


UDINT rIOBaseModule::processing(USINT issim)
{
	UNUSED(issim);
	return TRITONN_RESULT_OK;
}


UDINT rIOBaseModule::loadFromXML(tinyxml2::XMLElement* element, rDataConfig &cfg)
{
	m_name = XmlUtils::getAttributeString(element, XmlName::NAME, "");

	if (m_name.empty()) {
		cfg.ErrorLine = element->GetLineNum();
		cfg.ErrorID   = DATACFGERR_INVALID_NAME;
		return cfg.ErrorID;
	}

	return TRITONN_RESULT_OK;
}
