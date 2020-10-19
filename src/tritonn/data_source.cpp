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
#include "data_link.h"
#include "data_config.h"
#include "event_manager.h"
#include "data_variable.h"
#include "data_source.h"
#include "text_manager.h"
#include "xml_util.h"



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
	Inputs.clear();
	Outputs.clear();
}


//-------------------------------------------------------------------------------------------------
// Поиск выходного линка по его имени
rLink *rSource::GetOutputByName(const string &name)
{
	if(Outputs.empty())
	{
		return nullptr;
	}

	if(name.empty())
	{
		return Outputs[0];
	}

	for(UDINT ii = 0; ii < Outputs.size(); ++ii)
	{
		string aaa = Outputs[ii]->IO_Name;
		if(name == Outputs[ii]->IO_Name)
		{
			return Outputs[ii];
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
		SendEventSetLE(SOURCE_LE_OUTPUT, Event.Reinit(EID_SYSTEM_ERROUTPUT) << Descr);

		return std::numeric_limits<LREAL>::quiet_NaN();
	}

	// Проводим к нужному типу
	err = rUnits::ConvertValue(link->Value, link->Unit, result, unit);

	if(err)
	{
		SendEventSetLE(SOURCE_LE_UNIT, Event.Reinit(EID_SYSTEM_ERRUNIT) << Descr << link->Unit << unit);
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
		SendEventSetLE(SOURCE_LE_OUTPUT, Event.Reinit(EID_SYSTEM_ERROUTPUT) << Descr);

		return 0xFFFFFFFF;
	}

	return link->Unit;
}


//-------------------------------------------------------------------------------------------------
UDINT rSource::GetFault()
{
	return Fault;
}


const rTotal *rSource::GetTotal(void)
{
	return nullptr;
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

	for(UDINT ii = 0; ii < Inputs.size(); ++ii)
	{
		Inputs[ii]->Calculate();
	}

	return 0;
}



UDINT rSource::PostCalculate()
{
	for(UDINT ii = 0; ii < Outputs.size(); ++ii)
	{
		if(Outputs[ii]->Setup & LINK_SETUP_INPUT) continue;

		Outputs[ii]->CalculateLimit();
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
UDINT rSource::InitLink(UINT setup, rLink &link, UDINT unit, STRID nameid, const string &name, const string &shadow)
{
	link.Init(setup, unit, this, name, nameid);

	link.Shadow = shadow;
	link.Descr  = nameid;

	if(setup & LINK_SETUP_INPUT ) Inputs.push_back (&link);
	if(setup & LINK_SETUP_OUTPUT) Outputs.push_back(&link);

	// Вызываем функцию конечного класса
	InitLimitEvent(link);

	return 0;
}


UDINT rSource::ReinitLimitEvents()
{
	for(UDINT ii = 0; ii < Inputs.size(); ++ii)
	{
		InitLimitEvent(*Inputs[ii]);
	}

	for(UDINT ii = 0; ii < Outputs.size(); ++ii)
	{
		if(!(Outputs[ii]->Setup & LINK_SETUP_INPUT)) InitLimitEvent(*Outputs[ii]);
	}

	return 0;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rSource::GenerateVars(vector<rVariable *> &list)
{
	for(UDINT ii = 0; ii < Inputs.size(); ++ii)
	{
		Inputs[ii]->GenerateVars(list);
	}

	for(UDINT ii = 0; ii < Outputs.size(); ++ii)
	{
		if(Outputs[ii]->Setup & LINK_SETUP_INPUT) continue;

		Outputs[ii]->GenerateVars(list);
	}

	return 0;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rSource::LoadFromXML(tinyxml2::XMLElement *element, rDataConfig &cfg)
{
	const char *strAlias = element->Attribute("name");

	if(!strAlias) return 1; //TODO Можно еще алиас проверить на валидность имени

	if(cfg.Prefix.size())
	{
		Alias  = cfg.Prefix + ".";
	}

	Alias += strAlias;
	Alias  = String_tolower(Alias);
	Descr  = rDataConfig::GetAttributeUDINT(element, "descr", 0);

	// Загружаем все пределы по всем входам и выходам
	tinyxml2::XMLElement *limits = element->FirstChildElement(XmlName::LIMITS);

	if(nullptr == limits) return tinyxml2::XML_SUCCESS;

	for(tinyxml2::XMLElement *limit = limits->FirstChildElement(XmlName::LIMIT); nullptr != limit; limit = limit->NextSiblingElement(XmlName::LIMIT))
	{
		rLink *link = nullptr;
		string ioname = String_tolower(limit->Attribute(XmlName::NAME));

		for(UDINT ii = 0; ii < Inputs.size(); ++ii)
		{
			if(Inputs[ii]->IO_Name == ioname)
			{
				link = Inputs[ii];
				break;
			}
		}

		if(nullptr == link)
		{
			for(UDINT ii = 0; ii < Outputs.size(); ++ii)
			{
				if(Outputs[ii]->IO_Name == ioname)
				{
					link = Outputs[ii];
					break;
				}
			}
		}

		if(nullptr == link)
		{
			return DATACFGERR_LIMIT;
		}

		link->Limit.LoadFromXML(limit, cfg);
	}

	return tinyxml2::XML_SUCCESS;
}



//-------------------------------------------------------------------------------------------------
//
UDINT rSource::SaveKernel(FILE *file, UDINT isio, const string &objname, const string &comment, UDINT isglobal)
{
	const string Tag[2] = {"object", "io"};
	vector<rVariable *> list;

	GenerateVars(list);

	fprintf(file, "<!--\n\t%s\n-->\n", comment.c_str());

	fprintf(file, "<%s name=\"%s\"", Tag[isio ? 1 : 0].c_str(), objname.c_str());
	if(!isio) fprintf(file, " global=\"%s\"", (isglobal ? "true" : "false"));
	fprintf(file, ">\n");

	fprintf(file, "\t<values>\n");
	for(UDINT ii = 0; ii < list.size(); ++ii)
	{
		rVariable *v = list[ii];

		if(v->Flags & VARF___H_) continue;

		fprintf(file, "\t\t<value name=\"%s\" type=\"%s\" readonly=\"%i\" loadable=\"%i\" unit=\"%i\" access=\"0x%08X\"/>\n",
				  v->Name.c_str() + Alias.size() + 1, NAME_TYPE[v->Type].c_str(), (v->Flags & VARF_R___) ? 1 : 0, (v->Flags & VARF____L) ? 1 : 0, (UDINT)v->Unit, v->Access);
	}
	fprintf(file, "\t</values>\n");

	// Входа
	if(Inputs.size())
	{
		fprintf(file, "\t<inputs>\n");
		for(UDINT ii = 0; ii < Inputs.size(); ++ii)
		{
			UDINT shadow_count = 0;

			fprintf(file, "\t\t<input name=\"%s\" unit=\"%i\"", Inputs[ii]->IO_Name.c_str(), (UDINT)Inputs[ii]->Unit);

			for(UDINT jj = 0; jj < Inputs.size(); ++jj)
			{
				if(ii == jj) continue;

				if(Inputs[jj]->Shadow == Inputs[ii]->IO_Name)
				{
					if(0 == shadow_count) fprintf(file, ">\n");

					fprintf(file, "\t\t\t<shadow name=\"%s\"/>\n", Inputs[jj]->IO_Name.c_str());
					++shadow_count;
				}
			}

			if(0 == shadow_count)
			{
				if(Inputs[ii]->Shadow.size()) fprintf(file, " shadow=\"%s\"", Inputs[ii]->Shadow.c_str());
				fprintf(file, "/>\n");
			}
			else
			{
				fprintf(file, "\t\t</input>\n");
			}
		}
		fprintf(file, "\t</inputs>\n");
	}

	// Outputs
	if(Outputs.size())
	{
		fprintf(file, "\t<outputs>\n");
		for(UDINT ii = 0; ii < Outputs.size(); ++ii)
		{
			fprintf(file, "\t\t<output name=\"%s\" unit=\"%i\"%s/>\n", Outputs[ii]->IO_Name.c_str(), (UDINT)Outputs[ii]->Unit, (0 == ii) ? " default=\"1\"" : "");
		}

		fprintf(file, "\t\t<output name=\"fault\" unit=\"512\"/>\n");

		fprintf(file, "\t</outputs>\n");
	}

	fprintf(file, "</%s>\n", Tag[isio ? 1 : 0].c_str());

	for(UDINT ii = 0; ii < list.size(); ++ii)
	{
		delete list[ii];
	}
	list.clear();

	return 0;
}




UDINT rSource::CheckOutput(const string &name)
{
	string lowname = String_tolower(name);

	if(XmlName::FAULT == lowname) return 0;

	for(UDINT ii = 0; ii < Outputs.size(); ++ii)
	{
		if(lowname == name) return 0;
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

