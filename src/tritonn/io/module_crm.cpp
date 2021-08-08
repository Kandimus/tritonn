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
#include "xml_util.h"
#include "../error.h"
#include "../generator_md.h"

rModuleCRM::rModuleCRM(UDINT id) : rIOBaseModule(id)
{
	m_type    = Type::CRM;
	m_comment = "Module of prove";
	m_name    = "crm";

	while(m_channelDI.size() < CHANNEL_DI_COUNT) {
		auto ch_di = new rIODIChannel(m_channelDI.size(), String_format("Детектор %u", m_channelDI.size() + 1));

		ch_di->m_canIdx = m_channelDI.size();

		m_channelDI.push_back(ch_di);
		m_listChannel.push_back(ch_di);
	}

	m_channelFI = new rIOFIChannel(m_channelDI.size(), "Частота с ПР");

	m_channelFI->m_canIdx = 0;

	m_listChannel.push_back(m_channelFI);
}

rModuleCRM::rModuleCRM(const rModuleCRM* crm)  : rIOBaseModule(crm)
{
	m_channelDI.clear();
	m_listChannel.clear();

	for (auto channel : crm->m_channelDI) {
		auto ch_di = new rIODIChannel(*channel);

		m_channelDI.push_back(ch_di);
		m_listChannel.push_back(ch_di);
	}

	m_channelFI = new rIOFIChannel(*crm->m_channelFI);
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
	rLocker lock(m_rwlock); lock.Nop();

	rIOBaseModule::processing(issim);

	for (auto channel : m_channelDI) {
		if (channel->isOff()) {
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


rIOBaseChannel* rModuleCRM::getChannel(USINT num, rIOBaseChannel::Type type)
{
	if (num >= CHANNEL_DI_COUNT + 1) {
		return nullptr;
	}

	rLocker lock(m_rwlock); lock.Nop();

	if (num < CHANNEL_DI_COUNT) {
		return (m_channelDI[num]->getType() == type) ? new rIODIChannel(*m_channelDI[num]) : nullptr;
	}

	return (m_channelFI->getType() == type) ? new rIOFIChannel(*m_channelFI) : nullptr;
}

LREAL rModuleCRM::getFreq() const
{
	return m_channelFI->getFreq();
}

UDINT rModuleCRM::getCounter() const
{
	return m_channelFI->getCounter();
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

UDINT rModuleCRM::generateMarkDown(rGeneratorMD& md)
{
	md.add(this)
			.addRemark("[^simtype]: **Тип симуляции DI:**<br/>" + rIODIChannel::m_flagsSimType.getInfo(true) +
					   "<br/>**Тип симуляции FI:**</br>" + rIOFIChannel::m_flagsSimType.getInfo(true));

	return TRITONN_RESULT_OK;
}
