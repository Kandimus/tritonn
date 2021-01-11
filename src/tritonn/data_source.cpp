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

#include <limits>
#include "def.h"
#include "tinyxml2.h"
#include "xml_util.h"
#include "data_link.h"
#include "data_config.h"
#include "event_manager.h"
#include "variable_item.h"
#include "variable_list.h"
#include "data_source.h"
#include "text_manager.h"
#include "xml_util.h"
#include "text_manager.h"
#include "error.h"



rSource::rSource()
{
	Alias      = "";
	Descr      = 0;
	ID         = 0;
	Calculated = 0;
	Fault      = 0;
	LockErr    = 0;
	Station    = nullptr;
}

rSource::~rSource()
{
	m_inputs.clear();
	m_outputs.clear();
}


//-------------------------------------------------------------------------------------------------
// Поиск выходного линка по его имени
rLink *rSource::GetOutputByName(const string &name)
{
	if(m_outputs.empty()) {
		return nullptr;
	}

	if(name.empty()) {
		return m_outputs[0];
	}

	for(auto item : m_outputs) {
		if(name == item->IO_Name) {
			return item;
		}
	}

	return nullptr;
}


//-------------------------------------------------------------------------------------------------
// Выдача значения требуемого "выхода", с приведением к требуемым ед. измерения
LREAL rSource::GetValue(const string &name, UDINT unit, UDINT &err)
{
	LREAL  result = 0.0;
	rLink *link   = nullptr;

	// Если значение переменных еще не вычислено, то вначале вычисляем объект, потом возращаем значение
	if(!Calculated)
	{
		Calculate();
	}

	if(name == XmlName::FAULT) return (LREAL)Fault;

	link = GetOutputByName(name);

	if(nullptr == link)
	{
		err = 1;

		//TODO NOTE Должны ли мы в этом случаее уйти в SERVICE
		SendEventSetLE(SOURCE_LE_OUTPUT, Event.Reinit(EID_SYSTEM_ERROUTVAL) << ID << Descr << STRID(unit));

		return std::numeric_limits<LREAL>::quiet_NaN();
	}

	// Проводим к нужному типу
	err = rUnits::ConvertValue(link->Value, link->Unit, result, unit);

	if(err)
	{
		SendEventSetLE(SOURCE_LE_UNIT, Event.Reinit(EID_SYSTEM_ERRUNIT) <<ID << Descr << STRID(link->Unit) << STRID(unit));
	}

	return result;
}


//-------------------------------------------------------------------------------------------------
// Выдаем ед.измерения требуемого выхода. Реализовано для отчетов
STRID rSource::GetValueUnit(const string &name, UDINT &err)
{
	rLink *link = GetOutputByName(name);

	if(nullptr == link)
	{
		err = 1;

		//TODO NOTE Должны ли мы в этом случаее уйти в SERVICE
		SendEventSetLE(SOURCE_LE_OUTPUT, Event.Reinit(EID_SYSTEM_ERROUTPUT) << ID << Descr);

		return 0xFFFFFFFF;
	}

	return link->Unit;
}


//-------------------------------------------------------------------------------------------------
UDINT rSource::GetFault()
{
	return Fault;
}


UDINT rSource::PreCalculate()
{
	Calculated = 0;

	return 0;
}


UDINT rSource::Calculate()
{
	if(Calculated) return 1;

	Calculated = 1;
	Fault      = 0;

	for (auto link : m_inputs) {
		link->Calculate();
	}

	return 0;
}



UDINT rSource::PostCalculate()
{
	for (auto link : m_outputs) {
		if (link->m_setup & rLink::Setup::INPUT) {
			continue;
		}

		link->CalculateLimit();
	}

	return 0;
}


//-------------------------------------------------------------------------------------------------
// Переинициализируем временный Event, записывая ID объекта и строку с его описанием
rEvent &rSource::ReinitEvent(rEvent &event, UDINT eid)
{
	event.Reinit(eid) << ID << Descr;

	return event;
}

//-------------------------------------------------------------------------------------------------
// Переинициализируем временный Event, записывая ID объекта и строку с его описанием
rEvent &rSource::ReinitEvent(UDINT eid)
{
	return ReinitEvent(Event, eid);
}


//-------------------------------------------------------------------------------------------------
//
UDINT rSource::InitLink(UINT setup, rLink &link, UDINT unit, UDINT nameid, const string &name, const string &shadow)
{
	link.Init(setup, unit, this, name, nameid);

	link.Shadow = shadow;
	link.Descr  = nameid;

	if(setup & rLink::Setup::INPUT ) m_inputs.push_back (&link);
	if(setup & rLink::Setup::OUTPUT) m_outputs.push_back(&link);

	// Вызываем функцию конечного класса
	InitLimitEvent(link);

	return 0;
}


UDINT rSource::ReinitLimitEvents()
{
	for (auto link : m_inputs) {
		InitLimitEvent(*link);
	}

	for (auto link : m_outputs) {
		if (!(link->m_setup & rLink::Setup::INPUT)) {
			InitLimitEvent(*link);
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
UDINT rSource::LoadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix)
{
	if (!element) {
		return err.set(DATACFGERR_CONFIG, 0, "element is null");
	}

	m_lineNum = element->GetLineNum();

	const char *strAlias = element->Attribute(XmlName::NAME);

	//TODO Можно еще алиас проверить на валидность имени
	if (!strAlias) {
		return err.set(DATACFGERR_INVALID_NAME, m_lineNum);
	}

	if (prefix.size()) {
		Alias = prefix + ".";
	}

	Alias += strAlias;
	Alias  = String_tolower(Alias);
	Descr  = XmlUtils::getAttributeUDINT(element, XmlName::DESC, 0);

	// Загружаем все пределы по всем входам и выходам
	tinyxml2::XMLElement* limits = element->FirstChildElement(XmlName::LIMITS);

	if (nullptr == limits) {
		return TRITONN_RESULT_OK;
	}

	XML_FOR(limit_xml, limits, XmlName::LIMIT) {
		rLink*      link   = nullptr;
		std::string ioname = String_tolower(limit_xml->Attribute(XmlName::NAME));

		for (auto item : m_inputs) {
			if (item->IO_Name == ioname) {
				link = item;
				break;
			}
		}

		if (!link) {
			for (auto item : m_outputs) {
				if (item->IO_Name == ioname) {
					link = item;
					break;
				}
			}
		}

		if (!link) {
			return err.set(DATACFGERR_LIMIT, limit_xml->GetLineNum(), ioname);
		}

		link->Limit.LoadFromXML(limit_xml, err, prefix);
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

	for (auto var : list) {
		if (var->isHide()) {
			continue;
		}

		result += var->saveKernel(Alias.size() + 1, "\t\t");
	}
	result += "\t</values>\n";

	// Входа
	if (m_inputs.size()) {

		result += "\t<inputs>\n";

		for (auto link : m_inputs) {
			UDINT shadow_count = 0;

			result += String_format("\t\t<input name=\"%s\" unit=\"%i\"", link->IO_Name.c_str(), (UDINT)link->Unit);

			for (auto sublink : m_inputs) {
				if (link == sublink) {
					continue;
				}

				if (sublink->Shadow == link->IO_Name) {
					if (0 == shadow_count) {
						result += ">\n";
					}

					result += String_format("\t\t\t<shadow name=\"%s\"/>\n", sublink->IO_Name.c_str());
					++shadow_count;
				}
			}

			if (0 == shadow_count) {
				if (link->Shadow.size()) {
					result += String_format(" shadow=\"%s\"", link->Shadow.c_str());
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
					link->IO_Name.c_str(), (UDINT)link->Unit, (link == m_outputs[0]) ? " default=\"1\"" : "");
		}

		result += "\t\t<output name=\"fault\" unit=\"512\"/>\n"
				  "\t</outputs>\n";
	}

	result += String_format("</%s>\n", Tag[isio ? 1 : 0].c_str());

	return result;
}

UDINT rSource::generateMarkDown(rGeneratorMD& md)
{
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

			rTextManager::instance().Get(link->Unit, strunit);

			result += link->IO_Name + " | ";
			result += strunit + " | " + String_format("%u", static_cast<UDINT>(link->Unit)) + " | ";
			result += link->Limit.m_flagsSetup.getNameByBits(link->Limit.m_setup.Value) + " | ";
			result += link->Shadow + " | ";
			result += /*link->m_comment + */"\n";
		}
	}

	result += "\n## Outputs\n";
	result += "Output | Unit | Unit ID | Limits | Comment\n";
	result += ":-- |:--:|:--:|:--:|:--\n";
	for (auto link : m_outputs) {
		std::string strunit = "";

		rTextManager::instance().Get(link->Unit, strunit);

		result += link->IO_Name + " | ";
		result += strunit + " | " + String_format("%u", static_cast<UDINT>(link->Unit)) + " | ";

		result += link->Limit.m_flagsSetup.getNameByBits(link->Limit.m_setup.Value) + " | ";
		result += /*link->m_comment + */"\n";
	}

	rVariableList list;
	generateVars(list);

	result += "\n## Variable\n";
	result += list.getMarkDown();
	result += "\n";

	return result;
}

std::string rSource::getXmlInput() const
{
	std::string result = "";

	if (m_inputs.size()) {
		for (auto link : m_inputs) {
			result += "<" + link->IO_Name + "><link alias=\"object's output\"/></" + link->IO_Name + ">";

			if (link->Shadow.size()) {
				result += "<!-- Optional -->";
			}

			result += "\n";
		}
	}

	return result;
}

UDINT rSource::checkOutput(const string &name)
{
	string lowname = String_tolower(name);

	if(XmlName::FAULT == lowname) return 0;

	for(auto link : m_outputs)
	{
		if(lowname == link->IO_Name) return 0;
	}

	return 1;
}



// Функция проверяет наличие флага по условию, блокирует этот флаг в переменной LockErr
// и выдает сообщение
UDINT rSource::CheckExpr(bool expr, UDINT flag, rEvent &event_fault, rEvent &event_success)
{
	// Проверка полученных данных
	if(expr)
	{
		if(!(LockErr & flag))
		{
			LockErr |= flag;

			rEventManager::instance().Add(event_fault);
		}

		return 1;
	}
	else
	{
		if(LockErr & flag)
		{
			LockErr &= ~flag;

			rEventManager::instance().Add(event_success);
		}
	}

	return 0;
}


UDINT rSource::SendEventSetLE(UDINT flag, rEvent &event)
{
	if(!(LockErr & flag))
	{
		rEventManager::instance().Add(event);

		LockErr |= flag;

		return 1;
	}

	return 0;
}


UDINT rSource::SendEventClearLE(UDINT flag, rEvent &event)
{
	if(LockErr & flag)
	{
		rEventManager::instance().Add(event);
		LockErr &= ~flag;

		return 1;
	}

	return 0;
}

