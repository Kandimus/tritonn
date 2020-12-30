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
const DINT   MAX_SELECT_USEC           = 5000;     // Функция select будет ожидать 0 милисекунд


//-------------------------------------------------------------------------------------------------
// Сетевые константы
const UINT   TCP_PORT_LOG              = 22500;       // Порт логирования
const UINT   TCP_PORT_JSON             = 22501;       // Порт JSON
const UINT   TCP_PORT_TERM             = 22503;       // Порт терминала и конфигуратора
const UINT   TCP_PORT_MODBUS           = 502;         // Стандартный порт для ModbusTCP
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
const UDINT  MAX_LOG_CLIENT            = 2;           // Кол-во одновременных подключений к логам
const UDINT  MAX_JSON_CLIENT           = 8;           // Кол-во одновременных сессий JSON
const UDINT  MAX_JSON_IDLE             = 30;          // Максимальное время простоя без запросов в сессии JSON, сек
const UDINT  MAX_JSON_SESSION          = 10 * 60;     // Максимальное время сессии JSON, сек
const UDINT  MAX_MBTCP_CLIENT          = 8;

const UDINT  LM_SYSTEM                 = 0x00000001;  //
const UDINT  LM_TCPSERV                = 0x00000002;
const UDINT  LM_TCPCLNT                = 0x00000004;
const UDINT  LM_EVENT                  = 0x00000008;
const UDINT  LM_TERMINAL               = 0x00000010;
const UDINT  LM_TEXT                   = 0x00000020;
const UDINT  LM_OPCUA                  = 0x00000040;
const UDINT  LM_LOG                    = 0x08000000;
const UDINT  LM_I                      = 0x10000000;
const UDINT  LM_W                      = 0x20000000;
const UDINT  LM_A                      = 0x40000000;
const UDINT  LM_P                      = 0x80000000;
const UDINT  LM_ALL                    = 0xFFFFFFFF;


//-------------------------------------------------------------------------------------------------
// События
const UDINT  MAX_EVENT                 = 500;         // Константа определяется из размера энергонезависимой памяти и максимальное количество сохраняемых событий
const UDINT  MAX_EVENT_DATA            = 64;          // Размер области данных одного сообщения //TODO Может сделать расчетной?
const UDINT  MAX_EVENT_ALARM           = 3;           // Максимальное количество аварийных событий для индикации


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
const UDINT  MAX_DENSSOL               = 16;
const UDINT  MAX_REDUCEDDENS           = 32;
const UDINT  MAX_SAMPLER               = 32;

const UDINT  MAX_AI_SPLINE             = 4;
const UDINT  MAX_FI_SPLINE             = 4;
const UDINT  MAX_SELECTOR_INPUT        = 4;
const UDINT  MAX_SELECTOR_GROUP        = 8;

const UDINT  MAX_FACTOR_POINT          = 12;

const LREAL  MAX_TOTAL_LIMIT           = 9999999999.99999;

const UDINT  MAX_CONFIG_NAME           = 128;
const UDINT  MAX_CFGVER_SIZE           = 17;
const UDINT  MAX_HASH_SIZE             = SHA_DIGEST_LENGTH;

const UDINT  MAX_UNITS_COUNT           = 512;


const USINT  AES_KEY[16]               = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
const USINT  AES_IV[16]                = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};


//-------------------------------------------------------------------------------------------------
// Вспомогательные константы
const LREAL  COMPARE_LREAL_PREC        = 1.0E-10;
const LREAL  COMPARE_REAL_PREC         = 1.0E-7;
const USINT  PRECISION_DEFAUILT        = 5;


//-------------------------------------------------------------------------------------------------
// rDataManager
const USINT  LIVE_UNDEF                = 0;
const USINT  LIVE_STARTING             = 1;    // Запускается
const USINT  LIVE_REBOOT_COLD          = 3;
const USINT  LIVE_RUNNING              = 4;
const USINT  LIVE_HALT                 = 0xFF;

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


//THREADMASTER_FLAGS
const UDINT  TMF_NONE                  = 0x00000000;
const UDINT  TMF_RUN                   = 0x00000001;
const UDINT  TMF_DELETE                = 0x00000002;       // При закрытии нитей, удалить класс потока
const UDINT  TMF_NOTRUN                = 0x00000004;       // Нить еще не запущена


//-------------------------------------------------------------------------------------------------
// Настройка селектора
const UINT   SELECTOR_SETUP_OFF        = 0x0001;      // Селектор отключен
const UINT   SELECTOR_SETUP_NOEVENT    = 0x0004;      // Не выдавать сообщений
const UINT   SELECTOR_SETUP_MULTI      = 0x8000;      // Флаг мультиселектора (внутренний), не указывать в документации

const UINT   SELECTOR_MODE_NOCHANGE    = 1;           // Не переходить в случае аварии на другое входное значение
const UINT   SELECTOR_MODE_TOERROR     = 2;           // В случае аварии переходить на ручной ввод (keypad)
const UINT   SELECTOR_MODE_CHANGEPREV  = 3;           // В случае аварии переходить на предыдущее входное значение
const UINT   SELECTOR_MODE_CHANGENEXT  = 4;           // В случае аварии переходить на следующее входное значение


//-------------------------------------------------------------------------------------------------
// Тип отчета
const UINT   REPORT_UNDEF              = 0;
const UINT   REPORT_PERIODIC           = 1;
const UINT   REPORT_BATCH              = 2;

//-------------------------------------------------------------------------------------------------
// Периодичность отчета
const UINT   REPORT_PERIOD_HOUR        = 0;
const UINT   REPORT_PERIOD_2HOUR       = 1;
const UINT   REPORT_PERIOD_3HOUR       = 2;
const UINT   REPORT_PERIOD_4HOUR       = 3;
const UINT   REPORT_PERIOD_6HOUR       = 4;
const UINT   REPORT_PERIOD_8HOUR       = 5;
const UINT   REPORT_PERIOD_12HOUR      = 6;
const UINT   REPORT_PERIOD_DAYLY       = 7;
const UINT   REPORT_PERIOD_WEEKLY      = 8;
const UINT   REPORT_PERIOD_BIWEEKLY    = 9;
const UINT   REPORT_PERIOD_MONTHLY     = 10;
const UINT   REPORT_PERIOD_QUARTERLY   = 11;
const UINT   REPORT_PERIOD_ANNUAL      = 12;
const UINT   REPORT_PERIOD_5MIN        = 13;
const UINT   REPORT_PERIOD_15MIN       = 14;

//-------------------------------------------------------------------------------------------------
// Команды управления партионным отчетом
const UINT   REPORT_BATCH_NONE         = 0;
const UINT   REPORT_BATCH_START        = 1;
const UINT   REPORT_BATCH_STOP         = 2;
const UINT   REPORT_BATCH_RESTART      = 3;

//-------------------------------------------------------------------------------------------------
// Статус отчета
const UINT   REPORT_STATUS_IDLE        = 0;
const UINT   REPORT_STATUS_RUNNING     = 1;
const UINT   REPORT_STATUS_WAITING     = 2;
const UINT   REPORT_STATUS_COMPLETED   = 3;

//-------------------------------------------------------------------------------------------------
// Валидность отчета
const UINT   REPORT_MARK_UNDEF         = 0;
const UINT   REPORT_MARK_ILLEGAL       = 1; // Отчет завершен после перезагрузки. Мы не можем гарантировать точность данных
const UINT   REPORT_MARK_INCOMPLETE    = 2; // Отчет начат после перезагрузки, период отчета не полный
const UINT   REPORT_MARK_VALIDATE      = 3;
const UINT   REPORT_MARK_INPROGRESS    = 4; // Отчет в работе

const UINT   REPORT_DEFAULT_STORAGE    = 91;


//-------------------------------------------------------------------------------------------------
// Настройка переменной
const UDINT  VAR_SETUP_CONST           = 1;


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
const std::string DIR_REPORT           = DIR_HOME + DIR_FTP + "reports\\";
const std::string DIR_TIMEINFO         = DIR_HOME + "diag\\";
const std::string FILE_CONF            = DIR_HOME + "tritonn.conf";
const std::string FILE_RESTART         = DIR_HOME + "tritonn.restart";
const std::string FILE_MODBUS          = DIR_HOME + "modbus.xml";
const std::string FILE_SYSTEMTEXT      = DIR_HOME + "systemtext.xml";
const std::string FILE_SYSTEMEVENT     = DIR_HOME + "systemevent.xml";
#else
const std::string DIR_HOME             = "./";
const std::string DIR_CONF             = DIR_HOME + "conf/";
const std::string DIR_FTP              = DIR_HOME + "ftp/";
const std::string DIR_WWW              = DIR_HOME + "www/";
const std::string DIR_REPORT           = DIR_HOME + DIR_FTP + "reports/";
const std::string DIR_TIMEINFO         = DIR_HOME + "diag/";
const std::string FILE_CONF            = DIR_HOME + "tritonn.conf";
const std::string FILE_RESTART         = DIR_HOME + "tritonn.restart";
const std::string FILE_MODBUS          = DIR_HOME + "modbus.xml";
const std::string FILE_SYSTEMTEXT      = DIR_HOME + "systemtext.xml";
const std::string FILE_SYSTEMEVENT     = DIR_HOME + "systemevent.xml";
const std::string FILE_WWW_TREE_OBJ    = DIR_WWW  + "application/core/tree_objects.json";
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
	DATACFGERR_STREAMS,                     // 114
	DATACFGERR_STREAMSNF,
	DATACFGERR_STREAM,                      // 116
	DATACFGERR_DENSSOL,
	DATACFGERR_DENSSOL_NOSTN,               // 118
	DATACFGERR_SELECTOR,                    //
	DATACFGERR_REDUCEDDENS,                 // 120
	DATACFGERR_RESOLVELINK,                 //
	DATACFGERR_CHECKLINK,                   // 122
	DATACFGERR_UNUSED_123,                  // ----------------------------------------------------
	DATACFGERR_MAX_AI,                      // 124
	DATACFGERR_MAX_AO,                      //
	DATACFGERR_MAX_FI,                      // 126
	DATACFGERR_MAX_DI,                      //
	DATACFGERR_MAX_DO,                      // 128
	DATACFGERR_MAX_DENSSOL,                 //
	DATACFGERR_MAX_RDCDENS,                 // 130
	DATACFGERR_MAX_SELECTOR,                //
	DATACFGERR_MAX_STREAM,                  // 132
	DATACFGERR_MAX_STATION,                 //
	DATACFGERR_LIMIT,                       // 134
	DATACFGERR_REPORT,                      //
	DATACFGERR_NOREPORTS,                   // 136
	DATACFGERR_RESOLVETOTAL,                //
	DATACFGERR_VAR,                         // 138
	DATACFGERR_USERS_NF,                    //
	DATACFGERR_INTERNAL,                    // 140
	DATACFGERR_USERS_PARSE,                 //
	DATACFGERR_INTERFACES_NF_TBLOKS,        // 142     Not Found Template BLOCKS
	DATACFGERR_INTERFACES_NF_BLOCKS,        //
	DATACFGERR_INTERFACES_BADADDR,          // 144
	DATACFGERR_INTERFACES_BADBLOCK,         //
	DATACFGERR_INTERFACES_BADVAR,           // 146
	DATACFGERR_INTERFACES_NF_VAR,           //
	DATACFGERR_INTERFACES_ADDROVERFLOW,     // 148
	DATACFGERR_INCORRECT_IP,                //
	DATACFGERR_SECURITY_PARSE,              // 150
	DATACFGERR_SECURITY_DESCRYPT,           //
	DATACFGERR_SECURITY_NF,                 // 152
	DATACFGERR_OPCUA_USER_NF,               //
	DATACFGERR_OPCUA_BAD_USER,              // 154
	DATACFGERR_OPCUA_VAR_NF,                //
	DATACFGERR_UNKNOWN_MODULE,              // 156
	DATACFGERR_IO_CHANNEL,                  //
	DATACFGERR_INVALID_NAME,                // 158
	DATACFGERR_INVALID_MODULELINK,          //
	DATACFGERR_REALTIME_MODULELINK,         // 160
	DATACFGERR_NOTSYSTEXTFILE,              //
	DATACFGERR_INTERFACES_BADNAME,          // 162
	DATACFGERR_DI,                          //
	DATACFGERR_DO,                          // 164
	DATACFGERR_SAMPLER_TOTALS,              //
	DATACFGERR_SAMPLER_CAN,                 // 166
	DATACFGERR_SAMPLER_METHOD,              //
	DATACFGERR_SAMPLER_SETUP,               // 168
	DATACFGERR_SAMPLER_RESERVE,             //
	DATACFGERR_SAMPLER_RESERVE_NF,          // 170

	DATACFGERR_LANG_STRUCT = 200,
	DATACFGERR_LANG_UNKNOW,
	DATACFGERR_LANG_DUPID,
	DATACFGERR_LANG_ID,
	DATACFGERR_LANG_DEFAULT,

	DATACFGERR_PREC_ID = 300,

	XMLFILE_RESULT_NFHASH = 500,
	XMLFILE_RESULT_BADHASH,
	XMLFILE_RESULT_NOTEQUAL,

	// 1024..1999 OPC UA (open62541)
	// 2000 tritonn opc ua
	OPCUA_ERROR_VARNF = 2000,
	OPCUA_ERROR_BADVARTYPE,

};


// Перечень ошибок при файловом вводе-выводе (обычный ввод-вывод, не нить)
//enum FILEIOERROR
//{
//	FILEIOERROR_CFG = 1,
//	FILEIOERROR_JSONTREE,
//};




#define TRACE(mask, format, ...)              {rLogManager::Instance().Add(       (mask), __FILENAME__, __LINE__, (format), ##__VA_ARGS__);}
#define TRACEI(mask, format, ...)             {rLogManager::Instance().Add(LM_I | (mask), __FILENAME__, __LINE__, (format), ##__VA_ARGS__);}
#define TRACEW(mask, format, ...)             {rLogManager::Instance().Add(LM_W | (mask), __FILENAME__, __LINE__, (format), ##__VA_ARGS__);}
#define TRACEA(mask, format, ...)             {rLogManager::Instance().Add(LM_A | (mask), __FILENAME__, __LINE__, (format), ##__VA_ARGS__);}
#define TRACEP(mask, format, ...)             {rLogManager::Instance().Add(LM_P | (mask), __FILENAME__, __LINE__, (format), ##__VA_ARGS__);}
#define TRACEERROR(format, ...)               {rLogManager::OutErr(            __FILENAME__, __LINE__, (format), ##__VA_ARGS__);}



///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Предописание вспомогательных функций
//
extern void        mSleep(UDINT msec);

extern void        GetCurrentTime(Time64_T &t64, struct tm *stm);

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



