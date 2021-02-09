//=================================================================================================
//===
//=== module_fi4.cpp
//===
//=== Copyright (c) 2020 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс частотного модуля CAN (FI)
//===
//=================================================================================================

#include "module_crm.h"
#include "locker.h"
#include "tinyxml2.h"
#include "../error.h"
#include "../xml_util.h"

rBitsArray rModuleCRM::m_flagsSetup;

rModuleCRM::rModuleCRM()
{
	if (m_flagsSetup.empty()) {
		m_flagsSetup
				.add("OFF"    , static_cast<UINT>(rIOFIChannel::Setup::OFF))
				.add("AVERAGE", static_cast<UINT>(rIOFIChannel::Setup::AVERAGE));
	}

	while(m_channelDI.size() < CHANNEL_DI_COUNT) {
		m_channelDI.push_back(rIODIChannel());
	}

	m_type = Type::CRM;
}


UDINT rModuleCRM::processing(USINT issim)
{
	rIOBaseModule::processing(issim);

	for (auto& channel : m_channelDI) {
		if (channel.m_setup & rIODIChannel::Setup::OFF) {
			continue;
		}

		if (issim) {
			channel.simulate();
		}

		channel.processing();
	}

	return TRITONN_RESULT_OK;
}


std::unique_ptr<rIOBaseChannel> rModuleCRM::getChannel(USINT num)
{
	if (num >= CHANNEL_DI_COUNT) {
		return nullptr;
	}

	rLocker lock(m_mutex); UNUSED(lock);

	auto module_ptr = std::make_unique<rIOFIChannel>(m_channel[num]);

	return module_ptr;
}


UDINT rModuleCRM::generateVars(const std::string& prefix, rVariableList& list, bool issimulate)
{
	rIOBaseModule::generateVars(prefix, list, issimulate);

	for (UDINT ii = 0; ii < CHANNEL_DI_COUNT; ++ii) {
		std::string p = prefix + m_name + ".ch_" + String_format("%02i", ii + 1);
		m_channel[ii].generateVars(p, list, issimulate);
	}

	return TRITONN_RESULT_OK;
}


UDINT rModuleCRM::loadFromXML(tinyxml2::XMLElement* element, rError& err)
{
	if (rIOBaseModule::loadFromXML(element, err) != TRITONN_RESULT_OK) {
		return err.getError();
	}

	XML_FOR(channel_xml, element, XmlName::CHANNEL) {
		USINT       number   = XmlUtils::getAttributeUSINT (channel_xml, XmlName::NUMBER, 0xFF);
		std::string strSetup = XmlUtils::getAttributeString(channel_xml, XmlName::SETUP, "");

		if (number >= CHANNEL_COUNT) {
			return err.set(DATACFGERR_IO_CHANNEL, channel_xml->GetLineNum(), "invalid module count");
		}

		UDINT fault = 0;
		m_channel[number].m_setup = m_flagsSetup.getValue(strSetup, fault);

		if (fault) {
			return err.set(DATACFGERR_IO_CHANNEL, channel_xml->GetLineNum(), "invalide setup");
		}
	}

	return TRITONN_RESULT_OK;
}
