//=================================================================================================
//===
//=== module_crm.cpp
//===
//=== Copyright (c) 2021 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс модуля поверочной установки (CRM)
//===
//=================================================================================================

#include "module_crm.h"
#include "locker.h"
#include "tinyxml2.h"
#include "../error.h"
#include "../xml_util.h"

rModuleCRM::rModuleCRM()
{
	m_type    = Type::CRM;
	m_comment = "Module of prove";

	while(m_channelDI.size() < CHANNEL_DI_COUNT) {
		auto ch_di = new rIODIChannel(m_channelDI.size());
		m_channelDI.push_back(ch_di);
		m_listChannel.push_back(ch_di);
	}

	m_channelFI = new rIOFIChannel(m_channelDI.size());
	m_listChannel.push_back(m_channelFI);
}

rModuleCRM::~rModuleCRM()
{
	for (auto channel : m_channelDI) {
		if (channel) {
			delete channel;
		}
	}
	m_channelDI.clear();

	if (m_channelFI) {
		delete m_channelFI;
	}
	m_channelFI = nullptr;
}

UDINT rModuleCRM::processing(USINT issim)
{
	rIOBaseModule::processing(issim);

	for (auto channel : m_channelDI) {
		if (channel->m_setup & rIODIChannel::Setup::OFF) {
			continue;
		}

		if (issim) {
			channel->simulate();
		}

		channel->processing();
	}

	if (!(m_channelFI->m_setup & rIOFIChannel::Setup::OFF)) {
		if (issim) {
			m_channelFI->simulate();
		}

		m_channelFI->processing();
	}

	return TRITONN_RESULT_OK;
}


std::unique_ptr<rIOBaseChannel> rModuleCRM::getChannel(USINT num)
{
	if (num >= CHANNEL_DI_COUNT) {
		return nullptr;
	}

	rLocker lock(m_mutex); UNUSED(lock);

	if (num < CHANNEL_DI_COUNT) {
		auto module_ptr = std::make_unique<rIODIChannel>(*m_channelDI[num]);

		return module_ptr;
	}

	auto module_ptr = std::make_unique<rIOFIChannel>(*m_channelFI);

	return module_ptr;
}

LREAL rModuleCRM::getFreq() const
{
	return m_channelFI->getFreq();
}

UDINT rModuleCRM::getCounter() const
{
	return m_channelFI->getValue();
}

UINT rModuleCRM::getDetectors() const
{
	return ((m_channelDI[0]->getValue() != 0) << Detector::Det1) |
		   ((m_channelDI[1]->getValue() != 0) << Detector::Det2) |
		   ((m_channelDI[2]->getValue() != 0) << Detector::Det3) |
		   ((m_channelDI[3]->getValue() != 0) << Detector::Det4);

}

UDINT rModuleCRM::generateVars(const std::string& prefix, rVariableList& list, bool issimulate)
{
	rIOBaseModule::generateVars(prefix, list, issimulate);

	for (auto channel : m_listChannel) {
		std::string p = prefix + m_name + ".ch_" + String_format("%02i", channel->m_index + 1);
		channel->generateVars(p, list, issimulate);
	}

	return TRITONN_RESULT_OK;
}


UDINT rModuleCRM::loadFromXML(tinyxml2::XMLElement* element, rError& err)
{
	if (rIOBaseModule::loadFromXML(element, err) != TRITONN_RESULT_OK) {
		return err.getError();
	}

	XML_FOR(channel_xml, element, XmlName::CHANNEL) {
		USINT number = XmlUtils::getAttributeUSINT(channel_xml, XmlName::NUMBER, 0xFF);

		if (number > CHANNEL_DI_COUNT) {
			return err.set(DATACFGERR_IO_CHANNEL, channel_xml->GetLineNum(), "invalid module count");
		}

		if (number < CHANNEL_DI_COUNT) {
			m_channelDI[number]->loadFromXML(channel_xml, err);
		} else {
			m_channelFI->loadFromXML(channel_xml, err);
		}

		if (err.getError()) {
			return err.getError();
		}
	}

	return TRITONN_RESULT_OK;
}
