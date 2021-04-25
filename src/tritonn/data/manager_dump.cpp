//=================================================================================================
//===
//=== data/manager_dump.cpp
//===
//=== Copyright (c) 2021 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================

#include "../data_manager.h"
#include <string.h>
#include "../data_source.h"
#include "log_manager.h"
#include "simplefile.h"
#include "../total.h"
#include "../variable_item.h"
#include "../event/manager.h"
#include "../xml_util.h"
#include "../xmlfile.h"

UDINT rDumpFile::checkFile(const std::string& filename, const std::string& hash)
{
	std::string strhash = "";

	m_filename = filename;
	m_result   = XMLDumpFile(m_filename, m_xmlDoc, strhash);

	if (m_result == FILE_RESULT_NOTFOUND) {
		m_xmlDoc.Clear();

		return m_result;
	}

	if (m_result != TRITONN_RESULT_OK) {
		SimpleFileDelete(m_filename);
		m_xmlDoc.Clear();

		rEventManager::instance().addEventUDINT(EID_SYSTEM_DUMPERROR, HALT_REASON_DUMP | result);
		TRACEP(LOG::DATAMGR, "Can't load dump file '%s'. Error ID: %i", m_filename.c_str(), result);

		return m_result;
	}

	if (strhash != hash) {
		return m_result = XMLFILE_RESULT_NOTEQUAL;
	}

	return m_result = TRITONN_RESULT_OK;
}


void rDataManager::generateDumpPrefixes()
{
	m_dumpVars.m_prefix = "<" + std::string(XmlName::VARIABLES) + " " + std::string(XmlName::HASH) + "=\"" + m_hashCfg + "\">";
	m_dumpVars.m_suffix = "</" + std::string(XmlName::VARIABLES) + ">";

	m_dumpTotals.m_prefix = "<" + std::string(XmlName::TOTALS) + " " + std::string(XmlName::HASH) + "=\"" + m_hashCfg + "\">";
	m_dumpTotals.m_suffix = "</" + std::string(XmlName::TOTALS) + ">";
}

UDINT rDataManager::saveDataVariables()
{
	std::string text = m_dumpVars.m_prefix;

	m_doSaveVars.Set(0);

	for (auto item : m_varList) {

		if (item->isDumped()) {
			text += item->valueToXml();
		}
	}

	text += m_dumpVars.m_suffix;

	UDINT result = SimpleFileSave(FILE_DUMP_VARIABLES, text);

	if (result != TRITONN_RESULT_OK) {
		rEventManager::instance().addEventUDINT(EID_SYSTEM_DUMPERROR, HALT_REASON_DUMP | result);

		DoHalt(HALT_REASON_DUMP | result);

		TRACEP(LOG::DATAMGR, "Can't save variable dump file. Error ID: %i", result);
	}

	return result;
}

viod rDataManager::loadDumps()
{
	if (Live::STARTING != GetLiveStatus()) {
		return;
	}

	// TOTALS
	m_dumpTotals.checkFile(FILE_DUMP_TOTALS, m_hashCfg);
	if (m_dumpTotals.getResult() == XMLFILE_RESULT_NOTEQUAL) {

		TRACEW(LOG::DATAMGR, "Hash in dump file '%s' is not qual to hash in config file.", FILE_DUMP_TOTALS.c_str());
		SetLiveStatus(Live::DUMP_TOTALS);
		return;
	}

	if (m_dumpTotals.getResult() != TRITONN_RESULT_OK && m_dumpTotals.getResult() != FILE_RESULT_NOTFOUND) {
		rEventManager::instance().addEventUDINT(EID_SYSTEM_DUMPERROR, HALT_REASON_DUMP | m_dumpVars.getResult());

		DoHalt(HALT_REASON_CONFIGFILE | m_dumpVars.getResult());

		TRACEP(LOG::DATAMGR, "Can't load dump file '%s'. Error ID: %i.", FILE_DUMP_TOTALS.c_str(), m_dumpVars.getResult());
	}

	// VARS
	m_dumpVars.checkFile(FILE_DUMP_VARIABLES, m_hashCfg);
	if (m_dumpVars.getResult() == XMLFILE_RESULT_NOTEQUAL) {

		TRACEW(LOG::DATAMGR, "Hash in dump file '%s' is not qual to hash in config file.", FILE_DUMP_VARIABLES.c_str());
		SetLiveStatus(Live::DUMP_VARS);
		return;
	}

	if (m_dumpVars.getResult() != TRITONN_RESULT_OK && m_dumpVars.getResult() != FILE_RESULT_NOTFOUND) {
		rEventManager::instance().addEventUDINT(EID_SYSTEM_DUMPERROR, HALT_REASON_DUMP | m_dumpVars.getResult());

		DoHalt(HALT_REASON_CONFIGFILE | m_dumpVars.getResult());

		TRACEP(LOG::DATAMGR, "Can't load dump file '%s'. Error ID: %i.", FILE_DUMP_VARIABLES.c_str(), m_dumpVars.getResult());
	}

	SetLiveStatus(Live::RUNNING);
}

UDINT rDataManager::saveDataTotals()
{
	std::string text = m_dumpTotals.m_prefix;

	for (auto item : m_listSource) {
		auto total = item->getTotal();

		if (!total) {
			continue;
		}

		text += total->toXml(item->m_alias);
	}

	text += m_dumpTotals.m_suffix;

	UDINT result = SimpleFileSave(FILE_DUMP_TOTALS, text);

	if (result != TRITONN_RESULT_OK) {
		rEventManager::instance().addEventUDINT(EID_SYSTEM_DUMPERROR, HALT_REASON_DUMP | result);

		DoHalt(HALT_REASON_DUMP | result);

		TRACEP(LOG::DATAMGR, "Can't save totals dump file. Error ID: %i", result);
	}

	return result;
}

UDINT rDataManager::loadDataVariables()
{
	auto root = m_dumpVars.xmlRoot();

	for (auto item : m_varList) {

		if (item->isDumped()) {
			item->valueFromXml(root);
		}
	}

	m_dumpVars.xmlClear();

	return TRITONN_RESULT_OK;
}

UDINT rDataManager::loadDataTotals()
{
	auto root = m_dumpTotals.xmlRoot();

	for (auto item : m_listSource) {
		auto total = item->getTotalNoConst();

		if (!total) {
			continue;
		}

		total->fromXml(root, item->m_alias);
	}

	m_dumpTotals.xmlClear();

	return TRITONN_RESULT_OK;
}
