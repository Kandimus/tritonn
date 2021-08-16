//=================================================================================================
//===
//=== io_basechannel.cp
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Базовый класс канала
//===
//=================================================================================================

#include "basechannel.h"
#include "../variable_item.h"
#include "../variable_list.h"
#include "../units.h"


UDINT rIOBaseChannel::processing()
{
	return TRITONN_RESULT_OK;
}

UDINT rIOBaseChannel::generateVars(const std::string &name, rVariableList &list, bool issimulate)
{
	std::string p = name + ".";

	if (issimulate) {
		list.add(p + "simulate.type", rVariable::Flags::____, &m_simType, U_DIMLESS , 0, "Тип симуляции (зависит от типа канала) [^simtype]");
	}

	return TRITONN_RESULT_OK;
}

std::string rIOBaseChannel::getStrType() const
{
	switch(m_type) {
		case Type::UNDEF: return "UNDEF";
		case Type::AI:    return "AI";
		case Type::DI:    return "DI";
		case Type::DO:    return "DO";
		case Type::FI:    return "FI";
		case Type::AO:    return "AO";
		default: return "ERROR";
	}
}

rIOBaseChannel::Type rIOBaseChannel::getType() const
{
	return m_type;
}
