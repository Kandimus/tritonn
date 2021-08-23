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
#include "../text_id.h"
#include "../event/manager.h"
#include "../data_config.h"
#include "../variable_item.h"
#include "../variable_list.h"
#include "../units.h"
#include "../error.h"

rBitsArray rIOBaseModule::m_flagsType;
rBitsArray rIOBaseModule::m_flagsShortType;

rIOBaseModule::rIOBaseModule(UDINT id)
{
	if (m_flagsType.empty()) {
		m_flagsType
				.add("UNDEF" , static_cast<UINT>(Type::UNDEF) , "Модуль не определен")
				.add("CPU"   , static_cast<UINT>(Type::CPU)   , "Базовый модуль ЦПУ")
				.add("AI6a"  , static_cast<UINT>(Type::AI6a)  , "Модуль AI6a")
				.add("AI6p"  , static_cast<UINT>(Type::AI6p)  , "Модуль AI6p")
				.add("FI4"   , static_cast<UINT>(Type::FI4)   , "Модуль FI4")
				.add("DI8DO8", static_cast<UINT>(Type::DI8DO8), "Модуль DI8DO8")
				.add("DI16"  , static_cast<UINT>(Type::DI16)  , "Модуль DI16")
				.add("DO16"  , static_cast<UINT>(Type::DO16)  , "Модуль DO16")
				.add("CRM"   , static_cast<UINT>(Type::CRM)   , "Модуль CRM")
				.add("AO4"   , static_cast<UINT>(Type::AO4)   , "Модуль AO4");
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

void rIOBaseModule::setModule(void* data, ModuleInfo_str* info, ModuleSysData_str* sysdata, void* status, UDINT readAll, UDINT exchange)
{
	m_dataPtr        = data;
	m_moduleInfo     = info;
	m_moduleStatus   = static_cast<rModuleStatus*>(status);
	m_moduleSysData  = sysdata;
	m_moduleReadAll  = readAll;
	m_moduleExchange = exchange;
}

int rIOBaseModule::sendCanCommand(UDINT func, DINT idx, void* data)
{
#ifdef TRITONN_YOCTO
	return candrv_cmd(func, idx, data);
#else
	UNUSED(func);
	UNUSED(idx);
	UNUSED(data);

	return 0;
#endif
}

UDINT rIOBaseModule::processing(USINT issim)
{
	++m_pulling;

	if(issim || m_type == Type::CPU) {
		return TRITONN_RESULT_OK;
	}

	if (!m_moduleInfo || !m_dataPtr || !m_moduleStatus) {
		if (!m_isFault) {
			m_isFault = true;
			rEventManager::instance().add(rEvent(EID_HARDWARE_MODULE_ISNULL) << STRID(static_cast<UDINT>(m_type) + SID::HARWARE_SHORT_UNKNOW) << m_ID);
		}

		return DATACFGERR_HARDWARE_MODULEISNULL;
	}

	int result = 0;
	if (!m_moduleInfo->InWork) {
		result = sendCanCommand(m_moduleReadAll, m_ID, m_dataPtr);
	}
	if (m_moduleInfo->InWork) {
		result    = sendCanCommand(m_moduleExchange, m_ID, m_dataPtr);
		m_isFault = false;
	} else {
printf(">>>>>>>>>>>>>> RESULT: %i, InWork: %i\n", result, m_moduleInfo->InWork);
		if (!m_isFault) {
			m_isFault = true;
			rEventManager::instance().add(rEvent(EID_HARDWARE_MODULE_FAULT) << STRID(static_cast<UDINT>(m_type) + SID::HARWARE_SHORT_UNKNOW) << m_ID);
		}
	}

	m_module = *m_moduleSysData;
	m_status = *m_moduleStatus;

//	printModuleInfo();

	return TRITONN_RESULT_OK;
}

void rIOBaseModule::printModuleInfo()
{
	if (!m_moduleInfo || !m_moduleSysData) {
		return;
	}

	printf("Module type %s[%i]:\n", m_flagsType.getNameByValue(static_cast<UINT>(m_type)).c_str(), m_ID);
	printf("\tNodeID ....... %i\n", m_module.NodeID);
	printf("\tIDVendor ..... %i\n", m_module.IDVendor);
	printf("\tIDProdCode ... %i\n", m_module.IDProdCode);
	printf("\tIDRevision ... %i\n", m_module.IDRevision);
	printf("\tIDSerial ..... %i\n", m_module.IDSerial);
	printf("\tHeatrbeatState %i\n", m_module.HeatrbeatState);
	printf("\tHeatrbeatFl .. %i\n", m_module.HeatrbeatFl);
	printf("\tPDOCounter ... %i\n", m_module.PDOCounter);
	printf("\tPDOCycle ..... %i\n", m_module.PDOCycle);

	printf("\tCAN .......... %i\n", m_status.m_CAN);
	printf("\tFirmware ..... %i\n", m_status.m_firmware);
	printf("\tHardware ..... %i\n", m_status.m_hardware);
}

UDINT rIOBaseModule::generateVars(const std::string& prefix, rVariableList& list, bool issimulate)
{
	UNUSED(issimulate);

	m_alias = prefix + m_name;

	std::string p = m_alias + ".";

	list.add(p + "type"        , TYPE::UINT, rVariable::Flags::R___, &m_type             , U_DIMLESS , 0, "Тип модуля:<br>" + m_flagsType.getInfo(true));
	list.add(p + "node"        ,             rVariable::Flags::R___, &m_module.NodeID    , U_DIMLESS , 0, "Уникальный номер");
	list.add(p + "vendor"      ,             rVariable::Flags::R___, &m_module.IDVendor  , U_DIMLESS , 0, "Код производителя");
	list.add(p + "productCode" ,             rVariable::Flags::R___, &m_module.IDProdCode, U_DIMLESS , 0, "Код устройства");
	list.add(p + "revision"    ,             rVariable::Flags::R___, &m_module.IDRevision, U_DIMLESS , 0, "Версия сетевого драйвера");
	list.add(p + "serialNumber",             rVariable::Flags::R___, &m_module.IDSerial  , U_DIMLESS , 0, "Серийный номер нижнего уровня");
	list.add(p + "can"         ,             rVariable::Flags::R___, &m_status.m_CAN     , U_DIMLESS , 0, "Состояние обмена данными");
	list.add(p + "firmware"    ,             rVariable::Flags::R___, &m_status.m_firmware, U_DIMLESS , 0, "Версия ПО модуля");
	list.add(p + "hardware"    ,             rVariable::Flags::R___, &m_status.m_hardware, U_DIMLESS , 0, "Версия модуля");

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
		result += "\" setup=\"" + channel->getStrType() + " setup flags\"";
		result += channel->getXmlAttribute();
		result += " />\n";
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
			result += channel->getMarkDownFlags();//.getMarkDown(channel->getStrType() + " setup");
			ch_names.push_back(channel->getStrType());
		}

	}

	rVariableList list;
	generateVars("", list, true);

	result += list.getMarkDown();

	return result;
}

