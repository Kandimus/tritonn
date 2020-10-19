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

std::string rIOBaseModule::m_name = "BaseModule";

UDINT rIOBaseModule::processing(USINT issim)
{
	UNUSED(issim);
	return TRITONN_RESULT_OK;
}

