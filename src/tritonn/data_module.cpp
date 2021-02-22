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
#include "io/manager.h"


rDataModule::rDataModule()
{
	m_nochannel = false;
}

rDataModule::rDataModule(bool nochannel)
{
	m_nochannel = nochannel;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool rDataModule::isSetModule() const
{
	return m_module != FAULT && (m_channel != FAULT || m_nochannel);
}


std::string rDataModule::getAlias() const
{
	if (m_module == FAULT) {
		return "";
	}

	return rIOManager::instance().getModuleAlias(m_module);
}

UDINT rDataModule::loadFromXML(tinyxml2::XMLElement* element, rError& err)
{
	m_module  = XmlUtils::getAttributeUSINT(element, XmlName::MODULE , FAULT);
	m_channel = XmlUtils::getAttributeUSINT(element, XmlName::CHANNEL, FAULT);

	if (!isSetModule()) {
		return err.set(DATACFGERR_INVALID_MODULELINK, element->GetLineNum(), "");
	}

	return TRITONN_RESULT_OK;
}


