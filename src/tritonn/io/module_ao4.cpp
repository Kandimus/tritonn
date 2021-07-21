/*
 *
 * module_ao4.cpp
 *
 * Copyright (c) 2021 by RangeSoft.
 * All rights reserved.
 *
 * Litvinov "VeduN" Vitaliy O.
 *
 */

#include "module_ao4.h"
#include "locker.h"
#include "../units.h"
#include "../variable_list.h"
#include "xml_util.h"
#include "../error.h"
#include "../generator_md.h"

rModuleAO4::rModuleAO4(UDINT id) : rIOBaseModule(id)
{
	m_type    = Type::AO4;
	m_comment = "Module 4 output current active/passive channels";
	m_name    = "ao4";

	while (m_channel.size() < CHANNEL_COUNT) {
		auto ch_ao = new rIOAOChannel(static_cast<USINT>(m_channel.size()));

		ch_ao->m_canIdx = m_channel.size();

		m_channel.push_back(ch_ao);
		m_listChannel.push_back(ch_ao);
	}

	memset(&m_data, 0, sizeof(m_data));

	setModule(&m_data, &m_data.ModuleInfo, &m_data.System, &m_data.Read.Status, _K19_AO4_ModuleReadAll, _K19_AO4_ModuleExchange);
}

rModuleAO4::rModuleAO4(const rModuleAO4* ao4) : rIOBaseModule(ao4)
{
	for (auto channel : ao4->m_channel) {
		auto ch_ao = new rIOAOChannel(*channel);

		m_channel.push_back(ch_ao);
		m_listChannel.push_back(ch_ao);
	}
}

rModuleAO4::~rModuleAO4()
{
	for (auto channel : m_channel) {
		if (channel) {
			delete channel;
		}
	}
	m_channel.clear();
	m_listChannel.clear();
}


UDINT rModuleAO4::processing(USINT issim)
{
	rLocker lock(m_rwlock, rLocker::TYPELOCK::WRITE); lock.Nop();

	UDINT result = rIOBaseModule::processing(issim);
	if (result != TRITONN_RESULT_OK) {
		return result;
	}

	for (auto channel : m_channel) {
		USINT idx = channel->m_canIdx;

		if (issim) {
			channel->simulate();
		} else {
			channel->m_type = (m_data.Read.ChType[idx] == K19_AO4_CT_Active) ? rIOAOChannel::Type::ACTIVE : rIOAOChannel::Type::PASSIVE;

			m_data.Write.Data[idx]        = channel->m_ADC;
			m_data.Write.DataSetType[idx] = K19_AO4_DST_TrueUA;
		}

		channel->processing();
	}

	return TRITONN_RESULT_OK;
}

rIOBaseChannel* rModuleAO4::getChannel(USINT num)
{
	if (num >= CHANNEL_COUNT) {
		return nullptr;
	}

	rLocker lock(m_rwlock); lock.Nop();

	return new rIOAOChannel(*m_channel[num]);
}

UDINT rModuleAO4::generateVars(const std::string& prefix, rVariableList& list, bool issimulate)
{
	rIOBaseModule::generateVars(prefix, list, issimulate);

	for (auto channel : m_channel) {
		std::string p = prefix + m_name + ".ch_" + String_format("%02i", channel->m_index);
		channel->generateVars(p, list, issimulate);
	}

	return TRITONN_RESULT_OK;
}

UDINT rModuleAO4::loadFromXML(tinyxml2::XMLElement* element, rError& err)

{
	if (rIOBaseModule::loadFromXML(element, err) != TRITONN_RESULT_OK) {
		return err.getError();
	}

	XML_FOR(channel_xml, element, XmlName::CHANNEL) {
		USINT number = XmlUtils::getAttributeUSINT (channel_xml, XmlName::NUMBER, 0xFF);

		if (number >= CHANNEL_COUNT) {
			return err.set(DATACFGERR_IO_CHANNEL, channel_xml->GetLineNum(), "invalide number");
		}

		if (m_channel[number]->loadFromXML(channel_xml, err) != TRITONN_RESULT_OK) {
			return err.getError();
		}
	}

	return TRITONN_RESULT_OK;
}

UDINT rModuleAO4::generateMarkDown(rGeneratorMD& md)
{
	md.add(this);

	return TRITONN_RESULT_OK;
}
