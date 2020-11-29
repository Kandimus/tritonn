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

rIOBaseChannel::rIOBaseChannel()
{

}

rIOBaseChannel::~rIOBaseChannel()
{

}

UDINT rIOBaseChannel::generateVars(const std::string &name, rVariableList &list, bool issimulate)
{
	std::string p = name + ".";

	if (issimulate) {
		list.add(p + "simulate.type", TYPE_USINT, rVariable::Flags::____, &m_simType, U_DIMLESS , 0);
	}

	return TRITONN_RESULT_OK;
}



