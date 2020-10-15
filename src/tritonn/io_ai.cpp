//=================================================================================================
//===
//=== io_ai.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс аналового входног сигнала модуля CAN (AI)
//===
//=================================================================================================

#include "io_ai.h"



rIOAI6::rIOAI6()
{

}


rIOAI6::~rIOAI6()
{

}


UDINT rIOAI6::processing(USINT issim)
{
	if (issim) {
		for(auto ii = 0; ii < CHANNEL_COUNT; ++ii) {
			m_ai[ii].simulate();
		}

		return TRITONN_RESULT_OK;
	}

	return TRITONN_RESULT_OK;
}


rIOBaseChannel* rIOAI6::getChannel(USINT channel)
{
	if (channel < CHANNEL_COUNT) {
		return nullptr;
	}

	return &m_ai[channel];
}
