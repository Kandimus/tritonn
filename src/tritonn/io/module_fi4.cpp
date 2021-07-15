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

rModuleFI4::rModuleFI4(UDINT id) : rIOBaseModule(id)
{
	m_type    = Type::FI4;
	m_comment = "Module with 4 frequency input";
	m_name    = "fi4";

	while(m_channel.size() < CHANNEL_COUNT) {
		auto ch_fi = new rIOFIChannel(m_channel.size());
		m_channel.push_back(ch_fi);
		m_listChannel.push_back(ch_fi);
	}

	memset(&m_data, 0, sizeof(m_data));
	setModule(&m_data, &m_data.ModuleInfo, &m_data.System, _K19_FIO_ModuleReadAll, _K19_FIO_ModuleExchange);

#ifndef TRITONN_TEST
	m_channel[0]->m_simType  = rIOFIChannel::SimType::CONST;
	m_channel[0]->m_simValue = 5000;
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
	rLocker lock(m_rwlock); lock.Nop();

	rIOBaseModule::processing(issim);

	for (auto& channel : m_channel) {
		if (channel->m_setup & rIOFIChannel::Setup::OFF) {
			continue;
		}

		if (issim) {
			channel->simulate();
		}

		channel->processing();
	}

	return TRITONN_RESULT_OK;
}


rIOBaseChannel* rModuleFI4::getChannel(USINT num)
{
	if (num >= CHANNEL_COUNT) {
		return nullptr;
	}

	rLocker lock(m_rwlock); lock.Nop();

	return new rIOFIChannel(*m_channel[num]);
}


UDINT rModuleFI4::generateVars(const std::string& prefix, rVariableList& list, bool issimulate)
{
	rIOBaseModule::generateVars(prefix, list, issimulate);

	for (auto channel : m_channel) {
		std::string p = prefix + m_name + ".ch_" + String_format("%02i", channel->m_index);
		channel->generateVars(p, list, issimulate);
	}

	std::string p = prefix + m_name;
	list.add(p + IO::VARNAME_OUTTYPE, rVariable::Flags::RS__, &m_outtype, U_DIMLESS, ACCESS_SA, "Выбранный канал для коммутации с выходом");

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
