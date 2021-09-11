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

#include "module_fi4.h"
#include "defines.h"
#include "locker.h"
#include "xml_util.h"
#include "../error.h"
#include "../variable_class.h"
#include "../units.h"
#include "../generator_md.h"

rBitsArray rModuleFI4::m_flagsOutType;

rModuleFI4::rModuleFI4(UDINT id) : rIOBaseModule(id)
{
	m_type    = Type::FI4;
	m_comment = "Module with 4 frequency input";

	if (m_flagsOutType.empty()) {
		m_flagsOutType
				.add("", static_cast<USINT>(OutType::NONE)     , "выходной канал отключен")
				.add("", static_cast<USINT>(OutType::CHANNEL_1), "на выходной канал транслируется канал 1")
				.add("", static_cast<USINT>(OutType::CHANNEL_2), "на выходной канал транслируется канал 2")
				.add("", static_cast<USINT>(OutType::CHANNEL_3), "на выходной канал транслируется канал 3")
				.add("", static_cast<USINT>(OutType::CHANNEL_4), "на выходной канал транслируется канал 4");
	}

	while(m_channel.size() < CHANNEL_COUNT) {
		auto ch_fi = new rIOFIChannel(m_channel.size());

		ch_fi->m_canIdx = m_channel.size();

		m_channel.push_back(ch_fi);
		m_listChannel.push_back(ch_fi);
	}

	memset(&m_data, 0, sizeof(m_data));
	setModule(&m_data, &m_data.ModuleInfo, &m_data.System, &m_data.Read.Status, _K19_FIO_ModuleReadAll, _K19_FIO_ModuleExchange);

#ifndef TRITONN_YOCTO
	#ifndef TRITONN_TEST
		m_channel[0]->m_simType  = rIOFIChannel::SimType::CONST;
		m_channel[0]->m_simValue = 5000;
	#endif
#endif
}

rModuleFI4::rModuleFI4(const rModuleFI4* fi4) : rIOBaseModule(fi4)
{
	m_channel.clear();
	m_listChannel.clear();

	for (auto channel : fi4->m_channel) {
		auto ch_fi = new rIOFIChannel(*channel);

		m_channel.push_back(ch_fi);
		m_listChannel.push_back(ch_fi);
	}
}

rModuleFI4::~rModuleFI4()
{
	for (auto channel : m_channel) {
		if (channel) {
			delete channel;
		}
	}
	m_channel.clear();
}


UDINT rModuleFI4::processing(USINT issim)
{
	rLocker lock(m_rwlock, rLocker::TYPELOCK::WRITE); lock.Nop();

	UDINT result = rIOBaseModule::processing(issim);
	if (result != TRITONN_RESULT_OK) {
		return result;
	}

	bool need_pulling = false;

	for (auto& channel : m_channel) {
		USINT idx = channel->m_canIdx;

		if (issim) {
			need_pulling |= channel->simulate();
		} else {
			channel->m_freq    = m_data.Read.Frequency[idx];
			channel->m_counter = m_data.Read.Counter[idx];
			channel->m_filter  = m_data.Read.Filter[idx];
		}

		channel->processing();
	}

	m_data.Write.OutType = getOutType();

	if (need_pulling) {
		++m_pulling;
	}

	return TRITONN_RESULT_OK;
}


K19_FIO_OutType rModuleFI4::getOutType()
{
	switch(m_outtype) {
		case OutType::CHANNEL_1: return K19_FIO_OUT_CH1;
		case OutType::CHANNEL_2: return K19_FIO_OUT_CH2;
		case OutType::CHANNEL_3: return K19_FIO_OUT_CH3;
		case OutType::CHANNEL_4: return K19_FIO_OUT_CH4;
		default: return K19_FIO_OUT_NONE;
	}
}

UDINT rModuleFI4::getPulling()
{
	rLocker lock(m_rwlock); lock.Nop();
	return m_pulling;
}

UDINT rModuleFI4::getValue(USINT num, rIOBaseChannel::Type type, UDINT& fault)
{
	fault = checkChannelAccess(num, type);
	if (fault != TRITONN_RESULT_OK) {
		return true;
	}

	rLocker lock(m_rwlock); lock.Nop();

	return m_channel[num]->m_counter;
}

UDINT rModuleFI4::setValue(USINT num, rIOBaseChannel::Type type, UDINT  value)
{
	UNUSED(num);
	UNUSED(type);
	UNUSED(value);

	return DATACFGERR_REALTIME_WRONGCHANNEL;
}

LREAL rModuleFI4::getFreq(USINT num, rIOBaseChannel::Type type, UDINT& fault)
{
	fault = checkChannelAccess(num, type);
	if (fault != TRITONN_RESULT_OK) {
		return 0;
	}

	rLocker lock(m_rwlock); lock.Nop();

	return m_channel[num]->m_freq;
}

UDINT rModuleFI4::setOut(USINT num)
{
	if (num >= CHANNEL_COUNT) {
		return DATACFGERR_REALTIME_CHANNELLINK;
	}

	rLocker lock(m_rwlock); lock.Nop();

	m_outtype = static_cast<OutType>(num + 1);

	return TRITONN_RESULT_OK;
}

UDINT rModuleFI4::checkChannelAccess(USINT num, rIOBaseChannel::Type type)
{
	if (num >= CHANNEL_COUNT) {
		return DATACFGERR_REALTIME_CHANNELLINK;
	}

	if (m_channel[num]->m_type != type) {
		return DATACFGERR_REALTIME_WRONGCHANNEL;
	}

	return TRITONN_RESULT_OK;
}

UDINT rModuleFI4::generateVars(const std::string& prefix, rVariableList& list, bool issimulate)
{
	rIOBaseModule::generateVars(prefix, list, issimulate);

	std::string p = m_alias + ".";
	list.add(p + "out", TYPE::USINT, rVariable::Flags::____, &m_outtype, U_DIMLESS, 0, "Привязка выходной частоты:<br>" + m_flagsOutType.getInfo(true));

	for (auto channel : m_listChannel) {
		std::string p = prefix + m_name + ".ch_" + String_format("%02i", channel->m_index);
		channel->generateVars(p, list, issimulate);
	}

	return TRITONN_RESULT_OK;
}


UDINT rModuleFI4::loadFromXML(tinyxml2::XMLElement* element, rError& err)
{
	if (rIOBaseModule::loadFromXML(element, err) != TRITONN_RESULT_OK) {
		return err.getError();
	}

	XML_FOR(channel_xml, element, XmlName::CHANNEL) {
		USINT number = XmlUtils::getAttributeUSINT (channel_xml, XmlName::NUMBER, 0xFF);

		if (number >= CHANNEL_COUNT) {
			return err.set(DATACFGERR_IO_CHANNEL, channel_xml->GetLineNum(), "invalid module count");
		}

		if (m_channel[number]->loadFromXML(channel_xml, err) !=  TRITONN_RESULT_OK) {
			return err.getError();
		}
	}

	return TRITONN_RESULT_OK;
}

UDINT rModuleFI4::generateMarkDown(rGeneratorMD& md)
{
	md.add(this)
			.addRemark("[^simtype]: **Тип симуляции:**<br/>" + rIOFIChannel::m_flagsSimType.getInfo(true) + "<br/>");

	return TRITONN_RESULT_OK;
}
