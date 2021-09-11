﻿//=================================================================================================
//===
//=== module_ai6a.cpp
//===
//=== Copyright (c) 2019-2021 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================

#include "module_ai6a.h"
#include "locker.h"
#include "../units.h"
#include "../variable_list.h"
#include "xml_util.h"
#include "../error.h"
#include "../generator_md.h"

rBitsArray rModuleAI6a::m_flagsCorrect;

rModuleAI6a::rModuleAI6a(UDINT id) : rIOBaseModule(id)
{
	if (m_flagsCorrect.empty()) {
		m_flagsCorrect
				.add("", static_cast<UINT>(ModuleCorrectPoint::NONE)        , "Нет действий")
				.add("", static_cast<UINT>(ModuleCorrectPoint::SAVE_POINTS) , "Сохранить калибровки для всех каналов")
				.add("", static_cast<UINT>(ModuleCorrectPoint::CLEAR_POINTS), "Сбросить не сохранненые калибровки для всех каналов");
	}

	m_type    = Type::AI6a;
	m_comment = "Module 6 current/voltage active channels";

	while (m_channel.size() < CHANNEL_COUNT) {
		auto ch_ai = new rIOAIChannel(static_cast<USINT>(m_channel.size()), true);

		ch_ai->m_canIdx = m_channel.size();

		m_channel.push_back(ch_ai);
		m_listChannel.push_back(ch_ai);
	}

	memset(&m_data, 0, sizeof(m_data));

	setModule(&m_data, &m_data.ModuleInfo, &m_data.System, &m_data.Read.Status, _K19_AI6a_ModuleReadAll, _K19_AI6a_ModuleExchange);

#ifndef TRITONN_YOCTO
	#ifndef TRITONN_TEST
		m_channel[0]->m_simType  = rIOAIChannel::SimType::LINEAR;
		m_channel[0]->m_simSpeed = 1234;
	#endif
#endif
}

rModuleAI6a::rModuleAI6a(const rModuleAI6a* ai6a) : rIOBaseModule(ai6a)
{
	m_channel.clear();
	m_listChannel.clear();

	for (auto channel : ai6a->m_channel) {
		auto ch_ai = new rIOAIChannel(*channel);

		m_channel.push_back(ch_ai);
		m_listChannel.push_back(ch_ai);
	}
}

rModuleAI6a::~rModuleAI6a()
{
	for (auto channel : m_channel) {
		if (channel) {
			delete channel;
		}
	}
	m_channel.clear();
}


UDINT rModuleAI6a::processing(USINT issim)
{
	rLocker lock(m_rwlock, rLocker::TYPELOCK::WRITE); lock.Nop();

	UDINT result = rIOBaseModule::processing(issim);
	if (result != TRITONN_RESULT_OK) {
		return result;
	}

	bool need_pulling = false;

	for (auto channel : m_channel) {
		USINT idx = channel->m_index;

		if (issim) {
			need_pulling |= channel->simulate();
		} else {
			channel->m_ADC         = m_data.Read.Adc[idx];
			channel->m_current     = m_data.Read.Data[idx];
			channel->m_hardState   = m_data.Read.StateCh[idx]     != K19_AI6a_StateCh_Normal;
			channel->m_stateRedLED = m_data.Read.StateRedLED[idx] == K19_AI6a_StateRedLED_ON;

			m_data.Write.ChType[idx]       = getHardwareModuleChType(idx);
			m_data.Write.OutADCType[idx]   = K19_AI6a_OutType_ReducedADC;
			m_data.Write.OutDataType[idx]  = K19_AI6a_OutType_ReducedData;
			m_data.Write.RedLEDAction[idx] = (channel->m_setup & rIOAIChannel::Setup::OFF) ? K19_AI6a_RedLEDBlocked : K19_AI6a_RedLEDNormal;

			checkCorrectPoint(idx);
		}

		channel->processing();
	}

	if (need_pulling) {
		++m_pulling;
	}

	return TRITONN_RESULT_OK;
}

UDINT rModuleAI6a::getPulling()
{
	rLocker lock(m_rwlock); lock.Nop();
	return m_pulling;
}

UDINT rModuleAI6a::getValue(USINT num, rIOBaseChannel::Type type, UDINT& fault)
{
	fault = checkChannelAccess(num, type);
	if (fault != TRITONN_RESULT_OK) {
		return true;
	}

	rLocker lock(m_rwlock); lock.Nop();

	return m_channel[num]->m_ADC;
}

UDINT rModuleAI6a::setValue(USINT num, rIOBaseChannel::Type type, UDINT value)
{
	UNUSED(num);
	UNUSED(type);
	UNUSED(value);

	return DATACFGERR_REALTIME_WRONGCHANNEL;
}

REAL rModuleAI6a::getCurrent(USINT num, rIOBaseChannel::Type type, UDINT& fault)
{
	fault = checkChannelAccess(num, type);
	if (fault != TRITONN_RESULT_OK) {
		return true;
	}

	rLocker lock(m_rwlock); lock.Nop();

	return m_channel[num]->m_current;
}

UINT rModuleAI6a::getMinValue(USINT num, rIOBaseChannel::Type type, UDINT& fault)
{
	fault = checkChannelAccess(num, type);
	if (fault != TRITONN_RESULT_OK) {
		return true;
	}

	rLocker lock(m_rwlock); lock.Nop();

	return m_channel[num]->getMinValue();
}

UINT rModuleAI6a::getMaxValue(USINT num, rIOBaseChannel::Type type, UDINT& fault)
{
	fault = checkChannelAccess(num, type);
	if (fault != TRITONN_RESULT_OK) {
		return true;
	}

	rLocker lock(m_rwlock); lock.Nop();

	return m_channel[num]->getMaxValue();
}

UINT rModuleAI6a::getRange(USINT num, rIOBaseChannel::Type type, UDINT& fault)
{
	fault = checkChannelAccess(num, type);
	if (fault != TRITONN_RESULT_OK) {
		return true;
	}

	rLocker lock(m_rwlock); lock.Nop();

	return m_channel[num]->getRange();
}

USINT rModuleAI6a::getState(USINT num, rIOBaseChannel::Type type, UDINT& fault)
{
	fault = checkChannelAccess(num, type);
	if (fault != TRITONN_RESULT_OK) {
		return true;
	}

	rLocker lock(m_rwlock); lock.Nop();

	return m_channel[num]->m_state;
}

UDINT rModuleAI6a::checkChannelAccess(USINT num, rIOBaseChannel::Type type)
{
	if (num >= CHANNEL_COUNT) {
		return DATACFGERR_REALTIME_CHANNELLINK;
	}

	if (m_channel[num]->m_type != type) {
		return DATACFGERR_REALTIME_WRONGCHANNEL;
	}

	return TRITONN_RESULT_OK;
}

K19_AI6a_ChType rModuleAI6a::getHardwareModuleChType(UDINT index)
{
	switch(m_channel[index]->m_mode) {
		case rIOAIChannel::Mode::mA_0_20: return K19_AI6a_ChType_mA_0_20;
		case rIOAIChannel::Mode::mA_4_20: return K19_AI6a_ChType_mA_4_20;
		default: return m_data.Read.ChType[index];
	}
}

void rModuleAI6a::checkCorrectPoint(USINT idx)
{
	KXX_str setting;

	switch (m_channel[idx]->m_correct) {
		case rIOAIChannel::CorrectPoint::POINT_4mA:
			setting.ch  = idx;
			setting.dot = 0;
			sendCanCommand(_K19_AI6a_CorrSetDot, m_ID, &setting);
			m_channel[idx]->m_correct = rIOAIChannel::CorrectPoint::NONE;
			break;

		case rIOAIChannel::CorrectPoint::POINT_12mA:
			setting.ch  = idx;
			setting.dot = 1;
			sendCanCommand(_K19_AI6a_CorrSetDot, m_ID, &setting);
			m_channel[idx]->m_correct = rIOAIChannel::CorrectPoint::NONE;
			break;

		case rIOAIChannel::CorrectPoint::POINT_20mA:
			setting.ch  = idx;
			setting.dot = 2;
			sendCanCommand(_K19_AI6a_CorrSetDot, m_ID, &setting);
			m_channel[idx]->m_correct = rIOAIChannel::CorrectPoint::NONE;
			break;

		default:
			m_channel[idx]->m_correct = rIOAIChannel::CorrectPoint::NONE;
			break;
	}

	switch (m_correct) {
		case ModuleCorrectPoint::SAVE_POINTS:
			sendCanCommand(_K19_AI6a_CorrSave, m_ID, &m_data);
			m_correct = ModuleCorrectPoint::NONE;
			break;

		case ModuleCorrectPoint::CLEAR_POINTS:
			sendCanCommand(_K19_AI6a_CorrClear, m_ID, &m_data);
			m_correct = ModuleCorrectPoint::NONE;
			break;

		default:
			m_correct = ModuleCorrectPoint::NONE;
			break;
	}
}

UDINT rModuleAI6a::generateVars(const std::string& prefix, rVariableList& list, bool issimulate)
{
	rIOBaseModule::generateVars(prefix, list, issimulate);

	std::string p = m_alias + ".";
	list.add(p + "temperature",             rVariable::Flags::R___, &m_data.Read.Temp, U_C      ,             0, "Температура модуля в гр.С.");
	list.add(p + "correct"    , TYPE::UINT ,rVariable::Flags::____, &m_correct       , U_DIMLESS, ACCESS_SCALES, "Калибровка каналов:<br/>" + m_flagsCorrect.getInfo(true));

	for (auto channel : m_channel) {
		std::string p = prefix + m_name + ".ch_" + String_format("%02i", channel->m_index);
		channel->generateVars(p, list, issimulate);
	}

	return TRITONN_RESULT_OK;
}

UDINT rModuleAI6a::loadFromXML(tinyxml2::XMLElement* element, rError& err)

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

UDINT rModuleAI6a::generateMarkDown(rGeneratorMD& md)
{
	md.add(this)
			.addRemark("[^simtype]: **Тип симуляции:**<br/>" + rIOAIChannel::m_flagsSimType.getInfo(true) + "</br>");

	return TRITONN_RESULT_OK;
}
