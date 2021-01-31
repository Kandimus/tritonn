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

rBitsArray  rModuleAI6::m_flagsSetup;

rModuleAI6::rModuleAI6()
{
	if (m_flagsSetup.empty()) {
		m_flagsSetup
				.add("OFF"    , static_cast<UINT>(rIOAIChannel::Setup::OFF))
				.add("AVERAGE", static_cast<UINT>(rIOAIChannel::Setup::AVERAGE))
				.add("NOICE"  , static_cast<UINT>(rIOAIChannel::Setup::NOICE));
	}

	while(m_channel.size() < CHANNEL_COUNT) {
		m_channel.push_back(rIOAIChannel());
	}

	m_type = Type::AI6;

	m_channel[0].m_simSpeed = 1111;
	m_channel[0].m_simType  = rIOAIChannel::SimType::Linear;
}


rModuleAI6::~rModuleAI6()
{

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

	for (UDINT ii = 0; ii < CHANNEL_COUNT; ++ii) {
		std::string p = prefix + m_name + ".ch_" + String_format("%02i", ii + 1);
		m_channel[ii].generateVars(p, list, issimulate);
	}
	return TRITONN_RESULT_OK;
}


UDINT rModuleAI6::loadFromXML(tinyxml2::XMLElement* element, rError& err)
{
	if (rIOBaseModule::loadFromXML(element, err) != TRITONN_RESULT_OK) {
		return err.getError();
	}

	XML_FOR(channel_xml, element, XmlName::CHANNEL) {
		USINT       number   = XmlUtils::getAttributeUSINT (channel_xml, XmlName::NUMBER, 0xFF);
		std::string strSetup = XmlUtils::getAttributeString(channel_xml, XmlName::SETUP, "");

		if (number >= CHANNEL_COUNT) {
			return err.set(DATACFGERR_IO_CHANNEL, channel_xml->GetLineNum(), "invalide number");
		}

		UDINT fault = 0;
		m_channel[number].m_setup = m_flagsSetup.getValue(strSetup, fault);

		if (fault) {
			return err.set(DATACFGERR_IO_CHANNEL, channel_xml->GetLineNum(), "invalide setup");
		}
	}

	return TRITONN_RESULT_OK;
}
