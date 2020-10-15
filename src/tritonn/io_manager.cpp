//=================================================================================================
//===
//=== io_manager.h
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

#pragma once

#include "io_basechannel.h"
#include "io_manager.h"

rIOBaseChannel* rIOManager::getChannel(USINT module, USINT channel)
{
	if (module >= m_modules.size()) {
		return nullptr;
	}

	return m_modules[module]->getChannel(channel);
}

