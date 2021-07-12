/*
 *
 * module_di16.cpp
 *
 * Copyright (c) 2021 by RangeSoft.
 * All rights reserved.
 *
 * Litvinov "VeduN" Vitaliy O.
 *
 */

#include "module_di16.h"
#include "locker.h"
#include "xml_util.h"
#include "../error.h"
#include "../generator_md.h"

rModuleDI16::rModuleDI16(UDINT id) : rIOBaseModule(id)
{
	m_type    = Type::DI16;
	m_comment = "Module 16 discrete input";
	m_name    = "di16";

	while(m_channel.size() < CHANNEL_COUNT) {
		auto ch_di = new rIODIChannel(static_cast<USINT>(m_channel.size()));
		m_channel.push_back(ch_di);
		m_listChannel.push_back(ch_di);
	}

}

rModuleDI8DO8::rModuleDI8DO8(const rModuleDI8DO8* di8do8) : rIOBaseModule(di8do8)
{
	m_channelDI.clear();
	m_channelDO.clear();
	m_listChannel.clear();

	for (auto channel : di8do8->m_channelDI) {
		auto ch_di = new rIODIChannel(*channel);

		m_channelDI.push_back(ch_di);
		m_listChannel.push_back(ch_di);
	}

	for (auto channel : di8do8->m_channelDO) {
		auto ch_do = new rIODOChannel(*channel);

		m_channelDO.push_back(ch_do);
		m_listChannel.push_back(ch_do);
	}
}

rModuleDI8DO8::~rModuleDI8DO8()
{
	for (auto channel : m_channelDI) {
		if (channel) {
			delete channel;
		}
	}
	m_channelDI.clear();

	for (auto channel : m_channelDO) {
		if (channel) {
			delete channel;
		}
	}
	m_channelDO.clear();
}

UDINT rModuleDI8DO8::processing(USINT issim)
{
	rLocker lock(m_rwlock); lock.Nop();

	rIOBaseModule::processing(issim);

	for (auto channel : m_listChannel) {
		if (issim) {
			channel->simulate();
		}

		channel->processing();
	}

	return TRITONN_RESULT_OK;
}


rIOBaseChannel* rModuleDI8DO8::getChannel(USINT num)
{
	if (num >= CHANNEL_DI_COUNT + CHANNEL_DO_COUNT) {
		return nullptr;
	}

	rLocker lock(m_rwlock); lock.Nop();

	if (num < CHANNEL_DI_COUNT) {
		return new rIODIChannel(*m_channelDI[num]);
	}

	return new rIODOChannel(*m_channelDO[num - CHANNEL_DI_COUNT]);
}

UDINT rModuleDI8DO8::generateVars(const std::string& prefix, rVariableList& list, bool issimulate)
{
	rIOBaseModule::generateVars(prefix, list, issimulate);

	for (auto channel : m_listChannel) {
		std::string p = prefix + m_name + ".ch_" + String_format("%02i", channel->m_index);
		channel->generateVars(p, list, issimulate);
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
			m_channelDI[number]->loadFromXML(channel_xml, err);
		} else {
			m_channelDO[number - CHANNEL_DI_COUNT]->loadFromXML(channel_xml, err);
		}

		if (err.getError()) {
			return err.getError();
		}
	}

	return TRITONN_RESULT_OK;
}

UDINT rModuleDI8DO8::generateMarkDown(rGeneratorMD& md)
{
	md.add(this)
			.addRemark("[^simtype]: **Тип симуляции DI:**<br/>" + rIODIChannel::m_flagsSimType.getInfo(true) + "<br/>");

	return TRITONN_RESULT_OK;
}
