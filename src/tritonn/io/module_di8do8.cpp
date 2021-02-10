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
#include "../error.h"
#include "../xml_util.h"

rBitsArray rModuleDI8DO8::m_flagsDOSetup;

rModuleDI8DO8::rModuleDI8DO8()
{
	while(m_channelDI.size() < CHANNEL_DI_COUNT) {
		m_channelDI.push_back(rIODIChannel(static_cast<USINT>(m_channelDI.size())));
	}

	while(m_channelDO.size() < CHANNEL_DO_COUNT) {
		m_channelDO.push_back(rIODOChannel(static_cast<USINT>(CHANNEL_DI_COUNT + m_channelDO.size())));
	}

	m_type = Type::DI8DO8;
}


UDINT rModuleDI8DO8::processing(USINT issim)
{
	rIOBaseModule::processing(issim);

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

	for (auto& channel : m_channelDI) {
		std::string p = prefix + m_name + ".ch_" + String_format("%02i", channel.m_index + 1);
		channel.generateVars(p, list, issimulate);
	}

	for (auto& channel : m_channelDO) {
		std::string p = prefix + m_name + ".ch_" + String_format("%02i", channel.m_index + 1);
		channel.generateVars(p, list, issimulate);
	}
	return TRITONN_RESULT_OK;
}


UDINT rModuleDI8DO8::loadFromXML(tinyxml2::XMLElement* element, rError& err)
{
	if (rIOBaseModule::loadFromXML(element, err) != TRITONN_RESULT_OK) {
		return err.getError();
	}

	XML_FOR(channel_xml, element, XmlName::CHANNEL) {
		USINT number = XmlUtils::getAttributeUSINT (channel_xml, XmlName::NUMBER, 0xFF);

		if (number >= CHANNEL_DI_COUNT + CHANNEL_DO_COUNT) {
			return err.set(DATACFGERR_IO_CHANNEL, channel_xml->GetLineNum(), "invalide number");
		}

		if (number < CHANNEL_DI_COUNT) {
			m_channelDI[number].loadFromXML(channel_xml, err);
		} else {
			m_channelDO[number - CHANNEL_DI_COUNT].loadFromXML(channel_xml, err);
		}

		if (err.getError()) {
			return err.getError();
		}
	}

	return TRITONN_RESULT_OK;
}
