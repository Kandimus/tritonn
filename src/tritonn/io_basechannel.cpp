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

#pragma once

#include "io_basechannel.h"
#include "variable_item.h"
#include "variable_list.h"
#include "simpleargs.h"
#include "def_arguments.h"
#include "units.h"

rIOBaseChannel::rIOBaseChannel()
{

}

rIOBaseChannel::~rIOBaseChannel()
{

}

UDINT rIOBaseChannel::generateVars(const std::string &name, rVariableList &list)
{
	std::string p = name + ".";

	if (rSimpleArgs::instance().isSet(rArg::Simulate)) {
		list.add(p + "simulate.type", TYPE_USINT, rVariable::Flags::____, &m_simType, U_DIMLESS , 0);
	}

	return TRITONN_RESULT_OK;
}



