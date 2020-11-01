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

#include "io_basemodule.h"
#include "xml_util.h"
#include "data_config.h"
#include "variable_item.h"
#include "variable_list.h"
#include "simpleargs.h"
#include "def_arguments.h"
#include "units.h"


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

UDINT rIOBaseModule::generateVars(const std::string& prefix, rVariableList& list)
{
	std::string p = prefix + m_name + ".";

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
