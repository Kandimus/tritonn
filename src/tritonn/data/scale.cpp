/*
 *
 * data/scale.cpp
 *
 * Copyright (c) 2021 by RangeSoft.
 * All rights reserved.
 *
 * Litvinov "VeduN" Vitaliy O.
 *
 */

#include "scale.h"
#include "../variable_list.h"

void rScale::generateVars(rVariableList& list, const std::string& alias, STRID unit)
{
	list.add(alias + ".scales.low" , rVariable::Flags::___D, &m_min.Value, unit, ACCESS_SCALES, "Значение инженерного минимума");
	list.add(alias + ".scales.high", rVariable::Flags::___D, &m_max.Value, unit, ACCESS_SCALES, "Значение инженерного максимума");
}
