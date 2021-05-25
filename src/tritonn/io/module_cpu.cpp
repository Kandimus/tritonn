//=================================================================================================
//===
//=== module_cpu.cpp
//===
//=== Copyright (c) 2021 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================

#include "module_cpu.h"
#include "defines.h"
#include "locker.h"
#include "tinyxml2.h"
#include "../error.h"
#include "../xml_util.h"
#include "../variable_class.h"
#include "../units.h"
#include "../generator_md.h"

rModuleCPU::rModuleCPU(UDINT id) : rIOBaseModule(id)
{
	m_type    = Type::CPU;
	m_comment = "Base CPU module";
	m_name    = "cpu";
}

rModuleCPU::rModuleCPU(const rModuleCPU* cpu) : rIOBaseModule(cpu)
{
	;
}

UDINT rModuleCPU::processing(USINT issim)
{
	rLocker lock(m_mutex); UNUSED(lock);

	rIOBaseModule::processing(issim);

	return TRITONN_RESULT_OK;
}


UDINT rModuleCPU::generateVars(const std::string& prefix, rVariableList& list, bool issimulate)
{
	//rIOBaseModule::generateVars(prefix, list, issimulate);
	return TRITONN_RESULT_OK;
}


UDINT rModuleCPU::loadFromXML(tinyxml2::XMLElement* element, rError& err)
{
//	if (rIOBaseModule::loadFromXML(element, err) != TRITONN_RESULT_OK) {
//		return err.getError();
//	}

	return TRITONN_RESULT_OK;
}

UDINT rModuleCPU::generateMarkDown(rGeneratorMD& md)
{
	md.add(this);

	return TRITONN_RESULT_OK;
}

rIOBaseChannel* rModuleCPU::getChannel(USINT channel)
{
	return nullptr;
}

rIOBaseModule* rModuleCPU::getModulePtr()
{
	return nullptr;
}
