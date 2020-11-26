//=================================================================================================
//===
//=== data_config.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Сборный класс описывающий связи между элементами.
//=== Так же осуществляет загрузку конфигурации с диска.
//===
//=================================================================================================


#include "cJSON.h"
#include "data_config.h"
#include "event_manager.h"
#include "text_manager.h"
#include "log_manager.h"
#include "precision.h"
#include "simplefile.h"
#include "stringex.h"
#include "users.h"
#include "hash.h"
#include "threadmaster.h"
#include "io/manager.h"
#include "data_manager.h"
#include "data_link.h"
#include "data_ai.h"
#include "data_di.h"
#include "data_do.h"
#include "data_counter.h"
#include "data_denssol.h"
#include "data_reduceddensity.h"
#include "data_report.h"
#include "data_selector.h"
#include "data_station.h"
#include "data_stream.h"
#include "data_rvar.h"
#include "modbustcpslave_manager.h"
#include "opcua_manager.h"
#include "structures.h"
#include "xml_util.h"

std::vector<rBitFlag> rDataConfig::STNProductValues;
std::vector<rBitFlag> rDataConfig::STRFMeterFlags;
std::vector<rBitFlag> rDataConfig::SelectorSetupFlags;
std::vector<rBitFlag> rDataConfig::SelectorModeFlags;
std::vector<rBitFlag> rDataConfig::ReportTypeFlags;
std::vector<rBitFlag> rDataConfig::ReportPeriodFlags;


rDataConfig::rDataConfig()
{
	Prefix          = "";
	ErrorID         = tinyxml2::XML_SUCCESS;
	ErrorLine       = 0;
	CfgJSON         = cJSON_CreateObject();
	CfgJSON_VAR     = cJSON_CreateArray();
	CfgJSON_IO      = cJSON_CreateArray();
	CfgJSON_OBJ     = cJSON_CreateArray();
	CfgJSON_USR     = cJSON_CreateArray();
	XMLRootSecurity = nullptr;

	cJSON_AddItemToObject(CfgJSON, XmlName::VARS , CfgJSON_VAR);
	cJSON_AddItemToObject(CfgJSON, XmlName::IO   , CfgJSON_IO);
	cJSON_AddItemToObject(CfgJSON, XmlName::CALC , CfgJSON_OBJ);
	cJSON_AddItemToObject(CfgJSON, XmlName::USERS, CfgJSON_USR);
}


rDataConfig::~rDataConfig()
{
	// Линки удаляют сами объекты
	ListLink.clear();

	if(CfgJSON) cJSON_Delete(CfgJSON);
	CfgJSON = nullptr;
}


void rDataConfig::InitBitFlags()
{
	rDataConfig::STNProductValues.push_back(rBitFlag("PETROLEUM"   , PRODUCT_PETROLEUM));
	rDataConfig::STNProductValues.push_back(rBitFlag("GAZOLENE"    , PRODUCT_GAZOLENE));
	rDataConfig::STNProductValues.push_back(rBitFlag("TRANSITION"  , PRODUCT_TRANSITION));
	rDataConfig::STNProductValues.push_back(rBitFlag("JETFUEL"     , PRODUCT_JETFUEL));
	rDataConfig::STNProductValues.push_back(rBitFlag("FUELOIL"     , PRODUCT_FUELOIL));
	rDataConfig::STNProductValues.push_back(rBitFlag("SMARTBENZENE", PRODUCT_SMARTBENZENE));
	rDataConfig::STNProductValues.push_back(rBitFlag("LUBRICANT"   , PRODUCT_LUBRICANT));

	rDataConfig::STRFMeterFlags.push_back(rBitFlag("TURBINE"   , STR_FLOWMETER_TURBINE));
	rDataConfig::STRFMeterFlags.push_back(rBitFlag("CARIOLIS"  , STR_FLOWMETER_CARIOLIS));
	rDataConfig::STRFMeterFlags.push_back(rBitFlag("ULTRASONIC", STR_FLOWMETER_ULTRASONIC));

	rDataConfig::SelectorSetupFlags.push_back(rBitFlag("OFF"    , SELECTOR_SETUP_OFF));
	rDataConfig::SelectorSetupFlags.push_back(rBitFlag("NOEVENT", SELECTOR_SETUP_NOEVENT));

	rDataConfig::SelectorModeFlags.push_back(rBitFlag("NEXT"    , SELECTOR_MODE_CHANGENEXT));
	rDataConfig::SelectorModeFlags.push_back(rBitFlag("PREV"    , SELECTOR_MODE_CHANGEPREV));
	rDataConfig::SelectorModeFlags.push_back(rBitFlag("NOCHANGE", SELECTOR_MODE_NOCHANGE));
	rDataConfig::SelectorModeFlags.push_back(rBitFlag("ERROR"   , SELECTOR_MODE_TOERROR));

	//
	rDataConfig::ReportTypeFlags.push_back  (rBitFlag("PERIODIC" , REPORT_PERIODIC)        );
	rDataConfig::ReportTypeFlags.push_back  (rBitFlag("BATCH"    , REPORT_BATCH)           );

	rDataConfig::ReportPeriodFlags.push_back(rBitFlag("HOUR"     , REPORT_PERIOD_HOUR)     );
	rDataConfig::ReportPeriodFlags.push_back(rBitFlag("2HOUR"    , REPORT_PERIOD_2HOUR)    );
	rDataConfig::ReportPeriodFlags.push_back(rBitFlag("3HOUR"    , REPORT_PERIOD_3HOUR)    );
	rDataConfig::ReportPeriodFlags.push_back(rBitFlag("4HOUR"    , REPORT_PERIOD_4HOUR)    );
	rDataConfig::ReportPeriodFlags.push_back(rBitFlag("6HOUR"    , REPORT_PERIOD_6HOUR)    );
	rDataConfig::ReportPeriodFlags.push_back(rBitFlag("8HOUR"    , REPORT_PERIOD_8HOUR)    );
	rDataConfig::ReportPeriodFlags.push_back(rBitFlag("12HOUR"   , REPORT_PERIOD_12HOUR)   );
	rDataConfig::ReportPeriodFlags.push_back(rBitFlag("DAYLY"    , REPORT_PERIOD_DAYLY)    );
	rDataConfig::ReportPeriodFlags.push_back(rBitFlag("WEEKLY"   , REPORT_PERIOD_WEEKLY)   );
	rDataConfig::ReportPeriodFlags.push_back(rBitFlag("BIWEEKLY" , REPORT_PERIOD_BIWEEKLY) );
	rDataConfig::ReportPeriodFlags.push_back(rBitFlag("MONTHLY"  , REPORT_PERIOD_MONTHLY)  );
	rDataConfig::ReportPeriodFlags.push_back(rBitFlag("QUARTERLY", REPORT_PERIOD_QUARTERLY));
	rDataConfig::ReportPeriodFlags.push_back(rBitFlag("ANNUAL"   , REPORT_PERIOD_ANNUAL)   );
	rDataConfig::ReportPeriodFlags.push_back(rBitFlag("5MIN"     , REPORT_PERIOD_5MIN)     );
	rDataConfig::ReportPeriodFlags.push_back(rBitFlag("15MIN"    , REPORT_PERIOD_15MIN)    );
}


//-------------------------------------------------------------------------------------------------
// Возвращает строковое название флага, если выставлены несколько бит, то вернется срока с
// несколькими флагами
string rDataConfig::GetFlagNameByBit(vector<rBitFlag> &arr, UDINT value)
{
	string result = "";

	for(UDINT ii = 0; ii < arr.size(); ++ii)
	{
		if(arr[ii].Value & value)
		{
			result += ((result.size()) ? "|" : "") + arr[ii].Name;
		}
	}

	return result;
}


//-------------------------------------------------------------------------------------------------
// Возвращает строковое значение числа, проверка идет на равенство
string rDataConfig::GetFlagNameByValue(vector<rBitFlag> &arr, UDINT value)
{
	for(UDINT ii = 0; ii < arr.size(); ++ii)
	{
		if(arr[ii].Value == value)
		{
			return arr[ii].Name;
		}
	}

	return "";
}


//-------------------------------------------------------------------------------------------------
//
UDINT rDataConfig::GetFlagBitByStr(vector<rBitFlag> &arr, const string &name, UDINT &err)
{
	if(err) return 0;

	for(UDINT ii = 0; ii < arr.size(); ++ii)
	{
		if(arr[ii].Name == name)
		{
			return arr[ii].Value;
		}
	}

	err = 1;
	return 0;
}


//
UDINT rDataConfig::GetFlagFromStr(vector<rBitFlag> &arr, const string &str, UDINT &err)
{
	string name   = "";
	UDINT  result = 0;

	if(err) return result;
	if(str.empty()) return result;

	for(UDINT ii = 0; ii < str.size(); ++ii)
	{
		if(' ' == str[ii] || '|' == str[ii])
		{
			if(name.size())
			{
				result |= GetFlagBitByStr(arr, name, err);
				name    = "";

				if(err) break;
			}
		}
		else
		{
			name += str[ii];
		}
	}
	if(name.size())
	{
		result |= GetFlagBitByStr(arr, name, err);
	}

	return result;
}


//-------------------------------------------------------------------------------------------------
// Получение данных от менеджера данных
UDINT rDataConfig::LoadFile(const string &filename, rSystemVariable &sysvar, vector<rSource *> &listsrc, vector<rInterface *> &listiface, vector<rReport *> &listrpt)
{
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLDocument doc_security;
	tinyxml2::XMLElement* root = nullptr;
	std::string fullname      = DIR_CONF + filename;
	std::string info_devel    = "";
	std::string info_hash     = "";
	std::string info_name     = "";
	std::string info_ver      = "";

	FileName      = filename;
	Prefix        = "";
	ErrorLine     = 0;
	ErrorID       = TRITONN_RESULT_OK;
	SysVar        = &sysvar;
	ListSource    = &listsrc;
	ListReport    = &listrpt;
	ListInterface = &listiface;

	// Очистим структуру информации
	SysVar->ConfigInfo.Developer[0] = 0;
	SysVar->ConfigInfo.File[0]      = 0;
	SysVar->ConfigInfo.Hash[0]      = 0;
	SysVar->ConfigInfo.Name[0]      = 0;
	SysVar->ConfigInfo.Version[0]   = 0;

	//TODO Нужно проверить Hasp
	//TODO Нужно передавать не имя, а указатель на xml_root, так как в rDataManager::LoadConfig мы уже разобрали этот файл

	if(tinyxml2::XML_SUCCESS != doc.LoadFile(fullname.c_str()))
	{
		ErrorID   = doc.ErrorID();
		ErrorLine = 0;

		return ErrorID;
	}

	root = doc.FirstChildElement(XmlName::TRITONN);
	if(nullptr == root)
	{
		ErrorID   = DATACFGERR_STRUCT;
		ErrorLine = 0;

		return ErrorID;
	}

	if(TRITONN_RESULT_OK != LoadSecurity(root, doc_security))
	{
		return ErrorID;
	}

	if(TRITONN_RESULT_OK != LoadHardware(root))
	{
		return ErrorID;
	}

	if(TRITONN_RESULT_OK != LoadConfig(root))
	{
		return ErrorID;
	}

	// Загружаем пользователей
	if(TRITONN_RESULT_OK != LoadUsers(XMLRootSecurity, CfgJSON_USR))
	{
		tinyxml2::XMLElement *xml_root = root->FirstChildElement(XmlName::SECURITY);

		ErrorLine = (xml_root == nullptr) ? 0 : xml_root->GetLineNum();

		return ErrorID;
	}


	// Вся конфигурация загруженна, расчитываем линки
	if(tinyxml2::XML_SUCCESS != ResolveLinks())
	{
		return ErrorID;
	}

	// Находим нарастающие для отчетов
	if(tinyxml2::XML_SUCCESS != ResolveReports())
	{
		return ErrorID;
	}

	// Загружаем строки и события
	if(tinyxml2::XML_SUCCESS != LoadCustom(root))
	{
		return ErrorID;
	}

	// Загружаем интерфейсы
	if(tinyxml2::XML_SUCCESS != LoadComms(root))
	{
		return ErrorID;
	}

	// Загружаем события

	// Заполним информацию по конфиге
	strncpy(SysVar->ConfigInfo.File, filename.c_str(), MAX_CONFIG_NAME);



	//TODO прока пропишем жестко русский язык
	strncpy(SysVar->Lang, LANG_RU.c_str(), MAX_LANG_SIZE);

	// Сохраняем информацию для WEB
	SaveWeb();

	return 0;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rDataConfig::LoadSecurity(tinyxml2::XMLElement *root, tinyxml2::XMLDocument &doc_security)
{
	tinyxml2::XMLElement* xml_crypt = root->FirstChildElement(XmlName::SECURITY);
	string      aes_text  = "";
	string      xml_src   = "";

	if(nullptr == xml_crypt)
	{
		ErrorLine = root->GetLineNum();
		ErrorID   = DATACFGERR_SECURITY_NF;

		return ErrorID;
	}

	// Дешифруем блок пользователей
	aes_text = xml_crypt->GetText();
	aes_text = String_deletewhite(aes_text);

	if(DecryptEAS(aes_text, AES_KEY, AES_IV, xml_src))
	{
		ErrorLine = xml_crypt->GetLineNum();
		ErrorID   = DATACFGERR_SECURITY_DESCRYPT;

		return ErrorID;
	}

	if(tinyxml2::XML_SUCCESS != doc_security.Parse(xml_src.c_str()))
	{
		ErrorLine = xml_crypt->GetLineNum();
		ErrorID   = DATACFGERR_SECURITY_PARSE;

		return ErrorID;
	}

	// Парсим разкодированный блок пользователей
	XMLRootSecurity = doc_security.FirstChildElement(XmlName::SECURITY);
	if(nullptr == XMLRootSecurity)
	{
		ErrorLine = xml_crypt->GetLineNum();
		ErrorID   = DATACFGERR_SECURITY_PARSE;

		return ErrorID;
	}

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rDataConfig::LoadHardware(tinyxml2::XMLElement *root)
{
	tinyxml2::XMLElement *hardware = root->FirstChildElement(XmlName::HARDWARE);

	if(nullptr == hardware)
	{
		ErrorLine = root->GetLineNum();
		ErrorID   = DATACFGERR_NOTFOUND_HARDWARE;
		return ErrorID;
	}

	return rIOManager::instance().LoadFromXML(hardware, *this);
}


UDINT rDataConfig::LoadConfig(tinyxml2::XMLElement *root)
{
	tinyxml2::XMLElement *config = root->FirstChildElement(XmlName::CONFIG);
	cJSON      *jstn   = cJSON_CreateArray();

	if(nullptr == config)
	{
		ErrorID   = DATACFGERR_CONFIG;
		ErrorLine = root->GetLineNum();

		return ErrorID;
	}

	// Для начала обработаем секцию <io>
	Prefix = "io";
	if(tinyxml2::XML_SUCCESS != LoadIO(config, CfgJSON_IO, nullptr))
	{
		return ErrorID;
	}

	// Далее грузим объекты
	Prefix = "obj";
	if(TRITONN_RESULT_OK != LoadCalc(config, CfgJSON_OBJ, nullptr))
	{
		return ErrorID;
	}

	Prefix = "vars";
	if(TRITONN_RESULT_OK != LoadVariable(config)) return ErrorID;

	// грузим станции
	Prefix = "";
	cJSON_AddItemToObject(CfgJSON, XmlName::STATIONS, jstn);
	if(tinyxml2::XML_SUCCESS != LoadStation(config, jstn))
	{
		return ErrorID;
	}

	if(tinyxml2::XML_SUCCESS != LoadReport(config))
	{
		return ErrorID;
	}

	return tinyxml2::XML_SUCCESS;
}





//-------------------------------------------------------------------------------------------------
//
UDINT rDataConfig::LoadIO(tinyxml2::XMLElement *root, cJSON *jroot, rStation *owner)
{
	tinyxml2::XMLElement *xml_io = root->FirstChildElement(XmlName::IO);
	rSource    *source = nullptr;
	string      name   = "";

   // Данного элемента нет в дереве
	if(nullptr == xml_io)
	{
		return TRITONN_RESULT_OK;
	}

	// Разбираем элементы
	for(tinyxml2::XMLElement *obj = xml_io->FirstChildElement(); obj != nullptr; obj = obj->NextSiblingElement())
	{
		name   = obj->Name();
		source = nullptr;

		if(XmlName::AI == name) { if(SysVar->Max.m_ai >= MAX_IO_AI) return DATACFGERR_MAX_AI; source = dynamic_cast<rSource*>(new rAI());      source->ID = SysVar->Max.m_fi++; }
		if(XmlName::FI == name) { if(SysVar->Max.m_fi >= MAX_IO_FI) return DATACFGERR_MAX_FI; source = dynamic_cast<rSource*>(new rCounter()); source->ID = SysVar->Max.m_fi++; }
		if(XmlName::DI == name) { if(SysVar->Max.m_di >= MAX_IO_DI) return DATACFGERR_MAX_DI; source = dynamic_cast<rSource*>(new rDI());      source->ID = SysVar->Max.m_di++; }
		if(XmlName::DO == name) { if(SysVar->Max.m_do >= MAX_IO_DO) return DATACFGERR_MAX_DO; source = dynamic_cast<rSource*>(new rDO());      source->ID = SysVar->Max.m_do++; }

		if(nullptr == source)
		{
			ErrorID   = DATACFGERR_UNKNOWIO;
			ErrorLine = obj->GetLineNum();

			return ErrorID;
		}

		source->Station = owner;
		if(tinyxml2::XML_SUCCESS != (ErrorID = source->LoadFromXML(obj, *this)))
		{
			ErrorLine = obj->GetLineNum();

			return ErrorID;
		}

		ListSource->push_back(source);

		cJSON *jsrc = cJSON_CreateObject();
		cJSON *jitm = cJSON_CreateObject();
		cJSON_AddItemToObject(jsrc, XmlName::ALIAS, cJSON_CreateString(source->Alias.c_str()));
		cJSON_AddItemToObject(jsrc, XmlName::DESC , cJSON_CreateNumber(source->Descr));
		cJSON_AddItemToObject(jitm, source->RTTI(), jsrc);
		cJSON_AddItemToArray(jroot, jitm);
	}

	return TRITONN_RESULT_OK;
}


//
UDINT rDataConfig::LoadCalc(tinyxml2::XMLElement *root, cJSON *jroot, rStation *owner)
{
	tinyxml2::XMLElement* calc = root->FirstChildElement(XmlName::CALC);
	rSource    *source = nullptr;
	string      name   = "";

	// Данного элемента нет в дереве
	if(nullptr == calc)
	{
		return TRITONN_RESULT_OK;
	}

	// Разбираем элементы
	for(tinyxml2::XMLElement *obj = calc->FirstChildElement(); obj != nullptr; obj = obj->NextSiblingElement())
	{
		name   = obj->Name();
		source = nullptr;

		if(XmlName::DENSSOL     == name) { if(SysVar->Max.m_densSol     >= MAX_DENSSOL    ) return DATACFGERR_MAX_DENSSOL;  source = dynamic_cast<rSource*>(new rDensSol());     source->ID = SysVar->Max.m_densSol++;     }
		if(XmlName::REDUCEDDENS == name) { if(SysVar->Max.m_reducedDens >= MAX_REDUCEDDENS) return DATACFGERR_MAX_RDCDENS;  source = dynamic_cast<rSource*>(new rReducedDens()); source->ID = SysVar->Max.m_reducedDens++; }
		if(XmlName::MSELECTOR   == name) { if(SysVar->Max.m_selector    >= MAX_SELECTOR   ) return DATACFGERR_MAX_SELECTOR; source = dynamic_cast<rSource*>(new rSelector());    source->ID = SysVar->Max.m_selector++;    }
		if(XmlName::SELECTOR    == name) { if(SysVar->Max.m_selector    >= MAX_SELECTOR   ) return DATACFGERR_MAX_SELECTOR; source = dynamic_cast<rSource*>(new rSelector());    source->ID = SysVar->Max.m_selector++;    }
		if(XmlName::SAMPLER     == name) continue;

		if(nullptr == source)
		{
			ErrorID   = DATACFGERR_UNKNOWCALC;
			ErrorLine = obj->GetLineNum();

			return ErrorID;
		}

		source->Station = owner;
		if(TRITONN_RESULT_OK != (ErrorID = source->LoadFromXML(obj, *this)))
		{
			ErrorLine = obj->GetLineNum();

			return ErrorID;
		}

		ListSource->push_back(source);

		cJSON *jitm = cJSON_CreateObject();
		cJSON *jsrc = cJSON_CreateObject();
		cJSON_AddItemToObject(jsrc, XmlName::ALIAS, cJSON_CreateString(source->Alias.c_str()));
		cJSON_AddItemToObject(jsrc, XmlName::DESC , cJSON_CreateNumber(source->Descr));
		cJSON_AddItemToObject(jitm, source->RTTI(), jsrc);
		cJSON_AddItemToArray(jroot, jitm);
	}

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rDataConfig::LoadStation(tinyxml2::XMLElement *root, cJSON *jroot)
{
	tinyxml2::XMLElement *stations = root->FirstChildElement(XmlName::STATIONS);

	// Данного элемента нет в дереве
	if(nullptr == stations)
	{
		ErrorID   = DATACFGERR_STATIONSNF;
		ErrorLine = root->GetLineNum();

		return ErrorID;
	}

	// Разбираем элементы
	for(tinyxml2::XMLElement *stncfg = stations->FirstChildElement(XmlName::STATION); stncfg != nullptr; stncfg = stncfg->NextSiblingElement(XmlName::STATION))
	{
		rStation *stn = nullptr;

		if(SysVar->Max.m_station++ >= MAX_STATION)
		{
			ErrorID   = DATACFGERR_MAX_STATION;
			ErrorLine = stncfg->GetLineNum();

			return ErrorID;
		}

		stn    = new rStation();
		Prefix = "";

		if(tinyxml2::XML_SUCCESS != stn->LoadFromXML(stncfg, *this))
		{
			ErrorID   = DATACFGERR_STATION;
			ErrorLine = stncfg->GetLineNum();

			return ErrorID;
		}


		cJSON *jitm = cJSON_CreateObject();
		cJSON *jstn = cJSON_CreateObject();
		cJSON *jobj = cJSON_CreateArray();
		cJSON *jstr = cJSON_CreateArray();
		cJSON_AddItemToObject(jstn, XmlName::ALIAS  , cJSON_CreateString(stn->Alias.c_str()));
		cJSON_AddItemToObject(jstn, XmlName::DESC   , cJSON_CreateNumber(stn->Descr));
		cJSON_AddItemToObject(jstn, XmlName::STREAMS, jstr);
		cJSON_AddItemToObject(jstn, XmlName::CALC   , jobj);
		cJSON_AddItemToObject(jitm, stn->RTTI()    , jstn);
		cJSON_AddItemToArray (jroot, jitm);

		// Загрузим IO
		Prefix = stn->Alias + ".io";
		if(tinyxml2::XML_SUCCESS != LoadIO(stncfg, CfgJSON_IO, stn)) return ErrorID;

		// Загрузим Obj
		Prefix = stn->Alias + ".obj";
		if(tinyxml2::XML_SUCCESS != LoadCalc(stncfg, jobj, stn)) return ErrorID;

		// Загрузим Stream
		Prefix = stn->Alias;
		if(tinyxml2::XML_SUCCESS != LoadStream(stncfg, jstr, stn)) return ErrorID;

		ListSource->push_back(stn);
	}

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rDataConfig::LoadStream(tinyxml2::XMLElement *root, cJSON *jroot, rStation *owner)
{
	tinyxml2::XMLElement *streams   = root->FirstChildElement(XmlName::STREAMS);
	string      oldprefix = Prefix;

	// Данного элемента нет в дереве
	if(nullptr == streams)
	{
		ErrorLine = root->GetLineNum();
		ErrorID   = DATACFGERR_STREAMSNF;

		return ErrorID;
	}

	// Разбираем элементы
	for(tinyxml2::XMLElement *strcfg = streams->FirstChildElement(XmlName::STREAM); strcfg != nullptr; strcfg = strcfg->NextSiblingElement(XmlName::STREAM))
	{
		rStream *str = nullptr;

		if(SysVar->Max.m_stream++ >= MAX_STREAM)
		{
			ErrorLine = root->GetLineNum();
			ErrorID   = DATACFGERR_MAX_STREAM;

			return ErrorID;;
		}

		str          = new rStream();
		str->Station = owner;
		Prefix       = oldprefix;

		if(tinyxml2::XML_SUCCESS != str->LoadFromXML(strcfg, *this))
		{
			return ErrorID;
		}

		cJSON *jitm = cJSON_CreateObject();
		cJSON *jstr = cJSON_CreateObject();
		cJSON *jobj = cJSON_CreateArray();
		cJSON_AddItemToObject(jstr, XmlName::ALIAS, cJSON_CreateString(str->Alias.c_str()));
		cJSON_AddItemToObject(jstr, XmlName::DESC , cJSON_CreateNumber(str->Descr));
		cJSON_AddItemToObject(jstr, XmlName::CALC , jobj);
		cJSON_AddItemToObject(jitm, str->RTTI()  , jstr);
		cJSON_AddItemToArray (jroot, jitm);

		// Кросс-линк станции и линии
		owner->Stream.push_back(str);

		// Загрузим IO
		Prefix   = str->Alias + ".io";
		if(tinyxml2::XML_SUCCESS != LoadIO(strcfg, CfgJSON_IO, owner)) return ErrorID;

		// Загрузим Obj
		Prefix   = str->Alias + ".obj";
		if(tinyxml2::XML_SUCCESS != LoadCalc(strcfg, jobj, owner)) return ErrorID;

		ListSource->push_back(str);
	}

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rDataConfig::LoadReport(tinyxml2::XMLElement *root)
{
	tinyxml2::XMLElement *reportsystem = root->FirstChildElement(XmlName::REPORTSYSTEM);
	tinyxml2::XMLElement *reports      = nullptr;

	// Данного элемента нет в дереве
	if(nullptr == reportsystem)
	{
		return tinyxml2::XML_SUCCESS;
	}

	reports = reportsystem->FirstChildElement(XmlName::REPORTS);
	if(nullptr == reports)
	{
		ErrorLine = root->GetLineNum();
		ErrorID   = DATACFGERR_NOREPORTS;

		return ErrorID;
	}

	// Разбираем элементы
	for(tinyxml2::XMLElement *rptcfg = reports->FirstChildElement(XmlName::REPORT); rptcfg != nullptr; rptcfg = rptcfg->NextSiblingElement(XmlName::REPORT))
	{
		rReport *rpt = new rReport();

		if(tinyxml2::XML_SUCCESS != rpt->LoadFromXML(rptcfg, *this))
		{
			ErrorLine = root->GetLineNum();
			ErrorID   = DATACFGERR_REPORT;

			return ErrorID;
		}
	}

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rDataConfig::LoadCustom(tinyxml2::XMLElement *root)
{
	UDINT                 result    = TRITONN_RESULT_OK;
	tinyxml2::XMLElement *custom    = root->FirstChildElement(XmlName::CUSTOM);
	tinyxml2::XMLElement *userstr   = nullptr;
//	tinyxml2::XMLElement *userevent = nullptr;
	tinyxml2::XMLElement *precision = nullptr;

	if(nullptr == custom) return TRITONN_RESULT_OK;

	userstr   = custom->FirstChildElement(XmlName::STRINGS);
//	userevent = custom->FirstChildElement(XmlName::EVENTS);
	precision = custom->FirstChildElement(XmlName::PRECISION);

	//
	if(nullptr != userstr)
	{
		if(tinyxml2::XML_SUCCESS != rTextManager::Instance().Load(userstr))
		{
			ErrorID   = rTextManager::Instance().ErrorID;
			ErrorLine = rTextManager::Instance().ErrorLine;

			return ErrorID;
		}
	}

	//

	// Точность ед. измерения
	if(nullptr != precision)
	{
		if(tinyxml2::XML_SUCCESS != (result = rPrecision::Instance().Load(precision)))
		{
			ErrorID = result;
			ErrorLine = precision->GetLineNum();

			return ErrorID;
		}
	}


	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
UDINT rDataConfig::LoadVariable(tinyxml2::XMLElement *root)
{
	tinyxml2::XMLElement* xml_vars = root->FirstChildElement(XmlName::VARIABLES);
	rSource    *source   = nullptr;

	// Данного элемента нет в дереве
	if (nullptr == xml_vars) {
		return TRITONN_RESULT_OK;
	}

	// Разбираем элементы
	for(tinyxml2::XMLElement *xml_var = xml_vars->FirstChildElement(); xml_var != nullptr; xml_var = xml_var->NextSiblingElement())
	{
		source = new rRVar();

		if(tinyxml2::XML_SUCCESS != (ErrorID = source->LoadFromXML(xml_var, *this)))
		{
			delete source;
			ErrorLine = xml_var->GetLineNum();

			return ErrorID;
		}

		ListSource->push_back(source);

		cJSON *jsrc = cJSON_CreateObject();
//		cJSON *jitm = cJSON_CreateObject();
		cJSON_AddItemToObject(jsrc, XmlName::ALIAS, cJSON_CreateString(source->Alias.c_str()));
		cJSON_AddItemToObject(jsrc, XmlName::DESC , cJSON_CreateNumber(source->Descr));
//		cJSON_AddItemToObject(jitm, source->RTTI(), jsrc);
		cJSON_AddItemToArray(CfgJSON_VAR, /*jitm*/jsrc);
	}

	return TRITONN_RESULT_OK;
}



UDINT rDataConfig::LoadUsers(tinyxml2::XMLElement *root, cJSON *jroot)
{
	UNUSED(jroot);
	tinyxml2::XMLElement* xml_users = nullptr;

	xml_users = root->FirstChildElement(XmlName::USERS);
	if(nullptr == xml_users)
	{
		ErrorLine = 0;
		ErrorID   = DATACFGERR_USERS_PARSE;

		return ErrorID;
	}

	// Загружаем пользователей
	for(tinyxml2::XMLElement* xml_user = xml_users->FirstChildElement(XmlName::USER); xml_user != nullptr; xml_user = xml_user->NextSiblingElement(XmlName::USER))
	{
		tinyxml2::XMLElement *xml_password = xml_user->FirstChildElement(XmlName::PASSWORD);
		tinyxml2::XMLElement *xml_comms    = xml_user->FirstChildElement(XmlName::COMMS);
		tinyxml2::XMLElement *xml_right    = xml_user->FirstChildElement(XmlName::RIGHTS);

		if(nullptr == xml_user->Attribute(XmlName::NAME) || nullptr == xml_password || nullptr == xml_right || nullptr == xml_comms)
		{
			ErrorLine = 0;
			ErrorID   = DATACFGERR_USERS_PARSE;

			return ErrorID;
		}

		tinyxml2::XMLElement *xml_intaccess = xml_right->FirstChildElement(XmlName::INTERNAL);
		tinyxml2::XMLElement *xml_extaccess = xml_right->FirstChildElement(XmlName::EXTERNAL);
		tinyxml2::XMLElement *xml_ilogin    = xml_comms->FirstChildElement(XmlName::LOGIN);    // Интерфейс. Логин
		tinyxml2::XMLElement *xml_ipwd      = xml_comms->FirstChildElement(XmlName::PASSWORD); // Интерфейс. Пароль

		if(nullptr == xml_intaccess || nullptr == xml_extaccess || nullptr == xml_ipwd || nullptr == xml_ilogin)
		{
			ErrorLine = 0;
			ErrorID   = DATACFGERR_USERS_PARSE;

			return ErrorID;
		}

		// Interface
		USINT  pwd_hash [MAX_HASH_SIZE] = {0};
		USINT  ipwd_hash[MAX_HASH_SIZE] = {0};
		string name      = "";
		UDINT  ilogin    = 0;
		UDINT  intaccess = 0;
		UDINT  extaccess = 0;

		if(nullptr != xml_ilogin->GetText())
		{
			ilogin = atoi(xml_ilogin->GetText());

			if(nullptr == xml_ipwd->GetText() && ilogin)
			{
				ErrorLine = 0;
				ErrorID   = DATACFGERR_USERS_PARSE;

				return ErrorID;
			}

			String_ToBuffer(xml_ipwd->GetText(), ipwd_hash, MAX_HASH_SIZE);
		}

		name = xml_user->Attribute(XmlName::NAME);

		// Password
		String_ToBuffer(xml_password->GetText(), pwd_hash, MAX_HASH_SIZE);

		// Access
		String_IsValidHex(xml_intaccess->GetText(), intaccess);
		String_IsValidHex(xml_extaccess->GetText(), extaccess);

		intaccess &= ~(ACCESS_SA | ACCESS_ADMIN);
		extaccess &= ~(ACCESS_SA | ACCESS_ADMIN);

		// Наконец то добавляем пользователя
		rUser::Add(name, pwd_hash, intaccess, extaccess, ilogin, ipwd_hash);

		cJSON *jsrc = cJSON_CreateObject();
		cJSON_AddItemToObject(jsrc, XmlName::NAME, cJSON_CreateString(name.c_str()));
		cJSON_AddItemToArray(CfgJSON_USR, jsrc);
	}

	return TRITONN_RESULT_OK;
}




UDINT rDataConfig::LoadComms(tinyxml2::XMLElement *root)
{
	UDINT result = TRITONN_RESULT_OK;
	tinyxml2::XMLElement *xml_comms = nullptr;

	if(nullptr == root)
	{
		return TRITONN_RESULT_OK;
	}

	xml_comms = root->FirstChildElement(XmlName::COMMS);

	if(nullptr == xml_comms)
	{
		return TRITONN_RESULT_OK;
	}

	result = LoadModbus(xml_comms);
	if(TRITONN_RESULT_OK != result)
	{
		return result;
	}

	result = LoadOPCUA(xml_comms);
	if(TRITONN_RESULT_OK != result)
	{
		return result;
	}

	return TRITONN_RESULT_OK;
}


UDINT rDataConfig::LoadModbus(tinyxml2::XMLElement *root)
{
	UDINT result = TRITONN_RESULT_OK;
	tinyxml2::XMLElement *xml_modbus = nullptr;

	if(nullptr == root)
	{
		return 0;
	}

	xml_modbus = root->FirstChildElement(XmlName::MODBUS);

	if(nullptr == xml_modbus)
	{
		return 0;
	}

	for(tinyxml2::XMLElement *xml_item = xml_modbus->FirstChildElement(); xml_item != nullptr; xml_item = xml_item->NextSiblingElement())
	{
		string name = (xml_item->Name() == nullptr) ? "" : xml_item->Name();

		if(XmlName::SLAVETCP == name) { result = LoadModbusTCP(xml_item); }

		if(result != TRITONN_RESULT_OK)
		{
			return result;
		}
	}

	return TRITONN_RESULT_OK;
}



UDINT rDataConfig::LoadModbusTCP(tinyxml2::XMLElement *root)
{
	UDINT                   result   = TRITONN_RESULT_OK;
	rModbusTCPSlaveManager *slavetcp = new rModbusTCPSlaveManager();

	result = slavetcp->loadFromXML(root, *this);
	slavetcp->Pause.Set(-1);

	if(TRITONN_RESULT_OK != result)
	{
		delete slavetcp;

		ErrorLine = root->GetLineNum();
		ErrorID   = result;

		return ErrorID;
	}

	ListInterface->push_back(slavetcp);

	return result;
}



UDINT rDataConfig::LoadOPCUA(tinyxml2::XMLElement *root)
{
	UDINT                 result    = TRITONN_RESULT_OK;
	tinyxml2::XMLElement *xml_opcua = nullptr;
	rOPCUAManager        *opcua     = nullptr;

	if(nullptr == root)
	{
		return TRITONN_RESULT_OK;
	}

	xml_opcua = root->FirstChildElement(XmlName::OPCUA);

	if(nullptr == xml_opcua)
	{
		return TRITONN_RESULT_OK;
	}

	opcua  = new rOPCUAManager();
	result = opcua->loadFromXML(xml_opcua, *this);
	opcua->Pause.Set(-1);

	if(TRITONN_RESULT_OK != result)
	{
		delete opcua;

		ErrorLine = root->GetLineNum();
		ErrorID   = result;

		return ErrorID;
	}

	ListInterface->push_back(opcua);

	return result;
}


//-------------------------------------------------------------------------------------------------
// Основная особенность функции LoadLink от LoadShadowLink в том, что на вход в первую подается
// указатель на сам тег линка, а во вторую указатель на родителя
UDINT rDataConfig::LoadLink(tinyxml2::XMLElement* element, rLink& link)
{
	if(nullptr == element)
	{
		link.Source = nullptr;
		ErrorLine   = 0;
		ErrorID     = DATACFGERR_LINKNF;

		return ErrorID;
	}

	if(tinyxml2::XML_SUCCESS != link.LoadFromXML(element, *this)) тут нужно передавать указатель на линк
	{
		link.Source = nullptr;
		ErrorLine   = element->GetLineNum();
		ErrorID     = DATACFGERR_LINK;

		return ErrorID;
	}

	link.Source    = nullptr;
	link.m_lineNum = element->GetLineNum();

	ListLink.push_back(&link);

	return tinyxml2::XML_SUCCESS;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rDataConfig::LoadShadowLink(tinyxml2::XMLElement *element, rLink &link, rLink &mainlink, const string &name)
{
	if(nullptr != element)
	{
		return LoadLink(element, link);
	}

	link.Alias     = mainlink.Alias;
	link.FullTag   = mainlink.Alias + ":" + name;
	link.Param     = name;
	link.m_lineNum = mainlink.m_lineNum;

	ListLink.push_back(&link);

	return tinyxml2::XML_SUCCESS;
}


//
UDINT rDataConfig::ResolveLinks(void)
{
	for (auto link : ListLink) {
		// Проходим все загруженные объекты
		for (auto src : *ListSource) {
			if (!strcasecmp(link->Alias.c_str(), src->Alias.c_str())) {
				std::string aa = src->Alias;
				// Проверка на наличие требуемого параметра
				if(link->Param.size())
				{
					if(src->CheckOutput(link->Param))
					{
						src->CheckOutput(link->Param);
						ErrorStr  = link->FullTag;
						ErrorLine = link->m_lineNum;
						ErrorID   = DATACFGERR_CHECKLINK;

						return ErrorID;
					}
				}

				link->Source = src;
				break;
			}
		}
		if(nullptr == link->Source)
		{
			ErrorStr  = link->FullTag;
			ErrorLine = link->m_lineNum;
			ErrorID   = DATACFGERR_RESOLVELINK;

			return ErrorID;
		}
/*
		// Если поле параметра пустое, то можно удалить этот линк, и связать два объекта на
		// прямую, т.к. сами объекты запрашивают данные без указания параметра.
		if(link->Param.empty())
		{
			*link->Dest = link->Source;

			delete link;
			ListLink.erase(ListLink.begin() + ii);
			--ii;
		}
*/
	}

	return tinyxml2::XML_SUCCESS;
}


UDINT rDataConfig::ResolveReports(void)
{
	for(UDINT ii = 0; ii < ListReport->size(); ++ii)
	{
		rReportDataset *rpt = &(*ListReport)[ii]->Present;

		for(UDINT jj = 0; jj < rpt->AverageItems.size(); ++jj)
		{
			rReportTotal *tot = rpt->AverageItems[jj];

			for(UDINT ss = 0; ss < ListSource->size(); ++ss)
			{
				rSource      *scr    = (*ListSource)[ss];
				const rTotal *scrtot = scr->GetTotal();

				if(nullptr == scrtot) continue;

				if(tot->Alias == scr->Alias)
				{
					tot->Source = scrtot;
					break;
				}
			}

			if(nullptr == tot->Source)
			{
				ErrorStr  = tot->Name;
				ErrorLine = 0;
				ErrorID   = DATACFGERR_RESOLVETOTAL;

				return ErrorID;
			}
		}
	}

	return tinyxml2::XML_SUCCESS;
}


//-------------------------------------------------------------------------------------------------
void rDataConfig::SaveWeb()
{
	char  *str = cJSON_Print(CfgJSON);
	UDINT  result = SimpleFileSave(DIR_WWW + "application/core/tree_objects.json", str);

	free(str);
	if(TRITONN_RESULT_OK != result)
	{
		rEventManager::instance().AddEventUDINT(EID_SYSTEM_FILEIOERROR, HALT_REASON_WEBFILE | result);
		TRACEERROR("Can't save json tree");

		rDataManager::instance().DoHalt(HALT_REASON_WEBFILE | result);
		return;
	}

	//TODO Нужно вначале удалить все языковые файлы для web

	// Сохраняем массив строк
	vector<string>    langlist;
	vector<rTextItem> sidlist;

	rTextManager::Instance().GetListLang(langlist);

	for(UDINT ii = 0; ii < langlist.size(); ++ii)
	{
		string  text     = "";
		UDINT   result   = TRITONN_RESULT_OK;

		rTextManager::Instance().GetListSID(langlist[ii], sidlist);

		text = String_format("<?php\n$lang[\"language_locale\"] = \"%s\";\n\n", langlist[ii].c_str());

		for(UDINT jj = 0; jj < sidlist.size(); ++jj)
		{
			text += String_format("$lang[\"%u\"]=\"%s\";\n", sidlist[jj].ID, sidlist[jj].Text.c_str());
		}
		text += "?>";

		result = SimpleFileSave(String_format("%sapplication/language/%s/custom_lang.php", DIR_WWW.c_str(), langlist[ii].c_str()), text);
		if(TRITONN_RESULT_OK != result)
		{
			rEventManager::instance().AddEventUDINT(EID_SYSTEM_FILEIOERROR, HALT_REASON_WEBFILE | result);
			TRACEERROR("Can't save json tree");

			rDataManager::instance().DoHalt(HALT_REASON_WEBFILE | result);
			return;
		}
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Вспомогательные функции
//
///////////////////////////////////////////////////////////////////////////////////////////////////

tinyxml2::XMLElement *rDataConfig::GetRootSecurity()
{
	return XMLRootSecurity;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rDataConfig::GetAttributeUDINT(tinyxml2::XMLElement *element, const string &name, UDINT def)
{
	return (element->Attribute(name.c_str()) ) ? strtoul(element->Attribute(name.c_str()), NULL, 0)  : def;
}

USINT rDataConfig::GetAttributeUSINT (tinyxml2::XMLElement *element, const string &name, const USINT def)
{
	return (USINT)GetAttributeUDINT(element, name, def);
}


//-------------------------------------------------------------------------------------------------
//
string rDataConfig::GetAttributeString(tinyxml2::XMLElement *element, const string &name, const string &def)
{
	return (element->Attribute(name.c_str()) ) ? element->Attribute(name.c_str())  : def;
}



//-------------------------------------------------------------------------------------------------
//
string rDataConfig::GetTextString(tinyxml2::XMLElement *element, const string &def, UDINT &err)
{
	string result = def;

	if(nullptr == element)
	{
		err = 1;
		return result;
	}

	if(nullptr == element->GetText())
	{
		err = 1;
		return result;
	}

	result = element->GetText();
	return result;
}


//-------------------------------------------------------------------------------------------------
//
LREAL rDataConfig::GetTextLREAL(tinyxml2::XMLElement *element, LREAL def, UDINT &err)
{
	LREAL result = def;

	if(nullptr == element)
	{
		err = 1;
		return result;
	}

	if(tinyxml2::XML_SUCCESS != element->QueryDoubleText(&result))
	{
		err = 1;
		return result;
	}

	return result;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rDataConfig::GetTextUDINT(tinyxml2::XMLElement *element, UDINT def, UDINT &err)
{
	UDINT result = def;

	if(nullptr == element)
	{
		err = 1;
		return result;
	}

	if(tinyxml2::XML_SUCCESS != element->QueryIntText((int *)&result))
	{
		err = 1;
		return result;
	}

	return result;
}


//-------------------------------------------------------------------------------------------------
//
UINT rDataConfig::GetTextUINT(tinyxml2::XMLElement *element, UINT def, UDINT &err)
{
	USINT result = def;
	UDINT value  = GetTextUDINT(element, (UDINT)def, err);

	if(err) return result;

	if(value > 0xFFFF)
	{
		err = 1;
		return result;
	}

	result = value;

	return result;
}


//-------------------------------------------------------------------------------------------------
//
USINT rDataConfig::GetTextUSINT(tinyxml2::XMLElement *element, USINT def, UDINT &err)
{
	USINT result = def;
	UDINT value  = GetTextUDINT(element, (UDINT)def, err);

	if(err) return result;

	if(value > 0xFF)
	{
		err = 1;
		return result;
	}

	result = value;

	return result;
}




