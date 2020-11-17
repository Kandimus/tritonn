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

#include "locker.h"
#include "io_ai6.h"
#include "data_config.h"
#include "tinyxml2.h"
#include "xml_util.h"

std::string rIOAI6::m_rtti = "ai6";
rBitsArray  rIOAI6::m_flagsSetup;

rIOAI6::rIOAI6()
{
	if (m_flagsSetup.empty()) {
		m_flagsSetup
				.add("OFF"    , rIOAIChannel::Setup::OFF)
				.add("AVERAGE", rIOAIChannel::Setup::AVERAGE)
				.add("NOICE"  , rIOAIChannel::Setup::NOICE);
	}

	while(m_channel.size() < CHANNEL_COUNT) {
		m_channel.push_back(rIOAIChannel());
	}

	m_channel[0].m_simSpeed = 1111;
	m_channel[0].m_simType  = rIOAIChannel::SimType::Linear;
}


rIOAI6::~rIOAI6()
{

}


UDINT rIOAI6::processing(USINT issim)
{
	for (auto& channel : m_channel) {
		if (issim) {
			channel.simulate();
		}

		channel.processing();
	}

	return TRITONN_RESULT_OK;
}


std::unique_ptr<rIOBaseChannel> rIOAI6::getChannel(USINT num)
{
	if (num >= CHANNEL_COUNT) {
		return nullptr;
	}

	rLocker lock(m_mutex); UNUSED(lock);

	auto module_ptr = std::make_unique<rIOAIChannel>(m_channel[num]);

	return module_ptr;
}

UDINT rIOAI6::generateVars(const std::string& prefix, rVariableList& list)
{
	rIOBaseModule::generateVars(prefix, list);

	for (int ii = 0; ii < CHANNEL_COUNT; ++ii) {
		std::string p = prefix + m_name + ".ch_" + String_format("%02i", ii + 1);
		m_channel[ii].generateVars(p, list);
	}
	return TRITONN_RESULT_OK;
}


UDINT rIOAI6::loadFromXML(tinyxml2::XMLElement* element, rDataConfig &cfg)
{
	UDINT result = rIOBaseModule::loadFromXML(element, cfg);

	if (result != TRITONN_RESULT_OK) {
		return result;
	}

	XML_FOR(channel_xml, element, XmlName::CHANNEL) {
		UDINT       err      = 0;
		USINT       number   = XmlUtils::getAttributeUSINT (channel_xml, XmlName::NUMBER, 0xFF);
		std::string strSetup = XmlUtils::getAttributeString(channel_xml, XmlName::SETUP, "");

		if (number >= CHANNEL_COUNT) {
			cfg.ErrorLine = channel_xml->GetLineNum();
			cfg.ErrorID   = DATACFGERR_IO_CHANNEL;
			return cfg.ErrorID;
		}

		m_channel[number].m_setup = m_flagsSetup.getValue(strSetup, err);

		if (err) {
			cfg.ErrorLine = channel_xml->GetLineNum();
			cfg.ErrorID   = DATACFGERR_IO_CHANNEL;
			return cfg.ErrorID;
		}
	}

	return TRITONN_RESULT_OK;
}
