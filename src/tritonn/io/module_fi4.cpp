﻿//=================================================================================================
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
#include "tinyxml2.h"
#include "../error.h"
#include "../xml_util.h"
#include "../variable_class.h"
#include "../units.h"

rModuleFI4::rModuleFI4()
{
	while(m_channel.size() < CHANNEL_COUNT) {
		m_channel.push_back(rIOFIChannel(m_channel.size()));
	}

	m_type = Type::FI4;
}


UDINT rModuleFI4::processing(USINT issim)
{
	rIOBaseModule::processing(issim);

	for (auto& channel : m_channel) {
		if (channel.m_setup & rIOFIChannel::Setup::OFF) {
			continue;
		}

		if (issim) {
			channel.simulate();
		}

		channel.processing();
	}

	return TRITONN_RESULT_OK;
}


std::unique_ptr<rIOBaseChannel> rModuleFI4::getChannel(USINT num)
{
	if (num >= CHANNEL_COUNT) {
		return nullptr;
	}

	rLocker lock(m_mutex); UNUSED(lock);

	auto module_ptr = std::make_unique<rIOFIChannel>(m_channel[num]);

	return module_ptr;
}


UDINT rModuleFI4::generateVars(const std::string& prefix, rVariableList& list, bool issimulate)
{
	rIOBaseModule::generateVars(prefix, list, issimulate);

	for (UDINT ii = 0; ii < CHANNEL_COUNT; ++ii) {
		std::string p = prefix + m_name + ".ch_" + String_format("%02i", ii + 1);
		m_channel[ii].generateVars(p, list, issimulate);
	}

	std::string p = prefix + m_name;
	list.add(p + IO::VARNAME_OUTTYPE, TYPE_USINT, rVariable::Flags::RS__, &m_outtype, U_DIMLESS, ACCESS_SA);

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

		if (m_channel[number].loadFromXML(channel_xml, err) !=  TRITONN_RESULT_OK) {
			return err.getError();
		}
	}

	return TRITONN_RESULT_OK;
}
