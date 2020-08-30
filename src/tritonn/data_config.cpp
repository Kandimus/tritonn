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
#include "data_manager.h"
#include "data_link.h"
#include "data_ai.h"
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

using namespace tinyxml2;



vector<rBitFlag> rDataConfig::AIModeFlags;
vector<rBitFlag> rDataConfig::AISetupFlags;
vector<rBitFlag> rDataConfig::FISetupFlags;
vector<rBitFlag> rDataConfig::STNProductValues;
vector<rBitFlag> rDataConfig::STRFMeterFlags;
vector<rBitFlag> rDataConfig::SelectorSetupFlags;
vector<rBitFlag> rDataConfig::SelectorModeFlags;
vector<rBitFlag> rDataConfig::LimitSetupFlags;
vector<rBitFlag> rDataConfig::ReportTypeFlags;
vector<rBitFlag> rDataConfig::ReportPeriodFlags;
vector<rBitFlag> rDataConfig::VarSetupFlags;


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

	cJSON_AddItemToObject(CfgJSON, CFGNAME_VARS , CfgJSON_VAR);
	cJSON_AddItemToObject(CfgJSON, CFGNAME_IO   , CfgJSON_IO);
	cJSON_AddItemToObject(CfgJSON, CFGNAME_CALC , CfgJSON_OBJ);
	cJSON_AddItemToObject(CfgJSON, CFGNAME_USERS, CfgJSON_USR);
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
	rDataConfig::AIModeFlags.clear();
	rDataConfig::AIModeFlags.push_back(rBitFlag("PHIS"  , AI_MODE_PHIS));
	rDataConfig::AIModeFlags.push_back(rBitFlag("KEYPAD", AI_MODE_MKEYPAD));

	rDataConfig::AISetupFlags.clear();
	rDataConfig::AISetupFlags.push_back(rBitFlag("OFF"      , AI_SETUP_OFF));
	rDataConfig::AISetupFlags.push_back(rBitFlag("NOBUFFER" , AI_SETUP_NOBUFFER));
	rDataConfig::AISetupFlags.push_back(rBitFlag("VIRTUAL"  , AI_SETUP_VIRTUAL));
	rDataConfig::AISetupFlags.push_back(rBitFlag("NOICE"    , AI_SETUP_NOICE));
	rDataConfig::AISetupFlags.push_back(rBitFlag("KEYPAD"   , AI_SETUP_ERR_KEYPAD));
	rDataConfig::AISetupFlags.push_back(rBitFlag("LASTGOOD" , AI_SETUP_ERR_LASTGOOD));

	rDataConfig::LimitSetupFlags.push_back(rBitFlag("OFF" , LIMIT_SETUP_OFF));
	rDataConfig::LimitSetupFlags.push_back(rBitFlag("LOLO", LIMIT_SETUP_AMIN));
	rDataConfig::LimitSetupFlags.push_back(rBitFlag("LO"  , LIMIT_SETUP_WMIN));
	rDataConfig::LimitSetupFlags.push_back(rBitFlag("HI"  , LIMIT_SETUP_WMAX));
	rDataConfig::LimitSetupFlags.push_back(rBitFlag("HIHI", LIMIT_SETUP_AMAX));

	rDataConfig::FISetupFlags.clear();
	rDataConfig::FISetupFlags.push_back(rBitFlag("OFF"      , FI_SETUP_OFF));
	rDataConfig::FISetupFlags.push_back(rBitFlag("NOBUFFER" , FI_SETUP_NOBUFFER));

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

	rDataConfig::VarSetupFlags.push_back(rBitFlag("CONST"         , VAR_SETUP_CONST));
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
	XMLDocument doc;
	XMLDocument doc_security;
	XMLElement *root          = nullptr;
	string      fullname      = DIR_CONF + filename;
	string      info_devel    = "";
	string      info_hash     = "";
	string      info_name     = "";
	string      info_ver      = "";

	FileName      = filename;
	Prefix        = "";
	ErrorLine     = 0;
	ErrorID       = XML_SUCCESS;
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

	if(XML_SUCCESS != doc.LoadFile(fullname.c_str()))
	{
		ErrorID   = doc.ErrorID();
		ErrorLine = 0;

		return ErrorID;
	}

	root = doc.FirstChildElement(CFGNAME_TRITONN);
	if(nullptr == root)
	{
		ErrorID   = DATACFGERR_STRUCT;
		ErrorLine = 0;

		return ErrorID;
	}

	if(tinyxml2::XML_SUCCESS != LoadSecurity(root, doc_security))
	{
		return ErrorID;
	}

	if(XML_SUCCESS != LoadHardware(root))
	{
		return ErrorID;
	}

	if(XML_SUCCESS != LoadConfig(root))
	{
		return ErrorID;
	}

	// Загружаем пользователей
	if(XML_SUCCESS != LoadUsers(XMLRootSecurity, CfgJSON_USR))
	{
		tinyxml2::XMLElement *xml_root = root->FirstChildElement(CFGNAME_SECURITY);

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
	XMLElement *xml_crypt = root->FirstChildElement(CFGNAME_SECURITY);
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
	XMLRootSecurity = doc_security.FirstChildElement(CFGNAME_SECURITY);
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
	XMLElement *hardware = root->FirstChildElement(CFGNAME_HARDWARE);

	hardware = hardware;
/*
	if(nullptr == hardware)
	{
		ErrorStr = "Не найдена секция описания модулей ввода-вывода.";
		ErrorID  = DATACFGERR_HARDWARE;
		return ErrorID;
	}

	for(XMLElement *module = hardware->FirstChildElement(); module != nullptr; module = child->NextSiblingElement())
	{
		string mtype(module->Attribute("type", ""));
		string mver (module->Attribute("ver" , ""));



	}
*/
	return 0;
}


UDINT rDataConfig::LoadConfig(tinyxml2::XMLElement *root)
{
	XMLElement *config = root->FirstChildElement(CFGNAME_CONFIG);
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
	if(tinyxml2::XML_SUCCESS != LoadCalc(config, CfgJSON_OBJ, nullptr))
	{
		return ErrorID;
	}

	Prefix = "vars";
	if(XML_SUCCESS != LoadVariable(config)) return ErrorID;

	// грузим станции
	Prefix = "";
	cJSON_AddItemToObject(CfgJSON, CFGNAME_STATIONS, jstn);
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
	XMLElement *xml_io = root->FirstChildElement(CFGNAME_IO);
	rSource    *source = nullptr;
	string      name   = "";

   // Данного элемента нет в дереве
	if(nullptr == xml_io)
	{
		return XML_SUCCESS;
	}

	// Разбираем элементы
	for(XMLElement *obj = xml_io->FirstChildElement(); obj != nullptr; obj = obj->NextSiblingElement())
	{
		name   = obj->Name();
		source = nullptr;

		if(CFGNAME_AI == name) { if(SysVar->Max.AI >= MAX_IO_AI) return DATACFGERR_MAX_AI; source = new rAI();      source->ID = SysVar->Max.AI++; }
		if(CFGNAME_FI == name) { if(SysVar->Max.FI >= MAX_IO_FI) return DATACFGERR_MAX_FI; source = new rCounter(); source->ID = SysVar->Max.FI++; }

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
		cJSON_AddItemToObject(jsrc, CFGNAME_ALIAS, cJSON_CreateString(source->Alias.c_str()));
		cJSON_AddItemToObject(jsrc, CFGNAME_DESC , cJSON_CreateNumber(source->Descr));
		cJSON_AddItemToObject(jitm, source->RTTI(), jsrc);
		cJSON_AddItemToArray(jroot, jitm);
	}

	return XML_SUCCESS;
}


//
UDINT rDataConfig::LoadCalc(tinyxml2::XMLElement *root, cJSON *jroot, rStation *owner)
{
	XMLElement *calc   = root->FirstChildElement(CFGNAME_CALC);
	rSource    *source = nullptr;
	string      name   = "";

	// Данного элемента нет в дереве
	if(nullptr == calc)
	{
		return XML_SUCCESS;
	}

	// Разбираем элементы
	for(XMLElement *obj = calc->FirstChildElement(); obj != nullptr; obj = obj->NextSiblingElement())
	{
		name   = obj->Name();
		source = nullptr;

		if(CFGNAME_DENSSOL     == name) { if(SysVar->Max.DensSol     >= MAX_DENSSOL    ) return DATACFGERR_MAX_DENSSOL;  source = new rDensSol();     source->ID = SysVar->Max.DensSol++;     }
		if(CFGNAME_REDUCEDDENS == name) { if(SysVar->Max.ReducedDens >= MAX_REDUCEDDENS) return DATACFGERR_MAX_RDCDENS;  source = new rReducedDens(); source->ID = SysVar->Max.ReducedDens++; }
		if(CFGNAME_MSELECTOR   == name) { if(SysVar->Max.Selector    >= MAX_SELECTOR   ) return DATACFGERR_MAX_SELECTOR; source = new rSelector();    source->ID = SysVar->Max.Selector++;    }
		if(CFGNAME_SELECTOR    == name) { if(SysVar->Max.Selector    >= MAX_SELECTOR   ) return DATACFGERR_MAX_SELECTOR; source = new rSelector();    source->ID = SysVar->Max.Selector++;    }
		if(CFGNAME_SAMPLER     == name) continue;

		if(nullptr == source)
		{
			ErrorID   = DATACFGERR_UNKNOWCALC;
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

		cJSON *jitm = cJSON_CreateObject();
		cJSON *jsrc = cJSON_CreateObject();
		cJSON_AddItemToObject(jsrc, CFGNAME_ALIAS, cJSON_CreateString(source->Alias.c_str()));
		cJSON_AddItemToObject(jsrc, CFGNAME_DESC , cJSON_CreateNumber(source->Descr));
		cJSON_AddItemToObject(jitm, source->RTTI(), jsrc);
		cJSON_AddItemToArray(jroot, jitm);
	}

	return XML_SUCCESS;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rDataConfig::LoadStation(tinyxml2::XMLElement *root, cJSON *jroot)
{
	XMLElement *stations = root->FirstChildElement(CFGNAME_STATIONS);

	// Данного элемента нет в дереве
	if(nullptr == stations)
	{
		ErrorID   = DATACFGERR_STATIONSNF;
		ErrorLine = root->GetLineNum();

		return ErrorID;
	}

	// Разбираем элементы
	for(XMLElement *stncfg = stations->FirstChildElement(CFGNAME_STATION); stncfg != nullptr; stncfg = stncfg->NextSiblingElement(CFGNAME_STATION))
	{
		rStation *stn = nullptr;

		if(SysVar->Max.Station >= MAX_STATION)
		{
			ErrorID   = DATACFGERR_MAX_STATION;
			ErrorLine = stncfg->GetLineNum();

			return ErrorID;
		}

		stn    = new rStation();
		Prefix = "";
		SysVar->Max.Station++;
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
		cJSON_AddItemToObject(jstn, CFGNAME_ALIAS  , cJSON_CreateString(stn->Alias.c_str()));
		cJSON_AddItemToObject(jstn, CFGNAME_DESC   , cJSON_CreateNumber(stn->Descr));
		cJSON_AddItemToObject(jstn, CFGNAME_STREAMS, jstr);
		cJSON_AddItemToObject(jstn, CFGNAME_CALC   , jobj);
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

	return XML_SUCCESS;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rDataConfig::LoadStream(tinyxml2::XMLElement *root, cJSON *jroot, rStation *owner)
{
	XMLElement *streams   = root->FirstChildElement(CFGNAME_STREAMS);
	string      oldprefix = Prefix;

	// Данного элемента нет в дереве
	if(nullptr == streams)
	{
		ErrorLine = root->GetLineNum();
		ErrorID   = DATACFGERR_STREAMSNF;

		return ErrorID;
	}

	// Разбираем элементы
	for(XMLElement *strcfg = streams->FirstChildElement(CFGNAME_STREAM); strcfg != nullptr; strcfg = strcfg->NextSiblingElement(CFGNAME_STREAM))
	{
		rStream *str = nullptr;

		if(SysVar->Max.Stream >= MAX_STREAM)
		{
			ErrorLine = root->GetLineNum();
			ErrorID   = DATACFGERR_MAX_STREAM;

			return ErrorID;;
		}

		str          = new rStream();
		str->Station = owner;
		Prefix       = oldprefix;
		SysVar->Max.Stream++;

		if(tinyxml2::XML_SUCCESS != str->LoadFromXML(strcfg, *this))
		{
			ErrorLine = root->GetLineNum();
			ErrorID   = DATACFGERR_STREAM;

			return ErrorID;
		}

		cJSON *jitm = cJSON_CreateObject();
		cJSON *jstr = cJSON_CreateObject();
		cJSON *jobj = cJSON_CreateArray();
		cJSON_AddItemToObject(jstr, CFGNAME_ALIAS, cJSON_CreateString(str->Alias.c_str()));
		cJSON_AddItemToObject(jstr, CFGNAME_DESC , cJSON_CreateNumber(str->Descr));
		cJSON_AddItemToObject(jstr, CFGNAME_CALC , jobj);
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

	return XML_SUCCESS;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rDataConfig::LoadReport(tinyxml2::XMLElement *root)
{
	XMLElement *reportsystem = root->FirstChildElement(CFGNAME_REPORTSYSTEM);
	XMLElement *reports      = nullptr;

	// Данного элемента нет в дереве
	if(nullptr == reportsystem)
	{
		return tinyxml2::XML_SUCCESS;
	}

	reports = reportsystem->FirstChildElement(CFGNAME_REPORTS);
	if(nullptr == reports)
	{
		ErrorLine = root->GetLineNum();
		ErrorID   = DATACFGERR_NOREPORTS;

		return ErrorID;
	}

	// Разбираем элементы
	for(XMLElement *rptcfg = reports->FirstChildElement(CFGNAME_REPORT); rptcfg != nullptr; rptcfg = rptcfg->NextSiblingElement(CFGNAME_REPORT))
	{
		rReport *rpt = new rReport();

		if(tinyxml2::XML_SUCCESS != rpt->LoadFromXML(rptcfg, *this))
		{
			ErrorLine = root->GetLineNum();
			ErrorID   = DATACFGERR_REPORT;

			return ErrorID;
		}
	}

	return XML_SUCCESS;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rDataConfig::LoadCustom(tinyxml2::XMLElement *root)
{
	UDINT                 result    = tinyxml2::XML_SUCCESS;
	tinyxml2::XMLElement *custom    = root->FirstChildElement(CFGNAME_CUSTOM);
	tinyxml2::XMLElement *userstr   = nullptr;
	tinyxml2::XMLElement *userevent = nullptr;
	tinyxml2::XMLElement *precision = nullptr;

	if(nullptr == custom) return XML_SUCCESS;

	userstr   = custom->FirstChildElement(CFGNAME_STRINGS);
	userevent = custom->FirstChildElement(CFGNAME_EVENTS);
	precision = custom->FirstChildElement(CFGNAME_PRECISION);

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


	return XML_SUCCESS;
}


//-------------------------------------------------------------------------------------------------
UDINT rDataConfig::LoadVariable(tinyxml2::XMLElement *root)
{
	XMLElement *xml_vars = root->FirstChildElement(CFGNAME_VARIABLES);
	rSource    *source   = nullptr;

	// Данного элемента нет в дереве
	if(nullptr == xml_vars)
	{
		return XML_SUCCESS;
	}

	// Разбираем элементы
	for(XMLElement *xml_var = xml_vars->FirstChildElement(); xml_var != nullptr; xml_var = xml_var->NextSiblingElement())
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
		cJSON_AddItemToObject(jsrc, CFGNAME_ALIAS, cJSON_CreateString(source->Alias.c_str()));
		cJSON_AddItemToObject(jsrc, CFGNAME_DESC , cJSON_CreateNumber(source->Descr));
//		cJSON_AddItemToObject(jitm, source->RTTI(), jsrc);
		cJSON_AddItemToArray(CfgJSON_VAR, /*jitm*/jsrc);
	}

	return XML_SUCCESS;
}



UDINT rDataConfig::LoadUsers(tinyxml2::XMLElement *root, cJSON *jroot)
{
	XMLElement *xml_users = nullptr;

	xml_users = root->FirstChildElement(CFGNAME_USERS);
	if(nullptr == xml_users)
	{
		ErrorLine = 0;
		ErrorID   = DATACFGERR_USERS_PARSE;

		return ErrorID;
	}

	// Загружаем пользователей
	for(XMLElement *xml_user = xml_users->FirstChildElement(CFGNAME_USER); xml_user != nullptr; xml_user = xml_user->NextSiblingElement(CFGNAME_USER))
	{
		XMLElement *xml_password = xml_user->FirstChildElement(CFGNAME_PASSWORD);
		XMLElement *xml_comms    = xml_user->FirstChildElement(CFGNAME_COMMS);
		XMLElement *xml_right    = xml_user->FirstChildElement(CFGNAME_RIGHTS);

		if(nullptr == xml_user->Attribute(CFGNAME_NAME) || nullptr == xml_password || nullptr == xml_right || nullptr == xml_comms)
		{
			ErrorLine = 0;
			ErrorID   = DATACFGERR_USERS_PARSE;

			return ErrorID;
		}

		XMLElement *xml_intaccess = xml_right->FirstChildElement(CFGNAME_INTERNAL);
		XMLElement *xml_extaccess = xml_right->FirstChildElement(CFGNAME_EXTERNAL);
		XMLElement *xml_ilogin    = xml_comms->FirstChildElement(CFGNAME_LOGIN);    // Интерфейс. Логин
		XMLElement *xml_ipwd      = xml_comms->FirstChildElement(CFGNAME_PASSWORD); // Интерфейс. Пароль

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

		name = xml_user->Attribute(CFGNAME_NAME);

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
		cJSON_AddItemToObject(jsrc, CFGNAME_NAME, cJSON_CreateString(name.c_str()));
		cJSON_AddItemToArray(CfgJSON_USR, jsrc);
	}

	return XML_SUCCESS;
}




UDINT rDataConfig::LoadComms(tinyxml2::XMLElement *root)
{
	UDINT result = TRITONN_RESULT_OK;
	tinyxml2::XMLElement *xml_comms = nullptr;

	if(nullptr == root)
	{
		return TRITONN_RESULT_OK;
	}

	xml_comms = root->FirstChildElement(CFGNAME_COMMS);

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

	xml_modbus = root->FirstChildElement(CFGNAME_MODBUS);

	if(nullptr == xml_modbus)
	{
		return 0;
	}

	for(tinyxml2::XMLElement *xml_item = xml_modbus->FirstChildElement(); xml_item != nullptr; xml_item = xml_item->NextSiblingElement())
	{
		string name = (xml_item->Name() == nullptr) ? "" : xml_item->Name();

		if(CFGNAME_SLAVETCP == name) { result = LoadModbusTCP(xml_item); }

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

	result = slavetcp->LoadFromXML(root, *this);
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

	xml_opcua = root->FirstChildElement(CFGNAME_OPCUA);

	if(nullptr == xml_opcua)
	{
		return TRITONN_RESULT_OK;
	}

	opcua  = new rOPCUAManager();
	result = opcua->LoadFromXML(xml_opcua, *this);
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
UDINT rDataConfig::LoadLink(tinyxml2::XMLElement *element, rLink &link)
{
	if(nullptr == element)
	{
		link.Source = nullptr;
		ErrorLine   = 0;
		ErrorID     = DATACFGERR_LINKNF;

		return ErrorID;
	}

	if(tinyxml2::XML_SUCCESS != link.LoadFromXML(element, *this))
	{
		link.Source = nullptr;
		ErrorLine   = element->GetLineNum();
		ErrorID     = DATACFGERR_LINK;

		return ErrorID;
	}

	link.Source = nullptr;

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

	link.Alias   = mainlink.Alias;
	link.FullTag = mainlink.Alias + ":" + name;
	link.Param   = name;

	ListLink.push_back(&link);

	return tinyxml2::XML_SUCCESS;
}


//
UDINT rDataConfig::ResolveLinks(void)
{
	UDINT    ii   = 0;
	UDINT    jj   = 0;
	rLink   *link = nullptr;
	rSource *src  = nullptr;

	for(ii = 0; ii < ListLink.size(); ++ii)
	{
		link = ListLink[ii];

		// Проходим все загруженные объекты
		for(jj = 0; jj < ListSource->size(); ++jj)
		{
			src = (*ListSource)[jj];

			// Нашли нужный объект
			if(!strcasecmp(link->Alias.c_str(), src->Alias.c_str()))
			{
				// Проверка на наличие требуемого параметра
				if(link->Param.size())
				{
					if(src->CheckOutput(link->Param))
					{
						ErrorStr  = link->FullTag;
						ErrorLine = 0;
						ErrorID   = DATACFGERR_CHECKLINK;
					}
				}

				link->Source = src;
				break;
			}
		}
		if(nullptr == link->Source)
		{
			ErrorStr  = link->FullTag;
			ErrorLine = 0;
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
		rEventManager::Instance().AddEventUDINT(EID_SYSTEM_FILEIOERROR, HALT_REASON_WEBFILE | result);
		TRACEERROR("Can't save json tree");

		rDataManager::Instance().DoHalt(HALT_REASON_WEBFILE | result);
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
			rEventManager::Instance().AddEventUDINT(EID_SYSTEM_FILEIOERROR, HALT_REASON_WEBFILE | result);
			TRACEERROR("Can't save json tree");

			rDataManager::Instance().DoHalt(HALT_REASON_WEBFILE | result);
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




