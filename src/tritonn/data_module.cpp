//=================================================================================================
//===
//=== data_module.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Классы для описания модулей ввода-вывода
//===
//=================================================================================================


#include "tinyxml2.h"
#include "data_config.h"
#include "data_module.h"
#include "xml_util.h"


rDataModule::rDataModule()
{
	m_module  = 0xFF;
	m_channel = 0xFF;
}

rDataModule::~rDataModule()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool rDataModule::isSetModule() const
{
	return m_module != 0xFF && m_channel != 0xFF;
}


UDINT rDataModule::loadFromXML(tinyxml2::XMLElement* element, rError& err)
{
	m_module  = XmlUtils::getAttributeUSINT(element, XmlName::MODULE , 0xFF);
	m_channel = XmlUtils::getAttributeUSINT(element, XmlName::CHANNEL, 0xFF);

	if (!isSetModule()) {
		return err.set(DATACFGERR_INVALID_MODULELINK, element->GetLineNum(), "");
	}

	return TRITONN_RESULT_OK;
}


