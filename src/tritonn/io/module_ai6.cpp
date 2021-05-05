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

#include "module_ai6.h"
#include "locker.h"
#include "tinyxml2.h"
#include "../error.h"
#include "../xml_util.h"
#include "../generator_md.h"

rModuleAI6::rModuleAI6(UDINT id) : rIOBaseModule(id)
{
	m_type    = Type::AI6;
	m_comment = "Module 6 current/voltage channels";
	m_name    = "ai6";

	while (m_channel.size() < CHANNEL_COUNT) {
		auto ch_ai = new rIOAIChannel(static_cast<USINT>(m_channel.size()));
		m_channel.push_back(ch_ai);
		m_listChannel.push_back(ch_ai);
	}

#ifndef TRITONN_TEST
	m_channel[0]->m_simType  = rIOAIChannel::SimType::LINEAR;
	m_channel[0]->m_simSpeed = 1234;
#endif
}

rModuleAI6::rModuleAI6(const rModuleAI6* ai6) : rIOBaseModule(ai6)
{
	m_channel.clear();
	m_listChannel.clear();

	for (auto channel : ai6->m_channel) {
		auto ch_ai = new rIOAIChannel(*channel);

		m_channel.push_back(ch_ai);
		m_listChannel.push_back(ch_ai);
	}
}

rModuleAI6::~rModuleAI6()
{
	for (auto channel : m_channel) {
		if (channel) {
			delete channel;
		}
	}
	m_channel.clear();
}


UDINT rModuleAI6::processing(USINT issim)
{
	rLocker lock(m_mutex); UNUSED(lock);

	rIOBaseModule::processing(issim);

	for (auto channel : m_channel) {
		if (issim) {
			channel->simulate();
		}

		channel->processing();
	}

	return TRITONN_RESULT_OK;
}


rIOBaseChannel* rModuleAI6::getChannel(USINT num)
{
	if (num >= CHANNEL_COUNT) {
		return nullptr;
	}

	rLocker lock(m_mutex); UNUSED(lock);

	return new rIOAIChannel(*m_channel[num]);
}

UDINT rModuleAI6::generateVars(const std::string& prefix, rVariableList& list, bool issimulate)
{
	rIOBaseModule::generateVars(prefix, list, issimulate);

	for (auto channel : m_channel) {
		std::string p = prefix + m_name + ".ch_" + String_format("%02i", channel->m_index);
		channel->generateVars(p, list, issimulate);
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

		if (m_channel[number]->loadFromXML(channel_xml, err) != TRITONN_RESULT_OK) {
			return err.getError();
		}
	}

	return TRITONN_RESULT_OK;
}

UDINT rModuleAI6::generateMarkDown(rGeneratorMD& md)
{
	md.add(this)
			.addRemark("[^simtype]: **Тип симуляции:**<br/>" + rIOAIChannel::m_flagsSimType.getInfo(true) + "</br>");

	return TRITONN_RESULT_OK;
}
