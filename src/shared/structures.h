//=================================================================================================
//===
//=== threadclass.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Заголовочный файл, для описания структур
//===
//=================================================================================================

#pragma once

#include <string>
#include "types.h"
#include "def.h"

struct rVersion
{
	USINT m_major;
	USINT m_minor;
	UINT  m_build;
	UDINT m_hash;
//	USINT m_status; // Стату сборки 0 - betta, 1 - nonstable, 2 - stable
};

struct rMetrologyVer
{
	USINT m_major;
	USINT m_minor;
	UDINT m_crc;    // Контрольная сумма
};


// Информация об загруженной конфигурации
// Структура на массивах для того что бы ее можно было спокойно положить в rPacket_*
struct rConfigInfo
{
	char File[MAX_CONFIG_NAME];      // Путь до конфигурации (файл)
	char Name[MAX_CONFIG_NAME];      // Имя конфигурации. Срока "Наименование"
	char Version[MAX_CFGVER_SIZE];   // Версия кофигурации //NOTE Может сделать версию просто числом?
	char Developer[MAX_CONFIG_NAME]; // Срока "Разработчик"
	char Hash[MAX_HASH_SIZE];        // Контрольная сумма конфигурации
//	std::string m_filename; //TODO для protobuf нужно перейти на строки
//	std::string m_name;
//	std::string m_version;
//	std::string m_developer;
//	std::string m_hash;
};


// Состояние работы ядра
struct rState
{
	UDINT m_eventAlarm;   // Кол-во аварийных не квитированных сообщений
	Live  m_live;         // Текущий статус
//	UDINT HaltReason;     // Код ошибки перехода в Halt //TODO Нужно ли это
//	USINT ColdRestart;    // Команда на "холодную" перезагрузку
//	USINT WarmRestart;    // Команда на "горячую" перезагрузку
	USINT StartReason;    // Причина последней перезагрузки
	USINT m_isSimulate;
};

struct rEthernet
{
	std::string m_dev;
	std::string m_ip;
	std::string m_mask;
	std::string m_gateway;
};

// Учет времени работы потоков и функций
class rProfile
{
public:
	rProfile();
	virtual ~rProfile();

	UDINT Middle; // Среднее время
	UDINT Max;    // Максимальное пиковое
	UDINT Min;    // Минимальное пиковое
	UDINT MidMax; // Среднепиковое, в расчет идут значения выше 50% между midle и max

	UDINT CountMid;    // Счетчик проходов
	UDINT CountMidMax; //

	void Start();
	void Stop();
};
