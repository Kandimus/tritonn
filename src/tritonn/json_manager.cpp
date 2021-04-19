//=================================================================================================
//===
//=== json_manager.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс-поток для общения с json-клиентом
//===
//=================================================================================================

#include <string.h>
#include "cJSON.h"
#include "time64.h"
#include "stringex.h"
#include "hash.h"
#include "log_manager.h"
#include "data_manager.h"
#include "variable_item.h"
#include "data_snapshot_item.h"
#include "data_snapshot.h"
#include "users.h"
#include "listconf.h"
#include "json_client.h"
#include "json_manager.h"


extern rVariable *gVariable;


const char JSONSTR_ACHECK[]    = "a.check";
const char JSONSTR_ALOGIN[]    = "a.login";
const char JSONSTR_ALOGOUT[]   = "a.logout";
const char JSONSTR_ACCESS[]    = "access";
const char JSONSTR_ACCEDIT[]   = "accessedit";
const char JSONSTR_ACCVIS[]    = "accessvisible";
const char JSONSTR_ALARM[]     = "alarm";
const char JSONSTR_BUILD[]     = "build";
const char JSONSTR_COMMAND[]   = "command";
const char JSONSTR_CONFIG[]    = "config";
const char JSONSTR_CRC[]       = "crc";
const char JSONSTR_DATAGET[]   = "data.get";
const char JSONSTR_DATASET[]   = "data.set";
const char JSONSTR_DATA[]      = "data";
const char JSONSTR_DAY[]       = "day";
const char JSONSTR_DEV[]       = "dev";
const char JSONSTR_ERROR[]     = "error";
const char JSONSTR_ERRORID[]   = "error_id";
const char JSONSTR_ERRORMSG[]  = "error_msg";
const char JSONSTR_FILE[]      = "file";
const char JSONSTR_HASH[]      = "hash";
const char JSONSTR_HOUR[]      = "hour";
const char JSONSTR_ICONF[]     = "i.conf";
const char JSONSTR_ILISTCONF[] = "i.listconf";
const char JSONSTR_IRESTART[]  = "i.restart";
const char JSONSTR_ISTATUS[]   = "i.status";
const char JSONSTR_IDS[]       = "ids";
const char JSONSTR_ID[]        = "id";
const char JSONSTR_LIST[]      = "list";
const char JSONSTR_LIVE[]      = "live";
const char JSONSTR_LOCAL[]     = "local";
const char JSONSTR_MAJOR[]     = "major";
const char JSONSTR_MIN[]       = "min";
const char JSONSTR_MINOR[]     = "minor";
const char JSONSTR_MON[]       = "mon";
const char JSONSTR_NAME[]      = "name";
const char JSONSTR_PATCH[]     = "patch";
const char JSONSTR_PWD[]       = "pwd";
const char JSONSTR_RESULT[]    = "result";
const char JSONSTR_REQ[]       = "request";
const char JSONSTR_RESPONSE[]  = "response";
const char JSONSTR_RESTART[]   = "restart";
const char JSONSTR_STATUS[]    = "status";
const char JSONSTR_SEC[]       = "sec";
const char JSONSTR_SUCCESS[]   = "success";
const char JSONSTR_TIME[]      = "time";
const char JSONSTR_TOKEN[]     = "token";
const char JSONSTR_TS[]        = "ts";
const char JSONSTR_USERNAME[]  = "username";
const char JSONSTR_VALUE[]     = "value";
const char JSONSTR_VERSION[]   = "version";
const char JSONSTR_YEAR[]      = "year";


rActivity::rActivity()
{
	srand(time(NULL));
	Token   = rand();
	Idle    = time(nullptr);
	Session = Idle;
}


void rActivity::UpdateIdle()
{
	Idle = time(nullptr);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//
// КОНСТРУКТОРЫ И ДЕСТРУКТОР
rJSONManager::rJSONManager()
	: rTCPClass("0.0.0.0", LanPort::PORT_JSON, MAX_JSON_CLIENT) //TODO После тестов установить localhost 127.0.0.1
{
	RTTI       = "rJSONManager";
	LogMask   |= LOG::TERMINAL;
}


rJSONManager::~rJSONManager()
{
	;
}


//-------------------------------------------------------------------------------------------------
//
rJSONManager &rJSONManager::Instance()
{
	static rJSONManager Singleton;

	return Singleton;
}




//-------------------------------------------------------------------------------------------------
//
rThreadStatus rJSONManager::Proccesing()
{
	rThreadStatus thread_status = rThreadStatus::UNDEF;
	UDINT curtime = 0;

	while(1)
	{
		// Обработка команд нити
		thread_status = rTCPClass::Proccesing();
		if(!THREAD_IS_WORK(thread_status))
		{
			return thread_status;
		}

		// Удаление просроченных токенов
		curtime = time(nullptr);

		for(UDINT ii = 0; ii < Activity.size(); ++ii)
		{
			if((curtime - Activity[ii].Idle > MAX_JSON_IDLE) || (curtime - Activity[ii].Session > MAX_JSON_SESSION))
			{
				Activity.erase(Activity.begin() + ii);
				--ii;
			}
		}

		// Удаление лишних клиентов
		while(Activity.size() > MAX_JSON_CLIENT)
		{
			Activity.erase(Activity.begin());
		}

		rThreadClass::EndProccesing();
	}

	return rThreadStatus::UNDEF;
}


rClientTCP *rJSONManager::NewClient(SOCKET socket, sockaddr_in *addr)
{
	return (rClientTCP *)new rJSONClient(socket, addr);
}


UDINT rJSONManager::ClientRecv(rClientTCP *client, USINT *buff, UDINT size)
{
	rJSONClient *jsonclient = (rJSONClient *)client;
	USINT       *data    = jsonclient->Recv(buff, size);
	UDINT        result  = 0;
	string       answe   = "";

	if(JSONCLNT_RECV_ERROR == data)
	{
		unsigned char *ip = (unsigned char *)&client->Addr.sin_addr.s_addr;

		answe = GetErrorJSON(JSONERR_UNDEF, "Can't received JSON");

		TRACEW(LogMask, "JSON manager: Client [%i.%i.%i.%i] send unknow packet", ip[0], ip[1], ip[2], ip[3]);

		return 0;
	}

   // Посылку считали не полностью
	if(JSONCLNT_RECV_ERROR == data)
	{
		return 1;
	}

	// Обрабатываем посылку
	answe = ParsingJSON(jsonclient->Packet);

	Send(client, (char *)answe.c_str(), answe.size());

	return result;
}


rActivity *rJSONManager::GetActivity(DINT token)
{
	for(UDINT ii = 0; ii < Activity.size(); ++ii)
	{
		if(Activity[ii].Token == token)
		{
			return &Activity[ii];
		}
	}

	return nullptr;
}


UDINT rJSONManager::DeleteActivity(DINT token)
{
	for(UDINT ii = 0; ii < Activity.size(); ++ii)
	{
		if(Activity[ii].Token == token)
		{
			Activity.erase(Activity.begin() + ii);
			return 1;
		}
	}

	return 0;
}


string rJSONManager::JSONtoString(cJSON *root, int deleteobj)
{
	if(nullptr == root) return "";

	char   *str    = cJSON_PrintUnformatted(root);
	string  result = str;

	free(str);
	if(deleteobj) cJSON_Delete(root);

	return result;
}


//-------------------------------------------------------------------------------------------------
void rJSONManager::CreateErrorJSON(cJSON *root, JSON_ERROR err, const string &errmsg)
{
	cJSON  *error = nullptr;

	if(nullptr == root) return;

	error = cJSON_CreateObject();

	cJSON_AddItemToObject(root , JSONSTR_ERROR   , error);
	cJSON_AddItemToObject(error, JSONSTR_ERRORID , cJSON_CreateNumber(err));
	cJSON_AddItemToObject(error, JSONSTR_ERRORMSG, cJSON_CreateString(errmsg.c_str()));
}

//-------------------------------------------------------------------------------------------------
//
string rJSONManager::GetErrorJSON(JSON_ERROR err, const string &errmsg)
{
	cJSON  *root = cJSON_CreateObject();

	CreateErrorJSON(root, err, errmsg);
	return JSONtoString(root, true);
}


//-------------------------------------------------------------------------------------------------
//
string rJSONManager::ParsingJSON(const char *text)
{
	cJSON      *root   = cJSON_Parse(text);
	cJSON      *req    = nullptr;
	string      result = "";

	if(nullptr == root)
	{
		string ErrMsg = "";
		const char *error_ptr = cJSON_GetErrorPtr();

		if(error_ptr != NULL)
		{
			fprintf(stderr, "Error after: %s, size %i, first byte 0x%0X\n", error_ptr, strlen(text), text[0]);
			ErrMsg = String_format("Parse error after '%s'", error_ptr);
		}
		else
		{
			ErrMsg = "Unknow parse error";
		}

		if(text[0] != '{')
		{
			ErrMsg = "First char is not open brace";
		}
		else if(text[strlen(text) - 1] != '}')
		{
			ErrMsg = "Last char is not close brace";
		}


		return GetErrorJSON(JSONERR_BADREQ, ErrMsg);
	}

	req = cJSON_GetObjectItem(root, "request");


	if(req)
	{
		result = Packet_REQ(req);
	}
	else
	{
		result = GetErrorJSON(JSONERR_UNKNOWMETHOD, "request not found");
	}

	cJSON_Delete(root);

	return result;
}





//
string rJSONManager::Packet_REQ(cJSON *root)
{
	cJSON *jcmd = cJSON_GetObjectItem(root, JSONSTR_COMMAND);

	if(nullptr == jcmd) return GetErrorJSON(JSONERR_UNKNOWMETHOD, "command not found");

	//
	if(!strcmp(jcmd->valuestring, JSONSTR_DATAGET))
	{
		return Packet_DataGet(root);
	}
	else if(!strcmp(jcmd->valuestring, JSONSTR_DATASET))
	{
		return Packet_DataSet(root);
	}
	else if(!strcmp(jcmd->valuestring, JSONSTR_ALOGIN))
	{
		return Packet_Login(root);
	}
	else if(!strcmp(jcmd->valuestring, JSONSTR_ALOGOUT))
	{
		return Packet_Logout(root);
	}
	else if(!strcmp(jcmd->valuestring, JSONSTR_ACHECK))
	{
		return Packet_Check(root);
	}

	else if(!strcmp(jcmd->valuestring, JSONSTR_ISTATUS))
	{
		return Packet_Status(root);
	}
	else if(!strcmp(jcmd->valuestring, JSONSTR_ICONF))
	{
		return Packet_Conf(root);
	}
	else if(!strcmp(jcmd->valuestring, JSONSTR_ILISTCONF))
	{
		return Packet_ListConf(root);
	}
	else if(!strcmp(jcmd->valuestring, JSONSTR_IRESTART))
	{
		return Packet_Restart(root);
	}

	return GetErrorJSON(JSONERR_UNKNOWMETHOD, "unknow command");
}


//-------------------------------------------------------------------------------------------------
//
string rJSONManager::Packet_DataGet(cJSON *root)
{
	rSnapshot  ss(rDataManager::instance().getVariableClass());
//	cJSON     *jtoken   = cJSON_GetObjectItem(root, JSONSTR_TOKEN);
	cJSON     *jids     = cJSON_GetObjectItem(root, JSONSTR_IDS);
//	rActivity *act      = nullptr;
	// Ответ
	cJSON     *answe    = cJSON_CreateObject();
	cJSON     *response = cJSON_CreateObject();
	cJSON     *data     = nullptr;
	string     ts       = "";
	UDT udt;

	gettimeofday(&udt, NULL);
	ts       = String_format("%L", udt.tv_sec);

	cJSON_AddItemToObject(answe   , JSONSTR_RESPONSE, response);
	cJSON_AddItemToObject(response, JSONSTR_COMMAND , cJSON_CreateString(JSONSTR_DATAGET));
	cJSON_AddItemToObject(response, JSONSTR_TS      , cJSON_CreateString(ts.c_str()));

	if(nullptr == jids/* || nullptr == jtoken*/)
	{
		cJSON_AddItemToObject(response, JSONSTR_SUCCESS , cJSON_CreateFalse());
		CreateErrorJSON(response, JSONERR_BADPARAM, "");
		return JSONtoString(answe , true);
	}

	// Проверка факта входа пользователя
//	act = GetActivity(jtoken->valueint);
//	if(nullptr == act)
//	{
//		cJSON_AddItemToObject(response, JSONSTR_SUCCESS , cJSON_CreateFalse());
//		CreateErrorJSON(response, JSONERR_TOKEN_FAULT, "");
//		return JSONtoString(answe , true);
//	}

	cJSON_AddItemToObject(response, JSONSTR_SUCCESS , cJSON_CreateTrue());
	cJSON_AddItemToObject(response, JSONSTR_DATA    , data = cJSON_CreateArray());

	// Заполняем snapshot требуемыми переменными
	for(DINT ii = 0; ii < cJSON_GetArraySize(jids); ++ii)
	{
		cJSON *jvar = cJSON_GetArrayItem(jids, ii);

		ss.add(jvar->valuestring);
	}
	ss.get();

	// Заполняем ответ
	for(DINT ii = 0; ii < cJSON_GetArraySize(jids); ++ii)
	{
		rSnapshotItem   *item = ss[ii];
		const rVariable *var = item->getVariable();

		cJSON *avar = cJSON_CreateObject();
		cJSON *jvar = cJSON_GetArrayItem(jids, ii);

		cJSON_AddItemToObject(avar, JSONSTR_ID     , cJSON_CreateString(jvar->valuestring));
		cJSON_AddItemToObject(avar, JSONSTR_VALUE  , cJSON_CreateNumber(item->isAssigned() ? item->getValueLREAL() : 0.0));
		cJSON_AddItemToObject(avar, JSONSTR_ACCVIS , cJSON_CreateBool  (item->isAssigned() ? 1 : 0));
		cJSON_AddItemToObject(avar, JSONSTR_ACCEDIT, cJSON_CreateBool  (item->isAssigned() ? !var->isReadonly() : 0));
		cJSON_AddItemToObject(avar, JSONSTR_SUCCESS, cJSON_CreateBool  (item->isAssigned()  ? 1 : 0));

		cJSON_AddItemToArray(data, avar);
	}

	return JSONtoString(answe, true);
}



string rJSONManager::Packet_DataSet(cJSON *root)
{
	rSnapshot  ss(rDataManager::instance().getVariableClass());
	rActivity *act      = nullptr;
	cJSON     *jids     = cJSON_GetObjectItem(root, JSONSTR_IDS);
	cJSON     *jtoken   = cJSON_GetObjectItem(root, JSONSTR_TOKEN);
	cJSON     *jlocal   = cJSON_GetObjectItem(root, JSONSTR_LOCAL);
	// Ответ
	cJSON     *answe    = cJSON_CreateObject();
	cJSON     *response = cJSON_CreateObject();
	cJSON     *data     = nullptr;
	string     ts       = String_format("%L", timegm64(NULL));

	// Собираем ответ
	cJSON_AddItemToObject(answe   , JSONSTR_RESPONSE, response);
	cJSON_AddItemToObject(response, JSONSTR_TS      , cJSON_CreateString(ts.c_str()));
	cJSON_AddItemToObject(response, JSONSTR_COMMAND , cJSON_CreateString(JSONSTR_DATASET));

	if(nullptr == jids || nullptr == jtoken)
	{
		cJSON_AddItemToObject(response, JSONSTR_SUCCESS , cJSON_CreateFalse());
		CreateErrorJSON(response, JSONERR_BADPARAM, "");
		return JSONtoString(answe , true);
	}

	// Проверка на вход пользователя
	act = GetActivity(jtoken->valueint);
	if(nullptr == act)
	{
		cJSON_AddItemToObject(response, JSONSTR_SUCCESS , cJSON_CreateFalse());
		CreateErrorJSON(response, JSONERR_TOKEN_FAULT, "");
		return JSONtoString(answe , true);
	}

	// Пользователь авторизирован
	cJSON_AddItemToObject(response, JSONSTR_SUCCESS , cJSON_CreateTrue());
	cJSON_AddItemToObject(response, JSONSTR_DATA    , data = cJSON_CreateArray());

	UDINT local = (nullptr == jlocal) ? true : jlocal->valueint;

	ss.setAccess(act->User->GetAccess(local));

	// Заполняем snapshot требуемыми переменными
	for(DINT ii = 0; ii < cJSON_GetArraySize(jids); ++ii)
	{
		cJSON *jvar   = cJSON_GetArrayItem(jids, ii);
		cJSON *jname  = cJSON_GetObjectItem(jvar, JSONSTR_ID);
		cJSON *jvalue = cJSON_GetObjectItem(jvar, JSONSTR_VALUE);

		ss.add(jname->valuestring, string(jvalue->valuestring));
	}
	ss.set();

	// Разбираем запрос
	for(DINT ii = 0; ii < cJSON_GetArraySize(jids); ++ii)
	{
		cJSON  *avar   = cJSON_CreateObject();
		cJSON  *jvar   = cJSON_GetArrayItem(jids, ii);
		cJSON  *jname  = cJSON_GetObjectItem(jvar, JSONSTR_ID);
		cJSON  *jvalue = cJSON_GetObjectItem(jvar, JSONSTR_VALUE);

		cJSON_AddItemToObject(avar, JSONSTR_ID     , cJSON_CreateString(jname->valuestring));
		cJSON_AddItemToObject(avar, JSONSTR_VALUE  , cJSON_CreateString(jvalue->valuestring));
		cJSON_AddItemToObject(avar, JSONSTR_SUCCESS, cJSON_CreateBool  (ss[ii]->isWrited()));
		cJSON_AddItemToObject(avar, JSONSTR_RESULT , cJSON_CreateNumber(ss[ii]->getStatus()));

		cJSON_AddItemToArray(data, avar);
	}

	return JSONtoString(answe, true);
}


//-------------------------------------------------------------------------------------------------
//
string rJSONManager::Packet_Login(cJSON *root)
{
	cJSON     *juser    = cJSON_GetObjectItem(root, JSONSTR_USERNAME);
	cJSON     *jpwd     = cJSON_GetObjectItem(root, JSONSTR_PWD);
	UDINT      result   = 0;
	USINT      pwd[MAX_HASH_SIZE];
	rActivity  act;
	// ответ
	cJSON     *answe    = cJSON_CreateObject();
	cJSON     *response = cJSON_CreateObject();
	cJSON     *data     = nullptr;

	cJSON_AddItemToObject(answe   , JSONSTR_RESPONSE, response);
	cJSON_AddItemToObject(response, JSONSTR_COMMAND , cJSON_CreateString(JSONSTR_ALOGIN));


	if(juser == nullptr || jpwd == nullptr)
	{
		cJSON_AddItemToObject(response, JSONSTR_SUCCESS , cJSON_CreateFalse());
		CreateErrorJSON(response, JSONERR_BADPARAM, "");

		return JSONtoString(answe, true);
	}

	// Преобразуем хеш пароля
	String_ToBuffer(jpwd->valuestring, pwd, MAX_HASH_SIZE);

	// Попытка входа
	act.User = rUser::Login(juser->valuestring, pwd, result);

	// Не удачный вход
	if(nullptr == act.User)
	{
		cJSON_AddItemToObject(response, JSONSTR_SUCCESS , cJSON_CreateFalse());
		CreateErrorJSON(response, result == LOGIN_BLOCKED ? JSONERR_LOGIN_BLOCKED : JSONERR_LOGIN_FAULT, "");

		return JSONtoString(answe, true);
	}

	// Пользователю вход через WEB запрещен
	if(0 == (act.User->GetAccess() & ACCESS_WEB))
	{
		cJSON_AddItemToObject(response, JSONSTR_SUCCESS , cJSON_CreateFalse());
		CreateErrorJSON(response, JSONERR_LOGIN_FAULT, "");

		return JSONtoString(answe, true);
	}

	data = cJSON_CreateObject();

	cJSON_AddItemToObject(response, JSONSTR_SUCCESS, cJSON_CreateTrue());
	cJSON_AddItemToObject(response, JSONSTR_DATA   , data);
	cJSON_AddItemToObject(data    , JSONSTR_TOKEN  , cJSON_CreateNumber(act.Token));
	cJSON_AddItemToObject(data    , JSONSTR_ACCESS , cJSON_CreateNumber(act.User->GetAccess()));
	cJSON_AddItemToObject(data    , JSONSTR_RESULT , cJSON_CreateNumber(LOGIN_CHANGEPWD == result ? JSONERR_LOGIN_CHANGEPWD : JSONERR_LOGIN_OK));

	Activity.push_back(act);

	return JSONtoString(answe, true);
}


//-------------------------------------------------------------------------------------------------
//
string rJSONManager::Packet_Logout(cJSON *root)
{
	cJSON     *jtoken   = cJSON_GetObjectItem(root, JSONSTR_TOKEN);
	// ответ
	cJSON     *answe    = cJSON_CreateObject();
	cJSON     *response = cJSON_CreateObject();
	cJSON     *data     = nullptr;

	cJSON_AddItemToObject(answe   , JSONSTR_RESPONSE, response);
	cJSON_AddItemToObject(response, JSONSTR_COMMAND , cJSON_CreateString(JSONSTR_ALOGOUT));

	if(jtoken == nullptr)
	{
		cJSON_AddItemToObject(response, JSONSTR_SUCCESS , cJSON_CreateFalse());
		CreateErrorJSON(response, JSONERR_BADPARAM, "");
		return JSONtoString(answe, true);
	}

	DeleteActivity(jtoken->valueint);

	// Нашли токен или не нашли - все равно. Выдаем что пользователь вышел
	cJSON_AddItemToObject(response, JSONSTR_SUCCESS , cJSON_CreateTrue());
	cJSON_AddItemToObject(response, JSONSTR_DATA    , data = cJSON_CreateObject());
	cJSON_AddItemToObject(data    , JSONSTR_USERNAME, cJSON_CreateString("GUEST"));
	cJSON_AddItemToObject(data    , JSONSTR_ACCESS  , cJSON_CreateNumber(0));

	return JSONtoString(answe, true);
}


//-------------------------------------------------------------------------------------------------
//
string rJSONManager::Packet_Check(cJSON *root)
{
	cJSON     *jtoken   = cJSON_GetObjectItem(root, JSONSTR_TOKEN);
	rActivity *act      = nullptr;
	// ответ
	cJSON     *answe    = cJSON_CreateObject();
	cJSON     *response = cJSON_CreateObject();
	cJSON     *data     = nullptr;

	cJSON_AddItemToObject(answe   , JSONSTR_RESPONSE, response);
	cJSON_AddItemToObject(response, JSONSTR_COMMAND , cJSON_CreateString(JSONSTR_ACHECK));

	if(jtoken == nullptr)
	{
		cJSON_AddItemToObject(response, JSONSTR_SUCCESS , cJSON_CreateFalse());
		CreateErrorJSON(response, JSONERR_BADPARAM, "");
		return JSONtoString(answe, true);
	}

	act = GetActivity(jtoken->valueint);

	if(nullptr == act)
	{
		cJSON_AddItemToObject(response, JSONSTR_SUCCESS , cJSON_CreateFalse());
		CreateErrorJSON(response, JSONERR_TOKEN_FAULT, "");
		return JSONtoString(answe, true);
	}

	act->UpdateIdle();

	// Выдаем уровень доступа
	cJSON_AddItemToObject(response, JSONSTR_SUCCESS , cJSON_CreateTrue());
	cJSON_AddItemToObject(response, JSONSTR_DATA    , data = cJSON_CreateObject());
	cJSON_AddItemToObject(data    , JSONSTR_USERNAME, cJSON_CreateString(act->User->GetName().c_str()));
	cJSON_AddItemToObject(data    , JSONSTR_ACCESS  , cJSON_CreateNumber(act->User->GetAccess()));

	return JSONtoString(answe, true);
}


string rJSONManager::Packet_Status(cJSON */*root*/)
{
	rState state;
	STM    sdt;
	// ответ
	cJSON *answe    = cJSON_CreateObject();
	cJSON *response = cJSON_CreateObject();
	cJSON *time     = cJSON_CreateObject();

	cJSON_AddItemToObject(answe   , JSONSTR_RESPONSE, response);
	cJSON_AddItemToObject(response, JSONSTR_COMMAND , cJSON_CreateString(JSONSTR_ISTATUS));
	cJSON_AddItemToObject(response, JSONSTR_TIME    , time);

	rDataManager::instance().GetState(state);
	rDataManager::instance().GetTime (sdt);

	// Выдаем уровень доступа
	cJSON_AddItemToObject(response, JSONSTR_SUCCESS , cJSON_CreateTrue());
	cJSON_AddItemToObject(response, JSONSTR_LIVE    , cJSON_CreateNumber(state.Live));
	cJSON_AddItemToObject(response, JSONSTR_ALARM   , cJSON_CreateNumber(state.EventAlarm));
	cJSON_AddItemToObject(time    , JSONSTR_SEC     , cJSON_CreateNumber(sdt.tm_sec ));
	cJSON_AddItemToObject(time    , JSONSTR_MIN     , cJSON_CreateNumber(sdt.tm_min ));
	cJSON_AddItemToObject(time    , JSONSTR_HOUR    , cJSON_CreateNumber(sdt.tm_hour));
	cJSON_AddItemToObject(time    , JSONSTR_DAY     , cJSON_CreateNumber(sdt.tm_mday));
	cJSON_AddItemToObject(time    , JSONSTR_MON     , cJSON_CreateNumber(sdt.tm_mon ));
	cJSON_AddItemToObject(time    , JSONSTR_YEAR    , cJSON_CreateNumber(sdt.tm_year));

	return JSONtoString(answe, true);
}


string rJSONManager::Packet_Conf(cJSON */*root*/)
{
	rVersion    ver;
	rConfigInfo conf;
	// ответ
	cJSON *answe    = cJSON_CreateObject();
	cJSON *response = cJSON_CreateObject();
	cJSON *jver     = cJSON_CreateObject();
	cJSON *jconf    = cJSON_CreateObject();

	cJSON_AddItemToObject(answe   , JSONSTR_RESPONSE, response);
	cJSON_AddItemToObject(response, JSONSTR_COMMAND , cJSON_CreateString(JSONSTR_ICONF));
	cJSON_AddItemToObject(response, JSONSTR_VERSION , jver);
	cJSON_AddItemToObject(response, JSONSTR_CONFIG  , jconf);

	rDataManager::instance().GetConfigInfo(conf);
	rDataManager::instance().GetVersion   (ver);

	// Выдаем уровень доступа
	cJSON_AddItemToObject(response, JSONSTR_SUCCESS, cJSON_CreateTrue());
	cJSON_AddItemToObject(jver    , JSONSTR_MAJOR  , cJSON_CreateNumber(ver.m_major     ));
	cJSON_AddItemToObject(jver    , JSONSTR_MINOR  , cJSON_CreateNumber(ver.m_minor     ));
	cJSON_AddItemToObject(jver    , JSONSTR_BUILD  , cJSON_CreateNumber(ver.m_build     ));
	cJSON_AddItemToObject(jver    , JSONSTR_HASH   , cJSON_CreateNumber(ver.m_hash     ));
	cJSON_AddItemToObject(jconf   , JSONSTR_FILE   , cJSON_CreateString(conf.File     ));
	cJSON_AddItemToObject(jconf   , JSONSTR_NAME   , cJSON_CreateString(conf.Name     ));
	cJSON_AddItemToObject(jconf   , JSONSTR_VERSION, cJSON_CreateString(conf.Version  ));
	cJSON_AddItemToObject(jconf   , JSONSTR_DEV    , cJSON_CreateString(conf.Developer));
	cJSON_AddItemToObject(jconf   , JSONSTR_HASH   , cJSON_CreateString(conf.Hash     ));

	return JSONtoString(answe, true);
}


//
string rJSONManager::Packet_ListConf(cJSON */*root*/)
{
	// ответ
	cJSON *answe    = cJSON_CreateObject();
	cJSON *response = cJSON_CreateObject();
	cJSON *list     = nullptr;

	cJSON_AddItemToObject(answe   , JSONSTR_RESPONSE, response);
	cJSON_AddItemToObject(response, JSONSTR_COMMAND , cJSON_CreateString(JSONSTR_ILISTCONF));

	if(Live::REBOOT_COLD != rDataManager::instance().GetLiveStatus())
	{
		cJSON_AddItemToObject(response, JSONSTR_SUCCESS , cJSON_CreateFalse());
		CreateErrorJSON(response, JSONERR_NOTCOLDSTART, "");
		return JSONtoString(answe, true);
	}

	rListConfig::Load();

	// Выдаем список доступных конфигураций
	cJSON_AddItemToObject(response, JSONSTR_SUCCESS, cJSON_CreateTrue());
	cJSON_AddItemToObject(response, JSONSTR_LIST   , list = cJSON_CreateArray());

	for(UDINT ii = 0; ii < rListConfig::Size(); ++ii)
	{
		cJSON  *aconf  = cJSON_CreateObject();

		cJSON_AddItemToObject(aconf   , JSONSTR_FILE  , cJSON_CreateString(rListConfig::Get(ii)->Filename.c_str()));
		cJSON_AddItemToObject(aconf   , JSONSTR_NAME  , cJSON_CreateString(rListConfig::Get(ii)->Description.c_str()));
		cJSON_AddItemToObject(aconf   , JSONSTR_TIME  , cJSON_CreateString(String_FileTime(rListConfig::Get(ii)->Filetime).c_str()));
		cJSON_AddItemToObject(aconf   , JSONSTR_STATUS, cJSON_CreateNumber(rListConfig::Get(ii)->Status));
		cJSON_AddItemToObject(aconf   , JSONSTR_HASH  , cJSON_CreateString(rListConfig::Get(ii)->m_strHash.c_str()));

		cJSON_AddItemToArray(list, aconf);
	}

	return JSONtoString(answe, true);
}


string rJSONManager::Packet_Restart (cJSON *root)
{
	cJSON     *jtoken   = cJSON_GetObjectItem(root, JSONSTR_TOKEN);
	cJSON     *jconf    = cJSON_GetObjectItem(root, JSONSTR_CONFIG);
	cJSON     *jrestart = cJSON_GetObjectItem(root, JSONSTR_RESTART);
	cJSON     *jlocal   = cJSON_GetObjectItem(root, JSONSTR_LOCAL);
	rActivity *act      = nullptr;
	USINT      live     = rDataManager::instance().GetLiveStatus();
	string     conf     = "";
	// ответ
	cJSON     *answe    = cJSON_CreateObject();
	cJSON     *response = cJSON_CreateObject();

	cJSON_AddItemToObject(answe   , JSONSTR_RESPONSE, response);
	cJSON_AddItemToObject(response, JSONSTR_COMMAND , cJSON_CreateString(JSONSTR_IRESTART));

	if(jtoken == nullptr || nullptr == jrestart || nullptr == jlocal)
	{
		cJSON_AddItemToObject(response, JSONSTR_SUCCESS , cJSON_CreateFalse());
		CreateErrorJSON(response, JSONERR_BADPARAM, "");
		return JSONtoString(answe, true);
	}

	if(jconf)
	{
		conf = (jconf->valuestring) ? jconf->valuestring : "";
	}

	if(Live::REBOOT_COLD != live)
	{
		act = GetActivity(jtoken->valueint);

		if(nullptr == act)
		{
			cJSON_AddItemToObject(response, JSONSTR_SUCCESS , cJSON_CreateFalse());
			CreateErrorJSON(response, JSONERR_TOKEN_FAULT, "");
			return JSONtoString(answe, true);
		}
		act->UpdateIdle();

		if(!act->User->CheckAccess(ACCESS_REBOOT, jlocal->valueint))
		{
			cJSON_AddItemToObject(response, JSONSTR_SUCCESS , cJSON_CreateFalse());
			CreateErrorJSON(response, JSONERR_ACCESSDENIED, "");
			return JSONtoString(answe, true);
		}

		if(conf.size())
		{
			cJSON_AddItemToObject(response, JSONSTR_SUCCESS , cJSON_CreateFalse());
			CreateErrorJSON(response, JSONERR_NOTCOLDSTART, "");
			return JSONtoString(answe, true);
		}
	}
	else
	{
		if(conf.empty())
		{
			cJSON_AddItemToObject(response, JSONSTR_SUCCESS , cJSON_CreateFalse());
			CreateErrorJSON(response, JSONERR_CONFISEMPTY, "");
			return JSONtoString(answe, true);
		}
	}

	UDINT result = rDataManager::instance().Restart(jrestart->valueint, conf);

	// Выдаем результат
	cJSON_AddItemToObject(response, JSONSTR_SUCCESS , cJSON_CreateBool(0 == result));

	return JSONtoString(answe, true);
}

