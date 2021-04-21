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

void rDataManager::generateDumpPrefixes()
{
	m_dumpVariablesPrefix  = "<" + std::string(XmlName::VARIABLES) + " ";
	m_dumpVariablesPrefix += std::string(XmlName::HASH) + "=\"" + m_hashCfg + "\">";

	m_dumpVariablesSuffix += "</" + std::string(XmlName::VARIABLES) + ">";

	m_dumpTotalsPrefix  = "<" + std::string(XmlName::TOTALS) + " ";
	m_dumpTotalsPrefix += std::string(XmlName::HASH) + "=\"" + m_hashCfg + "\">";

	m_dumpTotalsSuffix += "</" + std::string(XmlName::TOTALS) + ">";
}

UDINT rDataManager::saveDataVariables()
{
	std::string text = m_dumpVariablesPrefix;

	m_doSaveVars.Set(0);

	for (auto item : m_varList) {

		if (item->isDumped()) {
			text += item->valueToXml();
		}
	}

	text += m_dumpVariablesSuffix;

	UDINT result = SimpleFileSave(FILE_DUMP_VARIABLES, text);

	if (result != TRITONN_RESULT_OK) {
		rEventManager::instance().addEventUDINT(EID_SYSTEM_DUMPERROR, HALT_REASON_DUMP | result);

		DoHalt(HALT_REASON_DUMP | result);

		TRACEP(LOG::DATAMGR, "Can't save variable dump file. Error ID: %i", result);
	}

	return result;
}

UDINT rDataManager::saveDataTotals()
{
	std::string text = m_dumpTotalsPrefix;

	for (auto item : m_listSource) {
		auto total = item->getTotal();

		if (!total) {
			continue;
		}

		text += total->toXml(item->m_alias);
	}

	text += m_dumpTotalsSuffix;

	UDINT result = SimpleFileSave(FILE_DUMP_TOTALS, text);

	if (result != TRITONN_RESULT_OK) {
		rEventManager::instance().addEventUDINT(EID_SYSTEM_DUMPERROR, HALT_REASON_DUMP | result);

		DoHalt(HALT_REASON_DUMP | result);

		TRACEP(LOG::DATAMGR, "Can't save totals dump file. Error ID: %i", result);
	}

	return result;
}

UDINT rDataManager::loadDataTotals()
{
	tinyxml2::XMLDocument doc;
	std::string           strhash = "";

	UDINT result = XMLDumpFile(FILE_DUMP_TOTALS, doc, strhash);

	if (result != TRITONN_RESULT_OK) {
		SimpleFileDelete(FILE_DUMP_TOTALS);

		rEventManager::instance().addEventUDINT(EID_SYSTEM_DUMPERROR, HALT_REASON_DUMP | result);
		TRACEP(LOG::DATAMGR, "Can't load totals dump file. Error ID: %i", result);

		return result;
	}

	if (strhash != m_hashCfg) {
		return XMLFILE_RESULT_NOTEQUAL;
	}

	auto root = doc.RootElement();

	for (auto item : m_listSource) {
		auto total = item->getTotalNoConst();

		if (!total) {
			continue;
		}

		total->fromXml(root, item->m_alias);
	}

	return TRITONN_RESULT_OK;
}
