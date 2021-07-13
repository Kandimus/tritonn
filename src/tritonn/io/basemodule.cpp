/*
 *
 * io_basemodule.cpp
 *
 * Copyright (c) 2019 by RangeSoft.
 * All rights reserved.
 *
 * Litvinov "VeduN" Vitaliy O.
 *
 */

#include "basemodule.h"
#include <algorithm>
#include "locker.h"
#include "basechannel.h"
#include "bits_array.h"
#include "xml_util.h"
#include "../data_config.h"
#include "../variable_item.h"
#include "../variable_list.h"
#include "../units.h"
#include "../error.h"

rBitsArray rIOBaseModule::m_flagsType;

rIOBaseModule::rIOBaseModule(UDINT id)
{
	if (m_flagsType.empty()) {
		m_flagsType
				.add("", static_cast<UINT>(Type::UNDEF) , "Модуль не определен")
				.add("", static_cast<UINT>(Type::CPU)   , "Базовый модуль ЦПУ")
				.add("", static_cast<UINT>(Type::AI6a)  , "Модуль AI6a")
				.add("", static_cast<UINT>(Type::AI6p)  , "Модуль AI6p")
				.add("", static_cast<UINT>(Type::FI4)   , "Модуль FI4")
				.add("", static_cast<UINT>(Type::DI8DO8), "Модуль DI8DO8")
				.add("", static_cast<UINT>(Type::DI16)  , "Модуль DI16")
				.add("", static_cast<UINT>(Type::DO16)  , "Модуль DO16")
				.add("", static_cast<UINT>(Type::CRM)   , "Модуль CRM");
	}

	m_ID = id;

	pthread_rwlock_init(&m_rwlock, nullptr);
}

rIOBaseModule::rIOBaseModule(const rIOBaseModule* module)
{
	pthread_rwlock_init(&m_rwlock, nullptr);

	m_ID      = module->m_ID;
	m_module  = module->m_module;
	m_type    = module->m_type;

	m_name    = module->m_name;
	m_alias   = module->m_alias;
	m_descr   = module->m_descr;
	m_comment = module->m_comment;

	m_listChannel.clear();
}

rIOBaseModule::~rIOBaseModule()
{
	pthread_rwlock_destroy(&m_rwlock);
}

void rIOBaseModule::setModule(void* data, ModuleInfo_str* info, ModuleSysData_str* sysdata, UDINT readAll, UDINT exchange)
{
	m_dataPtr        = data;
	m_moduleInfo     = info;
	m_moduleSysData  = sysdata;
	m_moduleReadAll  = readAll;
	m_moduleExchange = exchange;
}

UDINT rIOBaseModule::processing(USINT issim)
{
	if(issim) {
		return TRITONN_RESULT_OK;
	}

	if (!m_moduleInfo || !m_dataPtr)
	{
		return DATACFGERR_HARDWARE_MODULEISNULL;
	}

#ifdef TRITONN_YOCTO
	if (!m_moduleInfo->InWork) {
		candrv_cmd(m_moduleReadAll, m_ID, m_dataPtr);
	}
	if (m_moduleInfo->InWork) {
		candrv_cmd(m_moduleExchange, m_ID, m_dataPtr);
	} else {
		return DATACFGERR_HARDWARE_MODULEFAULT;
	}
	#endif

	// copy system data
	m_module = *m_moduleSysData;

	return TRITONN_RESULT_OK;
}

UDINT rIOBaseModule::generateVars(const std::string& prefix, rVariableList& list, bool issimulate)
{
	UNUSED(issimulate);

	m_alias = prefix + m_name;

	std::string p = m_alias + ".";

	list.add(p + "type"        , TYPE::UINT, rVariable::Flags::R___, &m_type             , U_DIMLESS , 0, "Нет данных");
	list.add(p + "node"        ,             rVariable::Flags::R___, &m_module.NodeID    , U_DIMLESS , 0, "Нет данных");
	list.add(p + "vendor"      ,             rVariable::Flags::R___, &m_module.IDVendor  , U_DIMLESS , 0, "Нет данных");
	list.add(p + "productCode" ,             rVariable::Flags::R___, &m_module.IDProdCode, U_DIMLESS , 0, "Нет данных");
	list.add(p + "revision"    ,             rVariable::Flags::R___, &m_module.IDRevision, U_DIMLESS , 0, "Нет данных");
	list.add(p + "serialNumber",             rVariable::Flags::R___, &m_module.IDSerial  , U_DIMLESS , 0, "Нет данных");
	list.add(p + "can"         ,             rVariable::Flags::R___, &m_CAN              , U_DIMLESS , 0, "Нет данных");
	list.add(p + "firmware"    ,             rVariable::Flags::R___, &m_firmware         , U_DIMLESS , 0, "Нет данных");
	list.add(p + "hardware"    ,             rVariable::Flags::R___, &m_hardware         , U_DIMLESS , 0, "Нет данных");

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
	UNUSED(md);
	return TRITONN_RESULT_OK;
}

std::string rIOBaseModule::getXmlChannels()
{
	std::string result = "";

	for (auto channel : m_listChannel) {
		result += "\t<channel number=\"" + String_format("%u", channel->m_index);
		result += "\" setup=\"" + channel->getStrType() + " setup flags\" />\n";
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

