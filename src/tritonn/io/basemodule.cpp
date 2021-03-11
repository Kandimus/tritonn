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

#include "basemodule.h"
#include <algorithm>
#include "basechannel.h"
#include "bits_array.h"
#include "../xml_util.h"
#include "../data_config.h"
#include "../variable_item.h"
#include "../variable_list.h"
#include "../units.h"
#include "../error.h"

rBitsArray rIOBaseModule::m_flagsType;

rIOBaseModule::rIOBaseModule()
{
	if (m_flagsType.empty()) {
		m_flagsType
				.add("", static_cast<UINT>(Type::UNDEF) , "Модуль не определен")
				.add("", static_cast<UINT>(Type::AI6)   , "Модуль AI6")
				.add("", static_cast<UINT>(Type::DI8DO8), "Модуль DI8DO8")
				.add("", static_cast<UINT>(Type::FI4)   , "Модуль FI4")
				.add("", static_cast<UINT>(Type::CRM)   , "Модуль CRM");
	}

	pthread_mutex_init(&m_mutex, nullptr);
}

rIOBaseModule::rIOBaseModule(const rIOBaseModule* module)
{
	pthread_mutex_init(&m_mutex, nullptr);

	m_type         = module->m_type;
	m_nodeID       = module->m_nodeID;
	m_vendorID     = module->m_vendorID;
	m_productCode  = module->m_productCode;
	m_revision     = module->m_revision;
	m_serialNumber = module->m_serialNumber;

	m_temperature  = module->m_temperature;
	m_CAN          = module->m_CAN;
	m_firmware     = module->m_firmware;
	m_hardware     = module->m_hardware;

	m_name  = module->m_name;
	m_alias = module->m_alias;
	m_descr = module->m_descr;
	m_comment = module->m_comment;

	m_listChannel.clear();
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

std::string rIOBaseModule::getAlias() const
{
	return m_alias;
}

std::string rIOBaseModule::getName() const
{
	return m_name;
}

STRID rIOBaseModule::getDescr() const
{
	return m_descr;
}

UDINT rIOBaseModule::generateVars(const std::string& prefix, rVariableList& list, bool issimulate)
{
	UNUSED(issimulate);

	m_alias = prefix + m_name;

	std::string p = m_alias + ".";

	list.add(p + "type"        , TYPE_UINT , rVariable::Flags::R__, &m_type        , U_DIMLESS , 0, "Нет данных");
	list.add(p + "node"        , TYPE_UINT , rVariable::Flags::R__, &m_nodeID      , U_DIMLESS , 0, "Нет данных");
	list.add(p + "vendor"      , TYPE_UDINT, rVariable::Flags::R__, &m_vendorID    , U_DIMLESS , 0, "Нет данных");
	list.add(p + "productCode" , TYPE_UDINT, rVariable::Flags::R__, &m_productCode , U_DIMLESS , 0, "Нет данных");
	list.add(p + "revision"    , TYPE_UDINT, rVariable::Flags::R__, &m_revision    , U_DIMLESS , 0, "Нет данных");
	list.add(p + "serialNumber", TYPE_UDINT, rVariable::Flags::R__, &m_serialNumber, U_DIMLESS , 0, "Нет данных");
	list.add(p + "temperature" , TYPE_REAL , rVariable::Flags::R__, &m_temperature , U_DIMLESS , 0, "Нет данных");
	list.add(p + "can"         , TYPE_UINT , rVariable::Flags::R__, &m_CAN         , U_DIMLESS , 0, "Нет данных");
	list.add(p + "firmware"    , TYPE_UINT , rVariable::Flags::R__, &m_firmware    , U_DIMLESS , 0, "Нет данных");
	list.add(p + "hardware"    , TYPE_UINT , rVariable::Flags::R__, &m_hardware    , U_DIMLESS , 0, "Нет данных");

	return TRITONN_RESULT_OK;
}

UDINT rIOBaseModule::loadFromXML(tinyxml2::XMLElement* element, rError& err)
{
	m_name  = XmlUtils::getAttributeString(element, XmlName::NAME, "");
	m_descr = XmlUtils::getAttributeUDINT (element, XmlName::DESC, 0);

	if (m_name.empty()) {
		return err.set(DATACFGERR_INVALID_NAME, element->GetLineNum());
	}

	return TRITONN_RESULT_OK;
}

UDINT rIOBaseModule::generateMarkDown(rGeneratorMD& md)
{
	UNUSED(md)
	return TRITONN_RESULT_OK;
}

std::string rIOBaseModule::getXmlChannels()
{
	std::string result = "";

	for (auto channel : m_listChannel) {
		result += "\t<channel number=\"" + String_format("%u", channel->m_index);
		result += " setup=\"" + channel->getStrType() + " setup flags\" />\n";
	}

	return result;
}

std::string rIOBaseModule::getMarkDown()
{
	std::string result = "";

	result += "\n## Channels\n";
	result += "Number | Type | Comment\n";
	result += ":-- |:--:|:--\n";

	for (auto channel : m_listChannel) {
		result += String_format("%u", channel->m_index) + " | ";
		result += channel->getStrType() + " | ";
		result += channel->m_comment + "\n";
	}

	result += "\n";
	std::vector<std::string> ch_names;
	for (auto channel : m_listChannel) {

		if (std::find(ch_names.begin(), ch_names.end(), channel->getStrType()) == ch_names.end()) {
			result += channel->getFlagsSetup().getMarkDown(channel->getStrType() + " setup");
			ch_names.push_back(channel->getStrType());
		}

	}

	rVariableList list;
	generateVars("", list, true);

	result += list.getMarkDown();

	return result;
}

