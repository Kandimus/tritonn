//=================================================================================================
//===
//=== data_source.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс-интерфейс для всех объектов, которые могут выступать в роли источника данных
//===
//=================================================================================================

#include "source.h"
#include <limits>
#include "xml_util.h"
#include "link.h"
#include "../data_config.h"
#include "../event/manager.h"
#include "../variable_list.h"
#include "../text_manager.h"
#include "../text_manager.h"
#include "../error.h"
#include "../generator_md.h"


rSource::rSource(const rStation* owner)
{
	m_station = owner;
}

rSource::~rSource()
{
	m_inputs.clear();
	m_outputs.clear();
}


//-------------------------------------------------------------------------------------------------
// Поиск выходного линка по его имени
rLink* rSource::getOutputByName(const std::string& name)
{
	if(m_outputs.empty()) {
		return nullptr;
	}

	if(name.empty()) {
		return m_outputs[0];
	}

	for(auto item : m_outputs) {
		if(name == item->m_ioName) {
			return item;
		}
	}

	return nullptr;
}


//-------------------------------------------------------------------------------------------------
// Выдача значения требуемого "выхода", с приведением к требуемым ед. измерения
LREAL rSource::getValue(const std::string& name, UDINT unit, UDINT& err)
{
	LREAL  result = 0.0;
	rLink* link   = nullptr;

	// Если значение переменных еще не вычислено, то вначале вычисляем объект, потом возращаем значение
	if (!m_isCalc) {
		calculate();
	}

	if(name == XmlName::FAULT) return (LREAL)m_fault;

	link = getOutputByName(name);

	if (!link) {
		err = 1;

		//TODO NOTE Должны ли мы в этом случаее уйти в SERVICE
		sendEventSetLE(SOURCE_LE_OUTPUT, m_event.reinit(EID_SYSTEM_ERROUTVAL) << m_ID << m_descr << STRID(unit));

		return std::numeric_limits<LREAL>::quiet_NaN();
	}

	// Проводим к нужному типу
	err = rUnits::ConvertValue(link->m_value, link->m_unit, result, unit);

	if(err) {
		sendEventSetLE(SOURCE_LE_UNIT, m_event.reinit(EID_SYSTEM_ERRUNIT) << m_ID << m_descr << STRID(link->m_unit) << STRID(unit));
	}

	return result;
}


//-------------------------------------------------------------------------------------------------
// Выдаем ед.измерения требуемого выхода. Реализовано для отчетов
STRID rSource::getValueUnit(const std::string& name, UDINT& err)
{
	auto link = getOutputByName(name);

	if(!link) {
		err = 1;

		//TODO NOTE Должны ли мы в этом случаее уйти в SERVICE
		sendEventSetLE(SOURCE_LE_OUTPUT, m_event.reinit(EID_SYSTEM_ERROUTPUT) << m_ID << m_descr);

		return 0xFFFFFFFF;
	}

	return link->m_unit;
}


//-------------------------------------------------------------------------------------------------
UDINT rSource::getFault()
{
	return m_fault;
}


UDINT rSource::preCalculate()
{
	m_isCalc = false;

	return TRITONN_RESULT_OK;
}


UDINT rSource::calculate()
{
	if (m_isCalc) {
		return 1;
	}

	m_isCalc = true;
	m_fault  = 0;

	for (auto link : m_inputs) {
		link->calculate();
	}

	return 0;
}



UDINT rSource::postCalculate()
{
	for (auto link : m_outputs) {
		if (link->m_setup & rLink::Setup::INPUT) {
			continue;
		}

		link->calculateLimit();
	}

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
// Переинициализируем временный Event, записывая ID объекта и строку с его описанием
rEvent& rSource::reinitEvent(rEvent& event, UDINT eid)
{
	event.reinit(eid) << m_ID << m_descr;

	return event;
}

//-------------------------------------------------------------------------------------------------
// Переинициализируем временный Event, записывая ID объекта и строку с его описанием
rEvent& rSource::reinitEvent(UDINT eid)
{
	return reinitEvent(m_event, eid);
}


//-------------------------------------------------------------------------------------------------
//
UDINT rSource::initLink(UINT setup, rLink& link, UDINT unit, STRID nameid, const std::string& name, const std::string& shadow)
{
	link.init(setup, unit, this, name, nameid);

	link.m_shadow = shadow;

	if(setup & rLink::Setup::INPUT ) m_inputs.push_back (&link);
	if(setup & rLink::Setup::OUTPUT) m_outputs.push_back(&link);

	// Вызываем функцию конечного класса
	initLimitEvent(link);

	return TRITONN_RESULT_OK;
}


UDINT rSource::reinitLimitEvents()
{
	for (auto link : m_inputs) {
		initLimitEvent(*link);
	}

	for (auto link : m_outputs) {
		if (!(link->m_setup & rLink::Setup::INPUT)) {
			initLimitEvent(*link);
		}
	}

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rSource::generateVars(rVariableList& list)
{
	for (auto link : m_inputs) {
		link->generateVars(list);
	}

	for (auto link : m_outputs) {
		if (link->m_setup & rLink::Setup::INPUT) {
			continue;
		}

		link->generateVars(list);
	}

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rSource::loadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix)
{
	if (!element) {
		return err.set(DATACFGERR_CONFIG, 0, "element is null");
	}

	m_lineNum = element->GetLineNum();

	auto strAlias = element->Attribute(XmlName::NAME);

	//TODO Можно еще алиас проверить на валидность имени
	if (!strAlias) {
		return err.set(DATACFGERR_INVALID_NAME, m_lineNum);
	}

	if (prefix.size()) {
		m_alias = prefix + ".";
	}

	m_alias += strAlias;
	m_alias  = String_tolower(m_alias);
	m_descr  = XmlUtils::getAttributeUDINT(element, XmlName::DESC, 0);

	// Загружаем все пределы по всем входам и выходам
	tinyxml2::XMLElement* limits = element->FirstChildElement(XmlName::LIMITS);

	if (nullptr == limits) {
		return TRITONN_RESULT_OK;
	}

	XML_FOR(limit_xml, limits, XmlName::LIMIT) {
		rLink*      link   = nullptr;
		std::string ioname = String_tolower(limit_xml->Attribute(XmlName::NAME));

		for (auto item : m_inputs) {
			if (item->m_ioName == ioname) {
				link = item;
				break;
			}
		}

		if (!link) {
			for (auto item : m_outputs) {
				if (item->m_ioName == ioname) {
					link = item;
					break;
				}
			}
		}

		if (!link) {
			return err.set(DATACFGERR_LIMIT, limit_xml->GetLineNum(), ioname);
		}

		link->m_limit.loadFromXML(limit_xml, err, prefix);
	}

	return TRITONN_RESULT_OK;
}


UDINT rSource::check(rError& err)
{
	UNUSED(err);
	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
std::string rSource::saveKernel(UDINT isio, const string &objname, const string &comment, UDINT isglobal)
{
	const std::string Tag[2] = {"object", "io"};
	std::string result = "";
	rVariableList list;

	generateVars(list);

	result += String_format("<!--\n\t%s\n-->\n"
							"<%s name=\"%s\"",
							comment.c_str(),
							Tag[isio ? 1 : 0].c_str(), objname.c_str());

	if (!isio) {
		result += String_format(" global=\"%s\"", (isglobal ? "true" : "false"));
	}
	result += ">\n\t<values>\n";

	result += "\t</values>\n";

	// Входа
	if (m_inputs.size()) {

		result += "\t<inputs>\n";

		for (auto link : m_inputs) {
			UDINT shadow_count = 0;

			result += String_format("\t\t<input name=\"%s\" unit=\"%i\"", link->m_ioName.c_str(), (UDINT)link->m_unit);

			for (auto sublink : m_inputs) {
				if (link == sublink) {
					continue;
				}

				if (sublink->m_shadow == link->m_ioName) {
					if (0 == shadow_count) {
						result += ">\n";
					}

					result += String_format("\t\t\t<shadow name=\"%s\"/>\n", sublink->m_ioName.c_str());
					++shadow_count;
				}
			}

			if (0 == shadow_count) {
				if (link->m_shadow.size()) {
					result += String_format(" shadow=\"%s\"", link->m_shadow.c_str());
				}
				result += "/>\n";
			}
			else
			{
				result += "\t\t</input>\n";
			}
		}
		result += "\t</inputs>\n";
	}

	// Outputs
	if (m_outputs.size()) {
		result += "\t<outputs>\n";

		for (auto link : m_outputs) {
			result += String_format("\t\t<output name=\"%s\" unit=\"%i\"%s/>\n",
					link->m_ioName.c_str(), (UDINT)link->m_unit, (link == m_outputs[0]) ? " default=\"1\"" : "");
		}

		result += "\t\t<output name=\"fault\" unit=\"512\"/>\n"
				  "\t</outputs>\n";
	}

	result += String_format("</%s>\n", Tag[isio ? 1 : 0].c_str());

	return result;
}

UDINT rSource::generateMarkDown(rGeneratorMD& md)
{
	UNUSED(md);

	return TRITONN_RESULT_OK;
}

std::string rSource::getMarkDown()
{
	std::string result = "";

	if (m_inputs.size()) {
		result += "\n## Inputs\n";
		result += "Input | Unit | Unit ID | Limits | Shadow | Comment\n";
		result += ":-- |:--:|:--:|:--:|:--:|:--\n";

		for (auto link : m_inputs) {
			std::string strunit = "";

			rTextManager::instance().get(link->m_unit, strunit);

			result += link->m_ioName + " | ";
			result += strunit + " | " + String_format("%u", static_cast<UDINT>(link->m_unit)) + " | ";
			result += link->m_limit.m_flagsSetup.getNameByBits(link->m_limit.m_setup.Value, ", ") + " | ";
			result += link->m_shadow + " | ";
			result += link->m_comment + "\n";
		}
	}

	if (m_outputs.size()) {
		result += "\n## Outputs\n";
		result += "Output | Unit | Unit ID | Limits | Comment\n";
		result += ":-- |:--:|:--:|:--:|:--\n";
		for (auto link : m_outputs) {
			std::string strunit = "";

			rTextManager::instance().get(link->m_unit, strunit);

			result += link->m_ioName + " | ";
			result += strunit + " | " + String_format("%u", static_cast<UDINT>(link->m_unit)) + " | ";

			result += link->m_limit.m_flagsSetup.getNameByBits(link->m_limit.m_setup.Value, ", ") + " | ";
			result += link->m_comment + "\n";
		}
	}

	rVariableList list;
	generateVars(list);

	result += list.getMarkDown();

	return result;
}

std::string rSource::getXmlLimits(const std::string& prefix) const
{
	std::string result = "";

	for (auto link : m_inputs) {
		result += link->m_limit.getXML(link->m_ioName, "\t" + prefix);
	}

	for (auto link : m_outputs) {
		if (link->m_setup & rLink::Setup::INPUT) {
			continue;
		}

		result += link->m_limit.getXML(link->m_ioName, "\t" + prefix);
	}

	if (result.size()) {
		result = std::string("<") + XmlName::LIMITS + ">" + rGeneratorMD::rItem::XML_OPTIONAL + "\n" +
				result +
				prefix + "</" + XmlName::LIMITS + ">\n";
	}

	return result;
}

std::string rSource::getXmlInput() const
{
	std::string result = "";

	if (m_inputs.size()) {
		for (auto link : m_inputs) {
			result += "\t<" + link->m_ioName + ">" + rGeneratorMD::rItem::XML_LINK + "</" + link->m_ioName + ">";

			if (link->m_shadow.size() || (link->m_limit.m_setup.Value & rLimit::Setup::OPTIONAL)) {
				result += "<!-- Optional -->";
			}

			result += "\n";
		}
	}

	result += "\t" + getXmlLimits("\t");

	return result;
}

bool rSource::checkOutput(const string &name)
{
	string lowname = String_tolower(name);

	if(XmlName::FAULT == lowname) return true;

	for(auto link : m_outputs)
	{
		if(lowname == link->m_ioName) return true;
	}

	return false;
}



// Функция проверяет наличие флага по условию, блокирует этот флаг в переменной LockErr
// и выдает сообщение
UDINT rSource::checkExpr(bool expr, UDINT flag, rEvent& event_fault, rEvent& event_success)
{
	// Проверка полученных данных
	if (expr) {
		if (!(m_lockErr & flag)) {
			m_lockErr |= flag;

			rEventManager::instance().add(event_fault);
		}

		return 1;
	} else {
		if (m_lockErr & flag) {
			m_lockErr &= ~flag;

			rEventManager::instance().add(event_success);
		}
	}

	return 0;
}


UDINT rSource::sendEventSetLE(UDINT flag, rEvent &event)
{
	if (!(m_lockErr & flag)) {
		rEventManager::instance().add(event);

		m_lockErr |= flag;

		return 1;
	}

	return 0;
}


UDINT rSource::sendEventClearLE(UDINT flag, rEvent &event)
{
	if (m_lockErr & flag) {
		rEventManager::instance().add(event);
		m_lockErr &= ~flag;

		return 1;
	}

	return 0;
}

