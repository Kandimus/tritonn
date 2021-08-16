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
#include "xml_util.h"
#include "../error.h"
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
	rLocker lock(m_rwlock); lock.Nop();

	rIOBaseModule::processing(issim);

	return TRITONN_RESULT_OK;
}

UDINT rModuleCPU::getPulling()
{
	return 0;
}

UDINT rModuleCPU::getValue(USINT num, rIOBaseChannel::Type type, UDINT& fault)
{
	UNUSED(num);
	UNUSED(type);

	fault = DATACFGERR_REALTIME_WRONGCHANNEL;
	return 0;
}

UDINT rModuleCPU::setValue(USINT num, rIOBaseChannel::Type type, UDINT value)
{
	UNUSED(num);
	UNUSED(type);
	UNUSED(value);

	return DATACFGERR_REALTIME_WRONGCHANNEL;
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
