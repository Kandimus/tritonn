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

		ch_di->m_canIdx = m_channel.size();

		m_channel.push_back(ch_di);
		m_listChannel.push_back(ch_di);
	}

	memset(&m_data, 0, sizeof(m_data));
	setModule(&m_data, &m_data.ModuleInfo, &m_data.System, &m_data.Read.Status, _K19_DI16_ModuleReadAll, _K19_DI16_ModuleExchange);
}

rModuleDI16::rModuleDI16(const rModuleDI16* di16) : rIOBaseModule(di16)
{
	m_channel.clear();
	m_listChannel.clear();

	for (auto channel : di16->m_channel) {
		auto ch_di = new rIODIChannel(*channel);

		m_channel.push_back(ch_di);
		m_listChannel.push_back(ch_di);
	}
}

rModuleDI16::~rModuleDI16()
{
	for (auto channel : m_channel) {
		if (channel) {
			delete channel;
		}
	}
	m_channel.clear();
}

UDINT rModuleDI16::processing(USINT issim)
{
	rLocker lock(m_rwlock); lock.Nop();

	UDINT result = rIOBaseModule::processing(issim);
	if (result != TRITONN_RESULT_OK) {
		return result;
	}

	for (auto channel : m_channel) {
		USINT idx = channel->m_canIdx;

		if (issim) {
			channel->simulate();
		} else {
			channel->m_phValue = m_data.Read.In[idx] == UL_K19_DI16_ChStHigh;
		}

		channel->processing();
	}

	m_data.Write.Filter = 0;

	return TRITONN_RESULT_OK;
}


UDINT rModuleDI16::getValue(USINT num, rIOBaseChannel::Type type, UDINT& fault)
{
	if (num >= CHANNEL_COUNT) {
		fault = DATACFGERR_REALTIME_CHANNELLINK;
		return false;
	}

	if (m_channel[num]->m_type != type) {
		fault = DATACFGERR_REALTIME_WRONGCHANNEL;
		return false;
	}

	rLocker lock(m_rwlock); lock.Nop();

	return m_channel[num]->m_value;
}


UDINT rModuleDI16::setValue(USINT num, rIOBaseChannel::Type type, UDINT value)
{
	UNUSED(num);
	UNUSED(type);
	UNUSED(value);

	return DATACFGERR_REALTIME_WRONGCHANNEL;
}

rIOBaseChannel* rModuleDI16::getChannel(USINT num, rIOBaseChannel::Type type)
{
	if (num >= CHANNEL_COUNT) {
		return nullptr;
	}

	if (m_channel[num]->getType() != type) {
		return nullptr;
	}

	rLocker lock(m_rwlock); lock.Nop();

	return new rIODIChannel(*m_channel[num]);
}



UDINT rModuleDI16::generateVars(const std::string& prefix, rVariableList& list, bool issimulate)
{
	rIOBaseModule::generateVars(prefix, list, issimulate);

	for (auto channel : m_listChannel) {
		std::string p = prefix + m_name + ".ch_" + String_format("%02i", channel->m_index);
		channel->generateVars(p, list, issimulate);
	}

	return TRITONN_RESULT_OK;
}


UDINT rModuleDI16::loadFromXML(tinyxml2::XMLElement* element, rError& err)
{
	if (rIOBaseModule::loadFromXML(element, err) != TRITONN_RESULT_OK) {
		return err.getError();
	}

	XML_FOR(channel_xml, element, XmlName::CHANNEL) {
		USINT number = XmlUtils::getAttributeUSINT (channel_xml, XmlName::NUMBER, 0xFF);

		if (number >= CHANNEL_COUNT) {
			return err.set(DATACFGERR_IO_CHANNEL, channel_xml->GetLineNum(), "invalide number");
		}

		m_channel[number]->loadFromXML(channel_xml, err);

		if (err.getError()) {
			return err.getError();
		}
	}

	return TRITONN_RESULT_OK;
}

UDINT rModuleDI16::generateMarkDown(rGeneratorMD& md)
{
	md.add(this)
			.addRemark("[^simtype]: **Тип симуляции DI:**<br/>" + rIODIChannel::m_flagsSimType.getInfo(true) + "<br/>");

	return TRITONN_RESULT_OK;
}
