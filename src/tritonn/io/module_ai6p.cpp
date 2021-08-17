﻿//=================================================================================================
//===
//=== module_ai6p.cpp
//===
//=== Copyright (c) 2019-2021 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================

#include "module_ai6p.h"
#include "locker.h"
#include "../units.h"
#include "../variable_list.h"
#include "xml_util.h"
#include "../error.h"
#include "../generator_md.h"

rModuleAI6p::rModuleAI6p(UDINT id) : rIOBaseModule(id)
{
	m_type    = Type::AI6p;
	m_comment = "Module 6 current/voltage passive channels";
	m_name    = "ai6p";

	while (m_channel.size() < CHANNEL_COUNT) {
		auto ch_ai = new rIOAIChannel(static_cast<USINT>(m_channel.size()), false);

		ch_ai->m_canIdx = m_channel.size();

		m_channel.push_back(ch_ai);
		m_listChannel.push_back(ch_ai);
	}

	memset(&m_data, 0, sizeof(m_data));

	setModule(&m_data, &m_data.ModuleInfo, &m_data.System, &m_data.Read.Status, _K19_AI6p_ModuleReadAll, _K19_AI6p_ModuleExchange);

#ifndef TRITONN_YOCTO
	#ifndef TRITONN_TEST
		m_channel[0]->m_simType  = rIOAIChannel::SimType::LINEAR;
		m_channel[0]->m_simSpeed = 1234;
	#endif
#endif
}

rModuleAI6p::rModuleAI6p(const rModuleAI6p* ai6p) : rIOBaseModule(ai6p)
{
	m_channel.clear();
	m_listChannel.clear();

	for (auto channel : ai6p->m_channel) {
		auto ch_ai = new rIOAIChannel(*channel);

		m_channel.push_back(ch_ai);
		m_listChannel.push_back(ch_ai);
	}
}

rModuleAI6p::~rModuleAI6p()
{
	for (auto channel : m_channel) {
		if (channel) {
			delete channel;
		}
	}
	m_channel.clear();
}


UDINT rModuleAI6p::processing(USINT issim)
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
			channel->m_ADC         = m_data.Read.Adc[idx];
			channel->m_current     = m_data.Read.Data[idx];
			channel->m_hardState   = m_data.Read.StateCh[idx]     != K19_AI6p_StateCh_Normal;
			channel->m_stateRedLED = m_data.Read.StateRedLED[idx] == K19_AI6p_StateRedLED_ON;

			m_data.Write.ChType[idx]       = getHardwareModuleChType(idx);
			m_data.Write.OutADCType[idx]   = K19_AI6p_OutType_TrueADC;
			m_data.Write.OutDataType[idx]  = K19_AI6p_OutType_ReducedData;
			m_data.Write.RedLEDAction[idx] = (channel->m_setup & rIOAIChannel::Setup::OFF) ? K19_AI6p_RedLEDBlocked : K19_AI6p_RedLEDNormal;
		}

		channel->processing();
	}
printf("AI6p[0] curr %.1f, type %i\n", m_data.Read.Data[0], m_data.Read.ChType[0]);
	return TRITONN_RESULT_OK;
}

UDINT rModuleAI6p::getPulling()
{
	rLocker lock(m_rwlock); lock.Nop();
	return m_pulling;
}

UDINT rModuleAI6p::getValue(USINT num, rIOBaseChannel::Type type, UDINT& fault)
{
	fault = checkChannelAccess(num, type);
	if (fault != TRITONN_RESULT_OK) {
		return true;
	}

	rLocker lock(m_rwlock); lock.Nop();

	return m_channel[num]->m_ADC;
}

UDINT rModuleAI6p::setValue(USINT num, rIOBaseChannel::Type type, UDINT value)
{
	UNUSED(num);
	UNUSED(type);
	UNUSED(value);

	return DATACFGERR_REALTIME_WRONGCHANNEL;
}

REAL rModuleAI6p::getCurrent(USINT num, rIOBaseChannel::Type type, UDINT& fault)
{
	fault = checkChannelAccess(num, type);
	if (fault != TRITONN_RESULT_OK) {
		return true;
	}

	rLocker lock(m_rwlock); lock.Nop();

	return m_channel[num]->m_current;
}

UINT rModuleAI6p::getMinValue(USINT num, rIOBaseChannel::Type type, UDINT& fault)
{
	fault = checkChannelAccess(num, type);
	if (fault != TRITONN_RESULT_OK) {
		return true;
	}

	rLocker lock(m_rwlock); lock.Nop();

	return m_channel[num]->getMinValue();
}

UINT rModuleAI6p::getMaxValue(USINT num, rIOBaseChannel::Type type, UDINT& fault)
{
	fault = checkChannelAccess(num, type);
	if (fault != TRITONN_RESULT_OK) {
		return true;
	}

	rLocker lock(m_rwlock); lock.Nop();

	return m_channel[num]->getMaxValue();
}

UINT rModuleAI6p::getRange(USINT num, rIOBaseChannel::Type type, UDINT& fault)
{
	fault = checkChannelAccess(num, type);
	if (fault != TRITONN_RESULT_OK) {
		return true;
	}

	rLocker lock(m_rwlock); lock.Nop();

	return m_channel[num]->getRange();
}

USINT rModuleAI6p::getState(USINT num, rIOBaseChannel::Type type, UDINT& fault)
{
	fault = checkChannelAccess(num, type);
	if (fault != TRITONN_RESULT_OK) {
		return true;
	}

	rLocker lock(m_rwlock); lock.Nop();

	return m_channel[num]->m_state;
}

UDINT rModuleAI6p::checkChannelAccess(USINT num, rIOBaseChannel::Type type)
{
	if (num >= CHANNEL_COUNT) {
		return DATACFGERR_REALTIME_CHANNELLINK;
	}

	if (m_channel[num]->m_type != type) {
		return DATACFGERR_REALTIME_WRONGCHANNEL;
	}

	return TRITONN_RESULT_OK;
}

K19_AI6p_ChType rModuleAI6p::getHardwareModuleChType(UDINT index)
{
	switch(m_channel[index]->m_mode) {
		case rIOAIChannel::Mode::mA_0_20:  return K19_AI6p_ChType_mA_0_20;
		case rIOAIChannel::Mode::mA_4_20:  return K19_AI6p_ChType_mA_4_20;
		case rIOAIChannel::Mode::V_m10_10: return K19_AI6p_ChType_V_m10_10;
		case rIOAIChannel::Mode::V_0_10:   return K19_AI6p_ChType_V_0_10;
		default: return m_data.Read.ChType[index];
	}
}

UDINT rModuleAI6p::generateVars(const std::string& prefix, rVariableList& list, bool issimulate)
{
	rIOBaseModule::generateVars(prefix, list, issimulate);

	std::string p = m_alias + ".";
	list.add(p + "temperature", rVariable::Flags::R___, &m_data.Read.Temp, U_C, 0, "Температура модуля в гр.С.");

	for (auto channel : m_channel) {
		std::string p = prefix + m_name + ".ch_" + String_format("%02i", channel->m_index);
		channel->generateVars(p, list, issimulate);
	}

	return TRITONN_RESULT_OK;
}

UDINT rModuleAI6p::loadFromXML(tinyxml2::XMLElement* element, rError& err)

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

UDINT rModuleAI6p::generateMarkDown(rGeneratorMD& md)
{
	md.add(this)
			.addRemark("[^simtype]: **Тип симуляции:**<br/>" + rIOAIChannel::m_flagsSimType.getInfo(true) + "</br>");

	return TRITONN_RESULT_OK;
}
