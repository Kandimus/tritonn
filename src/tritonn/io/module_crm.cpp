/*
 *
 * io/module_crm.cpp
 *
 * Copyright (c) 2021 by RangeSoft.
 * All rights reserved.
 *
 * Litvinov "VeduN" Vitaliy O.
 *
 */

#include "module_crm.h"
#include "simpleargs.h"
#include "locker.h"
#include "xml_util.h"
#include "../def_arguments.h"
#include "../error.h"
#include "../generator_md.h"

rModuleCRM::rModuleCRM(UDINT id) : rIOBaseModule(id)
{
	m_type    = Type::CRM;
	m_comment = "Module of prove";
	m_name    = "crm";

	while(m_channelDI.size() < CHANNEL_DI_COUNT) {
		auto ch_di = new rIODIChannel(m_channelDI.size(), String_format("Детектор %u", m_channelDI.size() + 1));

		ch_di->m_canIdx = m_channelDI.size();

		m_channelDI.push_back(ch_di);
		m_listChannel.push_back(ch_di);
	}

	m_channelFI = new rIOFIChannel(m_channelDI.size(), "Частота с ПР");

	m_channelFI->m_canIdx = 0;

	m_listChannel.push_back(m_channelFI);

	memset(&m_data, 0, sizeof(m_data));
	setModule(&m_data, &m_data.ModuleInfo, &m_data.System, &m_data.Read.Status, _K19_CRM_ModuleReadAll, _K19_CRM_ModuleExchange);
}

rModuleCRM::rModuleCRM(const rModuleCRM* crm)  : rIOBaseModule(crm)
{
	m_channelDI.clear();
	m_listChannel.clear();

	for (auto channel : crm->m_channelDI) {
		auto ch_di = new rIODIChannel(*channel);

		m_channelDI.push_back(ch_di);
		m_listChannel.push_back(ch_di);
	}

	m_channelFI = new rIOFIChannel(*crm->m_channelFI);
	m_listChannel.push_back(m_channelFI);
}

rModuleCRM::~rModuleCRM()
{
	for (auto channel : m_channelDI) {
		if (channel) {
			delete channel;
		}
	}
	m_channelDI.clear();

	if (m_channelFI) {
		delete m_channelFI;
	}
	m_channelFI = nullptr;
}

UDINT rModuleCRM::processing(USINT issim)
{
	rLocker lock(m_rwlock, rLocker::TYPELOCK::WRITE); lock.Nop();

	rIOBaseModule::processing(issim);

	bool need_pulling = false;

	for (auto channel : m_channelDI) {
		USINT idx = channel->m_canIdx;

		if (channel->isOff()) {
			continue;
		}

		if (issim) {
			need_pulling |= channel->simulate();
		} else {
			switch(idx) {
				case 0: channel->m_phValue = m_data.Read.DIStat.Ch1Stat; break;
				case 1: channel->m_phValue = m_data.Read.DIStat.Ch2Stat; break;
				case 2: channel->m_phValue = m_data.Read.DIStat.Ch3Stat; break;
				case 3: channel->m_phValue = m_data.Read.DIStat.Ch4Stat; break;
			}
		}

		channel->processing();
	}

	if (!(m_channelFI->m_setup & rIOFIChannel::Setup::OFF)) {
		if (issim) {
			need_pulling |= m_channelFI->simulate();
		} else {
			m_channelFI->m_freq    = m_data.Read.Frequency;
			m_channelFI->m_filter  = m_data.Read.Filter;
			m_channelFI->m_counter = 0;
		}

		m_channelFI->processing();
	}

	if (need_pulling) {
		++m_pulling;
	}

	return TRITONN_RESULT_OK;
}

UDINT rModuleCRM::getPulling()
{
	rLocker lock(m_rwlock); lock.Nop();
	return m_pulling;
}

UDINT rModuleCRM::getValue(USINT num, rIOBaseChannel::Type type, UDINT& fault)
{
	if (num >= CHANNEL_DI_COUNT + 1) {
		fault = DATACFGERR_REALTIME_CHANNELLINK;
		return 0;
	}

	if (num < CHANNEL_DI_COUNT) {
		if (m_channelDI[num]->m_type != type) {
			fault = DATACFGERR_REALTIME_WRONGCHANNEL;
			return 0;
		}

		rLocker lock(m_rwlock); lock.Nop();

		return m_channelDI[num]->m_value;
	}

	if (m_channelFI->m_type != type) {
		fault = DATACFGERR_REALTIME_WRONGCHANNEL;
		return 0;
	}

	rLocker lock(m_rwlock); lock.Nop();

	return m_channelFI->m_counter;
}

UDINT rModuleCRM::setValue(USINT num, rIOBaseChannel::Type type, UDINT  value)
{
	UNUSED(num);
	UNUSED(type);
	UNUSED(value);

	return DATACFGERR_REALTIME_WRONGCHANNEL;
}

LREAL rModuleCRM::getFreq()
{
	rLocker lock(m_rwlock); lock.Nop();

	return m_channelFI->getFreq();
}

UINT rModuleCRM::getDetectors()
{
	rLocker lock(m_rwlock); lock.Nop();

	return ((m_channelDI[0]->getValue() != 0) << Detector::Det1) |
		   ((m_channelDI[1]->getValue() != 0) << Detector::Det2) |
		   ((m_channelDI[2]->getValue() != 0) << Detector::Det3) |
		   ((m_channelDI[3]->getValue() != 0) << Detector::Det4);
}

rIOCRMInterface::State rModuleCRM::getState(USINT idx)
{
	rLocker lock(m_rwlock); lock.Nop();
	return convertState((!idx) ? m_data.Read.MeasureState[0] : m_data.Read.MeasureState[1]);
}

LREAL rModuleCRM::getTime(USINT idx)
{
	rLocker lock(m_rwlock); lock.Nop();
	return (!idx) ? m_data.Read.MeasureTimeUs[0] : m_data.Read.MeasureTimeUs[1];
}

LREAL rModuleCRM::getImp(USINT idx)
{
	rLocker lock(m_rwlock); lock.Nop();
	return (!idx) ? m_data.Read.MeasureImpulseNmb[0] : m_data.Read.MeasureImpulseNmb[1];
}

bool rModuleCRM::start()
{
	if (rSimpleArgs::instance().isSet(rArg::Simulate)) {
		return true;
	}

	rLocker lock(m_rwlock); lock.Nop();

	if (m_moduleInfo->InWork) {
		sendCanCommand(_K19_CRM_MeasureStart, m_ID, m_dataPtr);
		return true;
	}

	return false;
}

bool rModuleCRM::abort()
{
	if (rSimpleArgs::instance().isSet(rArg::Simulate)) {
		return true;
	}

	rLocker lock(m_rwlock); lock.Nop();

	if (m_moduleInfo->InWork) {
		sendCanCommand(_K19_CRM_MeasureStop, m_ID, m_dataPtr);
		return true;
	}

	return false;
}

rIOCRMInterface::State rModuleCRM::convertState(USINT state)
{
	switch(state)
	{
		case K19_CRM_Meas_Idle:          return rIOCRMInterface::State::IDLE;
		case K19_CRM_Meas_WaitForFirst:  return rIOCRMInterface::State::WAIT_1;
		case K19_CRM_Meas_WaitForSecond: return rIOCRMInterface::State::WAIT_2;
		case K19_CRM_Meas_Finished:      return rIOCRMInterface::State::FINISH;
		case K19_CRM_Meas_Timeout:       return rIOCRMInterface::State::TIMEOUT;
		default:                         return rIOCRMInterface::State::IDLE;
	}
}

UDINT rModuleCRM::generateVars(const std::string& prefix, rVariableList& list, bool issimulate)
{
	rIOBaseModule::generateVars(prefix, list, issimulate);

	for (auto channel : m_listChannel) {
		std::string p = prefix + m_name + ".ch_" + String_format("%02i", channel->m_index + 1);
		channel->generateVars(p, list, issimulate);
	}

	return TRITONN_RESULT_OK;
}


UDINT rModuleCRM::loadFromXML(tinyxml2::XMLElement* element, rError& err)
{
	if (rIOBaseModule::loadFromXML(element, err) != TRITONN_RESULT_OK) {
		return err.getError();
	}

	XML_FOR(channel_xml, element, XmlName::CHANNEL) {
		USINT number = XmlUtils::getAttributeUSINT(channel_xml, XmlName::NUMBER, 0xFF);

		if (number > CHANNEL_DI_COUNT) {
			return err.set(DATACFGERR_IO_CHANNEL, channel_xml->GetLineNum(), "invalid module count");
		}

		if (number < CHANNEL_DI_COUNT) {
			m_channelDI[number]->loadFromXML(channel_xml, err);
		} else {
			m_channelFI->loadFromXML(channel_xml, err);
		}

		if (err.getError()) {
			return err.getError();
		}
	}

	return TRITONN_RESULT_OK;
}

UDINT rModuleCRM::generateMarkDown(rGeneratorMD& md)
{
	md.add(this)
			.addRemark("[^simtype]: **Тип симуляции DI:**<br/>" + rIODIChannel::m_flagsSimType.getInfo(true) +
					   "<br/>**Тип симуляции FI:**</br>" + rIOFIChannel::m_flagsSimType.getInfo(true));

	return TRITONN_RESULT_OK;
}
