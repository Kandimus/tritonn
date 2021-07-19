//=================================================================================================
//===
//=== io_ai.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс аналового входног сигнала модуля CAN (AI)
//===
//=================================================================================================

#include "module_di8do8.h"
#include "locker.h"
#include "xml_util.h"
#include "../error.h"
#include "../generator_md.h"

rBitsArray rModuleDI8DO8::m_flagsDOSetup;

rModuleDI8DO8::rModuleDI8DO8(UDINT id) : rIOBaseModule(id)
{
	m_type    = Type::DI8DO8;
	m_comment = "Module with 8 discrete input and 8 discrete output";
	m_name    = "di8do8";

	while(m_channelDI.size() < CHANNEL_DI_COUNT) {
		auto ch_di = new rIODIChannel(static_cast<USINT>(m_channelDI.size()));
		m_channelDI.push_back(ch_di);
		m_listChannel.push_back(ch_di);
	}

	while(m_channelDO.size() < CHANNEL_DO_COUNT) {
		auto ch_do = new rIODOChannel(static_cast<USINT>(CHANNEL_DI_COUNT + m_channelDO.size()));
		m_channelDO.push_back(ch_do);
		m_listChannel.push_back(ch_do);
	}

	memset(&m_data, 0, sizeof(m_data));
	setModule(&m_data, &m_data.ModuleInfo, &m_data.System, _K19_DIDO8_ModuleReadAll, _K19_DIDO8_ModuleExchange);
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
	for (auto channel : m_listChannel) {
		if (channel) {
			delete channel;
		}
	}
	m_channelDI.clear();
	m_channelDO.clear();
}

UDINT rModuleDI8DO8::processing(USINT issim)
{
	rLocker lock(m_rwlock); lock.Nop();

	UDINT result = rIOBaseModule::processing(issim);
	if (result != TRITONN_RESULT_OK) {
		return result;
	}

	for (auto channel : m_channelDI) {
		USINT idx = channel->m_index;

		if (issim) {
			channel->simulate();
		} else {
			channel->m_phValue = m_data.Read.DI[idx] == UL_K19_DIDO8_ChStHigh;
		}

		channel->processing();
	}

	for (auto channel : m_channelDO) {
		USINT idx = channel->m_index;

		if (issim) {
			channel->simulate();
		} else {
			m_data.Write.DO[idx] = channel->m_value ? UL_K19_DIDO8_ChStHigh : UL_K19_DIDO8_ChStLow;
		}

		channel->processing();
	}

	m_data.Write.DIFilter = 0;

	static int ttt = 0;
	static int aaa = 1;
	if (++ttt > 20) {
		m_data.Write.DO[0] = aaa ? UL_K19_DIDO8_ChStHigh : UL_K19_DIDO8_ChStLow;
		aaa = !aaa;
		ttt = 0;
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
