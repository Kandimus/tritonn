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

#include "module_di8do8.h"
#include "locker.h"
#include "tinyxml2.h"
#include "../data_config.h"
#include "../xml_util.h"

rBitsArray  rModuleDI8DO8::m_flagsDISetup;
rBitsArray  rModuleDI8DO8::m_flagsDOSetup;

rModuleDI8DO8::rModuleDI8DO8()
{
	if (m_flagsDISetup.empty()) {
		m_flagsDISetup
				.add("OFF"     , rIODIChannel::Setup::OFF)
				.add("BOUNCE"  , rIODIChannel::Setup::BOUNCE)
				.add("INVERSED", rIODIChannel::Setup::INVERSED);
	}

	if (m_flagsDOSetup.empty()) {
		m_flagsDOSetup
				.add("OFF"     , rIODOChannel::Setup::OFF)
				.add("INVERTED", rIODOChannel::Setup::INVERTED);
	}

	while(m_channelDI.size() < CHANNEL_DI_COUNT) {
		m_channelDI.push_back(rIODIChannel());
	}

	while(m_channelDO.size() < CHANNEL_DO_COUNT) {
		m_channelDO.push_back(rIODOChannel());
	}
}


rModuleDI8DO8::~rModuleDI8DO8()
{

}


UDINT rModuleDI8DO8::processing(USINT issim)
{
	for (auto& channel : m_channelDI) {
		if (issim) {
			channel.simulate();
		}

		channel.processing();
	}

	for (auto& channel : m_channelDO) {
		if (issim) {
			channel.simulate();
		}

		channel.processing();
	}

	return TRITONN_RESULT_OK;
}


std::unique_ptr<rIOBaseChannel> rModuleDI8DO8::getChannel(USINT num)
{
	if (num >= CHANNEL_DI_COUNT + CHANNEL_DO_COUNT) {
		return nullptr;
	}

	rLocker lock(m_mutex); UNUSED(lock);

	if (num < CHANNEL_DI_COUNT) {
		auto module_ptr = std::make_unique<rIODIChannel>(m_channelDI[num]);
		return module_ptr;
	}

	auto module_ptr = std::make_unique<rIODOChannel>(m_channelDO[num - CHANNEL_DI_COUNT]);

	return module_ptr;
}

UDINT rModuleDI8DO8::generateVars(const std::string& prefix, rVariableList& list, bool issimulate)
{
	rIOBaseModule::generateVars(prefix, list, issimulate);

	for (UDINT ii = 0; ii < CHANNEL_DI_COUNT; ++ii) {
		std::string p = prefix + m_name + ".ch_" + String_format("%02i", ii + 1);
		m_channelDI[ii].generateVars(p, list, issimulate);
	}

	for (UDINT ii = 0; ii < CHANNEL_DO_COUNT; ++ii) {
		std::string p = prefix + m_name + ".ch_" + String_format("%02i", CHANNEL_DI_COUNT + ii + 1);
		m_channelDO[ii].generateVars(p, list, issimulate);
	}
	return TRITONN_RESULT_OK;
}


UDINT rModuleDI8DO8::loadFromXML(tinyxml2::XMLElement* element, rDataConfig &cfg)
{
	UDINT result = rIOBaseModule::loadFromXML(element, cfg);

	if (result != TRITONN_RESULT_OK) {
		return result;
	}

	XML_FOR(channel_xml, element, XmlName::CHANNEL) {
		UDINT       err      = 0;
		USINT       number   = XmlUtils::getAttributeUSINT (channel_xml, XmlName::NUMBER, 0xFF);
		std::string strSetup = XmlUtils::getAttributeString(channel_xml, XmlName::SETUP, "");

		if (number >= CHANNEL_DI_COUNT + CHANNEL_DO_COUNT) {
			cfg.ErrorLine = channel_xml->GetLineNum();
			cfg.ErrorID   = DATACFGERR_IO_CHANNEL;
			return cfg.ErrorID;
		}

		if (number < CHANNEL_DI_COUNT) {
			m_channelDI[number].m_setup = m_flagsDISetup.getValue(strSetup, err);
		} else {
			m_channelDO[number - CHANNEL_DI_COUNT].m_setup = m_flagsDOSetup.getValue(strSetup, err);
		}

		if (err) {
			cfg.ErrorLine = channel_xml->GetLineNum();
			cfg.ErrorID   = DATACFGERR_IO_CHANNEL;
			return cfg.ErrorID;
		}
	}

	return TRITONN_RESULT_OK;
}
