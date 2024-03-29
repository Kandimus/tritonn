﻿/*
 *
 * module_do16.cpp
 *
 * Copyright (c) 2021 by RangeSoft.
 * All rights reserved.
 *
 * Litvinov "VeduN" Vitaliy O.
 *
 */

#include "module_do16.h"
#include "locker.h"
#include "xml_util.h"
#include "../error.h"
#include "../generator_md.h"

rModuleDO16::rModuleDO16(UDINT id) : rIOBaseModule(id)
{
	m_type    = Type::DO16;
	m_comment = "Module 16 discrete outputs";

	while(m_channel.size() < CHANNEL_COUNT) {
		auto ch_do = new rIODOChannel(static_cast<USINT>(m_channel.size()));

		ch_do->m_canIdx = m_channel.size();

		m_channel.push_back(ch_do);
		m_listChannel.push_back(ch_do);
	}

	memset(&m_data, 0, sizeof(m_data));
	setModule(&m_data, &m_data.ModuleInfo, &m_data.System, &m_data.Read.Status, _K19_DO16_ModuleReadAll, _K19_DO16_ModuleExchange);
}

rModuleDO16::rModuleDO16(const rModuleDO16* do16) : rIOBaseModule(do16)
{
	for (auto channel : do16->m_channel) {
		auto ch_do = new rIODOChannel(*channel);

		m_channel.push_back(ch_do);
		m_listChannel.push_back(ch_do);
	}
}

rModuleDO16::~rModuleDO16()
{
	for (auto channel : m_channel) {
		if (channel) {
			delete channel;
		}
	}
	m_channel.clear();
	m_listChannel.clear();
}

UDINT rModuleDO16::processing(USINT issim)
{
	rLocker lock(m_rwlock, rLocker::TYPELOCK::WRITE); lock.Nop();

	UDINT result = rIOBaseModule::processing(issim);
	if (result != TRITONN_RESULT_OK) {
		return result;
	}

	bool need_pulling = false;

	for (auto channel : m_channel) {
		USINT idx = channel->m_canIdx;

		if (issim) {
			need_pulling |= channel->simulate();
		} else {
			m_data.Write.Out[idx] = channel->m_value ? UL_K19_DO16_OutHigh : UL_K19_DO16_OutLow;
		}

		channel->processing();
	}

	if (need_pulling) {
		++m_pulling;
	}

	return TRITONN_RESULT_OK;
}

UDINT rModuleDO16::getPulling()
{
	rLocker lock(m_rwlock); lock.Nop();
	return m_pulling;
}

UDINT rModuleDO16::getValue(USINT num, rIOBaseChannel::Type type, UDINT& fault)
{
	if (num >= CHANNEL_COUNT) {
		fault = DATACFGERR_REALTIME_CHANNELLINK;
		return false;
	}

	rLocker lock(m_rwlock); lock.Nop();

	if (m_channel[num]->m_type != type) {
		fault = DATACFGERR_REALTIME_WRONGCHANNEL;
		return false;
	}

	return m_channel[num]->m_value;
}


UDINT rModuleDO16::setValue(USINT num, rIOBaseChannel::Type type, UDINT value)
{
	if (num >= CHANNEL_COUNT) {
		return DATACFGERR_REALTIME_CHANNELLINK;
	}

	rLocker lock(m_rwlock); lock.Nop();

	if (m_channel[num]->m_type != type) {
		return DATACFGERR_REALTIME_WRONGCHANNEL;
	}

	m_channel[num]->m_value = (value != 0);
	return TRITONN_RESULT_OK;
}

UDINT rModuleDO16::generateVars(const std::string& prefix, rVariableList& list, bool issimulate)
{
	rIOBaseModule::generateVars(prefix, list, issimulate);

	for (auto channel : m_listChannel) {
		std::string p = prefix + m_name + ".ch_" + String_format("%02i", channel->m_index);
		channel->generateVars(p, list, issimulate);
	}

	return TRITONN_RESULT_OK;
}


UDINT rModuleDO16::loadFromXML(tinyxml2::XMLElement* element, rError& err)
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

UDINT rModuleDO16::generateMarkDown(rGeneratorMD& md)
{
	md.add(this);

	return TRITONN_RESULT_OK;
}
