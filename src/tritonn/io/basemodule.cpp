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
#include "../xml_util.h"
#include "../data_config.h"
#include "../variable_item.h"
#include "../variable_list.h"
#include "../units.h"
#include "../error.h"
//#include "../generator_md.h"


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

	list.add(p + "type"        , TYPE_UINT , rVariable::Flags::R___, &m_type        , U_DIMLESS , 0);
	list.add(p + "node"        , TYPE_UINT , rVariable::Flags::R___, &m_nodeID      , U_DIMLESS , 0);
	list.add(p + "vendor"      , TYPE_UDINT, rVariable::Flags::R___, &m_vendorID    , U_DIMLESS , 0);
	list.add(p + "productCode" , TYPE_UDINT, rVariable::Flags::R___, &m_productCode , U_DIMLESS , 0);
	list.add(p + "revision"    , TYPE_UDINT, rVariable::Flags::R___, &m_revision    , U_DIMLESS , 0);
	list.add(p + "serialNumber", TYPE_UDINT, rVariable::Flags::R___, &m_serialNumber, U_DIMLESS , 0);
	list.add(p + "temperature" , TYPE_REAL , rVariable::Flags::R___, &m_temperature , U_DIMLESS , 0);
	list.add(p + "can"         , TYPE_UINT , rVariable::Flags::R___, &m_CAN         , U_DIMLESS , 0);
	list.add(p + "firmware"    , TYPE_UINT , rVariable::Flags::R___, &m_firmware    , U_DIMLESS , 0);
	list.add(p + "hardware"    , TYPE_UINT , rVariable::Flags::R___, &m_hardware    , U_DIMLESS , 0);

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

UDINT rIOBaseModule::generateMarkDown(rGeneratorMD& md) const
{
	UNUSED(md)
	return TRITONN_RESULT_OK;
}


std::string rIOBaseModule::getMarkDown()
{
	std::string result = "";

	result += "\n## Channels\n";
	result += "Number | Unit | Unit ID | Limits | Shadow | Comment\n";
	result += ":-- |:--:|:--:|:--:|:--:|:--\n";

		for (auto link : m_inputs) {
			std::string strunit = "";

			rTextManager::instance().Get(link->m_unit, strunit);

			result += link->m_ioName + " | ";
			result += strunit + " | " + String_format("%u", static_cast<UDINT>(link->m_unit)) + " | ";
			result += link->m_limit.m_flagsSetup.getNameByBits(link->m_limit.m_setup.Value, ", ") + " | ";
			result += link->m_shadow + " | ";
			result += link->m_comment + "\n";
		}
	}

	result += "\n## Outputs\n";
	result += "Output | Unit | Unit ID | Limits | Comment\n";
	result += ":-- |:--:|:--:|:--:|:--\n";
	for (auto link : m_outputs) {
		std::string strunit = "";

		rTextManager::instance().Get(link->m_unit, strunit);

		result += link->m_ioName + " | ";
		result += strunit + " | " + String_format("%u", static_cast<UDINT>(link->m_unit)) + " | ";

		result += link->m_limit.m_flagsSetup.getNameByBits(link->m_limit.m_setup.Value, ", ") + " | ";
		result += link->m_comment + "\n";
	}

	rVariableList list;
	generateVars(list);

	result += "\n## Variable\n";
	result += list.getMarkDown();
	result += "\n";

	return result;
}

