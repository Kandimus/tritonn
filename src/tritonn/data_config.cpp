//=================================================================================================
//===
//=== data_config.cpp
//===
//=== Copyright (c) 2019-2021 by RangeSoft.
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
#include "event/manager.h"
#include "text_manager.h"
#include "log_manager.h"
#include "precision.h"
#include "simplefile.h"
#include "stringex.h"
#include "users.h"
#include "hash.h"
#include "threadmaster.h"
#include "io/manager.h"
#include "io/defines.h"
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
#include "data_sampler.h"
#include "data/prove.h"
#include "data/average.h"
#include "data/masswater.h"
#include "data/volwater.h"
#include "data/cinematicviscosity.h"
#include "interface/modbustcpslave_manager.h"
#include "interface/opcua_manager.h"
#include "structures.h"
#include "xml_util.h"


rDataConfig::rDataConfig()
{
	m_json     = cJSON_CreateObject();
	m_json_var = cJSON_CreateArray();
	m_json_io  = cJSON_CreateArray();
	m_json_obj = cJSON_CreateArray();
	m_json_usr = cJSON_CreateArray();
	m_json_hdw = cJSON_CreateArray();
	XMLRootSecurity = nullptr;

	cJSON_AddItemToObject(m_json, XmlName::HARDWARE, m_json_hdw);
	cJSON_AddItemToObject(m_json, XmlName::VARS    , m_json_var);
	cJSON_AddItemToObject(m_json, XmlName::IO      , m_json_io);
	cJSON_AddItemToObject(m_json, XmlName::CALC    , m_json_obj);
	cJSON_AddItemToObject(m_json, XmlName::USERS   , m_json_usr);
}


rDataConfig::~rDataConfig()
{
	m_error.clear();

	// Линки удаляют сами объекты
	ListLink.clear();

	if (m_json) {
		cJSON_Delete(m_json);
		m_json = nullptr;
	}
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
	SysVar        = &sysvar;
	ListSource    = &listsrc;
	ListReport    = &listrpt;
	ListInterface = &listiface;

	m_error.clear();

	// Очистим структуру информации
	SysVar->ConfigInfo.Developer[0] = 0;
	SysVar->ConfigInfo.File[0]      = 0;
	SysVar->ConfigInfo.Hash[0]      = 0;
	SysVar->ConfigInfo.Name[0]      = 0;
	SysVar->ConfigInfo.Version[0]   = 0;

	//TODO Нужно проверить Hasp
	//TODO Нужно передавать не имя, а указатель на xml_root, так как в rDataManager::LoadConfig мы уже разобрали этот файл

	if (tinyxml2::XML_SUCCESS != doc.LoadFile(fullname.c_str())) {
		return m_error.set(doc.ErrorID(), doc.ErrorLineNum(), doc.ErrorStr());
	}

	root = doc.FirstChildElement(XmlName::TRITONN);
	if (!root) {
		return m_error.set(DATACFGERR_STRUCT, 0, "Is not tritonn-conf file");
	}

	if (TRITONN_RESULT_OK != LoadSecurity(root, doc_security)) {
		return m_error.getError();
	}

	if (TRITONN_RESULT_OK != loadHardware(root)) {
		return m_error.getError();
	}

	if (TRITONN_RESULT_OK != LoadConfig(root)) {
		return m_error.getError();
	}

	if (TRITONN_RESULT_OK != LoadUsers(XMLRootSecurity, m_json_usr)) {
		return m_error.getError();
	}

	// Вся конфигурация загруженна, расчитываем линки
	if (TRITONN_RESULT_OK != ResolveLinks()) {
		return m_error.getError();
	}

	if (checkSource() != TRITONN_RESULT_OK) {
		return m_error.getError();
	}

	// Находим нарастающие для отчетов
	if (TRITONN_RESULT_OK != ResolveReports()) {
		return m_error.getError();
	}

	// Загружаем строки и события
	if (TRITONN_RESULT_OK != LoadCustom(root)) {
		return m_error.getError();
	}

	// Загружаем интерфейсы
	if (TRITONN_RESULT_OK != LoadComms(root)) {
		return m_error.getError();
	}

	// Загружаем события

	// Заполним информацию по конфиге
	strncpy(SysVar->ConfigInfo.File, filename.c_str(), MAX_CONFIG_NAME);

	//TODO прока пропишем жестко русский язык
	strncpy(SysVar->Lang, LANG_RU.c_str(), MAX_LANG_SIZE);

	// Сохраняем информацию для WEB
	saveWeb();

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rDataConfig::LoadSecurity(tinyxml2::XMLElement* root, tinyxml2::XMLDocument& doc_security)
{
	tinyxml2::XMLElement* xml_crypt = root->FirstChildElement(XmlName::SECURITY);
	std::string aes_text = "";
	std::string xml_src  = "";

	if (!xml_crypt) {
		return m_error.set(DATACFGERR_SECURITY_NF, root->GetLineNum());
	}

	// Дешифруем блок пользователей
	aes_text = xml_crypt->GetText();
	aes_text = String_deletewhite(aes_text);

	if (DecryptEAS(aes_text, AES_KEY, AES_IV, xml_src)) {
		return m_error.set(DATACFGERR_SECURITY_DESCRYPT, xml_crypt->GetLineNum());
	}

	if (tinyxml2::XML_SUCCESS != doc_security.Parse(xml_src.c_str())) {
		return m_error.set(DATACFGERR_SECURITY_PARSE, xml_crypt->GetLineNum());
	}

	// Парсим разкодированный блок пользователей
	XMLRootSecurity = doc_security.FirstChildElement(XmlName::SECURITY);
	if (!XMLRootSecurity) {
		return m_error.set(DATACFGERR_SECURITY_PARSE, xml_crypt->GetLineNum());
	}

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rDataConfig::loadHardware(tinyxml2::XMLElement* root)
{
	tinyxml2::XMLElement* hardware_xml = root->FirstChildElement(XmlName::HARDWARE);

	if (!hardware_xml) {
		return m_error.set(DATACFGERR_NOTFOUND_HARDWARE, root->GetLineNum());
	}

	XML_FOR(module_xml, hardware_xml, XmlName::MODULE) {
		std::string    type   = XmlUtils::getAttributeString(module_xml, XmlName::TYPE, "");

		if (type == "") {
			return m_error.set(DATACFGERR_UNKNOWN_MODULE, module_xml->GetLineNum());
		}

		type = String_tolower(type);

		rIOBaseModule* module = rIOManager::instance().addModule(type);
		if (!module) {
			return m_error.set(DATACFGERR_UNKNOWN_MODULE, module_xml->GetLineNum());
		}

		if (module->loadFromXML(module_xml, m_error) != TRITONN_RESULT_OK) {
			return m_error.getError();
		}

		cJSON *jsrc = cJSON_CreateObject();
		cJSON *jitm = cJSON_CreateObject();
		cJSON_AddItemToObject(jsrc, XmlName::ALIAS , cJSON_CreateString((IO::HARWARE_PREFIX + module->getName()).c_str()));
		cJSON_AddItemToObject(jsrc, XmlName::MODULE, cJSON_CreateString(module->getModuleType().c_str()));
		cJSON_AddItemToObject(jsrc, XmlName::DESC  , cJSON_CreateNumber(module->getDescr()));
		cJSON_AddItemToObject(jitm, module->getModuleType().c_str(), jsrc);
		cJSON_AddItemToArray(m_json_hdw, jitm);
	}

	return TRITONN_RESULT_OK;
}


UDINT rDataConfig::LoadConfig(tinyxml2::XMLElement* root)
{
	tinyxml2::XMLElement* config = root->FirstChildElement(XmlName::CONFIG);
	cJSON* jstn = cJSON_CreateArray();

	if (!config) {
		return m_error.set(DATACFGERR_CONFIG, root->GetLineNum());
	}

	if (loadSettings(config) != TRITONN_RESULT_OK) {
		return m_error.getError();
	}

	if (loadIO(config, m_json_io, nullptr, "io") != TRITONN_RESULT_OK) {
		return m_error.getError();
	}

	if (TRITONN_RESULT_OK != loadCalc(config, m_json_obj, nullptr, "obj")) {
		return m_error.getError();
	}

	if (TRITONN_RESULT_OK != LoadVariable(config)) {
		return m_error.getError();
	}

	cJSON_AddItemToObject(m_json, XmlName::STATIONS, jstn);
	if (TRITONN_RESULT_OK != LoadStation(config, jstn)) {
		return m_error.getError();
	}

	if (TRITONN_RESULT_OK != LoadReport(config)) {
		return m_error.getError();
	}

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rDataConfig::loadIO(tinyxml2::XMLElement* root, cJSON* jroot, rStation* owner, const std::string& prefix)
{
	tinyxml2::XMLElement* xml_io = root->FirstChildElement(XmlName::IO);
	rSource*    source = nullptr;
	std::string name   = "";

   // Данного элемента нет в дереве
	if (nullptr == xml_io) {
		return TRITONN_RESULT_OK;
	}

	// Разбираем элементы
	for (tinyxml2::XMLElement *obj = xml_io->FirstChildElement(); obj != nullptr; obj = obj->NextSiblingElement()) {
		name   = obj->Name();
		source = nullptr;

		//TODO реализовать как в модулях IO
		if (XmlName::AI == name) { if(SysVar->m_max[name] >= MAX_IO_AI) return m_error.set(DATACFGERR_MAX_AI, 0); source = dynamic_cast<rSource*>(new rAI(owner));      }
		if (XmlName::FI == name) { if(SysVar->m_max[name] >= MAX_IO_FI) return m_error.set(DATACFGERR_MAX_FI, 0); source = dynamic_cast<rSource*>(new rCounter(owner)); }
		if (XmlName::DI == name) { if(SysVar->m_max[name] >= MAX_IO_DI) return m_error.set(DATACFGERR_MAX_DI, 0); source = dynamic_cast<rSource*>(new rDI(owner));      }
		if (XmlName::DO == name) { if(SysVar->m_max[name] >= MAX_IO_DO) return m_error.set(DATACFGERR_MAX_DO, 0); source = dynamic_cast<rSource*>(new rDO(owner));      }

		if (!source) {
			return m_error.set(DATACFGERR_UNKNOWIO, obj->GetLineNum());
		}

		source->m_ID = SysVar->m_max[name]++;

		if(source->loadFromXML(obj, m_error, prefix) != TRITONN_RESULT_OK) {
			return m_error.getError();
		}

		ListSource->push_back(source);

		cJSON *jsrc = cJSON_CreateObject();
		cJSON *jitm = cJSON_CreateObject();
		cJSON_AddItemToObject(jsrc, XmlName::ALIAS, cJSON_CreateString(source->m_alias.c_str()));
		cJSON_AddItemToObject(jsrc, XmlName::DESC , cJSON_CreateNumber(source->m_descr));
		cJSON_AddItemToObject(jitm, source->RTTI(), jsrc);
		cJSON_AddItemToArray(jroot, jitm);
	}

	return TRITONN_RESULT_OK;
}


//
UDINT rDataConfig::loadCalc(tinyxml2::XMLElement* root, cJSON* jroot, rStation* owner, const std::string& prefix)
{
	tinyxml2::XMLElement* calc = root->FirstChildElement(XmlName::CALC);
	rSource*    source = nullptr;
	std::string name   = "";

	// Данного элемента нет в дереве
	if(!calc) {
		return TRITONN_RESULT_OK;
	}

	// Разбираем элементы
	for(tinyxml2::XMLElement *obj = calc->FirstChildElement(); obj != nullptr; obj = obj->NextSiblingElement())
	{
		name   = obj->Name();
		source = nullptr;

		if (XmlName::DENSSOL     == name) { if (SysVar->m_max[name] >= MAX_DENSSOL    ) return m_error.set(DATACFGERR_MAX_DENSSOL  , 0); source = dynamic_cast<rSource*>(new rDensSol(owner));            }
		if (XmlName::REDUCEDDENS == name) { if (SysVar->m_max[name] >= MAX_REDUCEDDENS) return m_error.set(DATACFGERR_MAX_RDCDENS  , 0); source = dynamic_cast<rSource*>(new rReducedDens(owner));        }
		if (XmlName::MSELECTOR   == name) { if (SysVar->m_max[name] >= MAX_MSELECTOR  ) return m_error.set(DATACFGERR_MAX_SELECTOR , 0); source = dynamic_cast<rSource*>(new rSelector(owner));           }
		if (XmlName::SELECTOR    == name) { if (SysVar->m_max[name] >= MAX_SELECTOR   ) return m_error.set(DATACFGERR_MAX_SELECTOR , 0); source = dynamic_cast<rSource*>(new rSelector(owner));           }
		if (XmlName::SAMPLER     == name) { if (SysVar->m_max[name] >= MAX_SAMPLER    ) return m_error.set(DATACFGERR_MAX_SAMPLER  , 0); source = dynamic_cast<rSource*>(new rSampler(owner));            }
		if (XmlName::PROVE       == name) { if (SysVar->m_max[name] >= MAX_SAMPLER    ) return m_error.set(DATACFGERR_MAX_PROVE    , 0); source = dynamic_cast<rSource*>(new rProve(owner));              }
		if (XmlName::AVERAGE     == name) { if (SysVar->m_max[name] >= MAX_AVERAGE    ) return m_error.set(DATACFGERR_MAX_AVERAGE  , 0); source = dynamic_cast<rSource*>(new rAverage(owner));            }
		if (XmlName::MASSWATER   == name) { if (SysVar->m_max[name] >= MAX_MASSWATER  ) return m_error.set(DATACFGERR_MAX_MASSWATER, 0); source = dynamic_cast<rSource*>(new rMassWater(owner));          }
		if (XmlName::VOLWATER    == name) { if (SysVar->m_max[name] >= MAX_VOLWATER   ) return m_error.set(DATACFGERR_MAX_VOLWATER , 0); source = dynamic_cast<rSource*>(new rVolWater(owner));           }
		if (XmlName::CINVISC     == name) { if (SysVar->m_max[name] >= MAX_CINVISC    ) return m_error.set(DATACFGERR_MAX_CINVISC  , 0); source = dynamic_cast<rSource*>(new rCinematicViscosity(owner)); }

		if(!source) {
			return m_error.set(DATACFGERR_UNKNOWCALC, obj->GetLineNum(), name);
		}

		source->m_ID = SysVar->m_max[name]++;

		if(source->loadFromXML(obj, m_error, prefix) != TRITONN_RESULT_OK) {
			return m_error.getError();
		}

		ListSource->push_back(source);

		cJSON* jitm = cJSON_CreateObject();
		cJSON* jsrc = cJSON_CreateObject();
		cJSON_AddItemToObject(jsrc, XmlName::ALIAS, cJSON_CreateString(source->m_alias.c_str()));
		cJSON_AddItemToObject(jsrc, XmlName::DESC , cJSON_CreateNumber(source->m_descr));
		cJSON_AddItemToObject(jitm, source->RTTI(), jsrc);
		cJSON_AddItemToArray(jroot, jitm);
	}

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rDataConfig::LoadStation(tinyxml2::XMLElement *root, cJSON *jroot)
{
	tinyxml2::XMLElement* stations_xml = root->FirstChildElement(XmlName::STATIONS);

	// Данного элемента нет в дереве
	if(!stations_xml) {
		return m_error.set(DATACFGERR_STATIONSNF, root->GetLineNum());
	}

	// Разбираем элементы
	XML_FOR(station_xml, stations_xml, XmlName::STATION) {
		if (SysVar->m_max[XmlName::STATION] >= MAX_STATION) {
			return m_error.set(DATACFGERR_MAX_STATION, station_xml->GetLineNum());
		}

		rStation* stn = new rStation();

		stn->m_ID = SysVar->m_max[XmlName::STATION]++;

		if (TRITONN_RESULT_OK != stn->loadFromXML(station_xml, m_error, "")) {
			return m_error.getError();
		}

		cJSON *jitm = cJSON_CreateObject();
		cJSON *jstn = cJSON_CreateObject();
		cJSON *jobj = cJSON_CreateArray();
		cJSON *jstr = cJSON_CreateArray();
		cJSON_AddItemToObject(jstn, XmlName::ALIAS  , cJSON_CreateString(stn->m_alias.c_str()));
		cJSON_AddItemToObject(jstn, XmlName::DESC   , cJSON_CreateNumber(stn->m_descr));
		cJSON_AddItemToObject(jstn, XmlName::STREAMS, jstr);
		cJSON_AddItemToObject(jstn, XmlName::CALC   , jobj);
		cJSON_AddItemToObject(jitm, stn->RTTI()     , jstn);
		cJSON_AddItemToArray (jroot, jitm);

		if (TRITONN_RESULT_OK != loadIO(station_xml, m_json_io, stn, stn->m_alias + ".io")) {
			return m_error.getError();
		}

		if (TRITONN_RESULT_OK != loadCalc(station_xml, jobj, stn, stn->m_alias + ".obj")) {
			return m_error.getError();
		}

		if (TRITONN_RESULT_OK != loadStream(station_xml, jstr, stn, stn->m_alias)) {
			return m_error.getError();
		}

		ListSource->push_back(stn);
	}

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rDataConfig::loadStream(tinyxml2::XMLElement* root, cJSON* jroot, rStation* owner, const std::string& prefix)
{
	tinyxml2::XMLElement* streams = root->FirstChildElement(XmlName::STREAMS);

	// Данного элемента нет в дереве
	if (!streams) {
		return m_error.set(DATACFGERR_STREAMSNF, root->GetLineNum());
	}

	XML_FOR(stream_xml, streams, XmlName::STREAM) {
		if (SysVar->m_max[XmlName::STREAM] >= MAX_STREAM) {
			return m_error.set(DATACFGERR_MAX_STREAM, stream_xml->GetLineNum());
		}

		rStream* str = new rStream(owner);

		str->m_ID = SysVar->m_max[XmlName::STREAM]++;

		if (TRITONN_RESULT_OK != str->loadFromXML(stream_xml, m_error, prefix)) {
			return m_error.getError();
		}

		cJSON *jitm = cJSON_CreateObject();
		cJSON *jstr = cJSON_CreateObject();
		cJSON *jobj = cJSON_CreateArray();
		cJSON_AddItemToObject(jstr, XmlName::ALIAS, cJSON_CreateString(str->m_alias.c_str()));
		cJSON_AddItemToObject(jstr, XmlName::DESC , cJSON_CreateNumber(str->m_descr));
		cJSON_AddItemToObject(jstr, XmlName::CALC , jobj);
		cJSON_AddItemToObject(jitm, str->RTTI()  , jstr);
		cJSON_AddItemToArray (jroot, jitm);

		// Кросс-линк станции и линии
		owner->addStream(str);

		if (TRITONN_RESULT_OK != loadIO(stream_xml, m_json_io, owner, str->m_alias + ".io")) {
			return m_error.getError();
		}

		if (TRITONN_RESULT_OK != loadCalc(stream_xml, jobj, owner, str->m_alias + ".obj")) {
			return m_error.getError();
		}

		ListSource->push_back(str);
	}

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rDataConfig::LoadReport(tinyxml2::XMLElement* root)
{
	tinyxml2::XMLElement* reportsystem = root->FirstChildElement(XmlName::REPORTSYSTEM);

	// Данного элемента нет в дереве
	if (!reportsystem) {
		return TRITONN_RESULT_OK;
	}

	tinyxml2::XMLElement* reports = reportsystem->FirstChildElement(XmlName::REPORTS);
	if (!reports) {
		return m_error.set(DATACFGERR_NOREPORTS, root->GetLineNum());
	}

	XML_FOR(report_xml, reports, XmlName::REPORT) {
		rReport* rpt = new rReport();

		if (TRITONN_RESULT_OK != rpt->loadFromXML(report_xml, m_error, "")) {
			return m_error.getError();
		}
	}

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rDataConfig::LoadCustom(tinyxml2::XMLElement* root)
{
	tinyxml2::XMLElement* custom    = root->FirstChildElement(XmlName::CUSTOM);
	tinyxml2::XMLElement* userstr   = nullptr;
//	tinyxml2::XMLElement* userevent = nullptr;
	tinyxml2::XMLElement* precision = nullptr;

	if (!custom) {
		return TRITONN_RESULT_OK;
	}

	userstr   = custom->FirstChildElement(XmlName::STRINGS);
//	userevent = custom->FirstChildElement(XmlName::EVENTS);
	precision = custom->FirstChildElement(XmlName::PRECISION);

	if (userstr) {
		if (TRITONN_RESULT_OK != rTextManager::instance().load(userstr, m_error)) {
			return m_error.getError();
		}
	}

	if (precision) {
		if (TRITONN_RESULT_OK != rPrecision::instance().load(precision, m_error)) {
			return m_error.getError();
		}
	}


	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
UDINT rDataConfig::LoadVariable(tinyxml2::XMLElement *root)
{
	tinyxml2::XMLElement* xml_vars = root->FirstChildElement(XmlName::VARIABLES);

	// Данного элемента нет в дереве
	if (nullptr == xml_vars) {
		return TRITONN_RESULT_OK;
	}

	XML_FOR(xml_var, xml_vars, XmlName::VARIABLE) {
		rSource* source = new rRVar();

		if (TRITONN_RESULT_OK != source->loadFromXML(xml_var, m_error, "var")) {
			delete source;

			return m_error.getError();
		}

		ListSource->push_back(source);

		cJSON *jsrc = cJSON_CreateObject();
		cJSON_AddItemToObject(jsrc, XmlName::ALIAS, cJSON_CreateString(source->m_alias.c_str()));
		cJSON_AddItemToObject(jsrc, XmlName::DESC , cJSON_CreateNumber(source->m_descr));
		cJSON_AddItemToArray(m_json_var, jsrc);
	}

	return TRITONN_RESULT_OK;
}



UDINT rDataConfig::LoadUsers(tinyxml2::XMLElement* root, cJSON* jroot)
{
	UNUSED(jroot);
	tinyxml2::XMLElement* xml_users = nullptr;

	xml_users = root->FirstChildElement(XmlName::USERS);
	if (!xml_users) {
		return m_error.set(DATACFGERR_USERS_PARSE, 0, "");
	}

	XML_FOR(xml_user, xml_users, XmlName::USER) {
		tinyxml2::XMLElement* xml_password = xml_user->FirstChildElement(XmlName::PASSWORD);
		tinyxml2::XMLElement* xml_comms    = xml_user->FirstChildElement(XmlName::COMMS);
		tinyxml2::XMLElement* xml_right    = xml_user->FirstChildElement(XmlName::RIGHTS);

		if (!xml_user->Attribute(XmlName::NAME) || !xml_password || !xml_right || !xml_comms) {
			return m_error.set(DATACFGERR_USERS_PARSE, 0, "");
		}

		tinyxml2::XMLElement* xml_intaccess = xml_right->FirstChildElement(XmlName::INTERNAL);
		tinyxml2::XMLElement* xml_extaccess = xml_right->FirstChildElement(XmlName::EXTERNAL);
		tinyxml2::XMLElement* xml_ilogin    = xml_comms->FirstChildElement(XmlName::LOGIN);    // Интерфейс. Логин
		tinyxml2::XMLElement* xml_ipwd      = xml_comms->FirstChildElement(XmlName::PASSWORD); // Интерфейс. Пароль

		if (!xml_intaccess || !xml_extaccess || !xml_ipwd || !xml_ilogin) {
			return m_error.set(DATACFGERR_USERS_PARSE, 0, "");
		}

		// Interface
		USINT  pwd_hash [MAX_HASH_SIZE] = {0};
		USINT  ipwd_hash[MAX_HASH_SIZE] = {0};
		string name      = "";
		UDINT  ilogin    = 0;
		UDINT  intaccess = 0;
		UDINT  extaccess = 0;

		if(xml_ilogin->GetText()) {
			ilogin = atoi(xml_ilogin->GetText());

			if (!xml_ipwd->GetText() && ilogin) {
				return m_error.set(DATACFGERR_USERS_PARSE, 0, "");
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

		cJSON* jsrc = cJSON_CreateObject();
		cJSON_AddItemToObject(jsrc, XmlName::NAME, cJSON_CreateString(name.c_str()));
		cJSON_AddItemToArray(m_json_usr, jsrc);
	}

	return TRITONN_RESULT_OK;
}




UDINT rDataConfig::LoadComms(tinyxml2::XMLElement *root)
{
	tinyxml2::XMLElement *xml_comms  = nullptr;

	if (!root) {
		return TRITONN_RESULT_OK;
	}

	xml_comms = root->FirstChildElement(XmlName::COMMS);

	if (!xml_comms) {
		return TRITONN_RESULT_OK;
	}

	if (LoadModbus(xml_comms) != TRITONN_RESULT_OK) {
		return m_error.getError();
	}

	if (LoadOPCUA(xml_comms) != TRITONN_RESULT_OK) {
		return m_error.getError();
	}

	return TRITONN_RESULT_OK;
}


UDINT rDataConfig::LoadModbus(tinyxml2::XMLElement* root)
{
	if (!root) {
		return TRITONN_RESULT_OK;
	}

	auto xml_modbus = root->FirstChildElement(XmlName::MODBUS);

	if (!xml_modbus) {
		return TRITONN_RESULT_OK;
	}

	XML_FOR_ALL(xml_item, xml_modbus) {
		std::string name = (xml_item->Name() == nullptr) ? "" : xml_item->Name();

		if(XmlName::SLAVETCP == name) { if(LoadModbusTCP(xml_item) != TRITONN_RESULT_OK) return m_error.getError(); }
		else {
			//TODO добавить ошибку
		}
	}

	return TRITONN_RESULT_OK;
}



UDINT rDataConfig::LoadModbusTCP(tinyxml2::XMLElement* root)
{
	auto slavetcp = new rModbusTCPSlaveManager();

	if(slavetcp->loadFromXML(root, m_error) != TRITONN_RESULT_OK) {
		delete slavetcp;

		return m_error.getError();
	}

	slavetcp->Pause.Set(-1);
	ListInterface->push_back(slavetcp);

	return TRITONN_RESULT_OK;
}



UDINT rDataConfig::LoadOPCUA(tinyxml2::XMLElement *root)
{
	tinyxml2::XMLElement* xml_opcua = nullptr;

	if (!root) {
		return TRITONN_RESULT_OK;
	}

	xml_opcua = root->FirstChildElement(XmlName::OPCUA);

	if(!xml_opcua) {
		return TRITONN_RESULT_OK;
	}

	rOPCUAManager* opcua = new rOPCUAManager();

	if (opcua->loadFromXML(xml_opcua, m_error) != TRITONN_RESULT_OK) {
		delete opcua;

		return m_error.getError();
	}

	opcua->Pause.Set(-1);
	ListInterface->push_back(opcua);

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
// Основная особенность функции LoadLink от LoadShadowLink в том, что на вход в первую подается
// указатель на сам тег линка, а во вторую указатель на родителя
//TODO можно также реализовать загрузку линка, где прописана просто числовая константа
UDINT rDataConfig::LoadLink(tinyxml2::XMLElement* element, rLink& link, bool required)
{
	link.m_source = nullptr;

	if (!element) {
		if (!required) {
			return TRITONN_RESULT_OK;
		}
		return m_error.set(DATACFGERR_LINKNF, link.m_lineNum, String_format("source '%s' is null", link.m_fullTag.c_str()));
	}

	if (TRITONN_RESULT_OK != link.loadFromXML(element, m_error, "")) {
		return m_error.getError();
	}

	link.m_lineNum = element->GetLineNum();

	ListLink.push_back(&link);

	return tinyxml2::XML_SUCCESS;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rDataConfig::LoadShadowLink(tinyxml2::XMLElement *element, rLink &link, rLink &mainlink, const string &name)
{
	if(element) {
		std::string aaa = element->Name();
		if (String_tolower(element->Name()) == XmlName::LINK) {
			return LoadLink(element, link);
		} else {
			return LoadLink(element->FirstChildElement(XmlName::LINK), link);
		}
	}

	link.m_alias   = mainlink.m_alias;
	link.m_fullTag = mainlink.m_alias + ":" + name;
	link.m_param   = name;
	link.m_lineNum = mainlink.m_lineNum;

	ListLink.push_back(&link);

	return tinyxml2::XML_SUCCESS;
}


const rSource* rDataConfig::getSource(const std::string& alias)
{
	for (auto item : *ListSource) {
		if (item->m_alias == alias) {
			return item;
		}
	}

	return nullptr;
}


UDINT rDataConfig::checkSource(void)
{
	for (auto  item : *ListSource) {
		if (item->check(m_error) != TRITONN_RESULT_OK) {
			return m_error.getError();
		}
	}

	return TRITONN_RESULT_OK;
}

//
UDINT rDataConfig::ResolveLinks(void)
{
	for (auto link : ListLink) {
		// Проходим все загруженные объекты
		for (auto src : *ListSource) {
			volatile std::string src_alias  = src->m_alias;
			volatile std::string link_alias = link->m_alias;

			if (!strcasecmp(link->m_alias.c_str(), src->m_alias.c_str())) {
				if (link->m_param.size()) {
					if (src->checkOutput(link->m_param)) {
						src->checkOutput(link->m_param);
						return m_error.set(DATACFGERR_CHECKLINK, link->m_lineNum, link->m_fullTag);
					}
				}

				link->m_source = src;
				break;
			}
		}

		if (!link->isValid()) {
			return m_error.set(DATACFGERR_RESOLVELINK, link->m_lineNum, link->m_fullTag);
		}
	}

	return TRITONN_RESULT_OK;
}


UDINT rDataConfig::ResolveReports(void)
{
	for (auto reports: *ListReport) {
		rReport::rDataset *rpt = &reports->m_present;

		for (auto tot: rpt->m_averageItems) {
			for(auto scr: *ListSource) {
				const rTotal *scrtot = scr->getTotal();

				if (!scrtot) {
					continue;
				}

				if (tot->m_alias == scr->m_alias) {
					tot->m_source = scrtot;
					break;
				}
			}

			if (!tot->m_source) {
				return m_error.set(DATACFGERR_RESOLVETOTAL, reports->m_lineNum, tot->m_name); //TODO добавить номер линии
			}
		}
	}

	return TRITONN_RESULT_OK;
}


UDINT rDataConfig::loadSettings(tinyxml2::XMLElement* root)
{
	if (!root) {
		return TRITONN_RESULT_OK;
	}

	auto xml_settings = root->FirstChildElement(XmlName::SETTINGS);

	if (!xml_settings) {
		return TRITONN_RESULT_OK;
	}

	UDINT fault = 0;

	rEventManager::instance().setStorage(
				XmlUtils::getTextUDINT(xml_settings->FirstChildElement(XmlName::EVENTSTORAGE),
									   rEventManager::instance().getStorage(),
									   fault));

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
void rDataConfig::saveWeb()
{
#ifdef WIN32
	return;
#endif

	const std::string begin_php = "<?php\n";
	char* str = cJSON_Print(m_json);
	UDINT result = SimpleFileSave(FILE_WWW_TREE_OBJ, str);

	free(str);
	if(TRITONN_RESULT_OK != result)
	{
		rEventManager::instance().addEventUDINT(EID_SYSTEM_FILEIOERROR, HALT_REASON_WEBFILE | result);
		TRACEP(LOG::CONFIG, "Can't save json tree");

		rDataManager::instance().DoHalt(HALT_REASON_WEBFILE | result);
		return;
	}

	//TODO Нужно вначале удалить все языковые файлы для web

	// Precision
	std::string text = begin_php;
	for (UDINT ii = 0; ii < MAX_UNITS_COUNT; ++ii) {
		text += String_format("$precision[\"core_%u\"]=%u;\n", ii, rPrecision::instance().get(ii));
	}

	result = SimpleFileSave(FILE_WWW_PRECISION, text);
	if (TRITONN_RESULT_OK != result) {
		rEventManager::instance().addEventUDINT(EID_SYSTEM_FILEIOERROR, HALT_REASON_WEBFILE | result);
		TRACEP(LOG::CONFIG, "Can't save precision file");

		rDataManager::instance().DoHalt(HALT_REASON_WEBFILE | result);
		return;
	}

	// Сохраняем массив строк
	std::vector<std::string> lang_list;
	std::vector<rTextItem>   sid_list;
	std::vector<rTextItem>   event_list;

	rTextManager::instance().getListLang(lang_list);

	for (auto& lang : lang_list) {
		UDINT result = TRITONN_RESULT_OK;

		rTextManager::instance().getListSID(lang, sid_list);
		rEventManager::instance().getTextClass().getListSID(lang, event_list);

		std::string text = begin_php;

		for(auto& item : sid_list) {
			text += String_format("$lang[\"core_%u\"]=\"%s\";\n", item.ID, item.Text.c_str());
		}

		std::string filename = DIR_WWW_LANG + lang + "/" + FILE_WWW_LANG;
		result = SimpleFileSave(filename, text);
		if (TRITONN_RESULT_OK != result) {
			rEventManager::instance().addEventUDINT(EID_SYSTEM_FILEIOERROR, HALT_REASON_WEBFILE | result);
			TRACEP(LOG::CONFIG, "Can't save sid file");

			rDataManager::instance().DoHalt(HALT_REASON_WEBFILE | result);
			return;
		}

		text = begin_php;

		for(auto& item : event_list) {
			text += String_format("$lang[\"event_%u\"]=\"%s\";\n", item.ID, item.Text.c_str());
		}

		filename = DIR_WWW_LANG + lang + "/" + FILE_WWW_EVENT;
		result = SimpleFileSave(filename, text);
		if(TRITONN_RESULT_OK != result) {
			rEventManager::instance().addEventUDINT(EID_SYSTEM_FILEIOERROR, HALT_REASON_WEBFILE | result);
			TRACEP(LOG::CONFIG, "Can't save event file");

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


