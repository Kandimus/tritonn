//=================================================================================================
//===
//=== threadclass.cpp
//===
//=== Copyright (c) 2019-2020 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Заголовочный файл, для описания констант
//===
//=================================================================================================

#pragma once

#include <string>
#include <openssl/sha.h>
#include "time64.h"
#include "types.h"
#include "stringid.h"


using std::string;

#ifdef TRITONN_SOURCE_PATH
	#define __FILENAME__            (&__FILE__[(std::string(TRITONN_SOURCE_PATH).size())])
#else
	#define __FILENAME__            (__FILE__)
#endif


//-------------------------------------------------------------------------------------------------
// Константы для управления потоками
const UDINT THREAD_FAULT        = -1;


//-------------------------------------------------------------------------------------------------
// Константы сокетов
const DINT   MAX_SELECT_SEC            = 0;          // Функция select будет ожидать 1 секунду
const DINT   MAX_SELECT_USEC           = 50000;     // Функция select будет ожидать 0 милисекунд


//-------------------------------------------------------------------------------------------------
// Сетевые константы

enum LanPort : UINT
{
	PORT_EVENT  = 22500,       // Порт логирования
	PORT_JSON   = 22501,       // Порт JSON
	PORT_TERM   = 22503,       // Порт терминала и конфигуратора
	PORT_MODBUS = 502,         // Стандартный порт для ModbusTCP
};

const UDINT  MAX_TCP_RECV_BUFF         = 16 * 1024;   //
const UDINT  MAX_TCP_SEND_BUFF         = 16 * 1024;   //
const UDINT  MAX_TCPCLIENT_BUFF        = 4 * MAX_TCP_RECV_BUFF; // Размер буффера принятых сообщений клиентом TCP

const UDINT  MARKER_PACKET_CMD         = 0x2592E66B;  // Маркер пакета с командой
const UDINT  MARKER_PACKET_LOGINANSWE  = 0x558CAEAC;  // Маркер пакета с регистрацией пользователя
const UDINT  MARKER_PACKET_LOGIN       = 0xBF268FA3;  // Маркер пакета с регистрацией пользователя
const UDINT  MARKER_PACKET_SET         = 0x7281A77D;  // Маркер пакета записи значений переменных
const UDINT  MARKER_PACKET_SETANSWE    = 0xF8E3C2C1;  // Маркер пакета результата на запись переменных
const UDINT  MARKER_PACKET_GET         = 0x72703587;  // Маркер пакета чтения переменных
const UDINT  MARKER_PACKET_GETANSWE    = 0xAF5750CB;  // Маркет пакета результата чтения переменных

const UDINT  MAX_VARIABLE_LENGTH       = 128;         // Предельная длинна имени переменной //TODO Стоит после создания дерева переменных уменьшить данное число до 64. В теории можно пробежаться по всем варам и узнать макс длину
const UDINT  MAX_VARVALUE_LENGTH       = 32;          // Предельное значение значения переменной
const UDINT  MAX_PACKET_SET_COUNT      = 32;          // Количество переменных для одновременной записи
const UDINT  MAX_PACKET_GET_COUNT      = 32;          // Количество переменных для одновременного считывания


//-------------------------------------------------------------------------------------------------
// Логи
const UDINT  MAX_LOG_TEXT              = 936;         // Общая длина посылки будет максимум 1024 байт
const UDINT  MAX_LOG_FILENAME          = 64;
const UINT   LOG_CP_WIN1251            = 0x0000;
const UINT   LOG_CP_UTF8               = 0x1000;
const UINT   LOG_CP_UNICODE            = 0x2000;
const UINT   LOG_CP_MASK               = 0x3000;
const UDINT  MAX_JSON_CLIENT           = 8;           // Кол-во одновременных сессий JSON
const UDINT  MAX_JSON_IDLE             = 30;          // Максимальное время простоя без запросов в сессии JSON, сек
const UDINT  MAX_JSON_SESSION          = 10 * 60;     // Максимальное время сессии JSON, сек
const UDINT  MAX_MBTCP_CLIENT          = 8;

//-------------------------------------------------------------------------------------------------
// Количество объектов
const UDINT  MAX_MODULE                = 16;
const UDINT  MAX_MODULE_CHANNEL        = 16;
const UDINT  MAX_IO_AI                 = 64;
const UDINT  MAX_IO_AO                 = 32;
const UDINT  MAX_IO_DI                 = 32;
const UDINT  MAX_IO_DO                 = 32;
const UDINT  MAX_IO_FI                 = 16;
const UDINT  MAX_IO_PR                 = 4;
const UDINT  MAX_STATION               = 4;
const UDINT  MAX_STREAM                = 16;
const UDINT  MAX_SELECTOR              = 16;
const UDINT  MAX_MSELECTOR             = 16;
const UDINT  MAX_DENSSOL               = 16;
const UDINT  MAX_REDUCEDDENS           = 32;
const UDINT  MAX_SAMPLER               = 32;
const UDINT  MAX_PROVE                 = 8;
const UDINT  MAX_AVERAGE               = 32;
const UDINT  MAX_MASSWATER             = 16;
const UDINT  MAX_VOLWATER              = 16;
const UDINT  MAX_KINVISC               = 8;
const UDINT  MAX_DYNVISC               = 8;

const LREAL  MAX_TOTAL_LIMIT           = 9999999999.99999;

const UDINT  MAX_CONFIG_NAME           = 128;
const UDINT  MAX_CFGVER_SIZE           = 17;
const UDINT  MAX_HASH_SIZE             = SHA_DIGEST_LENGTH * 2;

const UDINT  MAX_UNITS_COUNT           = 512;


const USINT  AES_KEY[16]               = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
const USINT  AES_IV[16]                = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};


//-------------------------------------------------------------------------------------------------
// Вспомогательные константы
const LREAL  COMPARE_LREAL_PREC        = 1.0E-10;
const LREAL  COMPARE_REAL_PREC         = 1.0E-7;

//-------------------------------------------------------------------------------------------------
// rDataManager
enum Live : USINT
{
	UNDEF = 0,
	STARTING,
	REBOOT_COLD,
	DUMP_TOTALS,
	DUMP_VARS,
	RUNNING,
	HALT        = 0xFF,
};

//
const USINT  RESTART_WARM              = 1;
const USINT  RESTART_COLD              = 2;
const USINT  RESTART_DEBUG             = 3;

// Причина возникновения Halt, в старшем слове причина, в младшее нужно положить код ошибки
const UDINT  HALT_REASON_CONFIGFILE    = 0x00010000;
const UDINT  HALT_REASON_WEBFILE       = 0x00020000;
const UDINT  HALT_REASON_HARDWARE      = 0x00030000;
const UDINT  HALT_REASON_REPORT        = 0x00040000;
const UDINT  HALT_REASON_OPC           = 0x00050000;
const UDINT  HALT_REASON_RUNTIME       = 0x00060000;
const UDINT  HALT_REASON_DUMP          = 0x00070000;

//THREADMASTER_FLAGS
const UDINT  TMF_NONE                  = 0x00000000;
const UDINT  TMF_RUN                   = 0x00000001;
const UDINT  TMF_DELETE                = 0x00000002;       // При закрытии нитей, удалить класс потока
const UDINT  TMF_NOTRUN                = 0x00000004;       // Нить еще не запущена


const UINT   REPORT_DEFAULT_STORAGE    = 91;


//-------------------------------------------------------------------------------------------------
//
const UDINT  LOGIN_OK                  = 0; // Логин успешен
const UDINT  LOGIN_FAULT               = 1; // Логин или пароль не верен
const UDINT  LOGIN_BLOCKED             = 2; // Пользователь заблокирован
const UDINT  LOGIN_CHANGEPWD           = 3; // Пользователю нужно сменить пароль

//
const USINT  USER_CHANGEPWD            = 0x01;        // Пользователь должен сменить пароль
const USINT  USER_MASK_BLOCKED         = 0xF0;
const USINT  USER_BLOCKEDMANUAL        = 0x10;        // Пользователя заблокировал администратор
const USINT  USER_BLOCKEDAUTOMAT       = 0x20;        // Пользователь заблокирован после 3-х попыток ввода пароля

//
const string USER_PWD_SALT             = "getsystem"; // Соль для пароля пользователя
const string XMLHASH_SALT              = "0123456789abcdefghijklmnoABCDEFGHIJKLMNO";

//-------------------------------------------------------------------------------------------------
// Биты доступа пользователя
const UDINT  ACCESS_EVENT              = 0x00000001;  //   Просмотр журнала событий
const UDINT  ACCESS_REPORT             = 0x00000002;  //   Просмотр архивных и текущих отчетов
const UDINT  ACCESS_PROTOCOL           = 0x00000004;  //   Просмотр протоколов КМХ и поверки
const UDINT  ACCESS_TRENDS             = 0x00000008;  //   Доступ к просмотру трендов
const UDINT  ACCESS_PRINTER            = 0x00000010;  //   Доступ к печати отчетов и протоколов
const UDINT  ACCESS_BATCH              = 0x00000020;  // + Запуск партионных отчетов
const UDINT  ACCESS_CONFIRMALARM       = 0x00000040;  //   Доступ к квитированию аварий
const UDINT  ACCESS_SAMPLERS           = 0x00000080;  // + Управление пробоотбониками
const UDINT  ACCESS_SETSAMPLERS        = 0x00000100;  // + Настройка пробоотборников
const UDINT  ACCESS_LIMITS             = 0x00000200;  // + Изменение лимитов
const UDINT  ACCESS_SCALES             = 0x00000400;  // + Изменение инж. пределов у объектов ввода-вывода, калибровка аналоговых сигналов
const UDINT  ACCESS_KEYPAD             = 0x00000800;  // + Установка и перевод в keypad режим всех объектов
const UDINT  ACCESS_FACTORS            = 0x00001000;  // + Изменение факторов всех объектов (плотномер, линия, поверка и т.д.)
const UDINT  ACCESS_MAINTENANCE        = 0x00002000;  // + Ввод и вывод линию в учет
const UDINT  ACCESS_SELECT             = 0x00004000;  // + Переключение селекторов
const UDINT  ACCESS_PROVE              = 0x00008000;  // + Доступ к проведению поверки
const UDINT  ACCESS_TOTALS             = 0x00010000;  // + Доступ к сбросу нарастающих
const UDINT  ACCESS_SYSTEM             = 0x00020000;  // + Доступ резервированию, уставке времени, созданию backup
const UDINT  ACCESS_DIAGNOSTIC         = 0x00040000;  //   Просмотр диагностики (Хеш суммы, время выполнения задач и т.д.)
const UDINT  ACCESS_REBOOT             = 0x00080000;  // + Доступ к перезагрузке ИВК
const UDINT  ACCESS_USB                = 0x00100000;  // ? Доступ к записи отчетов и трендов на USB
const UDINT  ACCESS_COMMMUNICATIONS    = 0x00200000;  // + Доступ к настроки OPC, Modbus, COM-портам, TCP
const UDINT  ACCESS_USERS              = 0x00400000;  //   Доступ к настройкам пользователей, сброс паролей
const UDINT  ACCESS_UNKNOWN_00800000   = 0x00800000;  //
const UDINT  ACCESS_UNKNOWN_01000000   = 0x01000000;
const UDINT  ACCESS_UNKNOWN_02000000   = 0x02000000;
const UDINT  ACCESS_UNKNOWN_04000000   = 0x04000000;
const UDINT  ACCESS_UNKNOWN_08000000   = 0x08000000;
const UDINT  ACCESS_SECURITYINTERFACE  = 0x10000000;  // + Доступ к записи в Modbus или OPC, при выбранной опции ScurityModbus/OPC
const UDINT  ACCESS_WEB                = 0x20000000;  //   Доступ к входу через web-интерфейс. Для ADMIN и SA должен быть выключен?
const UDINT  ACCESS_ADMIN              = 0x40000000;  // + Администратор (разработчик от ОЗНА). ВНИМАНИЕ! Это не обычный администратор!!!!! Это уровень программиста ОИ
const UDINT  ACCESS_SA                 = 0x80000000;  // + Темный лорд

const UDINT  ACCESS_MASK_SYSTEM        = 0xFFFFFFFF;
const UDINT  ACCESS_MASK_SA            = 0xFFFFFFFF & (~ACCESS_WEB);
const UDINT  ACCESS_MASK_ADMIN         = 0xFFFFFFFF & (~(ACCESS_WEB | ACCESS_SA));
const UDINT  ACCESS_MASK_VIEWHIDE      = (ACCESS_SA | ACCESS_ADMIN);


//-------------------------------------------------------------------------------------------------
// Директории
#ifdef WIN32
const std::string DIR_HOME             = ".\\";
const std::string DIR_CONF             = DIR_HOME + "conf\\";
const std::string DIR_FTP              = DIR_HOME + "ftp\\";
const std::string DIR_WWW              = DIR_HOME + "www\\";
const std::string DIR_EVENT            = DIR_HOME + "log\\";
const std::string DIR_REPORT           = DIR_HOME + DIR_FTP + "reports\\";
const std::string DIR_TIMEINFO         = DIR_HOME + "diag\\";
const std::string DIR_MARKDOWN         = DIR_HOME + "help\\";
const std::string FILE_CONF            = DIR_HOME + "tritonn.conf";
const std::string FILE_RESTART         = DIR_HOME + "tritonn.restart";
const std::string FILE_MODBUS          = DIR_HOME + "modbus.xml";
const std::string FILE_SYSTEMTEXT      = DIR_HOME + "systemtext.xml";
const std::string FILE_SYSTEMEVENT     = DIR_HOME + "systemevent.xml";
#else
const std::string DIR_HOME             = "./";
const std::string DIR_CONF             = DIR_HOME + "conf/";
const std::string DIR_LOG              = DIR_HOME + "log/";
const std::string DIR_FTP              = DIR_HOME + "ftp/";
const std::string DIR_WWW              = DIR_HOME + "www/";
const std::string DIR_DUMP             = DIR_HOME + "dump/";
const std::string DIR_EVENT            = DIR_WWW  + "application/core/events/";
const std::string DIR_REPORT           = DIR_FTP  + "reports/";
const std::string DIR_TIMEINFO         = DIR_HOME + "diag/";
const std::string DIR_MARKDOWN         = DIR_HOME + "help/";
const std::string FILE_CONF            = DIR_HOME + "tritonn.conf";
const std::string FILE_RESTART         = DIR_HOME + "tritonn.restart";
const std::string FILE_MODBUS          = DIR_HOME + "modbus.xml";
const std::string FILE_SYSTEMTEXT      = DIR_HOME + "systemtext.xml";
const std::string FILE_SYSTEMEVENT     = DIR_HOME + "systemevent.xml";
const std::string FILE_WWW_TREE_OBJ    = DIR_WWW  + "application/core/tree_objects.json";
const std::string FILE_WWW_PRECISION   = DIR_WWW  + "application/core/precision.php";
const std::string DIR_WWW_LANG         = DIR_WWW  + "application/language/";
const std::string FILE_WWW_LANG        = "custom_lang.php";
const std::string FILE_WWW_EVENT       = "event_lang.php";
const std::string FILE_DUMP_VARIABLES  = DIR_DUMP + "variables.xml";
const std::string FILE_DUMP_TOTALS     = DIR_DUMP + "totals.xml";
#endif


//
// Константы языков
const UDINT  MAX_LANG_SIZE             = 8;
const string LANG_EN                   = "en";
const string LANG_RU                   = "ru";


///////////////////////////////////////////////////////////////////////////////////////////////////
// Типы используемых данных
enum TT_TYPE
{
	TYPE_UNDEF = 0,
	TYPE_SINT,
	TYPE_USINT,
	TYPE_INT,
	TYPE_UINT,
	TYPE_DINT,
	TYPE_UDINT,
	TYPE_REAL,
	TYPE_LREAL,
	TYPE_STRID,
//	TYPE_DATA,
	TYPE__END
};


// Размеры типов данных
const UDINT       EPT_SIZE [TYPE__END] = {255, sizeof(SINT), sizeof(USINT), sizeof(INT), sizeof(UINT), sizeof(DINT), sizeof(UDINT), sizeof(REAL), sizeof(LREAL), sizeof(/*STRID*/UDINT)/*, 0*/};
const std::string NAME_TYPE[TYPE__END] = {"UNDEF", "SINT", "USINT", "INT", "UINT", "DINT", "UDINT", "REAL", "LREAL", "STRID"/*, "DATA"*/};


///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
const USINT MODBUS_ERROR_FUNC = 0x01;
const USINT MODBUS_ERROR_ADR  = 0x02;
const USINT MODBUS_ERROR_DATA = 0x03;


///////////////////////////////////////////////////////////////////////////////////////////////////
// Описание ошибок при работе с запросами JSON
enum JSON_ERROR
{
	JSONERR_UNDEF = 1,
	JSONERR_UNKNOWMETHOD,
	JSONERR_TOMANYREQ,
	JSONERR_ACCESSDENIED,
	JSONERR_BADREQ,            // Ошибка в структуре запроса (пропущенные данные и т.д.)
	JSONERR_TOMANYEQUALREQ,
	JSONERR_INTERNALFAULT,
	JSONERR_NEEDLOGIN,
	JSONERR_PAGENOTFOUND,
	JSONERR_DISABLEMETHOD,
	JSONERR_KEYFAULT,
	JSONERR_METHODLIMIT,
	JSONERR_BADPARAM = 100,
	JSONERR_IDFAULT,
	JSONERR_LOGIN_FAULT,        // Ошибка при логине пользователя
	JSONERR_LOGIN_BLOCKED,
	JSONERR_LOGIN_OK,
	JSONERR_LOGIN_CHANGEPWD,
	JSONERR_TOKEN_FAULT,        // Данный токен не найден
	JSONERR_NOTCOLDSTART,       // Режим не COLDSTART
	JSONERR_CONFISEMPTY,        // Конфигурация не указана
	JSONERR_NOTLOADDUMP,
};


//
enum rTritonn_Error
{
	TRITONN_RESULT_OK = 0,

	// 1..29 ошибки tinyxml2

	FILE_RESULT_NOTFOUND      = 31,         //  31 Файл не найден
	FILE_RESULT_CANTOPEN,                   //  32 Файл не может быть открыт
	FILE_RESULT_IOERROR,                    //  33 Ошибка ввода-вывода
	FILE_RESULT_READONLY,                   //  34 Файл не может быть записан (режим только чтение)
	FILE_RESULT_BLOCKED,                    //  35 Файл заблокирован
	FILE_RESULT_EACCESS,                    //  36 Доступ к файлу запрещен
	FILE_RESULT_ISDIR,                      //  37 Это директория
	FILE_RESULT_ISEMPTY,                    //  38 Файл пустой
	FILE_RESULT_EDIR,                       //  39 Ошибка считывания директории
	FILE_RESULT_CANTDELETE    = 40,         //  40 Ошибка операции удаления файла или директории
	FILE_RESULT_EFILE,                      //  41 Прочие ошибки файла (сбой функции stat)
	FILE_RESULT_CANTREMOVE,                 //  42

	DATACFGERR_STRUCT         = 100,        // 100
	DATACFGERR_NOTFOUND_HARDWARE,           //
	DATACFGERR_CONFIG,                      // 102
	DATACFGERR_IO,
	DATACFGERR_UNKNOWIO,                    // 104
	DATACFGERR_CALC,
	DATACFGERR_UNKNOWCALC,                  // 106
	DATACFGERR_AI,
	DATACFGERR_FI,                          // 108
	DATACFGERR_LINK,
	DATACFGERR_LINKNF,                      // 110
	DATACFGERR_STATIONS,
	DATACFGERR_STATIONSNF,                  // 112
	DATACFGERR_STATION,
	DATACFGERR_STATION_UNITS,               // 114
	DATACFGERR_STATION_WRONGSTREAM,         //
	DATACFGERR_STREAMS,                     // 116
	DATACFGERR_STREAMSNF,                   //
	DATACFGERR_STREAM,                      // 118
	DATACFGERR_STREAM_NOSTN,                //
	DATACFGERR_STREAM_TOMANYPOINTS,         // 120
	DATACFGERR_STREAM_FACTORS,              //
	DATACFGERR_STREAM_NOFREQCHANNEL,        // 122
	DATACFGERR_DENSSOL,                     //
	DATACFGERR_DENSSOL_NOSTN,               // 124
	DATACFGERR_SELECTOR,                    //
	DATACFGERR_REDUCEDDENS,                 // 126
	DATACFGERR_RESOLVELINK,                 //
	DATACFGERR_CHECKLINK,                   // 128
	DATACFGERR_MAX_AI,                      //
	DATACFGERR_MAX_AO,                      // 130
	DATACFGERR_MAX_FI,                      //
	DATACFGERR_MAX_DI,                      // 132
	DATACFGERR_MAX_DO,                      //
	DATACFGERR_MAX_DENSSOL,                 // 134
	DATACFGERR_MAX_RDCDENS,                 //
	DATACFGERR_MAX_SELECTOR,                // 136
	DATACFGERR_MAX_STREAM,                  //
	DATACFGERR_MAX_STATION,                 // 138
	DATACFGERR_MAX_SAMPLER,                 //
	DATACFGERR_MAX_PROVE,                   // 140
	DATACFGERR_MAX_AVERAGE,                 //
	DATACFGERR_MAX_MASSWATER,               // 142
	DATACFGERR_MAX_VOLWATER,                //
	DATACFGERR_MAX_KINVISC,                 // 144
	DATACFGERR_MAX_DYNVISC,                 //
	DATACFGERR_MAX_RESERV5________,         // 146
	DATACFGERR_LIMIT,                       //
	DATACFGERR_REPORT,                      // 148
	DATACFGERR_NOREPORTS,                   //
	DATACFGERR_RESOLVETOTAL,                // 150
	DATACFGERR_VAR,                         //
	DATACFGERR_VAR_SETUP,                   // 152
	DATACFGERR_VAR_DEFAULT,                 //
	DATACFGERR_VAR_LINK,                    // 154
	DATACFGERR_VAR_UNIT,                    //
	DATACFGERR_RESERV2____________,         // 156
	DATACFGERR_RESERV3____________,         //
	DATACFGERR_USERS_NF,                    // 158     Not Found Template BLOCKS
	DATACFGERR_INTERNAL,                    //
	DATACFGERR_USERS_PARSE,                 // 160
	DATACFGERR_INTERFACES_NF_TBLOKS,        //
	DATACFGERR_INTERFACES_NF_BLOCKS,        // 162
	DATACFGERR_INTERFACES_BAD_ADDR,         //
	DATACFGERR_INTERFACES_BAD_BLOCK,        // 164
	DATACFGERR_INTERFACES_BAD_VAR,          //
	DATACFGERR_INTERFACES_NF_VAR,           // 166
	DATACFGERR_INTERFACES_ADDR_OVERFLOW,    //
	DATACFGERR_INTERFACES_NF_STD_VAR,       // 168
	DATACFGERR_INTERFACES_BAD_STD_ADDR,     //
	DATACFGERR_INCORRECT_IP,                // 170
	DATACFGERR_SECURITY_PARSE,              //
	DATACFGERR_SECURITY_DESCRYPT,           // 172
	DATACFGERR_SECURITY_NF,                 //
	DATACFGERR_OPCUA_USER_NF,               // 174
	DATACFGERR_OPCUA_BAD_USER,              //
	DATACFGERR_OPCUA_VAR_NF,                // 176
	DATACFGERR_UNKNOWN_MODULE,              //
	DATACFGERR_IO_CHANNEL,                  // 178
	DATACFGERR_INVALID_NAME,                //
	DATACFGERR_INVALID_MODULELINK,          // 180
	DATACFGERR_REALTIME_MODULELINK,         //
	DATACFGERR_NOTSYSTEXTFILE,              // 182
	DATACFGERR_INTERFACES_BADNAME,          //
	DATACFGERR_DI,                          // 184
	DATACFGERR_DO,                          //
	DATACFGERR_SAMPLER_TOTALS,              // 186
	DATACFGERR_SAMPLER_CAN,                 //
	DATACFGERR_SAMPLER_METHOD,              // 188
	DATACFGERR_SAMPLER_SETUP,               //
	DATACFGERR_SAMPLER_RESERVE,             // 190
	DATACFGERR_SAMPLER_RESERVE_NF,          //
	DATACFGERR_OBJECT_UNITS,                // 192
	DATACFGERR_PORVE_MISSINGMODULE,         //
	DATACFGERR_AVERAGE_NOINPUT,             // 194
	DATACFGERR_AVERAGE_TOOMANYINPUT,        //
	DATACFGERR_AVERAGE_TOOFEWINPUT,         // 196
	DATACFGERR_AVERAGE_DIFFUNITS,           //
	DATACFGERR_AVERAGE_DIFFFAULTS,          // 198
	DATACFGERR_AVERAGE_TOOMANYFAULTS,       //
	DATACFGERR_AVERAGE_SETUP,               // 200
	DATACFGERR_MASSWATER_SETUP,             //
	DATACFGERR_MASSWATER_NOVOLWATER,        // 202
	DATACFGERR_MASSWATER_NODENSITY,         //
	DATACFGERR_MASSWATER_NOTEMPERATURE,     // 204
	DATACFGERR_VOLWATER_SETUP,              //
	DATACFGERR_VOLWATER_NOMASSWATER,        // 206
	DATACFGERR_VOLWATER_NODENSITY,          //
	DATACFGERR_VOLWATER_NOTEMPERATURE,      // 208
	DATACFGERR_KINVISC_NODYNVISC,           //
	DATACFGERR_KINVISC_NODENSITY,           // 210
	DATACFGERR_DYNVISC_NOKINVISC,           //
	DATACFGERR_DYNVISC_NODENSITY,           // 212

	DATACFGERR_LANG_STRUCT = 500,
	DATACFGERR_LANG_UNKNOW,
	DATACFGERR_LANG_DUPID,
	DATACFGERR_LANG_ID,
	DATACFGERR_LANG_DEFAULT,

	DATACFGERR_PREC_ID = 600,

	XMLFILE_RESULT_NFHASH = 700,
	XMLFILE_RESULT_BADHASH,
	XMLFILE_RESULT_NOTEQUAL,

	// 1024..1999 OPC UA (open62541)
	// 2000 tritonn opc ua
	OPCUA_ERROR_VARNF = 2000,
	OPCUA_ERROR_BADVARTYPE,

};


///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Предописание вспомогательных функций
//
extern void        mSleep(UDINT msec);

extern void        getCurrentTime(Time64_T &t64, struct tm *stm);
extern void        setCurrentTime(struct tm& stm);

extern LREAL       Round(LREAL val,  USINT prec);
extern UDINT       isvalidhex(const char *str, UDINT &val);

extern std::string String_FileTime(time_t t);
extern std::string String_format(const char *format, ...);
extern std::string String_tolower(const std::string &str);
extern bool        String_equali (const std::string &str1, const std::string &str2);
extern std::string String_ConvertByType(USINT type, void *val);

extern UINT        DayInMonthShift(UINT year, UINT month);
extern UINT        IsLeapYear(UINT year);
extern UINT        WeekNumber(tm &curtm);

extern void        Event_ID          (UDINT eid, UDINT id);
extern void        Event_ID_UINT     (UDINT eid, UDINT id, UINT val);
extern void        Event_ID_UDINT    (UDINT eid, UDINT id, UDINT val);
extern void        Event_ID_UINT_UINT(UDINT eid, UDINT id, UINT val1, UINT val2);



