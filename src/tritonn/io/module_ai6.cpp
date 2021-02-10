﻿//=================================================================================================
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

#include "module_ai6.h"
#include "locker.h"
#include "tinyxml2.h"
#include "../error.h"
#include "../xml_util.h"

rModuleAI6::rModuleAI6()
{
	while(m_channel.size() < CHANNEL_COUNT) {
		m_channel.push_back(rIOAIChannel(static_cast<USINT>(m_channel.size())));
	}

	m_type = Type::AI6;

	m_channel[0].m_simSpeed = 1111;
	m_channel[0].m_simType  = rIOAIChannel::SimType::Linear;
}


UDINT rModuleAI6::processing(USINT issim)
{
	rIOBaseModule::processing(issim);

	for (auto& channel : m_channel) {
		if (issim) {
			channel.simulate();
		}

		channel.processing();
	}

	return TRITONN_RESULT_OK;
}


std::unique_ptr<rIOBaseChannel> rModuleAI6::getChannel(USINT num)
{
	if (num >= CHANNEL_COUNT) {
		return nullptr;
	}

	rLocker lock(m_mutex); UNUSED(lock);

	auto module_ptr = std::make_unique<rIOAIChannel>(m_channel[num]);

	return module_ptr;
}

UDINT rModuleAI6::generateVars(const std::string& prefix, rVariableList& list, bool issimulate)
{
	rIOBaseModule::generateVars(prefix, list, issimulate);

	for (auto& channel : m_channel) {
		std::string p = prefix + m_name + ".ch_" + String_format("%02i", channel.m_index);
		channel.generateVars(p, list, issimulate);
	}

	return TRITONN_RESULT_OK;
}


UDINT rModuleAI6::loadFromXML(tinyxml2::XMLElement* element, rError& err)
{
	if (rIOBaseModule::loadFromXML(element, err) != TRITONN_RESULT_OK) {
		return err.getError();
	}

	XML_FOR(channel_xml, element, XmlName::CHANNEL) {
		USINT number = XmlUtils::getAttributeUSINT (channel_xml, XmlName::NUMBER, 0xFF);

		if (number >= CHANNEL_COUNT) {
			return err.set(DATACFGERR_IO_CHANNEL, channel_xml->GetLineNum(), "invalide number");
		}

		if (m_channel[number].loadFromXML(channel_xml, err) != TRITONN_RESULT_OK) {
			return err.getError();
		}
	}

	return TRITONN_RESULT_OK;
}
